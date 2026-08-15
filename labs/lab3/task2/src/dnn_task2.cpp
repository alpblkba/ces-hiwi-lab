#include "dnn_task2.h"

// ---------------------------------------------------------------------------
// Task 3.2: the Task 3.1 kernel with pragmas.
//
// The arithmetic below is character for character the same as task1. Every
// line that differs is a pragma, and each one is guarded by the VARIANT macro
// so that the five builds come from one file. Read the #if lines as "this
// pragma belongs to these variants".
//
// ARRAY_PARTITION goes at the top of the function, on the arrays. PIPELINE and
// UNROLL go immediately inside the loop they apply to. The dimensions are not
// arbitrary: the unrolled index is k, and k indexes x[i][k] (dimension 2) and
// W[k][j] (dimension 1). Those are the dimensions that have to be split for
// several k values to be read in the same cycle. y is not indexed by k, so
// partitioning it would achieve nothing.
// ---------------------------------------------------------------------------

void dnn_kernel(const data_t x[N][N], const data_t W[N][N],
                const acc_t bias[N], acc_t y[N][N]) {
#if VARIANT == 3 || VARIANT == 4
#pragma HLS ARRAY_PARTITION variable=x dim=2 complete
#pragma HLS ARRAY_PARTITION variable=W dim=1 complete
#endif
sample:
    for (int i = 0; i < N; i++) {
    neuron:
        for (int j = 0; j < N; j++) {
#if VARIANT == 1 || VARIANT == 4
#pragma HLS PIPELINE II=1
#endif
            acc_t acc = bias[j];          // start from the bias, not zero
        prod:
            for (int k = 0; k < N; k++) {
#if VARIANT == 2 || VARIANT == 4
#pragma HLS UNROLL
#endif
                acc += (acc_t)x[i][k] * (acc_t)W[k][j];
            }
            y[i][j] = (acc > 0) ? acc : (acc_t)0;   // ReLU
        }
    }
}

const char *variant_name(void) {
#if   VARIANT == 0
    return "0  baseline, no pragmas";
#elif VARIANT == 1
    return "1  PIPELINE on the neuron loop";
#elif VARIANT == 2
    return "2  UNROLL the product loop";
#elif VARIANT == 3
    return "3  ARRAY_PARTITION alone";
#elif VARIANT == 4
    return "4  all three together";
#else
    return "unknown VARIANT";
#endif
}
