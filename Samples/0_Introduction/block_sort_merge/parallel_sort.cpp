/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * parallel_sort.cpp -- Block sort + merge demo (host side)
 *
 * Generates random key-value pairs, sorts on device using block-level
 * odd-even transposition sort + bitonic merge, then verifies against
 * a CPU reference sort.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>
#include <utility>

#include <hggc_runtime.h>
#include <helper_hggc.h>
#include <helper_functions.h>

typedef unsigned int uint;

/* Device function prototype */
extern "C" void parallel_sort(
    uint *dst_key, uint *dst_val,
    uint *buf_key, uint *buf_val,
    uint *src_key, uint *src_val,
    int n, int sort_dir);

/* ── Input generation ──────────────────────────────────────── */

static void generate_input(std::vector<uint> &keys, std::vector<uint> &vals, int n)
{
    std::srand(2024);
    keys.resize(n);
    vals.resize(n);
    for (int i = 0; i < n; i++) {
        keys[i] = static_cast<uint>(std::rand()) % 65536;
        vals[i] = static_cast<uint>(i);  /* identity value for stability check */
    }
}

/* ── CPU reference sort ────────────────────────────────────── */

static void host_reference_sort(
    const std::vector<uint> &src_key, const std::vector<uint> &src_val,
    std::vector<uint> &dst_key, std::vector<uint> &dst_val, int n)
{
    /* Stable sort by key using std::stable_sort */
    std::vector<std::pair<uint, uint>> kv(n);
    for (int i = 0; i < n; i++) {
        kv[i] = {src_key[i], src_val[i]};
    }
    std::stable_sort(kv.begin(), kv.end(),
        [](const auto &a, const auto &b) { return a.first < b.first; });
    dst_key.resize(n);
    dst_val.resize(n);
    for (int i = 0; i < n; i++) {
        dst_key[i] = kv[i].first;
        dst_val[i] = kv[i].second;
    }
}

/* ── Verification ──────────────────────────────────────────── */

static bool verify_sorted(const std::vector<uint> &keys, int n)
{
    for (int i = 1; i < n; i++) {
        if (keys[i] < keys[i - 1]) {
            printf("  FAIL: keys[%d]=%u > keys[%d]=%u\n", i - 1, keys[i - 1], i, keys[i]);
            return false;
        }
    }
    return true;
}

static bool verify_stable(
    const std::vector<uint> &keys, const std::vector<uint> &vals,
    const std::vector<uint> &ref_keys, const std::vector<uint> &ref_vals, int n)
{
    for (int i = 0; i < n; i++) {
        if (keys[i] != ref_keys[i] || vals[i] != ref_vals[i]) {
            printf("  FAIL at [%d]: got (%u,%u) expected (%u,%u)\n",
                   i, keys[i], vals[i], ref_keys[i], ref_vals[i]);
            return false;
        }
    }
    return true;
}

/* ── Main ──────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    printf("[merge_sort_bitonic] Block Sort + Bitonic Merge Demo\n\n");

    findHggcDevice(argc, const_cast<const char **>(argv));

    /* Configuration: N must be power of 2, max 2*BLOCK_CAPACITY (1024) */
    const int N = 1024;
    printf("  Elements: %d\n", N);
    printf("  Block capacity: 512\n\n");

    /* Generate input */
    std::vector<uint> h_src_key(N), h_src_val(N);
    generate_input(h_src_key, h_src_val, N);

    /* CPU reference sort */
    std::vector<uint> h_ref_key, h_ref_val;
    host_reference_sort(h_src_key, h_src_val, h_ref_key, h_ref_val, N);

    /* Allocate device memory */
    size_t bytes = N * sizeof(uint);
    uint *d_src_key, *d_src_val, *d_buf_key, *d_buf_val, *d_dst_key, *d_dst_val;
    checkHggcErrors(hggcMalloc((void **)&d_src_key, bytes));
    checkHggcErrors(hggcMalloc((void **)&d_src_val, bytes));
    checkHggcErrors(hggcMalloc((void **)&d_buf_key, bytes));
    checkHggcErrors(hggcMalloc((void **)&d_buf_val, bytes));
    checkHggcErrors(hggcMalloc((void **)&d_dst_key, bytes));
    checkHggcErrors(hggcMalloc((void **)&d_dst_val, bytes));

    checkHggcErrors(hggcMemcpy(d_src_key, h_src_key.data(), bytes, hggcMemcpyHostToDevice));
    checkHggcErrors(hggcMemcpy(d_src_val, h_src_val.data(), bytes, hggcMemcpyHostToDevice));

    /* Run device sort */
    printf("  Running device sort...\n");
    HggcTimer timer;
    timer.reset();
    timer.start();

    parallel_sort(d_dst_key, d_dst_val, d_buf_key, d_buf_val,
                  d_src_key, d_src_val, N, 1);

    checkHggcErrors(hggcDeviceSynchronize());
    timer.stop();
    printf("  Device sort time: %.3f ms\n\n", timer.elapsed());

    /* Read back results */
    std::vector<uint> h_dst_key(N), h_dst_val(N);
    checkHggcErrors(hggcMemcpy(h_dst_key.data(), d_dst_key, bytes, hggcMemcpyDeviceToHost));
    checkHggcErrors(hggcMemcpy(h_dst_val.data(), d_dst_val, bytes, hggcMemcpyDeviceToHost));

    /* Verify */
    printf("  Verifying sort order...\n");
    bool sorted_ok = verify_sorted(h_dst_key, N);

    printf("  Verifying key-value stability...\n");
    bool stable_ok = verify_stable(h_dst_key, h_dst_val, h_ref_key, h_ref_val, N);

    /* Cleanup */
    checkHggcErrors(hggcFree(d_dst_val));
    checkHggcErrors(hggcFree(d_dst_key));
    checkHggcErrors(hggcFree(d_buf_val));
    checkHggcErrors(hggcFree(d_buf_key));
    checkHggcErrors(hggcFree(d_src_val));
    checkHggcErrors(hggcFree(d_src_key));

    if (sorted_ok && stable_ok) {
        printf("\n  Result: PASS\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n  Result: FAIL\n");
        return EXIT_FAILURE;
    }
}
