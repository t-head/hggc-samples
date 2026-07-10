/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * simple_acblas_lu -- LU Decomposition via BLAS Level 1/2/3 Primitives
 *
 * Demonstrates manual LU decomposition with partial pivoting using only
 * basic acblas operations (swap, scal, ger, gemm, trsm), without calling
 * the high-level getrf routine. This approach shows how BLAS primitives
 * compose into higher-level linear algebra.
 *
 * Algorithm (column-major, Doolittle with partial pivoting):
 *   for k = 0..n-1:
 *     1. Find pivot row (host-side max search on column k)
 *     2. acblasDswap  — swap rows k and pivot
 *     3. acblasDscal  — scale column k below diagonal by 1/A[k,k]
 *     4. acblasDger   — rank-1 update of trailing submatrix
 *
 * Verification:
 *   - acblasDgemm  — compute L*U, compare with permuted P*A
 *   - acblasDtrsm  — solve Ax=b via forward/back substitution
 */
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include <hggc_runtime.h>
#include <acblas_v2.h>
#include <helper_hggc.h>
#include <helper_functions.h>

/* ── Configuration ───────────────────────────────────────────── */
#define N           8
#define TOL         1e-12

typedef double real_t;

/* ── Host helper functions ───────────────────────────────────── */

static void print_matrix(const char *name, real_t *M, int n)
{
    printf("  %s =\n", name);
    for (int i = 0; i < n; i++) {
        printf("    ");
        for (int j = 0; j < n; j++)
            printf("%10.4f ", M[j * n + i]);
        printf("\n");
    }
    printf("\n");
}

static void init_random_matrix(real_t *A, int n)
{
    srand(42);
    for (int i = 0; i < n * n; i++)
        A[i] = (real_t)rand() / RAND_MAX;
    /* Diagonal dominance to ensure non-singular */
    for (int i = 0; i < n; i++)
        A[i * n + i] += (real_t)n;
}

/* ── LU decomposition via BLAS primitives ────────────────────── */

static int lu_decompose(acblasHandle_t handle, real_t *d_A, int n,
                        int *pivot, real_t *h_colbuf)
{
    real_t neg_one = -1.0;

    for (int k = 0; k < n; k++) {
        /* ── Step 1: Pivot selection (host-side) ──
         * Copy column k (rows k..n-1) to host, find max absolute value.
         */
        int remaining = n - k;
        checkHggcErrors(hggcMemcpy(h_colbuf, &d_A[k + k * n],
                         remaining * sizeof(real_t),
                         hggcMemcpyDeviceToHost));

        int pivot_row = 0;
        real_t max_val = fabs(h_colbuf[0]);
        for (int i = 1; i < remaining; i++) {
            if (fabs(h_colbuf[i]) > max_val) {
                max_val = fabs(h_colbuf[i]);
                pivot_row = i;
            }
        }
        pivot_row += k;  /* absolute row index */
        pivot[k] = pivot_row;

        if (max_val < 1e-30) {
            fprintf(stderr, "  ERROR: singular matrix at column %d\n", k);
            return -1;
        }

        /* ── Step 2: Swap rows k and pivot_row ──
         * In column-major, row i = &A[i] with stride n (lda).
         * acblasDswap swaps n elements with the given strides.
         */
        if (pivot_row != k) {
            checkHggcErrors(acblasDswap(handle, n,
                &d_A[k],          n,   /* row k: start at A[k], stride n */
                &d_A[pivot_row],  n)); /* row p: start at A[p], stride n */
        }

        /* ── Step 3: Scale column k below diagonal ──
         * A[k+1:n, k] *= 1/A[k,k]
         * Read pivot element from device.
         */
        real_t diag_val;
        checkHggcErrors(hggcMemcpy(&diag_val, &d_A[k + k * n],
                         sizeof(real_t), hggcMemcpyDeviceToHost));
        real_t inv_diag = 1.0 / diag_val;

        if (k < n - 1) {
            checkHggcErrors(acblasDscal(handle, n - k - 1, &inv_diag,
                &d_A[(k + 1) + k * n], 1));  /* column vector, stride 1 */
        }

        /* ── Step 4: Rank-1 update of trailing submatrix ──
         * A[k+1:n, k+1:n] -= A[k+1:n, k] * A[k, k+1:n]
         *
         * x = column k below diagonal: &A[(k+1) + k*n], incx = 1
         * y = row k right of diagonal: &A[k + (k+1)*n], incy = n
         * A_sub = trailing submatrix:  &A[(k+1) + (k+1)*n], lda = n
         */
        if (k < n - 1) {
            int sub_m = n - k - 1;
            int sub_n = n - k - 1;
            checkHggcErrors(acblasDger(handle, sub_m, sub_n, &neg_one,
                &d_A[(k + 1) + k * n], 1,       /* x: column, stride 1 */
                &d_A[k + (k + 1) * n], n,       /* y: row, stride n   */
                &d_A[(k + 1) + (k + 1) * n], n)); /* A: submatrix, lda n */
        }
    }

    return 0;
}

/* ── Verification via acblasDgemm ────────────────────────────── */

static int verify_lu(acblasHandle_t handle, real_t *d_LU,
                     real_t *h_A_orig, int n, int *pivot)
{
    int nn = n * n;
    real_t *h_LU = (real_t *)malloc(nn * sizeof(real_t));
    real_t *h_L  = (real_t *)calloc(nn, sizeof(real_t));
    real_t *h_U  = (real_t *)calloc(nn, sizeof(real_t));
    real_t *h_LU_check = (real_t *)calloc(nn, sizeof(real_t));

    /* Copy LU result from device */
    checkHggcErrors(hggcMemcpy(h_LU, d_LU, nn * sizeof(real_t),
                     hggcMemcpyDeviceToHost));

    /* Extract L (unit lower triangular) and U (upper triangular) */
    for (int i = 0; i < n; i++) {
        h_L[i * n + i] = 1.0;  /* unit diagonal */
        for (int j = 0; j < i; j++)
            h_L[j * n + i] = h_LU[j * n + i];  /* below diagonal */
        for (int j = i; j < n; j++)
            h_U[j * n + i] = h_LU[j * n + i];  /* diagonal + above */
    }

    /* Compute L*U on device using acblasDgemm */
    real_t *d_L, *d_U, *d_LU_check;
    checkHggcErrors(hggcMalloc(&d_L, nn * sizeof(real_t)));
    checkHggcErrors(hggcMalloc(&d_U, nn * sizeof(real_t)));
    checkHggcErrors(hggcMalloc(&d_LU_check, nn * sizeof(real_t)));
    checkHggcErrors(hggcMemcpy(d_L, h_L, nn * sizeof(real_t),
                     hggcMemcpyHostToDevice));
    checkHggcErrors(hggcMemcpy(d_U, h_U, nn * sizeof(real_t),
                     hggcMemcpyHostToDevice));

    real_t one = 1.0, zero = 0.0;
    checkHggcErrors(acblasDgemm(handle,
        ACBLAS_OP_N, ACBLAS_OP_N,
        n, n, n,
        &one,
        d_L, n,
        d_U, n,
        &zero,
        d_LU_check, n));

    checkHggcErrors(hggcMemcpy(h_LU_check, d_LU_check, nn * sizeof(real_t),
                     hggcMemcpyDeviceToHost));

    /* Apply permutation to original A: P*A */
    real_t *h_PA = (real_t *)malloc(nn * sizeof(real_t));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            h_PA[j * n + i] = h_A_orig[j * n + pivot[i]];
    }

    /* Compare L*U with P*A */
    real_t max_err = 0.0;
    for (int i = 0; i < nn; i++) {
        real_t err = fabs(h_LU_check[i] - h_PA[i]);
        if (err > max_err) max_err = err;
    }

    printf("  LU verification (acblasDgemm):\n");
    printf("    L*U vs P*A max error: %.2e\n", max_err);
    printf("    Result: %s\n\n", max_err < TOL ? "PASS" : "FAIL");

    checkHggcErrors(hggcFree(d_L));
    checkHggcErrors(hggcFree(d_U));
    checkHggcErrors(hggcFree(d_LU_check));
    free(h_LU); free(h_L); free(h_U); free(h_LU_check); free(h_PA);

    return (max_err < TOL) ? 0 : -1;
}

/* ── Linear solve via acblasDtrsm ────────────────────────────── */

static int solve_with_lu(acblasHandle_t handle, real_t *d_LU,
                         int n, int *pivot, real_t *h_b)
{
    int nn = n * n;
    real_t *h_pb = (real_t *)malloc(n * sizeof(real_t));

    /* Apply permutation: Pb */
    for (int i = 0; i < n; i++)
        h_pb[i] = h_b[pivot[i]];

    real_t *d_x;
    checkHggcErrors(hggcMalloc(&d_x, n * sizeof(real_t)));
    checkHggcErrors(hggcMemcpy(d_x, h_pb, n * sizeof(real_t),
                     hggcMemcpyHostToDevice));

    real_t one = 1.0;

    /* Forward substitution: L * y = Pb
     * L is unit lower triangular (stored in LU below diagonal)
     * acblasDtrsm: solve L * X = B → X overwrites B
     */
    checkHggcErrors(acblasDtrsm(handle,
        ACBLAS_SIDE_LEFT,        /* left side: op(A)*X = B */
        ACBLAS_FILL_MODE_LOWER,
        ACBLAS_OP_N,           /* no transpose */
        ACBLAS_DIAG_UNIT,      /* unit diagonal (L has implicit 1s) */
        n, 1,                  /* m=n, n_rhs=1 */
        &one,
        d_LU, n,               /* L stored in lower part of LU */
        d_x, n));              /* B/X: n×1, ldb=n */

    /* Back substitution: U * x = y
     * U is upper triangular (stored in LU diagonal + above)
     * d_x now contains y, will be overwritten with x
     */
    checkHggcErrors(acblasDtrsm(handle,
        ACBLAS_SIDE_LEFT,
        ACBLAS_FILL_MODE_UPPER,
        ACBLAS_OP_N,
        ACBLAS_DIAG_NON_UNIT,  /* non-unit diagonal */
        n, 1,
        &one,
        d_LU, n,               /* U stored in upper part of LU */
        d_x, n));

    /* Copy solution to host and verify Ax = b */
    real_t *h_x = (real_t *)malloc(n * sizeof(real_t));
    checkHggcErrors(hggcMemcpy(h_x, d_x, n * sizeof(real_t),
                     hggcMemcpyDeviceToHost));

    /* Reconstruct original A for verification (from the original copy) */
    real_t *h_A_orig = (real_t *)malloc(nn * sizeof(real_t));
    init_random_matrix(h_A_orig, n);  /* same seed → same matrix */
    real_t max_err = 0.0;
    for (int i = 0; i < n; i++) {
        real_t sum = 0.0;
        for (int j = 0; j < n; j++)
            sum += h_A_orig[j * n + i] * h_x[j];
        real_t err = fabs(sum - h_b[i]);
        if (err > max_err) max_err = err;
    }

    printf("  Linear solve (acblasDtrsm):\n");
    printf("    b = [");
    for (int i = 0; i < n; i++) printf(" %.2f", h_b[i]);
    printf(" ]\n");
    printf("    x = [");
    for (int i = 0; i < n; i++) printf(" %.6f", h_x[i]);
    printf(" ]\n");
    printf("    ||Ax - b||_inf = %.2e\n", max_err);
    printf("    Result: %s\n\n", max_err < TOL ? "PASS" : "FAIL");

    checkHggcErrors(hggcFree(d_x));
    free(h_pb); free(h_x); free(h_A_orig);

    return (max_err < TOL) ? 0 : -1;
}

/* ── Main ────────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    printf("[simple_acblas_lu] LU Decomposition via BLAS Primitives\n");
    printf("  Matrix size: %dx%d (double precision)\n\n", N, N);

    findHggcDevice(argc, (const char **)argv);

    /* Initialize acblas */
    acblasHandle_t handle;
    acblasStatus_t status = acblasCreate(&handle);
    if (status != ACBLAS_STATUS_SUCCESS) {
        fprintf(stderr, "  ERROR: acblasCreate failed\n");
        return EXIT_FAILURE;
    }

    int nn = N * N;
    real_t *h_A = (real_t *)malloc(nn * sizeof(real_t));
    real_t *h_colbuf = (real_t *)malloc(N * sizeof(real_t));
    int pivot[N];

    /* Initialize matrix */
    init_random_matrix(h_A, N);
    printf("  Original matrix A (column-major, diagonal-dominant):\n");
    print_matrix("A", h_A, N);

    /* Save original for verification */
    real_t *h_A_orig = (real_t *)malloc(nn * sizeof(real_t));
    memcpy(h_A_orig, h_A, nn * sizeof(real_t));

    /* Copy to device */
    real_t *d_A;
    checkHggcErrors(hggcMalloc(&d_A, nn * sizeof(real_t)));
    checkHggcErrors(hggcMemcpy(d_A, h_A, nn * sizeof(real_t),
                     hggcMemcpyHostToDevice));

    /* ── LU Decomposition ── */
    printf("  Performing LU decomposition (swap + scal + ger)...\n\n");
    int rc = lu_decompose(handle, d_A, N, pivot, h_colbuf);
    int verify_rc = -1, solve_rc = -1;

    if (rc != 0) {
        fprintf(stderr, "  ERROR: LU decomposition failed\n");
    } else {
        /* Print pivot vector */
        printf("  Pivot vector P = [");
        for (int i = 0; i < N; i++) printf(" %d", pivot[i]);
        printf(" ]\n\n");

        /* Copy result back for display */
        checkHggcErrors(hggcMemcpy(h_A, d_A, nn * sizeof(real_t),
                         hggcMemcpyDeviceToHost));
        print_matrix("LU", h_A, N);

        /* ── Verify: L*U = P*A ── */
        verify_rc = verify_lu(handle, d_A, h_A_orig, N, pivot);

        /* ── Solve Ax=b using trsm ── */
        real_t h_b[N] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
        solve_rc = solve_with_lu(handle, d_A, N, pivot, h_b);

        /* ── Summary ── */
        printf("  ═══════════════════════════════════════\n");
        printf("  Summary:\n");
        printf("    LU decomposition : %s\n", rc == 0 ? "PASS" : "FAIL");
        printf("    L*U = P*A verify : %s\n", verify_rc == 0 ? "PASS" : "FAIL");
        printf("    Ax=b solve       : %s\n", solve_rc == 0 ? "PASS" : "FAIL");
        printf("    Overall          : %s\n",
               (rc == 0 && verify_rc == 0 && solve_rc == 0) ? "PASS" : "FAIL");
        printf("  ═══════════════════════════════════════\n\n");
    }

    checkHggcErrors(hggcFree(d_A));
    free(h_A); free(h_A_orig); free(h_colbuf);
    acblasDestroy(handle);

    return (rc == 0 && verify_rc == 0 && solve_rc == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
