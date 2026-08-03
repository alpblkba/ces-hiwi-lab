#ifndef MATMUL_H
#define MATMUL_H

#include <ap_int.h>

// ---------------------------------------------------------------------------
// Lab 3 - a small dense (fully connected) layer, the core operation of a DNN.
//
// Sizes are compile-time constants on purpose. If the matrix size were a
// runtime argument, Vitis HLS could not compute a latency figure and the
// synthesis report would show "?" instead of a number - which would make the
// whole point of this lab (comparing latency between pragma variants)
// impossible.
//
// Data types follow quantised DNN practice: 8-bit operands with a wide
// accumulator. On the Blackboard this matters a lot. A 32-bit multiply costs
// roughly four DSP slices, and the device only has 66 of them, so an int
// version of this kernel already uses 63% of the DSPs before any optimisation
// is applied. The 8-bit version uses 12%, which is what leaves room to explore.
// ---------------------------------------------------------------------------

#define N 16                  // matrix dimension (N x N)

typedef ap_int<8>  data_t;    // activations and weights
typedef ap_int<32> acc_t;     // accumulator, wide enough to never overflow

void matmul(const data_t A[N][N], const data_t B[N][N], acc_t C[N][N]);

#endif
