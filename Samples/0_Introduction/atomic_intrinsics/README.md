# atomic_intrinsics - Simple Atomic Operations

## Description

This sample demonstrates 32-bit atomic operations and `unsigned long long` Add/And/Or/Xor/Min/Max/Exch/CAS in both static and HGRTC targets, covering the high 32 bits and global/shared memory.

Concurrent tests check deterministic final values. `Exch` only checks that the result belongs to the set of values written by valid threads; it does not assume an execution order. Single-thread sequential tests check each returned old value, the final value, and successful/failed CAS operations. Both the device implementation and CPU reference use `1u << (tid % 32)` for 32-bit OR to avoid undefined shifts.

- **Runtime API implementation** (`static.hg` + `atomic_kernel.hgh`): Based on the HGGC Runtime API, demonstrates the simplest use of global memory atomic instructions.
- **HGRTC runtime compilation implementation** (`runtime_compile.cpp` + `atomic_kernel_rtc.hg`): Uses HGRTC to just-in-time compile kernel source at program startup, loads and executes via the Driver API, demonstrating equivalent behavior of the same set of atomic intrinsics on the runtime compilation path.

## Key Concepts

Atomic Intrinsics, ULL, High 32 Bits, Global/Shared Memory, Concurrent Final Values, Sequential Old Values, Runtime Compilation.

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

hgMemcpyDtoH, hgLaunchKernel, hgMemcpyHtoD, hgCtxSynchronize, hgMemAlloc, hgMemFree, hgModuleGetFunction, hgModuleLoadData, hgModuleUnload

### HGRTC API

hgrtcCreateProgram, hgrtcCompileProgram, hgrtcGetProgramLogSize, hgrtcGetProgramLog, hgrtcGetHGBINSize, hgrtcGetHGBIN

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform. Requires SAIL 2.2.0 or later.
