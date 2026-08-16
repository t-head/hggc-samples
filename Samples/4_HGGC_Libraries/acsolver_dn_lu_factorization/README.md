# acsolver_dn_lu_factorization - LU Factorization In-Depth Demo

## Description

This sample uses the acsolverDn library to perform LU factorization with partial pivoting on a dense matrix, providing a detailed examination of the factorization result's internal structure.

Unlike a simple "factorize then solve" workflow, this program programmatically generates a test matrix, extracts and prints L, U factors and the pivot sequence, verifies P*A = L*U consistency on the host, then uses the factorization result to solve the linear system A*x = b and compares against the known true solution.

## Key Concepts

Linear Algebra, LU Factorization, ACSOLVER Library.

## Workflow

1. Generate a diagonally dominant random matrix A and known solution x_true, compute b = A*x_true
2. Perform LU factorization on the PPU (`acsolverDnDgetrf`), A is overwritten with compact L*U storage
3. Extract L (unit lower triangular) and U (upper triangular) factors, display pivot sequence
4. Verify P*A = L*U on host (reconstruction consistency check)
5. Solve A*x = b using the factorization result (`acsolverDnDgetrs`)
6. Compare against known solution x_true, compute relative residual ||b - A*x|| / (||A||*||x||)

## Command-Line Arguments

| Argument | Description | Default |
| --- | --- | --- |
| -N=<int> | matrix order | 5 |
| -seed=<int> | random seed | time-based |
| -device=<id> | specify Zhenwu PPU device | default device |

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### ACSOLVER API
acsolverDnCreate, acsolverDnDestroy, acsolverDnSetStream, acsolverDnDgetrf_bufferSize, acsolverDnDgetrf, acsolverDnDgetrs

### ACBLAS API
acblasCreate, acblasDestroy, acblasSetStream, acblasDgemm_v2

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMemcpy, hggcMemset, hggcStreamCreate, hggcStreamDestroy, hggcDeviceSynchronize

## Dependencies for Build/Run
ACSOLVER, ACBLAS

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
