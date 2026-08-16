# block_sort_merge - Block Sort + Bitonic Merge

## Description

This sample demonstrates a two-stage parallel sort on PPU:
1. **Block sort**: Each thread block sorts a chunk of elements in shared memory using an odd-even transposition network
2. **Bitonic merge**: Adjacent sorted blocks are merged using a bitonic merge network in shared memory

The sort handles key-value pairs and verifies stability (equal keys preserve original value ordering) against a CPU reference using `std::stable_sort`.

## Key Concepts

Parallel Sorting, Odd-Even Transposition Sort, Bitonic Merge, Key-Value Stability, Shared Memory

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMemcpy, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
