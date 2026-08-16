# hggc_graphs_perf_scaling - Graph API Performance Scaling

## Description

This sample measures how HGGC Graph API operations (capture, instantiation, launch) scale with graph topology size. Parallel-chain graphs of increasing complexity (10 to 200 nodes) are built via stream capture, then each phase is timed to show scaling behavior.

## Key Concepts

Graph Capture, Graph Instantiation, Graph Launch, Stream Capture, Performance Scaling, Parallel Topology

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcStreamCreate, hggcStreamDestroy, hggcStreamBeginCapture, hggcStreamEndCapture, hggcStreamSynchronize, hggcStreamWaitEvent, hggcEventCreate, hggcEventDestroy, hggcEventRecord, hggcGraphInstantiateWithFlags, hggcGraphLaunch, hggcGraphExecDestroy, hggcGraphDestroy

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
