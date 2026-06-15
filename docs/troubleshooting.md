# Troubleshooting Guide

## Purpose

This guide collects common problems expected during setup, HLS development, Vivado integration, and Vitis bring-up.

Students should use this document before asking for help. Instructors can also use it to quickly identify whether the issue is environmental, project-related, or conceptual.

## General debugging rule

Always identify the failing stage first:

1. login/setup
2. repository layout
3. tool availability
4. HLS synthesis
5. IP export
6. Vivado block design
7. synthesis/implementation
8. bitstream generation
9. Vitis hardware handoff
10. software build/run
11. board behavior

Do not debug the board if the build failed earlier.

## Problem: command not found

Example:

```text
vivado: command not found
vitis: command not found
vitis_hls: command not found
```

Possible causes:

- Xilinx tools are not installed
- environment setup script was not sourced
- wrong machine
- wrong shell
- PATH not configured

Check:

```bash
hostname
echo "$PATH"
which vivado || true
find /opt -maxdepth 3 -iname "*vivado*" 2>/dev/null | head
```

Resolution:

- confirm the intended machine
- ask whether a setup script or module must be loaded
- do not randomly edit .bashrc without understanding the lab environment

## Problem: GUI does not open

Symptoms:

```text
Can't open display
DISPLAY is not set
x11 failed
```

Check:

```bash
echo "$DISPLAY"
xdpyinfo >/dev/null && echo "x11 works" || echo "x11 failed"
```

Possible causes:

- ssh was started without X forwarding
- local X server is not running
- XQuartz configuration issue on macOS
- remote server does not allow X11 forwarding
- GUI is not supported on the selected node

Workaround:

- use command-line flow where possible
- document the failure
- switch to a known working access method if required

## Problem: wrong Xilinx version

Symptoms:

- project opens with upgrade warning
- generated IP behaves differently
- build scripts fail
- exported files are incompatible

Check:

```bash
vivado -version
vitis -version
```

Resolution:

- use the lab-approved version consistently
- do not mix generated artifacts from different versions unless explicitly allowed
- document the version in the task notes

## Problem: missing top-level function

Symptoms:

- HLS cannot synthesize
- tool cannot identify design entry point
- synthesis report is empty or unexpected

Likely cause:

- top function name not configured correctly
- source file added but not selected
- function signature changed but project settings were not updated

Resolution:

- confirm the intended top-level function
- confirm project settings
- clean and rerun synthesis if needed

## Problem: HLS synthesis succeeds but result is not useful

Possible causes:

- wrong interface pragmas or settings
- missing input/output behavior
- function optimized away
- testbench does not cover real cases
- design is combinational but expected to be controlled differently

Check:

- synthesis report
- generated interface summary
- latency/resource report
- warnings

## Problem: generated IP is not visible in Vivado

Possible causes:

- IP was not exported
- repository path not added
- Vivado IP catalog not refreshed
- wrong generated folder selected
- project was opened from a different working directory

Resolution:

- locate exported IP
- add IP repository path
- refresh IP catalog
- confirm IP appears with expected name and version

## Problem: block design validation fails

Possible causes:

- unconnected ports
- missing clock/reset
- incompatible interfaces
- wrong address assignment
- missing AXI interconnect or processor connection

Resolution:

- run validate design
- inspect first critical warning
- fix clock/reset first
- assign addresses if needed
- regenerate output products

## Problem: synthesis or implementation fails

Check logs:

```bash
find . -name "*.log" -o -name "*.jou" -o -name "*.rpt"
grep -R "ERROR:" . || true
grep -R "CRITICAL WARNING:" . || true
```

Common causes:

- invalid constraints
- missing source file
- duplicate module names
- wrong board part
- incompatible IP version
- stale generated output products

## Problem: bitstream generated but board behavior is wrong

Possible causes:

- wrong pin constraints
- inverted signal logic
- wrong board selected
- stale bitstream programmed
- software using old hardware handoff
- seven-segment encoding mismatch
- active-high versus active-low confusion

Resolution:

- verify board part
- verify constraints
- verify signal polarity
- rebuild from clean state
- confirm timestamp of bitstream/hardware handoff

## Problem: Vitis software project does not match hardware

Possible causes:

- old exported hardware platform
- BSP not regenerated
- wrong XSA file
- hardware changed but software project not updated

Resolution:

- re-export hardware from Vivado
- update or recreate platform project
- regenerate BSP
- rebuild application

## Problem: students cannot explain what they built

This is a documentation and teaching problem, not only a technical problem.

Ask:

- what is the input?
- what is the output?
- what is generated?
- what is hand-written?
- what is hardware?
- what is software?
- how did you verify it?

If the student cannot answer these, the lab text needs stronger conceptual scaffolding.

## Minimum help request format

```text
Task:
Machine:
Tool version:
Exact failing command:
First error message:
Relevant log file:
What I expected:
What happened:
What I already checked:
```
