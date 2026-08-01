# Lab 1.1: Vitis HLS bring-up

## Objective

Get the Xilinx 2022.2 toolchain running and complete one full Vitis HLS flow on a
minimal C++ function. The point of this task is the **tool flow**, not the design.

The seven-segment display comes in Lab 1.2. Here you only need a function small
enough that you can predict its output by hand.

## Scope

1. Verify the toolchain and open Vitis HLS.
2. Create a project with a top function and a testbench.
3. Run C simulation, C synthesis, and inspect the report.
4. Export the result as an IP block.

## Background

High-Level Synthesis takes a C or C++ function and generates hardware from it.
This does **not** mean the code runs as software on a processor. After synthesis
the function has become a circuit with input and output wires.

Two consequences follow, and they matter for every later task:

- The synthesizable function cannot use `new`, `malloc`, file I/O, operating
  system calls, or loops whose trip count is unknown at compile time.
- Anything interactive (reading a number from a terminal) belongs either in the
  **testbench**, which is ordinary C++ and is never synthesized, or in
  **processor software**, which arrives in Lab 2.2.

## Suggested design

Any small, deterministic function works. For example:

- two small integers in, their sum out, or
- a 4-bit input with a fixed combinational mapping out.

Use `ap_uint<N>` from `<ap_int.h>` rather than `int`. An `int` costs 32 bits of
hardware even when the value only ever needs 4.

## Workflow

1. Load the Xilinx environment and start `vitis_hls`.
2. Create a project; add the design file and, on the **separate** page, the testbench.
3. Select the top function; set part `xc7z007sclg400-1` and a 20 ns clock (50 MHz).
4. Run **C simulation** — does the function compute the right values?
5. Run **C synthesis** — what hardware did the tool build?
6. Read the report: timing, latency, interval, resource usage, interface ports.
7. **Export RTL** as a Vivado IP.

## Reading the synthesis report

Four things are worth finding, because you will need all of them again later:

| Item | Question it answers |
|------|---------------------|
| Timing (estimated vs target) | does the circuit fit inside one clock period? |
| Latency | how many cycles from input to output? |
| **Interval (II)** | how often can the block accept new input? |
| Interface | which RTL ports were created, and how wide? |

## Expected result

- A Vitis HLS project that builds from a clean start.
- A passing C simulation.
- A synthesis report you can read and explain.
- An exported IP block.
- A short written answer to: *how is this C++ function turned into hardware?*

## Notes

C simulation proves the **logic** is right. It says nothing about timing,
resources, or whether the design will behave correctly on a board. Only the
synthesis report shows that, which is why step 6 is not optional.
