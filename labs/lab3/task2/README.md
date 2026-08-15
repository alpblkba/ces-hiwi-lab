# Lab 3.2 — Pragma design space exploration

Same layer as Task 1. Only pragmas are added; the arithmetic is untouched.

Three pragmas, applied alone and then together:

```cpp
#pragma HLS PIPELINE II=1          // on the neuron loop
#pragma HLS UNROLL                 // on the prod loop
#pragma HLS ARRAY_PARTITION ...    // on x (dim 2) and W (dim 1)
```

All five variants come from **one** source file. `src/dnn_task2.cpp` guards
each pragma with `#if VARIANT == n`, and `hls/run_hls.tcl` builds every variant
by passing `-DVARIANT=n`:

```bash
cd labs/lab3/task2/hls
vitis_hls -f run_hls.tcl            # all five
vitis_hls -f run_hls.tcl -tclargs 2 # only variant 2
python3 summarise.py                # reports -> results.md
```

Editing the pragmas by hand works just as well, and doing two or three that way
before running the sweep is the part where the learning happens. The macro
exists so that five builds do not need five copies of the same arithmetic — and
so the claim "only the pragmas changed" is literally true.

| variant | pragmas |
|---------|---------|
| 0 | none — identical to Task 3.1 |
| 1 | `PIPELINE II=1` on `neuron` |
| 2 | `UNROLL` on `prod` |
| 3 | `ARRAY_PARTITION` on `x` and `W` |
| 4 | all three |

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
supply. See `hls/archive-16x16/` for the 16×16 measurements, where partitioning is
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

---

# On the board

Everything above is an estimate from a synthesis report. This half puts each
variant on the board and lets the hardware answer instead.

The claim being tested is the one this task rests on:

> `PIPELINE`, `UNROLL` and `ARRAY_PARTITION` change how fast a circuit is and
> how much of the device it uses. They must not change **what** it computes.

## Five bitstreams, not five registers

Each pragma variant is built as its **own** bitstream. Program one, run the
application, type a seed, note the checksum. Program the next, run the same
application unchanged, type the same seed. The checksum must not move.

This is deliberately more work than putting all five in one bitstream behind a
`variant` register, and it is worth it for two reasons:

- **The demonstration cannot be argued with.** One bitstream with a selector is
  one circuit behind a multiplexer. Five bitstreams are five physically
  different circuits.
- **The DSP budget comes back.** All five together used 56 of the 66 DSP
  slices, which left no room to push an unroll factor and find the resource
  cliff. Alone, each variant uses between 4 and 16.

The application reads a `variant_id` register the kernel reports from its own
compile-time constant, so it always names the bitstream that is actually
loaded rather than the one you meant to load.

## What the software does

```text
you type a seed at the terminal
        │
        ▼
C program on the ARM core
        │  writes seed, sets ap_start, polls ap_done, reads checksum
        ▼
AXI4-Lite registers
        │
        ▼
DNN kernel in the PL          y = ReLU(x·W + bias), 4×4, int8 in, int32 acc
```

and then — this is the part that matters — **the same program computes the
answer itself in plain C on the ARM and compares**. The golden model runs on
the board. There is no host tool in the loop, nothing to trust except the
terminal.

The operands are generated on chip from the seed, so only one register has to
be written. `dnn_app.c` reproduces that generator exactly; the two are a
contract, and both files say so.

## Menu

| | |
|---|---|
| 1 | self-test — the four reference seeds, hardware vs on-board model vs recorded history |
| 2 | run one seed you choose |
| 3 | show the operands and the expected output, then check the hardware against them |
| 4 | timing — measured cycles per layer evaluation |
| 5 | AXI alive check |

The self-test prints three columns, and all three have to agree. The hardware
column against the on-board model catches a broken circuit. Both of them
against the recorded column catches a *changed* golden model — the failure
mode where the software and the hardware drift together and agree on the wrong
answer.

| seed | checksum |
|------|----------|
| 1 | `0x00006328` |
| 42 | `0x00050AB8` |
| 1000 | `0x00003E6B` |
| 7 | `0x0000CC88` |

The four differ from each other, so a block that latched one answer and returns
it for ever cannot pass. Seed 1000 clamps several outputs, so ReLU and the bias
are exercised rather than being dead code.

## Timing

The fabric has no cycle counter, so the ARM's global timer measures instead.
The kernel takes a `reps` register and runs the layer that many times per call,
which pushes the AXI handshake down into the noise; with `reps` large the
figure converges on the loop latency.

This is what turns "pipelining raises latency from 11 to 21 cycles while
shrinking the circuit four-fold" from a line in a report into a number the
student watches change when they load a different bitstream.

## Files

| File | Purpose |
|------|---------|
| `src/dnn_kernel_axi.h` / `.cpp` | the AXI4-Lite wrapper, one variant per build |
| `src/dnn_kernel_axi_tb.cpp` | C simulation: checksums, `reps` behaviour, `variant_id` |
| `src/dnn_app.c` | the ARM application |
| `hls/run_hls_axi.tcl` | exports one AXI IP per variant |

## Workflow

```bash
# 1. five IPs
cd labs/lab3/task2/hls && vitis_hls -f run_hls.tcl

# 2. five bitstreams
for v in 0 1 2 3 4; do
    vivado -mode batch -source scripts/build_lab3_vivado.tcl -tclargs $v
done

# 3. one workspace, from the XSA of the variant you want to run first
xsct scripts/create_vitis_workspace.tcl \
     ~/vivado/lab3_dnn_bits/v0/dnn_system_wrapper_v0.xsa \
     labs/lab3/task2/src ~/vitis/lab3_ws dnn_app

# 4. program and run
xsct scripts/debug_init.tcl <bit> <ps7_init.tcl> ~/vitis/lab3_ws/dnn_app/Debug/dnn_app.elf
```

Step 3 is worth doing on the machine the board is attached to. The XSA travels
between machines; a Vitis workspace does not — it records the platform by
absolute path, and a copied one reports "Binary File not Found" while the ELF
is sitting exactly where it should be.

## Register map

Confirm against the generated `xdnn_kernel_axi_hw.h`. Vitis HLS assigns these
offsets from the argument order, so adding or reordering an argument moves
them — and a stale copy of this table is a genuinely expensive mistake.

| offset | register | |
|--------|----------|---|
| `0x00` | `ap_ctrl` | bit 0 `ap_start` (W), bit 1 `ap_done` (R, clear-on-read), bit 2 `ap_idle` |
| `0x10` | `seed` | 32-bit R/W |
| `0x18` | `reps` | 32-bit R/W |
| `0x20` | `checksum` | 32-bit R |
| `0x28` | `variant_id` | 32-bit R |

## Deliverables

- The self-test output for all five bitstreams, showing identical checksums.
- The post-implementation utilisation for each variant — **not** the
  C-synthesis estimate. On this kernel the estimate was out by roughly 2× on
  DSP and 3× on LUT, in opposite directions.
- The measured cycles per evaluation for at least the baseline and the
  pipelined variant, and one or two sentences on why the faster-throughput one
  has the higher latency.

## Instructor notes

`../board-validation/` holds the fpgatest configuration and two patches that
fix real bring-up failures found on this design (a stale MMU blocking register
access, and XSA extraction byproducts being counted as stale sources). That
harness is for validation, not for students.
