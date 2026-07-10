# bf16_tensor_cell_gemm - TIX Tensor Cell BF16 GEMM (Double-Buffer)

## Description

This sample implements BF16 matrix multiplication (D = A*B) using **pure TIX inline assembly**, demonstrating the low-level usage of Tensor Cell instructions and the double-buffer asynchronous pipeline:

Core TIX instructions:
- `ppu.cp.async.cg.shared.global` — Asynchronous Global -> Shared copy (16 bytes/thread)
- `ppu.cp.async.commit_group` / `ppu.cp.async.wait_group` — Async copy group management
- `ppu.tc01.ldmatrix.sync.aligned.m8n8.x4.b16` — A matrix Shared -> Register fragment load
- `ppu.tc01.ldmatrix.sync.aligned.m16n16.x1.trans.shared.b16` — B matrix load (.trans transpose generates col-major fragment)
- `ppu.tc01.mma.sync.aligned.m16n16k16.row.col.f32.bf16.bf16.f32` — 16x16x16 MMA

Algorithm structure:
1. **Prologue**: prefetch the first K-tile into shared buffer[0]
2. **Main loop**: prefetch next K-tile into buffer[1-cur], while using buffer[cur] for ldmatrix + mma
3. **Epilogue**: process the last K-tile, write back f32 results

Per-thread register fragment layout (m16n16k16 bf16):
- A fragment: 4 `.f16x2` registers (constraint `"r"`)
- B fragment: 4 `.f16x2` registers (constraint `"r"`)
- C/D fragment: 8 `.f32` registers (constraint `"f"`)

## Key Concepts

TIX Inline Assembly, Tensor Cell (tc01.mma), Async Copy (cp.async), Double-Buffer Pipeline, Matrix Fragment Layout, ldmatrix.trans (row-major to col-major conversion).

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.cp.async.cg.shared.global, ppu.cp.async.commit_group, ppu.cp.async.wait_group, ppu.tc01.ldmatrix.sync.aligned.m8n8.x4.b16, ppu.tc01.ldmatrix.sync.aligned.m16n16.x1.trans.shared.b16, ppu.tc01.mma.sync.aligned.m16n16k16

## Dependencies for Build/Run
CPP11

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
