# inline_tix - TIX Inline Assembly Instruction Showcase

## Description

This sample demonstrates embedding multiple categories of TIX instructions via `asm()` statements in HGGC kernels, each paired with a practical use case:

1. **ppu.fma.rtte.f32** — Fused multiply-add for Horner's method polynomial evaluation (single-rounding precision advantage)
2. **ppu.popc.b32** — Population count, computing Hamming weight
3. **ppu.mul.wide.s32** — Wide multiplication, 32×32 → 64-bit full-precision product
4. **ppu.bfe.u32** — Bit-field extraction, extracting a specified-length bit slice from an arbitrary position

Each test case is verified against a CPU reference implementation.

Provides 2 build modes:
- **Runtime API version** (`inline_tix.hg`): statically compiled, 4 test kernels and host in the same file
- **HGRTC runtime compilation version** (`inline_tix_hgrtc.cpp` + `inline_tix_kernel.hg`): HGRTC just-in-time compilation with Driver API

## Key Concepts

TIX Inline Assembly, Fused Multiply-Add (FMA), Bit Manipulation, Extended-Precision Arithmetic, Constraints and Operand Formats.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMemcpy, hggcFree, hggcDeviceSynchronize, hggcMalloc

### HGGC Driver API
hgMemcpyDtoH, hgMemcpyHtoD, hgLaunchKernel, hgCtxSynchronize, hgMemAlloc, hgMemFree, hgModuleGetFunction

## Dependencies for Build/Run
HGRTC (required for the runtime compilation variant)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
