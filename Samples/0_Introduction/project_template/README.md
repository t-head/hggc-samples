# project_template - GEMV Project Template

## Description

An HGGC project template based on **GEMV (matrix-vector multiplication `y = A · x`)**, suitable as a starting point for creating new HGGC projects.
The sample also demonstrates:

- **Mixed compilation**: `.hg` kernel + `.cpp` host reference implementation, compiled separately and linked into a single executable
- **Shared memory tiling**: loads vector `x` in tiles into shared memory for reuse by all threads within the same block
- **Multi-block parallelism**: each thread is responsible for one row of output vector `y`, with the grid automatically covering the entire M dimension
- **Complete host-side pipeline**: device query -> device memory allocation -> H->D / D->H data transfer -> kernel launch -> timing -> CPU verification

## Key Concepts

- Shared Memory and `__syncthreads()`.
- Block/thread indexing
- Device memory allocation and synchronization (`hggcMalloc` / `hggcMemcpy` / `hggcDeviceSynchronize`)
- Host/Device collaboration and CPU reference verification

## Default Problem Size

| Parameter | Value | Description |
|------|----|----|
| `kM` | 1024 | Number of matrix rows |
| `kN` | 2048 | Number of matrix columns / vector length |
| `kBlockSize` | 256 | Threads per block |
| `kTileSize`  | 256 | Shared memory tile width |

Adjust `kM` / `kN` to evaluate bandwidth and performance at larger data scales.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
- `hggcMalloc`, `hggcFree`
- `hggcMemcpy` (HostToDevice / DeviceToHost)
- `hggcDeviceSynchronize`

## Build and Run

```bash
cd build
cmake ..
make project_template
./Samples/0_Introduction/project_template/project_template
```

Expected output:
```
[project_template] GEMV (y = A·x)  M=1024  N=2048

  PPU kernel time : <elapsed> ms
  Result: PASS
```

## Prerequisites

Please download and install the T-Head SAIL toolkit for your platform.
