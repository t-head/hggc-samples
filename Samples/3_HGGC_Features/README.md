# 3. HGGC Features

### bf16_tensor_cell_gemm
Implements BF16 GEMM using architecture-specific TIX `ppu.tc01/tc02.mma.sync.aligned.m16n16k16`, demonstrating the cp.async + ldmatrix + mma double-buffer pipeline with visible register-level fragment layout.

### aiu_async_copy
Cross-architecture AIU asynchronous tensor copy: uses the ppu001 3D or ppu0015 2D tile descriptor for 128B swizzle bulk transfer, combined with `ppu.tc01/tc02.ldmatrix.swzl` decoding and host-side data integrity verification.

### aiu_gemm
AIU .swzl mode FP16 GEMM: uses architecture-specific AIU 128B swizzle transfer for A/B matrix tiles (16x64), `ldmatrix.swzl` decoding, `.trans` B loading, and `ppu.tc01/tc02.mma.m16n16k16.row.col` accumulation.

### memcpy_batch_async
Submits six independent D2D copies on a non-default stream of the selected device, groups them with two attribute-start indices, initializes destination sentinels with hggcMemsetAsync on the same stream, and verifies every byte after synchronization. Function-pointer overloads match the SAIL 2.2.1 rt2/rt3 signatures.

### array_roundtrip
Uses single-plane int32 arrays to verify 1D/2D/3D allocation, queries, full round trips, offset updates, and untouched regions. Distinguishes element extents from byte pitches/offsets and checks host padding.

### binary_partition_cg
Warp-level conditional branching: uses `cg::binary_partition()` to dynamically split a warp into positive/negative subgroups, each performing independent reduction for max/min, demonstrating runtime dynamic subgroup capabilities of cooperative groups.

### global_to_shmem_async_copy
Async copy synchronization strategy comparison: naive / cp.async+wait_group / cp.async+awbar (arrive-wait barrier) three approaches, showcasing the unified synchronization capability of awbar.

### graph_conditional_nodes
Graph conditional loop iterative solver: implements Jacobi iteration until convergence using conditional while nodes, with device-side `hggcGraphSetConditional` controlling loop termination.

### graph_memory_footprint
Graph memory node buffer reuse: automatic buffer reuse in a 3-step compute pipeline, demonstrating peak memory reduction from 16KB to 8KB.

### graph_memory_nodes
Covers ordinary launch and explicit upload before the first launch for both the Graph API and Stream Capture, giving four paths. The graph is alloc -> scale -> bias -> free; checks compare the CPU reference and all paths, and memory nodes use the selected device.

### imma_tensor_cell_gemm
Implements INT8 GEMM using architecture-specific TIX `ppu.tc01/tc02.mma.sync.aligned.m16n16k32`, with exact CPU-reference verification.

### power_iteration_graph
Power iteration for largest eigenvalue: demonstrates `hggcGraphExecKernelNodeSetParams` for dynamically updating kernel parameters (swapping src/dst pointers each step) without rebuilding the graph.

### new_delete
Device-side linked list: constructs a polymorphic singly linked list on the device heap using `new`/`delete`, demonstrating virtual function dispatch, virtual destructors, and pointer traversal.

### simple_hggc_graphs
Complete Graph node type showcase: Event Record/Wait nodes, Child Graph nodes, Host Callback, Graph Clone, and node traversal, demonstrating full Graph capabilities in a coherent pipeline.

### tf32_tensor_cell_gemm
Implements TF32 GEMM using architecture-specific TIX `ppu.tc01/tc02.mma.sync.aligned.m16n16k8`, compared with the bf16 version across precision and shape configurations.

### warp_aggregated_atomics_cg
CG advanced API comprehensive showcase: chains `labeled_partition`, `invoke_one`, `cg::reduce`, `cg::inclusive_scan` four core interfaces through a weighted histogram scenario, demonstrating the advantage of aggregated atomics compared to the naive version.
