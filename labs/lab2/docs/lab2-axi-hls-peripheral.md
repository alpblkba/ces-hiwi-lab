---
marp: true
theme: ces-kit
paginate: true
size: 16:9
lang: en
footer: "Lab 2 — Processor-controlled HLS peripheral"
title: "Lab 2: HLS IP integration and processor-side control"
description: "KIT CES — Customized Embedded Processors Lab, Lab 2 (Lab 2.1 Vivado integration + Lab 2.2 Vitis software control)"
headingDivider: 0
---

<!--
================================================================================
 Lab 2 deck — KIT / Chair of Embedded Systems (CES)
 Condensed from Lab2.pptx (48 slides -> 17 content slides + title + 2 dividers).

 SCOPE NOTE
   This deck covers the flow that exists in the repository today:
   a single-digit value 0-9 written by software through AXI4-Lite.
   The planned two-operand calculator variant is a separate revision.

 WHAT WAS CUT relative to Lab2.pptx
   - the three "what is AXI" slides            -> merged into slide 4
   - the generic Vivado new-project walkthrough -> one slide (9)
   - repeated "external port naming" warnings   -> stated once, slide 13
   - separate synthesis / implementation / bitstream / XSA slides -> slide 15
   - the two Vitis platform+application slides and both build slides -> 16 and 18
   - all prose paragraphs longer than two lines -> rewritten as 3-4 bullets

 SCREENSHOT CONVENTION
   Each <div class="shot"> is a reserved slot. Replace it with
       ![w:620](img/NN-name.png)
   when the real screenshot is available. Most images already exist in
   Lab2.pptx and can be re-exported from there at the listed names.

 BUILD
   marp lab2-axi-hls-peripheral.md --theme theme/ces-kit.css -o lab2.pdf
   python3 md2pptx.py lab2-axi-hls-peripheral.md --template CES_ppt_template-1.pptx -o lab2.pptx
================================================================================
-->

<!-- _class: title -->
<!-- _paginate: false -->

# Lab 2: Processor-controlled HLS peripheral

## From an HLS function to an AXI4-Lite IP driven by software

Customized Embedded Processors Lab &nbsp;·&nbsp; Chair of Embedded Systems (CES)

Vitis HLS · Vivado · Vitis 2022.2 &nbsp;·&nbsp; Blackboard Zynq-7000 (`xc7z007sclg400-1`)

---

## Lab 2 goal

<div class="shot">
<b>img/02-system-overview.png</b>
Block diagram: ARM PS → AXI Interconnect →
seven_segment_axi → display
<small>reuse the diagram from Lab2.pptx slide 8</small>
</div>

- **Lab 2.1** — turn the Lab 1 decoder into an AXI4-Lite IP and integrate it into a Zynq block design.
- **Lab 2.2** — control that hardware from a C program running on the ARM core.
- The displayed value is chosen by **software at runtime**, not hardcoded in the hardware design.
- Still no Verilog or VHDL: the decoder logic remains C/C++.

---

## Why AXI4-Lite?

- The ARM processor **cannot call** hardware logic like a C function. The IP must appear as a **memory-mapped peripheral**.
- Software writes a value to a register address; the fabric decodes that address and drives the display.
- **AXI4-Lite** is the right choice here: a few control registers, no high-bandwidth streaming.

| Interface | Use |
|-----------|-----|
| Full AXI4 | high-performance, memory-mapped burst transfers |
| **AXI4-Lite** | **simple control/status register access — this lab** |
| AXI4-Stream | direct data streaming, no addresses |

<div class="flow center">ARM processor  →  AXI Interconnect  →  AXI4-Lite slave (HLS IP)  →  7-segment display</div>

---

## What you will do

<div class="flow">Lab 2.1 — hardware
  1. copy the Lab 1 decoder into new Lab 2 source files
  2. add AXI4-Lite interface pragmas, rename the top function
  3. run C synthesis, inspect the generated interface
  4. export the result as a Vivado IP
  5. build a Zynq block design, connect AXI, make seg/an external
  6. assign an address, generate the bitstream, export the XSA

Lab 2.2 — software
  7. create a Vitis platform from the XSA
  8. write a C application that writes the digit register
  9. run it on the board and observe the display</div>

<div class="note"><b>Deliverable.</b> A working board demo plus a one-paragraph answer to: <i>which register does the software write, and what does the hardware compute?</i></div>

---

## HLS: new source files for Lab 2

<div class="shot">
<b>img/05-new-source-files.png</b>
Vitis HLS Explorer, right-click Source →
New File, showing
<code>seven_segment_axi.cpp/.h</code>
</div>

- New HLS project, then **create new files** — do not add the Lab 1 files as linked resources.
- `seven_segment_axi.cpp` and `seven_segment_axi.h`; copy the Lab 1 decoder body into them.
- Lab 1 stays untouched and still works; Lab 2 evolves independently.
- A testbench is optional here — C simulation may be skipped and you go straight to C synthesis.

---

## HLS: the AXI4-Lite interface

```cpp
void seven_segment_axi(ap_uint<4> digit,
                       ap_uint<8> *seg,
                       ap_uint<4> *an) {
#pragma HLS INTERFACE s_axilite port=digit  bundle=CTRL
#pragma HLS INTERFACE ap_none   port=seg
#pragma HLS INTERFACE ap_none   port=an
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL

    ap_uint<7> pattern_active_high;   // Lab 1 switch-case decoder

    ap_uint<8> pattern_active_low;
    pattern_active_low.range(6, 0) = ~pattern_active_high;
    pattern_active_low[7] = 1;        // decimal point off (active-low)

    *seg = pattern_active_low;
    *an  = 0b1110;                    // enable one digit (active-low)
}
```

- Also rename the include guard in the header to `SEVEN_SEGMENT_AXI_H`.
- The decoding logic itself is **unchanged** from Lab 1.

---

## HLS: what the pragmas produce

| Port | Interface | Driven by |
|------|-----------|-----------|
| `digit` | `s_axilite`, bundle `CTRL` | ARM processor, over AXI4-Lite |
| `seg[7:0]` | `ap_none` | FPGA fabric → display cathodes + DP |
| `an[3:0]` | `ap_none` | FPGA fabric → digit enable / anodes |
| `return` | `s_axilite`, bundle `CTRL` | block control register (start/done) |

- Only `digit` is memory-mapped. **`seg` and `an` are direct hardware outputs.**
- They will be made external in the block design and pinned through the XDC file.
- The Blackboard display is active-low, so the Lab 1 active-high pattern is inverted once, at the output.

---

## HLS: synthesize and export the IP

<div class="shot-row">
<div class="shot">
<b>img/08a-synth-axi-report.png</b>
Synthesis Summary showing the
<code>s_axi_CTRL</code> interface and the
register offsets
</div>
<div class="shot">
<b>img/08b-export-ip.png</b>
Export RTL dialog:
Vendor <code>ces.kit.edu</code>,
Library <code>hls</code>, Version 1.0
</div>
</div>

- Project → Project Settings → Synthesis: top function `seven_segment_axi`, then **Run C Synthesis**.
- The report must show an AXI4-Lite slave named **`s_axi_CTRL`** and note the **`digit` offset (0x10)** — you need it in software.
- Export as **Vivado IP (.zip)**; Display Name `seven_segment_axi`, Taxonomy `/UserIP`.
- Verify `solution1/impl/export.zip` and `solution1/impl/ip/component.xml`.

---

## Vivado: project and constraints

<div class="shot">
<b>img/09-vivado-project-xdc.png</b>
New Project summary page +
the Blackboard XDC file open,
seg / an pin constraints visible
</div>

- New RTL project, e.g. `~/vivado/lab2_task1`; part **`xc7z007sclg400-1`**. Add **no** design sources.
- Add the **Blackboard XDC** constraints file — at project creation or any time before bitstream generation.
- The XDC maps the logical ports `seg[7:0]` and `an[3:0]` to the physical display pins.
- Without it Vivado can still synthesize, but it does not know which pins drive the display.

---

## Vivado: block design and Zynq PS

<div class="shot">
<b>img/10-block-automation.png</b>
Diagram after Run Block Automation,
ZYNQ7 Processing System with
DDR and FIXED_IO external
</div>

- Flow Navigator → **Create Block Design**, name `seven_segment`.
- **Add IP → ZYNQ7 Processing System**, then **Run Block Automation**.
- Accept the defaults: Make Interface External = `FIXED_IO`, `DDR`; cross triggers disabled.
- `DDR` and `FIXED_IO` must be external so the PS can reach board DDR and its fixed pins.

---

## Vivado: import the HLS IP

<div class="shot-row">
<div class="shot">
<b>img/11a-ip-repository.png</b>
Settings → IP → Repository,
pointing at
<code>solution1/impl/ip</code>
</div>
<div class="shot">
<b>img/11b-ip-catalog.png</b>
IP Catalog → UserIP →
<code>seven_segment_axi</code>
</div>
</div>

- **Tools → Settings → IP → Repository**, add the exported `ip/` directory (not the `.zip`, not `impl/`).
- After a refresh the IP appears in the **IP Catalog** under **UserIP**.
- Back in the Diagram: **Add IP → `seven_segment_axi`**.
- IP not showing up = wrong directory or catalog not refreshed. Nothing else.

---

## Vivado: Connection Automation

<div class="shot">
<b>img/12-connection-automation.png</b>
Diagram after Run Connection Automation:
PS → AXI Interconnect →
seven_segment_axi_0/s_axi_CTRL
</div>

- **Run Connection Automation**, master `/processing_system7_0/M_AXI_GP0`, Bridge IP = **New AXI Interconnect**, all clocks **Auto**.
- Vivado inserts the interconnect and wires clock and reset automatically.
- Result: `M_AXI_GP0` → AXI Interconnect → `seven_segment_axi_0/s_axi_CTRL`.
- `seg[7:0]` and `an[3:0]` stay unconnected here — they are not part of the AXI path.

---

## Vivado: make `seg` and `an` external

<div class="shot">
<b>img/13-external-ports.png</b>
Right-click on the seg / an pins →
Make External, plus the resulting
external port names
</div>

- Right-click each output pin → **Make External**.
- These are physical outputs, not registers: software never touches them.

<div class="warn"><b>Naming matters.</b> The external ports must be called exactly <code>seg[7:0]</code> and <code>an[3:0]</code> to match the XDC file. If Vivado creates <code>seg_0[7:0]</code>, rename it via right-click → <b>External Port Properties</b>.</div>

---

## Vivado: address assignment

<div class="shot">
<b>img/14-address-editor.png</b>
Address Editor showing
<code>s_axi_CTRL</code> at
<code>0x4000_0000</code>, range 64K
</div>

- Address Editor: `seven_segment_axi_0/s_axi_CTRL` → **`0x4000_0000 – 0x4000_FFFF`**.
- The HLS register offsets are added to this base:

<div class="flow center">digit register  =  0x4000_0000  +  0x10  =  0x4000_0010</div>

- Then **Tools → Validate Design** (F6). Fix the first critical warning before anything else.

---

## Vivado: build and export the hardware

<div class="shot-row">
<div class="shot">
<b>img/15a-design-runs.png</b>
Design Runs tab:
synthesis and implementation
both complete
</div>
<div class="shot">
<b>img/15b-export-hardware.png</b>
Export Hardware wizard with
<b>Include bitstream</b> selected
</div>
</div>

- Right-click the block design → **Create HDL Wrapper** (let Vivado manage it).
- **Run Synthesis → Run Implementation → Generate Bitstream**, defaults throughout.
- **File → Export → Export Hardware**, *Include bitstream*, producing `seven_segment_wrapper.xsa`.
- Synthesis = netlist. Implementation = place & route + timing. Bitstream = the FPGA configuration.

---

## Vitis: platform and application project

<div class="shot-row">
<div class="shot">
<b>img/16a-platform-project.png</b>
Create Platform Project from
the exported XSA
</div>
<div class="shot">
<b>img/16b-application-project.png</b>
New Application Project:
processor <code>ps7_cortexa9_0</code>,
Empty Application
</div>
</div>

- `source .../Vitis/2022.2/settings64.sh && vitis`, workspace `~/vitis/lab2_task2`.
- **Platform project** `seven_segment_platform` from `seven_segment_wrapper.xsa`, then **Build Project**.
- **Application project** `seven_segment_app`, processor `ps7_cortexa9_0`, domain `standalone`, Empty Application.
- The XSA is what tells Vitis which processor exists and **where the IP is mapped** — hence platform first.

---

## Vitis: what your application must do

```c
#include "xparameters.h"
#include "xil_io.h"
#include "sleep.h"

#define SEVEN_SEG_BASE   XPAR_SEVEN_SEGMENT_AXI_0_S_AXI_CTRL_BASEADDR
#define DIGIT_OFFSET     0x10          /* from the HLS register map */

int main(void) {
    while (1) {
        for (u32 d = 0; d < 10; d++) {
            Xil_Out32(SEVEN_SEG_BASE + DIGIT_OFFSET, d);
            usleep(500000);
        }
    }
}
```

- You write this file yourself (`seven_segment_app/src/seven_segment_app.c`).
- Use the **generated** base-address macro from `xparameters.h` — never a hardcoded address.

---

## Run on the board

<div class="shot">
<b>img/18-launch-on-hardware.png</b>
Right-click app → Run As →
Launch on Hardware, plus the
console output
</div>

- **Build Project** → `seven_segment_app/Debug/seven_segment_app.elf`.
- **Run As → Launch on Hardware**: Vitis programs the bitstream over JTAG, then starts the ELF.
- **Expected behaviour:** the display cycles 0, 1, 2 … 9 with a visible delay.

<div class="flow">[ok] AXI4-Lite HLS IP exported          [ok] platform + application built
[ok] block design validated             [ok] ELF launched on ps7_cortexa9_0
[ok] bitstream and XSA generated        [ok] display driven from software</div>

<div class="note"><b>Wrong digit on the display?</b> Check the active-low inversion and the <code>seg</code>/<code>an</code> pin constraints — not the software.</div>
