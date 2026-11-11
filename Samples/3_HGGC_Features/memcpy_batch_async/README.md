# memcpy_batch_async - Batched Asynchronous D2D Copies

## Description

Creates a non-default stream on the device selected by `-device=N` and submits six independent D2D copies of 13, 257, 1023, 4099, 8191, and 16387 bytes. After initializing deterministic inputs and destination sentinels, two attribute groups with start indices `{0, 3}` cover `[0, 3)` and `[3, 6)`. The sample synchronizes and verifies every byte.

Destination sentinels are initialized with `hggcMemsetAsync` on the same stream as the batch. A stream created with `hggcStreamNonBlocking` does not participate in the default stream's implicit synchronization; using default-stream `hggcMemset` instead could overwrite completed copies.

The batch contains no data dependencies. Copy order is not assumed, and performance improvement is not a PASS criterion. Attributes use stream-ordered source access and the actual device ID for source/destination location hints.

## Key Concepts

Batch Async Memcpy, Attribute Groups, D2D, Non-Default Stream, Byte-by-Byte Verification.

## Supported Platforms

Linux, x86_64; source targets ppu001 and ppu0015.

## HGGC Runtime API

hggcGetDeviceCount, hggcSetDevice, hggcDeviceGetAttribute, hggcStreamCreateWithFlags, hggcMalloc, hggcMemcpy, hggcMemsetAsync, hggcMemcpyBatchAsync, hggcStreamSynchronize, hggcFree, hggcStreamDestroy, hggcGetErrorString

## Runtime Signature Differences

- SAIL 2.2.1 rt2: 9 parameters, with `void **dsts`, `void **srcs`, `size_t *sizes`, and `size_t *failIdx`.
- SAIL 2.2.1 rt3: 8 parameters, with `void *const *dsts`, `const void *const *srcs`, and `const size_t *sizes`, without `failIdx`.
- Overloads are selected by the actual function-pointer type. The rt3 path constructs a separate const source-pointer array instead of casting `void **`. It does not report a nonexistent failure index on error. Stream synchronization must also be checked for asynchronous errors.

## Prerequisites

Please download and install the T-Head SAIL toolkit for your platform. Requires SAIL 2.2.0 or later.

## Building and Running

Configure CMake from the repository root, then build the `memcpy_batch_async` target. Run it with:

```bash
./memcpy_batch_async -device=0
```
