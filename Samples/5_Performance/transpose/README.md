# transpose - Native vs AIU Transpose Throughput

## Description

This sample compares three matrix transpose methods for bf16 square matrices:
- **Native**: Shared memory tiling with 16x16 tiles and bank-conflict-free padding
- **AIU+ldmatrix**: AIU bulk copy to TSM + ldmatrix.swzl decode + transposed store (64x64 tiles, 4 warps per tile)
- **AIU pipeline**: Producer-consumer double-buffer with awbarrier sync (64x64 tiles, 1 producer warp + 4 consumer warps, cp.async.awbar.arrive linked AIU copy)

Tests multiple matrix sizes (64x64 to 16384x16384) to show throughput scaling. All methods verify correctness (output == input transposed).

## Key Concepts

Matrix Transpose, Shared Memory Tiling, AIU Bulk Copy, ldmatrix.swzl, awbarrier, Producer-Consumer Pipeline, Double Buffer, Throughput Benchmarking

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMemcpy, hggcEventCreate, hggcEventRecord, hggcEventSynchronize, hggcEventElapsedTime, hggcEventDestroy, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
