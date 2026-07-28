#ifndef SEVEN_SEGMENT_AXI_H
#define SEVEN_SEGMENT_AXI_H

#include <ap_int.h>

// ---------------------------------------------------------------------------
// Lab 2: the Lab 1 seven-segment display driver, turned into an AXI-Lite
// peripheral that also performs the arithmetic.
//
// The display half of this file is IDENTICAL to Lab 1: same segment table,
// same four-digit time-multiplexed scan, same active-low anode logic. What
// is new is only:
//   * the operands and operation arrive over AXI-Lite instead of being a
//     single plain input port, and
//   * a small block of application logic computes the value to display.
//
// The board wiring is unchanged:
//     seg[0]=A .. seg[6]=G, seg[7]=DP     (active-low, shared by all digits)
//     an[0]=rightmost .. an[3]=leftmost   (active-low, one at a time)
// ---------------------------------------------------------------------------

const unsigned SEVEN_SEG_CLK_HZ        = 50000000;
const unsigned SEVEN_SEG_REFRESH_TICKS = 50000;  // 1 ms per digit @ 50 MHz
const unsigned SEVEN_SEG_DIGITS        = 4;

// Operation selector written by the processor.
const unsigned OP_ADD = 0;
const unsigned OP_SUB = 1;
const unsigned OP_MUL = 2;
const unsigned OP_DIV = 3;

// Computes op1 <op_sel> op2 and shows the result on the four-digit display.
//
// Operands are 0..99. Negative results (subtraction) are shown with a leading
// minus sign. Division by zero, and any result that does not fit on four
// digits, is shown as "----" so the display always has a defined state.
void seven_segment_axi(ap_uint<7> op1, ap_uint<7> op2, ap_uint<2> op_sel,
                       ap_uint<8> *seg, ap_uint<4> *an);

#endif
