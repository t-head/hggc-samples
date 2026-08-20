# smem_bank_conflict - Shared Memory Bank Conflict Impact

## Description

This sample measures shared memory read throughput with different access strides to show how bank conflicts degrade performance. PPU shared memory has 32 banks (4 bytes each); when multiple threads in the same warp access the same bank, accesses are serialized.

Tests strides 1 (no conflict) through 32 (maximum conflict), reporting effective throughput (GB/s) and relative efficiency for each pattern.

## Key Concepts

Shared Memory, Bank Conflict, Access Stride, Memory Throughput, Performance Degradation

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMemcpy, hggcEventCreate, hggcEventRecord, hggcEventSynchronize, hggcEventElapsedTime, hggcEventDestroy, hggcDeviceSynchronize
## Prerequisites

Please download and install the T-Head SAIL toolkit for your platform.
