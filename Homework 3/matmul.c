/*
 * Homework 03 — Matrix Multiplication and Benchmarking
 *
 * Computes C = A * B, where A = a * I_N (every entry = a) and
 * B = b * I_N (every entry = b), I_N being the all-ones N x N matrix
 * (as defined in the assignment: A and B are square, uniformly filled).
 *
 * Since every row of A has N entries equal to a, and every column of B
 * has N entries equal to b:
 *     C[i][j] = sum_k A[i][k]*B[k][j] = sum_k (a*b) = N * a * b   for all i,j.
 *
 * This analytical result lets us verify correctness in O(1) instead of
 * O(N^2), as suggested by the "optimization challenge" in the assignment.
 *
 * Usage:
 *   ./matmul a b N fileout [order]
 *
 *   a, b     : scalar fill values for matrices A and B   (double)
 *   N        : matrix dimension (square N x N matrices)  (positive integer)
 *   fileout  : path of the text file where C is saved
 *   order    : optional, one of {ijk, ikj, jik, jki, kij, kji}
 *              (default: ijk). Selects the loop nesting order, to let the
 *              Makefile/REPORT benchmark all six permutations.
 *
 * Matrices are stored as flat 1D arrays in row-major order (C[i*N+j]),
 * consistently with how C actually lays out 2D arrays in memory
 * (see lecture notes, Chapter 2).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Allocate an N x N matrix (flattened, row-major) filled with `value`. */
static double *alloc_filled_matrix(long long N, double value) {
    double *M = (double *)malloc((size_t)N * (size_t)N * sizeof(double));
    if (M == NULL) return NULL;
    for (long long i = 0; i < N * N; i++) {
        M[i] = value;
    }
    return M;
}

/* Six loop-order variants for C = A*B, all operating on flat row-major
 * arrays A[i*N+k], B[k*N+j], C[i*N+j]. C is assumed to be pre-zeroed. */

static void matmul_ijk(const double *A, const double *B, double *C, long long N) {
    for (long long i = 0; i < N; i++)
        for (long long j = 0; j < N; j++)
            for (long long k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void matmul_ikj(const double *A, const double *B, double *C, long long N) {
    for (long long i = 0; i < N; i++)
        for (long long k = 0; k < N; k++)
            for (long long j = 0; j < N; j++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void matmul_jik(const double *A, const double *B, double *C, long long N) {
    for (long long j = 0; j < N; j++)
        for (long long i = 0; i < N; i++)
            for (long long k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void matmul_jki(const double *A, const double *B, double *C, long long N) {
    for (long long j = 0; j < N; j++)
        for (long long k = 0; k < N; k++)
            for (long long i = 0; i < N; i++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void matmul_kij(const double *A, const double *B, double *C, long long N) {
    for (long long k = 0; k < N; k++)
        for (long long i = 0; i < N; i++)
            for (long long j = 0; j < N; j++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void matmul_kji(const double *A, const double *B, double *C, long long N) {
    for (long long k = 0; k < N; k++)
        for (long long j = 0; j < N; j++)
            for (long long i = 0; i < N; i++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

int main(int argc, char *argv[]) {

    if (argc < 5) {
        fprintf(stderr, "Usage: %s a b N fileout [order]\n", argv[0]);
        fprintf(stderr, "  order in {ijk, ikj, jik, jki, kij, kji} (default ijk)\n");
        return 1;
    }

    double a = atof(argv[1]);
    double b = atof(argv[2]);
    long long N = atoll(argv[3]);
    const char *fileout = argv[4];
    const char *order = (argc >= 6) ? argv[5] : "ijk";

    if (N <= 0) {
        fprintf(stderr, "Error: N must be a positive integer.\n");
        return 1;
    }

    double *A = alloc_filled_matrix(N, a);
    double *B = alloc_filled_matrix(N, b);
    double *C = alloc_filled_matrix(N, 0.0); /* C starts at zero */

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: allocation failed for N = %lld "
                        "(requested ~%.2f GB total).\n",
                N, 3.0 * (double)N * (double)N * sizeof(double) / 1e9);
        free(A); free(B); free(C);
        return 1;
    }

    clock_t start = clock();

    if      (strcmp(order, "ijk") == 0) matmul_ijk(A, B, C, N);
    else if (strcmp(order, "ikj") == 0) matmul_ikj(A, B, C, N);
    else if (strcmp(order, "jik") == 0) matmul_jik(A, B, C, N);
    else if (strcmp(order, "jki") == 0) matmul_jki(A, B, C, N);
    else if (strcmp(order, "kij") == 0) matmul_kij(A, B, C, N);
    else if (strcmp(order, "kji") == 0) matmul_kji(A, B, C, N);
    else {
        fprintf(stderr, "Error: unknown order '%s'.\n", order);
        free(A); free(B); free(C);
        return 1;
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    /* ---- Smart O(1) correctness test ----
     * Analytically, C[i][j] = N * a * b for every entry, since
     * A = a*ones(N), B = b*ones(N). We therefore check only a handful of
     * representative entries (corners + center) against this closed-form
     * value, instead of looping over all N^2 elements again. */
    double expected = (double)N * a * b;
    long long check_indices[5][2] = {
        {0, 0}, {0, N - 1}, {N - 1, 0}, {N - 1, N - 1}, {N / 2, N / 2}
    };
    int all_ok = 1;
    double max_abs_err = 0.0;
    for (int t = 0; t < 5; t++) {
        long long i = check_indices[t][0];
        long long j = check_indices[t][1];
        double val = C[i * N + j];
        double err = val - expected;
        if (err < 0) err = -err;
        if (err > max_abs_err) max_abs_err = err;
        /* Allow a tiny floating-point tolerance, since N additions of a*b
         * are not guaranteed to be bit-identical to N*a*b for all N. */
        double tol = 1e-9 * (1.0 + (expected < 0 ? -expected : expected));
        if (err > tol) all_ok = 0;
    }

    /* ---- Save C to the output file ---- */
    FILE *fp = fopen(fileout, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open output file '%s'.\n", fileout);
        free(A); free(B); free(C);
        return 1;
    }
    for (long long i = 0; i < N; i++) {
        for (long long j = 0; j < N; j++) {
            fprintf(fp, "%.10g ", C[i * N + j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    /* ---- Report ---- */
    printf("==== Matrix Multiplication C = A*B (order=%s) ====\n", order);
    printf("N               = %lld\n", N);
    printf("a               = %.10g\n", a);
    printf("b               = %.10g\n", b);
    printf("expected C[i][j]= %.10g  (= N*a*b)\n", expected);
    printf("max |error|     = %.3e (sampled corners + center)\n", max_abs_err);
    printf("internal test   = %s\n", all_ok ? "PASSED" : "FAILED");
    printf("elapsed time    = %.6f s\n", elapsed);
    printf("output file     = %s\n", fileout);
    printf("====================================================\n");

    free(A);
    free(B);
    free(C);

    return all_ok ? 0 : 2;
}
