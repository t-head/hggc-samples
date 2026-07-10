/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * convolution_separable — Shared declarations
 */
#ifndef CONVOLUTION_SEPARABLE_COMMON_H
#define CONVOLUTION_SEPARABLE_COMMON_H

// Gaussian kernel half-width.  Full kernel length = 2*RADIUS + 1.
constexpr int KERNEL_RADIUS = 8;
constexpr int KERNEL_LENGTH = 2 * KERNEL_RADIUS + 1;

// Tile width for the row/column kernels.
constexpr int TILE_W = 128;  // pixels processed per block in the filter direction
constexpr int TILE_H = 8;    // rows (or cols) per block in the perpendicular dir

// ---- CPU reference (in convolutionSeparable_gold.cpp) ----
extern "C" void convolution_row_cpu(
    float *dst, const float *src, const float *kernel,
    int width, int height, int radius);

extern "C" void convolution_col_cpu(
    float *dst, const float *src, const float *kernel,
    int width, int height, int radius);

// ---- PPU kernels (in convolutionSeparable.hg) ----
extern "C" void upload_kernel_weights(const float *h_kernel);
extern "C" void convolution_row_ppu(float *d_dst, const float *d_src, int w, int h);
extern "C" void convolution_col_ppu(float *d_dst, const float *d_src, int w, int h);

#endif  // CONVOLUTION_SEPARABLE_COMMON_H
