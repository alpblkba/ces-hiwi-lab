#ifndef MATMUL_AXI_H
#define MATMUL_AXI_H

#include <ap_int.h>

// ---------------------------------------------------------------------------
// Lab 3.3 - the Lab 3 kernel as an AXI-Lite peripheral, so that the pragma
// study can be checked on real hardware.
//
// The point being tested is the central claim of Lab 3.2: pragmas change how
// fast the circuit is and how much of the device it uses, but they must NOT
// change what it computes. Two implementations of the same matrix multiply are
// synthesized side by side - one with no optimisation pragmas, one with the
// best combination found in Task 3.2 - and `variant` selects which one runs.
//
// For any given seed both must return the SAME checksum. If they differ, an
// optimisation changed the arithmetic, which is a bug.
//
//   seed     -> operands are generated on-chip, so no large data transfer
//   variant  -> 0 = unoptimised, 1 = PIPELINE + UNROLL 4 + PARTITION 4
//   checksum -> sum of all elements of the product matrix
// ---------------------------------------------------------------------------

#define N 16

typedef ap_int<8>  data_t;
typedef ap_int<32> acc_t;

void matmul_axi(ap_uint<32> seed, ap_uint<1> variant, ap_uint<32> *checksum);

#endif
