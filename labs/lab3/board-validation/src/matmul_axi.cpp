#include "matmul_axi.h"

// Operands are derived from the seed so the host only has to write one
// register instead of 512 bytes. The rule is deliberately simple so the same
// values can be reproduced exactly in a software model.
static void generate(ap_uint<32> seed, data_t A[N][N], data_t B[N][N]) {
gen_i:
    for (int i = 0; i < N; i++) {
    gen_j:
        for (int j = 0; j < N; j++) {
            A[i][j] = (data_t)(ap_uint<8>)((seed + 7 * i + 3 * j) & 0xFF);
            B[i][j] = (data_t)(ap_uint<8>)((seed * 2 + 5 * i + 11 * j) & 0xFF);
        }
    }
}

// ---- variant 0: exactly the Task 3.1 baseline, no optimisation pragmas ----
static acc_t mm_baseline(const data_t A[N][N], const data_t B[N][N]) {
    acc_t sum = 0;
row0:
    for (int i = 0; i < N; i++) {
    col0:
        for (int j = 0; j < N; j++) {
            acc_t acc = 0;
        prod0:
            for (int k = 0; k < N; k++) {
                acc += (acc_t)A[i][k] * (acc_t)B[k][j];
            }
            sum += acc;
        }
    }
    return sum;
}

// ---- variant 1: the best combination from Task 3.2 -----------------------
static acc_t mm_optimised(const data_t A[N][N], const data_t B[N][N]) {
#pragma HLS ARRAY_PARTITION variable=A dim=2 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=B dim=1 cyclic factor=4
    acc_t sum = 0;
row1:
    for (int i = 0; i < N; i++) {
    col1:
        for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
            acc_t acc = 0;
        prod1:
            for (int k = 0; k < N; k++) {
#pragma HLS UNROLL factor=4
                acc += (acc_t)A[i][k] * (acc_t)B[k][j];
            }
            sum += acc;
        }
    }
    return sum;
}

void matmul_axi(ap_uint<32> seed, ap_uint<1> variant, ap_uint<32> *checksum) {
#pragma HLS INTERFACE s_axilite port=seed     bundle=CTRL
#pragma HLS INTERFACE s_axilite port=variant  bundle=CTRL
#pragma HLS INTERFACE s_axilite port=checksum bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return   bundle=CTRL

    data_t A0[N][N], B0[N][N];
    data_t A1[N][N], B1[N][N];

    if (variant == 0) {
        generate(seed, A0, B0);
        *checksum = (ap_uint<32>)mm_baseline(A0, B0);
    } else {
        generate(seed, A1, B1);
        *checksum = (ap_uint<32>)mm_optimised(A1, B1);
    }
}
