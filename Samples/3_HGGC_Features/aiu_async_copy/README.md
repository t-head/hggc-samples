# aiu_async_copy - Cross-Architecture AIU Async Tensor Copy + ldmatrix.swzl Address Decoding

## Description

This sample demonstrates architecture-specific **ppu.cp.async.aiu** asynchronous tensor bulk copy from global memory to shared memory, together with **ppu.tc01/tc02.ldmatrix.swzl** address decoding and register loading — a complete pipeline:

1. **AIU copy** — Lane 0 of each of four warps uses a ppu001 3D descriptor or a ppu0015 2D tile descriptor to asynchronously copy its 16x1x64 tile into a separate, 128B-aligned shared-memory region. AIU performs the swizzle.
2. **Synchronization** — `commit_group` + `wait_group 0` + `__syncthreads` ensure that the copy is complete and visible to the whole block.
3. **ldmatrix.swzl load** — Each warp collectively executes four architecture-specific `ppu.tc01/tc02.ldmatrix.swzl` loads. Each load reads 16x16 b16 elements and produces four b32 registers per thread, for a total of 32 b16 elements per thread.
4. **Content verification** — Write each fragment back to its original positions. The host compares all 4096 elements position by position as `uint16_t` values and requires exact equality. Do not sort the output, as sorting could hide layout errors.

The source tensor is 64x1x64: 4096 b16 elements (8192B) initialized as increasing `uint16_t` bit patterns, with no floating-point arithmetic. One block has 128 threads (four warps); each warp handles 16x1x64 elements (2048B).

On ppu0015, swizzle-load addresses use 16B units: `(smem_addr + channel_byte_offset) >> 4`. The tc02 x4 fragment order is top-left, bottom-left, top-right, bottom-right. Swapping R1/R2 allows the common tc01 writeback order: top-left, top-right, bottom-left, bottom-right.

## Key Concepts

AIU Hardware Unit (ppu.cp.async.aiu), 128B Swizzle Address Transformation, ldmatrix.swzl Address Decoding, Async Copy Synchronization (commit_group/wait_group), Bulk Tensor Copy.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

The kernel selects the legacy 3D AIU/`tc01` path on ppu001 and the 2D tile AIU/`tc02` path on ppu0015.

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.cp.async.aiu.bulk.tensor.shared.global, ppu.cp.async.commit_group, ppu.cp.async.wait_group, ppu.tc01/tc02.ldmatrix.swzl, ppu.cvta.to.shared.u32

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
