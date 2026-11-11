# clock - Device-Side Clock Microbenchmark

## Description

This sample uses the device-side `clock()` intrinsic to measure per-block cycle counts for two illustrative memory access patterns, **coalesced** and **strided**. The coalesced loop repeatedly reads each thread's element; the strided loop wraps within a block-sized region and includes repeated addresses. These patterns are not an independent bandwidth benchmark, and their relative speed is not a correctness invariant.

After each kernel, the host checks every accumulated output against an independent high-precision CPU reference using `abs_error <= 1e-4 + 1e-5 * abs(reference)` and rejects non-finite or unwritten outputs. All block timers must report a valid nonzero elapsed interval. The cycle ratio is printed as `INFO`; `Result: PASS/FAIL` and the exit code depend on data, timing validity, and API success, not which pattern is faster.

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
