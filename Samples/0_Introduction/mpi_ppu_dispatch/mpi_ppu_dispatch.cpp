/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * mpi_ppu_dispatch.cpp -- MPI + PPU temperature gradient demo
 *
 * Each MPI rank receives a segment of a 1D temperature profile,
 * computes the gradient (central difference) on PPU, then gathers
 * results back to root for statistics and verification.
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "mpi_ppu_dispatch.h"

#define MPI_CHK(call)                                                       \
    do {                                                                    \
        if ((call) != MPI_SUCCESS) {                                        \
            std::cerr << "MPI error: \"" #call "\"" << std::endl;           \
            abort_mpi(-1);                                                  \
        }                                                                   \
    } while (0)

namespace {
constexpr int kBlockSize = 256;
constexpr int kGridSize  = 64;  /* elements per rank = 256 * 64 = 16384 */
}

int main(int argc, char *argv[])
{
    const int per_rank = kBlockSize * kGridSize;

    MPI_CHK(MPI_Init(&argc, &argv));

    int num_ranks = 0, rank = 0;
    MPI_CHK(MPI_Comm_size(MPI_COMM_WORLD, &num_ranks));
    MPI_CHK(MPI_Comm_rank(MPI_COMM_WORLD, &rank));

    const int total_elements = per_rank * num_ranks;

    /* Root generates temperature profile */
    float *root_temps = nullptr;
    if (rank == 0) {
        printf("[mpi_ppu_dispatch] Temperature Gradient via MPI + PPU\n");
        printf("  Ranks: %d, Elements/rank: %d, Total: %d\n\n",
               num_ranks, per_rank, total_elements);
        root_temps = new float[total_elements];
        init_temperatures(root_temps, total_elements);
    }

    /* Scatter temperature segments to all ranks */
    float *local_temps = new float[per_rank];
    MPI_CHK(MPI_Scatter(root_temps, per_rank, MPI_FLOAT,
                        local_temps, per_rank, MPI_FLOAT,
                        0, MPI_COMM_WORLD));

    if (rank == 0) {
        delete[] root_temps;
    }

    /* Compute gradient on PPU (in-place: temps → gradient) */
    compute_gradient(local_temps, kBlockSize, kGridSize);

    /* Gather gradients back to root */
    float *root_grads = nullptr;
    if (rank == 0) {
        root_grads = new float[total_elements];
    }

    MPI_CHK(MPI_Gather(local_temps, per_rank, MPI_FLOAT,
                       root_grads, per_rank, MPI_FLOAT,
                       0, MPI_COMM_WORLD));

    /* Root computes statistics and verifies against CPU reference */
    if (rank == 0) {
        /* Re-generate original temperatures for CPU verification */
        float *ref_temps = new float[total_elements];
        init_temperatures(ref_temps, total_elements);

        /* CPU reference gradient */
        float *ref_grads = new float[total_elements];
        ref_grads[0] = ref_temps[1] - ref_temps[0];
        ref_grads[total_elements - 1] = ref_temps[total_elements - 1] - ref_temps[total_elements - 2];
        for (int i = 1; i < total_elements - 1; i++) {
            ref_grads[i] = (ref_temps[i + 1] - ref_temps[i - 1]) * 0.5f;
        }

        /* Statistics on PPU results */
        float gmin, gmax, gmean;
        report_stats(root_grads, total_elements, &gmin, &gmax, &gmean);

        /* Verify against CPU reference (skip rank boundaries: first/last
           element of each rank uses forward/backward diff, not central) */
        float max_err = 0.0f;
        int verified = 0;
        for (int i = 0; i < total_elements; i++) {
            /* Skip global boundaries and rank boundaries */
            bool is_boundary = (i == 0) || (i == total_elements - 1);
            bool is_rank_boundary = (i % per_rank == 0) || (i % per_rank == per_rank - 1);
            if (is_boundary || is_rank_boundary) continue;

            float err = root_grads[i] - ref_grads[i];
            if (err < 0) err = -err;
            if (err > max_err) max_err = err;
            verified++;
        }

        printf("  Gradient Statistics:\n");
        printf("    Min   : %.4f\n", gmin);
        printf("    Max   : %.4f\n", gmax);
        printf("    Mean  : %.4f\n", gmean);
        printf("    Expected ~%.2f (uniform gradient)\n", 100.0f / total_elements);
        printf("\n  Verification:\n");
        printf("    Verified interior elements: %d (rank boundaries skipped)\n", verified);
        printf("    Max error vs CPU: %.6e\n", max_err);
        printf("    Result: %s\n", max_err < 1e-4 ? "PASS" : "FAIL");

        delete[] ref_temps;
        delete[] ref_grads;
        delete[] root_grads;
    }

    delete[] local_temps;
    MPI_CHK(MPI_Finalize());

    return 0;
}

void abort_mpi(int err)
{
    fprintf(stderr, "Aborting (code %d)\n", err);
    MPI_Abort(MPI_COMM_WORLD, err);
}
