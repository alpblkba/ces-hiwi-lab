#include "seven_segment.h"
#include <iostream>

static ap_uint<7> expected_pattern(unsigned int digit) {
    switch (digit) {
        case 0: return 0b1111110;
        case 1: return 0b0110000;
        case 2: return 0b1101101;
        case 3: return 0b1111001;
        case 4: return 0b0110011;
        case 5: return 0b1011011;
        case 6: return 0b1011111;
        case 7: return 0b1110000;
        case 8: return 0b1111111;
        case 9: return 0b1111011;
        default: return 0b0000000;
    }
}

int main() {
    bool failed = false;

    for (unsigned int digit = 0; digit < 16; digit++) {
        ap_uint<7> segments = 0;
        seven_segment(digit, &segments);

        ap_uint<7> expected = expected_pattern(digit);

        if (segments != expected) {
            std::cout << "mismatch for digit " << digit
                      << ": got " << segments.to_uint()
                      << ", expected " << expected.to_uint()
                      << std::endl;
            failed = true;
        }
    }

    if (failed) {
        return 1;
    }

    std::cout << "seven_segment test passed" << std::endl;
    return 0;
}
