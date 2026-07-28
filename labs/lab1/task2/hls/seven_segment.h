#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <ap_int.h>

// ---------------------------------------------------------------------------
// Blackboard seven-segment display
// ---------------------------------------------------------------------------
// The board has FOUR digits that all share ONE 8-bit cathode bus:
//
//     seg[0]=A  seg[1]=B  seg[2]=C  seg[3]=D
//     seg[4]=E  seg[5]=F  seg[6]=G  seg[7]=DP      (all active-low)
//
//     an[0] = rightmost digit (ones), an[3] = leftmost   (active-low)
//
// Because the cathodes are shared, only ONE digit can show a pattern at any
// instant. If two anodes were enabled at once, both digits would show the
// SAME pattern. Displaying different digits therefore requires time
// multiplexing: enable one digit, drive its pattern, switch to the next, and
// repeat fast enough that the eye blends them into a steady image.
//
// That requires the block to remember where it is in the scan, so this
// function is CLOCKED (it uses `static` variables, which become registers),
// unlike a plain combinational decoder.
// ---------------------------------------------------------------------------

// ap_clk frequency of the HLS block (PS FCLK_CLK0 in the Vivado design).
const unsigned SEVEN_SEG_CLK_HZ = 50000000;

// How long a single digit stays lit before the scan moves on.
// 50 000 cycles @ 50 MHz = 1 ms per digit -> 4 ms per full sweep = 250 Hz,
// far above the ~60 Hz needed to look flicker-free.
const unsigned SEVEN_SEG_REFRESH_TICKS = 50000;

// Number of physical digit positions on the board.
const unsigned SEVEN_SEG_DIGITS = 4;

// Displays `value` (0 .. 9999) on the four-digit display.
// Leading zeros are blanked, so 42 shows as "  42" and 7 shows as "   7".
void seven_segment(ap_uint<14> value, ap_uint<8> *seg, ap_uint<4> *an);

#endif
