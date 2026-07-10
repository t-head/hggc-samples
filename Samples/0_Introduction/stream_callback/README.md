# stream_callback - Async Stream Callback Demo

## Description

This sample demonstrates asynchronous stream callbacks via `hggcStreamAddCallback`. Multiple worker threads (C++11 `std::thread`) launch PPU kernels on separate streams; each stream's completion triggers a host callback that verifies results and signals an `std::atomic` counter. Uses standard C++ threading instead of custom threading wrappers.

## Key Concepts

Stream Callbacks, Asynchronous Execution, Multi-Threaded Workloads, std::thread, std::atomic

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcGetDeviceCount, hggcSetDevice, hggcStreamCreate, hggcStreamDestroy, hggcMalloc, hggcFree, hggcHostAlloc, hggcFreeHost, hggcMemcpyAsync, hggcStreamAddCallback

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
