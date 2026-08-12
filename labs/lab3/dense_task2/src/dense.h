#ifndef DENSE_H
#define DENSE_H

#include <ap_int.h>

// One dense (fully connected) neural-network layer:
//
//     y = ReLU( x * W + bias )
//
// x    : N samples, each with N input features
// W    : weight matrix, N inputs -> N neurons
// bias : one bias term per neuron
// y    : N samples, each with N activations
//
// 8-bit operands with a 32-bit accumulator, as in quantised inference.
#define N 4

typedef ap_int<8>  data_t;   // activations and weights
typedef ap_int<32> acc_t;    // accumulator, bias and output

void dense_layer(const data_t x[N][N], const data_t W[N][N],
                 const acc_t bias[N], acc_t y[N][N]);

#endif
