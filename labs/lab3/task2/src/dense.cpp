#include "dense.h"

void dense_layer(const data_t x[N][N], const data_t W[N][N],
                 const acc_t bias[N], acc_t y[N][N]) {
sample:
    for (int i = 0; i < N; i++) {
    neuron:
        for (int j = 0; j < N; j++) {
            acc_t acc = bias[j];          // start from the bias, not zero
        prod:
            for (int k = 0; k < N; k++) {
                acc += (acc_t)x[i][k] * (acc_t)W[k][j];
            }
            y[i][j] = (acc > 0) ? acc : (acc_t)0;   // ReLU
        }
    }
}
