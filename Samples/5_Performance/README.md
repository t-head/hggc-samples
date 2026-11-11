# 5. Performance

### aiu_throughput
Compares cp.async and architecture-specific AIU bulk tensor copies for BF16 tensors from 2KB to 32KB, verifies data integrity, and reports throughput and speedup.

### hggc_graphs_perf_scaling
This sample analyzes the performance characteristics of the HGGC Graph API across different graph sizes. It focuses on how the API scales with graph size.

### mem_strategy_bench
Compares managed memory with prefetch, mapped zero-copy memory, pinned asynchronous transfers, and pageable synchronous transfers for an elementwise scale kernel over multiple data sizes.

### smem_bank_conflict
Measures shared-memory read throughput with strides from 1 to 32 to demonstrate the impact of bank conflicts, reporting effective throughput and relative efficiency.

### transpose
Compares native shared-memory tiling, architecture-specific AIU+ldmatrix, and producer-consumer AIU pipeline transposes for BF16 square matrices, verifying results and reporting throughput.
