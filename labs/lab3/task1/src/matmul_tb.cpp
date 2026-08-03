#include "matmul.h"
#include <cstdio>
#include <cstdlib>

// Golden reference, written in plain C with no HLS types, so that it is
// genuinely independent of the design under test.
static void golden(const int A[N][N], const int B[N][N], int C[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int acc = 0;
            for (int k = 0; k < N; k++) acc += A[i][k] * B[k][j];
            C[i][j] = acc;
        }
}

int main() {
    data_t A[N][N], B[N][N];
    acc_t  C[N][N];
    int    Ai[N][N], Bi[N][N], Cgold[N][N];

    // Deterministic pseudo-random inputs covering the full signed 8-bit range,
    // including the extremes, so sign handling is exercised.
    srand(1);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int a = (rand() % 255) - 128;
            int b = (rand() % 255) - 128;
            Ai[i][j] = a;  A[i][j] = (data_t)a;
            Bi[i][j] = b;  B[i][j] = (data_t)b;
        }

    matmul(A, B, C);
    golden(Ai, Bi, Cgold);

    int errors = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            if ((int)C[i][j] != Cgold[i][j]) {
                if (errors < 5)
                    printf("mismatch at C[%d][%d]: got %d, expected %d\n",
                           i, j, (int)C[i][j], Cgold[i][j]);
                errors++;
            }

    if (errors) {
        printf("matmul test FAILED (%d mismatches)\n", errors);
        return 1;
    }
    printf("matmul test passed (%dx%d, int8 operands, int32 accumulator)\n", N, N);
    return 0;
}
