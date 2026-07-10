# vector_add - Weighted Vector Linear Combination

## Description

This sample demonstrates weighted vector linear combination (`out[i] = alpha * x[i] + beta * y[i] + gamma`) on PPU, implemented through four API paths:
- **Runtime API** (`vector_add.hg`): Single-file kernel + host
- **Runtime Compilation** (`runtime_compile.cpp`): HGRTC JIT compilation of kernel source
- **Driver API** (`driver_api.cpp`): Offline fatbin loading via Driver API
- **Multi-Device VMM** (`mmap_multidevice.cpp`): Striped virtual address mapping across peer-capable devices

All variants use deterministic sinusoidal input and verify with relative error tolerance.

## Key Concepts

Vector Operations, Runtime API, Driver API, Runtime Compilation, Virtual Memory Management, Multi-Device Mapping

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcGetLastError

### HGGC Driver API
hgInit, hgCtxCreate, hgCtxDestroy, hgModuleLoadData, hgModuleGetFunction, hgMemAlloc, hgMemFree, hgMemcpyHtoD, hgMemcpyDtoH, hgLaunchKernel

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant); a Zhenwu PPU with multi-device P2P support (required for the multi-device VMM variant, at least 2 interconnectable devices)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
