# driver_runtime_interop - Simple Driver-Runtime Interop

## Description

A simple sample demonstrating how the HGGC Driver API and Runtime API work together to load an hggc fatbinary of a vector addition kernel and execute the vector addition.

## Key Concepts

HGGC Driver API, HGGC Runtime API, Vector Addition.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcStreamCreateWithFlags, hggcFree, hggcMallocHost, hggcFreeHost, hggcStreamSynchronize, hggcMalloc, hggcMemcpyAsync, hggcStreamDestroy

### HGGC Driver API
hgLaunchKernel, hgModuleLoadData, hgCtxDestroy, hgModuleUnload, hgModuleGetFunction, hgCtxCreate, hgInit

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
