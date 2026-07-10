# ctx_management - Multi-Context Lifecycle Management

## Description

This sample demonstrates the **Multi-Context management** mechanism of the HGGC Driver API:

1. **Creating multiple contexts** — Creates N independent `HGcontext` instances on the same device.
2. **Context migration** — Worker threads bind/unbind contexts via `hgCtxPushCurrent` / `hgCtxPopCurrent`.
3. **Context sharing** — Multiple threads take turns using the same context (serialized access).
4. **HGRTC compilation** — Each context independently compiles and loads kernel modules.
5. **Proper destruction** — `hgModuleUnload` + `hgCtxDestroy` cleanup.

Core APIs demonstrated:
- `hgCtxCreate` — Creates a floating context.
- `hgCtxPushCurrent` — Binds a context to the current thread.
- `hgCtxPopCurrent` — Unbinds a context from the current thread (becomes floating).
- `hgCtxSynchronize` — Waits for all work in the context to complete.
- `hgCtxDestroy` — Destroys the context.

## Key Concepts

Driver API Context Management, Multi-threaded Context Migration, hgCtxPushCurrent / hgCtxPopCurrent, HGRTC Runtime Compilation.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Driver API
hgInit, hgDeviceGet, hgDeviceGetName, hgDeviceGetAttribute, hgDeviceGetCount, hgCtxCreate, hgCtxPushCurrent, hgCtxPopCurrent, hgCtxSynchronize, hgCtxDestroy, hgModuleLoadData, hgModuleGetFunction, hgModuleUnload, hgLaunchKernel, hgMemAlloc, hgMemFree, hgMemcpyDtoH

## Dependencies for Build/Run
HGRTC, C++11 (std::thread)

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
