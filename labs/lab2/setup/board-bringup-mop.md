# Lab 2 — Method of Procedure: from source to a working calculator

A four-digit seven-segment calculator. The arithmetic and the display scan are
a single HLS block in the FPGA fabric; the operands arrive over AXI4-Lite from
a C program running on the ARM core. You type two numbers and an operation at a
terminal, and the answer appears on the board.

Follow this top to bottom on the machine the board is attached to. Every stage
ends with a **checkpoint**. If a checkpoint does not hold, stop and fix it
there — carrying a broken stage forward is what turns a ten-minute problem into
a three-hour one, because the failure surfaces somewhere unrelated to its
cause.

```text
A. Vitis HLS   C++  ->  C simulation  ->  C synthesis  ->  exported AXI IP
B. Vivado      IP   ->  block design + pin constraints  ->  bitstream  ->  XSA
C. Vitis       XSA  ->  platform      ->  application   ->  ELF
D. Board       bitstream + ELF        ->  a lit display and a live terminal
```

Target: Blackboard, Zynq-7000 `xc7z007sclg400-1`, Vivado / Vitis / Vitis HLS
**2022.2**.

---

## What you are building

```text
you type at a terminal
        │
        ▼
C program on the ARM core            three register writes, nothing else
        │  Xil_Out32(base + 0x10, op1)
        │  Xil_Out32(base + 0x18, op2)
        │  Xil_Out32(base + 0x20, op_sel)
        ▼
AXI4-Lite slave
        │
        ▼
seven_segment_axi in the fabric      arithmetic + four-digit time-multiplexed scan
        │
        ▼
seg[7:0] and an[3:0] on real package pins
        │
        ▼
the display on the board
```

Three properties of this design shape everything below.

**It has real pins.** `seg` and `an` leave the chip. That means a constraints
file, and it means the block design has external ports whose names have to
match that file exactly.

**It is free-running.** The interface is `ap_ctrl_none`, so there is no
`ap_start` and **no control register at all** — the generated register map
begins at `0x10` and `0x00`–`0x0c` are reserved. The block scans the display
continuously, whether or not any software is running.

**It therefore lights up on its own.** All three registers reset to zero, so the
moment the bitstream is loaded the block computes `0 + 0` and displays it. That
gives you a five-second test that splits the whole problem in half — covered in
D.2, and the single most useful thing in this document.

There are two independent outputs to watch: the display and the terminal. They
fail separately, and knowing which one is dead tells you which half to look at.

> **Confidence note.** The register offsets below come from the generated
> `xseven_segment_axi_hw.h`, and the pin assignments from the project's own
> constraints file — both are certain. Which PS UART the board's USB bridge is
> wired to is **not** settled; see B.4.

---

## Stage 0 — Before you touch anything

### 0.1 Hardware

- Board powered, USB cable to the host. One cable carries JTAG and the UART as
  two channels of a single FTDI device.
- Boot mode strapped for **JTAG**. If the board boots from SD or QSPI instead, a
  bootloader runs at power-on and leaves the CPU in a state your debugger did
  not create, which makes later register access fail for reasons unrelated to
  your design.

### 0.2 The serial port

```bash
ls -l /dev/serial/by-id/
```

A healthy Blackboard looks like this:

```text
usb-Xilinx_JTAG+Serial_887100000306-if01-port0 -> ../../ttyUSB1
```

`if01` is the FTDI's second channel, the UART. The JTAG channel is claimed by
the Xilinx driver and never appears as a tty, so seeing exactly **one** node is
correct. Use whatever node this command prints — the number is assigned by udev
and can move between sessions.

```bash
groups | grep -q dialout && echo "dialout ok" || echo "NOT in dialout"
```

Without `dialout` membership the port cannot be opened. Fixing it needs a
re-login:

```bash
sudo usermod -aG dialout $USER
```

If no FTDI device appears at all, the cable drivers were never installed:

```bash
sudo /Software/xilinx/2022.2/Vivado/2022.2/data/xicom/cable_drivers/lin64/install_script/install_drivers/install_drivers
```

### 0.3 Tools

Each tool has its own `settings64.sh`, and sourcing the wrong one leaves you in
a shell where `vivado` exists and `vitis_hls` does not.

```bash
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh   # Stage A
source /Software/xilinx/2022.2/Vivado/2022.2/settings64.sh      # Stage B
source /Software/xilinx/2022.2/Vitis/2022.2/settings64.sh       # Stages C and D
```

All three must report **2022.2**. An XSA written by one release and opened by
another fails in ways that read like file corruption.

### 0.4 Sources

```bash
cd ~/work/ces-hiwi-lab
ls labs/lab2/task1/src labs/lab2/task2/vitis/seven_segment_app/src
```

Expect `seven_segment_axi.{h,cpp}`, `seven_segment_axi_tb.cpp` and
`seven_segment_app.c`.

**Checkpoint 0:** three tools at 2022.2, one `ttyUSB` node visible by id, and
the sources present.

---

## Stage A — Vitis HLS: C++ to an exported IP

### A.1 The block

```cpp
void seven_segment_axi(ap_uint<7> op1, ap_uint<7> op2, ap_uint<2> op_sel,
                       ap_uint<8> *seg, ap_uint<4> *an);
```

| | |
|---|---|
| `op1`, `op2` | operands, 0…99, written by the processor |
| `op_sel` | 0 add, 1 subtract, 2 multiply, 3 divide |
| `seg` | eight cathodes, **active low**, `seg[0]`=A … `seg[6]`=G, `seg[7]`=decimal point |
| `an` | four anodes, **active low**, one at a time, `an[0]` is the rightmost digit |

The interface pragmas are the substance of Lab 2.1:

```cpp
#pragma HLS INTERFACE s_axilite port=op1 bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op2 bundle=CTRL
#pragma HLS INTERFACE s_axilite port=op_sel bundle=CTRL
#pragma HLS INTERFACE ap_none port=seg
#pragma HLS INTERFACE ap_none port=an
#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS PIPELINE II=1
```

Three inputs become AXI registers. `seg` and `an` stay plain wires headed for
package pins. `ap_ctrl_none` is what makes the block free-running, and is the
reason there is no `ap_start` register for software to write.

Hardware cannot throw an exception, so every input has a defined display:
negative results show a leading minus (segment G alone), and division by zero
or any result wider than four digits shows `----`. `99 × 99 = 9801` still fits,
so multiplication never overflows in this range.

### A.2 Compile on the host first

Faster than the tool, and the error messages are far better.

```bash
cd labs/lab2/task1/src
g++ -O2 -std=c++11 -I $XILINX_HLS/include \
    seven_segment_axi.cpp seven_segment_axi_tb.cpp -o /tmp/sseg_test
/tmp/sseg_test
```

### A.3 Synthesize and export

```bash
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh
mkdir -p ~/vitis/lab2_task1 && cd ~/vitis/lab2_task1
cp ~/work/ces-hiwi-lab/labs/lab2/task1/src/* .

cat > run_hls.tcl <<'EOF'
open_project -reset lab2_task1_hls
set_top seven_segment_axi
add_files seven_segment_axi.cpp
add_files -tb seven_segment_axi_tb.cpp
open_solution -reset "solution1" -flow_target vivado
set_part {xc7z007sclg400-1}
create_clock -period 20 -name default
csim_design
csynth_design
export_design -format ip_catalog -vendor ces.kit.edu -library hls -version 1.0
exit
EOF

vitis_hls -f run_hls.tcl
```

**The `-vendor` is not optional.** Vivado instantiates this IP by the exact
VLNV `ces.kit.edu:hls:seven_segment_axi:1.0`. Exported under the default
vendor, it will not be found.

**The clock period is not arbitrary either.** The header derives the refresh
rate from a fixed constant:

```cpp
const unsigned SEVEN_SEG_CLK_HZ        = 50000000;
const unsigned SEVEN_SEG_REFRESH_TICKS = 50000;  // 1 ms per digit @ 50 MHz
```

Synthesize at 20 ns and drive `FCLK_CLK0` at 50 MHz in Stage B, and the two
agree. Run the fabric faster and the scan simply runs faster, which is
harmless. Run it much slower and each digit stays lit too long, which is
visible flicker.

### A.4 GUI path

1. `vitis_hls &` → **Create Project**, location outside the repository.
2. **Add Files**: `seven_segment_axi.cpp`. **Top Function**:
   `seven_segment_axi`.
3. **Add Testbench**: `seven_segment_axi_tb.cpp`.
4. **Part** `xc7z007sclg400-1`, **clock period** `20` ns.
5. **Run C Simulation** — must pass before any synthesis figure means anything.
6. **Run C Synthesis**.
7. **Export RTL** → format *IP Catalog*. Open the configuration dialog and set
   **Vendor** `ces.kit.edu`, **Library** `hls`, **Version** `1.0`.

### A.5 The register map is a contract

```bash
find ~/vitis/lab2_task1 -name 'xseven_segment_axi_hw.h' -exec cat {} +
```

| offset | register | width |
|--------|----------|-------|
| `0x00`–`0x0c` | reserved — **no `ap_ctrl`**, the block is free-running | |
| `0x10` | `op1` | 7 bits |
| `0x18` | `op2` | 7 bits |
| `0x20` | `op_sel` | 2 bits |

`seven_segment_app.c` hard-codes these three offsets. Vitis HLS assigns them
from the argument order, so adding or reordering an argument moves them and the
application must be updated in the same commit.

**Checkpoint A:** C simulation passed,
`lab2_task1_hls/solution1/impl/ip/component.xml` exists, and the VLNV is right:

```bash
grep -m1 -A3 "spirit:vendor" ~/vitis/lab2_task1/lab2_task1_hls/solution1/impl/ip/component.xml
```

---

## Stage B — Vivado: IP to bitstream and XSA

### B.1 Batch

```bash
source /Software/xilinx/2022.2/Vivado/2022.2/settings64.sh
cd ~/work/ces-hiwi-lab
vivado -mode batch -source scripts/build_lab2_vivado.tcl
```

Read the paths at the top of that script first — `ip_repo` and `xdc` are
absolute and may not match where you put things.

### B.2 GUI path, step by step

1. **Create Project** → RTL Project, *Do not specify sources*, part
   `xc7z007sclg400-1`.
2. **Settings → IP → Repository** → add
   `~/vitis/lab2_task1/lab2_task1_hls/solution1/impl/ip` → **Refresh**. Point
   at the directory *containing* `component.xml`. The IP then appears in the
   catalog under *User Repository*.
3. **Add Sources → Add or create constraints** → add
   `labs/lab2/task1/vivado/lab2_task1.srcs/constrs_1/new/session2_task1.xdc`.
4. **Create Block Design**, name it `seven_segment`.
5. **Add IP** → `ZYNQ7 Processing System`. **Run Block Automation** → make
   FIXED_IO and DDR external. Do **not** apply a board preset: no Blackboard
   board file is installed on our machines, and a preset for a different board
   gives you the wrong DDR settings.
6. Double-click the PS block and set three things:
   - **PS-PL Configuration** → `M_AXI_GP0` **enabled**
   - **Clock Configuration** → `FCLK_CLK0` **enabled**, **50 MHz** (A.3)
   - **MIO Configuration → I/O Peripherals → UART** — see B.4
7. **Add IP** → `seven_segment_axi`.
8. **Run Connection Automation** → select all. This wires `s_axi_CTRL` to
   `M_AXI_GP0` through a new AXI Interconnect and connects clocks and resets.
9. **Bring the display out.** Right-click the `seg` pin on the IP block →
   **Make External**. Same for `an`.

   Then **rename both external ports to exactly `seg` and `an`**: select the
   port and edit the name in the External Port Properties panel.

   Vivado names them `seg_0` and `an_0` by default, and the constraints file
   refers to `seg` and `an`. When the names disagree the constraints match
   nothing, the pins are placed wherever the tool likes, the build succeeds, and
   the display stays dark with no error anywhere. This is the most expensive
   silent mistake in Stage B.
10. **Address Editor** → confirm `seven_segment_axi_0` has an address. Write it
    down; Stage C needs it.
11. **Validate Design** (F6).
12. In *Sources*, right-click the `.bd` → **Create HDL Wrapper** → let Vivado
    manage it.
13. **Generate Bitstream**.
14. **File → Export → Export Hardware** → **Include bitstream** → save the XSA.

### B.3 The constraints, and the warnings you should ignore

These are the lines that matter — the pins the display is physically on:

```tcl
# anodes, active low, an[0] is the rightmost digit
set_property -dict { PACKAGE_PIN K19  IOSTANDARD LVCMOS33 } [get_ports { an[0] }];
set_property -dict { PACKAGE_PIN H17  IOSTANDARD LVCMOS33 } [get_ports { an[1] }];
set_property -dict { PACKAGE_PIN M18  IOSTANDARD LVCMOS33 } [get_ports { an[2] }];
set_property -dict { PACKAGE_PIN L16  IOSTANDARD LVCMOS33 } [get_ports { an[3] }];

# cathodes, active low, seg[0]=A .. seg[6]=G, seg[7]=decimal point
set_property -dict { PACKAGE_PIN K14  IOSTANDARD LVCMOS33 } [get_ports { seg[0] }];
set_property -dict { PACKAGE_PIN H15  IOSTANDARD LVCMOS33 } [get_ports { seg[1] }];
set_property -dict { PACKAGE_PIN J18  IOSTANDARD LVCMOS33 } [get_ports { seg[2] }];
set_property -dict { PACKAGE_PIN J15  IOSTANDARD LVCMOS33 } [get_ports { seg[3] }];
set_property -dict { PACKAGE_PIN M17  IOSTANDARD LVCMOS33 } [get_ports { seg[4] }];
set_property -dict { PACKAGE_PIN J16  IOSTANDARD LVCMOS33 } [get_ports { seg[5] }];
set_property -dict { PACKAGE_PIN H18  IOSTANDARD LVCMOS33 } [get_ports { seg[6] }];
set_property -dict { PACKAGE_PIN K18  IOSTANDARD LVCMOS33 } [get_ports { seg[7] }];
```

The same file also constrains `clk`, `led[*]` and `data_in[*]`, none of which
exist in this design. Vivado emits critical warnings that no ports matched, and
complains that the `create_clock` on `clk` cannot find its port. **This is
expected noise** — the clock comes from the PS as `FCLK_CLK0`, not from a
package pin.

A warning about `seg` or `an` not matching is a different matter entirely. That
one is fatal to the display, and it means step 9 above was not completed.

### B.4 The UART — decide this before you build

Lab 2.2 is interactive: the application reads operands with `scanf` and prints
with `printf`. With no PS UART in the design, `printf` has nowhere to go and
`scanf` blocks for ever. Neither produces an error message; the board simply
sits there.

Enable it under **MIO Configuration → I/O Peripherals → UART**.

Which UART instance is the open question. The evidence available on our
machines:

- A Blackboard platform that is known to work (`~/lpd-lab/blkboard`) enables
  **both**: `UART_CLK_CTRL = 0x00000A03` — bits 0 and 1 set — with UART0 on
  MIO 14/15 and UART1 on MIO 48/49. Its `STDOUT_BASEADDRESS` is `0xE0000000`,
  which is **UART0**.
- MIO 48/49 is the conventional UART1 routing on Zynq boards in general, but a
  convention is not this board's schematic.

**Do what that platform does: enable both.** UART0 on MIO 14–15 and UART1 on
MIO 48–49, both at 115200. They use different pins and do not conflict.

The payoff is large. With both present in the hardware, choosing between them
becomes a **software** decision — one BSP setting, no new bitstream. Getting it
wrong the other way costs a full synthesis run per guess.

Base addresses, for reading `xparameters.h` later:

| instance | base |
|----------|------|
| UART0 | `0xE0000000` |
| UART1 | `0xE0001000` |

### B.5 Check the implemented result

```text
BUILD_RESULT: OK
XSA: .../seven_segment_wrapper.xsa
```

Open the implemented design:

- **Report Timing Summary** → WNS must be **positive**. A negative WNS design
  may still program and may still appear to work, and it is not trustworthy.
- **I/O Ports** window, or `report_io` → `an[0]` must read `K19` and `seg[0]`
  must read `K14`. Anywhere else means the constraints did not apply, and the
  display will not work no matter what the software does.

**Checkpoint B:** `BUILD_RESULT: OK`, positive WNS, `seg`/`an` on the pins in
B.3, a `.bit` and an `.xsa` on disk, and the AXI base address written down.

---

## Stage C — Vitis: XSA to ELF

### C.0 The vocabulary, because the wizards assume you know it

Vitis stacks several things with confusingly similar names. Getting these
straight once makes the rest of the stage mechanical.

| Term | What it actually is |
|------|---------------------|
| **Workspace** | A directory, not a project. Everything below lives inside it. Vitis has one open at a time. |
| **Hardware specification / XSA** | The handoff file from Stage B: bitstream, `ps7_init.*`, address map. The only artefact that travels between machines. |
| **Platform project** | Built **from** an XSA. The hardware plus the software layer that lets code reach it. Produces a `.xpfm` descriptor that applications point at. |
| **Domain** | A slot inside a platform: one processor plus one OS. Ours is `standalone_domain` = `ps7_cortexa9_0` + `standalone`. A platform can hold several; ours holds one. |
| **BSP** (Board Support Package) | Generated inside the domain: drivers for every IP in the XSA, `xparameters.h`, the C library glue, and the `stdin`/`stdout` setting. "Modify BSP settings" edits this. |
| **Application project** | Your program — `seven_segment_app.c` and nothing else. Targets one platform and one domain. Produces the ELF. |
| **System project** | A container grouping the applications that run **together** on the board, one per processor. Created automatically as `<app>_system`. Here it holds a single application — but it has its own Build button and its own launch type, which is where people get lost. |

```text
XSA -> platform project -> domain (BSP) -> application project -> ELF
                                                  └ grouped by ┘
                                                  system project
```

The dependency runs one way. Change the XSA and everything downstream is stale.

### C.1 One workspace, and never a copied one

A Vitis `.prj` records its platform by **absolute path**, and so does the
generated `_ide/launch/*.launch`. Copy a workspace to another machine and Vitis
cannot resolve the platform, concludes the application was never built, and
reports **"Binary File not Found"** while the ELF sits in `Debug/` exactly
where it belongs.

There is a second version of this that is harder to spot: **two workspaces,
each containing a `seven_segment_app`, pointing at different platforms** — one
built before the UART was added and one after. The symptom is a terminal that
reports "connected" and prints nothing, for ever.

Before debugging anything else, find out what your application actually targets:

```bash
find ~/vitis -maxdepth 4 -name "*.prj" -exec grep -Ho 'platform="[^"]*"' {} +
```

and whether that platform has a UART at all:

```bash
find ~/vitis -name xparameters.h -path "*bsp*" \
     -exec grep -H -E "STDOUT_BASEADDRESS|XPAR_PS7_UART_[01]_BASEADDR" {} +
```

`STDOUT_BASEADDRESS` must be `0xE0000000` or `0xE0001000`. Any other value —
and in particular a platform with no `XPAR_PS7_UART_*` line at all — means
`printf` is writing to something that is not a UART.

### C.2 Batch

```bash
source /Software/xilinx/2022.2/Vitis/2022.2/settings64.sh
cd ~/work/ces-hiwi-lab
xsct scripts/create_vitis_workspace.tcl \
     ~/vivado/lab2_calc/seven_segment_wrapper.xsa \
     labs/lab2/task2/vitis/seven_segment_app/src \
     ~/vitis/lab2_ws \
     seven_segment_app
```

It deletes and recreates the workspace every time, deliberately: a reused
workspace is how a stale platform survives an XSA change while the application
silently keeps the old address map.

The equivalent XSCT commands, to drive it by hand:

```tcl
setws ~/vitis/lab2_ws
platform create -name seven_segment_platform -hw <path>.xsa \
                -proc ps7_cortexa9_0 -os standalone
platform write
platform generate
app create -name seven_segment_app -platform seven_segment_platform \
           -domain standalone_domain -template {Empty Application(C)}
importsources -name seven_segment_app -path <src dir> -target-path src
app build -name seven_segment_app
```

### C.3 GUI path, click by click

#### C.3.1 Open the workspace

```bash
vitis -workspace ~/vitis/lab2_ws &
```

Without `-workspace` you get a launcher dialog. Point it at a **new or empty**
directory.

The **Explorer** panel on the left is the project tree. The **Assistant** panel
(Window → Show View → Assistant) holds build configurations and Run/Debug
entries. You will use both.

If Vitis opens on an old workspace, **File → Switch Workspace**. Check the
title bar — two workspaces holding similar-looking projects is exactly the trap
in C.1.

#### C.3.2 Create the platform project

1. **File → New → Platform Project**.
2. **Platform project name**: `seven_segment_platform`. Next.
3. Choose **Create a new platform from hardware (XSA)** and browse to the XSA
   from Stage B.
4. **Operating system** `standalone`, **Processor** `ps7_cortexa9_0`.
5. **Generate boot components** — see C.5. Not needed on a JTAG-strapped board;
   leaving it on is harmless and avoids one class of launcher complaint.
6. Finish. The platform editor (`platform.spr`) opens.

#### C.3.3 Set the BSP's serial port

The platform editor tree:

```text
seven_segment_platform
└── ps7_cortexa9_0
    └── standalone_domain
        ├── Board Support Package
        └── Drivers
```

1. Click **standalone_domain** → **Modify BSP Settings…**
2. **Overview → standalone** → set **stdin** and **stdout**.
   With both UARTs in the hardware (B.4), start with `ps7_uart_0` — that is
   what the known-working Blackboard platform uses.
3. OK.

If the dropdown offers only `none`, there is no UART in the hardware. Stop.
This cannot be fixed in Vitis; go back to B.4, rebuild the bitstream and the
XSA, and restart Stage C.

While you are in this editor, open **Drivers**. It lists every IP from the XSA
with the driver assigned to it. `seven_segment_axi_0` should have a generated
driver; if it says `none`, the IP was not recognised and `xparameters.h` will
have no base address for it.

#### C.3.4 Build the platform

Select `seven_segment_platform` in Explorer → **Build Project** (hammer, or
Ctrl+B, or right-click → Build Project).

This is the step that writes `xparameters.h`. **The `stdout` setting you just
made does not exist until this build runs** — changing the BSP and not
rebuilding is a common way to conclude, wrongly, that the setting did nothing.

#### C.3.5 Create the application project

1. **File → New → Application Project**.
2. **Select a platform from repository** → `seven_segment_platform`. Next.
3. **Application project name**: `seven_segment_app`. The **System project
   name** field fills in as `seven_segment_app_system` — leave it.
4. **Domain**: `standalone_domain` on `ps7_cortexa9_0`. Next.
5. **Template**: **Empty Application (C)**. Not the C++ template —
   `seven_segment_app.c` is C, and C++ links a heavier runtime for no reason.
6. Finish.

#### C.3.6 Put your source in it

1. If the template left a `main.c` or `helloworld.c` in `seven_segment_app/src`,
   delete it. Two `main()` functions is a link error that reads like nonsense.
2. Right-click `seven_segment_app/src` → **Import Sources…** → *File system* →
   navigate to `labs/lab2/task2/vitis/seven_segment_app/src` → tick **only**
   `seven_segment_app.c` → Finish.

#### C.3.7 Build, and know which node you are building

```text
seven_segment_platform          <- BSP and hardware
seven_segment_app_system        <- container
└── seven_segment_app           <- your program
    ├── src/seven_segment_app.c
    └── Debug/                  <- appears after the first build
```

`seven_segment_app` and `seven_segment_app_system` each have a Build button.
Either works here. It matters when a build reports success and the ELF did not
change — check which node was selected, and check the timestamp:

```bash
ls -la ~/vitis/lab2_ws/seven_segment_app/Debug/seven_segment_app.elf
```

#### C.3.8 What a correct workspace looks like

```text
lab2_ws/
├── seven_segment_platform/
│   ├── hw/                      XSA contents: .bit, ps7_init.{c,h,tcl}
│   ├── export/                  the .xpfm applications point at
│   ├── ps7_cortexa9_0/standalone_domain/bsp/
│   │   └── ps7_cortexa9_0/include/xparameters.h
│   └── platform.spr             double-click to reopen the platform editor
├── seven_segment_app/
│   ├── src/seven_segment_app.c
│   ├── Debug/seven_segment_app.elf
│   └── _ide/
│       ├── bitstream/           copy of the .bit, used by "Program FPGA"
│       ├── psinit/ps7_init.tcl
│       └── launch/*.launch      the run configuration, absolute paths and all
└── seven_segment_app_system/
    └── seven_segment_app_system.sprj
```

#### C.3.9 Verify the base address symbol

The application uses:

```c
#define CALC_BASEADDR  XPAR_SEVEN_SEGMENT_AXI_0_S_AXI_CTRL_BASEADDR
```

Confirm it exists and agrees with the address Stage B printed:

```bash
find ~/vitis/lab2_ws -name xparameters.h -exec grep -i "SEVEN_SEGMENT_AXI" {} +
```

If the block design instance was named something else, the symbol differs. Fix
the `#define`; do not guess.

The offsets the application writes are `0x10`, `0x18` and `0x20`, and there is
deliberately no `ap_start` write. Cross-check against the generated header from
A.5.

### C.4 Set up the run configuration

**Run → Run Configurations…** The categories in the left list:

- **Single Application Debug** — one ELF on one processor. This is ours.
  Double-click the *category name* to create a configuration under it; there is
  no pre-made entry to select.
- **System Project Debug** — every application in a system project, launched
  together. For multi-core designs.
- **Attach to Running Target** — connect to something already running.

In the **Target Setup** tab, four settings decide whether this works at all:

| Setting | Set it to | Why |
|---------|-----------|-----|
| Reset entire system | **unchecked** | On Zynq-7000 this asserts `PS_SRST`, which drops the DAP *and* erases the PL. It is the direct cause of `cannot reset APU`. |
| Program FPGA | **checked** | Loads the bitstream from `_ide/bitstream/`. |
| Run ps7_init | **checked** | Configures DDR, the PL clocks and the UART. |
| Run ps7_post_config | **checked** | Finishes PS configuration after the fabric is loaded. |

In the **Application** tab, confirm the ELF path. Leave *Stop at 'main'*
unchecked unless you intend to single-step.

### C.5 About the FSBL

On a JTAG-strapped board you do not need one. `ps7_init` performs the same PS
configuration, and the debugger loads your ELF directly.

You may still meet a launcher complaint that an FSBL is missing. That happens
when the run configuration has the FSBL option enabled — often inherited from
an imported `.launch` file — while the platform was created without boot
components. Two ways out:

- **Preferred:** turn the FSBL option **off** in the run configuration's Target
  Setup. Nothing else changes.
- **Also fine:** tick **Generate boot components** when creating the platform.
  It builds `zynq_fsbl` and costs build time, nothing more.

If you do run an FSBL, one property of it is useful: **it prints its own banner
to the UART.** An FSBL that runs while the terminal stays blank is strong
evidence that the serial path is wrong, and points at B.4 rather than at your
application.

Be aware that the FSBL runs `ps7_init` itself, so with both the FSBL and *Run
ps7_init* enabled the PS is configured twice. Harmless here — but if PS
behaviour ever looks inconsistent, turn one of them off.

### C.6 When the hardware changes

A new bitstream means a new XSA and a stale platform. Two options:

- **Right-click the platform → Update Hardware Specification**, point at the new
  XSA, rebuild the platform, rebuild the application. Works, and occasionally
  leaves stale generated files behind.
- **Delete the workspace and rerun C.2.** Under a minute, and cannot leave
  anything stale.

Prefer the second.

**Checkpoint C:** `Debug/seven_segment_app.elf` exists, the `xparameters.h`
symbol matches the address from Stage B, and `STDOUT_BASEADDRESS` is a real
UART base.

---

## Stage D — On the board

### D.1 Open the terminal first

Open it before programming anything, so you catch the application's first line.

```bash
screen /dev/ttyUSB1 115200
```

Use whatever node `ls -l /dev/serial/by-id/` reported. `Ctrl-A` then `k` quits
`screen`. Vitis' own **Serial Terminal** view works equally well — but only one
program can hold the port, so close one before opening the other.

### D.2 Program the bitstream and look at the display, before any software

This is the most valuable five seconds in the whole procedure. Do not skip it.

**Xilinx → Program Device**, or:

```bash
xsct -eval "connect; targets -set -filter {name =~ \"xc7z*\"}; fpga -file <path>.bit"
```

Now look at the board. The block is free-running and all three registers reset
to zero, so it computes `0 + 0` and displays it. **The display must be lit.**

- **Display lit** → the fabric, the pin constraints, the external port names and
  the PL clock are all correct. Every remaining problem is in the PS, the UART
  or the software, and you never need to open Vivado again.
- **Display dark** → stop here. The fault is in Stage B: external ports named
  `seg_0`/`an_0`, constraints not added to the project, `FCLK_CLK0` not
  enabled, or the bitstream not actually programmed. No amount of Vitis work
  will fix it.

That single observation splits the problem cleanly in half.

### D.3 Load and run the application

```bash
cd ~/work/ces-hiwi-lab
xsct scripts/debug_init.tcl \
     <bitstream>.bit \
     ~/vitis/lab2_ws/seven_segment_platform/hw/ps7_init.tcl \
     ~/vitis/lab2_ws/seven_segment_app/Debug/seven_segment_app.elf
```

Each step prints `ok` or `FAILED` with a reason. The ordering matters, and is
worth understanding because the GUI launcher gets it wrong by default:

1. connect, JTAG at 10 MHz
2. `rst -system`
3. **wait 3 s** — the reset drops the DAP; touching a target before it returns
   is exactly what produces `cannot reset APU`
4. **program the PL** — the system reset wiped the fabric, so anything loaded
   earlier is gone
5. select the A9, `rst -processor` — clears a leftover MMU
6. `stop`, then `ps7_init` and `ps7_post_config`
7. write and read back `0x100000` — proves DDR before the download needs it
8. `dow` the ELF, `con`

From the GUI instead, use the run configuration from C.4.

### D.4 Use it

```text
=== HLS seven-segment calculator ===
The result appears on the four-digit display.
Negative results show a leading minus sign;
division by zero shows '----'.

First number (0-99):
```

Enter an operand, a second operand, and an operation (0 add, 1 subtract,
2 multiply, 3 divide). The display updates on the next clock cycle: the
processor writes three registers and then does nothing at all — the scan and
the arithmetic continue in the fabric on their own.

Four cases worth trying, because each exercises a defined behaviour:

| input | display | what it demonstrates |
|-------|---------|----------------------|
| `12 + 34` | `46` | the ordinary path |
| `10 - 25` | `-15` | segment G alone used as a minus sign |
| `99 × 99` | `9801` | the largest result that still fits on four digits |
| `5 ÷ 0` | `----` | division by zero has a defined display, not a crash |

**Checkpoint D:** the terminal prompts, the display changes as you enter
numbers, and all four cases behave as listed.

### D.5 If the terminal is silent but the display works

You enabled both UARTs in B.4, so finding the right one costs a platform
rebuild and no synthesis:

1. Platform editor → `standalone_domain` → **Modify BSP Settings** →
   `stdin`/`stdout` → the other `ps7_uart_*`.
2. **Rebuild the platform**, then rebuild the application.
3. Run again.

Whichever instance produces output is the one the board's USB bridge is wired
to. **Record it in B.4 of this document when you find out**, so nobody has to
discover it a second time.

---

# Troubleshooting

Work from the symptom, not from a guess.

## Stage A — HLS

**`vitis_hls: command not found`**
The environment was not sourced, or was sourced in a different shell than the
one running the command.

**C simulation fails**
Build with `g++` first (A.2) and debug there. Same code, far better errors.

**Vivado cannot find the IP in Stage B**
Almost always the VLNV. Export with `-vendor ces.kit.edu -library hls -version
1.0`, or set those fields in the Export RTL dialog. Verify:
```bash
grep -m1 -A3 "spirit:vendor" .../solution1/impl/ip/component.xml
```

**`export_design` fails**
A solution that was never synthesized, or a stale project directory. Delete the
project directory and rerun.

## Stage B — Vivado

**The IP does not appear in the catalog**
The repository path points one level too high. Use the directory containing
`component.xml` — `.../solution1/impl/ip`, not `.../impl`. Then **Refresh IP
Catalog**.

**`create_bd_cell` fails with an unknown VLNV**
Same cause. The design asks for `ces.kit.edu:hls:seven_segment_axi:1.0`
exactly.

**Critical warnings about `clk`, `led`, `data_in`**
Expected. Those constraints target ports this design does not have, and the
clock comes from the PS rather than a package pin.

**A warning that `seg` or `an` matched no ports**
Not expected, and fatal to the display. The external ports are still named
`seg_0` and `an_0`. Rename them (B.2 step 9) and rebuild.

**Implementation fails timing, negative WNS**
At this design size that indicates something wrong rather than tight. Check
that the HLS clock period and `FCLK_CLK0` agree.

**The XSA contains no bitstream**
`write_hw_platform` needs `-include_bit`, and the Export Hardware wizard needs
*Include bitstream*. Without it the platform builds cleanly and the board is
never programmed.

## Stage C — Vitis

**"Binary File not Found" although the ELF exists**
A workspace copied from another machine, carrying absolute paths. Delete it and
rebuild from the XSA (C.2).

**The wrong workspace is open**
Run the two commands in C.1. If the platform your application targets has no
`XPAR_PS7_UART_*`, you are in a workspace that predates the UART. **File →
Switch Workspace.**

**"Create Application Project" lists no platform**
The platform was never built. An unbuilt platform has no `.xpfm` to point at.

**The build succeeds but the ELF is unchanged**
The wrong node was selected — application versus system project. Check the ELF
timestamp.

**"multiple definition of main"**
The template's own `main.c` is still in `src/` beside `seven_segment_app.c`.

**`XPAR_SEVEN_SEGMENT_AXI_0_...` is undefined**
The block design instance has a different name, or the IP received no driver.
Open the platform editor → `standalone_domain` → **Drivers** and check
`seven_segment_axi_0`.

**`stdin`/`stdout` offers only `none`**
No UART in the hardware. B.4 — this needs a new bitstream and a new XSA, not a
Vitis setting.

**The BSP `stdout` change appears to do nothing**
The platform was not rebuilt afterwards. `xparameters.h` is generated by that
build (C.3.4).

**The launcher complains about a missing FSBL**
C.5. Turn the FSBL option off in Target Setup, or create the platform with boot
components.

**The platform still shows the old address map after a new XSA**
"Update Hardware Specification" left something stale. Delete the workspace and
rerun C.2.

## Stage D — Board

**The display is dark immediately after programming the bitstream**
The most informative failure available, because it rules out everything on the
software side. In order: external ports not renamed to `seg`/`an`; constraints
file never added to the project; `FCLK_CLK0` not enabled; the bitstream not
actually programmed. Check the I/O Ports report — `an[0]` must be on `K19`.

**The display is lit but shows nonsense**
The scan is running, so the fabric and the pins are fine. Either the segment
polarity is inverted, the digit table is wrong, or the operand registers hold
values you did not intend. Since everything resets to zero, look first at what
the software wrote.

**The display flickers**
The scan is too slow. `SEVEN_SEG_REFRESH_TICKS` assumes 50 MHz; a much lower
`FCLK_CLK0` leaves each digit lit too long. Match the clock to
`SEVEN_SEG_CLK_HZ`, or scale the constant.

**Several digits glow faintly, or all four show the same thing**
The anode drive is wrong — more than one `an` bit low at a time, or inverted
polarity. `an` is active low, exactly one digit at a time.

**Nothing on the terminal, but the display works**
The fabric is fine and the problem is entirely the serial path. In order: wrong
tty node (`ls -l /dev/serial/by-id/`), not in `dialout`, `stdout` not set to a
UART in the BSP, the platform not rebuilt after that change, or the wrong UART
instance (D.5). Do not go back to Vivado for anything except B.4.

**The terminal says "connected" and stays blank**
"Connected" only means the device node opened. It says nothing about whether
anything is transmitting. Same list as above.

**Garbage characters**
Baud mismatch. 115200 at both ends.

**`scanf` never returns**
Input is not reaching the UART. Same causes as no output — the path is
bidirectional and fails as a unit.

**`cannot reset APU`, `APB-AP transaction error`, `DAP status ...`**
A system reset dropped the DAP and something touched a target before it came
back. Uncheck "Reset entire system", or use `debug_init.tcl`, which waits. If
it persists, power-cycle the board: a half-awake DAP does not recover on its
own.

**`memory write error at 0x100000`**
That is the first DDR word, where the ELF is loaded. The DDR controller is not
configured — `ps7_init` did not run, or ran and was undone by a later reset.

**`MMU section translation fault`**
The A9 still has its MMU enabled from an earlier run. Bare-metal Xilinx
applications do enable the MMU, and `SCTLR.M` survives `stop`, so every
physical address the debugger uses is translated through page tables that no
longer describe anything. Halting does not clear it; `rst -processor` does, and
so does a power cycle. This is why the first run works and the second does not.

**The application hangs on its first register write**
The PL is not programmed, or holds a different design. An AXI write with no
slave stalls `M_AXI_GP0` and the CPU with it — no exception, no message.
Remember that a system reset erases the fabric. The display gives you an
independent check here: if it is lit, the PL is loaded.

## When nothing makes sense

1. Power-cycle the board. Clears the MMU, the DAP and the fabric at once.
2. Re-open the terminal afterwards — the `ttyUSB` node can move.
3. Program only the bitstream and look at the display (D.2). It halves the
   problem in five seconds.
4. Rebuild the Vitis workspace from the XSA. Under a minute, and eliminates
   every stale path at once.
5. Confirm you are programming the bitstream you believe you are — `ls -la` on
   its timestamp.
