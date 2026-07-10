# 5. Performance

### aligned_types
A simple test demonstrating the large access speed gap between aligned and misaligned structures. It measures per-element copy throughput of aligned and misaligned structures on large data blocks.

### hggc_graphs_perf_scaling
This sample analyzes the performance characteristics of the HGGC Graph API across different graph sizes. It focuses on how the API scales with graph size.

### large_kernel_parameter
A simple test demonstrating performance and usability improvements for large kernel parameters introduced in HGGC.

### transpose
This sample demonstrates matrix transpose. It showcases different performance optimizations to achieve high performance.

### unified_memory_perf
This sample demonstrates performance comparison of unified memory (with/without hints) versus other memory types (such as zero-copy buffers, pageable memory, page-locked memory) for synchronous and asynchronous transfers on a single PPU using a matrix multiplication kernel.
