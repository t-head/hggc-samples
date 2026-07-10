/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * driver_api.cpp -- Tiled GEMM with bias via Driver API
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
#include <helper_hggc_drvapi.h>
#include <helper_string.h>
#include <helper_timer.h>

#ifndef FATBIN_FILE
#define FATBIN_FILE "driver_api_kernel64.fatbin"
#endif

namespace {

constexpr float  kAlpha        = 1.5f;
constexpr float  kBeta         = 0.5f;
constexpr double kRelTolerance = 1.0e-4;

HGdevice  g_dev = 0;
HGcontext g_ctx = nullptr;
HGmodule  g_module = nullptr;

void random_init(float *data, int n, unsigned seed)
{
    std::srand(seed);
    for (int i = 0; i < n; i++)
        data[i] = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f;
}

void cpu_gemm_ref(const float *A, const float *B, const float *C0, float *Cref,
                  int ha, int wa, int wb, float alpha, float beta)
{
    for (int i = 0; i < ha; i++)
        for (int j = 0; j < wb; j++) {
            float s = 0.0f;
            for (int k = 0; k < wa; k++) s += A[i * wa + k] * B[k * wb + j];
            Cref[i * wb + j] = alpha * s + beta * C0[i * wb + j];
        }
}

}  // namespace

int main(int argc, char **argv)
{
    printf("[matrix_mul_driver_api] Tiled GEMM with Bias (Driver API)\n\n");

    checkHggcErrors(hgInit(0));
    g_dev = findHggcDeviceDRV(argc, const_cast<const char **>(argv));

    char dev_name[128] = {0};
    checkHggcErrors(hgDeviceGetName(dev_name, sizeof(dev_name), g_dev));
    printf("  Device: %s\n\n", dev_name);

#if HGGC_VERSION >= 13000
    checkHggcErrors(hgCtxCreate(&g_ctx, NULL, 0, g_dev));
#else
    checkHggcErrors(hgCtxCreate(&g_ctx, 0, g_dev));
#endif

    std::string module_path;
    std::ostringstream fatbin_stream;
    if (!findFatbinPath(FATBIN_FILE, module_path, argv, fatbin_stream)) {
        fprintf(stderr, "  Cannot locate fatbin\n");
        return EXIT_FAILURE;
    }
    checkHggcErrors(hgModuleLoadData(&g_module, fatbin_stream.str().c_str()));

    int tile = 32;
    const char *knames[] = {"gemm_bias_bs32", "gemm_bias_bs16", "gemm_bias_bs8"};
    HGfunction func = nullptr;
    for (int i = 0; i < 3; i++) {
        checkHggcErrors(hgModuleGetFunction(&func, g_module, knames[i]));
        int max_blocks = 0, max_threads = 0;
        checkHggcErrors(hgOccupancyMaxPotentialBlockSize(
            &max_blocks, &max_threads, func, 0,
            2 * tile * tile * sizeof(float), 0));
        if (tile * tile <= max_threads) break;
        tile /= 2;
    }
    printf("  Tile size: %d\n", tile);

    int wA = 4 * tile, hA = 4 * tile, wB = 6 * tile;
    size_t n_a = (size_t)hA * wA, n_b = (size_t)wA * wB, n_c = (size_t)hA * wB;
    size_t sz_a = n_a * 4, sz_b = n_b * 4, sz_c = n_c * 4;

    std::vector<float> h_A(n_a), h_B(n_b), h_C(n_c), h_ref(n_c);
    random_init(h_A.data(), n_a, 42);
    random_init(h_B.data(), n_b, 123);
    random_init(h_C.data(), n_c, 456);
    cpu_gemm_ref(h_A.data(), h_B.data(), h_C.data(), h_ref.data(), hA, wA, wB, kAlpha, kBeta);

    HGdeviceptr dA, dB, dC;
    checkHggcErrors(hgMemAlloc(&dA, sz_a));
    checkHggcErrors(hgMemAlloc(&dB, sz_b));
    checkHggcErrors(hgMemAlloc(&dC, sz_c));
    checkHggcErrors(hgMemcpyHtoD(dA, h_A.data(), sz_a));
    checkHggcErrors(hgMemcpyHtoD(dB, h_B.data(), sz_b));
    checkHggcErrors(hgMemcpyHtoD(dC, h_C.data(), sz_c));

    HggcTimer timer;
    timer.start();

    size_t mat_wA = wA, mat_wB = wB;
    float alpha = kAlpha, beta = kBeta;
    void *args[] = {&dC, &dA, &dB, &mat_wA, &mat_wB, &alpha, &beta};

    checkHggcErrors(hgLaunchKernel(func, wB / tile, hA / tile, 1,
                                   tile, tile, 1,
                                   2 * tile * tile * sizeof(float),
                                   nullptr, args, nullptr));
    checkHggcErrors(hgMemcpyDtoH(h_C.data(), dC, sz_c));

    timer.stop();
    printf("  Kernel time: %.3f ms\n\n", timer.elapsed());

    int mismatches = 0;
    for (size_t i = 0; i < n_c; i++) {
        float d = h_C[i] - h_ref[i];
        if (d < 0) d = -d;
        float denom = fabsf(h_ref[i]);
        if (denom < 1.0f) denom = 1.0f;
        if (d / denom > kRelTolerance) mismatches++;
    }

    printf("  Mismatches: %d / %zu\n", mismatches, n_c);
    printf("  Result: %s\n", mismatches == 0 ? "PASS" : "FAIL");

    checkHggcErrors(hgMemFree(dA));
    checkHggcErrors(hgMemFree(dB));
    checkHggcErrors(hgMemFree(dC));
    checkHggcErrors(hgCtxDestroy(g_ctx));
    return mismatches == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
