/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * histogram — Host driver
 *
 * Runs three histogram kernel variants on the same dataset, validates each
 * against a CPU reference, and compares throughput to illustrate the impact
 * of privatization and contention reduction.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <hggc_runtime.h>
#include <helper_hggc.h>
#include <helper_functions.h>

#include "histogram_common.h"

namespace {

constexpr int kDataSize = 32 * 1024 * 1024;  // 32 MB
constexpr int kWarmup   = 2;
constexpr int kIters    = 16;

struct Strategy {
    const char *name;
    void (*func)(uint32_t *, const uint8_t *, int);
};

bool validate(const uint32_t *ppu, const uint32_t *cpu)
{
    for (int i = 0; i < NUM_BINS; ++i) {
        if (ppu[i] != cpu[i]) {
            printf("    MISMATCH at bin %d: PPU=%u  CPU=%u\n", i, ppu[i], cpu[i]);
            return false;
        }
    }
    return true;
}

}  // namespace

int main(int argc, char **argv)
{
    printf("[histogram] Progressive optimization: 256-bin byte histogram\n\n");

    findHggcDevice(argc, (const char **)argv);

    printf("  Data size: %d MB\n\n", kDataSize / (1024 * 1024));

    // ---- Host data ----
    uint8_t *h_data = static_cast<uint8_t *>(malloc(kDataSize));
    for (int i = 0; i < kDataSize; ++i) {
        h_data[i] = static_cast<uint8_t>((i * 37 + 101) & 0xFF);
    }

    uint32_t h_cpu_hist[NUM_BINS];
    histogram_cpu(h_cpu_hist, h_data, kDataSize);

    // ---- Device data ----
    uint8_t  *d_data = nullptr;
    uint32_t *d_hist = nullptr;
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&d_data), kDataSize));
    checkHggcErrors(hggcMalloc(reinterpret_cast<void **>(&d_hist), NUM_BINS * sizeof(uint32_t)));
    checkHggcErrors(hggcMemcpy(d_data, h_data, kDataSize, hggcMemcpyHostToDevice));

    uint32_t h_ppu_hist[NUM_BINS];

    // ---- Run all strategies ----
    Strategy strategies[] = {
        {"Global atomicAdd (naive)",       histogram_global_atomic},
        {"Shared-mem privatization",       histogram_shared_private},
        {"Warp-level privatization",       histogram_warp_private},
    };

    bool all_pass = true;
    HggcTimer timer;

    for (const auto &s : strategies) {
        printf("  %-35s", s.name);

        // Warmup.
        for (int i = 0; i < kWarmup; ++i) s.func(d_hist, d_data, kDataSize);
        checkHggcErrors(hggcDeviceSynchronize());

        // Timed iterations.
        timer.reset();
        timer.start();
        for (int i = 0; i < kIters; ++i) s.func(d_hist, d_data, kDataSize);
        checkHggcErrors(hggcDeviceSynchronize());
        timer.stop();

        double ms_per_iter = timer.elapsed() / kIters;
        double gb_per_sec  = (kDataSize * 1e-9) / (ms_per_iter * 1e-3);

        // Validate last result.
        checkHggcErrors(hggcMemcpy(h_ppu_hist, d_hist, NUM_BINS * sizeof(uint32_t), hggcMemcpyDeviceToHost));
        bool ok = validate(h_ppu_hist, h_cpu_hist);
        all_pass = all_pass && ok;

        printf("%6.2f ms  %5.1f GB/s  %s\n", ms_per_iter, gb_per_sec, ok ? "PASS" : "FAIL");
    }

    // ---- Cleanup ----
    free(h_data);
    checkHggcErrors(hggcFree(d_data));
    checkHggcErrors(hggcFree(d_hist));

    printf("\n  Result: %s\n", all_pass ? "PASS" : "FAIL");
    return all_pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
