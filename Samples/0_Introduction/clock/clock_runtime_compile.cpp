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
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

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
template <typename Clock>
static long double compute_avg_clocks(const Clock *timer, int num_blocks)
{
    using Counter = typename std::make_unsigned<Clock>::type;
    long double total = 0;
    for (int i = 0; i < num_blocks; ++i) {
        const Counter elapsed = static_cast<Counter>(timer[i + num_blocks]) -
                                static_cast<Counter>(timer[i]);
        if (elapsed == 0 || elapsed > std::numeric_limits<Counter>::max() / 2) {
            printf("  Invalid timer at block %d\n", i);
            return 0;
        }
        total += static_cast<long double>(elapsed);
    }
    return total / num_blocks;
}

// Validate both addressing modes with an independent high-precision sum; speed ranking is not a correctness criterion.
static bool validate_output(const float *input, const float *output,
                            bool strided, const char *label)
{
    int mismatches = 0;
    for (int gid = 0; gid < kTotalElems; ++gid) {
        long double expected = 0;
        for (int i = 0; i < kNumIters; ++i) {
            const int index = strided
                ? (gid / kNumThreads) * kNumThreads +
                  ((gid % kNumThreads) * 32 + i) % kNumThreads
                : gid;
            expected += static_cast<long double>(input[index]);
        }
        const long double error = std::fabs(static_cast<long double>(output[gid]) - expected);
        const long double tolerance = 1e-4L + 1e-5L * std::fabs(expected);
        if (!std::isfinite(output[gid]) || error > tolerance) {
            if (mismatches == 0) {
                printf("  %s first mismatch at [%d]: got=%.8g expected=%.8Lf\n",
                       label, gid, output[gid], expected);
            }
            ++mismatches;
        }
    }
    printf("  %s mismatches=%d\n", label, mismatches);
    return mismatches == 0;
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
    if (!h_input) return EXIT_FAILURE;
    std::vector<float> h_output(kTotalElems);
    const std::vector<float> h_unwritten(kTotalElems, std::numeric_limits<float>::quiet_NaN());
    const std::vector<clock_t> h_timer_unwritten(kTimerSlots, 0);
    for (int i = 0; i < kTotalElems; ++i) {
        h_input[i] = static_cast<float>(i % 128) * 0.01f;
    }
    clock_t *h_timer = static_cast<clock_t *>(malloc(timer_bytes));
    if (!h_timer) {
        free(h_input);
        return EXIT_FAILURE;
    }

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
    checkHggcErrors(hgMemcpyHtoD(d_output, h_unwritten.data(), output_bytes));
    checkHggcErrors(hgMemcpyHtoD(d_timer, h_timer_unwritten.data(), timer_bytes));
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
    checkHggcErrors(hgMemcpyDtoH(h_output.data(), d_output, output_bytes));
    const bool coalesced_ok = validate_output(h_input, h_output.data(), false, "Coalesced");

    // ---- Strided pass ----
    checkHggcErrors(hgMemcpyHtoD(d_output, h_unwritten.data(), output_bytes));
    checkHggcErrors(hgMemcpyHtoD(d_timer, h_timer_unwritten.data(), timer_bytes));
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
    checkHggcErrors(hgMemcpyDtoH(h_output.data(), d_output, output_bytes));
    const bool strided_ok = validate_output(h_input, h_output.data(), true, "Strided");
    const bool pass = coalesced_ok && strided_ok && avg_coalesced > 0 && avg_strided > 0;

    // ---- Report ----
    printf("  Coalesced : %.0Lf clocks/block\n", avg_coalesced);
    printf("  Strided   : %.0Lf clocks/block\n", avg_strided);
    if (avg_coalesced > 0 && avg_strided > 0) {
        printf("  INFO strided/coalesced cycle ratio: %.2Lf\n", avg_strided / avg_coalesced);
    }
    printf("\n  Result: %s\n", pass ? "PASS" : "FAIL");

    // Cleanup.
    free(h_input);
    free(h_timer);
    checkHggcErrors(hgMemFree(d_input));
    checkHggcErrors(hgMemFree(d_output));
    checkHggcErrors(hgMemFree(d_timer));

    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
