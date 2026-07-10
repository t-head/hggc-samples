# power_iteration_graph - Graph Exec Dynamic Parameter Update (Power Iteration)

## Description

This sample demonstrates `hggcGraphExecKernelNodeSetParams` for **dynamically updating kernel parameters** without rebuilding the graph, using the **Power Iteration method** to find the largest eigenvalue of a matrix.

Algorithm:
```
v_new = A * v_old / ||A * v_old||
Repeat until convergence, swapping v_old <-> v_new each step
```

Graph structure (built via Stream Capture):
```
matvec(A, src, dst) -> norm(dst) -> normalize(dst)
```

After each iteration, `hggcGraphExecKernelNodeSetParams` swaps src/dst pointers without re-instantiating the graph.

Difference from `graph_conditional_nodes` (device-side while loop): in this sample, the loop is controlled on the host side, the graph exec is repeatedly launched, but node parameters are updated before each launch.

## Key Concepts

Graph Exec Parameter Update (hggcGraphExecKernelNodeSetParams), Power Iteration, Stream Capture, Iterative Convergence.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcGraphExecKernelNodeSetParams, hggcGraphGetNodes, hggcStreamBeginCapture, hggcStreamEndCapture, hggcGraphInstantiate, hggcGraphLaunch, hggcGraphExecDestroy, hggcGraphDestroy, hggcFree, hggcMalloc, hggcMemcpy, hggcStreamCreateWithFlags, hggcStreamDestroy, hggcStreamSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
