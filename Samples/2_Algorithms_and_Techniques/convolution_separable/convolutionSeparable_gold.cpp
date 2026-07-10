/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * convolutionSeparable_gold.cpp — CPU reference for separable convolution
 */
#include "convolutionSeparable_common.h"

/// Row-wise 1-D convolution with zero-padded boundary.
extern "C" void convolution_row_cpu(
    float *dst, const float *src, const float *kernel,
    int width, int height, int radius)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int sx = x + k;
                float val = (sx >= 0 && sx < width) ? src[y * width + sx] : 0.0f;
                sum += val * kernel[radius + k];
            }
            dst[y * width + x] = sum;
        }
    }
}

/// Column-wise 1-D convolution with zero-padded boundary.
extern "C" void convolution_col_cpu(
    float *dst, const float *src, const float *kernel,
    int width, int height, int radius)
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;
            for (int k = -radius; k <= radius; ++k) {
                int sy = y + k;
                float val = (sy >= 0 && sy < height) ? src[sy * width + x] : 0.0f;
                sum += val * kernel[radius + k];
            }
            dst[y * width + x] = sum;
        }
    }
}
