# Lab 3.2 — Pragma design space exploration

## Objective

Take the Task 3.1 kernel — unchanged — and apply `PIPELINE`, `UNROLL` and
`ARRAY_PARTITION`, first one at a time and then in combination. For each
variant, record latency, initiation interval and resource use, and explain
**why** the numbers moved.

The deliverable is the table and the reasoning, not a single fastest
configuration.

## Before you measure anything: the tool is already optimising

Vitis HLS applies optimisations even when the source contains no pragmas at all.
Synthesizing the Task 3.1 kernel with default settings produces this in the log:

```text
INFO: [XFORM 203-510] Pipelining loop 'col' ... automatically.
INFO: [HLS 200-489] Unrolling loop 'prod' ... completely with a factor of 16.
```

The effect is large:

| baseline | latency | DSP | LUT |
|----------|---------|-----|-----|
| tool defaults, no pragmas | 2 057 | 8 | 1 264 |
| automatic pipelining disabled | **21 025** | 1 | 198 |

The tool gives a **10× speedup for free**. If you measure your pragmas against
the 2 057 figure you will conclude that they do nothing, or that they make the
design worse — because you would not be adding optimisation, you would be
overriding the tool's own and usually doing a worse job of it.

Every number below therefore uses:

```tcl
config_compile -pipeline_loops 0
```

so that each pragma's own contribution is visible. This is the single most
important methodological point in the lab.

## Results

All figures are real Vitis HLS 2022.2 C-synthesis output, `xc7z007sclg400-1`,
10 ns clock, 16×16 int8 matmul. Speed-up is relative to the 21 025-cycle
baseline.

| # | configuration | latency | speed-up | DSP | FF | LUT |
|---|---------------|---------|----------|-----|----|-----|
| V00 | baseline, no pragmas | 21 025 | 1.0× | 1 | 75 | 198 |
| V01 | `PIPELINE` on `prod` | 6 401 | 3.3× | 1 | 145 | 285 |
| V02 | `PIPELINE` on `col` | 2 057 | 10.2× | 8 | 373 | 1 260 |
| V03 | `UNROLL 2`, no partition | 10 785 | 1.9× | 1 | 99 | 288 |
| V04 | `UNROLL 4`, no partition | 5 665 | 3.7× | 2 | 131 | 405 |
| V05 | `UNROLL 8`, no partition | 4 641 | 4.5× | 4 | 171 | 645 |
| V06 | `UNROLL 16`, no partition | 3 233 | 6.5× | 8 | 688 | 1 104 |
| V07 | `ARRAY_PARTITION 4` only | **21 025** | **1.0×** | 1 | 81 | 222 |
| V08 | `UNROLL 2` + `PARTITION 2` | 10 785 | 1.9× | 1 | 95 | 240 |
| V09 | `UNROLL 4` + `PARTITION 4` | 5 665 | 3.7× | 2 | 111 | 306 |
| V10 | `UNROLL 8` + `PARTITION 8` | 3 617 | 5.8× | 4 | 158 | 443 |
| V11 | `UNROLL 16` + `PARTITION complete` | 1 585 | 13.3× | 8 | 463 | 639 |
| V12 | `PIPELINE col` + `UNROLL 4` + `PARTITION 4` | **518** | **40.6×** | 8 | 331 | 1 037 |

## What each pragma actually did

**`ARRAY_PARTITION` on its own does nothing.** V07 is 21 025 cycles — identical
to the baseline, to the cycle. Splitting a memory into several smaller memories
only helps if something is trying to read them at the same time. Nothing in the
baseline is, so the extra ports sit unused. This is the cleanest single result
in the lab: a pragma that changes the hardware and changes the performance not
at all.

**`PIPELINE` is the strongest single pragma, and where you put it matters.**
On the innermost loop (V01) it gives 3.3×. On the `col` loop one level up (V02)
it gives 10.2×, because pipelining a loop forces everything inside it to unroll,
so the whole inner product is built as parallel hardware. Same pragma, one line
higher, five times the effect.

**`UNROLL` scales, but sub-linearly.** Doubling the factor does not halve the
latency: 1.9× → 3.7× → 4.5× → 6.5× for factors 2, 4, 8, 16. Sixteen times the
arithmetic buys 6.5× the speed. The gap is the cost of getting operands in and
out.

**`UNROLL` + `ARRAY_PARTITION` only pays off at high factors.** This is more
subtle than the usual "always partition what you unroll" advice, and the data
says so plainly:

| factor | unroll alone | unroll + partition | gain |
|--------|--------------|--------------------|------|
| 2 | 10 785 | 10 785 | none |
| 4 | 5 665 | 5 665 | none |
| 8 | 4 641 | 3 617 | 1.28× |
| 16 | 3 233 | 1 585 | 2.04× |

At factors 2 and 4 the partition changes nothing at all — it only trims a few
LUTs. A block RAM already has two ports, and the scheduler can spread a small
number of accesses across cycles. Only once the unrolled loop demands more
concurrent reads than the memory can deliver does partitioning start to pay,
and by factor 16 it is worth a clean 2×.

**The combination beats any single pragma by a wide margin.** V12 combines a
pipelined `col` loop with a partially unrolled and matched-partition inner loop:
518 cycles, 40.6× the baseline, using 12 % of the DSPs and 7 % of the LUTs.

## The resource cliff

The cliff does not appear in the table above, because with automatic pipelining
disabled nothing exceeded 8 DSP. It appears when a full unroll is combined with
the tool's automatic pipelining:

| configuration | latency | DSP | fits on `xc7z007s`? |
|---------------|---------|-----|---------------------|
| V11 with auto-pipelining left on | 143 | **128** | **no** — 194 % of the 66 available |
| V06 with auto-pipelining left on | 269 | **128** | **no** |
| V12 (recommended) | 518 | 8 | yes, 12 % |

The fastest result the tool will happily report is not implementable on this
board. Vitis HLS does not stop you: C synthesis reports an estimate, and the
design only fails later, in Vivado. Checking the estimate against the device
budget is part of reading the report.

Note also that 143 cycles versus 518 is a 3.6× gain bought with 16× the DSPs, on
a device that does not have them. That is the shape of a bad trade.

## What to hand in

- The results table, with your own numbers.
- For each pragma, one or two sentences on what changed in the hardware.
- An answer to: *`ARRAY_PARTITION` on its own produced exactly the baseline
  latency. Was the pragma ignored? If not, what did it change?*
- An answer to: *which configuration would you actually put on the board, and
  why is it not the fastest one?*

## Reproducing

`src/sweep.py` generates every variant, synthesizes it, and writes the table.
See `commands.md`. Doing two or three variants by hand first is worthwhile — the
script is for the sweep, not for learning where the pragmas go.

## Optional extensions

- `DATAFLOW` to overlap operand loading with computation.
- `ap_fixed` instead of integers, and what that costs.
- Push `N` to 32 and find where the device runs out.
