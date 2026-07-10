# cooperative_sync - Cooperative Synchronization Primitives

## Description

This sample demonstrates two HGGC cooperative synchronization primitives: vector normalization under an Arrive-Wait Barrier, and basic intra-block usage of Cooperative Groups. Both paths share device discovery and command-line parsing, switchable via `--mode=barrier|groups` subcommand; defaults to `groups` mode.

## Key Concepts

Arrive-Wait Barrier, Cooperative Groups.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API

hggcStreamCreateWithFlags, hggcFree, hggcDeviceGetAttribute, hggcMallocHost, hggcFreeHost, hggcStreamSynchronize, hggcLaunchCooperativeKernel, hggcMalloc, hggcOccupancyMaxActiveBlocksPerMultiprocessor, hggcMemcpyAsync, hggcOccupancyMaxPotentialBlockSize, hggcDeviceSynchronize, hggcGetErrorString, hggcStreamDestroy

## Dependencies for Build/Run
CPP11, MBCG (required for the `--mode=barrier` path)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.

**Mode Description**

- **`--mode=barrier`**: Arrive-Wait Barrier implementation — all threads complete phase synchronization via arrive-wait primitives, demonstrating vector normalization under a device-side cooperative barrier.
- **`--mode=groups`** (default): Cooperative Groups implementation — uses the `thread_block` / `thread_block_tile` / `coalesced_threads` abstractions provided by `cooperative_groups` to show basic intra-block usage of Cooperative Groups.
