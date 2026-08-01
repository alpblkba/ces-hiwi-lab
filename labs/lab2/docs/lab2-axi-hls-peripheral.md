---
marp: true
theme: ces-kit
paginate: true
size: 16:9
lang: en
footer: "Lab 2 — HLS calculator as an AXI peripheral"
title: "Lab 2: A calculator in HLS, controlled by the processor"
description: "KIT CES — Customized Embedded Processors Lab, Lab 2 (Lab 2.1 Vivado integration + Lab 2.2 Vitis software control)"
headingDivider: 0
---

<!--
================================================================================
 Lab 2 deck — KIT / Chair of Embedded Systems (CES)
 Source material: github.com/alpblkba/ces-hiwi-lab (labs/lab2)

 TARGET LENGTH: 15 content slides (feedback: "10-15 pages, not 40")

 DESIGN NOTE
   Lab 2 deliberately reuses the Lab 1 display block unchanged. The whole
   teaching point is that students should be able to derive this lab from
   their own Lab 1 solution by adding pragmas and one switch statement.

 SCREENSHOT CONVENTION
   Every <div class="shot"> is a reserved slot. Replace the whole div with
       ![w:620](img/NN-name.png)

 BUILD
   marp lab2-axi-hls-peripheral.md --theme theme/ces-kit.css -o lab2.pdf
   python3 md2pptx.py lab2-axi-hls-peripheral.md --template CES_ppt_template-1.pptx -o lab2.pptx
================================================================================
-->

<!-- _class: title -->
<!-- _paginate: false -->

# Lab 2: A calculator in HLS

## The Lab 1 display block, now driven by the processor

Customized Embedded Processors Lab &nbsp;·&nbsp; Chair of Embedded Systems (CES)

Vitis HLS · Vivado · Vitis 2022.2 &nbsp;·&nbsp; Blackboard Zynq-7000 (`xc7z007sclg400-1`)

---

## Lab 2 goal

<div class="shot">
<b>img/02-system-overview.png</b>
Block diagram: ARM PS → AXI Interconnect →
seven_segment_axi → four-digit display
</div>

- You type two numbers and an operation into a **terminal**.
- The ARM processor sends them to your **HLS block** over AXI4-Lite.
- The block computes the result and shows it on the **seven-segment display**.
- Still no Verilog: the arithmetic and the display logic are both C++.

---

## What actually changes from Lab 1

<div class="flow">Lab 1                                Lab 2
--------------------------------     --------------------------------
one input: value                     three inputs: op1, op2, op_sel
#pragma ... ap_none port=value       #pragma ... s_axilite port=op1
                                     #pragma ... s_axilite port=op2
                                     #pragma ... s_axilite port=op_sel
                                     + one switch statement (the maths)

segment table                        IDENTICAL
active-low conversion                IDENTICAL
tick / pos scan registers            IDENTICAL
*an = ~(1 << pos)                    IDENTICAL
#pragma HLS PIPELINE II=1            IDENTICAL</div>

<div class="note"><b>This is the point of the lab.</b> If your Lab 1 works, Lab 2 is three pragmas plus the arithmetic. The display half is not rewritten — it is reused.</div>

---

## Why AXI4-Lite?

- The ARM core **cannot call** hardware like a C function. The block must look like **memory**.
- Software writes to an address; the fabric decodes it and hands the value to your logic.
- **AXI4-Lite** fits: a handful of control registers, no streaming bandwidth needed.

| Interface | Use |
|-----------|-----|
| Full AXI4 | high-performance burst transfers |
| **AXI4-Lite** | **simple register access — this lab** |
| AXI4-Stream | continuous data, no addresses |

<div class="flow center">ARM processor  →  AXI Interconnect  →  AXI4-Lite slave (HLS IP)  →  display</div>

---

## The new interface

```cpp
void seven_segment_axi(ap_uint<7> op1, ap_uint<7> op2, ap_uint<2> op_sel,
                       ap_uint<8> *seg, ap_uint<4> *an) {
#pragma HLS INTERFACE s_axilite port=op1    bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op2    bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op_sel bundle=CTRL
#pragma HLS INTERFACE ap_none   port=seg
#pragma HLS INTERFACE ap_none   port=an
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1
```

- Only the three **inputs** are memory-mapped. `seg` and `an` stay plain wires.
- `ap_ctrl_none` keeps the block **free-running**, so the display never stops scanning.
- Operands are 0…99; `op_sel` selects the operation.

---

## The arithmetic, and its edge cases

```cpp
switch (op_sel) {
    case OP_ADD: result = op1 + op2; break;   // 0
    case OP_SUB: result = op1 - op2; break;   // 1
    case OP_MUL: result = op1 * op2; break;   // 2
    default:                                  // 3 = divide
        if (op2 == 0) error = true;           // no defined result
        else result = op1 / op2;
}
```

- **Negative** results (subtraction) show a leading minus sign — segment G alone.
- **Division by zero** and anything that will not fit shows `----`.
- The display always has a **defined state**: hardware cannot throw an exception.

<div class="note">Range check: <code>99 × 99 = 9801</code> still fits on four digits, so multiplication never overflows here.</div>

---

## The register map

After synthesis, Vitis HLS generates `xseven_segment_axi_hw.h`:

| Offset | Register |
|--------|----------|
| `0x00` … `0x0c` | reserved |
| `0x10` | `op1` |
| `0x18` | `op2` |
| `0x20` | `op_sel` |

<div class="warn">Because the block is <code>ap_ctrl_none</code>, there is <b>no <code>ap_start</code> register</b>. The software never starts the block and never polls for completion — it only writes these three values.</div>

- Always read these offsets from the generated header, never guess them.

---

## Synthesize and export

<div class="shot">
<b>img/08-synth-axi-interface.png</b>
Synthesis report Interface table showing
the s_axi_CTRL ports plus seg / an,
and Interval = 1
</div>

- Top function `seven_segment_axi`, part `xc7z007sclg400-1`, clock 20 ns.
- The report must show an AXI4-Lite slave named **`s_axi_CTRL`**, and `seg` / `an` as `ap_none`.
- Confirm **Interval = 1** again — same flicker trap as Lab 1.
- **Export RTL** as Vivado IP; verify `solution1/impl/ip/component.xml`.

---

## Vivado: project and constraints

- New RTL project, part **`xc7z007sclg400-1`**, **no** design sources.
- Add the **Blackboard XDC**: it maps `seg[7:0]` and `an[3:0]` to the display pins.
- No extra Verilog or VHDL is added in this lab — the anode logic lives in the HLS block.

```text
seg[0] → K14 (SSEG_CA)  …  seg[7] → K18 (SSEG_DP)
an[0]  → K19 (SSEG_AN0) …  an[3]  → L16 (SSEG_AN3)
```

<div class="note">This is why the scan was written in HLS: the block design stays simple, and the lab never asks you to write RTL.</div>

---

## Vivado: block design

<div class="shot">
<b>img/10-block-design.png</b>
Diagram after Run Connection Automation:
ZYNQ7 PS → AXI Interconnect →
seven_segment_axi_0/s_axi_CTRL
</div>

- **Create Block Design**, add **ZYNQ7 Processing System**, then **Run Block Automation**.
- **Settings → IP → Repository**: add the exported `solution1/impl/ip` directory.
- **Add IP → `seven_segment_axi`** from the catalog under *UserIP*.
- **Run Connection Automation** to wire AXI, clock and reset.

---

## Vivado: external ports and address

<div class="shot">
<b>img/11-external-ports.png</b>
Right-click seg / an → Make External,
plus the Address Editor showing
s_axi_CTRL at 0x4000_0000
</div>

- Right-click the `seg` and `an` pins → **Make External**.
- Address Editor: assign `s_axi_CTRL` to **`0x4000_0000`**, range 64K.

<div class="warn"><b>Naming matters.</b> The external ports must be exactly <code>seg</code> and <code>an</code> to match the XDC. If Vivado creates <code>seg_0</code>, rename it in External Port Properties.</div>

---

## Vivado: build and export

- **Create HDL Wrapper**, then **Run Synthesis → Implementation → Generate Bitstream**.
- **File → Export → Export Hardware**, *include bitstream* → `seven_segment_wrapper.xsa`.

<div class="flow center">synthesis = netlist   ·   implementation = place & route + timing   ·   bitstream = FPGA configuration</div>

<div class="warn">Whenever the HLS code changes, the IP must be re-exported, the block design refreshed (<b>Report IP Status → Upgrade</b>), and the bitstream and XSA regenerated. A stale XSA is the most common reason a working design misbehaves on the board.</div>

---

## Vitis: platform and application

<div class="shot">
<b>img/13-vitis-projects.png</b>
Create Platform Project from the XSA,
then New Application Project on
ps7_cortexa9_0, Empty Application
</div>

- Platform project from `seven_segment_wrapper.xsa`, then **Build Project**.
- Application project `seven_segment_app`, processor `ps7_cortexa9_0`, standalone, empty.
- The XSA tells Vitis which processor exists and **where your IP is mapped** — platform first.

---

## Vitis: the software

```c
#define CALC_BASEADDR   XPAR_SEVEN_SEGMENT_AXI_0_S_AXI_CTRL_BASEADDR
#define OP1_REG_OFFSET    0x10
#define OP2_REG_OFFSET    0x18
#define OPSEL_REG_OFFSET  0x20

op1 = read_operand("First number");     /* over the terminal */
op2 = read_operand("Second number");
op_sel = read_operation();

Xil_Out32(CALC_BASEADDR + OP1_REG_OFFSET,   op1);
Xil_Out32(CALC_BASEADDR + OP2_REG_OFFSET,   op2);
Xil_Out32(CALC_BASEADDR + OPSEL_REG_OFFSET, op_sel);
```

- Three writes, and the display updates on the very next clock cycle.
- Use the **generated** base-address macro from `xparameters.h` — never a literal address.
- Validate the operands in software too: a 7-bit register silently truncates 100 to 4.

---

## Run on the board

<div class="shot">
<b>img/15-board-result.png</b>
Terminal showing the entered operands
next to a photo of the display
holding the result
</div>

- **Run As → Launch on Hardware**: Vitis programs the bitstream, then starts the ELF.
- Open a serial terminal at **115200 baud** to type the operands.
- Try `40 + 2` → `42`, then `5 - 9` → `-4`, then `7 / 0` → `----`.

<div class="flow">[ok] AXI4-Lite IP exported            [ok] platform + application built
[ok] block design validated           [ok] operands accepted over the terminal
[ok] bitstream and XSA generated      [ok] result correct on the display</div>

---

## Checklist and deliverables

<div class="flow">[ok] Lab 1 display block reused unchanged
[ok] three AXI4-Lite pragmas added, top function renamed
[ok] arithmetic implemented, division by zero handled
[ok] Interval = 1 confirmed in the synthesis report
[ok] IP exported, imported, address assigned
[ok] bitstream and XSA regenerated from the current IP
[ok] software writes op1 / op2 / op_sel and the display follows</div>

| Reference result | Value |
|------------------|-------|
| HLS Interval (II) | 1, Fmax ~81 MHz |
| Post-route timing | WNS +12.7 ns, 0 failing endpoints |
| Utilisation | 7.4 % LUT, 4.1 % FF, 1 DSP, 0 BRAM |
| Bonded I/O | 12 (8 segment + 4 anode) |

<div class="note"><b>Deliverables.</b> The HLS sources, the block design, the XSA, the Vitis application, a board demo, and a short answer to: <i>which registers does the software write, and what does the hardware compute?</i></div>
