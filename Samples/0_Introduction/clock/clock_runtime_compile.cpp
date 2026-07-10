/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * clock_runtime_compile — Memory coalescing microbenchmark (HGRTC + Driver API)
 *
 * Same benchmark as `clock.hg` but compiles the kernel source at runtime
 * via libHGRTC and launches through the Driver API.  Demonstrates:
 *   - Runtime compilation with HGRTC
 *   - Driver API module loading and kernel launch
 *   - Device-side clock() for per-block cycle measurement
 */
#include <cstdio>
#include <cstdlib>

#include <hggc_runtime.h>
#include <hgrtc_helper.h>
#include <helper_functions.h>

namespace {
constexpr int kNumBlocks   = 32;
constexpr int kNumThreads  = 256;
constexpr int kNumIters    = 64;
constexpr int kTotalElems  = kNumBlocks * kNumThreads;
constexpr int kTimerSlots  = kNumBlocks * 2;
}  // namespace

/// Compute average clocks/block from the start/end timer array.
static long double compute_avg_clocks(const long *timer, int num_blocks)
{
    long double total = 0;
    for (int i = 0; i < num_blocks; ++i) {
        total += static_cast<long double>(timer[i + num_blocks] - timer[i]);
    }
    return total / num_blocks;
}

int main(int argc, char **argv)
{
    printf("[clock_runtime_compile] Memory coalescing microbenchmark (HGRTC)\n\n");

    typedef long clock_t;

    const size_t input_bytes  = sizeof(float)   * kTotalElems;
    const size_t output_bytes = sizeof(float)   * kTotalElems;
    const size_t timer_bytes  = sizeof(clock_t) * kTimerSlots;

    // Host input.
    float *h_input = static_cast<float *>(malloc(input_bytes));
    for (int i = 0; i < kTotalElems; ++i) {
        h_input[i] = static_cast<float>(i % 128) * 0.01f;
    }
    clock_t *h_timer = static_cast<clock_t *>(malloc(timer_bytes));

    // Compile kernel source at runtime.
    char  *hgbin       = nullptr;
    size_t hgbin_size  = 0;
    char  *kernel_path = findSampleAsset("clock_kernel.hg", argv[0]);
    compileFileToHGBIN(kernel_path, argc, argv, &hgbin, &hgbin_size, 0);
    free(kernel_path);

    HGmodule module = loadHGBIN(hgbin, argc, argv);

    HGfunction fn_coalesced;
    HGfunction fn_strided;
    checkHggcErrors(hgModuleGetFunction(&fn_coalesced, module, "clock_coalesced"));
    checkHggcErrors(hgModuleGetFunction(&fn_strided,   module, "clock_strided"));

    // Device allocations (Driver API).
    HGdeviceptr d_input, d_output, d_timer;
    checkHggcErrors(hgMemAlloc(&d_input,  input_bytes));
    checkHggcErrors(hgMemAlloc(&d_output, output_bytes));
    checkHggcErrors(hgMemAlloc(&d_timer,  timer_bytes));
    checkHggcErrors(hgMemcpyHtoD(d_input, h_input, input_bytes));

    int num_iters = kNumIters;

    // ---- Coalesced pass ----
    void *args_coal[] = {
        &d_input, &d_output, &d_timer, &num_iters
    };
    checkHggcErrors(hgLaunchKernel(fn_coalesced,
                                   kNumBlocks, 1, 1,
                                   kNumThreads, 1, 1,
                                   0, 0,
                                   args_coal, nullptr));
    checkHggcErrors(hgCtxSynchronize());
    checkHggcErrors(hgMemcpyDtoH(h_timer, d_timer, timer_bytes));

    const long double avg_coalesced = compute_avg_clocks(h_timer, kNumBlocks);

    // ---- Strided pass ----
    void *args_strd[] = {
        &d_input, &d_output, &d_timer, &num_iters
    };
    checkHggcErrors(hgLaunchKernel(fn_strided,
                                   kNumBlocks, 1, 1,
                                   kNumThreads, 1, 1,
                                   0, 0,
                                   args_strd, nullptr));
    checkHggcErrors(hgCtxSynchronize());
    checkHggcErrors(hgMemcpyDtoH(h_timer, d_timer, timer_bytes));

    const long double avg_strided = compute_avg_clocks(h_timer, kNumBlocks);

    // ---- Report ----
    printf("  Coalesced : %.0Lf clocks/block\n", avg_coalesced);
    printf("  Strided   : %.0Lf clocks/block\n", avg_strided);
    printf("  Ratio     : %.2Lfx slower\n", avg_strided / avg_coalesced);
    printf("\n  %s\n", (avg_strided > avg_coalesced)
           ? "Result = PASS (strided is slower as expected)"
           : "Result = UNEXPECTED (coalesced not faster)");

    // Cleanup.
    free(h_input);
    free(h_timer);
    checkHggcErrors(hgMemFree(d_input));
    checkHggcErrors(hgMemFree(d_output));
    checkHggcErrors(hgMemFree(d_timer));

    return EXIT_SUCCESS;
}
