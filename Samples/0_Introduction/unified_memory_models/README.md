# unified_memory_models - Unified Memory Programming Model Comparison

## Description

This sample compares two unified memory programming models on HGGC: using Zero MemCopy where the kernel directly reads and writes pinned system memory, and using OpenMP and Streams with Unified Memory on a single PPU. Both paths share device discovery and command-line parsing, switchable via `--mode=zero_copy|managed_streams` subcommand; by default, if the device has managed memory capability, `managed_streams` is selected automatically, otherwise falls back to `zero_copy`.

## Key Concepts

Performance Strategies, Pinned System Paged Memory, Vector Addition, HGGC System Integration, OpenMP, ACBLAS, Multithreading, Unified Memory, HGGC Streams and Events.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API

hggcHostAlloc, hggcSetDeviceFlags, hggcSetDevice, hggcGetDeviceCount, hggcHostGetDevicePointer, hggcDeviceSynchronize, hggcFreeHost, hggcGetDeviceProperties, hggcStreamDestroy, hggcFree, hggcMallocManaged, hggcStreamAttachMemAsync, hggcStreamSynchronize, hggcStreamCreate

## Dependencies for Build/Run
OpenMP, UVM, ACBLAS (required for the `--mode=managed_streams` path)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.

**Mode Description**

- **`--mode=zero_copy`**: Uses Zero MemCopy, where the kernel directly reads and writes pinned system memory.
- **`--mode=managed_streams`**: Uses OpenMP and Streams with Unified Memory on a single PPU.
