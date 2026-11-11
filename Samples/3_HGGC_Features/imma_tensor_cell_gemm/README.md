# imma_tensor_cell_gemm - TIX Tensor Cell INT8 GEMM (m16n16k32)

## Description

This sample implements INT8 matrix multiplication (D = A*B, s8 input, s32 accumulation) using **pure TIX inline assembly**.

Comparison with bf16/tf32 versions:

| | bf16 | tf32 | **int8 (this sample)** |
|---|---|---|---|
| MMA shape | m16n16k16 | m16n16k8 | **m16n16k32** |
| A/B type | bf16 (2B) | tf32 (4B) | **s8 (1B)** |
| C/D type | f32 | f32 | **s32** |
| K per tile | 16 | 8 | **32** |
| Iterations (K=256) | 16 | 32 | **8** |

Core TIX instructions:
- `ppu.tc01/tc02.mma.sync.aligned.m16n16k32.row.col.s32.s8.s8.s32` — INT8 MMA
- `ppu.cp.async.cg.shared.global` — Async copy
- `ppu.tc01/tc02.ldmatrix.sync.aligned` — Fragment load

Precision characteristic: integer MMA computation has no rounding error, results should exactly match the CPU reference.

## Key Concepts

TIX Inline Assembly, Tensor Cell (tc01/tc02.mma m16n16k32), INT8 Quantized Computation, Double-Buffer Pipeline.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

The kernel selects `ppu.tc01` on ppu001 and `ppu.tc02` on ppu0015.

On ppu0015, A row addresses use `row = lane % 16` and byte-column offset `(lane / 16) * 16`. B retains the shared-memory layout of paired K half-fragments (K rows 0, 16, 1, 17, ...), uses `m16n16.x2.trans.shared.b8`, and swaps B1/B2 in both the main loop and epilogue to match the tc02 MMA K half-fragment order. The ppu001 path retains `m16n16.x2.diagswap.trans.shared.b8`, without extra B-register swaps.

The eight INT8 s32 accumulators use the same [architecture-specific C/D coordinate mapping](../bf16_tensor_cell_gemm/README.md#architecture-specific-fragment-layout) as floating-point GEMM, but tc01 and tc02 have different writeback orders.

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.tc01/tc02.mma.sync.aligned.m16n16k32, ppu.cp.async.cg.shared.global, ppu.tc01/tc02.ldmatrix.sync.aligned, ppu.cvta.to.shared.u32

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
