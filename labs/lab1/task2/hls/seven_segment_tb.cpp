#include "seven_segment.h"
#include <iostream>

// ---------------------------------------------------------------------------
// Golden reference, written independently of the design under test.
// Active-high patterns, segment A in bit 0 (same convention as the XDC).
// ---------------------------------------------------------------------------
static ap_uint<7> golden_pattern(unsigned int digit) {
    switch (digit) {
        case 0: return 0x3F;
        case 1: return 0x06;
        case 2: return 0x5B;
        case 3: return 0x4F;
        case 4: return 0x66;
        case 5: return 0x6D;
        case 6: return 0x7D;
        case 7: return 0x07;
        case 8: return 0x7F;
        case 9: return 0x6F;
        default: return 0x00;
    }
}

static ap_uint<8> golden_active_low(ap_uint<7> pattern_active_high) {
    ap_uint<8> out;
    out.range(6, 0) = ~pattern_active_high;
    out[7] = 1;  // DP off
    return out;
}

// What each of the four digit positions should show for `value`.
static void golden_display(unsigned int value, ap_uint<8> expected[SEVEN_SEG_DIGITS]) {
    unsigned int msd_pos;
    if      (value >= 1000) msd_pos = 3;
    else if (value >= 100)  msd_pos = 2;
    else if (value >= 10)   msd_pos = 1;
    else                    msd_pos = 0;

    unsigned int scale = 1;
    for (unsigned int p = 0; p < SEVEN_SEG_DIGITS; p++) {
        if (p <= msd_pos) {
            expected[p] = golden_active_low(golden_pattern((value / scale) % 10));
        } else {
            expected[p] = golden_active_low(0x00);  // blanked leading zero
        }
        scale *= 10;
    }
}

// ---------------------------------------------------------------------------
// Runs the design long enough to observe one complete scan of all four
// digits, recording the pattern that was driven while each anode was active.
// This is how the multiplexed display is verified without any hardware: the
// design only ever drives one digit at a time, so we watch it over time.
// ---------------------------------------------------------------------------
static void observe_full_sweep(unsigned int value,
                               ap_uint<8> seen[SEVEN_SEG_DIGITS],
                               bool seen_valid[SEVEN_SEG_DIGITS]) {
    for (unsigned int p = 0; p < SEVEN_SEG_DIGITS; p++) {
        seen_valid[p] = false;
    }

    const unsigned long cycles =
        (unsigned long)SEVEN_SEG_DIGITS * SEVEN_SEG_REFRESH_TICKS + 16;

    for (unsigned long i = 0; i < cycles; i++) {
        ap_uint<8> seg = 0;
        ap_uint<4> an = 0;

        seven_segment(value, &seg, &an);

        // Exactly one anode bit must be low (active) at any time.
        int active = -1;
        int active_count = 0;
        for (int p = 0; p < (int)SEVEN_SEG_DIGITS; p++) {
            if (an[p] == 0) {
                active = p;
                active_count++;
            }
        }

        if (active_count != 1) {
            std::cout << "ERROR: " << active_count
                      << " anodes active at once (must be exactly 1)" << std::endl;
            return;
        }

        seen[active] = seg;
        seen_valid[active] = true;
    }
}

static bool check_value(unsigned int value) {
    ap_uint<8> seen[SEVEN_SEG_DIGITS];
    bool seen_valid[SEVEN_SEG_DIGITS];
    ap_uint<8> expected[SEVEN_SEG_DIGITS];

    observe_full_sweep(value, seen, seen_valid);
    golden_display(value, expected);

    bool ok = true;
    for (unsigned int p = 0; p < SEVEN_SEG_DIGITS; p++) {
        if (!seen_valid[p]) {
            std::cout << "value " << value << ": digit position " << p
                      << " was never enabled during a full sweep" << std::endl;
            ok = false;
            continue;
        }
        if (seen[p] != expected[p]) {
            std::cout << "value " << value << ": position " << p
                      << " drove 0x" << std::hex << seen[p].to_uint()
                      << ", expected 0x" << expected[p].to_uint() << std::dec
                      << std::endl;
            ok = false;
        }
    }
    return ok;
}

int main() {
    // Fixed vectors: single digit, two digits, leading-zero blanking,
    // both display boundaries, and a value using all four positions.
    unsigned int values[] = {0, 7, 42, 99, 100, 9999};

    bool failed = false;
    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        if (!check_value(values[i])) {
            failed = true;
        }
    }

    if (failed) {
        std::cout << "seven_segment test FAILED" << std::endl;
        return 1;
    }

    std::cout << "seven_segment test passed" << std::endl;
    return 0;
}
