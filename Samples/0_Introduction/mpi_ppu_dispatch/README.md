# mpi_ppu_dispatch - MPI + PPU Temperature Gradient

## Description

This sample demonstrates MPI + PPU integration for 1D temperature gradient computation. A temperature profile is distributed across MPI ranks via `MPI_Scatter`, each rank computes a central-difference gradient on PPU, then results are collected via `MPI_Gather` to root for statistics and CPU reference verification.

Pipeline:
1. Root generates a 1D temperature profile (linear gradient + random perturbation)
2. `MPI_Scatter` distributes segments to all ranks
3. Each rank computes gradient on PPU: `out[i] = (T[i+1] - T[i-1]) / 2`
4. `MPI_Gather` collects gradients back to root
5. Root reports min/max/mean statistics and verifies against CPU reference

## Key Concepts

MPI Integration, PPU Kernel, Central Difference, Scatter/Gather, Temperature Gradient

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcGetLastError

## Dependencies for Build/Run
MPI

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
