# CES HiWi Lab

Material for the Customized Embedded Processors lab at the KIT Chair of
Embedded Systems. Three labs that take a student from a first HLS kernel to a
neural-network accelerator running on an FPGA and driven from software.

## Target

- Board: **Blackboard**, Zynq-7000 `xc7z007sclg400-1`
- Tools: **Vivado / Vitis / Vitis HLS 2022.2**
- Students work on the lab's own x86 Ubuntu machines, at the board. No remote
  access is required or assumed.

## The labs

| | Topic |
|---|---|
| **1.1** | Vitis HLS bring-up — first project, first synthesis report |
| **1.2** | Seven-segment display driver in HLS |
| **2.1** | Turning it into an AXI4-Lite IP and building a bitstream |
| **2.2** | Driving that IP from a C program on the ARM core |
| **3.1** | A dense neural-network layer, synthesized with no pragmas — the baseline |
| **3.2** | `PIPELINE`, `UNROLL`, `ARRAY_PARTITION`: what each one costs and buys, then the same kernel checked on the board |

Labs 1 and 2 build hardware that is small and control-shaped, where the C++ is
essentially a description of the circuit. Lab 3 is the opposite case: a regular
loop nest where nothing in the source says how much hardware to build, and the
pragmas decide. That is where HLS earns its place.

## Layout

```text
labs/
  lab1/ lab2/ lab3/
    README.md      what the lab is about
    task*/         per-task README, commands, sources, reports
    setup/         shared setup notes
    docs/          slides
scripts/           build, bring-up and workspace scripts
examples/          reference HLS examples, including Xilinx's own
assets/            images
```

Each task directory is self-contained: a student should be able to work from
one directory without reading the others.

## Scripts

| | |
|---|---|
| `build_lab2_vivado.tcl` | Lab 2 block design through to a bitstream |
| `build_lab3_vivado.tcl` | one Lab 3 bitstream per pragma variant |
| `create_vitis_workspace.tcl` | build a Vitis workspace from an XSA, without the GUI |
| `debug_init.tcl` | program the board and run an ELF, without the GUI launcher |

The last two exist because a Vitis workspace records its platform by absolute
path and does not survive being copied between machines. Move the XSA, rebuild
the workspace where the board is.

## Status

In active development. Lab 1 and Lab 2 are complete and have run on hardware.
Lab 3's kernel, testbenches and application are written and pass in simulation;
the five-bitstream board flow has not yet been run on the lab machines.

The `setup/` directories are still placeholders.
