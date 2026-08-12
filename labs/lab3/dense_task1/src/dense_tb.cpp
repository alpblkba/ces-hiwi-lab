#include "dense.h"
#include <cstdio>
#include <cstdlib>

static void golden(const int x[N][N], const int W[N][N], const int b[N], int y[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int acc = b[j];
            for (int k = 0; k < N; k++) acc += x[i][k] * W[k][j];
            y[i][j] = acc > 0 ? acc : 0;
        }
}

int main() {
    data_t X[N][N], Wq[N][N]; acc_t B[N], Y[N][N];
    int xi[N][N], wi[N][N], bi[N], yg[N][N];

    srand(1);
    for (int i = 0; i < N; i++) {
        bi[i] = (rand() % 2001) - 1000;  B[i] = bi[i];
        for (int j = 0; j < N; j++) {
            int a = (rand() % 255) - 128, w = (rand() % 255) - 128;
            xi[i][j] = a; X[i][j] = (data_t)a;
            wi[i][j] = w; Wq[i][j] = (data_t)w;
        }
    }

    dense_layer(X, Wq, B, Y);
    golden(xi, wi, bi, yg);

    int err = 0, relu_hits = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            if (yg[i][j] == 0) relu_hits++;
            if ((int)Y[i][j] != yg[i][j]) {
                printf("mismatch y[%d][%d]: got %d expected %d\n",
                       i, j, (int)Y[i][j], yg[i][j]);
                err++;
            }
        }

    if (err) { printf("dense_layer test FAILED (%d)\n", err); return 1; }
    printf("dense_layer test passed (%dx%d, bias + ReLU, %d output(s) clamped by ReLU)\n",
           N, N, relu_hits);
    return 0;
}
