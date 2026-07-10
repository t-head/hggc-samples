/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 */
#ifndef MATMUL_DIMS_H_
#define MATMUL_DIMS_H_

// Outer matrix shape (in terms of the runtime tile variable @c blockSize).
#define MAT_A_W (4 * blockSize)
#define MAT_A_H (6 * blockSize)
#define MAT_B_W (4 * blockSize)
#define MAT_B_H MAT_A_W
#define MAT_C_W MAT_B_W
#define MAT_C_H MAT_A_H

#endif  // MATMUL_DIMS_H_
