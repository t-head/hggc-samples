# graph_memory_footprint - Graph Memory Node Buffer Reuse

## Description

This sample demonstrates how HGGC Graph memory nodes can **automatically reuse temporary buffers**, reducing peak memory footprint of multi-step compute pipelines.

Scenario: a 3-step compute pipeline (`input*2 -> +1 -> *0.5`), each step requiring a different-sized temporary buffer.

Comparison:
- **No reuse**: all buffers exist simultaneously -> peak = buf1 + buf2 + buf3 = 16 KB
- **Graph memory nodes**: alloc/free nodes let the runtime reuse physical memory after previous step's release -> peak ≈ max(buf1, buf2, buf3) = 8 KB

APIs demonstrated:
- `hggcGraphAddMemAllocNode` — Allocates temporary memory within a graph
- `hggcGraphAddMemFreeNode` — Frees within a graph (enables runtime reuse)
- `hggcDeviceGetGraphMemAttribute` — Queries actual memory usage
- `hggcDeviceGraphMemTrim` — Releases all unused graph memory

## Key Concepts

Graph Memory Nodes, Buffer Reuse, Peak Memory Optimization, Compute Pipeline.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcGraphCreate, hggcGraphAddMemAllocNode, hggcGraphAddMemFreeNode, hggcGraphAddKernelNode, hggcGraphInstantiate, hggcGraphLaunch, hggcDeviceGetGraphMemAttribute, hggcDeviceGraphMemTrim, hggcGraphExecDestroy, hggcGraphDestroy, hggcFree, hggcMalloc, hggcMemcpy, hggcStreamCreateWithFlags, hggcStreamDestroy, hggcStreamSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
