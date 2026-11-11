# bf16_tensor_cell_gemm - TIX Tensor Cell BF16 GEMM (Double-Buffer)

## Description

This sample implements BF16 matrix multiplication (D = A*B) using **pure TIX inline assembly**, demonstrating the low-level usage of Tensor Cell instructions and the double-buffer asynchronous pipeline:

Core TIX instructions:
- `ppu.cp.async.cg.shared.global` — Asynchronous Global -> Shared copy (16 bytes/thread)
- `ppu.cp.async.commit_group` / `ppu.cp.async.wait_group` — Async copy group management
- `ppu.tc01/tc02.ldmatrix.sync.aligned.m8n8.x4.b16` — A matrix Shared -> Register fragment load
- Architecture-specific transpose `ldmatrix` form — B matrix load (.trans generates col-major fragment)
- `ppu.tc01/tc02.mma.sync.aligned.m16n16k16.row.col.f32.bf16.bf16.f32` — 16x16x16 MMA

Algorithm structure:
1. **Prologue**: prefetch the first K-tile into shared buffer[0]
2. **Main loop**: prefetch next K-tile into buffer[1-cur], while using buffer[cur] for ldmatrix + mma
3. **Epilogue**: process the last K-tile, write back f32 results

Per-thread register fragment layout (m16n16k16 bf16):
- A fragment: 4 `.f16x2` registers (constraint `"r"`)
- B fragment: 4 `.f16x2` registers (constraint `"r"`)
- C/D fragment: 8 `.f32` registers (constraint `"f"`)

## Key Concepts

TIX Inline Assembly, Tensor Cell (tc01/tc02.mma), Async Copy (cp.async), Double-Buffer Pipeline, Matrix Fragment Layout, ldmatrix.trans (row-major to col-major conversion).

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

The kernel selects `ppu.tc01` on ppu001 and `ppu.tc02` on ppu0015.

### Architecture-Specific Fragment Layout

On ppu0015, A/B row addresses use `row = lane % 16` and `col = (lane / 16) * 8`; B uses `m8n8.x4.trans.shared.b16`. Loaded fragments feed tc02 MMA directly, without swapping intermediate A/B registers. The ppu001 path retains its original row addresses and B's `m16n16.x1.trans.shared.b16` load.

For C/D register `R[i]` (`i = 0..7`), coordinates are relative to the current 16x16 output tile:

| Architecture | row | col |
|---|---|---|
| tc01 | `lane / 4 + (i / 4) * 8` | `lane % 4 + (i % 4) * 4` |
| tc02 | `lane / 4 + ((i % 4) / 2) * 8` | `(lane % 4) * 2 + (i % 2) + (i / 4) * 8` |

Here `/` denotes integer division. Each tc02 lane holds two adjacent columns: R0/R1 are top-left, R2/R3 bottom-left, R4/R5 top-right, and R6/R7 bottom-right. The tc01 writeback order cannot be reused on tc02. Within the same architecture, TF32, AIU FP16, and INT8 GEMM use this same C/D coordinate mapping.

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.cp.async.cg.shared.global, ppu.cp.async.commit_group, ppu.cp.async.wait_group, ppu.tc01/tc02.ldmatrix.sync.aligned, ppu.tc01/tc02.mma.sync.aligned.m16n16k16

## Dependencies for Build/Run
CPP11

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
