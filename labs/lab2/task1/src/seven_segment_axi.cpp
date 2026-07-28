#include "seven_segment_axi.h"

// ---------------------------------------------------------------------------
// Segment wiring and digit table - unchanged from Lab 1.
// The values come straight from the constraints file: seg[0]=SSEG_CA ...
// seg[6]=SSEG_CG, seg[7]=SSEG_DP. This is the only place the wiring appears.
// ---------------------------------------------------------------------------
static const unsigned SEG_A = 1u << 0;
static const unsigned SEG_B = 1u << 1;
static const unsigned SEG_C = 1u << 2;
static const unsigned SEG_D = 1u << 3;
static const unsigned SEG_E = 1u << 4;
static const unsigned SEG_F = 1u << 5;
static const unsigned SEG_G = 1u << 6;

static ap_uint<7> digit_to_pattern(ap_uint<4> digit) {
    switch (digit) {
        case 0: return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
        case 1: return SEG_B | SEG_C;
        case 2: return SEG_A | SEG_B | SEG_G | SEG_E | SEG_D;
        case 3: return SEG_A | SEG_B | SEG_G | SEG_C | SEG_D;
        case 4: return SEG_F | SEG_G | SEG_B | SEG_C;
        case 5: return SEG_A | SEG_F | SEG_G | SEG_C | SEG_D;
        case 6: return SEG_A | SEG_F | SEG_G | SEG_E | SEG_C | SEG_D;
        case 7: return SEG_A | SEG_B | SEG_C;
        case 8: return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
        case 9: return SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
        default: return 0;
    }
}

// Only the middle bar lit: used as a minus sign and as the "----" error
// display. Naming it SEG_G makes that obvious.
static const ap_uint<7> PATTERN_DASH  = SEG_G;
static const ap_uint<7> PATTERN_BLANK = 0;

static ap_uint<8> to_active_low(ap_uint<7> pattern_active_high) {
    ap_uint<8> out;
    out.range(6, 0) = ~pattern_active_high;
    out[7] = 1;  // DP off
    return out;
}

void seven_segment_axi(ap_uint<7> op1, ap_uint<7> op2, ap_uint<2> op_sel,
                       ap_uint<8> *seg, ap_uint<4> *an) {
// The three inputs become registers in an AXI-Lite slave that the processor
// writes. Everything else is exactly the Lab 1 interface: the segment and
// anode buses are plain wires, and ap_ctrl_none keeps the block free-running
// so the display keeps scanning without the CPU having to start it.
#pragma HLS INTERFACE s_axilite port=op1 bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op2 bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op_sel bundle=CTRL
#pragma HLS INTERFACE ap_none port=seg
#pragma HLS INTERFACE ap_none port=an
#pragma HLS INTERFACE ap_ctrl_none port=return
// One execution per clock cycle, so the display scan keeps the timing
// assumed by SEVEN_SEG_REFRESH_TICKS. See the Lab 1 source for details.
#pragma HLS PIPELINE II=1

    // -----------------------------------------------------------------------
    // Application logic - this is the part that is new compared to Lab 1.
    // -----------------------------------------------------------------------
    ap_int<16> result = 0;
    bool error = false;

    switch (op_sel) {
        case OP_ADD:
            result = (ap_int<16>)op1 + (ap_int<16>)op2;
            break;
        case OP_SUB:
            result = (ap_int<16>)op1 - (ap_int<16>)op2;
            break;
        case OP_MUL:
            result = (ap_int<16>)op1 * (ap_int<16>)op2;
            break;
        default:  // OP_DIV
            if (op2 == 0) {
                error = true;  // division by zero has no defined result
            } else {
                result = (ap_int<16>)op1 / (ap_int<16>)op2;
            }
            break;
    }

    bool negative = (result < 0);
    ap_uint<14> magnitude = negative ? (ap_uint<14>)(-result) : (ap_uint<14>)result;

    // A negative number needs one position for the minus sign, so it can only
    // use three of the four digits.
    if (magnitude > 9999 || (negative && magnitude > 999)) {
        error = true;
    }

    // -----------------------------------------------------------------------
    // Display block - identical to Lab 1 apart from the error and minus-sign
    // cases added above.
    // -----------------------------------------------------------------------
    static ap_uint<16> tick = 0;
    static ap_uint<2>  pos  = 0;

    if (tick == SEVEN_SEG_REFRESH_TICKS - 1) {
        tick = 0;
        pos  = pos + 1;
    } else {
        tick = tick + 1;
    }

    ap_uint<3> msd_pos;
    if      (magnitude >= 1000) msd_pos = 3;
    else if (magnitude >= 100)  msd_pos = 2;
    else if (magnitude >= 10)   msd_pos = 1;
    else                        msd_pos = 0;

    ap_uint<4> digit;
    switch (pos) {
        case 0: digit = magnitude % 10; break;
        case 1: digit = (magnitude / 10) % 10; break;
        case 2: digit = (magnitude / 100) % 10; break;
        default: digit = (magnitude / 1000) % 10;
    }

    ap_uint<3> pos_wide = pos;

    if (error) {
        *seg = to_active_low(PATTERN_DASH);              // "----"
    } else if (pos_wide <= msd_pos) {
        *seg = to_active_low(digit_to_pattern(digit));   // a real digit
    } else if (negative && pos_wide == msd_pos + 1) {
        *seg = to_active_low(PATTERN_DASH);              // minus sign
    } else {
        *seg = to_active_low(PATTERN_BLANK);             // leading zero
    }

    *an = ~(ap_uint<4>(1) << pos);
}
