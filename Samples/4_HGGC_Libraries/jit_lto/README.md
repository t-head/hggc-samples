# jit_lto - Mixed LTO IR + HGBIN JIT Linking

## Description

This sample demonstrates runtime JIT linking of two separately compiled modules using mixed input types:
- **Module A (LTO IR)**: A JIT-compiled kernel (`weighted_average`) compiled with `-dlto`, calling an external device function (`blend`)
- **Module B (HGBIN)**: The `blend` device function implementation compiled as a regular HGBIN (without LTO)

Unlike pure-LTO linking demos, this shows a mixed-input scenario where a JIT-compiled kernel links against a pre-compiled library function — a common pattern for plugin architectures and hot-patchable kernels.

Algorithm: weighted average (`out[i] = w*x[i] + (1-w)*y[i]`), verified against CPU reference.

## Key Concepts

HGGC Driver API, Runtime Compilation (HGRTC), JIT Linking (hgJitLink), LTO, Mixed IR+HGBIN Linking

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGRTC API
hgrtcCreateProgram, hgrtcCompileProgram, hgrtcGetLTOIR, hgrtcGetHGBIN, hgrtcGetProgramLog, hgrtcDestroyProgram

### hgJitLink API
hgJitLinkCreate, hgJitLinkAddData (INPUT_LTOIR + INPUT_HGBIN), hgJitLinkComplete, hgJitLinkGetLinkedHgbin, hgJitLinkDestroy, hgJitLinkVersion

### HGGC Driver API
hgInit, hgDeviceGet, hgDeviceGetAttribute, hgCtxCreate, hgCtxDestroy, hgCtxSynchronize, hgModuleLoadData, hgModuleGetFunction, hgModuleUnload, hgMemAlloc, hgMemFree, hgMemcpyHtoD, hgMemcpyDtoH, hgLaunchKernel

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
