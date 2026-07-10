# device_query - PPU Device Property Inspector

## Description

This sample queries and displays PPU device properties organized by functional category (Compute, Memory, Scheduling, Texture Limits, Features) using two API paths:
- **Runtime API** (`device_query.cpp`): Uses `DeviceReporter` class with grouped property report and box-drawing format
- **Driver API** (`device_query_drv.cpp`): Uses `hgDeviceGetAttribute` per-attribute queries with sectioned key-value output

Both versions also enumerate P2P topology between devices.

## Key Concepts

HGGC Runtime API, HGGC Driver API, Device Query, Property Inspection, P2P Topology

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcGetDeviceCount, hggcGetDeviceProperties, hggcSetDevice, hggcDriverGetVersion, hggcRuntimeGetVersion, hggcDeviceGetAttribute, hggcDeviceCanAccessPeer, hggcGetErrorString

### HGGC Driver API
hgInit, hgDeviceGetCount, hgDeviceGetName, hgDeviceTotalMem, hgDeviceGetAttribute, hgDriverGetVersion, hgDeviceCanAccessPeer

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
