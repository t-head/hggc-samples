# binary_partition_cg - Warp-Level Conditional Branching (Binary Partition)

## Description

This sample demonstrates `cg::binary_partition()` for **Warp-level dynamic conditional branching**:

Given a set of floating-point numbers, each warp dynamically splits into two groups at runtime based on value sign:
- **Positive group**: intra-group reduction for maximum (`cg::reduce` + `greater`)
- **Negative group**: intra-group reduction for minimum (`cg::reduce` + `less`)

Each subgroup's leader thread atomically writes the result to global memory, then compared against a CPU reference implementation for verification.

Application scenarios:
- Conditional routing in ML inference (MoE gating)
- Particle classification in physics simulation
- Heterogeneous neighbor processing in graph algorithms

## Key Concepts

Cooperative Groups, Binary Partition, Warp-Level Reduction, Dynamic Subgroups.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize, cg::binary_partition, cg::reduce

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
