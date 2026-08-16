# assert - Device-Side Assertion Invariant Checking

## Description

This sample demonstrates how to use device-side `assert()` as an invariant checking tool for kernel output:

1. A **prefix-max kernel** produces monotonically non-decreasing output
2. A **verification kernel** asserts that adjacent elements satisfy `array[i] <= array[i+1]`
3. The host runs two scenarios: correct output (assert does not trigger) and intentionally corrupted output (assert triggers, host detects error state)

This demonstrates the practical use of `assert()` as a debugging tool in real development, rather than a simple "thread ID out-of-bounds" demo.

Two build targets:

- **Runtime API version** (`assert.hg`): kernel and host code statically compiled.
- **HGRTC runtime compilation version** (`assert_runtime_compile.cpp` + `assert_kernel.hg`): libHGRTC just-in-time compilation + Driver API load and execution.

## Key Concepts

Device Assert, Invariant Checking, Runtime Compilation, Error Propagation.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize, hggcGetErrorString, hggcGetLastError, hggcDeviceReset

### HGGC Driver API
hgMemAlloc, hgMemFree, hgMemcpyHtoD, hgModuleGetFunction, hgLaunchKernel, hgCtxSynchronize

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
