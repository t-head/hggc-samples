# clock - Device-Side Clock Microbenchmark

## Description

This sample uses the device-side `clock()` intrinsic to measure cycle overhead of different memory access patterns, comparing **coalesced** vs. **strided** access to visually demonstrate the impact of memory coalescing on throughput.

Two build targets share the same algorithmic logic, demonstrating different compilation/loading paths:

- **Runtime API version** (`clock.hg`): kernel and host code statically compiled in the same file.
- **HGRTC runtime compilation version** (`clock_runtime_compile.cpp` + `clock_kernel.hg`): kernel source just-in-time compiled by libHGRTC at program startup, loaded and executed via the Driver API.

## Key Concepts

On-Device Clock Measurement, Memory Coalescing, HGRTC Runtime Compilation, Performance Analysis.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

### HGGC Driver API
hgMemAlloc, hgMemFree, hgMemcpyHtoD, hgMemcpyDtoH, hgModuleGetFunction, hgLaunchKernel, hgCtxSynchronize

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
