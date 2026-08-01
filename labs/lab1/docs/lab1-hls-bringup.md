---
marp: true
theme: ces-kit
paginate: true
size: 16:9
lang: en
footer: "Lab 1 — Vitis HLS bring-up"
title: "Lab 1: Vitis HLS and the seven-segment display"
description: "KIT CES — Customized Embedded Processors Lab, Lab 1 (HLS bring-up + four-digit seven-segment driver)"
headingDivider: 0
---

<!--
================================================================================
 Lab 1 deck — KIT / Chair of Embedded Systems (CES)
 Source material: github.com/alpblkba/ces-hiwi-lab (labs/lab1)

 TARGET LENGTH: 14 content slides (feedback: "10-15 pages, not 40")

 SCREENSHOT CONVENTION
   Every <div class="shot"> is a reserved slot. Replace the whole div with
       ![w:620](img/NN-name.png)
   Only 5 screenshots are used, on purpose - the crucial steps only.

 BUILD
   marp lab1-hls-bringup.md --theme theme/ces-kit.css -o lab1.pdf
   python3 md2pptx.py lab1-hls-bringup.md --template CES_ppt_template-1.pptx -o lab1.pptx
================================================================================
-->

<!-- _class: title -->
<!-- _paginate: false -->

# Lab 1: Vitis HLS and the seven-segment display

## Describing hardware in C++ instead of Verilog

Customized Embedded Processors Lab &nbsp;·&nbsp; Chair of Embedded Systems (CES)

Vitis HLS 2022.2 &nbsp;·&nbsp; Blackboard Zynq-7000 (`xc7z007sclg400-1`)

---

## What this lab is about

- You write a **C++ function**. Vitis HLS turns it into **real hardware** (RTL).
- The function is *not* software: after synthesis it becomes a block with input and output wires.
- Target: drive the Blackboard's **four-digit seven-segment display**.
- No Verilog and no VHDL is written anywhere in this lab.

<div class="flow center">C++ function  →  C simulation  →  C synthesis  →  RTL + reports  →  exported IP
                                                                       │
                                                    Lab 2:  Vivado block design  →  board</div>

<div class="note"><b>Tool roles.</b> Vitis HLS turns C/C++ into an IP block. Vivado builds the system around it. Vitis builds the software that runs on the ARM core.</div>

---

## The display: four digits, one bus

<div class="shot">
<b>img/03-seven-segment-board.png</b>
Photo or diagram of the Blackboard display,
segments labelled A…G and the four digit
positions labelled an0…an3
</div>

- Seven segments (A…G) plus a decimal point form one digit.
- The board has **four digits**, but they **share one 8-bit segment bus**.
- `an[3:0]` selects *which* digit is currently connected to that bus.
- Segments and anodes are both **active-low**: a `0` turns something on.

---

## You cannot show two digits at once

<div class="warn">If two anodes are enabled at the same time, both digits receive the <b>same</b> pattern. Displaying <code>4</code> and <code>2</code> simultaneously is physically impossible.</div>

- The display is driven by **time multiplexing**: light one digit, then the next, quickly.
- At ~1 ms per digit the eye blends them into one steady four-digit number.
- So the block must **remember** which digit it is currently driving.

<div class="flow">time ──►
  an0 lit, seg = pattern of "2"     (1 ms)
  an1 lit, seg = pattern of "4"     (1 ms)
  an2 lit, seg = blank              (1 ms)
  an3 lit, seg = blank              (1 ms)     … repeats 250x per second</div>

---

## Combinational vs. clocked

- A **combinational** function has no memory: its output depends only on its inputs *right now*.
- Such a function can never scan, because scanning needs a notion of **"later"**.
- In HLS, `static` local variables become **registers** — they keep their value between clock cycles.
- With `ap_ctrl_none` the block is **free-running**: its body executes once every clock cycle.

```cpp
static ap_uint<16> tick = 0;   // counts cycles inside one digit slot
static ap_uint<2>  pos  = 0;   // which digit is lit right now
```

<div class="note"><code>pos</code> is 2 bits wide, so <code>3 + 1</code> wraps to <code>0</code> on its own: the scan runs 0 → 1 → 2 → 3 → 0 forever.</div>

---

## Which bit drives which segment

The constraints file (XDC) fixes this mapping — it is not a free choice:

```text
seg[0] = SSEG_CA (A)     seg[4] = SSEG_CE (E)
seg[1] = SSEG_CB (B)     seg[5] = SSEG_CF (F)
seg[2] = SSEG_CC (C)     seg[6] = SSEG_CG (G)
seg[3] = SSEG_CD (D)     seg[7] = SSEG_DP (decimal point)
```

```cpp
static const unsigned SEG_A = 1u << 0;   // … through …
static const unsigned SEG_G = 1u << 6;

case 1: return SEG_B | SEG_C;            //  |    (right bar)
case 7: return SEG_A | SEG_B | SEG_C;    // ‾|
```

<div class="warn"><b>Get the bit order wrong and the display lies.</b> If A lands in the wrong bit, every pattern is mirrored: <code>2</code> appears as <code>5</code>, <code>6</code> as <code>9</code>. Symmetric digits such as <code>0</code> and <code>8</code> hide the mistake.</div>

---

## Driving the anodes

- Only **one** anode may be low at any instant — that is the whole idea of the scan.
- A single-digit design would hardcode `an = 0b1110` (digit 0 always on).
- The four-digit version is that same constant, **shifted to the active position**:

```cpp
*an = ~(ap_uint<4>(1) << pos);
```

| `pos` | `an` | digit lit |
|-------|------|-----------|
| 0 | `1110` | rightmost (ones) |
| 1 | `1101` | tens |
| 2 | `1011` | hundreds |
| 3 | `0111` | leftmost (thousands) |

---

## Putting it together

```cpp
void seven_segment(ap_uint<14> value, ap_uint<8> *seg, ap_uint<4> *an) {
#pragma HLS INTERFACE ap_none port=value
#pragma HLS INTERFACE ap_none port=seg
#pragma HLS INTERFACE ap_none port=an
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1

    /* advance tick / pos                        */
    /* pick the digit belonging to this position */
    /* blank the leading zeros                   */
    /* drive seg and an                          */
}
```

- `value` is 0…9999. Two digits are the standard case; three and four come for free.
- Leading zeros are blanked, so `42` shows as `··42`, not `0042`.

---

## The testbench

- The synthesized function has **no `cin`, no `scanf`, no file I/O** — none of that is hardware.
- The testbench is plain C++ that calls the function and checks its outputs.
- It uses **fixed values**, so any failure is reproducible.

```cpp
unsigned int values[] = {0, 7, 42, 99, 100, 9999};
```

- To observe a full scan it calls the function ~200 000 times and records which
  pattern each anode received.
- It also asserts that **exactly one anode is active** in every single cycle.

<div class="note">Interactive input arrives in <b>Lab 2.2</b>, from a C program on the ARM processor — never from the testbench.</div>

---

## Creating the Vitis HLS project

<div class="shot">
<b>img/10-hls-create-project.png</b>
Create Project wizard: project name,
then the Add Sources page showing the
design and testbench files
</div>

```bash
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh
vitis_hls &
```

- Design files: `seven_segment.cpp`, `seven_segment.h`
- Testbench: `seven_segment_tb.cpp` — added on the **separate** testbench page.
- Top function `seven_segment`; part **`xc7z007sclg400-1`**; clock **20 ns** (50 MHz).

---

## C simulation

<div class="shot">
<b>img/11-csim-passed.png</b>
Console after Run C Simulation showing
"seven_segment test passed" and
"CSim done with 0 errors"
</div>

- C simulation compiles the function as ordinary C++ — **no hardware is generated yet**.
- It answers one question: *is the digit-to-pattern mapping correct?*
- It runs your fixed test vectors and reports pass or fail.

<div class="warn">C simulation cannot see <b>timing</b>. Everything on the next slide is invisible here.</div>

---

## C synthesis — and the number that matters

<div class="shot">
<b>img/12-synthesis-interval.png</b>
Synthesis Summary with the Timing and
Latency tables visible, the Interval
column highlighted
</div>

- Check **Interval = 1**: the block performs one execution per clock cycle.
- Without `#pragma HLS PIPELINE II=1` the tool spreads the body over ~35 cycles.
  The scan then runs 35× too slowly and the display **flickers visibly**.
- Also check: estimated clock under 20 ns, and no BRAM or DSP for a decoder.

<div class="note"><b>This is the lesson of the lab.</b> The C simulation passes either way. Only the synthesis report reveals the problem.</div>

---

## Exporting the IP

<div class="shot">
<b>img/13-export-ip.png</b>
Export RTL dialog, format Vivado IP,
and the resulting
<code>solution1/impl/ip/component.xml</code>
</div>

- **Solution → Export RTL**, format **Vivado IP (.zip)**.
- Verify afterwards that `solution1/impl/export.zip` and
  `solution1/impl/ip/component.xml` exist.
- This exported IP is exactly what Lab 2 imports into a Vivado block design.

---

## Checklist

<div class="flow">[ok] Vitis HLS project created, part and clock configured
[ok] segment table written from the XDC pin mapping
[ok] scan implemented with static tick / pos registers
[ok] anodes driven as ~(1 << pos), exactly one active at a time
[ok] C simulation passes on the fixed vectors
[ok] synthesis report checked: Interval = 1, timing met
[ok] IP exported

[next] Lab 2 — the same block, controlled by the processor over AXI</div>

| Reference result | Value |
|------------------|-------|
| Interval (II) | 1 |
| Estimated Fmax | ~111 MHz (target 50 MHz) |
| BRAM / DSP | 0 / 0 |

<div class="note"><b>Deliverables.</b> The three source files, the synthesis report, the exported IP, and a short answer to: <i>why does this display need a clock at all?</i></div>
