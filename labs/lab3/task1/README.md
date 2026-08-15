# Lab 3.1 — The unoptimised baseline

One dense (fully connected) neural-network layer:

```
y = ReLU(x·W + bias)
```

4×4, 8-bit operands, 32-bit accumulator. Sizes are compile-time constants, so
Vitis HLS can report a latency number instead of `?`.

## Files

```
src/dnn_task1.h      sizes and types
src/dnn_task1.cpp    the layer, no optimisation pragmas
src/dnn_task1_tb.cpp testbench with an independent golden model
```

The three loops are labelled `sample`, `neuron` and `prod`. Task 2 attaches
pragmas to those labels. Labels do not affect the hardware.

## Build

```bash
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh
cd labs/lab3/task1/hls
vitis_hls -f run_hls.tcl        # part xc7z007sclg400-1, 10 ns clock
```

C simulation must pass before any synthesis number means anything.

## The testbench

It checks behaviour, not synthesis, and it is worth reading before it is worth
running. Twelve cases, each aimed at something that actually breaks: the ReLU
boundary, the sign of an 8-bit operand, the width of the accumulator, whether
the two matrix indices got swapped, and whether one sample can see another's
data. A random input finds none of these reliably, which is why `random` is the
last case in the list rather than the only one.

Build it with an ordinary compiler first — it is the same code, and the errors
are far easier to read than the tool's:

```bash
g++ -O2 -std=c++11 -I $XILINX_HLS/include \
    ../src/dnn_task1.cpp ../src/dnn_task1_tb.cpp -o dnn_test
./dnn_test              # every case
./dnn_test list         # what the cases are
./dnn_test relu-edge index-order
./dnn_test random 42
```

The same words go through `csim_design -argv {relu-edge}` inside the tool.

## Baseline, after place and route

```
LUT      323 / 14400    2.24%
FF       313 / 28800    1.09%
BRAM       0 / 50       0.00%
DSP       16 / 66      24.24%
latency   11 cycles  (0.110 us)
```

These are the numbers every variant in Task 2 is compared against.

## Note on the data type

8-bit operands are not an arbitrary choice. A 32-bit multiply costs about four
DSP slices and this device has 66, so an `int` version of the same layer leaves
almost no room to explore anything in Task 2. Quantised inference works the same
way in practice.
