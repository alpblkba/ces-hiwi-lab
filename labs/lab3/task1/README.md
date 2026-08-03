# Lab 3.1 — The unoptimised baseline

## Objective

Synthesize a small dense neural-network layer with **no optimisation pragmas**
and record what the tool produces: latency, initiation interval, and resource
use. This is the reference every variant in Task 3.2 is measured against.

There is nothing to make fast in this task. The goal is a trustworthy starting
point and the ability to read the synthesis report.

## The kernel

A dense (fully connected) layer is a matrix multiply. For `N = 16`:

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

That is 16 × 16 × 16 = **4096 multiply-accumulate operations**. The loops are
labelled `row`, `col` and `prod` so that Task 3.2 can attach a pragma to one
specific loop. Labels do not affect the hardware.

## Two decisions made before you start, and why

**The size is a compile-time constant.** If `N` were a function argument, Vitis
HLS could not know the trip count and would report the latency as `?`. Since the
entire point of Lab 3 is comparing latency between variants, the bound has to be
known at compile time.

**The operands are 8-bit, with a 32-bit accumulator.** This mirrors quantised
DNN practice, but on this board it is closer to a requirement than a choice. A
32-bit multiply costs about four DSP slices, and the device has 66:

| kernel data type | DSP used at baseline | share of device |
|------------------|---------------------|-----------------|
| `int` (32-bit) | 42 | 63 % |
| `ap_int<8>` operands, `ap_int<32>` accumulator | 8 | 12 % |

Both give **identical latency**, because the data type does not change how many
operations there are. It only changes what each one costs. Starting from the
32-bit version would leave no DSP budget to explore anything in Task 3.2.

## Files

| File | Purpose |
|------|---------|
| `src/matmul.h` | sizes and data types |
| `src/matmul.cpp` | the kernel, no optimisation pragmas |
| `src/matmul_tb.cpp` | testbench with an independent golden model |

The testbench computes the same product in plain `int` C, with no HLS types, and
compares element by element. Inputs are deterministic pseudo-random values over
the full signed 8-bit range, so sign handling is exercised and any failure is
reproducible.

## Workflow

1. Create a Vitis HLS project with `matmul` as the top function.
2. Part `xc7z007sclg400-1`, clock period **10 ns**.
3. Run **C simulation** — it must pass before any synthesis figure is meaningful.
4. Run **C synthesis**.
5. Record from the report: latency, interval, BRAM, DSP, FF, LUT.

See `commands.md` for the exact commands.

## Reading the report

Four numbers matter, and they are the same four you will compare in Task 3.2:

| Figure | Meaning |
|--------|---------|
| **Latency** | cycles from start to finish for one complete call |
| **Interval (II)** | cycles before the next call can start |
| **DSP** | hardware multipliers used — the scarce resource here |
| **LUT / FF / BRAM** | logic, registers, and memory |

Also look at the **Loop** section further down the report. It lists each labelled
loop with its own trip count, latency and interval. That table is where the
effect of a pragma becomes visible, well before the totals change.

## Expected result

- C simulation passes.
- A synthesis report with a determinate latency (a number, not `?`).
- Baseline figures recorded and ready to compare against.

## Deliverables

- The three source files.
- The baseline synthesis report.
- A short answer to: *the kernel performs 4096 multiply-accumulate operations,
  yet the design uses only a handful of DSP slices. Where do the other
  multiplications happen?*
