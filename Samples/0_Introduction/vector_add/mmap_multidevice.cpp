/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * mmap_multidevice.cpp -- Weighted vector combination with striped
 * multi-device VA mapping (HGGC Driver API)
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <hggc.h>
#include <helper_functions.h>
#include <helper_hggc_drvapi.h>

#include "multi_device_mmap.hpp"

namespace {

constexpr int   kNumElements = 65536;
constexpr int   kBlockDim    = 256;
constexpr float kAlpha       = 2.5f;
constexpr float kBeta        = -1.5f;
constexpr float kGamma       = 100.0f;
constexpr float kRelTol      = 1e-5f;

#ifndef FATBIN_FILE
#define FATBIN_FILE "mmap_multidevice_kernel64.fatbin"
#endif

HGdevice    g_dev    = 0;
HGcontext   g_ctx    = nullptr;
HGmodule    g_module = nullptr;
HGfunction  g_func   = nullptr;

float *g_h_x   = nullptr;
float *g_h_y   = nullptr;
float *g_h_out = nullptr;

HGdeviceptr g_d_x   = 0;
HGdeviceptr g_d_y   = 0;
HGdeviceptr g_d_out = 0;
size_t      g_va_bytes = 0;

void fill_sinusoidal(float *data, int n, float phase)
{
    for (int i = 0; i < n; i++)
        data[i] = sinf(static_cast<float>(i) * 0.01f + phase);
}

std::vector<HGdevice> collect_backing_devices(HGdevice primary)
{
    int total = 0;
    checkHggcErrors(hgDeviceGetCount(&total));

    std::vector<HGdevice> backing;
    backing.reserve(total);
    backing.push_back(primary);

    for (int cand = 0; cand < total; cand++) {
        if (cand == primary) continue;
        int peer_ok = 0;
        checkHggcErrors(hgDeviceCanAccessPeer(&peer_ok, primary, cand));
        if (!peer_ok) continue;
        int va_ok = 0;
        checkHggcErrors(hgDeviceGetAttribute(
            &va_ok, HG_DEVICE_ATTRIBUTE_VIRTUAL_ADDRESS_MANAGEMENT_SUPPORTED, cand));
        if (!va_ok) continue;
        backing.push_back(cand);
    }
    return backing;
}

int cleanup()
{
    checkHggcErrors(multi_device_mmap_free(g_d_x,   g_va_bytes));
    checkHggcErrors(multi_device_mmap_free(g_d_y,   g_va_bytes));
    checkHggcErrors(multi_device_mmap_free(g_d_out, g_va_bytes));
    free(g_h_x); free(g_h_y); free(g_h_out);
    g_h_x = g_h_y = g_h_out = nullptr;
    if (g_ctx) { checkHggcErrors(hgCtxDestroy(g_ctx)); g_ctx = nullptr; }
    return EXIT_SUCCESS;
}

void load_kernel_module(int argc, char **argv)
{
    std::string module_path;
    std::ostringstream fatbin_stream;
    if (!findFatbinPath(FATBIN_FILE, module_path, argv, fatbin_stream)) {
        fprintf(stderr, "  Cannot locate fatbin: %s\n", FATBIN_FILE);
        exit(EXIT_FAILURE);
    }
    std::string blob = fatbin_stream.str();
    if (blob.empty()) { fprintf(stderr, "  Fatbin empty.\n"); exit(EXIT_FAILURE); }
    checkHggcErrors(hgModuleLoadData(&g_module, blob.c_str()));
    checkHggcErrors(hgModuleGetFunction(&g_func, g_module, "weighted_sum_kernel"));
}

}  // namespace

int main(int argc, char **argv)
{
    printf("[vector_add_mmap] Weighted Combination with Multi-Device VA\n");

    const size_t bytes = static_cast<size_t>(kNumElements) * sizeof(float);

    checkHggcErrors(hgInit(0));
    g_dev = findHggcDeviceDRV(argc, const_cast<const char **>(argv));

    int va_ok = 0;
    checkHggcErrors(hgDeviceGetAttribute(
        &va_ok, HG_DEVICE_ATTRIBUTE_VIRTUAL_ADDRESS_MANAGEMENT_SUPPORTED, g_dev));
    printf("  VA management supported: %d\n", va_ok);
    if (!va_ok) { printf("  Skipping (VA not supported)\n"); return EXIT_SKIPPED; }

    std::vector<HGdevice> mapping_devices = {g_dev};
    std::vector<HGdevice> backing_devices = collect_backing_devices(g_dev);

#if HGGC_VERSION >= 13000
    checkHggcErrors(hgCtxCreate(&g_ctx, NULL, 0, g_dev));
#else
    checkHggcErrors(hgCtxCreate(&g_ctx, 0, g_dev));
#endif
    load_kernel_module(argc, argv);

    g_h_x   = static_cast<float *>(malloc(bytes));
    g_h_y   = static_cast<float *>(malloc(bytes));
    g_h_out = static_cast<float *>(malloc(bytes));
    fill_sinusoidal(g_h_x, kNumElements, 0.0f);
    fill_sinusoidal(g_h_y, kNumElements, 1.5708f);

    checkHggcErrors(multi_device_mmap_alloc(&g_d_x,   &g_va_bytes, bytes, backing_devices, mapping_devices));
    checkHggcErrors(multi_device_mmap_alloc(&g_d_y,   nullptr,     bytes, backing_devices, mapping_devices));
    checkHggcErrors(multi_device_mmap_alloc(&g_d_out, nullptr,     bytes, backing_devices, mapping_devices));

    checkHggcErrors(hgMemcpyHtoD(g_d_x, g_h_x, bytes));
    checkHggcErrors(hgMemcpyHtoD(g_d_y, g_h_y, bytes));

    int grid = (kNumElements + kBlockDim - 1) / kBlockDim;
    float alpha = kAlpha, beta = kBeta, gamma = kGamma;
    int n = kNumElements;
    void *args[] = {&g_d_x, &g_d_y, &g_d_out, &n, &alpha, &beta, &gamma};
    checkHggcErrors(hgLaunchKernel(g_func, grid, 1, 1, kBlockDim, 1, 1,
                                    0, nullptr, args, nullptr));

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
