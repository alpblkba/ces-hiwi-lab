# Lab 1.2: One-digit seven-segment display with HLS

## Objective

In this task, you will implement a simple seven-segment display decoder using C/C++ and Vitis HLS.

The design receives a single digit as input and produces the corresponding seven-segment output pattern. The mandatory part of the task is a one-digit display decoder. 
No handwritten HDL code is required for the decoder logic. The hardware block is generated from the HLS C/C++ implementation.

## Scope

In this task, you will:

1. write a C/C++ function for a one-digit seven-segment decoder,
2. synthesize the function with Vitis HLS,
3. verify the mapping using Vitis HLS simulation

## Background

High-Level Synthesis, or HLS, is a design flow where hardware is described using a higher-level programming language such as C or C++ instead of writing register-transfer level logic directly in Verilog or VHDL. The HLS tool analyzes the C/C++ function and generates hardware logic from it. This does not mean that the code runs as software on a processor. After synthesis, the function becomes a hardware block with input and output ports.

In this lab, HLS is used because it allows a small hardware function to be described in a familiar programming style while still producing synthesizable hardware. This is useful for learning the hardware design flow step by step. Students can first check the behavior of the function with a normal C/C++ testbench, then use Vitis HLS to generate RTL and synthesis reports. A seven-segment decoder is a suitable first HLS example because the function is small, deterministic, and easy to verify. The input is a digit, and the output is a fixed pattern of segment control bits. This makes it possible to focus on the HLS workflow without introducing unnecessary algorithmic complexity.

The important point is that the C/C++ function describes hardware behavior, not a general-purpose software program. Features such as dynamic memory allocation, file I/O, operating-system calls, or unbounded loops are not appropriate for this kind of simple hardware block. The function should have clear inputs, clear outputs, and a predictable mapping from input values to output bits.

In this task, we are going to implement a one-digit seven-segment decoder, optionally you can implement a two-digit version additionally. The design can be implemented as a lookup table or as a switch-case statement. Both approaches describe the same hardware idea: each valid digit selects one predefined segment pattern. For invalid input values, the design should still produce a defined output, such as turning all segments off. The segment pattern depends on the board convention. Some displays are active-high, where a segment turns on when its control bit is 1. Others are active-low, where a segment turns on when its control bit is 0. The implementation should therefore be written so that the selected convention is clear and easy to change if the board requires the opposite polarity.
The purpose of this task is not only to create a working decoder, but also to understand the basic HLS flow: write the C/C++ function, test it in C simulation, synthesize it, inspect the report, and prepare the generated hardware block for later integration in Vivado. This same workflow will be used again for larger hardware functions later in the lab.

A seven-segment display consists of seven individually controlled segments. By enabling different combinations of these segments, decimal digits can be displayed.

In this task, the HLS function maps an input digit to a segment pattern. For example, the input value 0 should enable the segments required to display the digit 0. The same idea applies to the digits 1 through 9.

The exact electrical convention of the board determines whether a segment is active-high or active-low. This means that a segment may turn on when its output bit is 1, or it may turn on when its output bit is 0. The lab setup or board documentation will specify which convention is used.

## Design description

The mandatory design is a one-digit decoder.

Input:

- one digit value
- valid range: 0 to 9

Output:

- seven segment control bits
- one bit per segment

The function should produce a valid segment pattern for each decimal digit from 0 to 9.

For input values outside the decimal range, the design should produce a defined output. A common choice is to turn all segments off.

## Recommended implementation approach

Start with a small C/C++ HLS function that maps one input digit to one seven-segment output pattern.

The implementation should be simple and deterministic:

- no dynamic memory allocation,
- no file I/O,
- no operating-system calls,
- no unbounded loops,
- no unnecessary global state.

A lookup table is usually the simplest implementation for this task. A switch-case implementation is also acceptable. In both cases, each valid decimal digit should select one predefined segment pattern.

For invalid input values, the function should still produce a defined output. A common choice is to turn all segments off.

## HLS workflow

The recommended workflow is:

1. create the C/C++ HLS source file,
2. select the decoder function as the HLS top function,
3. configure the target part or board as instructed,
4. run C synthesis,
5. inspect the synthesis report,
6. export the generated RTL or IP block.

If instructed, the digit-to-segment mapping can also be checked with Vitis HLS simulation before synthesis. This is useful for catching simple mapping mistakes early, but the main goal of this task is to generate a synthesizable HLS block for later Vivado integration.

## Optional extension: two-digit display

After the one-digit version works, the design can be extended to support two decimal digits.

A possible two-digit extension receives a value from 0 to 99 and produces two seven-segment patterns:

- one pattern for the tens digit,
- one pattern for the ones digit.

The two-digit extension is optional. Complete and synthesize the one-digit decoder first.

## Expected result

At the end of this task, you should have:

- a C/C++ HLS implementation of a one-digit seven-segment decoder,
- a selected HLS top function,
- a successful Vitis HLS synthesis run,
- generated reports for the synthesized design,
- an exported HLS IP or RTL output for later Vivado integration.

If simulation is used, the digit-to-segment mapping should match the expected display behavior before synthesis.

## Notes

Keep the HLS function small and easy to inspect. The purpose of this task is to understand the basic HLS flow, not to build a complex display driver.

Do not write the decoder directly in Verilog. The decoder logic should come from the C/C++ HLS implementation.

Do not continue to the Vivado integration step until the HLS synthesis result is available and the generated reports can be inspected.
