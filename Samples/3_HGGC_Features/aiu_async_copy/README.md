# aiu_async_copy - AIU TIX 1.0 Async Tensor Copy + ldmatrix.swzl Address Decoding

## Description

This sample demonstrates **ppu.cp.async.aiu** (TIX 1.0) asynchronous tensor bulk copy from global memory to shared memory, along with the companion **ppu.tc01.ldmatrix.swzl** for decoding swizzle addresses and loading into registers — a complete pipeline:

1. **AIU copy** — Thread 0 uses `ppu.cp.async.aiu.bulk.tensor.shared.global.3d.cg.padz.swzl.b16` to asynchronously copy a 16x16 bf16 matrix from global memory to 128B-aligned shared memory, with AIU hardware automatically performing swizzle transformation on shared memory addresses to reduce bank conflicts.
2. **Synchronization** — `commit_group` + `wait_group 0` + `__syncthreads` ensures copy completion and visibility to all threads.
3. **ldmatrix.swzl load** — A warp of 32 threads collectively executes `ppu.tc01.ldmatrix.swzl.sync.aligned.m8n8.x4.shared.b16`, decoding data from the swizzle-layout shared memory, with each thread obtaining 4 b32 registers (8 bf16 values total).
4. **Content verification** — Each thread writes its register fragment back to global memory; the host sorts both input/output and performs element-wise comparison to verify data transfer correctness.

Payload: a single 16x16 bf16 matrix (256 elements), sequential incrementing pattern, pure data movement (no arithmetic), focused on the AIU + ldmatrix.swzl cooperative mechanism.

## Key Concepts

AIU Hardware Unit (ppu.cp.async.aiu), 128B Swizzle Address Transformation, ldmatrix.swzl Address Decoding, Async Copy Synchronization (commit_group/wait_group), Bulk Tensor Copy.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.cp.async.aiu.bulk.tensor.shared.global, ppu.cp.async.commit_group, ppu.cp.async.wait_group, ppu.tc01.ldmatrix.swzl.sync.aligned.m8n8.x4.shared.b16, ppu.cvta.to.shared.u32

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
