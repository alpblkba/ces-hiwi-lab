#include "seven_segment.h"

void seven_segment(ap_uint<4> digit, ap_uint<7> *segments) {
#pragma HLS INTERFACE ap_none port=digit
#pragma HLS INTERFACE ap_none port=segments
#pragma HLS INTERFACE ap_ctrl_none port=return

    ap_uint<7> pattern;

    switch (digit) {
        case 0: pattern = 0b1111110; break;
        case 1: pattern = 0b0110000; break;
        case 2: pattern = 0b1101101; break;
        case 3: pattern = 0b1111001; break;
        case 4: pattern = 0b0110011; break;
        case 5: pattern = 0b1011011; break;
        case 6: pattern = 0b1011111; break;
        case 7: pattern = 0b1110000; break;
        case 8: pattern = 0b1111111; break;
        case 9: pattern = 0b1111011; break;
        default: pattern = 0b0000000; break;
    }

    *segments = pattern;
}
