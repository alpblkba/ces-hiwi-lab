#ifndef DNN_TASK2_H
#define DNN_TASK2_H

#include <ap_int.h>

// Same DNN kernel as Task 3.1. Nothing about the arithmetic changes in Task
// 3.2 -- only pragmas are added, and the testbench proves the results are
// identical.
#define N 4

typedef ap_int<8>  data_t;   // activations and weights
typedef ap_int<32> acc_t;    // accumulator, bias and output

// Which pragma variant this build uses. Set from the Tcl script with
//     add_files dnn_task2.cpp -cflags "-DVARIANT=2"
// so that all five builds come from ONE source file. Editing the pragmas by
// hand works just as well -- see the README; the macro exists so the sweep and
// the five bitstreams do not need five copies of the same code.
//
//   0  baseline, no pragmas          (identical to Task 3.1)
//   1  PIPELINE on the neuron loop
//   2  UNROLL the product loop
//   3  ARRAY_PARTITION alone
//   4  all three together
#ifndef VARIANT
#define VARIANT 4
#endif

void dnn_kernel(const data_t x[N][N], const data_t W[N][N],
                const acc_t bias[N], acc_t y[N][N]);

const char *variant_name(void);

#endif
