# Lab 2.2 — Driving the calculator from software

## Objective

Write a C program that runs on the ARM processor, reads two numbers and an
operation from a terminal, and sends them to the HLS calculator built in
Lab 2.1. The result appears on the seven-segment display.

This is where the lab becomes interactive. The HLS block never reads a keyboard —
it cannot. The processor does that and hands the values over through registers.

## The hardware/software boundary

```text
you type at a terminal
        │
        ▼
C program on the ARM core          (software, this task)
        │  Xil_Out32(...)
        ▼
AXI4-Lite registers
        │
        ▼
HLS calculator + display scan      (hardware, Lab 2.1)
        │
        ▼
seven-segment display
```

Everything above the registers is software you can debug with `printf`.
Everything below is a circuit that runs whether or not the processor is busy.

## What the software has to do

Three register writes. That is the entire hardware interaction:

```c
#define CALC_BASEADDR    XPAR_SEVEN_SEGMENT_AXI_0_S_AXI_CTRL_BASEADDR
#define OP1_REG_OFFSET    0x10
#define OP2_REG_OFFSET    0x18
#define OPSEL_REG_OFFSET  0x20

Xil_Out32(CALC_BASEADDR + OP1_REG_OFFSET,   op1);
Xil_Out32(CALC_BASEADDR + OP2_REG_OFFSET,   op2);
Xil_Out32(CALC_BASEADDR + OPSEL_REG_OFFSET, op_sel);
```

There is **no start and no polling**. Because the HLS block was built with
`ap_ctrl_none` it is free-running: it picks up the new values on its next clock
cycle and keeps refreshing the display on its own.

Always use the generated `XPAR_...` macro from `xparameters.h`. Never write a
literal address — the macro follows whatever address the block design assigned.

## Validate the input in software

The hardware registers are only 7 bits wide. Writing `100` into a 7-bit register
does not produce an error; it silently stores `100 & 0x7F`. Range-check the
operands before writing them, and reject an unknown operation.

Division by zero is handled in hardware (the display shows `----`), but telling
the user about it in the terminal is friendlier.

## Workflow

1. Start Vitis with a workspace such as `~/vitis/lab2_task2`.
2. Create a **platform project** from the `seven_segment_wrapper.xsa` produced in
   Lab 2.1, then build it.
3. Create an **application project** on `ps7_cortexa9_0`, standalone, empty.
4. Write `seven_segment_app.c` and build it.
5. Connect the board, open a serial terminal at **115200 baud**.
6. **Run As → Launch on Hardware**: Vitis programs the bitstream and starts the ELF.

## Test cases worth trying

| Input | Expected display |
|-------|------------------|
| `40 + 2` | `42` |
| `9 - 4` | `5` |
| `5 - 9` | `-4` (minus sign) |
| `99 × 99` | `9801` (all four digits) |
| `7 / 0` | `----` |

## Debugging: is it hardware or software?

The terminal output tells you what the software *sent*. The display tells you
what the hardware *did*. Compare them before changing any code.

- Nothing on the display at all → bitstream not programmed, or the XSA is older
  than the hardware.
- The terminal works but the display never changes → check the register offsets
  against the generated `xseven_segment_axi_hw.h`.
- Wrong digits, right value → the segment table or its bit order, not the software.
- Flickering → the initiation interval in the HLS synthesis report, not the software.

## Expected result

- A platform and application project that build from the current XSA.
- A working board demo covering the test cases above.
- A short answer to: *which registers does the software write, and what does the
  hardware compute?*
