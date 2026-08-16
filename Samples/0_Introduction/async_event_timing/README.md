# async_event_timing - Asynchronous API

## Description

This sample demonstrates how to use HGGC events for PPU timing and how to achieve parallel execution of the CPU and the PPU. Events are inserted into the HGGC call stream. Since HGGC stream calls are asynchronous, the CPU can perform computations while the PPU executes tasks (including DMA memory copies between host and device). The CPU can query HGGC events to determine whether the PPU has completed its tasks.

## Key Concepts

Asynchronous Data Transfers, HGGC Streams and Events.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcProfilerStop, hggcMalloc, hggcMemcpyAsync, hggcFree, hggcMallocHost, hggcProfilerStart, hggcDeviceSynchronize, hggcEventRecord, hggcFreeHost, hggcMemset, hggcEventDestroy, hggcEventQuery, hggcEventElapsedTime, hggcGetDeviceProperties, hggcEventCreate

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
