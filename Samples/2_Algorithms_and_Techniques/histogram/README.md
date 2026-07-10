# histogram - Progressively Optimized 256-Bin Histogram

## Description

This sample implements a 256-bin byte histogram through **three progressively optimized strategies**, demonstrating the impact of atomic operations and memory privatization on throughput:

1. **Global atomicAdd** (baseline) — Each byte directly atomically accumulated to global memory, highest contention
2. **Shared memory privatization** — Each block maintains a private histogram in shared memory, merged to global at the end
3. **Warp-level privatization** — Each warp owns an independent sub-histogram, eliminating inter-warp contention, highest throughput

All three strategies run on the same dataset for comparison, outputting respective throughput (GB/s) and verifying correctness against a CPU reference implementation.

## Key Concepts

Atomic Operations, Privatization, Contention and Concurrency, Shared Memory Optimization.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize, atomicAdd

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
