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
| C/D layout | 8 sub-matrices of 8x4 | 8 sub-matrices of 8x4 (same) |

Core TIX instructions:
- `ppu.tc01.mma.sync.aligned.m16n16k8.row.col.f32.tf32.tf32.f32` — 16x16x8 MMA
- `ppu.cp.async.cg.shared.global` — Async copy
- `ppu.ld.shared.b32` — Load tf32 fragment from shared memory
- `ppu.cvta.to.shared.u32` — Address space conversion

## Key Concepts

TIX Inline Assembly, Tensor Cell (tc01.mma m16n16k8), TF32 Precision, Double-Buffer Pipeline.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.tc01.mma.sync.aligned.m16n16k8, ppu.cp.async.cg.shared.global, ppu.ld.shared.b32, ppu.cvta.to.shared.u32

## Dependencies for Build/Run
CPP11

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
