#include "dnn_task1.h"

// Task 3.1 baseline: no optimisation pragmas at all. The loops are labelled so
// that Task 3.2 can attach a pragma to one specific loop. Labels do not affect
// the hardware.
void dnn_kernel(const data_t x[N][N], const data_t W[N][N],
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
