#include "dnn_kernel_axi.h"

// ---------------------------------------------------------------------------
// Operand generation.
//
// This runs on the FPGA, and dnn_app.c runs the identical formula on the ARM
// core. If the two ever drift apart the board reports a failure that has
// nothing to do with the pragmas, so treat these six lines as a contract:
// change them here and change them there in the same commit.
//
// The & 0xFF then cast to ap_int<8> is what puts operands over the full signed
// range including -128, which is the value that catches sign-extension bugs.
// ---------------------------------------------------------------------------
static void gen(ap_uint<32> seed, data_t x[N][N], data_t W[N][N], acc_t b[N]) {
#pragma HLS INLINE
gi: for (int i = 0; i < N; i++) {
        b[i] = (acc_t)((seed + 31 * i) % 401) - 200;
    gj: for (int j = 0; j < N; j++) {
            x[i][j] = (data_t)(ap_uint<8>)((seed + 7 * i + 3 * j) & 0xFF);
            W[i][j] = (data_t)(ap_uint<8>)((seed * 2 + 5 * i + 11 * j) & 0xFF);
        }
    }
}

// ---------------------------------------------------------------------------
// The layer under test. Byte for byte the Task 3.2 kernel, reduced to a single
// checksum so the whole result fits in one readable register.
//
// INLINE is here so that the loop labels below are always reachable as
// dnn_kernel_axi/neuron and dnn_kernel_axi/prod whichever variant is built. It
// is a structural pragma, applied identically to all five variants, so it does
// not tilt the comparison.
// ---------------------------------------------------------------------------
static acc_t layer(const data_t x[N][N], const data_t W[N][N], const acc_t b[N]) {
#pragma HLS INLINE
#if VARIANT == 3 || VARIANT == 4
#pragma HLS ARRAY_PARTITION variable=x dim=2 complete
#pragma HLS ARRAY_PARTITION variable=W dim=1 complete
#endif
    acc_t s = 0;
sample:
    for (int i = 0; i < N; i++) {
    neuron:
        for (int j = 0; j < N; j++) {
#if VARIANT == 1 || VARIANT == 4
#pragma HLS PIPELINE II=1
#endif
            acc_t a = b[j];
        prod:
            for (int k = 0; k < N; k++) {
#if VARIANT == 2 || VARIANT == 4
#pragma HLS UNROLL
#endif
                a += (acc_t)x[i][k] * (acc_t)W[k][j];
            }
            s += (a > 0) ? a : (acc_t)0;      // ReLU, then folded into the sum
        }
    }
    return s;
}

void dnn_kernel_axi(ap_uint<32> seed, ap_uint<32> reps,
                    ap_uint<32> *checksum, ap_uint<32> *variant_id) {
#pragma HLS INTERFACE s_axilite port=seed       bundle=CTRL
#pragma HLS INTERFACE s_axilite port=reps       bundle=CTRL
#pragma HLS INTERFACE s_axilite port=checksum   bundle=CTRL
#pragma HLS INTERFACE s_axilite port=variant_id bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return     bundle=CTRL

    data_t x[N][N], W[N][N];
    acc_t  b[N];
    gen(seed, x, W, b);

    const acc_t b0 = b[0];
    const unsigned n = (reps == 0) ? 1u : (unsigned)reps;

    acc_t total = 0;
rep:
    for (unsigned r = 0; r < n; r++) {
        // Every repetition has to differ, or the tool hoists the whole layer
        // out of the loop and the timing measurement reads zero work. Bumping
        // one bias term is the cheapest change that cannot be optimised away,
        // and it leaves reps = 1 equal to the plain checksum for this seed.
        b[0] = b0 + (acc_t)r;
        total += layer(x, W, b);
    }

    // For reps > 1 this sum can wrap 32 bits. That is fine: the functional
    // tests all use reps = 1, and the timing mode does not look at the value.
    *checksum   = (ap_uint<32>)total;
    *variant_id = (ap_uint<32>)VARIANT;
}
