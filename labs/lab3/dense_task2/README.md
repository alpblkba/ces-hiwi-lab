# Dense task 2 — pragmas

Same layer as Task 1. Only pragmas are added; the arithmetic is untouched.

Three pragmas, applied alone and then together:

```cpp
#pragma HLS PIPELINE II=1          // on the neuron loop
#pragma HLS UNROLL                 // on the prod loop
#pragma HLS ARRAY_PARTITION ...    // on x (dim 2) and W (dim 1)
```

`src/dense.cpp` here has all three applied. The individual variants are built
by the board-validation design, which contains all five side by side.

## Results

Measured on hardware, `xc7z007sclg400-1` at 100 MHz. Resource figures come from
the post-place-and-route hierarchical report, not from the C-synthesis estimate.
Latency is the HLS figure — the fabric has no cycle counter.

**baseline**
```
LUT      323 / 14400    2.24%
FF       313 / 28800    1.09%
BRAM       0 / 50       0.00%
DSP       16 / 66      24.24%
latency   11 cycles
```

**PIPELINE**
```
LUT      104 / 14400    0.72%
FF        99 / 28800    0.34%
BRAM       0 / 50       0.00%
DSP        4 / 66       6.06%
latency   21 cycles
```

**UNROLL**
```
LUT      321 / 14400    2.23%
FF       313 / 28800    1.09%
BRAM       0 / 50       0.00%
DSP       16 / 66      24.24%
latency   11 cycles
```

**ARRAY_PARTITION alone**
```
LUT      384 / 14400    2.67%
FF       313 / 28800    1.09%
BRAM       0 / 50       0.00%
DSP       16 / 66      24.24%
latency   11 cycles
```

**all three**
```
LUT      104 / 14400    0.72%
FF        99 / 28800    0.34%
BRAM       0 / 50       0.00%
DSP        4 / 66       6.06%
latency   21 cycles
```

## What the numbers say

**ARRAY_PARTITION on its own costs area and buys nothing.** 384 LUT against the
baseline's 323, same latency, same DSP count. Splitting a memory only helps if
something reads it concurrently, and nothing here does. This is the most useful
result in the lab: a pragma can change the generated hardware and change the
performance not at all.

At this size partitioning never pays, in any combination. It starts to matter
once the unrolled loop asks for more concurrent accesses than the memory can
supply. See `matmul_task2/` for the 16×16 measurements, where partitioning is
worth 1.28× at unroll 8 and 2.04× at full unroll.

**UNROLL changes almost nothing** — 321 LUT against 323, identical latency.
The loop is four iterations long and Vitis HLS already unrolls it without being
asked.

**PIPELINE is the only pragma that moves anything, and it trades the opposite
way from what students expect.** Latency goes up (11 → 21 cycles) while
resources drop by roughly 4× (16 → 4 DSP). Pipelining lets the tool reuse one
multiply-accumulate across cycles instead of building four in parallel. Faster
throughput, more cycles per call, much smaller circuit.

**All three together is identical to PIPELINE alone.** The other two pragmas
contribute nothing on top.

## Board validation

All five variants sit in one bitstream, selected by a register. Every one of
them returns the same checksum for the same input:

```
seed 1     -> 0x00006328      seed 42   -> 0x00050AB8
seed 1000  -> 0x00003E6B      seed 7    -> 0x0000CC88
```

11 tests, all passing. Whole design: 2942 LUT (20.43%), 2844 FF (9.88%),
56 DSP (84.85%), WNS +8.263 ns.

That is the claim worth checking on silicon: pragmas change speed and area, and
must leave the result alone.
