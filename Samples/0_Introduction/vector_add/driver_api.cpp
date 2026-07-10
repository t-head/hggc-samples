/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * driver_api.cpp -- Weighted vector combination via Driver API
 */

#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include <hggc.h>
#include <helper_hggc_drvapi.h>
#include <helper_functions.h>

namespace {

constexpr int   kNumElements = 65536;
constexpr int   kBlockDim    = 256;
constexpr float kAlpha       = 2.5f;
constexpr float kBeta        = -1.5f;
constexpr float kGamma       = 100.0f;
constexpr float kRelTol      = 1e-5f;

}  // namespace

#ifndef FATBIN_FILE
#define FATBIN_FILE "driver_api_kernel64.fatbin"
#endif

static HGdevice    g_dev;
static HGcontext   g_ctx;
static HGmodule    g_module;
static HGfunction  g_func;
static float      *g_h_x = nullptr;
static float      *g_h_y = nullptr;
static float      *g_h_out = nullptr;
static HGdeviceptr g_d_x  = 0;
static HGdeviceptr g_d_y  = 0;
static HGdeviceptr g_d_out = 0;

static int  cleanup();
static void fill_sinusoidal(float *data, int n, float phase);

int main(int argc, char **argv)
{
    printf("[vector_add_driver_api] Weighted Linear Combination (Driver API)\n");
    const size_t bytes = static_cast<size_t>(kNumElements) * sizeof(float);

    checkHggcErrors(hgInit(0));
    g_dev = findHggcDeviceDRV(argc, (const char **)argv);
#if HGGC_VERSION >= 13000
    checkHggcErrors(hgCtxCreate(&g_ctx, NULL, 0, g_dev));
#else
    checkHggcErrors(hgCtxCreate(&g_ctx, 0, g_dev));
#endif

    std::string module_path;
    std::ostringstream fatbin_stream;
    if (!findFatbinPath(FATBIN_FILE, module_path, argv, fatbin_stream)) {
        return EXIT_FAILURE;
    }
    if (fatbin_stream.str().empty()) {
        fprintf(stderr, "  Fatbin file empty.\n");
        return EXIT_FAILURE;
    }

    checkHggcErrors(hgModuleLoadData(&g_module, fatbin_stream.str().c_str()));
    checkHggcErrors(hgModuleGetFunction(&g_func, g_module, "weighted_sum_kernel"));

    g_h_x   = static_cast<float *>(malloc(bytes));
    g_h_y   = static_cast<float *>(malloc(bytes));
    g_h_out = static_cast<float *>(malloc(bytes));
    fill_sinusoidal(g_h_x, kNumElements, 0.0f);
    fill_sinusoidal(g_h_y, kNumElements, 1.5708f);

    checkHggcErrors(hgMemAlloc(&g_d_x,   bytes));
    checkHggcErrors(hgMemAlloc(&g_d_y,   bytes));
    checkHggcErrors(hgMemAlloc(&g_d_out, bytes));
    checkHggcErrors(hgMemcpyHtoD(g_d_x, g_h_x, bytes));
    checkHggcErrors(hgMemcpyHtoD(g_d_y, g_h_y, bytes));

    int grid = (kNumElements + kBlockDim - 1) / kBlockDim;
    float alpha = kAlpha, beta = kBeta, gamma = kGamma;
    int n = kNumElements;
    void *args[] = {&g_d_x, &g_d_y, &g_d_out, &n, &alpha, &beta, &gamma};
    checkHggcErrors(hgLaunchKernel(g_func, grid, 1, 1, kBlockDim, 1, 1,
                                    0, NULL, args, NULL));
    checkHggcErrors(hgCtxSynchronize());

    checkHggcErrors(hgMemcpyDtoH(g_h_out, g_d_out, bytes));

    int mismatches = 0;
    for (int i = 0; i < kNumElements; i++) {
        float expected = kAlpha * g_h_x[i] + kBeta * g_h_y[i] + kGamma;
        float diff = g_h_out[i] - expected;
        if (diff < 0) diff = -diff;
        float denom = fabsf(expected);
        if (denom < 1.0f) denom = 1.0f;
        if (diff / denom > kRelTol) mismatches++;
    }

    cleanup();
    printf("  Mismatches: %d / %d\n", mismatches, kNumElements);
    printf("  Result: %s\n", mismatches == 0 ? "PASS" : "FAIL");
    return mismatches == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int cleanup()
{
    checkHggcErrors(hgMemFree(g_d_x));
    checkHggcErrors(hgMemFree(g_d_y));
    checkHggcErrors(hgMemFree(g_d_out));
    if (g_h_x)   free(g_h_x);
    if (g_h_y)   free(g_h_y);
    if (g_h_out) free(g_h_out);
    checkHggcErrors(hgCtxDestroy(g_ctx));
    return EXIT_SUCCESS;
}

static void fill_sinusoidal(float *data, int n, float phase)
{
    for (int i = 0; i < n; i++)
        data[i] = sinf(static_cast<float>(i) * 0.01f + phase);
}
