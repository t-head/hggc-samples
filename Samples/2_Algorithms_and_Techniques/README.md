# 2. Parallel Algorithms and Techniques

### convolution_separable
Separable Gaussian blur: row/column separated convolution + shared memory halo loading + constant memory broadcast.

### histogram
Progressively optimized 256-bin histogram: three strategies compared — global atomic, shared memory privatization, and warp-level privatization.

### inline_tix
TIX inline assembly instruction showcase: four practical TIX instruction examples — FMA, popcount, wide multiplication, and bit-field extraction.

### radix_sort_thrust
Data-parallel pipeline using the Thrust library: sort -> unique -> transform -> reduce -> scan.

### stream_compact
Stream compaction (conditional filtering): extracts elements satisfying a predicate from an array, with three progressive strategies (naive atomic / ballot+popc / two-pass scan).

### scan
Multi-block exclusive prefix sum (Blelloch algorithm): work-efficient parallel scan with up-sweep/down-sweep, supporting non-power-of-two input sizes.

### spmv
CSR sparse matrix-vector multiplication: scalar (one thread per row) and vector (one warp per row) strategies, demonstrating irregular parallelism and load balancing.

### warp_bitonic_sort
Warp Shuffle bitonic sort: pure-register `__shfl_xor_sync` implementation for 32-element intra-warp sorting + shared memory merge into block-level sorted sequence.

### warp_redux
Hardware warp-level reduction (`ppu.redux.sync`): single-instruction warp sum/min/max/xor, cross-verified against `__shfl_down_sync` loop.

### stream_ordered_allocation
Memory pool lifecycle management: three strategies compared — synchronous allocation / asynchronous allocation / pool reuse, demonstrating release threshold, address reuse, and pool trimming mechanisms.

### ctx_management
Multi-context lifecycle management: creating multiple HGcontexts, Push/Pop migration between threads, serialized access to shared contexts, and HGRTC runtime compilation.
