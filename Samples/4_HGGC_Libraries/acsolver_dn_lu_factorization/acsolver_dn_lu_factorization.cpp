/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * ---------------------------------------------------------------------------
 * LU Factorization Deep Dive
 * ---------------------------------------------------------------------------
 * This sample demonstrates LU factorization with partial pivoting using acsolverDn,
 * providing a detailed examination of the factorization result's internal structure.
 *
 * Unlike a simple "factorize then solve" workflow, this program:
 *   1. Programmatically generates a diagonally dominant random matrix A and known solution x_true
 *   2. Performs LU factorization on PPU (A = P·L·U), extracts and prints L, U factors and pivot sequence
 *   3. Verifies P·A = L·U on host (reconstruction consistency check)
 *   4. Solves A·x = b using the factorization result and compares with the known solution
 *   5. Computes residual ||b - A·x|| / (||A||·||x||)
 *
 * Matrix order can be specified via -N command-line argument (default 5 for readable console output).
 * ---------------------------------------------------------------------------
 */
#include <hggc_runtime.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "acblas_v2.h"
#include "acsolverDn.h"
#include "helper_hggc.h"

/* ------------------------------------------------------------------ */
/*  Constants                                                            */
/* ------------------------------------------------------------------ */
#define DEFAULT_N 5  /* Default matrix order: small enough to print L and U */

/* ------------------------------------------------------------------ */
/*  Helper functions                                                     */
/* ------------------------------------------------------------------ */

/* Wall-clock time in seconds */
static double wallTime(void)
{
#if defined(__linux__)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1.0e-6;
#else
    return (double)clock() / CLOCKS_PER_SEC;
#endif
}

/* Vector infinity norm */
static double vecNormInf(int n, const double *x)
{
    double best = 0.0;
    for (int i = 0; i < n; i++) {
        double v = fabs(x[i]);
        if (v > best) best = v;
    }
    return best;
}

/* Matrix infinity norm (column-major, lda is leading dimension) */
static double matNormInf(int m, int n, const double *A, int lda)
{
    double best = 0.0;
    for (int i = 0; i < m; i++) {
        double rowSum = 0.0;
        for (int j = 0; j < n; j++) {
            rowSum += fabs(A[i + j * lda]);
        }
        if (rowSum > best) best = rowSum;
    }
    return best;
}

/* Print column-major matrix */
static void printMatrix(const char *name, const double *M, int rows, int cols, int lda)
{
    printf("%s [%dx%d] =\n", name, rows, cols);
    for (int i = 0; i < rows; i++) {
        printf("   ");
        for (int j = 0; j < cols; j++) {
            printf(" %12.6f", M[i + j * lda]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Print vector */
static void printVector(const char *name, const double *v, int n)
{
    printf("%s =\n   ", name);
    for (int i = 0; i < n; i++) {
        printf(" %12.6f", v[i]);
    }
    printf("\n\n");
}

/* Generate diagonally dominant random matrix (column-major) */
static void generateMatrix(double *A, int n, int lda, unsigned int seed)
{
    srand(seed);
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            A[i + j * lda] = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0;
        }
        /* Add n times diagonal to ensure strict diagonal dominance */
        A[j + j * lda] += (double)n;
    }
}

/* Extract lower triangular factor L (unit diagonal) from compact LU storage */
static void extractL(const double *LU, double *L, int n, int lda)
{
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            if (i > j)
                L[i + j * lda] = LU[i + j * lda];
            else if (i == j)
                L[i + j * lda] = 1.0;
            else
                L[i + j * lda] = 0.0;
        }
    }
}

/* Extract upper triangular factor U (including diagonal) from compact LU storage */
static void extractU(const double *LU, double *U, int n, int lda)
{
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            if (i <= j)
                U[i + j * lda] = LU[i + j * lda];
            else
                U[i + j * lda] = 0.0;
        }
    }
}

/*
 * Apply row permutation to matrix M according to LAPACK ipiv convention (forward).
 * ipiv is 1-based, i.e., ipiv[i]-1 is the 0-based row index.
 * Applying the pivot permutation forward to A yields P·A.
 */
static void applyPivotForward(const int *ipiv, double *M, int n, int lda)
{
    for (int i = 0; i < n; i++) {
        int pivot = ipiv[i] - 1;  /* Convert to 0-based */
        if (pivot != i) {
            for (int j = 0; j < n; j++) {
                double tmp         = M[i + j * lda];
                M[i + j * lda]     = M[pivot + j * lda];
                M[pivot + j * lda] = tmp;
            }
        }
    }
}

/* Host-side matrix multiplication C = alpha * A * B + beta * C (column-major) */
static void hostMatmul(double alpha, const double *A, int lda,
                       const double *B, int ldb,
                       double beta, double *C, int ldc,
                       int m, int k, int n)
{
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            double sum = 0.0;
            for (int p = 0; p < k; p++) {
                sum += A[i + p * lda] * B[p + j * ldb];
            }
            C[i + j * ldc] = alpha * sum + beta * C[i + j * ldc];
        }
    }
}

/*
 * Verify P·A = L·U
 * Returns ||P·A - L·U||_inf
 */
static double verifyFactorization(const double *A_orig, const double *L,
                                  const double *U, const int *ipiv,
                                  int n, int lda)
{
    /* Compute L·U */
    double *LU_prod = (double *)malloc(sizeof(double) * lda * n);
    memset(LU_prod, 0, sizeof(double) * lda * n);
    hostMatmul(1.0, L, lda, U, lda, 0.0, LU_prod, lda, n, n, n);

    /* Apply pivot permutation to A to get P·A */
    double *PA = (double *)malloc(sizeof(double) * lda * n);
    memcpy(PA, A_orig, sizeof(double) * lda * n);
    applyPivotForward(ipiv, PA, n, lda);

    /* Compute difference */
    double maxErr = 0.0;
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            double diff = fabs(PA[i + j * lda] - LU_prod[i + j * lda]);
            if (diff > maxErr) maxErr = diff;
        }
    }

    free(LU_prod);
    free(PA);
    return maxErr;
}

/* ------------------------------------------------------------------ */
/*  Command-line help                                                         */
/* ------------------------------------------------------------------ */
static void showUsage(void)
{
    printf("Usage: acsolver_dn_lu_factorization [options]\n");
    printf("  -N=<int>      Matrix order (default %d)\n", DEFAULT_N);
    printf("  -seed=<int>   Random seed (default time-based)\n");
    printf("  -device=<id>  Specify PPU device\n");
    printf("  -h            Show this help\n");
    exit(0);
}

/* ------------------------------------------------------------------ */
/*  Main program                                                             */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    /* ---- Parse command-line arguments ---- */
    if (hasArg(argc, (const char **)argv, "h")) {
        showUsage();
    }

    int n = DEFAULT_N;
    if (hasArg(argc, (const char **)argv, "N")) {
        n = getArgInt(argc, (const char **)argv, "N");
        if (n < 2) {
            fprintf(stderr, "Error: N must be >= 2\n");
            return EXIT_FAILURE;
        }
    }

    unsigned int seed = (unsigned int)time(NULL);
    if (hasArg(argc, (const char **)argv, "seed")) {
        seed = (unsigned int)getArgInt(argc, (const char **)argv, "seed");
    }

    int lda = n;  /* Leading dimension */

    /* ---- Select device ---- */
    findHggcDevice(argc, (const char **)argv);

    printf("============================================================\n");
    printf("  LU Factorization Deep Dive (acsolverDn)\n");
    printf("  Matrix order N = %d, lda = %d, random seed = %u\n", n, lda, seed);
    printf("============================================================\n\n");

    /* ---- Host memory allocation ---- */
    double *h_A      = (double *)malloc(sizeof(double) * lda * n);  /* Original matrix A */
    double *h_A_copy = (double *)malloc(sizeof(double) * lda * n);  /* Copy of A (for verification) */
    double *h_LU     = (double *)malloc(sizeof(double) * lda * n);  /* Compact LU after factorization */
    double *h_L      = (double *)malloc(sizeof(double) * lda * n);  /* Lower triangular factor L */
    double *h_U      = (double *)malloc(sizeof(double) * lda * n);  /* Upper triangular factor U */
    double *h_x_true = (double *)malloc(sizeof(double) * n);        /* Known true solution */
    double *h_b      = (double *)malloc(sizeof(double) * n);        /* Right-hand side b = A*x_true */
    double *h_x      = (double *)malloc(sizeof(double) * n);        /* Computed solution x */
    double *h_r      = (double *)malloc(sizeof(double) * n);        /* Residual r = b - A*x */
    int    *h_ipiv   = (int *)malloc(sizeof(int) * n);              /* Pivot indices */

    assert(h_A && h_A_copy && h_LU && h_L && h_U &&
           h_x_true && h_b && h_x && h_r && h_ipiv);

    /* ---- Step 1: Generate test matrix and known solution ---- */
    printf("Step 1: Generate %dx%d diagonally dominant random matrix\n", n, n);

    generateMatrix(h_A, n, lda, seed);
    memcpy(h_A_copy, h_A, sizeof(double) * lda * n);

    /* Generate known solution x_true = [1, 2, ..., n] */
    for (int i = 0; i < n; i++) {
        h_x_true[i] = (double)(i + 1);
    }

    /* Compute b = A * x_true (host-side) */
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += h_A[i + j * lda] * h_x_true[j];
        }
        h_b[i] = sum;
    }

    if (n <= 10) {
        printMatrix("A", h_A, n, n, lda);
        printVector("x_true", h_x_true, n);
        printVector("b", h_b, n);
    }

    /* ---- Step 2: Initialize acsolverDn and acblas handles ---- */
    printf("Step 2: Create acsolverDn / acblas handles\n");

    acsolverDnHandle_t solverHandle = NULL;
    acblasHandle_t     blasHandle   = NULL;
    hggcStream_t       stream       = NULL;

    checkHggcErrors(acsolverDnCreate(&solverHandle));
    checkHggcErrors(acblasCreate(&blasHandle));
    checkHggcErrors(hggcStreamCreate(&stream));

    checkHggcErrors(acsolverDnSetStream(solverHandle, stream));
    checkHggcErrors(acblasSetStream(blasHandle, stream));

    /* ---- Step 3: Device memory allocation and data transfer ---- */
    printf("Step 3: Allocate PPU memory and transfer data\n");

    double *d_A    = NULL;  /* Matrix after factorization (overwritten with compact LU) */
    double *d_b    = NULL;  /* Right-hand side (solution stored in-place) */
    double *d_r    = NULL;  /* Residual vector */
    double *d_Aref = NULL;  /* Copy of original matrix (for residual computation) */
    int    *d_ipiv = NULL;  /* Device-side pivot indices */
    int    *d_info = NULL;  /* Device-side status code */

    checkHggcErrors(hggcMalloc((void **)&d_A,    sizeof(double) * lda * n));
    checkHggcErrors(hggcMalloc((void **)&d_b,    sizeof(double) * n));
    checkHggcErrors(hggcMalloc((void **)&d_r,    sizeof(double) * n));
    checkHggcErrors(hggcMalloc((void **)&d_Aref, sizeof(double) * lda * n));
    checkHggcErrors(hggcMalloc((void **)&d_ipiv, sizeof(int) * n));
    checkHggcErrors(hggcMalloc((void **)&d_info, sizeof(int)));

    checkHggcErrors(hggcMemcpy(d_A,    h_A, sizeof(double) * lda * n, hggcMemcpyHostToDevice));
    checkHggcErrors(hggcMemcpy(d_Aref, h_A, sizeof(double) * lda * n, hggcMemcpyHostToDevice));
    checkHggcErrors(hggcMemcpy(d_b,    h_b, sizeof(double) * n,       hggcMemcpyHostToDevice));
    checkHggcErrors(hggcMemset(d_info, 0, sizeof(int)));

    /* Pre-declare variables used after goto */
    double reconErr = 0.0;
    double x_err = 0.0;
    double minus_one = -1.0;
    double one = 1.0;
    double r_inf = 0.0;
    double A_inf = 0.0;
    double x_inf = 0.0;

    /* ---- Step 4: LU factorization (acsolverDnDgetrf) ---- */
    printf("Step 4: Perform LU factorization (acsolverDnDgetrf)\n");

    int bufferSize = 0;
    double *d_buffer = NULL;

    checkHggcErrors(acsolverDnDgetrf_bufferSize(solverHandle, n, n, d_A, lda, &bufferSize));
    printf("       Workspace size = %d bytes\n", bufferSize);

    checkHggcErrors(hggcMalloc((void **)&d_buffer, sizeof(double) * bufferSize));

    double t0 = wallTime();

    checkHggcErrors(acsolverDnDgetrf(solverHandle, n, n, d_A, lda, d_buffer, d_ipiv, d_info));

    checkHggcErrors(hggcDeviceSynchronize());
    double t1 = wallTime();

    printf("       Factorization time = %.6f s\n", t1 - t0);

    /* Check status code */
    int h_info = 0;
    checkHggcErrors(hggcMemcpy(&h_info, d_info, sizeof(int), hggcMemcpyDeviceToHost));
    if (h_info != 0) {
        fprintf(stderr, "Error: LU factorization failed, info = %d", h_info);
        if (h_info > 0) {
            fprintf(stderr, "(U(%d,%d) = 0, matrix is singular)\n", h_info, h_info);
        }
        fprintf(stderr, "\n");
        goto cleanup;
    }
    printf("       Factorization succeeded (info = 0)\n\n");

    /* ---- Step 5: Extract and display L, U factors and pivot sequence ---- */
    printf("Step 5: Extract L, U factors and display pivot sequence\n");

    /* Copy compact LU and ipiv back to host */
    checkHggcErrors(hggcMemcpy(h_LU,   d_A,    sizeof(double) * lda * n, hggcMemcpyDeviceToHost));
    checkHggcErrors(hggcMemcpy(h_ipiv, d_ipiv, sizeof(int) * n,          hggcMemcpyDeviceToHost));

    extractL(h_LU, h_L, n, lda);
    extractU(h_LU, h_U, n, lda);

    if (n <= 10) {
        printMatrix("L (unit lower triangular)", h_L, n, n, lda);
        printMatrix("U (upper triangular)", h_U, n, n, lda);

        printf("Pivot sequence ipiv =\n   ");
        for (int i = 0; i < n; i++) {
            printf(" %d", h_ipiv[i]);
        }
        printf("\n\n");
    }

    /* ---- Step 6: Verify P·A = L·U ---- */
    printf("Step 6: Verify P·A = L·U\n");

    reconErr = verifyFactorization(h_A_copy, h_L, h_U, h_ipiv, n, lda);
    printf("       ||P·A - L·U||_inf = %E\n", reconErr);

    if (reconErr < 1e-10) {
        printf("       ✓ Factorization consistency verified\n\n");
    } else {
        printf("       ✗ Warning: large factorization reconstruction error\n\n");
    }

    /* ---- Step 7: Solve A·x = b (acsolverDnDgetrs) ---- */
    printf("Step 7: Solve A·x = b using LU factorization (acsolverDnDgetrs)\n");

    /* Save original b to d_r (getrs overwrites d_b in-place with solution x) */
    checkHggcErrors(hggcMemcpy(d_r, d_b, sizeof(double) * n, hggcMemcpyDeviceToDevice));

    t0 = wallTime();
    checkHggcErrors(acsolverDnDgetrs(solverHandle, ACBLAS_OP_N, n, 1,
                                     d_A, lda, d_ipiv, d_b, n, d_info));
    checkHggcErrors(hggcDeviceSynchronize());
    t1 = wallTime();

    printf("       Solve time = %.6f s\n", t1 - t0);

    checkHggcErrors(hggcMemcpy(h_x, d_b, sizeof(double) * n, hggcMemcpyDeviceToHost));

    /* Check solve status */
    checkHggcErrors(hggcMemcpy(&h_info, d_info, sizeof(int), hggcMemcpyDeviceToHost));
    if (h_info != 0) {
        fprintf(stderr, "Error: triangular solve failed, info = %d\n", h_info);
        goto cleanup;
    }

    if (n <= 10) {
        printVector("x (computed solution)", h_x, n);
        printVector("x_true", h_x_true, n);
    }

    /* ---- Step 8: Verify solution accuracy ---- */
    printf("Step 8: Verify solution accuracy\n");

    /* Error between x and x_true */
    x_err = 0.0;
    for (int i = 0; i < n; i++) {
        double diff = fabs(h_x[i] - h_x_true[i]);
        if (diff > x_err) x_err = diff;
    }

    /* Compute residual r = b - A·x (PPU side)
     * d_r already holds original b saved before getrs, d_b is now solution x
     * Execute d_r = -A * d_b + d_r = -A*x + b = b - A*x */
    minus_one = -1.0;
    one       = 1.0;
    checkHggcErrors(acblasDgemm_v2(
        blasHandle, ACBLAS_OP_N, ACBLAS_OP_N, n, 1, n,
        &minus_one, d_Aref, lda, d_b, n, &one, d_r, n));

    checkHggcErrors(hggcMemcpy(h_r, d_r, sizeof(double) * n, hggcMemcpyDeviceToHost));

    r_inf = vecNormInf(n, h_r);
    A_inf = matNormInf(n, n, h_A, lda);
    x_inf = vecNormInf(n, h_x);

    printf("       ||x - x_true||_inf        = %E\n", x_err);
    printf("       ||b - A·x||_inf           = %E\n", r_inf);
    printf("       ||A||_inf                  = %E\n", A_inf);
    printf("       ||x||_inf                  = %E\n", x_inf);
    printf("       Relative residual ||b-A·x||/(||A||·||x||) = %E\n", r_inf / (A_inf * x_inf));

    if (x_err < 1e-8) {
        printf("       ✓ Solution accuracy verified\n\n");
    } else {
        printf("       ✗ Warning: large solution error\n\n");
    }

    /* ---- Summary ---- */
    printf("============================================================\n");
    printf("  Demo complete\n");
    printf("  Matrix order:       %d\n", n);
    printf("  Reconstruction err: %E\n", reconErr);
    printf("  Max solution err:   %E\n", x_err);
    printf("  Relative residual:  %E\n", r_inf / (A_inf * x_inf));
    printf("============================================================\n");

cleanup:
    /* ---- Release resources ---- */
    if (d_buffer)  checkHggcErrors(hggcFree(d_buffer));
    if (d_A)       checkHggcErrors(hggcFree(d_A));
    if (d_b)       checkHggcErrors(hggcFree(d_b));
    if (d_r)       checkHggcErrors(hggcFree(d_r));
    if (d_Aref)    checkHggcErrors(hggcFree(d_Aref));
    if (d_ipiv)    checkHggcErrors(hggcFree(d_ipiv));
    if (d_info)    checkHggcErrors(hggcFree(d_info));

    if (solverHandle) checkHggcErrors(acsolverDnDestroy(solverHandle));
    if (blasHandle)   checkHggcErrors(acblasDestroy(blasHandle));
    if (stream)       checkHggcErrors(hggcStreamDestroy(stream));

    if (h_A)      free(h_A);
    if (h_A_copy) free(h_A_copy);
    if (h_LU)     free(h_LU);
    if (h_L)      free(h_L);
    if (h_U)      free(h_U);
    if (h_x_true) free(h_x_true);
    if (h_b)      free(h_b);
    if (h_x)      free(h_x);
    if (h_r)      free(h_r);
    if (h_ipiv)   free(h_ipiv);

    return 0;
}
