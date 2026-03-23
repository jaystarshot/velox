/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "velox/connectors/hive/storage_adapters/gcs/GcsReadFile.h"
#include "velox/connectors/hive/storage_adapters/gcs/GcsUtil.h"


#include <chrono>
#include <cstdlib>
#include <mutex>
#include <optional>
#include <nlohmann/json.hpp>
#include "google/cloud/common_options.h"
#include "google/cloud/storage/async/bucket_name.h"
#include "google/cloud/storage/async/object_descriptor.h"
#include "google/cloud/storage/async/read_all.h"
#include "velox/common/base/Counters.h"
#include "velox/common/base/StatsReporter.h"

namespace facebook::velox::filesystems {

namespace gc = ::google::cloud;
namespace gcs = ::google::cloud::storage;
namespace gcse = ::google::cloud::storage_experimental;

class GcsReadFile::Impl {
 public:
  Impl(
      const std::string& path,
      std::shared_ptr<gcs::Client> client,
      std::shared_ptr<gcse::AsyncClient> asyncClient)
      : client_(std::move(client)), asyncClient_(std::move(asyncClient)) {
    setBucketAndKeyFromGcsPath(path, bucket_, key_);
  }

  // Gets the length of the file.
  // Checks if there are any issues reading the file.
  void initialize(const filesystems::FileOptions& options) {
    if (options.fileSize.has_value()) {
      VELOX_CHECK_GE(
          options.fileSize.value(), 0, "File size must be non-negative");
      length_ = options.fileSize.value();
    }

    // Make it a no-op if invoked twice.
    if (length_ != -1) {
      return;
    }
    // get metadata and initialize length
    auto metadata = client_->GetObjectMetadata(bucket_, key_);
    if (!metadata.ok()) {
      checkGcsStatus(
          metadata.status(),
          "Failed to get metadata for GCS object",
          bucket_,
          key_);
    }
    length_ = (*metadata).size();
    VELOX_CHECK_GE(length_, 0);
  }

  std::string_view pread(
      uint64_t offset,
      uint64_t length,
      void* buffer,
      std::atomic<uint64_t>& bytesRead,
      const FileIoContext& context) const {
    preadInternal(offset, length, static_cast<char*>(buffer), bytesRead);
    return {static_cast<char*>(buffer), length};
  }

  std::string pread(
      uint64_t offset,
      uint64_t length,
      std::atomic<uint64_t>& bytesRead,
      const FileIoContext& context) const {
    std::string result(length, 0);
    preadInternal(offset, length, result.data(), bytesRead);
    return result;
  }

  uint64_t preadv(
      uint64_t offset,
      const std::vector<folly::Range<char*>>& buffers,
      std::atomic<uint64_t>& bytesRead,
      const FileIoContext& context) const {
    // 'buffers' contains Ranges(data, size)  with some gaps (data = nullptr) in
    // between. This call must populate the ranges (except gap ranges)
    // sequentially starting from 'offset'. If a range pointer is nullptr, the
    // data from stream of size range.size() will be skipped.
    size_t length = 0;
    size_t wastedBytes = 0;
    size_t dataRanges = 0;
    for (const auto range : buffers) {
      length += range.size();
      if (range.data()) {
        ++dataRanges;
      } else {
        wastedBytes += range.size();
      }
    }
    RECORD_METRIC_VALUE(facebook::velox::kMetricGCSPreadVCount, 1);
    RECORD_METRIC_VALUE(
        facebook::velox::kMetricGCSPreadVBufferListSizeAvg, dataRanges);
    RECORD_METRIC_VALUE(facebook::velox::kMetricGCSPreadVBytesRead, length);
    RECORD_METRIC_VALUE(
        facebook::velox::kMetricGCSPreadVWastedBytes, wastedBytes);

    // Sync path: read the entire contiguous range and scatter.
    std::string result(length, 0);
    preadInternal(offset, length, result.data(), bytesRead);
    size_t resultOffset = 0;
    for (auto range : buffers) {
      if (range.data()) {
        memcpy(range.data(), &(result.data()[resultOffset]), range.size());
      }
      resultOffset += range.size();
    }
    return length;
  }

  uint64_t size() const {
    return length_;
  }

  uint64_t memoryUsage() const {
    return sizeof(GcsReadFile) // this class
        + sizeof(gcs::Client) // pointee
        + kUploadBufferSize; // buffer size
  }

  std::string getName() const {
    return key_;
  }

 private:
  void ensureDescriptorOpen() const {
    std::call_once(descriptorInitFlag_, [this]() {
      RECORD_METRIC_VALUE(facebook::velox::kMetricGCSBidiOpenCount, 1);
      auto result =
          asyncClient_->Open(gcse::BucketName(bucket_), key_).get();
      VELOX_CHECK(
          result.ok(),
          "Failed to open bidi descriptor for {}/{}: {}",
          bucket_,
          key_,
          result.status().message());
      descriptor_.emplace(*std::move(result));
    });
  }

  void readBidi(uint64_t offset, uint64_t length, char* buffer) const {
    RECORD_METRIC_VALUE(facebook::velox::kMetricGCSBidiReadCount, 1);
    ensureDescriptorOpen();
    auto [reader, token] = descriptor_->Read(offset, length);
    auto payload = gcse::ReadAll(std::move(reader), std::move(token)).get();
    VELOX_CHECK(
        payload.ok(),
        "Bidi read failed for {}/{} at offset {} length {}: {}",
        bucket_,
        key_,
        offset,
        length,
        payload.status().message());
    size_t written = 0;
    for (auto sv : payload->contents()) {
      memcpy(buffer + written, sv.data(), sv.size());
      written += sv.size();
    }
  }

  void preadInternal(
      uint64_t offset,
      uint64_t length,
      char* position,
      std::atomic<uint64_t>& bytesRead_) const {
    if (asyncClient_) {
      auto startTime = std::chrono::high_resolution_clock::now();
      readBidi(offset, length, position);
      recordLatency(startTime);
      bytesRead_ += length;
      emitReadMetric(length);
      return;
    }
    auto startTime = std::chrono::high_resolution_clock::now();

    gcs::ObjectReadStream stream = client_->ReadObject(
        bucket_,
        key_,
        gcs::ReadRange(offset, offset + length),
        gcsRequestOptions());
    if (!stream) {
      checkGcsStatus(
          stream.status(), "Failed to get GCS object", bucket_, key_);
    }

    stream.read(position, length);
    recordLatency(startTime);

    if (!stream) {
      checkGcsStatus(
          stream.status(), "Failed to get read object", bucket_, key_);
    }
    bytesRead_ += length;
    emitReadMetric(length);
  }

  void recordLatency(
      std::chrono::high_resolution_clock::time_point startTime) const {
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::high_resolution_clock::now() - startTime)
                          .count();
    RECORD_METRIC_VALUE(facebook::velox::kMetricGCSReadLatency, durationMs);
  }

  void emitReadMetric(uint64_t length) const {
    if (length < 100 * 1024) {
      RECORD_METRIC_VALUE(facebook::velox::kMetricGCSRead0To1kb, 1);
    } else if (length < 1024 * 1024) {
      RECORD_METRIC_VALUE(facebook::velox::kMetricGCSRead1kbTo1mb, 1);
    } else if (length < 10 * 1024 * 1024) {
      RECORD_METRIC_VALUE(facebook::velox::kMetricGCSRead1mbTo10mb, 1);
    } else {
      RECORD_METRIC_VALUE(facebook::velox::kMetricGCSRead10mb, 1);
    }
    RECORD_METRIC_VALUE(facebook::velox::kMetricGCSReadSum, length);
  }

  std::shared_ptr<gcs::Client> client_;
  std::shared_ptr<gcse::AsyncClient> asyncClient_;
  std::string bucket_;
  std::string key_;
  std::atomic<int64_t> length_ = -1;

  mutable std::once_flag descriptorInitFlag_;
  mutable std::optional<gcse::ObjectDescriptor> descriptor_;
};

GcsReadFile::GcsReadFile(
    const std::string& path,
    std::shared_ptr<gcs::Client> client,
    std::shared_ptr<gcse::AsyncClient> asyncClient)
    : impl_(std::make_unique<Impl>(
          path,
          std::move(client),
          std::move(asyncClient))) {}

GcsReadFile::~GcsReadFile() = default;

void GcsReadFile::initialize(const filesystems::FileOptions& options) {
  impl_->initialize(options);
}

std::string_view GcsReadFile::pread(
    uint64_t offset,
    uint64_t length,
    void* buffer,
    const FileIoContext& context) const {
  return impl_->pread(offset, length, buffer, bytesRead_, context);
}

std::string GcsReadFile::pread(
    uint64_t offset,
    uint64_t length,
    const FileIoContext& context) const {
  return impl_->pread(offset, length, bytesRead_, context);
}
uint64_t GcsReadFile::preadv(
    uint64_t offset,
    const std::vector<folly::Range<char*>>& buffers,
    const FileIoContext& context) const {
  return impl_->preadv(offset, buffers, bytesRead_, context);
}

uint64_t GcsReadFile::size() const {
  return impl_->size();
}

uint64_t GcsReadFile::memoryUsage() const {
  return impl_->memoryUsage();
}

std::string GcsReadFile::getName() const {
  return impl_->getName();
}

uint64_t GcsReadFile::getNaturalReadSize() const {
  return kUploadBufferSize;
}

} // namespace facebook::velox::filesystems
