# matrix_mul - Tiled GEMM with Bias

## Description

This sample demonstrates tiled GEMM with bias (`C = alpha * A * B + beta * C`) on PPU, implemented through three API paths:
- **Runtime API** (`matrix_mul.hg`): Single-file kernel + host with event-based timing
- **Runtime Compilation** (`runtime_compile.cpp`): HGRTC JIT compilation of kernel source
- **Driver API** (`driver_api.cpp`): Offline fatbin loading with occupancy-based tile selection

All variants use random input with fixed seeds and verify against a CPU reference triple-loop computation with relative error tolerance.

## Key Concepts

Tiled GEMM, Shared Memory, Bias Term, CPU Reference Verification, Runtime API, Driver API, Runtime Compilation

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcEventCreate, hggcEventRecord, hggcEventSynchronize, hggcEventElapsedTime, hggcEventDestroy, hggcDeviceSynchronize

### HGGC Driver API
hgInit, hgCtxCreate, hgCtxDestroy, hgModuleLoadData, hgModuleGetFunction, hgMemAlloc, hgMemFree, hgMemcpyHtoD, hgMemcpyDtoH, hgLaunchKernel, hgOccupancyMaxPotentialBlockSize

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
