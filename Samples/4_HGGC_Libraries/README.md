# 4. HGGC Libraries

### acsolver_dn_lu_factorization
This sample uses acsolverDn for LU factorization with partial pivoting, extracts and displays L, U factors and the pivot sequence, verifies P*A = L*U consistency, and solves a linear system.

### acdnn_conv_activation
This sample uses the ACDNN library to build a CNN forward computation pipeline: 2D convolution + ReLU activation, with host-side result verification.

### hg_jpeg
This sample demonstrates single-image and batch JPEG image decoding using the HGGCJPEG library.

### hg_jpeg_encoder
This sample demonstrates single-image JPEG encoding using the HGGCJPEG library.

### jit_lto
This sample demonstrates the available JIT LTO APIs.

### simple_acblas_lu
This sample demonstrates matrix LU factorization using the ACBLAS API acblasDgetrfBatched().

### simple_acfft
This sample uses ACFFT to compute 1D convolution — transforms both signal and filter to the frequency domain, multiplies, then inverse-transforms back to the time domain; also demonstrates plan creation using both simple and advanced ACFFT APIs.
