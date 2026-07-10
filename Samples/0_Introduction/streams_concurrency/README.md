# streams_concurrency - Stream-Level Concurrency and Compute-Copy Overlap

## Description

This sample demonstrates three typical use cases of HGGC stream-level concurrency: overlapping kernel execution with host-to-PPU memory copies using HGGC streams (and leveraging HGGC pinned host memory), concurrent execution of multiple kernels using HGGC streams, and overlapping kernel execution with device data copies using HGGC streams. All three paths share device discovery and command-line parsing, switchable via `--mode=streams|hyperq|multicopy` subcommand; defaults to the `streams` path.

## Key Concepts

Asynchronous Data Transfers, HGGC Streams and Events, HGGC System Integration, Performance Strategies, Overlap Compute and Copy, PPU Performance.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API

hggcMemcpy, hggcSetDeviceFlags, hggcSetDevice, hggcEventDestroy, hggcStreamCreate, hggcMallocHost, hggcEventCreateWithFlags, hggcFreeHost, hggcMemcpyAsync, hggcGetDeviceCount, hggcStreamDestroy, hggcMemset, hggcEventElapsedTime, hggcHostAlloc, hggcFree, hggcEventSynchronize, hggcEventRecord, hggcMalloc, hggcGetDeviceProperties, hggcEventCreate, hggcDeviceSynchronize, hggcDeviceGetAttribute

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.

**Mode Description**

- **`--mode=streams`** (default): Uses HGGC streams to overlap kernel execution with host-to-PPU memory copies, leveraging HGGC pinned generic host memory.
- **`--mode=hyperq`**: Uses HGGC streams to execute multiple kernels concurrently.
- **`--mode=multicopy`**: Uses HGGC streams to overlap kernel execution with device data copies.
