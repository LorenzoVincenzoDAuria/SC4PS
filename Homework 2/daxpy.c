/*
 * Homework 02 — Vector Sum in C  (d = a*x + y, the "DAXPY" operation)
 *
 * Usage:
 *   ./daxpy N a x y
 *
 * Arguments (all acquired dynamically from the command line):
 *   N : dimensionality of the vectors x, y, d   (positive integer)
 *   a : scalar multiplier                        (double)
 *   x : uniform value used to fill every element of vector x  (double)
 *   y : uniform value used to fill every element of vector y  (double)
 *
 * The program:
 *   1. Allocates x, y, d dynamically on the heap (so large N, e.g. 1e8, works).
 *   2. Computes d[i] = a*x[i] + y[i] for every i.
 *   3. Runs an internal test verifying every d[i] is EXACTLY equal to a*x+y.
 *   4. Reports timing information.
 *
 * Author: course homework solution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Usage: %s N a x y\n", argv[0]);
        fprintf(stderr, "  N : vector dimension (positive integer)\n");
        fprintf(stderr, "  a : scalar multiplier (double)\n");
        fprintf(stderr, "  x : uniform fill value for vector x (double)\n");
        fprintf(stderr, "  y : uniform fill value for vector y (double)\n");
        return 1;
    }

    /* ---- Parse command-line arguments ---- */
    long long N = atoll(argv[1]);
    double a = atof(argv[2]);
    double xval = atof(argv[3]);
    double yval = atof(argv[4]);

    if (N <= 0) {
        fprintf(stderr, "Error: N must be a positive integer (got %lld).\n", N);
        return 1;
    }

    /* ---- Dynamic allocation on the heap (mandatory for large N) ---- */
    double *x = (double *)malloc((size_t)N * sizeof(double));
    double *y = (double *)malloc((size_t)N * sizeof(double));
    double *d = (double *)malloc((size_t)N * sizeof(double));

    if (x == NULL || y == NULL || d == NULL) {
        fprintf(stderr, "Error: memory allocation failed for N = %lld "
                        "(requested ~%.2f GB total).\n",
                N, 3.0 * (double)N * sizeof(double) / 1e9);
        free(x); free(y); free(d);
        return 1;
    }

    /* ---- Fill vectors with the uniform values provided ---- */
    for (long long i = 0; i < N; i++) {
        x[i] = xval;
        y[i] = yval;
    }

    /* ---- Compute d = a*x + y, with timing ---- */
    clock_t start = clock();
    for (long long i = 0; i < N; i++) {
        d[i] = a * x[i] + y[i];
    }
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    /* ---- Internal correctness test ----
     * We verify that every element of d is EXACTLY equal to the
     * mathematically expected value a*x + y (computed once, since x and y
     * are uniform). "Exactly equal" here means bit-for-bit identical
     * floating-point values (==), not "close enough".
     */
    double expected = a * xval + yval;
    long long mismatches = 0;
    for (long long i = 0; i < N; i++) {
        if (d[i] != expected) {
            mismatches++;
        }
    }

    /* ---- Report results ---- */
    printf("==== Vector Sum (DAXPY): d = a*x + y ====\n");
    printf("N               = %lld\n", N);
    printf("a               = %.17g\n", a);
    printf("x (uniform)     = %.17g\n", xval);
    printf("y (uniform)     = %.17g\n", yval);
    printf("expected value  = %.17g\n", expected);
    printf("d[0]            = %.17g\n", d[0]);
    printf("d[N-1]          = %.17g\n", d[N - 1]);
    printf("mismatches      = %lld / %lld\n", mismatches, N);
    printf("internal test   = %s\n", (mismatches == 0) ? "PASSED" : "FAILED");
    printf("elapsed time    = %.6f s\n", elapsed);
    printf("==========================================\n");

    free(x);
    free(y);
    free(d);

    return (mismatches == 0) ? 0 : 2;
}
