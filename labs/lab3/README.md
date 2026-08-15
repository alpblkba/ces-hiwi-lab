# Lab 3 — HLS pragma design space exploration on a DNN kernel

## Overview

Labs 1 and 2 used HLS to describe hardware that was **small and control-shaped**:
a display driver, a scan counter, a few arithmetic operations. The C++ was
essentially a description of the circuit you wanted, and the tool built it.

Lab 3 is the opposite case, and it is where HLS earns its place. The kernel is a
**dense (fully connected) layer** — the dominant operation in a neural network —
which is a regular nested loop over multiply-accumulate operations. Nothing about
the C++ says how much hardware to build. The same source can become a small slow
circuit or a large fast one, and **the pragmas decide which**.

The lab is about learning to read that trade-off from the synthesis report.

## Structure

| Task | Content |
|------|---------|
| **3.1** | Synthesize the kernel with no optimisation pragmas. Establish the baseline latency and resource cost. |
| **3.2** | Apply `PIPELINE`, `UNROLL` and `ARRAY_PARTITION` — first alone, then combined — and explain what each one did and why. |

Both tasks use **the same kernel**. Nothing changes between them except
the pragmas, so every number is directly comparable to the Task 3.1 baseline.

## Target

- Board: **Blackboard, Zynq-7000 `xc7z007sclg400-1`** (same as Labs 1 and 2)
- Clock: **10 ns (100 MHz)**
- Flow: Vitis HLS C synthesis, then Vivado implementation and one bitstream per
  pragma variant for the board half of Task 3.2

Task 3.1 stops at C synthesis, because the question there is how the
pragmas change the generated architecture and the synthesis report answers that
directly. The board half of Task 3.2 exists because the synthesis report is an **estimate**: on
this kernel it was out by roughly 2× on DSP and 3× on LUT against the
post-place-and-route figures, in opposite directions. Any claim about what fits
on the device has to come from implementation, and any claim about what the
circuit computes has to come from the board.

## The device budget, and why it dominates this lab

| Resource | Available on `xc7z007s` |
|----------|------------------------|
| LUT | 14 400 |
| FF | 28 800 |
| **DSP** | **66** |
| BRAM | 50 |

66 DSP slices is a small budget, and it is the limit you will hit first. A 32-bit
integer multiply costs roughly four DSP slices, so an `int` version of this kernel
consumes **63 % of the DSPs before a single optimisation pragma is applied**,
leaving no room to explore anything.

The kernel therefore uses **8-bit operands with a 32-bit accumulator**, which is
also what quantised neural networks do in practice. The same design then uses
about **12 %** of the DSPs at baseline, and the whole exploration fits.

This is worth understanding before starting: the data type was not chosen for
accuracy reasons. It was chosen because it is what makes the lab possible on this
device.

## What you should be able to explain afterwards

- Why `UNROLL` on its own gives almost nothing, and what has to change for it to help.
- What `ARRAY_PARTITION` actually does to the generated memory.
- The difference between latency and initiation interval, and which one `PIPELINE` targets.
- Where the resource cliff is on this device, and how you would find it on another.

## Directory layout

```text
labs/lab3/
├── README.md          this file
├── docs/              slides
├── setup/             shared setup notes
├── task1/
│   ├── README.md      baseline task
│   ├── commands.md    reproducible commands
│   ├── src/           dnn_task1.{h,cpp} + testbench
│   └── hls/           run_hls.tcl, synthesis report
├── task2/
│   ├── README.md      pragma exploration
│   ├── commands.md    reproducible commands
│   ├── src/           dnn_task2.{h,cpp} + testbench, the AXI wrapper and dnn_app.c
│   └── hls/           run_hls.tcl, run_hls_axi.tcl, summarise.py, reports
│       └── archive-16x16/   superseded 16x16 matmul measurements
└── board-validation/  instructor-only fpgatest harness, not student material
```

## Optional extensions

Not required, and not part of the assessed work. Listed because they are the
natural next questions:

- `DATAFLOW` to overlap the loading of operands with the computation.
- `ap_fixed` types instead of integers, and the accuracy/resource trade-off.
- A convolution layer instead of a dense layer, reusing the same method.

## References

- [PIPELINE](https://docs.amd.com/r/en-US/ug1399-vitis-hls/pragma-HLS-pipeline)
- [UNROLL](https://docs.amd.com/r/en-US/ug1399-vitis-hls/pragma-HLS-unroll)
- [ARRAY_PARTITION](https://docs.amd.com/r/en-US/ug1399-vitis-hls/pragma-HLS-array_partition)
- [Vitis HLS Introductory Examples](https://github.com/Xilinx/Vitis-HLS-Introductory-Examples) (Apache 2.0) — the
  `Array/array_partition_complete` example is the conceptual starting point for this lab.
