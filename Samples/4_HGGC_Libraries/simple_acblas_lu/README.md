# simple_acblas_lu - LU Decomposition via BLAS Primitives

## Description

This sample implements LU decomposition with partial pivoting using **BLAS Level 1/2/3 primitives**,
demonstrating how basic BLAS operations compose into higher-level linear algebra algorithms.

Instead of calling the high-level `getrfBatched`, the decomposition is built step-by-step using:
- `acblasDswap` — Row swapping (pivot selection)
- `acblasDscal` — Column scaling (normalization)
- `acblasDger` — Rank-1 update (Schur complement)
- `acblasDgemm` — Verify L*U = P*A
- `acblasDtrsm` — Triangular solve (Ax=b forward/back substitution)

## Algorithm

```
for k = 0..n-1:
  1. Find pivot (host-side column max search)
  2. acblasDswap — swap row k with pivot row
  3. acblasDscal — scale below-diagonal by 1/A[k,k]
  4. acblasDger  — rank-1 update of trailing submatrix
```

Verification: compute L*U via `acblasDgemm`, compare with permuted P*A.

Linear solve: forward substitution (Ly=Pb) and back substitution (Ux=y) via `acblasDtrsm`.

## Key Concepts

ACBLAS Library, LU Decomposition, BLAS Level 1/2/3, Partial Pivoting, Doolittle Factorization

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree

### ACBLAS API
acblasCreate, acblasDestroy, acblasDswap, acblasDscal, acblasDger, acblasDgemm, acblasDtrsm

## Dependencies for Build/Run
ACBLAS

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
