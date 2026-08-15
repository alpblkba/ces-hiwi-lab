#ifndef DNN_KERNEL_AXI_H
#define DNN_KERNEL_AXI_H

#include <ap_int.h>

// AXI4-Lite wrapper around the Task 3.2 DNN kernel, so the layer can be driven
// from a C program running on the ARM core.
//
// One bitstream contains ONE pragma variant. The five variants are five
// separate builds of this file with different -DVARIANT values, which is what
// makes the board demonstration honest: five physically different circuits,
// programmed one after the other, must return the same checksum for the same
// seed. A single bitstream with a variant register could always be dismissed
// as one circuit behind a multiplexer.
//
// The operands are generated on chip from `seed`, so the software only has to
// write one register. The software recreates the same operands with the same
// formula and checks the answer itself -- see dnn_app.c.
#define N 4

typedef ap_int<8>  data_t;
typedef ap_int<32> acc_t;

// Which pragma variant this build carries. Reported back over AXI in
// `variant_id`, so the application can name the bitstream that is loaded
// instead of the student having to remember.
//
//   0  baseline, no pragmas
//   1  PIPELINE on the neuron loop
//   2  UNROLL the product loop
//   3  ARRAY_PARTITION alone
//   4  all three together
#ifndef VARIANT
#define VARIANT 0
#endif

// reps: how many times to run the layer per call. 1 for a functional check;
// a large number for timing, so that the AXI handshake stops dominating the
// measurement. 0 is treated as 1.
void dnn_kernel_axi(ap_uint<32> seed, ap_uint<32> reps,
                    ap_uint<32> *checksum, ap_uint<32> *variant_id);

#endif
