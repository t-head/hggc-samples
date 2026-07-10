# 3. HGGC Features

### bf16_tensor_cell_gemm
Implements BF16 GEMM using pure TIX inline assembly (`ppu.tc01.mma.sync.aligned.m16n16k16`), demonstrating the cp.async + ldmatrix + mma double-buffer pipeline with visible register-level fragment layout.

### aiu_async_copy
AIU TIX 1.0 asynchronous tensor copy: uses `ppu.cp.async.aiu.bulk.tensor.shared.global.3d` for 128B swizzle bulk transfer from global to shared memory, combined with `ppu.tc01.ldmatrix.swzl` to decode addresses and load into registers, with host-side sorting verification for data integrity.

### aiu_gemm
AIU .swzl mode BF16 GEMM: uses AIU 128B swizzle bulk transfer for A/B matrix tiles (16x64), `ldmatrix.swzl` for address decoding, B matrix loaded with `.trans` transpose as col-major fragment, `ppu.tc01.mma.m16n16k16.row.col` accumulation, relative error < 5%.

### binary_partition_cg
Warp-level conditional branching: uses `cg::binary_partition()` to dynamically split a warp into positive/negative subgroups, each performing independent reduction for max/min, demonstrating runtime dynamic subgroup capabilities of cooperative groups.

### global_to_shmem_async_copy
Async copy synchronization strategy comparison: naive / cp.async+wait_group / cp.async+awbar (arrive-wait barrier) three approaches, showcasing the unified synchronization capability of awbar.

### graph_conditional_nodes
Graph conditional loop iterative solver: implements Jacobi iteration until convergence using conditional while nodes, with device-side `hggcGraphSetConditional` controlling loop termination.

### graph_memory_footprint
Graph memory node buffer reuse: automatic buffer reuse in a 3-step compute pipeline, demonstrating peak memory reduction from 16KB to 8KB.

### graph_memory_nodes
Graph construction method comparison: the same computation built using both explicit Graph API and Stream Capture, verifying identical results and demonstrating the pros and cons of both approaches.

### imma_tensor_cell_gemm
Implements INT8 GEMM using pure TIX inline assembly (`ppu.tc01.mma.sync.aligned.m16n16k32`), integer MMA with no rounding error, exact match with CPU reference.

### power_iteration_graph
Power iteration for largest eigenvalue: demonstrates `hggcGraphExecKernelNodeSetParams` for dynamically updating kernel parameters (swapping src/dst pointers each step) without rebuilding the graph.

### new_delete
Device-side linked list: constructs a polymorphic singly linked list on the device heap using `new`/`delete`, demonstrating virtual function dispatch, virtual destructors, and pointer traversal.

### simple_hggc_graphs
Complete Graph node type showcase: Event Record/Wait nodes, Child Graph nodes, Host Callback, Graph Clone, and node traversal, demonstrating full Graph capabilities in a coherent pipeline.

### tf32_tensor_cell_gemm
Implements TF32 GEMM using pure TIX inline assembly (`ppu.tc01.mma.sync.aligned.m16n16k8`), compared with the bf16 version to show Tensor Cell usage differences across precision/shape configurations.

### warp_aggregated_atomics_cg
CG advanced API comprehensive showcase: chains `labeled_partition`, `invoke_one`, `cg::reduce`, `cg::inclusive_scan` four core interfaces through a weighted histogram scenario, demonstrating the advantage of aggregated atomics compared to the naive version.
