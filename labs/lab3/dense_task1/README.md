# Dense task 1 — baseline

One dense (fully connected) neural-network layer:

```
y = ReLU(x·W + bias)
```

4×4, 8-bit operands, 32-bit accumulator. Sizes are compile-time constants, so
Vitis HLS can report a latency number instead of `?`.

## Files

```
src/dense.h        sizes and types
src/dense.cpp      the layer, no optimisation pragmas
src/dense_tb.cpp   testbench with an independent golden model
```

The three loops are labelled `sample`, `neuron` and `prod`. Task 2 attaches
pragmas to those labels. Labels do not affect the hardware.

## Build

```bash
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh
vitis_hls -f run_hls.tcl        # part xc7z007sclg400-1, 10 ns clock
```

C simulation must pass before any synthesis number means anything. The
testbench checks every output against a plain-C reference. With the default
seed, 6 of the 16 outputs are clamped by ReLU, so the activation is exercised
rather than being dead code.

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
