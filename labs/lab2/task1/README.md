# Lab 2.1 — HLS calculator as an AXI4-Lite IP

## Objective

Turn the Lab 1 display driver into an AXI4-Lite peripheral that also performs
arithmetic, then integrate it into a Zynq block design and build a bitstream.

No Verilog or VHDL is written, and **no extra RTL source is added in Vivado**.
The digit multiplexing stays inside the HLS block, which keeps the block design
small enough to build in one session.

## What changes compared to Lab 1

If your Lab 1 works, this task is a small edit — that is the point.

| Part of the design | Lab 2 |
|---|---|
| segment table (`SEG_A … SEG_G`) | unchanged |
| active-low conversion | unchanged |
| `tick` / `pos` scan registers | unchanged |
| `*an = ~(1 << pos)` | unchanged |
| `#pragma HLS PIPELINE II=1` | unchanged |
| input ports | `value` becomes `op1`, `op2`, `op_sel` |
| interface pragmas | three `s_axilite` lines instead of one `ap_none` |
| new logic | one `switch` that computes the result |

## The interface

```cpp
void seven_segment_axi(ap_uint<7> op1, ap_uint<7> op2, ap_uint<2> op_sel,
                       ap_uint<8> *seg, ap_uint<4> *an) {
#pragma HLS INTERFACE s_axilite port=op1    bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op2    bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op_sel bundle=CTRL
#pragma HLS INTERFACE ap_none   port=seg
#pragma HLS INTERFACE ap_none   port=an
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1
```

Only the three inputs become registers. `seg` and `an` stay plain wires that go
to the board pins.

`ap_ctrl_none` keeps the block **free-running**, so the display never stops
scanning and the processor does not have to start it. A useful consequence:
the generated register map has **no `ap_start` register at all**.

## Arithmetic and edge cases

Operands are 0 … 99. `op_sel` selects add (0), subtract (1), multiply (2),
divide (3).

Hardware cannot throw an exception, so every case must produce a defined
display:

- negative results (subtraction) show a leading minus sign — segment G alone
- division by zero shows `----`
- anything that does not fit on four digits shows `----`

`99 × 99 = 9801` still fits, so multiplication never overflows in this range.

## The register map

Vitis HLS generates `xseven_segment_axi_hw.h` during export. Read the offsets
from that file rather than guessing them:

| Offset | Register |
|--------|----------|
| `0x00` … `0x0c` | reserved (no `ap_start`) |
| `0x10` | `op1` |
| `0x18` | `op2` |
| `0x20` | `op_sel` |

## Workflow

1. Copy the Lab 1 sources to `seven_segment_axi.{h,cpp}`; rename the top
   function and the include guard.
2. Add the three `s_axilite` pragmas and the arithmetic `switch`.
3. C simulation, then C synthesis — confirm **Interval = 1** again.
4. **Export RTL** as a Vivado IP.
5. In Vivado: new project, part `xc7z007sclg400-1`, add the Blackboard XDC,
   **no design sources**.
6. Block design: ZYNQ7 PS + Block Automation, add the IP repository, add the
   IP, run Connection Automation.
7. Make `seg` and `an` external — named exactly `seg` and `an`.
8. Assign the address, validate, create the HDL wrapper.
9. Synthesis → Implementation → Generate Bitstream.
10. **Export Hardware** with the bitstream included, producing the XSA.

## Common failure modes

**A stale IP.** The exported IP keeps the same version number
(`ces.kit.edu:hls:seven_segment_axi:1.0`) every time it is re-exported, so
Vivado may quietly reuse a cached older copy. After changing the HLS code,
refresh the IP catalog and check the block design really shows the new ports.
Rebuilding the project from scratch is the reliable cure.

**External ports renamed.** If Vivado creates `seg_0` instead of `seg`, the XDC
constraint no longer matches and the pins are never assigned. Rename it in
External Port Properties.

**A stale XSA.** Whenever the hardware changes, the bitstream *and* the XSA must
be regenerated. Lab 2.2 will otherwise build against an old description.

## Expected result

- An exported IP whose synthesis report shows `s_axi_CTRL`, `seg`, `an` and Interval = 1.
- A validated block design with the display outputs pinned by the XDC.
- A bitstream and an XSA generated from the current IP.
- A short answer to: *where is the HLS IP connected inside the system?*
