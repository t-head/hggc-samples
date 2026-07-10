# scan - Multi-Block Exclusive Prefix Sum (Blelloch Algorithm)

## Description

This sample implements an arbitrary-length **exclusive prefix sum** using the classic Blelloch work-efficient algorithm:

1. **Up-sweep (reduction phase)**: builds a partial sum tree from bottom to top
2. **Down-sweep (distribution phase)**: distributes prefix sums from top to bottom

Multi-block strategy uses a three-pass execution:
- Pass 1: each block independently scans its assigned chunk, writing out block totals
- Pass 2: performs prefix sum on the block totals array (single block)
- Pass 3: adds the block prefix to each element

Supports non-power-of-two input lengths, verified for correctness across multiple sizes.

## Key Concepts

Prefix Sum (Scan), Blelloch Algorithm, Work-Efficient Parallel Algorithms, Shared Memory Tree Reduction.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
