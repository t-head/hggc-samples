/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * runtime_compile.cpp -- Tiled GEMM with bias via HGRTC JIT
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <hggc_runtime.h>
#include <hgrtc_helper.h>
#include <helper_functions.h>

namespace {

constexpr int    kTileSize     = 32;
constexpr int    kIterations   = 100;
constexpr float  kAlpha        = 1.5f;
constexpr float  kBeta         = 0.5f;
constexpr double kRelTolerance = 1.0e-4;

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

const char *kernel_name(int tile) { return tile == 16 ? "gemm_bias_b16" : "gemm_bias_b32"; }

}  // namespace

int main(int argc, char **argv)
{
    printf("[matrix_mul_runtime_compile] Tiled GEMM with Bias (JIT)\n\n");

    int wA = 4 * kTileSize, hA = 4 * kTileSize, wB = 6 * kTileSize;
    if (hasArg(argc, const_cast<const char **>(argv), "wA"))
        wA = getArgInt(argc, const_cast<const char **>(argv), "wA");
    if (hasArg(argc, const_cast<const char **>(argv), "hA"))
        hA = getArgInt(argc, const_cast<const char **>(argv), "hA");
    if (hasArg(argc, const_cast<const char **>(argv), "wB"))
        wB = getArgInt(argc, const_cast<const char **>(argv), "wB");

    printf("  A(%dx%d) * B(%dx%d) = C(%dx%d)\n", hA, wA, wA, wB, hA, wB);
    printf("  alpha=%.2f, beta=%.2f\n\n", kAlpha, kBeta);

    size_t n_a = (size_t)hA * wA, n_b = (size_t)wA * wB, n_c = (size_t)hA * wB;
    size_t sz_a = n_a * sizeof(float), sz_b = n_b * sizeof(float), sz_c = n_c * sizeof(float);

    std::vector<float> h_A(n_a), h_B(n_b), h_C(n_c), h_ref(n_c);
    random_init(h_A.data(), n_a, 42);
    random_init(h_B.data(), n_b, 123);
    random_init(h_C.data(), n_c, 456);
    cpu_gemm_ref(h_A.data(), h_B.data(), h_C.data(), h_ref.data(), hA, wA, wB, kAlpha, kBeta);

    char  *hgbin = nullptr;
    size_t hgbin_sz = 0;
    char  *kpath = findSampleAsset("runtime_compile_kernel.hg", argv[0]);
    compileFileToHGBIN(kpath, argc, argv, &hgbin, &hgbin_sz, 1);
    HGmodule module = loadHGBIN(hgbin, argc, argv);

    HGfunction func;
    checkHggcErrors(hgModuleGetFunction(&func, module, kernel_name(kTileSize)));

    HGdeviceptr dA, dB, dC;
    checkHggcErrors(hgMemAlloc(&dA, sz_a));
    checkHggcErrors(hgMemAlloc(&dB, sz_b));
    checkHggcErrors(hgMemAlloc(&dC, sz_c));
    checkHggcErrors(hgMemcpyHtoD(dA, h_A.data(), sz_a));
    checkHggcErrors(hgMemcpyHtoD(dB, h_B.data(), sz_b));
    checkHggcErrors(hgMemcpyHtoD(dC, h_C.data(), sz_c));

    dim3 block(kTileSize, kTileSize);
    dim3 grid(wB / kTileSize, hA / kTileSize);
    float alpha = kAlpha, beta = kBeta;
    void *args[] = {&dC, &dA, &dB, &wA, &wB, &alpha, &beta};

    printf("  Running %d iterations...\n", kIterations);
    for (int i = 0; i < kIterations; i++) {
        checkHggcErrors(hgLaunchKernel(func, grid.x, grid.y, 1,
                                       block.x, block.y, 1, 0, 0, args, 0));
        checkHggcErrors(hgCtxSynchronize());
    }

    /* Verify */
    checkHggcErrors(hgMemcpyHtoD(dC, h_C.data(), sz_c));
    checkHggcErrors(hgLaunchKernel(func, grid.x, grid.y, 1,
                                   block.x, block.y, 1, 0, 0, args, 0));
    checkHggcErrors(hgCtxSynchronize());

    checkHggcErrors(hgMemcpyDtoH(h_C.data(), dC, sz_c));

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
    return mismatches == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
