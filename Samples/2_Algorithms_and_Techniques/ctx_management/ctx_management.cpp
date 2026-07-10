/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * ctx_management -- Driver API Multi-Context Lifecycle Demo
 *
 * Demonstrates the core context management APIs of the HGGC Driver API:
 *
 *   1. Create multiple contexts on the same device
 *   2. Push/pop contexts between host threads (context migration)
 *   3. Each context independently loads a module and launches a kernel
 *   4. Proper cleanup with hgCtxDestroy
 *
 * Shows how hgCtxPushCurrent / hgCtxPopCurrent enable context sharing
 * across threads -- a pattern used in multi-threaded applications where
 * different CPU threads need to submit work to the same or different PPUs.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

#include <hggc.h>
#include <hggc_runtime.h>
#include <hgrtc_helper.h>
#include <helper_functions.h>
#include <helper_hggc_drvapi.h>

namespace {

constexpr int NUM_CONTEXTS = 3;   // create 3 contexts on the same device
constexpr int NUM_THREADS  = 6;   // 6 worker threads share the contexts
constexpr int DATA_SIZE    = 64;

std::mutex              g_print_mutex;
std::atomic<int>        g_pass_count{0};

// Embedded kernel source (compiled at runtime via HGRTC).
const char *g_kernel_source = R"(
extern "C" __global__ void fill_kernel(int *data, int value, int n)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) data[idx] = value + idx;
}
)";

}  // namespace

// ═══════════════════════════════════════════════════════════════════════
// Worker thread: push a context, launch kernel, verify, pop context
// ═══════════════════════════════════════════════════════════════════════

static void worker_thread(int thread_id, HGcontext ctx, HGfunction kernel)
{
    // Attach context to this thread.
    HGresult status = hgCtxPushCurrent(ctx);
    if (status != HGGC_SUCCESS) {
        std::lock_guard<std::mutex> lock(g_print_mutex);
        printf("  Thread %d: hgCtxPushCurrent FAILED (%d)\n", thread_id, (int)status);
        return;
    }

    // Allocate device memory within this context.
    HGdeviceptr d_data;
    checkHggcErrors(hgMemAlloc(&d_data, DATA_SIZE * sizeof(int)));

    // Launch kernel: fill with (thread_id * 100 + idx).
    int fill_value = thread_id * 100;
    int n = DATA_SIZE;
    void *args[] = {&d_data, &fill_value, &n};
    checkHggcErrors(hgLaunchKernel(kernel,
                                   1, 1, 1,    // grid
                                   DATA_SIZE, 1, 1,  // block
                                   0, nullptr,
                                   args, nullptr));
    checkHggcErrors(hgCtxSynchronize());

    // Read back and verify.
    int h_data[DATA_SIZE];
    checkHggcErrors(hgMemcpyDtoH(h_data, d_data, DATA_SIZE * sizeof(int)));

    bool pass = true;
    for (int i = 0; i < DATA_SIZE; ++i) {
        if (h_data[i] != fill_value + i) {
            pass = false;
            break;
        }
    }

    checkHggcErrors(hgMemFree(d_data));

    // Detach context from this thread.
    checkHggcErrors(hgCtxPopCurrent(nullptr));

    if (pass) g_pass_count++;

    {
        std::lock_guard<std::mutex> lock(g_print_mutex);
        printf("  Thread %d -> Context %p : fill_value=%d  %s\n",
               thread_id, (void *)ctx, fill_value, pass ? "PASS" : "FAIL");
    }
}

// ═══════════════════════════════════════════════════════════════════════
// Main
// ═══════════════════════════════════════════════════════════════════════

int main(int argc, char **argv)
{
    printf("[ctx_management] Driver API Multi-Context Lifecycle\n\n");

    checkHggcErrors(hgInit(0));

    int device_count = 0;
    checkHggcErrors(hgDeviceGetCount(&device_count));
    if (device_count == 0) {
        printf("  No PPU devices found.\n");
        return EXIT_FAILURE;
    }

    HGdevice device;
    checkHggcErrors(hgDeviceGet(&device, 0));

    char name[256];
    checkHggcErrors(hgDeviceGetName(name, sizeof(name), device));
    printf("  Device 0: %s\n", name);
    printf("  Creating %d contexts, launching %d worker threads\n\n", NUM_CONTEXTS, NUM_THREADS);

    // ---- Create multiple contexts on the same device ----
    HGcontext contexts[NUM_CONTEXTS];
    HGmodule  modules[NUM_CONTEXTS];
    HGfunction kernels[NUM_CONTEXTS];

    // Query device arch once for HGRTC compilation.
    int major = 0, minor = 0;
    checkHggcErrors(hgDeviceGetAttribute(&major,
                    HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, device));
    checkHggcErrors(hgDeviceGetAttribute(&minor,
                    HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, device));
    const char *archName = hgPpuArchName(major, minor);
    char archOpt[64];
    std::snprintf(archOpt, sizeof(archOpt), "--ppu-arch=%s", archName);
    const char *compileOpts[] = {archOpt};

    for (int i = 0; i < NUM_CONTEXTS; ++i) {
#if HGGC_VERSION >= 13000
        checkHggcErrors(hgCtxCreate(&contexts[i], NULL, 0, device));
#else
        checkHggcErrors(hgCtxCreate(&contexts[i], 0, device));
#endif

        // Compile kernel within this context via HGRTC (from source string).
        hgrtcProgram prog;
        HGRTC_CHECK("hgrtcCreateProgram",
            hgrtcCreateProgram(&prog, g_kernel_source, "ctx_management.hg", 0, NULL, NULL));
        hgrtcResult compile_rc = hgrtcCompileProgram(prog, 1, compileOpts);

        // Print log if any.
        size_t log_bytes = 0;
        HGRTC_CHECK("hgrtcGetProgramLogSize", hgrtcGetProgramLogSize(prog, &log_bytes));
        if (log_bytes > 1) {
            char *log = new char[log_bytes + 1];
            hgrtcGetProgramLog(prog, log);
            log[log_bytes] = '\0';
            printf("  HGRTC log (ctx %d): %s\n", i, log);
            delete[] log;
        }
        HGRTC_CHECK("hgrtcCompileProgram", compile_rc);

        size_t hgbin_size = 0;
        HGRTC_CHECK("hgrtcGetHGBINSize", hgrtcGetHGBINSize(prog, &hgbin_size));
        char *hgbin = new char[hgbin_size];
        HGRTC_CHECK("hgrtcGetHGBIN", hgrtcGetHGBIN(prog, hgbin));
        HGRTC_CHECK("hgrtcDestroyProgram", hgrtcDestroyProgram(&prog));

        checkHggcErrors(hgModuleLoadData(&modules[i], hgbin));
        delete[] hgbin;
        checkHggcErrors(hgModuleGetFunction(&kernels[i], modules[i], "fill_kernel"));

        // Pop context so it becomes floating (not attached to main thread).
        checkHggcErrors(hgCtxPopCurrent(nullptr));

        printf("  Context %d created: %p\n", i, (void *)contexts[i]);
    }
    printf("\n");

    // ---- Launch worker threads, each picks a context (round-robin) ----
    std::vector<std::thread> threads;
    for (int t = 0; t < NUM_THREADS; ++t) {
        int ctx_idx = t % NUM_CONTEXTS;
        threads.emplace_back(worker_thread, t, contexts[ctx_idx], kernels[ctx_idx]);
    }

    // Note: multiple threads can sequentially push/pop the SAME context,
    // but only one thread can hold a context at a time. In this demo we
    // let threads run concurrently -- if they share a context they'll
    // serialize on hgCtxPushCurrent (which blocks until the context is free).

    for (auto &th : threads) {
        th.join();
    }

    // ---- Cleanup: destroy all contexts ----
    printf("\n  Destroying contexts...\n");
    for (int i = 0; i < NUM_CONTEXTS; ++i) {
        checkHggcErrors(hgModuleUnload(modules[i]));
        checkHggcErrors(hgCtxDestroy(contexts[i]));
    }

    bool all_pass = (g_pass_count == NUM_THREADS);
    printf("\n  %d/%d threads passed.\n", g_pass_count.load(), NUM_THREADS);
    printf("  Result: %s\n", all_pass ? "PASS" : "FAIL");
    return all_pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
