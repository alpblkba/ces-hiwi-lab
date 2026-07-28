---
marp: true
theme: ces-kit
paginate: true
size: 16:9
lang: en
footer: "Lab 1 — Vitis HLS bring-up"
title: "Lab 1: Vitis HLS bring-up and seven-segment decoder"
description: "KIT CES — Customized Embedded Processors Lab, Lab 1 (Lab 1.1 setup + Lab 1.2 seven-segment HLS decoder)"
headingDivider: 0
---

<!--
================================================================================
 Lab 1 deck — KIT / Chair of Embedded Systems (CES)
 Source material: github.com/alpblkba/ces-hiwi-lab  (labs/lab1/task1, labs/lab1/task2)
 Reference style: session1.pptx  (title + 3-4 short bullets + one large screenshot)
 Corporate design: CES_ppt_template-1.pptx (Arial, KIT green #009682, 10 x 5.63 in)

 STRUCTURE
   Section A — initial setup .............. 8 slides
   Section B — seven-segment HLS decoder .. 9 slides
   -> 17 content slides + 1 title + 2 section dividers = 20 rendered slides

 SCREENSHOT CONVENTION
   Every <div class="shot"> is a reserved slot. Once the real screenshot
   exists, replace the whole div with a Marp image directive, e.g.
       ![w:620](img/03-repo-tree.png)
   Keep the file name printed inside the box (NN-short-name.png). The NN
   prefix is only a sort key, it does not have to match the final slide
   number after edits.

 ALL 14 REQUESTED PLACEHOLDERS, and where they live
   repository tree ................ Repository structure
   Xilinx 2022.2 version check .... Remote machine and toolchain check
   Vitis HLS Welcome/Create ....... Starting Vitis HLS 2022.2
   project name and location ...... New project: name, location, sources
   adding design source ........... New project: name, location, sources
   adding testbench source ........ New project: name, location, sources
   selecting top function ......... Solution settings
   FPGA part + 10 ns clock ........ Solution settings
   Vivado 2022.2 opened ........... Starting Vivado 2022.2
   source code in Vitis HLS ....... Source files in the HLS project
   C simulation OK for input 42 ... Running C simulation
   C synthesis OK ................. C synthesis and the synthesis report
   Synthesis Summary .............. C synthesis and the synthesis report
   Export RTL/IP OK ............... Expected result and IP export

 BUILD
   marp lab1-hls-bringup.md --theme theme/ces-kit.css -o lab1.pdf
   python3 md2pptx.py lab1-hls-bringup.md --template CES_ppt_template-1.pptx -o lab1.pptx
================================================================================
-->

<!-- _class: title -->
<!-- _paginate: false -->

# Lab 1: Vitis HLS bring-up

## Seven-segment decoder as a first HLS design

Customized Embedded Processors Lab &nbsp;·&nbsp; Chair of Embedded Systems (CES)

Vitis HLS 2022.2 &nbsp;·&nbsp; Vivado 2022.2 &nbsp;·&nbsp; Blackboard Zynq-7000 (`xc7z007sclg400-1`)

---

<!-- _class: part -->

# Section A — Initial setup

Access the machine, load the toolchain, create the first HLS project.

---

## Lab objective and tool flow

- **Goal of Lab 1.1:** get the Xilinx 2022.2 toolchain running and create a working Vitis HLS project.
- **Goal of Lab 1.2:** describe a small hardware function in C/C++ and let Vitis HLS generate the RTL.
- No VHDL and no Verilog is written in this lab. The decoder logic comes from C/C++.
- Vivado is only *opened and verified* in Lab 1 — it is used for real in Lab 2.

<div class="flow center">C/C++ function  →  C simulation  →  C synthesis  →  RTL + reports  →  exported IP
                                                                        │
                                                     Lab 2:  Vivado block design  →  bitstream</div>

<div class="note"><b>Tool roles.</b> Vitis HLS turns C/C++ into an RTL IP block. Vivado builds the system around that IP and produces the bitstream. Vitis builds the software that runs on the ARM core.</div>

---

## Repository structure

<div class="shot">
<b>img/03-repo-tree.png</b>
Terminal output of <code>tree -L 3 labs/</code> or
<code>find labs -maxdepth 3 -type f | sort</code>
<small>full window, monospace font readable at 100 %</small>
</div>

- One directory per task, under `labs/lab1/` and `labs/lab2/`.
- Each task ships a `README.md`, a `commands.md` and a `questions.md`.
- Create your Vitis HLS / Vivado projects **outside** the tool installation directory, in your own writable working directory.
- Keep every lab artefact under one tree — incomplete submissions are usually a directory problem, not a design problem.

---

## Remote machine and toolchain check

<div class="shot">
<b>img/04-version-check.png</b>
Terminal after login and sourcing:
<code>hostname</code>, <code>pwd</code>, and
<code>vitis_hls -version</code> showing <b>2022.2</b>
<small>the version string must be readable in the screenshot</small>
</div>

```bash
ssh -Y <u-identifier>@<host>.kit.edu
hostname && pwd && cd <lab-working-directory>

XIL=/Software/xilinx/2022.2
source $XIL/Vitis_HLS/2022.2/settings64.sh
source $XIL/Vivado/2022.2/settings64.sh
vitis_hls -version | head -n 5
```

- `-Y` enables trusted X11 forwarding, so the Xilinx GUIs can open on your machine.
- `settings64.sh` must be **sourced**, not executed — it sets `PATH`, `XILINX_VIVADO` and `XILINX_HLS` in the current shell.
- All tools must report the same version, **2022.2**.
- `command not found` after sourcing = wrong path or wrong machine, not a broken installation.

---

## Starting Vitis HLS 2022.2

<div class="shot">
<b>img/06-hls-welcome.png</b>
Vitis HLS 2022.2 Welcome page with the
<b>Create Project</b> tile highlighted
<small>full application window</small>
</div>

```bash
vitis_hls &
```

- The `&` keeps the shell usable while the GUI runs.
- The Welcome page confirms both the GUI and X11 forwarding work.
- If the window never appears: check `echo $DISPLAY` and `xdpyinfo`.
- Choose **Create Project** — do not open an existing project yet.

---

## New project: name, location, sources

<div class="shot-row">
<div class="shot">
<b>img/07a-project-name.png</b>
Project Configuration page:
<code>lab1_task2_hls</code> +
project location
</div>
<div class="shot">
<b>img/07b-add-sources.png</b>
Add/Remove Design Files and
Add/Remove Testbench Files pages
</div>
</div>

- Project name `lab1_task2_hls`; location = your lab working directory. **No spaces in the path.**
- Design files: `seven_segment.cpp`, `seven_segment.h`
- Testbench files: `seven_segment_tb.cpp` — added on the *separate* testbench page, not with the design files.
- Vitis HLS copies nothing: the files stay where they are and are referenced by path.

---

## Solution settings: top function, part, clock

<div class="shot-row">
<div class="shot">
<b>img/08a-select-top.png</b>
Top Function browser with
<code>seven_segment</code> selected
</div>
<div class="shot">
<b>img/08b-part-clock.png</b>
Solution Configuration:
part <code>xc7z007sclg400-1</code>,
clock period <b>10 ns</b>
</div>
</div>

- **Top function** = the C/C++ function that becomes the hardware block. Everything it calls is synthesized with it.
- Solution name `solution1`, **Flow target: Vivado IP Flow Target**.
- Clock period `10 ns` (100 MHz) and part `xc7z007sclg400-1` — the Blackboard Zynq-7000 device.
- Wrong part or wrong clock here means the reports and the exported IP are useless later.

---

## Starting Vivado 2022.2

<div class="shot">
<b>img/09-vivado-open.png</b>
Vivado 2022.2 Quick Start window,
version visible in the title bar
<small>full application window</small>
</div>

```bash
vivado &
```

- In Lab 1 Vivado is only opened to prove the installation and the GUI work.
- Confirm the title bar reads **2022.2** — mixed versions break IP hand-over between HLS and Vivado.
- Close it again; no Vivado project is created in Lab 1.

<div class="note"><b>Lab 2 preview.</b> The IP you export at the end of Lab 1 is imported into a Vivado block design in Lab 2.1.</div>

---

## Setup verification

<div class="flow">[ok] logged in on the assigned lab machine
[ok] lab working directory exists and is writable
[ok] repository checked out, task directories visible
[ok] Xilinx 2022.2 environment sourced
[ok] vitis_hls -version and vivado -version both report 2022.2
[ok] X11 forwarding works, both GUIs open
[ok] Vitis HLS project created with top function and part configured

[next] Lab 1.2 — implement the seven-segment decoder</div>

- If any line above fails, fix it before writing code. Do not debug HLS on a broken environment.
- Record machine name, tool version and working directory in your notes — the first question in the oral check.

---

<!-- _class: part -->

# Section B — Lab 1.2: Seven-segment decoder

From a C/C++ function to synthesized RTL.

---

## Task 1: Seven-segment decoder using HLS

- Implement a **two-digit decimal display decoder** in C/C++.
- Convert an input value from **0 to 99** into two seven-segment patterns.
- **Verify** the design using C simulation.
- **Synthesize** the function into RTL using Vitis HLS.

<div class="flow center">Input value (0 … 99)
       ↓
C/C++ HLS function  seven_segment()
       ↓
Tens digit  and  ones digit
       ↓
Two seven-segment patterns</div>

---

## Why start with a seven-segment decoder?

- The input/output behaviour is **simple and deterministic** — one input value, one fixed output pattern.
- The expected result can be **verified visually**, on the report and later on the board.
- The task introduces the **complete HLS workflow** without any algorithmic complexity.
- The **same workflow** is reused later for larger accelerator designs.

<div class="note"><b>The point of the task is not the display.</b> The seven-segment decoder is the smallest example that still exercises every step of the HLS flow: source, testbench, C simulation, C synthesis, report, IP export.</div>

---

## Seven-segment display concept

<div class="shot">
<b>img/13-seven-segment-digits.png</b>
Segment naming diagram (A…G)
plus the digit patterns 0-9
<small>reuse the figure from the earlier session deck</small>
</div>

- Seven individually controlled LEDs, arranged as an “8”; each digit is one combination of segments.
- Segment order used in this lab: **A B C D E F G**, A = most significant bit.
- The Blackboard display is **active-low**: a segment lights up when its bit is `0`.
- The decoder therefore computes an active-high pattern and inverts it once, at the output.

---

## Input / output specification

| Port | Type | Direction | Meaning |
|------|------|-----------|---------|
| `value` | `ap_uint<7>` | in | decimal value, valid range 0 … 99 |
| `seg_tens` | `ap_uint<8>` | out | tens-digit pattern, `[6:0]` = G…A, `[7]` = DP |
| `seg_ones` | `ap_uint<8>` | out | ones-digit pattern, same encoding |

- `tens = value / 10`, `ones = value % 10` — both bounded, so HLS synthesizes plain combinational logic.
- Undefined input (`value > 99`): produce a **defined** output — all segments off.
- `ap_uint<N>` from `<ap_int.h>` gives exact bit widths; `int` would waste 32 bits of hardware.

<div class="note"><b>Two digits are now the standard.</b> Three or four digits are an optional extension. Physical digit multiplexing (the <code>an[3:0]</code> anode select) is added when the IP is integrated in Vivado.</div>

---

## Where does the user input come from?

<div class="warn"><b>A synthesizable HLS function contains no <code>std::cin</code>, no <code>scanf</code>, no terminal input and no file I/O.</b></div>

- User input is provided by the **testbench**, or later by **processor-side software** over AXI.
- The HLS function only converts the number arriving on its **input port** into segment patterns.
- The interactive part lives entirely on the *verification* side of the boundary.

<div class="flow">Interactive testbench          (plain C++, not synthesized)
        │
        │  value = 42
        ▼
seven_segment()               (synthesized into RTL)
        │
        ├──  seg_tens  =  pattern for 4
        └──  seg_ones  =  pattern for 2</div>

<div class="note">This distinction prevents the classic misconception: <i>“I wrote C++, so this is a software program.”</i></div>

---

## Source files in the HLS project

<div class="shot">
<b>img/16-source-in-hls.png</b>
Vitis HLS Explorer with Source /
Test Bench expanded, <code>seven_segment.cpp</code>
open in the editor
<small>Explorer tree and editor both visible</small>
</div>

- `seven_segment.h` — port declaration and `#include <ap_int.h>`
- `seven_segment.cpp` — the decoder: one `switch` over the digit, then one inversion for active-low
- `seven_segment_tb.cpp` — reads a value, calls the function, compares against expected patterns

<div class="note"><b>Explorer check.</b> Design files under <b>Source</b>, testbench under <b>Test Bench</b>. A testbench in the wrong group is synthesized and will fail.</div>

---

## Running C simulation

<div class="shot">
<b>img/17-csim-42.png</b>
Console after <b>Run C Simulation</b>,
input <b>42</b>, expected and actual
patterns printed, <code>C simulation ... PASSED</code>
<small>console pane, exit code 0 visible</small>
</div>

- C simulation compiles and runs the function as ordinary C++ — **no hardware is generated yet**.
- It answers one question only: *is the digit-to-pattern mapping correct?*
- Reference run: input `42` → tens pattern for `4`, ones pattern for `2`.
- Also check `0`, `9`, `99` and one invalid value such as `100`.

---

## C synthesis and the synthesis report

<div class="shot-row">
<div class="shot">
<b>img/18a-csynth-ok.png</b>
Flow Navigator after
<b>Run C Synthesis</b>,
“Finished C synthesis”
</div>
<div class="shot">
<b>img/18b-synthesis-summary.png</b>
Synthesis Summary:
timing, latency,
LUT / FF utilisation
</div>
</div>

- **Timing:** estimated clock must be below the 10 ns target, with positive slack.
- **Latency:** a pure decoder is combinational — expect 0 or 1 cycle.
- **Utilisation:** a handful of LUTs and FFs; **no BRAM, no DSP**. Anything else means the `switch` was not recognised as a lookup.
- **Interface summary:** confirm `value`, `seg_tens`, `seg_ones` appear as RTL ports with the expected widths.

---

## Expected result and IP export

<div class="shot">
<b>img/19-export-ip.png</b>
Export RTL dialog +
<code>solution1/impl/ip/component.xml</code>
in the file browser
<small>“Export RTL completed successfully”</small>
</div>

- **Export RTL / IP**, format **Vivado IP (.zip)** — this is the artefact Lab 2 consumes.
- Verify afterwards: `solution1/impl/export.zip` and `solution1/impl/ip/component.xml` exist.

<div class="flow">[ok] C simulation passed for 42 and the boundary cases
[ok] C synthesis finished, 10 ns timing met
[ok] report inspected: latency, resources, interface
[ok] IP exported

[next] Lab 2.1 — import this IP into a Vivado block design</div>
