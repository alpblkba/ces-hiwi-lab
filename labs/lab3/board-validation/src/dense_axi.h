#ifndef DENSE_AXI_H
#define DENSE_AXI_H
#include <ap_int.h>
#define N 4
typedef ap_int<8>  data_t;
typedef ap_int<32> acc_t;
// variant: 0=baseline 1=PIPELINE 2=UNROLL 3=PARTITION-only 4=combined
void dense_axi(ap_uint<32> seed, ap_uint<3> variant, ap_uint<32> *checksum);
#endif
