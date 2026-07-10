/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * assert_runtime_compile — Device-side assert (HGRTC + Driver API)
 *
 * Same invariant-checker logic as `assert.hg`, but the kernel source is
 * compiled at runtime via libHGRTC and launched through the Driver API.
 * Demonstrates how device assertions (HGGC_ERROR_ASSERT) propagate
 * through the driver-level synchronization path.
 */
#include <cstdio>
#include <cstdlib>

#include <hggc_runtime.h>
#include <hgrtc_helper.h>
#include <helper_functions.h>

namespace {
constexpr int kN         = 1024;
constexpr int kBlockSize = 256;
constexpr int kGridSize  = (kN + kBlockSize - 1) / kBlockSize;
}  // namespace

int main(int argc, char **argv)
{
    printf("[assert_runtime_compile] Device-side invariant checker (HGRTC)\n\n");

    const size_t bytes = sizeof(float) * kN;

    // Compile kernel source.
    char  *hgbin       = nullptr;
    size_t hgbin_size  = 0;
    char  *kernel_path = findSampleAsset("assert_kernel.hg", argv[0]);
    compileFileToHGBIN(kernel_path, argc, argv, &hgbin, &hgbin_size, 0);
    free(kernel_path);

    HGmodule module = loadHGBIN(hgbin, argc, argv);

    HGfunction fn_prefix_max;
    HGfunction fn_validate;
    checkHggcErrors(hgModuleGetFunction(&fn_prefix_max, module, "prefix_max_kernel"));
    checkHggcErrors(hgModuleGetFunction(&fn_validate,   module, "validate_monotonic_kernel"));

    // Host input.
    float *h_input = static_cast<float *>(malloc(bytes));
    for (int i = 0; i < kN; ++i) {
        h_input[i] = static_cast<float>((i * 7 + 13) % 100);
    }

    // Device allocations.
    HGdeviceptr d_input, d_output;
    checkHggcErrors(hgMemAlloc(&d_input,  bytes));
    checkHggcErrors(hgMemAlloc(&d_output, bytes));
    checkHggcErrors(hgMemcpyHtoD(d_input, h_input, bytes));

    int n = kN;

    // ---- Scenario 1: correct computation ----
    printf("  Scenario 1: valid prefix-max output\n");
    void *args_compute[] = {&d_input, &d_output, &n};
    // Launch with 1 thread for a correct sequential prefix-max.
    checkHggcErrors(hgLaunchKernel(fn_prefix_max,
                                   1, 1, 1,
                                   1, 1, 1,
                                   0, 0,
                                   args_compute, nullptr));
    checkHggcErrors(hgCtxSynchronize());

    void *args_validate[] = {&d_output, &n};
    checkHggcErrors(hgLaunchKernel(fn_validate,
                                   kGridSize, 1, 1,
                                   kBlockSize, 1, 1,
                                   0, 0,
                                   args_validate, nullptr));
    HGresult status1 = hgCtxSynchronize();

    if (status1 == HGGC_SUCCESS) {
        printf("    -> No assertion fired (PASS)\n\n");
    } else {
        printf("    -> Unexpected error code: %d\n\n", static_cast<int>(status1));
    }

    // ---- Scenario 2: corrupt output, expect assert ----
    printf("  Scenario 2: corrupted output (expect assert)\n");

    // Write a bad value at the midpoint via host-side memcpy.
    float bad_val = -999.0f;
    checkHggcErrors(hgMemcpyHtoD(
        d_output + static_cast<HGdeviceptr>(sizeof(float) * (kN / 2)),
        &bad_val, sizeof(float)));

    printf("    Launching validation kernel...\n");
    printf("\n-- Begin assert output\n\n");
    checkHggcErrors(hgLaunchKernel(fn_validate,
                                   kGridSize, 1, 1,
                                   kBlockSize, 1, 1,
                                   0, 0,
                                   args_validate, nullptr));
    HGresult status2 = hgCtxSynchronize();
    printf("\n-- End assert output\n\n");

    bool pass = false;
    if (status2 == HGGC_ERROR_ASSERT) {
        printf("    -> Device assert fired as expected (PASS)\n");
        pass = true;
    } else {
        printf("    -> Expected HGGC_ERROR_ASSERT but got: %d (FAIL)\n",
               static_cast<int>(status2));
    }

    // Cleanup. If the device assert fired the context is tainted;
    // reset the device instead of calling individual hgMemFree operations.
    free(h_input);
    if (pass) {
        hggcDeviceReset();
    } else {
        checkHggcErrors(hgMemFree(d_input));
        checkHggcErrors(hgMemFree(d_output));
    }

    printf("\n  Result: %s\n", pass ? "PASS" : "FAIL");
    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
