/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 */
#ifndef MPI_PPU_DISPATCH_H
#define MPI_PPU_DISPATCH_H

extern "C" {
void  init_temperatures(float *data, int n);
void  compute_gradient(float *host_data, int block_size, int grid_size);
void  report_stats(float *data, int n, float *out_min, float *out_max, float *out_mean);
void  abort_mpi(int err);
}

#endif /* MPI_PPU_DISPATCH_H */
