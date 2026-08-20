# warp_aggregated_atomics_cg - CG Advanced API: Weighted Histogram

## Description

This sample demonstrates comprehensive usage of **Cooperative Groups advanced APIs**, chaining five core CG interfaces through a weighted histogram scenario:

1. **`cg::coalesced_threads()`** — Obtains the currently active thread group (handles branch divergence)
2. **`cg::labeled_partition(group, label)`** — Dynamic grouping by label, threads with the same label form a subgroup
3. **`cg::reduce(group, val, op)`** — Intra-group reduction (computes total weight per group)
4. **`cg::inclusive_scan(group, val, op)`** — Intra-group prefix sum (computes each thread's offset within the group)
5. **`group.invoke_one(lambda)`** — Only the leader executes the lambda, return value is broadcast to all group members

Scenario: each thread holds a `(bucket_label, weight)` pair, need to compute count and total weight per bucket.
The CG version greatly reduces atomic contention through aggregated atomic operations (only one atomicAdd inside invoke_one),
compared against the naive version (each thread independently atomicAdd) to demonstrate the performance advantage.

## Key Concepts

Cooperative Groups, labeled_partition, invoke_one, inclusive_scan, Aggregated Atomic Operations.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### Cooperative Groups API
cg::coalesced_threads, cg::labeled_partition, cg::reduce, cg::inclusive_scan, group.invoke_one

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
