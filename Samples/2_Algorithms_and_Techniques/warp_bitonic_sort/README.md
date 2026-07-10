# warp_bitonic_sort - Warp Shuffle Bitonic Sort

## Description

This sample implements a **pure-register** bitonic sort using `__shfl_xor_sync` for 32-element intra-warp sorting without shared memory. It then merges sorted sequences from each warp into block-level sorted output through a bitonic merge network in shared memory.

Two-phase design:
1. **Warp-level sort** (Phase 1): each lane holds one element; 5 rounds of shuffle-XOR compare-and-swap complete a 32-element bitonic sort, entirely in registers.
2. **Block-level merge** (Phase 2): merges 8 sorted warps (256 elements total) into one sorted block via a shared memory bitonic merge network.

Differences from traditional implementation:
- Traditional approach: all data loaded into shared memory -> all compare-and-swap done in shared memory.
- This approach: intra-warp sorting with zero shared memory overhead -> shared memory used only for cross-warp merging.

## Key Concepts

Bitonic Sort, Warp Shuffle (__shfl_xor_sync), Sorting Networks, Register-Level Optimization.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
