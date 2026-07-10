/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * histogram_common.h — Shared declarations for the histogram sample
 */
#ifndef HISTOGRAM_COMMON_H
#define HISTOGRAM_COMMON_H

#include <cstdint>

constexpr int NUM_BINS   = 256;
constexpr int kBlockSize = 256;

// ---- CPU reference (histogram_gold.cpp) ----
void histogram_cpu(uint32_t *hist, const uint8_t *data, int n);

// ---- PPU kernels (histogram.hg) ----
// Three strategies with increasing optimization level:
void histogram_global_atomic(uint32_t *d_hist, const uint8_t *d_data, int n);
void histogram_shared_private(uint32_t *d_hist, const uint8_t *d_data, int n);
void histogram_warp_private(uint32_t *d_hist, const uint8_t *d_data, int n);

#endif  // HISTOGRAM_COMMON_H
