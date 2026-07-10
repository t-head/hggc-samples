/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * runtime_compile.cpp -- Weighted vector combination via HGRTC JIT
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <hggc.h>
#include <hggc_runtime.h>
#include <helper_functions.h>
#include <hgrtc_helper.h>

namespace {

constexpr int   kNumElements = 65536;
constexpr int   kBlockDim    = 256;
constexpr float kAlpha       = 2.5f;
constexpr float kBeta        = -1.5f;
constexpr float kGamma       = 100.0f;
constexpr float kRelTol      = 1e-5f;

}  // namespace

int main(int argc, char **argv)
{
    char  *hgbin      = nullptr;
    size_t hgbin_size = 0;
    char  *kernel_path = findSampleAsset("runtime_compile_kernel.hg", argv[0]);
    compileFileToHGBIN(kernel_path, argc, argv, &hgbin, &hgbin_size, 0);

    HGmodule   module = loadHGBIN(hgbin, argc, argv);
    HGfunction func;
    checkHggcErrors(hgModuleGetFunction(&func, module, "weighted_sum_kernel"));

    const size_t bytes = static_cast<size_t>(kNumElements) * sizeof(float);
    printf("[vector_add_runtime_compile] Weighted Linear Combination (JIT)\n");
    printf("  N = %d, alpha = %.2f, beta = %.2f, gamma = %.2f\n\n",
           kNumElements, kAlpha, kBeta, kGamma);

    float *h_x   = static_cast<float *>(malloc(bytes));
    float *h_y   = static_cast<float *>(malloc(bytes));
    float *h_out = static_cast<float *>(malloc(bytes));

    for (int i = 0; i < kNumElements; i++) {
        h_x[i] = sinf(static_cast<float>(i) * 0.01f);
        h_y[i] = cosf(static_cast<float>(i) * 0.01f);
    }

    HGdeviceptr d_x, d_y, d_out;
    checkHggcErrors(hgMemAlloc(&d_x,   bytes));
    checkHggcErrors(hgMemAlloc(&d_y,   bytes));
    checkHggcErrors(hgMemAlloc(&d_out, bytes));

    checkHggcErrors(hgMemcpyHtoD(d_x, h_x, bytes));
    checkHggcErrors(hgMemcpyHtoD(d_y, h_y, bytes));

    int grid = (kNumElements + kBlockDim - 1) / kBlockDim;
    float alpha = kAlpha, beta = kBeta, gamma = kGamma;
    int n = kNumElements;
    void *args[] = {&d_x, &d_y, &d_out, &n, &alpha, &beta, &gamma};

    dim3 block(kBlockDim, 1, 1);
    dim3 gdim(grid, 1, 1);
    checkHggcErrors(hgLaunchKernel(func,
                                   gdim.x, gdim.y, gdim.z,
                                   block.x, block.y, block.z,
                                   0, 0, args, 0));
    checkHggcErrors(hgCtxSynchronize());

    checkHggcErrors(hgMemcpyDtoH(h_out, d_out, bytes));

    int mismatches = 0;
    for (int i = 0; i < kNumElements; i++) {
        float expected = kAlpha * h_x[i] + kBeta * h_y[i] + kGamma;
        float diff = h_out[i] - expected;
        if (diff < 0) diff = -diff;
        float denom = fabsf(expected);
        if (denom < 1.0f) denom = 1.0f;
        if (diff / denom > kRelTol) mismatches++;
    }

    printf("  Mismatches: %d / %d\n", mismatches, kNumElements);
    printf("  Result: %s\n", mismatches == 0 ? "PASS" : "FAIL");

    checkHggcErrors(hgMemFree(d_x));
    checkHggcErrors(hgMemFree(d_y));
    checkHggcErrors(hgMemFree(d_out));
    free(h_x);
    free(h_y);
    free(h_out);

    return mismatches == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
