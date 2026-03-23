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

#include "velox/common/config/Config.h"
#include "velox/common/file/File.h"
#include "velox/connectors/hive/HiveConfig.h"
#include "velox/connectors/hive/storage_adapters/gcs/GcsFileSystem.h"
#include "velox/connectors/hive/storage_adapters/gcs/GcsUtil.h"

#include <folly/init/Init.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

DEFINE_string(gcs_path, "", "GCS file path (e.g. gs://bucket/object)");
DEFINE_string(
    gcs_credentials,
    "",
    "Path to GCS service account JSON key file");
DEFINE_int32(iterations, 10, "Number of iterations per benchmark");
DEFINE_string(
    mode,
    "all",
    "Benchmark mode: 'sequential', 'random', 'preadv', or 'all'");
DEFINE_int32(
    read_size_kb,
    0,
    "Read size in KB for random reads (0 = run default set)");
DEFINE_int32(preadv_ranges, 8, "Number of data ranges in preadv test");
DEFINE_int32(preadv_gap_kb, 64, "Gap size in KB between preadv ranges");

using Clock = std::chrono::high_resolution_clock;

constexpr size_t kKiB = 1024;
constexpr size_t kMiB = 1024 * kKiB;

struct Result {
  int64_t duration_ms;
  size_t bytes_read;
};

struct BenchmarkStats {
  std::string mode;
  size_t readSize;
  size_t dataSize;
  double avgMs;
  int64_t p50Ms;
  int64_t p90Ms;
  int64_t minMs;
  int64_t maxMs;
  double throughputMBs;
};

std::shared_ptr<const facebook::velox::config::ConfigBase> makeConfig(
    bool asyncEnabled) {
  std::unordered_map<std::string, std::string> cfg;
  if (!FLAGS_gcs_credentials.empty()) {
    cfg[facebook::velox::connector::hive::HiveConfig::kGcsCredentialsPath] =
        FLAGS_gcs_credentials;
  }
  cfg[facebook::velox::connector::hive::HiveConfig::kGcsReadAsyncEnabled] =
      asyncEnabled ? "true" : "false";
  return std::make_shared<const facebook::velox::config::ConfigBase>(
      std::move(cfg));
}

std::unique_ptr<facebook::velox::ReadFile> openFile(
    const std::string& path,
    bool asyncEnabled) {
  auto config = makeConfig(asyncEnabled);
  auto gcfs =
      std::make_unique<facebook::velox::filesystems::GcsFileSystem>(config);
  gcfs->initializeClient();
  return gcfs->openFileForRead(path);
}

Result benchSequential(facebook::velox::ReadFile& file, size_t bufferSize) {
  auto fileSize = file.size();
  std::vector<char> buffer(bufferSize);
  size_t totalRead = 0;

  auto start = Clock::now();
  for (uint64_t offset = 0; offset < fileSize; offset += bufferSize) {
    uint64_t toRead =
        std::min(bufferSize, static_cast<size_t>(fileSize - offset));
    file.pread(offset, toRead, buffer.data());
    totalRead += toRead;
  }
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          Clock::now() - start)
          .count();
  return {elapsed, totalRead};
}

Result benchRandom(facebook::velox::ReadFile& file, size_t readSize) {
  auto fileSize = file.size();
  if (fileSize == 0 || readSize == 0)
    return {0, 0};

  std::vector<uint64_t> offsets;
  for (uint64_t o = 0; o < fileSize; o += readSize) {
    offsets.push_back(o);
  }
  std::mt19937 gen(42);
  std::shuffle(offsets.begin(), offsets.end(), gen);

  std::vector<char> buffer(readSize);
  size_t totalRead = 0;

  auto start = Clock::now();
  for (auto offset : offsets) {
    auto toRead =
        std::min(readSize, static_cast<size_t>(fileSize - offset));
    file.pread(offset, toRead, buffer.data());
    totalRead += toRead;
  }
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          Clock::now() - start)
          .count();
  return {elapsed, totalRead};
}

Result benchPreadv(
    facebook::velox::ReadFile& file,
    int numRanges,
    size_t rangeSize,
    size_t gapSize) {
  auto fileSize = file.size();
  size_t totalSpan = numRanges * rangeSize + (numRanges - 1) * gapSize;
  if (totalSpan > fileSize) {
    totalSpan = fileSize;
  }

  std::vector<folly::Range<char*>> buffers;
  std::vector<std::vector<char>> dataBuffers;
  dataBuffers.reserve(numRanges);

  size_t remaining = totalSpan;
  for (int i = 0; i < numRanges && remaining > 0; ++i) {
    size_t dataSize = std::min(rangeSize, remaining);
    dataBuffers.emplace_back(dataSize);
    buffers.emplace_back(dataBuffers.back().data(), dataSize);
    remaining -= dataSize;

    if (i < numRanges - 1 && remaining > 0) {
      size_t gap = std::min(gapSize, remaining);
      buffers.emplace_back(nullptr, gap);
      remaining -= gap;
    }
  }

  size_t totalRead = 0;
  for (const auto& r : buffers) {
    if (r.data())
      totalRead += r.size();
  }

  auto start = Clock::now();
  file.preadv(0, buffers);
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          Clock::now() - start)
          .count();
  return {elapsed, totalRead};
}

BenchmarkStats computeStats(
    const std::string& mode,
    size_t dataSize,
    size_t readSize,
    const std::vector<int64_t>& durations) {
  BenchmarkStats stats{};
  stats.mode = mode;
  stats.readSize = readSize;
  stats.dataSize = dataSize;

  if (durations.empty()) {
    return stats;
  }

  auto sorted = durations;
  std::sort(sorted.begin(), sorted.end());
  auto n = sorted.size();
  auto total = std::accumulate(sorted.begin(), sorted.end(), 0LL);
  stats.avgMs = static_cast<double>(total) / n;
  stats.p50Ms = sorted[n / 2];
  stats.p90Ms = sorted[std::min(static_cast<size_t>(n * 0.9), n - 1)];
  stats.minMs = sorted.front();
  stats.maxMs = sorted.back();
  double dataMB = static_cast<double>(dataSize) / kMiB;
  stats.throughputMBs =
      stats.avgMs > 0 ? dataMB / (stats.avgMs / 1000.0) : 0;
  return stats;
}

void printStats(const BenchmarkStats& s, int iterations) {
  std::cout << "\n==== " << s.mode << " ====\n";
  double dataMB = static_cast<double>(s.dataSize) / kMiB;
  std::cout << "  Data size:  " << std::fixed << std::setprecision(2) << dataMB
            << " MB\n";
  if (s.readSize > 0) {
    std::cout << "  Read size:  " << s.readSize / kKiB << " KB\n";
  }
  std::cout << "  Iterations: " << iterations << "\n";
  std::cout << "  Avg:        " << std::fixed << std::setprecision(1)
            << s.avgMs << " ms\n";
  std::cout << "  P50:        " << s.p50Ms << " ms\n";
  std::cout << "  P90:        " << s.p90Ms << " ms\n";
  std::cout << "  Min:        " << s.minMs << " ms\n";
  std::cout << "  Max:        " << s.maxMs << " ms\n";
  std::cout << "  Throughput: " << std::fixed << std::setprecision(2)
            << s.throughputMBs << " MB/s\n";
}

std::vector<BenchmarkStats> runBenchmark(
    const std::string& label,
    bool asyncEnabled,
    const std::string& mode,
    size_t fileSize) {
  auto file = openFile(FLAGS_gcs_path, asyncEnabled);
  std::vector<BenchmarkStats> allStats;

  if (mode == "sequential" || mode == "all") {
    std::vector<int64_t> durations;
    for (int i = 0; i < FLAGS_iterations; ++i) {
      auto r = benchSequential(*file, 4 * kMiB);
      std::cout << "  [" << label << " seq] iter " << i + 1 << ": "
                << r.bytes_read / kMiB << " MB in " << r.duration_ms
                << " ms\n";
      durations.push_back(r.duration_ms);
    }
    auto stats =
        computeStats(label + " Sequential", fileSize, 0, durations);
    printStats(stats, FLAGS_iterations);
    allStats.push_back(stats);
  }

  if (mode == "random" || mode == "all") {
    std::vector<size_t> sizes;
    if (FLAGS_read_size_kb > 0) {
      sizes.push_back(static_cast<size_t>(FLAGS_read_size_kb) * kKiB);
    } else {
      sizes = {4 * kMiB, 1 * kMiB};
    }
    for (auto sz : sizes) {
      std::vector<int64_t> durations;
      for (int i = 0; i < FLAGS_iterations; ++i) {
        auto r = benchRandom(*file, sz);
        std::cout << "  [" << label << " rand " << sz / kKiB << "KB] iter "
                  << i + 1 << ": " << r.bytes_read / kMiB << " MB in "
                  << r.duration_ms << " ms\n";
        durations.push_back(r.duration_ms);
      }
      auto stats =
          computeStats(label + " Random " + std::to_string(sz / kKiB) + "KB",
                       fileSize, sz, durations);
      printStats(stats, FLAGS_iterations);
      allStats.push_back(stats);
    }
  }

  if (mode == "preadv" || mode == "all") {
    size_t rangeSize = 1 * kMiB;
    size_t gapSize = static_cast<size_t>(FLAGS_preadv_gap_kb) * kKiB;
    std::vector<int64_t> durations;
    for (int i = 0; i < FLAGS_iterations; ++i) {
      auto r = benchPreadv(*file, FLAGS_preadv_ranges, rangeSize, gapSize);
      std::cout << "  [" << label << " preadv] iter " << i + 1 << ": "
                << r.bytes_read / kKiB << " KB in " << r.duration_ms
                << " ms\n";
      durations.push_back(r.duration_ms);
    }
    size_t totalDataSize = FLAGS_preadv_ranges * rangeSize;
    auto stats =
        computeStats(label + " Preadv", totalDataSize, rangeSize, durations);
    printStats(stats, FLAGS_iterations);
    allStats.push_back(stats);
  }

  return allStats;
}

void printComparisonSummary(
    const std::vector<BenchmarkStats>& syncStats,
    const std::vector<BenchmarkStats>& asyncStats) {
  std::cout << "\n";
  std::cout << "================================================================"
               "====================\n";
  std::cout << "  COMPARISON SUMMARY\n";
  std::cout << "================================================================"
               "====================\n";

  // Header row: short mode names derived from stats
  auto shortMode = [](const BenchmarkStats& s) -> std::string {
    // Strip "Sync " or "Async " prefix to get the benchmark name
    auto pos = s.mode.find(' ');
    return pos != std::string::npos ? s.mode.substr(pos + 1) : s.mode;
  };

  std::cout << std::left << std::setw(22) << "Benchmark" << std::right
            << std::setw(12) << "Sync Avg" << std::setw(12) << "Async Avg"
            << std::setw(10) << "Speedup" << std::setw(14) << "Sync MB/s"
            << std::setw(14) << "Async MB/s" << "\n";
  std::cout << std::string(84, '-') << "\n";

  size_t count = std::min(syncStats.size(), asyncStats.size());
  for (size_t i = 0; i < count; ++i) {
    double speedup = (asyncStats[i].avgMs > 0)
        ? syncStats[i].avgMs / asyncStats[i].avgMs
        : 0;
    std::string name = shortMode(syncStats[i]);

    std::cout << std::left << std::setw(22) << name << std::right
              << std::setw(10) << std::fixed << std::setprecision(0)
              << syncStats[i].avgMs << " ms" << std::setw(10)
              << asyncStats[i].avgMs << " ms" << std::setw(8)
              << std::setprecision(2) << speedup << "x" << std::setw(11)
              << std::setprecision(2) << syncStats[i].throughputMBs
              << " MB/s" << std::setw(11) << asyncStats[i].throughputMBs
              << " MB/s"
              << "\n";
  }
  std::cout << std::string(84, '=') << "\n";
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage(
      "GCS sync vs async read benchmark.\n"
      "Usage: " +
      std::string(argv[0]) +
      " --gcs_path=gs://bucket/object [--gcs_credentials=key.json]");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::Init init(&argc, &argv);

  if (FLAGS_gcs_path.empty()) {
    std::cerr << "Error: --gcs_path is required.\n";
    gflags::ShowUsageWithFlags(argv[0]);
    return 1;
  }

  auto syncFile = openFile(FLAGS_gcs_path, false);
  auto fileSize = syncFile->size();
  syncFile.reset();
  std::cout << "File: " << FLAGS_gcs_path << " (" << fileSize / kMiB
            << " MB)\n\n";

  std::cout << "========================================\n";
  std::cout << "  SYNC (ReadObject) path\n";
  std::cout << "========================================\n";
  auto syncStats = runBenchmark("Sync", false, FLAGS_mode, fileSize);

  std::cout << "\n========================================\n";
  std::cout << "  ASYNC (Bidi ObjectDescriptor) path\n";
  std::cout << "========================================\n";
  auto asyncStats = runBenchmark("Async", true, FLAGS_mode, fileSize);

  printComparisonSummary(syncStats, asyncStats);

  std::cout << "\nDone.\n";
  return 0;
}
