# global_to_shmem_async_copy - Async Copy Synchronization Strategy Comparison

## Description

This sample compares three **synchronization strategies** for Global -> Shared Memory async copy, showcasing the PPU-specific awbar mechanism:

1. **Naive** — Explicit load (global -> register -> shared) + `__syncthreads()`
2. **cp.async + commit_group/wait_group** — Classic async copy pipeline (TIX `ppu.cp.async` instruction family)
3. **cp.async + awbar** — Arrive-wait barrier, unifying async copy completion and thread synchronization in the same primitive

What makes awbar unique:
- `ppu.cp.async.awbar.arrive` automatically associates copy completion events with the barrier
- Threads and async operations share the same counter
- `ppu.awbar.test_wait` non-blocking poll, supporting multi-phase parity switching
- More fine-grained than commit_group/wait_group, no need for global group numbering

Payload: tiled vector reduction (simple, focused on synchronization mechanism differences).

## Key Concepts

Async Copy (cp.async), Arrive-Wait Barrier (awbar), Synchronization Strategy Comparison, PPU-Specific Synchronization Primitives.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs and Instructions Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### TIX Instructions
ppu.cp.async.ca.shared.global, ppu.cp.async.commit_group, ppu.cp.async.wait_group, ppu.cp.async.awbar.arrive, ppu.awbar.init, ppu.awbar.arrive, ppu.awbar.test_wait, ppu.cvta.to.shared.u32

## Dependencies for Build/Run
CPP11

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
