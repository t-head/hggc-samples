# spmv - CSR Sparse Matrix-Vector Multiplication

## Description

This sample implements **CSR-format sparse matrix-vector multiplication (y = A * x)**, demonstrating the differences between two PPU parallel strategies on irregular data structures:

1. **Scalar (one thread per row)** — Each thread independently traverses one row's non-zero elements to compute the inner product. Simplest implementation, but suffers from load imbalance when row lengths are uneven.
2. **Vector (one warp per row)** — A warp (32 threads) cooperatively processes one row: each lane reads non-zero elements in a strided manner, with a final warp shuffle reduction for summation. Suitable for long rows and variable-length rows (e.g., power-law graphs).

Key learning points:
- **Indirect addressing**: `col_idx[j]` leads to irregular memory access patterns.
- **Load imbalance**: vastly different numbers of non-zeros across rows.
- **Warp cooperative reduction**: practical application of `__shfl_down_sync` in SpMV.

The test matrix is a synthetic banded sparse matrix (diagonally dominant) with configurable bandwidth.

## Key Concepts

Sparse Matrix, CSR Format, Irregular Parallelism, Load Balancing, Warp Cooperative Reduction.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize, __shfl_down_sync

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
