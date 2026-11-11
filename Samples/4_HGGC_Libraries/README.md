# 4. HGGC Libraries

### acsolver_dn_lu_factorization
This sample uses acsolverDn for LU factorization with partial pivoting, extracts and displays L, U factors and the pivot sequence, verifies P*A = L*U consistency, and solves a linear system.

### acdnn_conv_activation
This sample uses the ACDNN library to build a CNN forward computation pipeline: 2D convolution + ReLU activation, with host-side result verification.

### acdnn_softmax_bf16
Available only in the SAIL 2.2.1 branch, this BF16 softmax sample uses NCHW, ACCURATE/CHANNEL, and C=1/17/257. It computes a stable CPU reference from the actual quantized BF16 inputs and checks elementwise errors and channel probability sums. Requires SAIL 2.2.1 and the corresponding ACDNN library. CPU-reference tests do not substitute for library execution. See the [sample README](acdnn_softmax_bf16/README.md) for details.

### hg_jpeg
Decodes JPEG images in batches with HGGCJPEG and computes per-channel pixel statistics on the device, demonstrating library/kernel interoperability.

### hg_jpeg_encoder
Decodes JPEG images and re-encodes them at quality levels 30/50/70/90, reports the quality-size tradeoff, and saves the highest-quality output.

### jit_lto
Demonstrates runtime JIT linking of separately compiled LTOIR and HGBIN modules with libhgJitLink, then loads and executes the linked Saxpy kernel.

### simple_acblas_lu
Builds LU factorization with partial pivoting from BLAS Level 1/2/3 primitives, verifies L*U = P*A, and solves a linear system with triangular solves.

### simple_acfft
Uses ACFFT for spectrum analysis and frequency-domain filtering: transforms a composite signal, detects dominant frequencies, applies a low-pass filter, reconstructs with inverse FFT, and compares with a CPU DFT reference.
