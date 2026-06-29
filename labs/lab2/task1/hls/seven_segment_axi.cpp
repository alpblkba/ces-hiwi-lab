#include "seven_segment_axi.h"

void seven_segment_axi(ap_uint<4> digit, ap_uint<8> *seg, ap_uint<4> *an) {
#pragma HLS INTERFACE s_axilite port=digit bundle=CTRL
#pragma HLS INTERFACE ap_none port=seg
#pragma HLS INTERFACE ap_none port=an
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    ap_uint<7> pattern_active_high;

    switch (digit) {
        case 0: pattern_active_high = 0b1111110; break;
        case 1: pattern_active_high = 0b0110000; break;
        case 2: pattern_active_high = 0b1101101; break;
        case 3: pattern_active_high = 0b1111001; break;
        case 4: pattern_active_high = 0b0110011; break;
        case 5: pattern_active_high = 0b1011011; break;
        case 6: pattern_active_high = 0b1011111; break;
        case 7: pattern_active_high = 0b1110000; break;
        case 8: pattern_active_high = 0b1111111; break;
        case 9: pattern_active_high = 0b1111011; break;
        default: pattern_active_high = 0b0000000; break;
    }

    ap_uint<8> pattern_active_low;
    pattern_active_low.range(6, 0) = ~pattern_active_high;
    pattern_active_low[7] = 1;  // DP off, active-low

    *seg = pattern_active_low;
    *an  = 0b1110;              // enable one digit, active-low
}
