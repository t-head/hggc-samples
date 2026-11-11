# aiu_gemm - AIU GEMM: FP16 GEMM via AIU .swzl Bulk Tensor Copy

## Description

FP16 matrix multiplication D = A x B using AIU `.swzl` mode (128B swizzle) for A and B matrix transfers,
combined with `ldmatrix.swzl` for address decoding and loading.

## Key Features

- **AIU .swzl** bulk transfer of 16x64 tiles (dim_c=64 -> 128B swizzle).
- **A matrix**: pre-arranged as 16 M-rows x 64 K-cols, `ldmatrix.swzl.m8n8.x4.b16` (no trans) load.
- **B matrix**: pre-arranged as 16 K-rows x 64 N-cols (original row-major, not transposed),
  4 N-tiles packed, using the architecture-specific transpose `ldmatrix.swzl` form -> column-major fragment.
- **channel_offset**: A selects K-tile, B selects N-tile.
- **ppu.tc01/tc02.mma.m16n16k16.row.col** accumulation computation.
- Random input verification, relative error < 5%.

## Tiling Scheme

```
A tile: 16 M-rows x 64 K-cols (1 M-tile x 1 K-group)
B tile: 16 K-rows x 64 N-cols (1 K-tile x 4 N-tiles packed)

Each block outputs 16x64 (1 M-tile x 4 N-tiles)
Grid: (N/64, M/16)
```

## B Matrix End-to-End Layout

```
gmem: B[k][n] row-major (KxN)
  -> pre-tile: B_tiled[r=K, c=N] remains row-major
  -> AIU .swzl: smem remains row-major (with swizzle)
  -> architecture-specific ldmatrix.swzl.trans: .trans transpose
  -> fragment: column-major (same n, adjacent k) -> MMA .col check
```

## Key Concepts

AIU Hardware Unit (ppu.cp.async.aiu), 128B Swizzle Bulk Transfer, ldmatrix.swzl Address Decoding, Tensor Cell MMA (ppu.tc01/tc02.mma), FP16 Matrix Multiplication.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

The kernel selects the legacy 3D AIU/`tc01` path on ppu001 and the 2D tile AIU/`tc02` path on ppu0015.

On ppu0015, swizzle-load addresses use 16B units: `(smem_addr + channel_byte_offset) >> 4`. A uses `m8n8.x4.shared.b16` and B uses `m8n8.x4.trans.shared.b16`; both feed tc02 MMA directly, without extra fragment swaps. The ppu001 path retains its original descriptor, byte addresses, and B's `m16n16.x1.trans.shared.b16` load.

C/D use the [architecture-specific coordinate mapping](../bf16_tensor_cell_gemm/README.md#architecture-specific-fragment-layout). Each tc02 lane holds two adjacent columns: registers cover the upper/lower fragments of the left half first, then the right half. The tc01 path retains its original writeback order.

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.cp.async.aiu.bulk.tensor.shared.global, ppu.tc01/tc02.ldmatrix.swzl, ppu.tc01/tc02.mma.sync.aligned.m16n16k16

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
