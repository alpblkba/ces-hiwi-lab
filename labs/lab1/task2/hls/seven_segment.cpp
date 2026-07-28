#include "seven_segment.h"

// ---------------------------------------------------------------------------
// Which bit drives which physical segment.
//
// This is the ONLY place the board wiring appears. The values come straight
// from the constraints file (session2_task1.xdc):
//
//     seg[0] = SSEG_CA      seg[4] = SSEG_CE
//     seg[1] = SSEG_CB      seg[5] = SSEG_CF
//     seg[2] = SSEG_CC      seg[6] = SSEG_CG
//     seg[3] = SSEG_CD      seg[7] = SSEG_DP (decimal point, kept off)
//
// If the display were ever wired differently, only these seven lines change;
// the digit table below stays exactly as it is.
// ---------------------------------------------------------------------------
static const unsigned SEG_A = 1u << 0;
static const unsigned SEG_B = 1u << 1;
static const unsigned SEG_C = 1u << 2;
static const unsigned SEG_D = 1u << 3;
static const unsigned SEG_E = 1u << 4;
static const unsigned SEG_F = 1u << 5;
static const unsigned SEG_G = 1u << 6;

// ---------------------------------------------------------------------------
// Digit -> which segments light up (active-high; inverted later for the board)
// ---------------------------------------------------------------------------
//        A          Each case below just names the segments that form the
//      F   B        digit, so it can be checked against the drawing by eye
//        G          instead of decoding a hex constant.
//      E   C
//        D
// ---------------------------------------------------------------------------
static ap_uint<7> digit_to_pattern(ap_uint<4> digit) {
    switch (digit) {
        case 0: return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
        case 1: return SEG_B | SEG_C;                                   //  |
        case 2: return SEG_A | SEG_B | SEG_G | SEG_E | SEG_D;
        case 3: return SEG_A | SEG_B | SEG_G | SEG_C | SEG_D;
        case 4: return SEG_F | SEG_G | SEG_B | SEG_C;
        case 5: return SEG_A | SEG_F | SEG_G | SEG_C | SEG_D;
        case 6: return SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D;
        case 7: return SEG_A | SEG_B | SEG_C;                           // ‾|
        case 8: return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
        case 9: return SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
        default: return 0;  // blank
    }
}

// Converts an active-high pattern into what the board actually needs:
// segments are active-low, and the decimal point (bit 7) stays off.
static ap_uint<8> to_active_low(ap_uint<7> pattern_active_high) {
    ap_uint<8> out;
    out.range(6, 0) = ~pattern_active_high;
    out[7] = 1;  // DP off
    return out;
}

void seven_segment(ap_uint<14> value, ap_uint<8> *seg, ap_uint<4> *an) {
#pragma HLS INTERFACE ap_none port=value
#pragma HLS INTERFACE ap_none port=seg
#pragma HLS INTERFACE ap_none port=an
#pragma HLS INTERFACE ap_ctrl_none port=return
// Forces the block to complete one execution every clock cycle (II=1).
// Without this, Vitis HLS spreads the body over ~35 cycles, which would make
// the scan ~35x slower than intended and produce visible flicker. Note that
// C simulation cannot reveal this: only the synthesis report shows it.
#pragma HLS PIPELINE II=1

    // These two variables are the only state in the design. Because the
    // block is free-running (ap_ctrl_none), its body executes once per clock
    // cycle and `static` variables become flip-flops that keep their value
    // from one cycle to the next. This is what gives the design a notion of
    // "later", which a purely combinational decoder does not have.
    static ap_uint<16> tick = 0;  // counts clock cycles within one digit slot
    static ap_uint<2>  pos  = 0;  // which digit position is lit right now

    // Advance the scan. `pos` is 2 bits wide, so 3 + 1 wraps back to 0 on
    // its own: the scan runs 0 -> 1 -> 2 -> 3 -> 0 forever.
    if (tick == SEVEN_SEG_REFRESH_TICKS - 1) {
        tick = 0;
        pos  = pos + 1;
    } else {
        tick = tick + 1;
    }

    // Position of the most significant digit that should be visible.
    // Everything to the left of it is a leading zero and stays blank.
    ap_uint<2> msd_pos;
    if      (value >= 1000) msd_pos = 3;
    else if (value >= 100)  msd_pos = 2;
    else if (value >= 10)   msd_pos = 1;
    else                    msd_pos = 0;  // 0..9 still shows one digit

    // Pick the decimal digit belonging to the position currently being lit.
    ap_uint<4> digit;
    switch (pos) {
        case 0: digit = value % 10; break;         // ones
        case 1: digit = (value / 10) % 10; break;  // tens
        case 2: digit = (value / 100) % 10; break; // hundreds
        default: digit = (value / 1000) % 10;      // thousands
    }

    // Drive the shared cathode bus for this one digit...
    if (pos <= msd_pos) {
        *seg = to_active_low(digit_to_pattern(digit));
    } else {
        *seg = to_active_low(0x00);  // leading zero -> blank
    }

    // ...and enable exactly that digit's anode. This is the hard-coded
    // 0b1110 of a single-digit design, shifted to the active position:
    //   pos = 0 -> 1110, pos = 1 -> 1101, pos = 2 -> 1011, pos = 3 -> 0111
    *an = ~(ap_uint<4>(1) << pos);
}
