# multi_device_collab - Multi-Device Collaboration

## Description

This sample demonstrates two typical paths for collaborative work across multiple HGGC devices: multi-PPU asynchronous dispatch based on HGGC context management and multi-threaded access, and cross-device kernel computation between two PPUs with peer-to-peer (P2P) capability via P2P copies, P2P addressing, and Unified Virtual Addressing (UVA). Both paths share device enumeration and topology probing, switchable via `--mode=multi_dispatch|p2p` subcommand; by default, if at least one P2P-reachable PPU pair exists, `p2p` is selected automatically, otherwise falls back to `multi_dispatch`.

## Key Concepts

Performance Strategies, Asynchronous Data Transfers, HGGC Streams and Events, Unified Virtual Address Space, Peer to Peer Data Transfers, Multithreading, Multi-PPU.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcStreamDestroy, hggcFree, hggcMallocHost, hggcSetDevice, hggcFreeHost, hggcStreamSynchronize, hggcMalloc, hggcMemcpyAsync, hggcStreamCreate, hggcGetDeviceCount, hggcMemcpy, hggcEventCreateWithFlags, hggcEventSynchronize, hggcDeviceDisablePeerAccess, hggcDeviceSynchronize, hggcEventRecord, hggcGetDeviceProperties, hggcDeviceEnablePeerAccess, hggcEventDestroy, hggcEventElapsedTime, hggcDeviceCanAccessPeer

## Dependencies for Build/Run
only-64-bit

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.

**Mode Description**

- **`--mode=multi_dispatch`**: Multi-PPU asynchronous dispatch — uses HGGC context management and multi-threaded access to run HGGC kernels across multiple PPUs.
- **`--mode=p2p`**: Cross-device P2P collaboration — performs cross-device kernel computation between two identical PPUs via peer-to-peer (P2P) copies, P2P addressing, and Unified Virtual Addressing (UVA).
