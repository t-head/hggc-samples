# graph_memory_nodes - Graph Construction Method Comparison (API vs Stream Capture)

## Description

This sample demonstrates **two equivalent approaches** for building HGGC Graphs, verifying that they produce identical execution results:

1. **Graph API (explicit construction)** — Manually creates nodes and specifies dependency relationships
   - `hggcGraphAddMemAllocNode` / `hggcGraphAddMemFreeNode` + `hggcGraphAddKernelNode`
   - More flexible, allows precise control over dependency topology

2. **Stream Capture (implicit construction)** — Normal HGGC code in capture mode automatically generates a graph
   - `hggcStreamBeginCapture` -> normal kernel launch -> `hggcStreamEndCapture`
   - More concise code, identical to non-graph code

Computes `output = input * 2 + 1` using the graph `MemAlloc -> scale(x2) -> bias(+1) -> MemFree`, without memcpy nodes.

Each construction method runs both an ordinary launch and an explicit `hggcGraphUpload` before its first launch, giving four independent paths. Each path uses a fresh executable instance and separate output. Upload, launch, and synchronization use the same non-default stream, followed by finite-value checks, CPU-reference comparison, and cross-path comparison. There is no performance threshold for PASS.

`-device=N` selects the actual device, and the explicit allocation node's `poolProps.location.id` matches that selection.

## Key Concepts

HGGC Graph API, Stream Capture, Explicit Upload, Graph Memory Nodes, Device Selection, Four-Path Correctness Comparison.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcGraphCreate, hggcGraphAddKernelNode, hggcGraphAddMemAllocNode, hggcGraphAddMemFreeNode, hggcGraphInstantiate, hggcGraphUpload, hggcGraphLaunch, hggcGraphExecDestroy, hggcGraphDestroy, hggcStreamBeginCapture, hggcStreamEndCapture, hggcStreamCreateWithFlags, hggcFree, hggcFreeAsync, hggcMalloc, hggcMallocAsync, hggcMemcpy, hggcStreamDestroy, hggcStreamSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform. Requires SAIL 2.2.0 or later.
