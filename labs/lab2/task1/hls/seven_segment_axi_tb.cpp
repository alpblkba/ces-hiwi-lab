#include "seven_segment_axi.h"
#include <iostream>

static ap_uint<8> expected_pattern(unsigned int digit) {
    switch (digit) {
        case 0: return 0b01111110; // DP off, A B C D E F on, G off
        case 1: return 0b00110000;
        case 2: return 0b01101101;
        case 3: return 0b01111001;
        case 4: return 0b00110011;
        case 5: return 0b01011011;
        case 6: return 0b01011111;
        case 7: return 0b01110000;
        case 8: return 0b01111111;
        case 9: return 0b01111011;
        default: return 0b00000000;
    }
}

int main() {
    bool failed = false;

    for (unsigned int digit = 0; digit < 16; digit++) {
        ap_uint<8> seg = 0;
        ap_uint<4> an = 0;

        seven_segment_axi(digit, &seg, &an);

        ap_uint<8> expected_seg = expected_pattern(digit);
        ap_uint<4> expected_an = 0b1110;

        if (seg != expected_seg) {
            std::cout << "segment mismatch for digit " << digit
                      << ": got " << seg.to_uint()
                      << ", expected " << expected_seg.to_uint()
                      << std::endl;
            failed = true;
        }

        if (an != expected_an) {
            std::cout << "anode mismatch for digit " << digit
                      << ": got " << an.to_uint()
                      << ", expected " << expected_an.to_uint()
                      << std::endl;
            failed = true;
        }
    }

    if (failed) {
        return 1;
    }

    std::cout << "seven_segment_axi test passed" << std::endl;
    return 0;
}
