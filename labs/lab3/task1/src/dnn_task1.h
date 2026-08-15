#ifndef DNN_TASK1_H
#define DNN_TASK1_H

#include <ap_int.h>

// One dense (fully connected) neural-network layer -- the DNN kernel this lab
// is built around:
//
//     y = ReLU( x * W + bias )
//
// x    : N samples, each with N input features
// W    : weight matrix, N inputs -> N neurons
// bias : one bias term per neuron
// y    : N samples, each with N activations
//
// 8-bit operands with a 32-bit accumulator, as in quantised inference.
//
// bias and y are acc_t, not data_t. That is not cosmetic: the accumulator has
// to hold N products of two 8-bit numbers plus the bias, and the testbench has
// a case that fails loudly if either is narrowed to 8 bits.
#define N 4

typedef ap_int<8>  data_t;   // activations and weights
typedef ap_int<32> acc_t;    // accumulator, bias and output

void dnn_kernel(const data_t x[N][N], const data_t W[N][N],
                const acc_t bias[N], acc_t y[N][N]);

#endif
