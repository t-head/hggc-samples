# stream_ordered_allocation - Memory Pool Lifecycle Management

## Description

This sample provides an in-depth demonstration of the complete lifecycle and mechanisms of HGGC **Stream-Ordered Memory Pools**, revealing the value of pool reuse through performance comparison of three allocation strategies:

1. **hggcMalloc (synchronous)** — Baseline: synchronous allocation/deallocation per iteration, highest overhead
2. **hggcMallocAsync (threshold=0)** — Asynchronous allocation, but memory is immediately returned to the OS after release, no pool reuse
3. **hggcMallocAsync (threshold=MAX)** — Asynchronous allocation + high release threshold, memory stays in the pool for reuse by subsequent allocations

Pool management mechanisms demonstrated:
- `hggcDeviceGetDefaultMemPool` — Obtains the default memory pool
- `hggcMemPoolSetAttribute(hggcMemPoolAttrReleaseThreshold)` — Controls the release strategy
- `hggcMemPoolTrimTo` — Manually trims the pool, returning unused memory
- **Address reuse observation** — In pool reuse mode, consecutive iterations allocate to the same address

Outputs a formatted performance comparison table, visually demonstrating reduced allocation overhead from pool reuse.

## Key Concepts

Stream-Ordered Memory Pool, Async Allocation, Pool Reuse and Trimming, Release Threshold Strategy.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMallocAsync, hggcFreeAsync, hggcDeviceGetDefaultMemPool, hggcMemPoolSetAttribute, hggcMemPoolTrimTo, hggcStreamCreateWithFlags, hggcStreamSynchronize, hggcDeviceGetAttribute, hggcDeviceSynchronize, hggcFree, hggcMalloc, hggcMemcpy, hggcMemcpyAsync, hggcMemset, hggcMemsetAsync, hggcStreamDestroy

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
