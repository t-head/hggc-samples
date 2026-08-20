# aiu_throughput - AIU vs cp.async Copy Throughput

## Description

This sample compares two PPU async data copy mechanisms for moving bf16 tensors of increasing size (2KB to 32KB) from global memory through shared memory and back:
- **cp.async**: Per-thread 4-byte async copy with commit/wait synchronization
- **AIU bulk**: Per-warp tensor copy with swizzle + ldmatrix.swzl decode

Tests five tensor sizes (16x1x64 through 256x1x64 bf16) to show throughput scaling. Both methods verify data integrity (output == input) and report GB/s with speedup ratio.

## Key Concepts

AIU Bulk Copy, cp.async, Tensor Shared Memory, ldmatrix.swzl, Throughput Benchmarking, Scaling Analysis

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001 only

This sample uses AIU (`ppu.cp.async.aiu.*`) and Tensor Cell (`ppu.tc01.*`) instructions that are not supported on ppu0015.

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMemcpy, hggcEventCreate, hggcEventRecord, hggcEventSynchronize, hggcEventElapsedTime, hggcEventDestroy, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
