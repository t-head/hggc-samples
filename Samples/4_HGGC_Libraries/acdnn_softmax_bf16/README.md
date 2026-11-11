# acdnn_softmax_bf16

## Description

Available only in `feat/sdk-2.2.1-samples`. Requires SAIL 2.2.1 and the corresponding ACDNN library. Adding this sample does not imply that the `ACDNN_DATA_BF16` enum or softmax API was first introduced in this version.

Uses fully packed NCHW input/output tensors, `ACDNN_DATA_BF16`, `ACDNN_SOFTMAX_ACCURATE`, and `ACDNN_SOFTMAX_MODE_CHANNEL`, with host FP32 scaling factors `alpha=1` and `beta=0`. Each N/H/W position is normalized independently across C on an explicit non-default stream.

N=2, H=3, and W=5 are fixed. The following cases run serially:

| C | Input | Element Count |
| --- | --- | --- |
| 1 | Constant -3.25; output must be 1 | 30 |
| 17 | Deterministic nonuniform FP32 input, quantized before computation | 510 |
| 257 | Deterministic large positive/negative values in [-160,160] | 7710 |

`softmax_reference.h` stores BF16 bit patterns as `uint16_t` and uses `memcpy` for bit conversion. FP32→BF16 conversion uses round-to-nearest, ties-to-even. The CPU reference reconstructs FP32 values from the actual uploaded BF16 inputs, then computes stable softmax by subtracting the channel maximum and accumulating in double precision. Unquantized inputs must not be used as the reference.

Host input/output use page-locked buffers. H2D initialization, BF16 NaN sentinel filling, softmax, and D2H copies are queued on the same stream, avoiding ordering assumptions between nonblocking and default streams. `beta=0` must not read the old output. After synchronization, the sample checks finiteness, nonnegativity, elementwise `abs_error <= 5e-4 + 1e-2*abs(reference)`, and channel probability-sum error `<=1e-2` at each N/H/W position. It prints the maximum absolute error, maximum probability-sum error, invalid-value count, and mismatch count. API errors or numerical failures return a nonzero exit code and stop execution; tolerances are not relaxed, and failures are not automatically retried or silently skipped.

## Building and Running

Linux, x86_64; source targets ppu001 and ppu0015.

Configure CMake and build the target:

```bash
cmake -S . -B /path/to/build221 -DHGGCToolkit_ROOT=/path/to/sdk221/PPU_SDK -DCMAKE_HG_COMPILER=/path/to/sdk221/PPU_SDK/bin/hgcc -DCMAKE_HG_ARCHITECTURES=ppu_10
cmake --build /path/to/build221 --target acdnn_softmax_bf16 --parallel 2
```

Run from the target directory:

```bash
./acdnn_softmax_bf16 -device=0
```

## APIs Involved

### ACDNN API

acdnnCreate, acdnnDestroy, acdnnSetStream, acdnnCreateTensorDescriptor, acdnnSetTensor4dDescriptor, acdnnDestroyTensorDescriptor, acdnnSoftmaxForward

### HGGC Runtime API

hggcMalloc, hggcFree, hggcMallocHost, hggcFreeHost, hggcMemcpyAsync, hggcMemsetAsync, hggcStreamCreateWithFlags, hggcStreamSynchronize, hggcStreamDestroy

## Dependencies for Build/Run

HGGC::toolkit, HGGC::acdnn

## Prerequisites

Please download and install the T-Head SAIL toolkit for your platform. Requires SAIL 2.2.1 or later.
