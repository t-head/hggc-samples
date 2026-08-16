# cpp_template_kernels - C++ Template Kernels

## Description

This sample demonstrates advanced usage of C++ templates in PPU kernels through a **Policy-based Parallel Reduction**:
- **Template template parameters**: reduction policies (SumOp / MaxOp) injected as parameters into the kernel
- **Type-safe dynamic shared memory wrapper**: implemented via `DynamicSharedBuffer<T>`
- **Template specialization**: provides identity values for different data types (e.g., `INT_MIN` / `-FLT_MAX`)
- **Multi-type x multi-operation combination dispatch**: `{float, int}` x `{Sum, Max}` four combinations verified at once

## Key Concepts

C++ Templates, Template Template Parameters, Dynamic Shared Memory, Parallel Reduction.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcGetDeviceProperties, hggcFree, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
