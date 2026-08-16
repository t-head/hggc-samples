# mem_strategy_bench - Memory Allocation Strategy Performance

## Description

This sample compares 4 PPU memory allocation strategies for a simple element-wise scale kernel:
- **Managed Memory**: `hggcMallocManaged` + `hggcMemPrefetchAsync` for device/host migration
- **Zero Copy**: `hggcHostAlloc` (mapped) + `hggcHostGetDevicePointer` for direct host access from device
- **Pinned + Async**: `hggcHostAlloc` (portable) + `hggcMemcpyAsync` for staged transfer
- **Pageable + Sync**: `malloc` + `hggcMemcpy` for baseline comparison

Tests multiple data sizes (256KB to 4MB) to show how each strategy scales with data volume.

## Key Concepts

Managed Memory, Zero Copy, Pinned Memory, Async Memcpy, Memory Allocation Strategy, Performance Benchmarking

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMallocManaged, hggcMemPrefetchAsync, hggcHostAlloc, hggcFreeHost, hggcHostGetDevicePointer, hggcMemcpy, hggcMemcpyAsync, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
