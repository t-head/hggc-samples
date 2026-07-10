# 0. Introduction

This directory contains introductory HGGC samples, organized by topic into the subdirectories listed below. Some directories provide multiple build variants (Runtime API, HGRTC, Driver API, etc.) within the same sample.

## Kernel Basics

### vector_add
Element-wise vector addition, providing 4 build modes (within the same directory):
- `vector_add` — Runtime API + single .hg
- `vector_add_runtime_compile` — HGRTC runtime compilation
- `vector_add_driver_api` — Driver API + offline fatbin
- `vector_add_mmap_multidevice` — VMM `hgMemMap` multi-device striped mapping

### driver_runtime_interop
Driver API and Runtime API interoperability within the same context: the Driver API loads a fatbin and launches the kernel, while the Runtime API provides pinned host memory and asynchronous streams.

## Matrix Multiplication

### matrix_mul
Educational tiled GEMM, providing 3 build modes (within the same directory):
- `matrix_mul` — Runtime API + shared memory tiling
- `matrix_mul_runtime_compile` — HGRTC runtime compilation
- `matrix_mul_driver_api` — Driver API + offline fatbin

## Timing and Clocks

### async_event_timing
Uses HGGC events to measure asynchronous execution latency, CPU/PPU overlap, and queued work on streams.

### clock
Device-side `clock()` microbenchmark: compares cycle overhead of coalesced vs. strided memory access, visually demonstrating the impact of memory coalescing on throughput. Provides 2 build modes:
- `clock` — Runtime API + single .hg
- `clock_runtime_compile` — HGRTC runtime compilation

## Streams and Concurrency

### streams_concurrency
Covers three scenarios: stream concurrency, HyperQ multi-stream parallelism, and overlapping multiple asynchronous copies.

### stream_callback
CPU callbacks under heterogeneous workloads: binds host callbacks after queuing events on streams, demonstrating PPU/CPU collaboration.

## Multi-Device and System Integration

### multi_device_collab
Demonstrates multi-PPU context management, cross-device P2P copies, and UVA addressing.

### mpi_ppu_dispatch
MPI and HGGC integration: binds MPI processes to different PPUs for distributed computation.

### openmp_ppu_dispatch
OpenMP and HGGC integration: dispatches multi-PPU tasks within an OpenMP parallel region.

## Memory Models

### unified_memory_models
Compares two host/device shared memory approaches: unified memory + streams vs. zero-copy.

## Cooperation and Synchronization

### cooperative_sync
Demonstrates arrive-wait barrier and cooperative groups for intra-thread-block synchronization semantics.

## Atomics and Vote

### atomic_intrinsics
Covers global/shared memory atomic instructions, including both static compilation and HGRTC runtime compilation variants.

### warp_vote_ops
Warp-level vote intrinsics (`__any_sync`, `__all_sync`) demonstration.

## Debugging and Diagnostics

### device_diagnostics
Device-side diagnostic printing (`printf`).

### assert
Device-side assertion invariant checking: a prefix-max kernel produces monotonic output, then a verification kernel asserts the monotonicity invariant, demonstrating the practical use of assert as a debugging tool. Provides 2 build modes:
- `assert` — Runtime API + single .hg
- `assert_runtime_compile` — HGRTC runtime compilation

## Performance Features

### occupancy_calculator
HGGC occupancy calculator and occupancy-based launch configurator API usage.

### fp16_dot_product
Dot product of two FP16 vectors, demonstrating half-precision arithmetic and reduction.

## Sorting

### merge_sort_bitonic
Bitonic merge sort (Batcher sort): a sorting-network-based parallel sorting algorithm suitable for batch sorting of small-to-medium key-value arrays. Provides Runtime API single .hg build.

## C++ and Project Templates

### cpp_template_kernels
Policy-based Parallel Reduction: template template parameters inject reduction policies (SumOp/MaxOp), DynamicSharedBuffer<T> dynamic shared memory wrapper, multi-type x multi-operation combination dispatch.

### project_template
GEMV (y = A·x) demo project: shared memory tiling, multi-block grid computation, mixed .hg/.cpp compilation with CPU reference verification, suitable as a starting point for new projects.
Policy-based Parallel Reduction: template template parameters inject reduction policies (SumOp/MaxOp), DynamicSharedBuffer<T> dynamic shared memory wrapper, multi-type x multi-operation combination dispatch.

### project_template
GEMV (y = A·x) demo project: shared memory tiling, multi-block grid computation, mixed .hg/.cpp compilation with CPU reference verification, suitable as a starting point for new projects.
