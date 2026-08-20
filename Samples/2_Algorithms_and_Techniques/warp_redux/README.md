# warp_redux - Hardware Warp-Level Reduction (ppu.redux.sync)

## Description

This sample demonstrates the TIX `ppu.redux.sync` instruction — a single hardware instruction that performs an entire warp reduction without requiring multiple shuffle iterations.

Each reduction operation is implemented in two ways and cross-verified:
- **ppu.redux.sync** (single instruction, hardware accelerated)
- **__shfl_down_sync loop** (traditional 5-round shuffle approach)

Operations demonstrated:
1. **ppu.redux.sync.add.s32** — Warp sum.
2. **ppu.redux.sync.min.s32** — Warp minimum.
3. **ppu.redux.sync.max.s32** — Warp maximum.
4. **ppu.redux.sync.xor.b32** — Warp XOR (parity check).

## Key Concepts

Warp-Level Reduction, TIX Inline Assembly, ppu.redux.sync, Hardware-Accelerated Primitives, __shfl_down_sync Comparison.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.redux.sync.add.s32, ppu.redux.sync.min.s32, ppu.redux.sync.max.s32, ppu.redux.sync.xor.b32

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
