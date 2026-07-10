/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * histogram_gold.cpp — CPU reference for 256-bin byte histogram
 */
#include <cstring>

#include "histogram_common.h"

void histogram_cpu(uint32_t *hist, const uint8_t *data, int n)
{
    memset(hist, 0, NUM_BINS * sizeof(uint32_t));
    for (int i = 0; i < n; ++i) {
        hist[data[i]]++;
    }
}
