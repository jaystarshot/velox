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

#pragma once

#include <folly/Range.h>

namespace facebook::velox {

/// Velox metrics Registration.
void registerVeloxMetrics();

constexpr std::string_view kMetricHiveFileHandleGenerateLatencyMs{
    "velox.hive_file_handle_generate_latency_ms"};

constexpr std::string_view kMetricCacheShrinkCount{"velox.cache_shrink_count"};

constexpr std::string_view kMetricCacheShrinkTimeMs{"velox.cache_shrink_ms"};

constexpr std::string_view kMetricMaxSpillLevelExceededCount{
    "velox.spill_max_level_exceeded_count"};

constexpr std::string_view kMetricQueryMemoryReclaimTimeMs{
    "velox.query_memory_reclaim_time_ms"};

constexpr std::string_view kMetricQueryMemoryReclaimedBytes{
    "velox.query_memory_reclaim_bytes"};

constexpr std::string_view kMetricQueryMemoryReclaimCount{
    "velox.query_memory_reclaim_count"};

constexpr std::string_view kMetricTaskMemoryReclaimCount{
    "velox.task_memory_reclaim_count"};

constexpr std::string_view kMetricTaskMemoryReclaimWaitTimeMs{
    "velox.task_memory_reclaim_wait_ms"};

constexpr std::string_view kMetricTaskMemoryReclaimExecTimeMs{
    "velox.task_memory_reclaim_exec_ms"};

constexpr std::string_view kMetricTaskMemoryReclaimWaitTimeoutCount{
    "velox.task_memory_reclaim_wait_timeout_count"};

constexpr std::string_view kMetricTaskSplitsCount{"velox.task_splits_count"};

constexpr std::string_view kMetricOpMemoryReclaimTimeMs{
    "velox.op_memory_reclaim_time_ms"};

constexpr std::string_view kMetricOpMemoryReclaimedBytes{
    "velox.op_memory_reclaim_bytes"};

constexpr std::string_view kMetricOpMemoryReclaimCount{
    "velox.op_memory_reclaim_count"};

constexpr std::string_view kMetricMemoryNonReclaimableCount{
    "velox.memory_non_reclaimable_count"};

constexpr std::string_view kMetricMemoryPoolInitialCapacityBytes{
    "velox.memory_pool_initial_capacity_bytes"};

constexpr std::string_view kMetricMemoryPoolCapacityGrowCount{
    "velox.memory_pool_capacity_growth_count"};

constexpr std::string_view kMetricMemoryPoolUsageLeakBytes{
    "velox.memory_pool_usage_leak_bytes"};

constexpr std::string_view kMetricMemoryPoolReservationLeakBytes{
    "velox.memory_pool_reservation_leak_bytes"};

constexpr std::string_view kMetricMemoryAllocatorDoubleFreeCount{
    "velox.memory_allocator_double_free_count"};

constexpr std::string_view kMetricArbitratorLocalArbitrationCount{
    "velox.arbitrator_local_arbitration_count"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationCount{
    "velox.arbitrator_global_arbitration_count"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationNumReclaimVictims{
    "velox.arbitrator_global_arbitration_num_reclaim_victims"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationFailedVictimCount{
    "velox.arbitrator_global_arbitration_failed_victim_count"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationBytes{
    "velox.arbitrator_global_arbitration_bytes"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationTimeMs{
    "velox.arbitrator_global_arbitration_time_ms"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationWaitCount{
    "velox.arbitrator_global_arbitration_wait_count"};

constexpr std::string_view kMetricArbitratorGlobalArbitrationWaitTimeMs{
    "velox.arbitrator_global_arbitration_wait_time_ms"};

constexpr std::string_view kMetricArbitratorAbortedCount{
    "velox.arbitrator_aborted_count"};

constexpr std::string_view kMetricArbitratorFailuresCount{
    "velox.arbitrator_failures_count"};

constexpr std::string_view kMetricArbitratorOpExecTimeMs{
    "velox.arbitrator_op_exec_time_ms"};

constexpr std::string_view kMetricArbitratorFreeCapacityBytes{
    "velox.arbitrator_free_capacity_bytes"};

constexpr std::string_view kMetricArbitratorFreeReservedCapacityBytes{
    "velox.arbitrator_free_reserved_capacity_bytes"};

constexpr std::string_view kMetricDriverYieldCount{"velox.driver_yield_count"};

constexpr std::string_view kMetricDriverQueueTimeMs{
    "velox.driver_queue_time_ms"};

constexpr std::string_view kMetricDriverExecTimeMs{"velox.driver_exec_time_ms"};

constexpr std::string_view kMetricSpilledInputBytes{"velox.spill_input_bytes"};

constexpr std::string_view kMetricSpilledBytes{"velox.spill_bytes"};

constexpr std::string_view kMetricSpilledRowsCount{"velox.spill_rows_count"};

constexpr std::string_view kMetricSpilledFilesCount{"velox.spill_files_count"};

constexpr std::string_view kMetricSpillFillTimeMs{"velox.spill_fill_time_ms"};

constexpr std::string_view kMetricSpillSortTimeMs{"velox.spill_sort_time_ms"};

constexpr std::string_view kMetricSpillExtractVectorTimeMs{
    "velox.spill_extract_vector_time_ms"};

constexpr std::string_view kMetricSpillSerializationTimeMs{
    "velox.spill_serialization_time_ms"};

constexpr std::string_view kMetricSpillWritesCount{"velox.spill_writes_count"};

constexpr std::string_view kMetricSpillFlushTimeMs{"velox.spill_flush_time_ms"};

constexpr std::string_view kMetricSpillWriteTimeMs{"velox.spill_write_time_ms"};

constexpr std::string_view kMetricSpillMemoryBytes{"velox.spill_memory_bytes"};

constexpr std::string_view kMetricSpillPeakMemoryBytes{
    "velox.spill_peak_memory_bytes"};

constexpr std::string_view kMetricFileWriterEarlyFlushedRawBytes{
    "velox.file_writer_early_flushed_raw_bytes"};

constexpr std::string_view kMetricHiveSortWriterFinishTimeMs{
    "velox.hive_sort_writer_finish_time_ms"};

constexpr std::string_view kMetricArbitratorRequestsCount{
    "velox.arbitrator_requests_count"};

constexpr std::string_view kMetricMemoryAllocatorMappedBytes{
    "velox.memory_allocator_mapped_bytes"};

constexpr std::string_view kMetricMemoryAllocatorExternalMappedBytes{
    "velox.memory_allocator_external_mapped_bytes"};

constexpr std::string_view kMetricMemoryAllocatorAllocatedBytes{
    "velox.memory_allocator_allocated_bytes"};

constexpr std::string_view kMetricMemoryAllocatorTotalUsedBytes{
    "velox.memory_allocator_total_used_bytes"};

constexpr std::string_view kMetricMmapAllocatorDelegatedAllocatedBytes{
    "velox.mmap_allocator_delegated_allocated_bytes"};

constexpr std::string_view kMetricCacheMaxAgeSecs{"velox.cache_max_age_secs"};

constexpr std::string_view kMetricMemoryCacheNumTinyEntries{
    "velox.memory_cache_num_tiny_entries"};

constexpr std::string_view kMetricMemoryCacheNumLargeEntries{
    "velox.memory_cache_num_large_entries"};

constexpr std::string_view kMetricMemoryCacheNumEmptyEntries{
    "velox.memory_cache_num_empty_entries"};

constexpr std::string_view kMetricMemoryCacheNumSharedEntries{
    "velox.memory_cache_num_shared_entries"};

constexpr std::string_view kMetricMemoryCacheNumExclusiveEntries{
    "velox.memory_cache_num_exclusive_entries"};

constexpr std::string_view kMetricMemoryCacheNumPrefetchedEntries{
    "velox.memory_cache_num_prefetched_entries"};

constexpr std::string_view kMetricMemoryCacheTotalTinyBytes{
    "velox.memory_cache_total_tiny_bytes"};

constexpr std::string_view kMetricMemoryCacheTotalLargeBytes{
    "velox.memory_cache_total_large_bytes"};

constexpr std::string_view kMetricMemoryCacheTotalTinyPaddingBytes{
    "velox.memory_cache_total_tiny_padding_bytes"};

constexpr std::string_view kMetricMemoryCacheTotalLargePaddingBytes{
    "velox.memory_cache_total_large_padding_bytes"};

constexpr std::string_view kMetricMemoryCacheTotalPrefetchBytes{
    "velox.memory_cache_total_prefetched_bytes"};

constexpr std::string_view kMetricMemoryCacheSumEvictScore{
    "velox.memory_cache_sum_evict_score"};

constexpr std::string_view kMetricMemoryCacheNumHits{
    "velox.memory_cache_num_hits"};

constexpr std::string_view kMetricMemoryCacheHitBytes{
    "velox.memory_cache_hit_bytes"};

constexpr std::string_view kMetricMemoryCacheNumNew{
    "velox.memory_cache_num_new"};

constexpr std::string_view kMetricMemoryCacheNumEvicts{
    "velox.memory_cache_num_evicts"};

constexpr std::string_view kMetricMemoryCacheNumSavableEvicts{
    "velox.memory_cache_num_savable_evicts"};

constexpr std::string_view kMetricMemoryCacheNumEvictChecks{
    "velox.memory_cache_num_evict_checks"};

constexpr std::string_view kMetricMemoryCacheNumWaitExclusive{
    "velox.memory_cache_num_wait_exclusive"};

constexpr std::string_view kMetricMemoryCacheNumAllocClocks{
    "velox.memory_cache_num_alloc_clocks"};

constexpr std::string_view kMetricMemoryCacheNumAgedOutEntries{
    "velox.memory_cache_num_aged_out_entries"};

constexpr std::string_view kMetricMemoryCacheNumStaleEntries{
    "velox.memory_cache_num_stale_entries"};

constexpr std::string_view kMetricSsdCacheCachedRegions{
    "velox.ssd_cache_cached_regions"};

constexpr std::string_view kMetricSsdCacheCachedEntries{
    "velox.ssd_cache_cached_entries"};

constexpr std::string_view kMetricSsdCacheCachedBytes{
    "velox.ssd_cache_cached_bytes"};

constexpr std::string_view kMetricSsdCacheReadEntries{
    "velox.ssd_cache_read_entries"};

constexpr std::string_view kMetricSsdCacheReadBytes{
    "velox.ssd_cache_read_bytes"};

constexpr std::string_view kMetricSsdCacheWrittenEntries{
    "velox.ssd_cache_written_entries"};

constexpr std::string_view kMetricSsdCacheWrittenBytes{
    "velox.ssd_cache_written_bytes"};

constexpr std::string_view kMetricSsdCacheAgedOutEntries{
    "velox.ssd_cache_aged_out_entries"};

constexpr std::string_view kMetricSsdCacheAgedOutRegions{
    "velox.ssd_cache_aged_out_regions"};

constexpr std::string_view kMetricSsdCacheOpenSsdErrors{
    "velox.ssd_cache_open_ssd_errors"};

constexpr std::string_view kMetricSsdCacheOpenCheckpointErrors{
    "velox.ssd_cache_open_checkpoint_errors"};

constexpr std::string_view kMetricSsdCacheOpenLogErrors{
    "velox.ssd_cache_open_log_errors"};

constexpr std::string_view kMetricSsdCacheMetaFileDeleteErrors{
    "velox.ssd_cache_delete_meta_file_errors"};

constexpr std::string_view kMetricSsdCacheGrowFileErrors{
    "velox.ssd_cache_grow_file_errors"};

constexpr std::string_view kMetricSsdCacheWriteSsdErrors{
    "velox.ssd_cache_write_ssd_errors"};

constexpr std::string_view kMetricSsdCacheWriteNoSpaceErrors{
    "velox.ssd_cache_write_no_space_errors"};

constexpr std::string_view kMetricSsdCacheWriteSsdDropped{
    "velox.ssd_cache_write_ssd_dropped"};

constexpr std::string_view kMetricSsdCacheWriteExceedEntryLimit{
    "velox.ssd_cache_write_exceed_entry_limit"};

constexpr std::string_view kMetricSsdCacheWriteCheckpointErrors{
    "velox.ssd_cache_write_checkpoint_errors"};

constexpr std::string_view kMetricSsdCacheReadCorruptions{
    "velox.ssd_cache_read_corruptions"};

constexpr std::string_view kMetricSsdCacheReadSsdErrors{
    "velox.ssd_cache_read_ssd_errors"};

constexpr std::string_view kMetricSsdCacheReadCheckpointErrors{
    "velox.ssd_cache_read_checkpoint_errors"};

constexpr std::string_view kMetricSsdCacheReadWithoutChecksum{
    "velox.ssd_cache_read_without_checksum"};

constexpr std::string_view kMetricSsdCacheCheckpointsRead{
    "velox.ssd_cache_checkpoints_read"};

constexpr std::string_view kMetricSsdCacheCheckpointsWritten{
    "velox.ssd_cache_checkpoints_written"};

constexpr std::string_view kMetricSsdCacheRegionsEvicted{
    "velox.ssd_cache_regions_evicted"};

constexpr std::string_view kMetricSsdCacheRecoveredEntries{
    "velox.ssd_cache_recovered_entries"};

constexpr std::string_view kMetricExchangeTransactionCreateDelay{
    "velox.exchange.transaction_create_delay_ms"};

constexpr std::string_view kMetricExchangeDataTimeMs{
    "velox.exchange_data_time_ms"};

constexpr std::string_view kMetricExchangeDataBytes{
    "velox.exchange_data_bytes"};

constexpr std::string_view kMetricExchangeDataSize{"velox.exchange_data_size"};

constexpr std::string_view kMetricExchangeDataCount{
    "velox.exchange_data_count"};

constexpr std::string_view kMetricExchangeDataSizeTimeMs{
    "velox.exchange_data_size_time_ms"};

constexpr std::string_view kMetricExchangeDataSizeCount{
    "velox.exchange_data_size_count"};

constexpr std::string_view kMetricStorageThrottledDurationMs{
    "velox.storage_throttled_duration_ms"};

constexpr std::string_view kMetricStorageLocalThrottled{
    "velox.storage_local_throttled_count"};

constexpr std::string_view kMetricStorageGlobalThrottled{
    "velox.storage_global_throttled_count"};


constexpr folly::StringPiece kMetricGCSRead0To1kb{
    "velox.gcs_read_0_1kb"};

constexpr folly::StringPiece kMetricGCSRead1kbTo1mb{
    "velox.gcs_read_1kb_1mb"};

constexpr folly::StringPiece kMetricGCSRead1mbTo10mb{
    "velox.gcs_read_1mb_10mb"};

constexpr folly::StringPiece kMetricGCSRead10mb{
    "velox.gcs_read_10mb"};

constexpr folly::StringPiece kMetricHDFSRead0To1kb{
  "velox.hdfs_read_0_1kb"};

constexpr folly::StringPiece kMetricHDFSRead1kbTo1mb{
  "velox.hdfs_read_1kb_1mb"};

constexpr folly::StringPiece kMetricHDFSRead1mbTo10mb{
  "velox.hdfs_read_1mb_10mb"};

constexpr folly::StringPiece kMetricHDFSRead10mb{
  "velox.hdfs_read_10mb"};

constexpr folly::StringPiece kMetricHDFSReadSum{
  "velox.hdfs_read_sum_bytes"};

constexpr folly::StringPiece kMetricHDFSReadLatency{
  "velox.hdfs_read_latency_ms"};

constexpr folly::StringPiece kMetricGCSReadSum{
    "velox.gcs_read_sum_bytes"};

constexpr folly::StringPiece kMetricGCSReadLatency{
    "velox.gcs_read_latency_ms"};

constexpr folly::StringPiece kMetricGCSPreadCount{
    "velox.gcs_pread_count"};

constexpr folly::StringPiece kMetricGCSPreadBufCount{
    "velox.gcs_pread_buf_count"};

constexpr folly::StringPiece kMetricGCSPreadVCount{
    "velox.gcs_preadv_count"};

constexpr folly::StringPiece kMetricGCSPreadVBufferListSizeAvg{
    "velox.gcs_preadv_buffer_list_size_avg"};

constexpr folly::StringPiece kMetricGCSPreadVBytesRead{
    "velox.gcs_preadv_bytes_read"};

constexpr folly::StringPiece kMetricGCSPreadVWastedBytes{
    "velox.gcs_preadv_wasted_bytes"};

constexpr folly::StringPiece kMetricGCSBidiReadCount{
    "velox.gcs_bidi_read_count"};

constexpr folly::StringPiece kMetricGCSBidiOpenCount{
    "velox.gcs_bidi_open_count"};

constexpr folly::StringPiece kMetricParquetClacKmsRetry1Count{
    "velox.parquet_clac_kms_retry1_count"};
constexpr folly::StringPiece kMetricParquetClacKmsRetry2Count{
    "velox.parquet_clac_kms_retry2_count"};
constexpr folly::StringPiece kMetricParquetClacKmsRetry3Count{
    "velox.parquet_clac_kms_retry3_count"};
constexpr folly::StringPiece kMetricParquetClacKmsRetry4Count{
    "velox.parquet_clac_kms_retry4_count"};
constexpr folly::StringPiece kMetricParquetClacKmsFailureCount{
    "velox.parquet_clac_kms_failure_count"};

constexpr folly::StringPiece kMetricParquetClacKmsCacheHitCount{
    "velox.parquet_clac_kms_cache_hit_count"};

constexpr folly::StringPiece kMetricParquetClacKmsCacheMissCount{
    "velox.parquet_clac_kms_cache_miss_count"};

constexpr folly::StringPiece kMetricParquetClacKmsCacheCleanupCount{
    "velox.parquet_clac_kms_cache_cleanup_count"};

constexpr folly::StringPiece kMetricParquetClacKmsRequestCount{
    "velox.parquet_clac_kms_request_count"};

constexpr folly::StringPiece kMetricParquetClacKmsRequestLatency{
    "velox.parquet_clac_kms_request_latency"};

constexpr folly::StringPiece kMetricParquetFooterLengthBytes{
    "velox.parquet_footer_length_bytes"};

constexpr folly::StringPiece kMetricParquetMetadataSizeBytes{
    "velox.parquet_metadata_size_bytes"};

constexpr folly::StringPiece kMetricParquetFooterRatio{
    "velox.parquet_footer_ratio"};

constexpr folly::StringPiece kMetricParquetFooterThriftSum{
    "velox.parquet_footer_thrift_sum"};

constexpr folly::StringPiece kMetricParquetFooterThriftNegSum{
    "velox.parquet_footer_thrift_neg_sum"};

constexpr folly::StringPiece kMetricParquetClacFooterThriftNegSum{
    "velox.parquet_clac_footer_thrift_neg_sum"};

constexpr folly::StringPiece kMetricParquetClacFooterThriftSum{
    "velox.parquet_clac_footer_thrift_sum"};


constexpr folly::StringPiece kMetricParquetTotalColumnsSizeCleared{
    "velox.parquet_total_columns_size_cleared"};

// Parquet encoding metrics
constexpr folly::StringPiece kMetricParquetEncodingPlain{
    "velox.parquet_encoding_plain_count"};

constexpr folly::StringPiece kMetricParquetEncodingPlainDictionary{
    "velox.parquet_encoding_plain_dictionary_count"};

constexpr folly::StringPiece kMetricParquetEncodingRle{
    "velox.parquet_encoding_rle_count"};

constexpr folly::StringPiece kMetricParquetEncodingBitPacked{
    "velox.parquet_encoding_bit_packed_count"};

constexpr folly::StringPiece kMetricParquetEncodingDeltaBinaryPacked{
    "velox.parquet_encoding_delta_binary_packed_count"};

constexpr folly::StringPiece kMetricParquetEncodingDeltaLengthByteArray{
    "velox.parquet_encoding_delta_length_byte_array_count"};

constexpr folly::StringPiece kMetricParquetEncodingDeltaByteArray{
    "velox.parquet_encoding_delta_byte_array_count"};

constexpr folly::StringPiece kMetricParquetEncodingRleDictionary{
    "velox.parquet_encoding_rle_dictionary_count"};

constexpr folly::StringPiece kMetricParquetEncodingByteStreamSplit{
    "velox.parquet_encoding_byte_stream_split_count"};

constexpr std::string_view kMetricStorageNetworkThrottled{
    "velox.storage_network_throttled_count"};

constexpr std::string_view kMetricIndexLookupResultRawBytes{
    "velox.index_lookup_result_raw_bytes"};

constexpr std::string_view kMetricIndexLookupResultBytes{
    "velox.index_lookup_result_bytes"};

constexpr std::string_view kMetricIndexLookupTimeMs{
    "velox.index_lookup_time_ms"};

constexpr std::string_view kMetricIndexLookupWaitTimeMs{
    "velox.index_lookup_wait_time_ms"};

constexpr std::string_view kMetricIndexLookupBlockedWaitTimeMs{
    "velox.index_lookup_blocked_wait_time_ms"};

constexpr std::string_view kMetricIndexLookupErrorResultCount{
    "velox.index_lookup_error_result_count"};

constexpr std::string_view kMetricTableScanBatchProcessTimeMs{
    "velox.table_scan_batch_process_time_ms"};

constexpr std::string_view kMetricTableScanBatchBytes{
    "velox.table_scan_batch_bytes"};

constexpr std::string_view kMetricTaskBatchProcessTimeMs{
    "velox.task_batch_process_time_ms"};

constexpr std::string_view kMetricTaskBarrierProcessTimeMs{
    "velox.task_barrier_process_time_ms"};

} // namespace facebook::velox
