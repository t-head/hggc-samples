/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * CPU reference implementation for GEMV:  y = A · x
 * Used to verify the PPU kernel result.
 */
extern "C" void gemv_reference(float *y, const float *A, const float *x,
                               int M, int N);

void gemv_reference(float *y, const float *A, const float *x, int M, int N)
{
    for (int i = 0; i < M; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < N; ++j) {
            sum += A[i * N + j] * x[j];
        }
        y[i] = sum;
    }
}
