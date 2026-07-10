# radix_sort_thrust - Thrust Data-Parallel Algorithm Pipeline

## Description

This sample demonstrates building a complete data processing pipeline using the Thrust library, showcasing the combined use of multiple Thrust primitives:

1. **sort** — Radix sort (Thrust's classic strength)
2. **unique** — Removing consecutive duplicates
3. **transform** — Applying a unary functor (squaring each element)
4. **reduce** — Global reduction (summation)
5. **inclusive_scan** — Prefix sum

Each step is independently timed, demonstrating Thrust's capability as a composable data-parallel toolkit.

## Key Concepts

Data-Parallel Algorithms, Thrust Library, Algorithm Composition, Pipeline Processing.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcEventCreate, hggcEventRecord, hggcEventSynchronize, hggcEventElapsedTime, hggcEventDestroy

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
