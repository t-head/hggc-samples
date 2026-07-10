/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * convolution_separable — Host driver
 *
 * Generates a Gaussian kernel, applies separable row+column convolution on
 * the PPU, validates against a CPU reference, and reports throughput.
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <hggc_runtime.h>
#include <helper_hggc.h>
#include <helper_functions.h>

#include "convolutionSeparable_common.h"

namespace {

constexpr int kImageW    = 2048;
constexpr int kImageH    = 2048;
constexpr int kWarmup    = 2;
constexpr int kIters     = 16;
constexpr float kSigma   = 3.0f;  // Gaussian sigma

/// Build a normalized 1-D Gaussian kernel of given radius and sigma.
void make_gaussian_kernel(float *kernel, int radius, float sigma)
{
    float sum = 0.0f;
    for (int i = -radius; i <= radius; ++i) {
        float val = expf(-0.5f * (i * i) / (sigma * sigma));
        kernel[radius + i] = val;
        sum += val;
    }
    // Normalize.
    for (int i = 0; i < 2 * radius + 1; ++i) {
        kernel[i] /= sum;
    }
}

}  // namespace

int main(int argc, char **argv)
{
    printf("[convolution_separable] Gaussian blur via separable convolution\n\n");

    findHggcDevice(argc, (const char **)argv);

    const int pixels = kImageW * kImageH;
    const size_t bytes = sizeof(float) * pixels;

    // ---- Generate Gaussian kernel ----
    float h_kernel[KERNEL_LENGTH];
    make_gaussian_kernel(h_kernel, KERNEL_RADIUS, kSigma);
    printf("  Kernel: Gaussian  sigma=%.1f  radius=%d  length=%d\n", kSigma, KERNEL_RADIUS, KERNEL_LENGTH);
    printf("  Image : %d x %d  (%.1f MP)\n\n", kImageW, kImageH, pixels * 1e-6f);

    // ---- Allocate host buffers ----
    float *h_input     = static_cast<float *>(malloc(bytes));
    float *h_buf       = static_cast<float *>(malloc(bytes));
    float *h_cpu_out   = static_cast<float *>(malloc(bytes));
    float *h_ppu_out   = static_cast<float *>(malloc(bytes));

    // Deterministic input.
    for (int i = 0; i < pixels; ++i) {
        h_input[i] = static_cast<float>((i * 17 + 31) % 256) / 255.0f;
    }

    // ---- Allocate device buffers ----
    float *d_input  = nullptr;
    float *d_buf    = nullptr;
    float *d_output = nullptr;
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&d_input),  bytes));
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&d_buf),    bytes));
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&d_output), bytes));
    checkHggcErrors(hggcMemcpy(d_input, h_input, bytes, hggcMemcpyHostToDevice));

    upload_kernel_weights(h_kernel);

    // ---- PPU convolution (warmup + timed) ----
    HggcTimer timer;

    for (int i = -kWarmup; i < kIters; ++i) {
        if (i == 0) {
            checkHggcErrors(hggcDeviceSynchronize());
            timer.start();
        }
        convolution_row_ppu(d_buf, d_input, kImageW, kImageH);
        convolution_col_ppu(d_output, d_buf, kImageW, kImageH);
    }
    checkHggcErrors(hggcDeviceSynchronize());
    timer.stop();

    const double elapsed_ms = timer.elapsed();
    const double sec_per_iter = (elapsed_ms * 1e-3) / kIters;
    const double mpix_per_sec = (pixels * 1e-6) / sec_per_iter;

    printf("  PPU throughput : %.2f MPixels/sec  (%.3f ms/iter, %d iters)\n\n",
           mpix_per_sec, elapsed_ms / kIters, kIters);

    // ---- CPU reference ----
    printf("  Running CPU reference...\n");
    convolution_row_cpu(h_buf, h_input, h_kernel, kImageW, kImageH, KERNEL_RADIUS);
    convolution_col_cpu(h_cpu_out, h_buf, h_kernel, kImageW, kImageH, KERNEL_RADIUS);

    // ---- Validate ----
    checkHggcErrors(hggcMemcpy(h_ppu_out, d_output, bytes, hggcMemcpyDeviceToHost));

    double delta_sq = 0.0, ref_sq = 0.0;
    for (int i = 0; i < pixels; ++i) {
        double diff = h_ppu_out[i] - h_cpu_out[i];
        delta_sq += diff * diff;
        ref_sq += static_cast<double>(h_cpu_out[i]) * h_cpu_out[i];
    }
    const double l2_norm = sqrt(delta_sq / ref_sq);
    printf("  Relative L2 norm: %E\n", l2_norm);

    // ---- Cleanup ----
    free(h_input);
    free(h_buf);
    free(h_cpu_out);
    free(h_ppu_out);
    checkHggcErrors(hggcFree(d_input));
    checkHggcErrors(hggcFree(d_buf));
    checkHggcErrors(hggcFree(d_output));

    const bool pass = (l2_norm < 1e-6);
    printf("\n  Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
