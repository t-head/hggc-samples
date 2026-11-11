# tf32_tensor_cell_gemm - TIX Tensor Cell TF32 GEMM (m16n16k8)

## Description

This sample implements TF32 precision matrix multiplication (D = A*B) using **pure TIX inline assembly**, demonstrating Tensor Cell instruction usage with the m16n16k8 shape.

Key differences from the bf16 version (`bf16_tensor_cell_gemm`):

| | bf16 version | tf32 version (this sample) |
|---|---|---|
| MMA shape | m16n16k**16** | m16n16k**8** |
| A/B data type | bf16 (2B, `.f16x2` pack) | tf32/f32 (4B, `.b32`) |
| K-tile size | 16 | 8 |
| A/B load method | ldmatrix | ppu.ld.shared.b32 (element-wise) |
| C/D layout | Writeback mapping selected for tc01/tc02 | Same as bf16 within each architecture |

Core TIX instructions:
- `ppu.tc01/tc02.mma.sync.aligned.m16n16k8.row.col.f32.tf32.tf32.f32` — 16x16x8 MMA
- `ppu.cp.async.cg.shared.global` — Async copy
- `ppu.ld.shared.b32` — Load tf32 fragment from shared memory
- `ppu.cvta.to.shared.u32` — Address space conversion

## Key Concepts

TIX Inline Assembly, Tensor Cell (tc01/tc02.mma m16n16k8), TF32 Precision, Double-Buffer Pipeline.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

The kernel selects `ppu.tc01` on ppu001 and `ppu.tc02` on ppu0015.

On ppu0015, A1/A2 are swapped after shared-memory loads so that A register `i = 0..3` maps to `row = lane / 4 + (i % 2) * 8` and `k = lane % 4 + (i / 2) * 4`. B keeps its original load order. The main loop and epilogue use the same load path and current double-buffer index.

C/D use the [architecture-specific coordinate mapping](../bf16_tensor_cell_gemm/README.md#architecture-specific-fragment-layout): tc02 R0/R1, R2/R3, R4/R5, and R6/R7 represent adjacent columns in the top-left, bottom-left, top-right, and bottom-right quadrants, respectively. The tc01 path retains its original order.

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.tc01/tc02.mma.sync.aligned.m16n16k8, ppu.cp.async.cg.shared.global, ppu.ld.shared.b32, ppu.cvta.to.shared.u32

## Dependencies for Build/Run
CPP11

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
