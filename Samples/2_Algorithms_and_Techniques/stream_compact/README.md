# stream_compact - Stream Compaction (Conditional Filtering)

## Description

This sample demonstrates **Stream Compaction** — extracting elements satisfying a predicate from an input array into a compact output array. Using "extracting primes" as a practical use case, it showcases three progressively optimized strategies:

1. **Naive atomic** — Threads satisfying the condition use `atomicAdd` to obtain output positions and directly scatter. Simplest implementation, but output is unordered.
2. **Ballot + popc** — Warp-level `__ballot_sync` collects predicate results, `__popc` computes intra-warp prefix offsets, block-level uses shared atomic for coordination, finally global atomic obtains block base address. Output is ordered within each block.
3. **Two-pass scan** — First pass marks + block-level exclusive prefix sum computes write offsets; second pass scatters by offset. Output is fully ordered (preserves original relative order).

All three strategies run on the same dataset [0, 65536), individually timed, and verified against a CPU reference implementation.

## Key Concepts

Stream Compaction, Prefix Sum, Warp Ballot, Atomic Operations, Conditional Filtering.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize, atomicAdd, __ballot_sync, __popc, __shfl_sync

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
