/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * inline_tix_hgrtc — TIX Instruction Showcase (HGRTC + Driver API)
 *
 * Same kernel logic as inline_tix.hg, but compiled at runtime via
 * libHGRTC.  Demonstrates that inline asm() statements survive JIT
 * compilation and produce identical results.
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <hggc_runtime.h>
#include <hgrtc_helper.h>
#include <helper_functions.h>

namespace {
constexpr int N     = 1024;
constexpr int BLOCK = 256;
constexpr int GRID  = (N + BLOCK - 1) / BLOCK;
}  // namespace

int main(int argc, char **argv)
{
    printf("[inline_tix_hgrtc] TIX Instruction Showcase (HGRTC)\n\n");

    // Compile kernel source.
    char  *hgbin      = nullptr;
    size_t hgbin_size = 0;
    char  *kpath      = findSampleAsset("inline_tix_kernel.hg", argv[0]);
    compileFileToHGBIN(kpath, argc, argv, &hgbin, &hgbin_size, 0);
    free(kpath);

    HGmodule module = loadHGBIN(hgbin, argc, argv);

    // Resolve all 4 kernel functions.
    HGfunction fn_horner, fn_popc, fn_wmul, fn_bfe;
    checkHggcErrors(hgModuleGetFunction(&fn_horner, module, "horner_fma_kernel"));
    checkHggcErrors(hgModuleGetFunction(&fn_popc,   module, "popc_kernel"));
    checkHggcErrors(hgModuleGetFunction(&fn_wmul,   module, "wide_mul_kernel"));
    checkHggcErrors(hgModuleGetFunction(&fn_bfe,    module, "bfe_kernel"));

    bool all_pass = true;
    int n = N;

    // ---- Test 1: FMA Horner ----
    printf("  Test 1: ppu.fma.rtte.f32 (Horner polynomial)\n");
    {
        float *h_x = (float *)malloc(N * sizeof(float));
        for (int i = 0; i < N; ++i) h_x[i] = (i - N / 2) * 0.01f;

        HGdeviceptr d_x, d_out;
        checkHggcErrors(hgMemAlloc(&d_x,  N * sizeof(float)));
        checkHggcErrors(hgMemAlloc(&d_out, N * sizeof(float)));
        checkHggcErrors(hgMemcpyHtoD(d_x, h_x, N * sizeof(float)));

        void *args[] = {&d_out, &d_x, &n};
        checkHggcErrors(hgLaunchKernel(fn_horner, GRID, 1, 1, BLOCK, 1, 1, 0, 0, args, nullptr));
        checkHggcErrors(hgCtxSynchronize());

        float *h_out = (float *)malloc(N * sizeof(float));
        checkHggcErrors(hgMemcpyDtoH(h_out, d_out, N * sizeof(float)));

        bool pass = true;
        for (int i = 0; i < N && pass; ++i) {
            float x = h_x[i];
            // Use fmaf() to match PPU's FMA single-rounding semantics
            float ref = fmaf(fmaf(fmaf(3.0f, x, 2.0f), x, 1.0f), x, 0.5f);
            if (fabsf(h_out[i] - ref) > 1e-5f) pass = false;
        }
        printf("    %s\n", pass ? "PASS" : "FAIL");
        all_pass = all_pass && pass;

        hgMemFree(d_x); hgMemFree(d_out); free(h_x); free(h_out);
    }

    // ---- Test 2: Popcount ----
    printf("  Test 2: ppu.popc.b32 (population count)\n");
    {
        unsigned int *h_data = (unsigned int *)malloc(N * sizeof(unsigned int));
        for (int i = 0; i < N; ++i) h_data[i] = (unsigned int)(i * 0x9e3779b9u);

        HGdeviceptr d_data, d_out;
        checkHggcErrors(hgMemAlloc(&d_data, N * sizeof(unsigned int)));
        checkHggcErrors(hgMemAlloc(&d_out,  N * sizeof(int)));
        checkHggcErrors(hgMemcpyHtoD(d_data, h_data, N * sizeof(unsigned int)));

        void *args[] = {&d_out, &d_data, &n};
        checkHggcErrors(hgLaunchKernel(fn_popc, GRID, 1, 1, BLOCK, 1, 1, 0, 0, args, nullptr));
        checkHggcErrors(hgCtxSynchronize());

        int *h_out = (int *)malloc(N * sizeof(int));
        checkHggcErrors(hgMemcpyDtoH(h_out, d_out, N * sizeof(int)));

        bool pass = true;
        for (int i = 0; i < N && pass; ++i) {
            if (h_out[i] != __builtin_popcount(h_data[i])) pass = false;
        }
        printf("    %s\n", pass ? "PASS" : "FAIL");
        all_pass = all_pass && pass;

        hgMemFree(d_data); hgMemFree(d_out); free(h_data); free(h_out);
    }

    // ---- Test 3: Wide multiply ----
    printf("  Test 3: ppu.mul.wide.s32 (32x32 -> 64-bit product)\n");
    {
        int *h_a = (int *)malloc(N * sizeof(int));
        int *h_b = (int *)malloc(N * sizeof(int));
        for (int i = 0; i < N; ++i) { h_a[i] = i - 512; h_b[i] = i * 3 + 7; }

        HGdeviceptr d_a, d_b, d_out;
        checkHggcErrors(hgMemAlloc(&d_a,  N * sizeof(int)));
        checkHggcErrors(hgMemAlloc(&d_b,  N * sizeof(int)));
        checkHggcErrors(hgMemAlloc(&d_out, N * sizeof(long long)));
        checkHggcErrors(hgMemcpyHtoD(d_a, h_a, N * sizeof(int)));
        checkHggcErrors(hgMemcpyHtoD(d_b, h_b, N * sizeof(int)));

        void *args[] = {&d_out, &d_a, &d_b, &n};
        checkHggcErrors(hgLaunchKernel(fn_wmul, GRID, 1, 1, BLOCK, 1, 1, 0, 0, args, nullptr));
        checkHggcErrors(hgCtxSynchronize());

        long long *h_out = (long long *)malloc(N * sizeof(long long));
        checkHggcErrors(hgMemcpyDtoH(h_out, d_out, N * sizeof(long long)));

        bool pass = true;
        for (int i = 0; i < N && pass; ++i) {
            if (h_out[i] != (long long)h_a[i] * (long long)h_b[i]) pass = false;
        }
        printf("    %s\n", pass ? "PASS" : "FAIL");
        all_pass = all_pass && pass;

        hgMemFree(d_a); hgMemFree(d_b); hgMemFree(d_out);
        free(h_a); free(h_b); free(h_out);
    }

    // ---- Test 4: Bit-field extract ----
    printf("  Test 4: ppu.bfe.u32 (bit-field extract)\n");
    {
        int pos = 8, len = 8;
        unsigned int *h_data = (unsigned int *)malloc(N * sizeof(unsigned int));
        for (int i = 0; i < N; ++i) h_data[i] = 0xDEAD0000u | (unsigned int)i;

        HGdeviceptr d_data, d_out;
        checkHggcErrors(hgMemAlloc(&d_data, N * sizeof(unsigned int)));
        checkHggcErrors(hgMemAlloc(&d_out,  N * sizeof(unsigned int)));
        checkHggcErrors(hgMemcpyHtoD(d_data, h_data, N * sizeof(unsigned int)));

        void *args[] = {&d_out, &d_data, &pos, &len, &n};
        checkHggcErrors(hgLaunchKernel(fn_bfe, GRID, 1, 1, BLOCK, 1, 1, 0, 0, args, nullptr));
        checkHggcErrors(hgCtxSynchronize());

        unsigned int *h_out = (unsigned int *)malloc(N * sizeof(unsigned int));
        checkHggcErrors(hgMemcpyDtoH(h_out, d_out, N * sizeof(unsigned int)));

        bool pass = true;
        for (int i = 0; i < N && pass; ++i) {
            unsigned int ref = (h_data[i] >> pos) & ((1u << len) - 1u);
            if (h_out[i] != ref) pass = false;
        }
        printf("    %s\n", pass ? "PASS" : "FAIL");
        all_pass = all_pass && pass;

        hgMemFree(d_data); hgMemFree(d_out); free(h_data); free(h_out);
    }

    printf("\n  Result: %s\n", all_pass ? "PASS" : "FAIL");
    return all_pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
