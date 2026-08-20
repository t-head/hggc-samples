# atomic_intrinsics - Simple Atomic Operations

## Description

This sample demonstrates the use of global memory atomic intrinsics through different code paths:

- **Runtime API implementation** (`static.hg` + `atomic_kernel.hgh`): Based on the HGGC Runtime API, demonstrates the simplest use of global memory atomic instructions.
- **HGRTC runtime compilation implementation** (`runtime_compile.cpp` + `atomic_kernel_rtc.hg`): Uses HGRTC to just-in-time compile kernel source at program startup, loads and executes via the Driver API, demonstrating equivalent behavior of the same set of atomic intrinsics on the runtime compilation path.

## Key Concepts

Atomic Intrinsics, Runtime Compilation.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API

hggcStreamCreateWithFlags, hggcFree, hggcMallocHost, hggcFreeHost, hggcStreamSynchronize, hggcMalloc, hggcMemcpyAsync, hggcStreamDestroy

### HGGC Driver API

hgMemcpyDtoH, hgLaunchKernel, hgMemcpyHtoD, hgCtxSynchronize, hgMemAlloc, hgMemFree, hgModuleGetFunction

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
