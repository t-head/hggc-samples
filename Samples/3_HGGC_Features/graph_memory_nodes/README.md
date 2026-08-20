# graph_memory_nodes - Graph Construction Method Comparison (API vs Stream Capture)

## Description

This sample demonstrates **two equivalent approaches** for building HGGC Graphs, verifying that they produce identical execution results:

1. **Graph API (explicit construction)** — Manually creates nodes and specifies dependency relationships
   - `hggcGraphAddMemAllocNode` / `hggcGraphAddMemFreeNode` + `hggcGraphAddKernelNode`
   - More flexible, allows precise control over dependency topology

2. **Stream Capture (implicit construction)** — Normal HGGC code in capture mode automatically generates a graph
   - `hggcStreamBeginCapture` -> normal kernel launch -> `hggcStreamEndCapture`
   - More concise code, identical to non-graph code

Computation: `output = input * 2 + 1` (two kernel nodes + one memcpy node)

## Key Concepts

HGGC Graph API, Stream Capture, Graph Construction Mode Comparison.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcGraphCreate, hggcGraphAddKernelNode, hggcGraphAddMemAllocNode, hggcGraphAddMemFreeNode, hggcGraphInstantiate, hggcGraphLaunch, hggcGraphExecDestroy, hggcGraphDestroy, hggcStreamBeginCapture, hggcStreamEndCapture, hggcStreamCreateWithFlags, hggcFree, hggcFreeAsync, hggcMalloc, hggcMallocAsync, hggcMemcpy, hggcStreamDestroy, hggcStreamSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
