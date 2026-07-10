/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * ---------------------------------------------------------------------------
 * ACDNN Convolution + ReLU Pipeline Demo
 * ---------------------------------------------------------------------------
 * This sample demonstrates building a classic CNN forward computation pipeline using the ACDNN library:
 *   1. Create input tensor (NCHW format, FP32) initialized with random data
 *   2. Create convolution filter (KCRS format) and perform 2D convolution forward computation
 *   3. Apply ReLU activation to the convolution output
 *   4. Copy results back to host and print
 *   5. Verify PPU results against a naive loop-based reference convolution+ReLU on host
 *
 * Batch/channel/spatial/kernel sizes can be adjusted via command-line arguments.
 * ---------------------------------------------------------------------------
 */
#include <hggc_runtime.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <acdnn.h>

#include "helper_hggc.h"

/* ACDNN error-checking macro (acdnnStatus_t differs from hggcError_t) */
#define checkAcdnnErrors(call)                                              \
    do {                                                                    \
        acdnnStatus_t status = (call);                                      \
        if (status != ACDNN_STATUS_SUCCESS) {                               \
            fprintf(stderr, "ACDNN error at %s:%d code=%d\n",              \
                    __FILE__, __LINE__, (int)status);                       \
            exit(EXIT_FAILURE);                                             \
        }                                                                   \
    } while (0)

/* ------------------------------------------------------------------ */
/*  Default parameters                                                            */
/* ------------------------------------------------------------------ */
#define DEF_N 1    /* batch size   */
#define DEF_C 4    /* input chan   */
#define DEF_H 8    /* height       */
#define DEF_W 8    /* width        */
#define DEF_K 8    /* out channels */
#define DEF_R 3    /* kernel size  */
#define DEF_S 3    /* kernel size  */

/* ------------------------------------------------------------------ */
/*  Helper functions                                                           */
/* ------------------------------------------------------------------ */

static double wallTime(void)
{
#if defined(__linux__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1.0e-6;
#else
    return (double)clock() / CLOCKS_PER_SEC;
#endif
}

/* Print first few elements of an NCHW tensor */
static void printTensor(const char *name, const float *data,
                        int n, int c, int h, int w, int maxElems)
{
    int total = n * c * h * w;
    int limit = total < maxElems ? total : maxElems;
    printf("%s [%dx%dx%dx%d] (showing first %d of %d elements):\n",
           name, n, c, h, w, limit, total);
    int idx = 0;
    for (int in = 0; in < n && idx < limit; in++) {
        for (int ic = 0; ic < c && idx < limit; ic++) {
            printf("  [n=%d, c=%d]: ", in, ic);
            for (int ih = 0; ih < h && idx < limit; ih++) {
                for (int iw = 0; iw < w && idx < limit; iw++) {
                    printf("%8.4f ", data[((in * c + ic) * h + ih) * w + iw]);
                    idx++;
                }
            }
            printf("\n");
        }
    }
    printf("\n");
}

/*
 * Host-side reference implementation: 2D convolution (cross-correlation, NCHW, KCRS)
 * pad_h, pad_w: zero-padding; stride_u, stride_v: stride
 */
static void hostConvReLU(const float *x, const float *filter,
                         float *y, int N, int C, int H, int W,
                         int K, int R, int S,
                         int pad_h, int pad_w, int stride_u, int stride_v,
                         int applyRelu)
{
    int outH = (H + 2 * pad_h - R) / stride_u + 1;
    int outW = (W + 2 * pad_w - S) / stride_v + 1;

    for (int n = 0; n < N; n++) {
        for (int k = 0; k < K; k++) {
            for (int oh = 0; oh < outH; oh++) {
                for (int ow = 0; ow < outW; ow++) {
                    float sum = 0.0f;
                    for (int c = 0; c < C; c++) {
                        for (int r = 0; r < R; r++) {
                            for (int s = 0; s < S; s++) {
                                int ih = oh * stride_u - pad_h + r;
                                int iw = ow * stride_v - pad_w + s;
                                if (ih >= 0 && ih < H && iw >= 0 && iw < W) {
                                    float xv = x[((n * C + c) * H + ih) * W + iw];
                                    float wv = filter[((k * C + c) * R + r) * S + s];
                                    sum += xv * wv;
                                }
                            }
                        }
                    }
                    if (applyRelu && sum < 0.0f)
                        sum = 0.0f;
                    y[((n * K + k) * outH + oh) * outW + ow] = sum;
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Command-line help                                                          */
/* ------------------------------------------------------------------ */
static void showUsage(void)
{
    printf("Usage: acdnn_conv_activation [options]\n");
    printf("  -N=<int>   batch size      (default %d)\n", DEF_N);
    printf("  -C=<int>   input channels  (default %d)\n", DEF_C);
    printf("  -H=<int>   height          (default %d)\n", DEF_H);
    printf("  -W=<int>   width           (default %d)\n", DEF_W);
    printf("  -K=<int>   output channels (default %d)\n", DEF_K);
    printf("  -R=<int>   kernel height    (default %d)\n", DEF_R);
    printf("  -S=<int>   kernel width     (default %d)\n", DEF_S);
    printf("  -device=<id>  Specify PPU device\n");
    printf("  -h            Show this help\n");
    exit(0);
}

/* ------------------------------------------------------------------ */
/*  Main program                                                              */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    /* ---- Parse command-line arguments ---- */
    if (hasArg(argc, (const char **)argv, "h")) {
        showUsage();
    }

    int N = DEF_N, C = DEF_C, H = DEF_H, W = DEF_W;
    int K = DEF_K, R = DEF_R, S = DEF_S;

    if (hasArg(argc, (const char **)argv, "N")) N = getArgInt(argc, (const char **)argv, "N");
    if (hasArg(argc, (const char **)argv, "C")) C = getArgInt(argc, (const char **)argv, "C");
    if (hasArg(argc, (const char **)argv, "H")) H = getArgInt(argc, (const char **)argv, "H");
    if (hasArg(argc, (const char **)argv, "W")) W = getArgInt(argc, (const char **)argv, "W");
    if (hasArg(argc, (const char **)argv, "K")) K = getArgInt(argc, (const char **)argv, "K");
    if (hasArg(argc, (const char **)argv, "R")) R = getArgInt(argc, (const char **)argv, "R");
    if (hasArg(argc, (const char **)argv, "S")) S = getArgInt(argc, (const char **)argv, "S");

    /* Output size equals input size when padding=1, stride=1 */
    int pad_h = 1, pad_w = 1, stride_u = 1, stride_v = 1;
    int outH = (H + 2 * pad_h - R) / stride_u + 1;
    int outW = (W + 2 * pad_w - S) / stride_v + 1;

    findHggcDevice(argc, (const char **)argv);

    printf("============================================================\n");
    printf("  ACDNN Convolution + ReLU Pipeline Demo\n");
    printf("  Input:  N=%d C=%d H=%d W=%d\n", N, C, H, W);
    printf("  Filter: K=%d C=%d R=%d S=%d\n", K, C, R, S);
    printf("  Output: N=%d K=%d H=%d W=%d\n", N, K, outH, outW);
    printf("  padding=%dx%d, stride=%dx%d\n", pad_h, pad_w, stride_u, stride_v);
    printf("============================================================\n\n");

    /* ---- Step 1: Allocate and initialize data on host ---- */
    printf("Step 1: Initialize input and filter data\n");

    size_t xSize = (size_t)N * C * H * W * sizeof(float);
    size_t fSize = (size_t)K * C * R * S * sizeof(float);
    size_t ySize = (size_t)N * K * outH * outW * sizeof(float);

    float *h_x = (float *)malloc(xSize);
    float *h_f = (float *)malloc(fSize);
    float *h_y = (float *)malloc(ySize);
    float *h_y_ref = (float *)malloc(ySize);

    assert(h_x && h_f && h_y && h_y_ref);

    srand(42);
    for (size_t i = 0; i < N * C * H * W; i++)
        h_x[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    for (size_t i = 0; i < K * C * R * S; i++)
        h_f[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

    if (N * C * H * W <= 256)
        printTensor("Input X", h_x, N, C, H, W, 256);
    if (K * C * R * S <= 256)
        printTensor("Filter W", h_f, K, C, R, S, 256);

    /* ---- Step 2: Create ACDNN handle and stream ---- */
    printf("Step 2: Create ACDNN handle\n");

    acdnnHandle_t acdnnHandle = NULL;
    hggcStream_t stream = NULL;

    checkAcdnnErrors(acdnnCreate(&acdnnHandle));
    checkHggcErrors(hggcStreamCreate(&stream));
    checkAcdnnErrors(acdnnSetStream(acdnnHandle, stream));

    /* ---- Step 3: Device memory allocation and data transfer ---- */
    printf("Step 3: Allocate PPU memory and transfer data\n");

    float *d_x = NULL, *d_f = NULL, *d_y = NULL, *d_y_act = NULL;

    checkHggcErrors(hggcMalloc((void **)&d_x, xSize));
    checkHggcErrors(hggcMalloc((void **)&d_f, fSize));
    checkHggcErrors(hggcMalloc((void **)&d_y, ySize));
    checkHggcErrors(hggcMalloc((void **)&d_y_act, ySize));

    checkHggcErrors(hggcMemcpy(d_x, h_x, xSize, hggcMemcpyHostToDevice));
    checkHggcErrors(hggcMemcpy(d_f, h_f, fSize, hggcMemcpyHostToDevice));

    /* ---- Step 4: Create descriptors ---- */
    printf("Step 4: Create tensor, filter, and convolution descriptors\n");

    acdnnTensorDescriptor_t xDesc = NULL, yDesc = NULL;
    acdnnFilterDescriptor_t fDesc = NULL;
    acdnnConvolutionDescriptor_t convDesc = NULL;
    acdnnActivationDescriptor_t actDesc = NULL;

    /* Input tensor descriptor: NCHW, FP32 */
    checkAcdnnErrors(acdnnCreateTensorDescriptor(&xDesc));
    checkAcdnnErrors(acdnnSetTensor4dDescriptor(xDesc, ACDNN_TENSOR_NCHW, ACDNN_DATA_FLOAT,
                                                N, C, H, W));

    /* Output tensor descriptor: NCHW, FP32 */
    checkAcdnnErrors(acdnnCreateTensorDescriptor(&yDesc));
    checkAcdnnErrors(acdnnSetTensor4dDescriptor(yDesc, ACDNN_TENSOR_NCHW, ACDNN_DATA_FLOAT,
                                                N, K, outH, outW));

    /* Filter descriptor: KCRS, FP32 */
    checkAcdnnErrors(acdnnCreateFilterDescriptor(&fDesc));
    checkAcdnnErrors(acdnnSetFilter4dDescriptor(fDesc, ACDNN_DATA_FLOAT, ACDNN_TENSOR_KCRS,
                                                K, C, R, S));

    /* Convolution descriptor: padding=1, stride=1, dilation=1, cross-correlation */
    checkAcdnnErrors(acdnnCreateConvolutionDescriptor(&convDesc));
    checkAcdnnErrors(acdnnSetConvolution2dDescriptor(convDesc,
                                                    pad_h, pad_w,
                                                    stride_u, stride_v,
                                                    1, 1,  /* dilation_h, dilation_w */
                                                    ACDNN_CROSS_CORRELATION,
                                                    ACDNN_DATA_FLOAT));

    /* Activation descriptor: ReLU */
    checkAcdnnErrors(acdnnCreateActivationDescriptor(&actDesc));
    checkAcdnnErrors(acdnnSetActivationDescriptor(actDesc,
                                                 ACDNN_ACTIVATION_RELU,
                                                 ACDNN_NOT_PROPAGATE_NAN,
                                                 0.0));

    /* ---- Step 5: Query workspace and perform convolution ---- */
    printf("Step 5: Perform convolution forward (acdnnConvolutionForward)\n");

    /* Query workspace size */
    size_t workspaceSize = 0;
    checkAcdnnErrors(acdnnGetConvolutionForwardWorkspaceSize(acdnnHandle,
                                                             xDesc, fDesc, convDesc, yDesc,
                                                             ACDNN_CONVOLUTION_FWD_ALGO_IMPLICIT_GEMM,
                                                             &workspaceSize));
    printf("       Convolution algorithm = IMPLICIT_GEMM\n");
    printf("       Workspace = %zu bytes\n", workspaceSize);

    void *d_workspace = NULL;
    if (workspaceSize > 0)
        checkHggcErrors(hggcMalloc(&d_workspace, workspaceSize));

    const float alpha = 1.0f, beta = 0.0f;

    double t0 = wallTime();

    checkAcdnnErrors(acdnnConvolutionForward(acdnnHandle,
                                              &alpha,
                                              xDesc, d_x,
                                              fDesc, d_f,
                                              convDesc,
                                              ACDNN_CONVOLUTION_FWD_ALGO_IMPLICIT_GEMM,
                                              d_workspace, workspaceSize,
                                              &beta,
                                              yDesc, d_y));

    checkHggcErrors(hggcDeviceSynchronize());
    double t1 = wallTime();
    printf("       Convolution time = %.6f s\n\n", t1 - t0);

    /* ---- Step 6: Perform ReLU activation ---- */
    printf("Step 6: Perform ReLU activation (acdnnActivationForward)\n");

    t0 = wallTime();

    checkAcdnnErrors(acdnnActivationForward(acdnnHandle,
                                            actDesc,
                                            &alpha,
                                            yDesc, d_y,
                                            &beta,
                                            yDesc, d_y_act));

    checkHggcErrors(hggcDeviceSynchronize());
    t1 = wallTime();
    printf("       ReLU time = %.6f s\n\n", t1 - t0);

    /* ---- Step 7: Copy results back and print ---- */
    printf("Step 7: Copy results back\n");

    checkHggcErrors(hggcMemcpy(h_y, d_y_act, ySize, hggcMemcpyDeviceToHost));

    if (N * K * outH * outW <= 256)
        printTensor("Output (Conv+ReLU)", h_y, N, K, outH, outW, 256);

    /* ---- Step 8: Host-side reference verification ---- */
    printf("Step 8: Host-side reference verification\n");

    hostConvReLU(h_x, h_f, h_y_ref,
                 N, C, H, W, K, R, S,
                 pad_h, pad_w, stride_u, stride_v, /*applyRelu=*/1);

    /* Compute maximum error */
    float maxErr = 0.0f;
    int totalElems = N * K * outH * outW;
    for (int i = 0; i < totalElems; i++) {
        float diff = fabsf(h_y[i] - h_y_ref[i]);
        if (diff > maxErr) maxErr = diff;
    }

    printf("       Total elements = %d\n", totalElems);
    printf("       Max absolute error = %E\n", maxErr);

    if (maxErr < 2e-3f) {
        printf("       ✓ Verification passed (threshold 2e-3, normal precision range for GEMM convolution)\n\n");
    } else {
        printf("       ✗ Warning: error exceeds 2e-3\n\n");
    }

    /* ---- Summary ---- */
    printf("============================================================\n");
    printf("  Demo complete\n");
    printf("  Input:  %dx%dx%dx%d\n", N, C, H, W);
    printf("  Filter: %dx%dx%dx%d\n", K, C, R, S);
    printf("  Output: %dx%dx%dx%d\n", N, K, outH, outW);
    printf("  Max error: %E\n", maxErr);
    printf("============================================================\n");

    /* ---- Release resources ---- */
    if (d_workspace) checkHggcErrors(hggcFree(d_workspace));
    if (d_x)         checkHggcErrors(hggcFree(d_x));
    if (d_f)         checkHggcErrors(hggcFree(d_f));
    if (d_y)         checkHggcErrors(hggcFree(d_y));
    if (d_y_act)     checkHggcErrors(hggcFree(d_y_act));

    if (actDesc)  checkAcdnnErrors(acdnnDestroyActivationDescriptor(actDesc));
    if (convDesc) checkAcdnnErrors(acdnnDestroyConvolutionDescriptor(convDesc));
    if (fDesc)    checkAcdnnErrors(acdnnDestroyFilterDescriptor(fDesc));
    if (xDesc)    checkAcdnnErrors(acdnnDestroyTensorDescriptor(xDesc));
    if (yDesc)    checkAcdnnErrors(acdnnDestroyTensorDescriptor(yDesc));

    if (acdnnHandle) checkAcdnnErrors(acdnnDestroy(acdnnHandle));
    if (stream)       checkHggcErrors(hggcStreamDestroy(stream));

    free(h_x);
    free(h_f);
    free(h_y);
    free(h_y_ref);

    return 0;
}
