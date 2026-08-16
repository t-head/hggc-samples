# openmp_ppu_dispatch - Multi-Device Array Scaling via OpenMP

## Description

This sample demonstrates multi-PPU application development using OpenMP. Each OpenMP thread binds to a PPU device and processes a slice of a float array with a scale-and-shift kernel (`out[i] = in[i] * scale + shift`). Results are verified against a CPU reference with tolerance-based comparison.

## Key Concepts

OpenMP Integration, Multi-Device Dispatch, Array Scaling, CPU Reference Verification

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcGetDeviceCount, hggcGetDeviceProperties, hggcSetDevice, hggcGetDevice, hggcMalloc, hggcMemcpy, hggcFree

## Dependencies for Build/Run
OpenMP

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
