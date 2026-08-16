# graph_conditional_nodes - Graph Conditional Loop Iterative Solver

## Description

This sample implements a **device-side convergence-controlled iterative solver** using HGGC Graph conditional nodes: Jacobi method for solving tridiagonal linear systems, looping until convergence.

Graph structure:
```
init_kernel -> WHILE [ jacobi_step -> check_convergence ] -> done_kernel
```

Key mechanisms:
- `hggcGraphConditionalHandleCreate` creates a conditional handle (default value=1, ensuring at least one execution)
- `hggcGraphAddNode(hggcGraphNodeTypeConditional, hggcGraphCondTypeWhile)` adds a while conditional node
- `hggcGraphSetConditional(handle, 0/1)` controls loop termination on the device side
- `hggcStreamBeginCaptureToGraph` uses stream capture to populate the loop body

Complementary to `power_iteration_graph` (which uses `hggcGraphExecKernelNodeSetParams` to dynamically update kernel parameters) — this sample demonstrates the device-side loop control capability of conditional nodes.

## Key Concepts

HGGC Graph Conditional Nodes, Device-Side Decision Making, Iterative Convergence, Stream Capture.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcGraphCreate, hggcGraphAddNode, hggcGraphConditionalHandleCreate, hggcGraphSetConditional, hggcGraphInstantiate, hggcGraphLaunch, hggcGraphExecDestroy, hggcGraphDestroy, hggcStreamCreate, hggcStreamBeginCaptureToGraph, hggcStreamEndCapture, hggcDeviceSynchronize, hggcFree, hggcMalloc, hggcMemcpy, hggcStreamDestroy

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
