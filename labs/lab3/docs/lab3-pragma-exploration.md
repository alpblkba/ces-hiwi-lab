---
marp: true
theme: ces-kit
paginate: true
size: 16:9
lang: en
footer: "Lab 3 — HLS pragma exploration"
title: "Lab 3: Pragma design space exploration on a DNN kernel"
description: "KIT CES — Customized Embedded Processors Lab, Lab 3 (baseline + PIPELINE / UNROLL / ARRAY_PARTITION exploration)"
headingDivider: 0
---

<!--
================================================================================
 Lab 3 deck — KIT / Chair of Embedded Systems (CES)
 Source material: github.com/alpblkba/ces-hiwi-lab (labs/lab3)

 All figures in this deck are real Vitis HLS 2022.2 C-synthesis output for
 xc7z007sclg400-1 at 10 ns. They are not illustrative numbers.

 BUILD
   marp lab3-pragma-exploration.md --theme theme/ces-kit.css -o lab3.pdf
   python3 scripts/md2pptx_ces.py labs/lab3/docs/lab3-pragma-exploration.md
================================================================================
-->

<!-- _class: title -->
<!-- _paginate: false -->

# Lab 3: Pragma design space exploration

## The same C++, from 21 025 cycles down to 518

Customized Embedded Processors Lab &nbsp;·&nbsp; Chair of Embedded Systems (CES)

Vitis HLS 2022.2 &nbsp;·&nbsp; Blackboard Zynq-7000 (`xc7z007sclg400-1`)

---

## What is different about this lab

- Labs 1 and 2 described hardware that was **small and control-shaped**. The C++ was more or less the circuit.
- Lab 3 uses a **dense neural-network layer**: a regular nest of multiply-accumulate loops.
- Nothing in the source says how much hardware to build. **The pragmas decide that.**
- One source, many architectures — from tiny and slow to large and fast.

<div class="flow center">16 x 16 x 16  =  4096 multiply-accumulate operations
one C++ function  →  many possible circuits</div>

---

## The kernel

```cpp
void matmul(const data_t A[N][N], const data_t B[N][N], acc_t C[N][N]) {
row:
    for (int i = 0; i < N; i++) {
    col:
        for (int j = 0; j < N; j++) {
            acc_t acc = 0;
        prod:
            for (int k = 0; k < N; k++) {
                acc += (acc_t)A[i][k] * (acc_t)B[k][j];
            }
            C[i][j] = acc;
        }
    }
}
```

- `N = 16`, fixed at compile time — a runtime size would make the report say `?` instead of a latency.
- Loops are **labelled** so a pragma can target one specific loop.

---

## Two decisions made before the exploration starts

**Size is a compile-time constant.** With a runtime `N`, Vitis HLS cannot compute
  a trip count and reports latency as `?`. A lab about comparing latency needs a number.

**Operands are 8-bit, accumulator 32-bit** — quantised-DNN practice, but here it is
  closer to a requirement:

| data type | DSP at baseline | share of the 66 available |
|-----------|-----------------|---------------------------|
| `int` (32-bit) | 42 | 63 % |
| `ap_int<8>` → `ap_int<32>` | 8 | 12 % |

<div class="note">Both give <b>identical latency</b>. The data type does not change how many operations there are, only what each one costs. Starting from 32-bit would leave no DSP budget to explore with.</div>

---

## The trap: the tool optimises before you do

Synthesizing the kernel with **no pragmas at all** still produces this:

<div class="flow">INFO: [XFORM 203-510] Pipelining loop 'col' ... automatically.
INFO: [HLS 200-489] Unrolling loop 'prod' ... completely with a factor of 16.</div>

| baseline | latency | DSP | LUT |
|----------|---------|-----|-----|
| tool defaults | 2 057 | 8 | 1 264 |
| automatic pipelining disabled | **21 025** | 1 | 198 |

<div class="warn">Measured against 2 057, your pragmas look useless or harmful — you would not be adding optimisation, you would be <b>overriding the tool's own</b>. Every result in this lab uses <code>config_compile -pipeline_loops 0</code>.</div>

---

## Task 3.1 — the baseline

<div class="flow center">latency  21 025 cycles  (0.21 ms @ 100 MHz)
DSP 1   ·   FF 75   ·   LUT 198   ·   BRAM 0</div>

- 4096 multiply-accumulate operations, executed essentially one at a time.
- Uses **1 %** of the device. There is a great deal of room to trade.
- This single number is the reference for everything in Task 3.2.

<div class="note"><b>Worth asking.</b> The kernel performs 4096 multiplications but the design contains one multiplier. Where do the other 4095 happen?</div>

---

## Task 3.2 — results

| # | configuration | latency | speed-up | DSP | LUT |
|---|---------------|---------|----------|-----|-----|
| V00 | baseline | 21 025 | 1.0× | 1 | 198 |
| V01 | `PIPELINE` on `prod` | 6 401 | 3.3× | 1 | 285 |
| V02 | `PIPELINE` on `col` | 2 057 | 10.2× | 8 | 1 260 |
| V05 | `UNROLL 8`, no partition | 4 641 | 4.5× | 4 | 645 |
| V06 | `UNROLL 16`, no partition | 3 233 | 6.5× | 8 | 1 104 |
| V07 | `ARRAY_PARTITION` only | **21 025** | **1.0×** | 1 | 222 |
| V11 | `UNROLL 16` + `PARTITION complete` | 1 585 | 13.3× | 8 | 639 |
| V12 | `PIPELINE` + `UNROLL 4` + `PARTITION 4` | **518** | **40.6×** | 8 | 1 037 |

---

## ARRAY_PARTITION on its own does nothing

<div class="flow center">baseline                 21 025 cycles
ARRAY_PARTITION only     21 025 cycles</div>

- Identical **to the cycle**. The pragma was applied; it simply had no one to serve.
- Splitting a memory into several smaller ones only helps if something wants to
  read them **at the same time**.
- Nothing in the baseline does, so the extra ports sit unused.

<div class="note">A pragma that changes the generated hardware and changes the performance not at all. Worth remembering before adding directives on faith.</div>

---

## PIPELINE: where you put it matters more than whether you use it

| pragma placement | latency | speed-up |
|------------------|---------|----------|
| on `prod`, the innermost loop | 6 401 | 3.3× |
| on `col`, one level up | 2 057 | **10.2×** |

- Pipelining a loop forces everything **inside** it to unroll.
- On `col`, that means the entire 16-step inner product becomes parallel hardware.
- Same pragma, one line higher in the file, three times the effect.

---

## UNROLL scales, but sub-linearly

| factor | latency | speed-up | DSP |
|--------|---------|----------|-----|
| 1 (baseline) | 21 025 | 1.0× | 1 |
| 2 | 10 785 | 1.9× | 1 |
| 4 | 5 665 | 3.7× | 2 |
| 8 | 4 641 | 4.5× | 4 |
| 16 | 3 233 | 6.5× | 8 |

- 16× the arithmetic buys **6.5×** the speed.
- The missing factor is the cost of getting operands in and out of memory.

---

## When partitioning actually starts to pay

| unroll factor | unroll alone | + matching partition | gain |
|---------------|--------------|----------------------|------|
| 2 | 10 785 | 10 785 | none |
| 4 | 5 665 | 5 665 | none |
| 8 | 4 641 | 3 617 | 1.28× |
| 16 | 3 233 | 1 585 | **2.04×** |

- At low factors the partition changes **nothing** — a block RAM already has two ports.
- It only pays once the unrolled loop asks for more concurrent reads than the memory can deliver.

<div class="note">More nuanced than "always partition what you unroll". Below the port limit, the pragma is free of benefit as well as nearly free of cost.</div>

---

## Which dimension to partition

```cpp
#pragma HLS ARRAY_PARTITION variable=A dim=2 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=B dim=1 cyclic factor=4
```

- The unrolled loop variable is `k`.
- `k` indexes `A[i][k]` — the **second** dimension — and `B[k][j]` — the **first**.
- Those are the dimensions that must be split so several `k` values can be read at once.
- `C` is not indexed by `k`. Partitioning it achieves nothing.

<div class="note"><b>Rule of thumb.</b> Partition the dimension the unrolled index walks along, and match the factor to the unroll factor.</div>

---

## The resource cliff

| configuration | latency | DSP | fits on the board? |
|---------------|---------|-----|--------------------|
| full unroll, auto-pipelining left on | 143 | **128** | **no** — 194 % of 66 |
| `UNROLL 16` + `PARTITION complete` | 1 585 | 8 | yes |
| V12, recommended | 518 | 8 | yes, 12 % |

- The fastest figure Vitis HLS will report is **not implementable** on this device.
- C synthesis does not stop you — the design fails much later, in Vivado.
- 143 versus 518 cycles is 3.6× bought with 16× the DSPs, on a board that has not got them.

<div class="warn">Always check the estimate against the device budget. The report gives you a number, not a verdict.</div>

---

## Summary

<div class="flow">[ok] baseline established with the tool's automatic optimisations disabled
[ok] each pragma measured in isolation, then in combination
[ok] ARRAY_PARTITION alone      -> no effect whatsoever
[ok] PIPELINE placement         -> 3.3x on the inner loop, 10.2x one level up
[ok] UNROLL                     -> sub-linear, 16x hardware for 6.5x speed
[ok] UNROLL + PARTITION         -> pays only above the memory port limit
[ok] best implementable result  -> 518 cycles, 40.6x, 12% DSP, 7% LUT

[next] optional: DATAFLOW, ap_fixed types, larger N</div>

<div class="note"><b>Hand in</b> the table with your own numbers, one or two sentences per pragma on what changed in the hardware, and an argument for which configuration you would actually put on the board.</div>
