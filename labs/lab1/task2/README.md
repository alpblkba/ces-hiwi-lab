# Lab 1.2: Seven-segment display with HLS

## Objective

Drive the Blackboard's four-digit seven-segment display from a C++ function
synthesized with Vitis HLS. No Verilog or VHDL is written.

The display shows a number you pass in. Two digits are the standard case; the
same design handles three and four digits without extra work.

## The hardware you are driving

The board has four digits, and they **share one 8-bit segment bus**:

```text
seg[0] = SSEG_CA (A)     seg[4] = SSEG_CE (E)
seg[1] = SSEG_CB (B)     seg[5] = SSEG_CF (F)
seg[2] = SSEG_CC (C)     seg[6] = SSEG_CG (G)
seg[3] = SSEG_CD (D)     seg[7] = SSEG_DP (decimal point)

an[0] = rightmost digit  …  an[3] = leftmost digit
```

Segments and anodes are **active-low**: a `0` turns a segment or a digit on.

Because the cathodes are shared, **only one digit can display a pattern at any
instant**. If two anodes are enabled together, both digits show the same thing.

## The consequence: the design needs a clock

To show different digits you light them **one at a time, in turn, quickly**. At
roughly 1 ms per digit the eye merges them into one steady number.

That means the block has to remember where it is in the scan. A purely
combinational function cannot: its output depends only on its inputs *right now*,
so it has no notion of "later".

In HLS, `static` local variables become registers, and `ap_ctrl_none` makes the
block free-running so its body executes once per clock cycle:

```cpp
static ap_uint<16> tick = 0;   // cycles elapsed inside the current digit slot
static ap_uint<2>  pos  = 0;   // which digit is lit right now
```

`pos` is two bits wide, so `3 + 1` wraps to `0` by itself.

## Design description

Input:

- one value, 0 … 9999 (only 0 … 99 is needed for the mandatory two-digit case)

Outputs:

- `seg[7:0]` — the segment pattern for the digit currently lit
- `an[3:0]` — which digit is currently lit, active-low, exactly one bit at `0`

Behaviour:

- leading zeros are blanked, so `42` shows as `··42` rather than `0042`
- the segment table is derived from the pin mapping above, not guessed
- every input produces a defined output

## Two things that are easy to get wrong

**Segment bit order.** If segment A is placed in the wrong bit, every pattern is
mirrored: `2` appears as `5`, `6` as `9`. Symmetric digits such as `0` and `8`
still look correct, which hides the mistake. Write the table from named bits
(`SEG_A … SEG_G`) so the wiring appears in exactly one place.

**Initiation interval.** Add `#pragma HLS PIPELINE II=1`. Without it Vitis HLS
spreads the function body over roughly 35 clock cycles, the scan runs about 35×
too slowly, and the display flickers visibly. **C simulation cannot detect this** —
only the Interval column of the synthesis report shows it.

## Testbench

The testbench is plain C++ and is never synthesized. It uses **fixed values**, so
any failure is reproducible:

```cpp
unsigned int values[] = {0, 7, 42, 99, 100, 9999};
```

To observe a complete scan it calls the function often enough to cover all four
digit slots and records which pattern each anode received. It also checks that
**exactly one anode is active in every cycle**.

There is no keyboard input here. Interactive input arrives in Lab 2.2, from a C
program running on the ARM processor.

## Workflow

1. Write `seven_segment.h`, `seven_segment.cpp`, `seven_segment_tb.cpp`.
2. Create the Vitis HLS project; part `xc7z007sclg400-1`, clock 20 ns.
3. Run C simulation until the fixed vectors pass.
4. Run C synthesis and confirm **Interval = 1** and that timing is met.
5. Export the design as a Vivado IP.

## Expected result

- Three source files and a passing C simulation.
- A synthesis report showing Interval = 1, no BRAM and no DSP.
- An exported IP under `solution1/impl/ip/`.
- A short answer to: *why does this display need a clock at all?*

## Optional extensions

- Show three or four digits: the scan already covers all four positions, so only
  the blanking rule changes.
- Use the decimal point (`seg[7]`).
- Make the refresh rate a parameter and find the point where flicker becomes visible.
