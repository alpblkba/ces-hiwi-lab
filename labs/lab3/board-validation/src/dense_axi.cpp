#include "dense_axi.h"

static void gen(ap_uint<32> seed, data_t x[N][N], data_t W[N][N], acc_t b[N]) {
gi: for (int i = 0; i < N; i++) {
        b[i] = (acc_t)((seed + 31 * i) % 401) - 200;
    gj: for (int j = 0; j < N; j++) {
            x[i][j] = (data_t)(ap_uint<8>)((seed + 7 * i + 3 * j) & 0xFF);
            W[i][j] = (data_t)(ap_uint<8>)((seed * 2 + 5 * i + 11 * j) & 0xFF);
        }
    }
}

// V00 - no pragmas
static acc_t v_base(const data_t x[N][N], const data_t W[N][N], const acc_t b[N]) {
    acc_t s = 0;
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
        acc_t a = b[j];
        for (int k = 0; k < N; k++) a += (acc_t)x[i][k] * (acc_t)W[k][j];
        s += (a > 0) ? a : (acc_t)0;
    }
    return s;
}
// V02 - PIPELINE on the neuron loop
static acc_t v_pipe(const data_t x[N][N], const data_t W[N][N], const acc_t b[N]) {
    acc_t s = 0;
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
        acc_t a = b[j];
        for (int k = 0; k < N; k++) a += (acc_t)x[i][k] * (acc_t)W[k][j];
        s += (a > 0) ? a : (acc_t)0;
    }
    return s;
}
// V04 - UNROLL the product loop, no partition
static acc_t v_unroll(const data_t x[N][N], const data_t W[N][N], const acc_t b[N]) {
    acc_t s = 0;
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
        acc_t a = b[j];
        for (int k = 0; k < N; k++) {
#pragma HLS UNROLL
            a += (acc_t)x[i][k] * (acc_t)W[k][j];
        }
        s += (a > 0) ? a : (acc_t)0;
    }
    return s;
}
// V05 - ARRAY_PARTITION alone, nothing unrolled
static acc_t v_part(const data_t x[N][N], const data_t W[N][N], const acc_t b[N]) {
#pragma HLS ARRAY_PARTITION variable=x dim=2 complete
#pragma HLS ARRAY_PARTITION variable=W dim=1 complete
    acc_t s = 0;
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
        acc_t a = b[j];
        for (int k = 0; k < N; k++) a += (acc_t)x[i][k] * (acc_t)W[k][j];
        s += (a > 0) ? a : (acc_t)0;
    }
    return s;
}
// V08 - all three together
static acc_t v_all(const data_t x[N][N], const data_t W[N][N], const acc_t b[N]) {
#pragma HLS ARRAY_PARTITION variable=x dim=2 complete
#pragma HLS ARRAY_PARTITION variable=W dim=1 complete
    acc_t s = 0;
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) {
#pragma HLS PIPELINE II=1
        acc_t a = b[j];
        for (int k = 0; k < N; k++) {
#pragma HLS UNROLL
            a += (acc_t)x[i][k] * (acc_t)W[k][j];
        }
        s += (a > 0) ? a : (acc_t)0;
    }
    return s;
}

void dense_axi(ap_uint<32> seed, ap_uint<3> variant, ap_uint<32> *checksum) {
#pragma HLS INTERFACE s_axilite port=seed     bundle=CTRL
#pragma HLS INTERFACE s_axilite port=variant  bundle=CTRL
#pragma HLS INTERFACE s_axilite port=checksum bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return   bundle=CTRL
    data_t x[N][N], W[N][N]; acc_t b[N];
    gen(seed, x, W, b);
    acc_t r;
    switch (variant) {
        case 1:  r = v_pipe(x, W, b);   break;
        case 2:  r = v_unroll(x, W, b); break;
        case 3:  r = v_part(x, W, b);   break;
        case 4:  r = v_all(x, W, b);    break;
        default: r = v_base(x, W, b);   break;
    }
    *checksum = (ap_uint<32>)r;
}
