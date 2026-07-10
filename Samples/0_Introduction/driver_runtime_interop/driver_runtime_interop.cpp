/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <hggc.h>
#include <hggc_runtime.h>

#include <helper_functions.h>
#include <helper_hggc.h>

namespace {

#ifndef FATBIN_FILE
#define FATBIN_FILE "vector_add_kernel64.fatbin"
#endif

constexpr int   kElementCount = 50000;
constexpr int   kBlockDim     = 256;
constexpr float kTolerance    = 1.0e-7f;

// Host- and device-side staging buffers shared by main() and the cleanup helper.
float *g_host_a = nullptr;
float *g_host_b = nullptr;
float *g_host_c = nullptr;
float *g_dev_a  = nullptr;
float *g_dev_b  = nullptr;
float *g_dev_c  = nullptr;

/// Adapter that translates an HGresult into the project's exit-on-failure flow.
void check_drv(HGresult code, const char *expr, const char *file, int line)
{
    if (code != HGGC_SUCCESS) {
        std::fprintf(stderr, "HGGC driver-API error at %s:%d code=%u expr=%s\n",
                     file, line, static_cast<unsigned>(code), expr);
        std::exit(EXIT_FAILURE);
    }
}

#define CHECK_DRV(expr) check_drv((expr), #expr, __FILE__, __LINE__)

/// Fill an array of length @p count with deterministic-ish random floats.
void random_fill(float *data, int count)
{
    for (int i = 0; i < count; ++i) {
        data[i] = std::rand() / static_cast<float>(RAND_MAX);
    }
}

/// Locate the fatbin on disk and stream its bytes into @p ostrm.
bool resolve_fatbin(const char *fatbin_name, std::string &out_path, char **argv, std::ostringstream &ostrm)
{
    char *located = findSampleAsset(fatbin_name, argv[0]);
    if (located == nullptr) {
        std::printf("Cannot locate fatbin: %s\n", fatbin_name);
        return false;
    }
    out_path = located;
    free(located);
    if (out_path.empty()) {
        std::printf("Empty fatbin path for %s\n", fatbin_name);
        return false;
    }
    std::printf("> resolve_fatbin found <%s>\n", out_path.c_str());
    if (out_path.rfind("fatbin") != std::string::npos) {
        std::ifstream input(out_path.c_str(), std::ios::binary);
        ostrm << input.rdbuf();
    }
    return true;
}

/// Tear down all host- and device-side resources.
int teardown(HGcontext &ctx)
{
    checkHggcErrors(hggcFree(g_dev_a));
    checkHggcErrors(hggcFree(g_dev_b));
    checkHggcErrors(hggcFree(g_dev_c));

    if (g_host_a != nullptr) checkHggcErrors(hggcFreeHost(g_host_a));
    if (g_host_b != nullptr) checkHggcErrors(hggcFreeHost(g_host_b));
    if (g_host_c != nullptr) checkHggcErrors(hggcFreeHost(g_host_c));

    CHECK_DRV(hgCtxDestroy(ctx));
    return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char **argv)
{
    std::printf("driver_runtime_interop starting...\n");

    const size_t bytes = static_cast<size_t>(kElementCount) * sizeof(float);

    // Driver-side bring up: pick a device and create a context.
    HGdevice   dev_handle = 0;
    HGcontext  ctx        = nullptr;
    HGmodule   module     = nullptr;
    HGfunction kernel     = nullptr;

    CHECK_DRV(hgInit(0));
    dev_handle = findHggcDevice(argc, const_cast<const char **>(argv));
#if HGGC_VERSION >= 13000
    CHECK_DRV(hgCtxCreate(&ctx, NULL, 0, dev_handle));
#else
    CHECK_DRV(hgCtxCreate(&ctx, 0, dev_handle));
#endif

    // Locate and load the precompiled module.
    std::string        module_path;
    std::ostringstream fatbin_stream;
    if (!resolve_fatbin(FATBIN_FILE, module_path, argv, fatbin_stream)) {
        std::exit(EXIT_FAILURE);
    }
    if (fatbin_stream.str().empty()) {
        std::printf("Empty fatbin contents.\n");
        std::exit(EXIT_FAILURE);
    }
    std::printf("> loaded module: <%s>\n", module_path.c_str());
    CHECK_DRV(hgModuleLoadData(&module, fatbin_stream.str().c_str()));
    CHECK_DRV(hgModuleGetFunction(&kernel, module, "element_mul_kernel"));

    // Runtime-side allocations and async data movement.
    checkHggcErrors(hggcMallocHost(&g_host_a, bytes));
    checkHggcErrors(hggcMallocHost(&g_host_b, bytes));
    checkHggcErrors(hggcMallocHost(&g_host_c, bytes));
    random_fill(g_host_a, kElementCount);
    random_fill(g_host_b, kElementCount);

    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&g_dev_a), bytes));
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&g_dev_b), bytes));
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&g_dev_c), bytes));

    hggcStream_t stream;
    checkHggcErrors(hggcStreamCreateWithFlags(&stream, hggcStreamNonBlocking));
    checkHggcErrors(hggcMemcpyAsync(g_dev_a, g_host_a, bytes, hggcMemcpyHostToDevice, stream));
    checkHggcErrors(hggcMemcpyAsync(g_dev_b, g_host_b, bytes, hggcMemcpyHostToDevice, stream));

    // Driver-side kernel launch on the runtime stream (interop point).
    int   element_count = kElementCount;
    void *kernel_args[] = {&g_dev_a, &g_dev_b, &g_dev_c, &element_count};
    const int grid_dim  = (kElementCount + kBlockDim - 1) / kBlockDim;

    CHECK_DRV(hgLaunchKernel(kernel,
                             grid_dim, 1, 1,
                             kBlockDim, 1, 1,
                             0, stream,
                             kernel_args, nullptr));

    checkHggcErrors(hggcMemcpyAsync(g_host_c, g_dev_c, bytes, hggcMemcpyDeviceToHost, stream));
    checkHggcErrors(hggcStreamSynchronize(stream));
    checkHggcErrors(hggcStreamDestroy(stream));

    // Verify against a CPU reference.
    int passed = 1;
    for (int i = 0; i < kElementCount; ++i) {
        const float reference = g_host_a[i] * g_host_b[i];
        if (std::fabs(g_host_c[i] - reference) > kTolerance) {
            passed = 0;
            break;
        }
    }

    CHECK_DRV(hgModuleUnload(module));
    teardown(ctx);

    std::printf("%s\n", passed ? "Result = PASS" : "Result = FAIL");
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
