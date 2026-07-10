# simple_hggc_graphs - Complete Graph Node Type Showcase

## Description

This sample demonstrates **all core node types** of HGGC Graphs in a coherent pipeline:

**Demo 1: Event Record/Wait Nodes**
- `hggcGraphAddEventRecordNode` — Marks completion points within a graph
- `hggcGraphAddEventWaitNode` — Waits for events from another graph
- Enables precise cross-graph synchronization

**Demo 2: Child Graph Nodes**
- `hggcGraphAddChildGraphNode` — Embeds a complete graph as a single node in the parent graph
- Enables modular graph composition

**Demo 3: Graph Clone**
- `hggcGraphClone` — Copies an existing graph
- `hggcGraphGetNodes` — Traverses the cloned graph's nodes
- Used to create graph variants (modifying selected nodes)

Also demonstrates: Kernel nodes, Host Callback nodes (`hggcGraphAddHostNode`).

## Key Concepts

HGGC Graph Node Types, Event Record/Wait, Child Graph, Graph Clone, Host Callback.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcGraphCreate, hggcGraphAddKernelNode, hggcGraphAddEventRecordNode, hggcGraphAddEventWaitNode, hggcGraphAddHostNode, hggcGraphAddChildGraphNode, hggcGraphClone, hggcGraphGetNodes, hggcGraphInstantiate, hggcGraphLaunch, hggcGraphExecDestroy, hggcGraphDestroy, hggcGraphKernelNodeSetParams, hggcEventCreate, hggcEventDestroy, hggcFree, hggcMalloc, hggcMemcpy, hggcStreamCreateWithFlags, hggcStreamDestroy, hggcStreamSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
