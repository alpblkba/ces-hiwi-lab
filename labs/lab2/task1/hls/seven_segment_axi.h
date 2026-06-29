#ifndef SEVEN_SEGMENT_AXI_H
#define SEVEN_SEGMENT_AXI_H

#include <ap_int.h>

void seven_segment_axi(ap_uint<4> digit, ap_uint<8> *seg, ap_uint<4> *an);

#endif
