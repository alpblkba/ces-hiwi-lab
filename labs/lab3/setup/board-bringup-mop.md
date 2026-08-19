# Lab 3 — Method of Procedure: from source to a running board

Follow this top to bottom on the machine the board is attached to. Every stage
ends with a **checkpoint**: if the checkpoint does not hold, stop and fix it
there. Carrying a broken stage forward is what turns a ten-minute problem into
a three-hour one, because the failure surfaces somewhere unrelated.

The flow is four stages:

```text
A. Vitis HLS      C++  ->  C simulation  ->  C synthesis  ->  exported AXI IP
B. Vivado         IP   ->  block design  ->  bitstream    ->  XSA
C. Vitis          XSA  ->  platform      ->  application  ->  ELF
D. Board          bitstream + ELF        ->  a terminal you can type into
```

Target: Blackboard, Zynq-7000 `xc7z007sclg400-1`, 100 MHz PL clock,
Vivado / Vitis / Vitis HLS **2022.2**.

> **What is verified and what is not.** The C++ and the golden models have been
> compiled and run, and all testbenches pass. The Tcl scripts and the GUI steps
> below have **not** been executed end to end on a lab machine. Two things in
> particular to confirm as you go, both flagged again at the relevant step: the
> PS UART MIO pins, and the AXI register offsets.

---

## Stage 0 — Before you touch anything

### 0.1 Hardware

- Board powered on, USB cable to the host. The Blackboard's single USB gives
  both JTAG and the UART, as two channels on one FTDI device.
- Boot mode strapped for **JTAG**. If the board is strapped for SD or QSPI, a
  bootloader runs at power-on, leaves the CPU in a state your debugger did not
  create, and register access starts failing for reasons that have nothing to
  do with your design.

### 0.2 Host

```bash
lsusb | grep -i ftdi
ls /dev/ttyUSB*
groups | grep -q dialout && echo "dialout ok" || echo "NOT in dialout group"
```

You should see an FTDI device and normally **two** `/dev/ttyUSB*` nodes. The
first is JTAG, the second is the UART — that second one is the terminal you
will type into in Stage D.

If you are not in `dialout`, the serial port will refuse to open. Fixing it
needs a re-login:

```bash
sudo usermod -aG dialout $USER
```

If no FTDI device appears at all, the Xilinx cable drivers were never
installed:

```bash
sudo /Software/xilinx/2022.2/Vivado/2022.2/data/xicom/cable_drivers/lin64/install_script/install_drivers/install_drivers
```

### 0.3 Tools

Each Xilinx tool has its own `settings64.sh`, and sourcing the wrong one gives
you a shell where `vivado` exists but `vitis_hls` does not. Source only what
the current stage needs, in a fresh shell if you are unsure.

```bash
# Stage A
source /Software/xilinx/2022.2/Vitis_HLS/2022.2/settings64.sh
vitis_hls -version | head -2

# Stage B
source /Software/xilinx/2022.2/Vivado/2022.2/settings64.sh
vivado -version | head -1

# Stage C and D
source /Software/xilinx/2022.2/Vitis/2022.2/settings64.sh
xsct -eval "puts [version]"
```

**All three must read 2022.2.** An XSA written by one release and opened by
another fails in ways that read like file corruption.

### 0.4 The repository

```bash
cd ~/work/ces-hiwi-lab       # or wherever your checkout is
ls labs/lab3/task2/src
```

Expect: `dnn_task2.{h,cpp}`, `dnn_task2_tb.cpp`, `dnn_kernel_axi.{h,cpp}`,
`dnn_kernel_axi_tb.cpp`, `dnn_app.c`.

**Checkpoint 0:** three tools report 2022.2, an FTDI device is visible, two
`ttyUSB` nodes exist, and the sources are present.

---

## Stage A — Vitis HLS: C++ to an exported IP

You are building five IPs, one per pragma variant. They differ only in which
pragmas survive the preprocessor: `-DVARIANT=n` selects them.

| variant | pragmas |
|---------|---------|
| 0 | none — the baseline |
| 1 | `PIPELINE II=1` on the `neuron` loop |
| 2 | `UNROLL` on the `prod` loop |
| 3 | `ARRAY_PARTITION` on `x` and `W` |
| 4 | all three |

### A.1 Sanity check on the host compiler first

Before involving the tool at all. It is the same code, it takes two seconds,
and the error messages are far better than the tool's.

```bash
cd labs/lab3/task2/src
g++ -O2 -std=c++11 -I $XILINX_HLS/include dnn_task2.cpp dnn_task2_tb.cpp -o /tmp/dnn_test
/tmp/dnn_test
```

Expect `all 12 case(s) passed`. Other useful forms:

```bash
/tmp/dnn_test list                    # what the cases are
/tmp/dnn_test relu-edge index-order   # named cases only
/tmp/dnn_test random 42               # pseudo-random input, that seed
```

### A.2 Batch build, all five variants

```bash
cd labs/lab3/task2/hls
vitis_hls -f run_hls_axi.tcl
```

One variant only:

```bash
vitis_hls -f run_hls_axi.tcl -tclargs 2
```

This runs C simulation before every synthesis, synthesizes, exports the IP
under a per-variant name (`dnn_kernel_axi_v0` … `_v4`), and copies each report
next to the script.

### A.3 The GUI path, if you prefer it or are demonstrating it

1. `vitis_hls &`
2. **Create Project** → name it, choose a location outside the repository.
3. **Add Files**: `dnn_kernel_axi.cpp`. **Top Function**: `dnn_kernel_axi`.
4. **Add Testbench**: `dnn_kernel_axi_tb.cpp`.
5. **Part**: `xc7z007sclg400-1`. **Clock period**: `10` ns.
6. Right-click the solution → **Solution Settings** → *Synthesis* and
   *Simulation* → CFLAGS: `-DVARIANT=0` (and the same under Simulation, or the
   testbench and the kernel disagree about which variant they are).
7. **Run C Simulation** → must pass.
8. **Run C Synthesis**.
9. **Export RTL** → format *IP Catalog*, and set the **IP name** to
   `dnn_kernel_axi_v0`. Change both the CFLAG and this name for each variant.

The per-variant IP name is not cosmetic. Five IPs sharing one VLNV means Vivado
picks whichever it cached first, and you get five bitstreams that behave
identically — which looks exactly like "the pragmas do nothing".

### A.4 Read the report

```bash
REPORT=dnn_kernel_axi_V0/sol/syn/report/dnn_kernel_axi_csynth.rpt
grep -A6 "Latency (cycles)" $REPORT | head -8
sed -n '/Utilization Estimates/,/Interface/p' $REPORT | grep -E "^\|(Total|Available|Utilization)"
```

For the kernel-only sweep (Task 3.2's table, no AXI wrapper):

```bash
vitis_hls -f run_hls.tcl
python3 summarise.py
cat results.md
```

**Checkpoint A:** for each variant you want to build,
`dnn_kernel_axi_V<n>/sol/impl/ip/component.xml` exists, and C simulation
passed. Note the reported DSP figure — it is an estimate, and on this kernel it
was out by a factor of two against the implemented design.

---

## Stage B — Vivado: IP to bitstream and XSA

### B.1 Batch

```bash
source /Software/xilinx/2022.2/Vivado/2022.2/settings64.sh
cd ~/work/ces-hiwi-lab
vivado -mode batch -source scripts/build_lab3_vivado.tcl -tclargs 0
```

All five:

```bash
for v in 0 1 2 3 4; do vivado -mode batch -source scripts/build_lab3_vivado.tcl -tclargs $v; done
```

Each run writes to `~/vivado/lab3_dnn_bits/v<n>/` and prints two lines you need:

```text
ADDR_SEG: .../SEG_dnn_kernel_axi_0_Reg -> 0x40000000 size 64K
BUILD_RESULT: OK - variant 0
```

**Write the address down.** Stage C needs it.

### B.2 The GUI path, step by step

1. **Create Project** → RTL Project, *Do not specify sources*, part
   `xc7z007sclg400-1`.
2. **Settings → IP → Repository** → add
   `labs/lab3/task2/hls/dnn_kernel_axi_V0/sol/impl/ip` → **Refresh**. The IP
   should appear in the catalog under *User Repository*.
3. **Create Block Design**, name it `dnn_system`.
4. **Add IP** → `ZYNQ7 Processing System`.
5. **Run Block Automation** → make FIXED_IO and DDR external. Do **not** apply
   a board preset unless a Blackboard board file is installed; without one the
   preset is for a different board and the DDR settings will be wrong.
6. Double-click the PS block and set three things:
   - **PS-PL Configuration** → `M_AXI_GP0` interface **enabled**
   - **Clock Configuration** → `FCLK_CLK0` **enabled**, 100 MHz
   - **MIO Configuration → I/O Peripherals → UART 1** **enabled**, MIO 48–49,
     baud 115200

   The UART step is the one everyone skips. The application reads a seed with
   `scanf` and prints with `printf`; with no PS UART in the design, `printf`
   goes nowhere and `scanf` blocks for ever. There is no error message — the
   board just sits there. **Confirm the MIO pins against the Blackboard's own
   documentation or master XDC**; MIO 48–49 is the usual routing on Zynq boards
   but has not been verified for this one.
7. **Add IP** → `dnn_kernel_axi_v0`.
8. **Run Connection Automation** → select all. This creates the AXI
   interconnect and wires clocks and resets.
9. **Address Editor** tab → confirm the IP has an address. Note it.
10. **Validate Design** (F6). Fix anything it reports before continuing.
11. In *Sources*, right-click the `.bd` → **Create HDL Wrapper** → let Vivado
    manage it.
12. **Generate Bitstream**.
13. **File → Export → Export Hardware** → **Include bitstream** → save the XSA.

### B.3 Check the implemented numbers, not the estimates

After implementation, open the implemented design and look at
**Report Utilization** and **Report Timing Summary**.

- WNS must be **positive**. A negative WNS means the design did not meet
  timing; it may still program and may still appear to work, and it is not
  trustworthy.
- The DSP and LUT figures here are the real ones. They are what belongs in any
  report — the C-synthesis estimate for this kernel was out by roughly 2× on
  DSP and 3× on LUT, in opposite directions.

**Checkpoint B:** `BUILD_RESULT: OK`, a `.bit` and an `.xsa` exist, WNS is
positive, and you have written down the AXI base address.

---
## Stage C — Vitis: XSA to ELF

### C.0 The vocabulary, because the wizards assume you know it

Vitis stacks four things with confusingly similar names. Get these straight
once and the rest of the stage is mechanical.

| Term | What it actually is |
|------|---------------------|
| **Workspace** | A directory. Not a project. Everything below lives inside it. Vitis can only have one open at a time. |
| **Hardware specification / XSA** | The handoff file Vivado wrote in Stage B. Contains the bitstream, `ps7_init.*`, and the address map. This is the *only* thing that has to travel between machines. |
| **Platform project** | Built **from** an XSA. It is the hardware plus the software layer that lets code talk to it. Produces a `.xpfm` descriptor that applications point at. |
| **Domain** | A slot inside a platform: one processor + one OS. Ours is `standalone_domain` = `ps7_cortexa9_0` + `standalone`. A platform can hold several domains; ours holds one. |
| **BSP** (Board Support Package) | Generated inside the domain. The drivers for every IP in the XSA, `xparameters.h`, the C library glue, and the `stdin`/`stdout` setting. When you "modify BSP settings" this is what you are editing. |
| **Application project** | Your actual program — `dnn_app.c` and nothing else. Targets one platform and one domain. Produces the ELF. |
| **System project** | A container that groups the applications meant to run **together** on the board, one per processor. Created automatically, named `<app>_system`. On a single-core standalone design it holds exactly one application and you can mostly ignore it — except that it has its own Build button and its own launch type, which is where people get lost. |

The dependency runs one way:

```text
XSA  ->  platform project  ->  domain (BSP)  ->  application project  ->  ELF
                                                        └─ grouped by ─┘
                                                          system project
```

Change the XSA and everything downstream is stale. That is why the script
deletes and rebuilds the whole workspace rather than updating it in place.

### C.1 Never copy a workspace between machines

A Vitis `.prj` records its platform by **absolute path**, and so does
`_ide/launch/*.launch`. Copy a workspace to another machine and Vitis cannot
resolve the platform, concludes the application was never built, and reports
**"Binary File not Found"** while the ELF sits in `Debug/` exactly where it
should be.

The XSA is portable. The workspace is not. Move the XSA, rebuild the workspace
here.

### C.2 The batch path

```bash
source /Software/xilinx/2022.2/Vitis/2022.2/settings64.sh
cd ~/work/ces-hiwi-lab
xsct scripts/create_vitis_workspace.tcl \
     ~/vivado/lab3_dnn_bits/v0/dnn_system_wrapper_v0.xsa \
     labs/lab3/task2/src \
     ~/vitis/lab3_ws \
     dnn_app
```

It deletes and recreates the workspace every time, on purpose: a reused
workspace is how a stale platform survives an XSA change and the application
silently keeps the old address map.

It also imports only the `.c` files. `labs/lab3/task2/src` holds the HLS kernel
next to the ARM application; a stray `.cpp` in an ARM project pulls in
`ap_int.h` and the build fails on something that is not an error at all, just
the wrong compiler.

The equivalent XSCT commands, if you want to drive it by hand:

```tcl
setws ~/vitis/lab3_ws
platform create -name dnn_app_platform -hw <path>.xsa -proc ps7_cortexa9_0 -os standalone
platform write
platform generate
app create -name dnn_app -platform dnn_app_platform -domain standalone_domain \
           -template {Empty Application(C)}
importsources -name dnn_app -path labs/lab3/task2/src -target-path src
app build -name dnn_app
```

### C.3 The GUI path, click by click

#### C.3.1 Open the workspace

```bash
vitis -workspace ~/vitis/lab3_ws &
```

Without `-workspace` you get a launcher dialog first. Point it at an **empty or
new** directory — pointing it at a workspace built on another machine is the
"Binary File not Found" trap from C.1.

The welcome screen offers *Create Platform Project*, *Create Application
Project*, and *Import Project*. The left-hand **Explorer** panel is the project
tree; the **Assistant** panel (bottom left, or Window → Show View → Assistant)
is where build configurations and Run/Debug entries live. You will use both.

#### C.3.2 Create the platform project

1. **File → New → Platform Project**, or *Create Platform Project* on the
   welcome page.
2. **Platform project name**: `dnn_app_platform`. Next.
3. Choose the tab **Create a new platform from hardware (XSA)**. Browse to
   `~/vivado/lab3_dnn_bits/v0/dnn_system_wrapper_v0.xsa`.
4. **Operating system**: `standalone`. **Processor**: `ps7_cortexa9_0`.
5. **Generate boot components**: uncheck it. It builds an FSBL you will not
   use — the board is strapped for JTAG boot and the debugger loads the ELF
   directly. Leaving it on only costs build time.
6. Finish. The **platform editor** (`platform.spr`) opens.

#### C.3.3 Set the BSP's serial port — do not skip this

In the platform editor's left tree:

```text
dnn_app_platform
└── ps7_cortexa9_0
    └── standalone_domain
        ├── Board Support Package
        └── Drivers
```

1. Click **standalone_domain** → the right panel shows **Board Support
   Package**.
2. Click **Modify BSP Settings…**.
3. In **Overview → standalone**, set **stdin** and **stdout** to
   `ps7_uart_1`.
4. OK.

If the dropdown offers nothing but `none`, there is no UART in the hardware.
Stop — this cannot be fixed in Vitis. Go back to **B.2 step 6**, enable UART1
in the PS configuration, regenerate the bitstream and the XSA, and start Stage
C again.

While you are in this editor, **Drivers** is worth a look: it lists every IP
from the XSA and the driver assigned to it. `dnn_kernel_axi_0` should appear
with a generated driver. If it says `none`, the IP was not recognised and
`xparameters.h` will be missing its address define.

#### C.3.4 Build the platform

Select `dnn_app_platform` in Explorer → **Build Project** (the hammer, or
Ctrl+B, or right-click → Build Project).

This compiles the BSP. It is what produces `xparameters.h` and the IP drivers.
Applications cannot be created against a platform that has not been built.

#### C.3.5 Create the application project

1. **File → New → Application Project**.
2. **Select a platform from repository** → pick `dnn_app_platform`. Next.
3. **Application project name**: `dnn_app`.
   Notice the field below it: **System project name** fills in automatically as
   `dnn_app_system`. That is the container from C.0 — leave it.
4. **Domain**: `standalone_domain` on `ps7_cortexa9_0`. Next.
5. **Template**: **Empty Application (C)**. Not C++ — `dnn_app.c` is C, and the
   C++ template links a heavier runtime for no reason.
6. Finish.

#### C.3.6 Put your source in it

1. If the template left a `main.c` or `helloworld.c` under `dnn_app/src`,
   delete it. Two `main()` functions is a link error that reads like nonsense.
2. Right-click `dnn_app/src` → **Import Sources…** → *File system* → browse to
   `labs/lab3/task2/src` → tick **only** `dnn_app.c` → Finish.

   Do not tick the `.cpp` or `.h` files. They are the HLS kernel; an ARM
   project cannot compile them and does not need them.

#### C.3.7 Build, and know which node you are building

The Explorer tree now looks like this:

```text
dnn_app_platform          <- BSP and hardware
dnn_app_system            <- container
└── dnn_app               <- your program
    ├── src/dnn_app.c
    └── Debug/            <- appears after the first build
```

- Selecting **`dnn_app`** and pressing the hammer builds just the application.
- Selecting **`dnn_app_system`** builds every application in the system.

Either works here because there is only one application. It matters when a
build "succeeds" but the ELF you were expecting did not change — check which
node was selected.

The ELF lands at `~/vitis/lab3_ws/dnn_app/Debug/dnn_app.elf`.

#### C.3.8 What the workspace looks like when it is right

```text
lab3_ws/
├── dnn_app_platform/
│   ├── hw/                     XSA contents: .bit, ps7_init.{c,h,tcl}
│   ├── export/                 the .xpfm applications point at
│   ├── ps7_cortexa9_0/standalone_domain/bsp/
│   │   └── ps7_cortexa9_0/include/xparameters.h
│   └── platform.spr            double-click to reopen the platform editor
├── dnn_app/
│   ├── src/dnn_app.c
│   ├── Debug/dnn_app.elf
│   └── _ide/
│       ├── bitstream/          copy of the .bit, used by "Program FPGA"
│       ├── psinit/ps7_init.tcl
│       └── launch/*.launch     the run configuration, absolute paths and all
└── dnn_app_system/
    └── dnn_app_system.sprj
```

#### C.3.9 Set up the run configuration

**Run → Run Configurations…** In the left list:

- **Single Application Debug** — one ELF on one processor. This is ours.
- **System Project Debug** — every application in a system project, launched
  together. For multi-core designs.
- **Attach to Running Target** — connect to something already running.

Double-click **Single Application Debug** to create a new one. Then, in the
**Target Setup** tab, the four checkboxes that decide whether this works:

| Setting | Set it to | Why |
|---------|-----------|-----|
| Reset entire system | **unchecked** | On Zynq-7000 this asserts `PS_SRST`, which drops the DAP *and* erases the PL. It is the direct cause of `cannot reset APU`. |
| Program FPGA | **checked** | Loads the bitstream from `_ide/bitstream/`. |
| Run ps7_init | **checked** | Configures DDR and the PL clocks. Without it the ELF download to `0x100000` fails. |
| Run ps7_post_config | **checked** | Finishes the PS configuration after the fabric is loaded. |

In the **Application** tab, confirm the ELF path and leave *Stop at 'main'*
unchecked unless you want to single-step.

Apply, then Run.

#### C.3.10 Open the serial terminal

**Window → Show View → Vitis Serial Terminal**, then the **+** in that view.
Port `/dev/ttyUSB1`, baud `115200`, 8 data bits, no parity, 1 stop bit.

Open it **before** you run, so you catch the application's banner. An external
`screen /dev/ttyUSB1 115200` works just as well — but only one program can hold
the port, so close one before opening the other.

#### C.3.11 When the hardware changes

You rebuild a bitstream for the next variant, so you have a new XSA. Two
options:

- **Right-click the platform → Update Hardware Specification**, point at the
  new XSA, rebuild the platform, rebuild the application. Works, and
  occasionally leaves stale generated files behind.
- **Delete the workspace and rerun C.2.** Takes under a minute and cannot leave
  anything stale.

Use the second one. For this lab the hardware only differs by pragmas, the
address map is identical across variants, and the same ELF runs on all five —
so in practice you rebuild the workspace once and only reprogram the bitstream
between variants (Stage D).

Useful menu, worth knowing: **Xilinx → Program Device** programs the FPGA
without launching anything, and **Xilinx → Program Flash** writes to QSPI,
which is not something this lab does.

### C.4 Two things to verify before you run anything

**The base address symbol.** `dnn_app.c` expects
`XPAR_DNN_KERNEL_AXI_0_S_AXI_CTRL_BASEADDR`. The real name comes from the block
design's instance name.

```bash
find ~/vitis/lab3_ws -name xparameters.h -exec grep -i "DNN_KERNEL_AXI" {} +
```

If the name differs, fix the `#define` at the top of `dnn_app.c`. The file
falls back to a hard-coded `0x40000000` and emits a `#warning` if the symbol is
missing — do not ignore that warning; check it against the address Stage B
printed.

**The register offsets.** These are assigned by Vitis HLS from the argument
order, so adding or reordering an argument moves them.

```bash
find labs/lab3/task2/hls/dnn_kernel_axi_V0 -name '*_hw.h' -exec grep ADDR {} +
```

The same header is copied into the BSP, so this also works:

```bash
find ~/vitis/lab3_ws -name 'xdnn_kernel_axi_hw.h' -exec grep ADDR {} +
```

Compare against `dnn_app.c`:

| offset | register |
|--------|----------|
| `0x00` | `ap_ctrl` — bit 0 `ap_start` (W), bit 1 `ap_done` (R, clear-on-read) |
| `0x10` | `seed` |
| `0x18` | `reps` |
| `0x20` | `checksum` (read only) |
| `0x28` | `variant_id` (read only) |

**These offsets are predicted, not confirmed.** They follow the 8-byte stride
of an earlier generated header for this design. Check them once; if they
differ, edit the `R_*` defines in `dnn_app.c` and rebuild.

**Checkpoint C:** `~/vitis/lab3_ws/dnn_app/Debug/dnn_app.elf` exists, the
`xparameters.h` symbol matches, and the register offsets match the generated
header.

---

## Stage D — On the board

### D.1 Open the terminal first

Open it before you program anything, so you see the application's first line.

```bash
screen /dev/ttyUSB1 115200
```

`Ctrl-A` then `k` to quit `screen`. If `screen` is not installed, `minicom -D
/dev/ttyUSB1 -b 115200` or Vitis' own **Serial Terminal** view work equally
well. It is normally `ttyUSB1`, not `ttyUSB0` — the first channel is JTAG.

### D.2 Program and run, without the GUI launcher

```bash
cd ~/work/ces-hiwi-lab
xsct scripts/debug_init.tcl \
     ~/vivado/lab3_dnn_bits/v0/lab3_v0.runs/impl_1/dnn_system_wrapper.bit \
     ~/vitis/lab3_ws/dnn_app_platform/hw/ps7_init.tcl \
     ~/vitis/lab3_ws/dnn_app/Debug/dnn_app.elf
```

Each step prints `ok` or `FAILED` with the reason. The order it uses is not
negotiable, and it is worth understanding because the GUI gets it wrong:

1. connect, JTAG at 10 MHz
2. `rst -system`
3. **wait 3 s** — the reset drops the DAP; touching a target before it returns
   is exactly what produces `cannot reset APU`
4. **program the PL** — the system reset wiped the fabric, so anything you
   loaded in Hardware Manager is gone
5. select the A9, `rst -processor` — clears a leftover MMU
6. `stop`, then `ps7_init` and `ps7_post_config`
7. write and read back `0x100000` — proves DDR before the download needs it
8. `dow` the ELF, `con`

### D.3 Or from the Vitis GUI

**Run → Run Configurations → Single Application Debug**, and change one thing:

- **uncheck "Reset entire system"**
- leave **"Program FPGA"** checked

That single checkbox causes most of Stage D's failures. On Zynq-7000 a system
reset asserts `PS_SRST`, which drops the DAP *and* clears the PL configuration.

### D.4 Use the application

```text
=== Lab 3 - DNN kernel on the PL ===
bitstream variant : 0  (baseline, no pragmas)
base address      : 0x40000000

  1  self-test, the four reference seeds
  2  run one seed you choose
  3  show the operands and the expected output
  4  timing, cycles per layer evaluation
  5  AXI alive check
  q  quit
```

**Start with 5.** It writes a pattern to the `seed` register and reads it back,
which exercises the AXI path and the PL clock without involving the layer. If
that fails, nothing below it means anything.

**Then 1.** Three columns must agree: what the hardware returned, what the ARM
computed itself, and what was recorded the first time this design ran.

| seed | checksum |
|------|----------|
| 1 | `0x00006328` |
| 42 | `0x00050AB8` |
| 1000 | `0x00003E6B` |
| 7 | `0x0000CC88` |

**Then the point of the whole lab.** Program the next bitstream, run the same
application unchanged, type the same seeds. The variant line at the top changes.
The checksums do not.

Option 4 with a large `reps` (100000 is reasonable) gives measured cycles per
evaluation, which is where the pipelined variant visibly trades latency for
area.

**Checkpoint D:** the self-test passes on all five bitstreams, with identical
checksums and different variant lines.

---

# Troubleshooting

Ordered by where the failure appears. Work from the symptom, not from a guess.

## Stage A — HLS

**`vitis_hls: command not found`**
The environment was not sourced, or was sourced in a different shell. Each tool
has its own `settings64.sh`.

**C simulation fails**
Build with `g++` first (A.1) and debug there. Same code, better errors. Run the
named cases to narrow it: `relu-edge`, `index-order`, `int8-extremes`,
`wide-bias` each target a specific class of bug.

**Latency reported as `?`**
A loop bound stopped being a compile-time constant. Check that `N` is a
`#define` and no size arrives as a function argument.

**A pragma appears to do nothing**
Read the log — the tool says when it ignores a directive and why.
```bash
grep -iE "WARNING|ignored|unable|cannot" vitis_hls.log | head
```
At 4×4 some pragmas genuinely do nothing because the tool already applied the
transformation on its own. That is a result, not a fault.

**DSP estimate above 66**
The design does not fit. Vitis HLS reports the estimate and does not stop you;
it fails later, in Vivado. Record it and note the configuration is not
implementable on this device.

**`export_design` fails**
Usually a solution that was never synthesized, or a stale project directory.
Delete the project directory and rerun — the scripts use `-reset` for this
reason.

## Stage B — Vivado

**The IP does not appear in the catalog**
The repository path is wrong, or the catalog was not refreshed. Point at the
directory *containing* `component.xml` — that is `.../sol/impl/ip`, not
`.../impl`. Then **Refresh IP Catalog**.

**`create_bd_cell` fails with an unknown VLNV**
The IP name does not match. Each variant is exported as
`dnn_kernel_axi_v<n>`; the script builds the VLNV from the variant number. If
you exported from the GUI with the default name, either re-export with the
right name or pass the repository path explicitly.

**Five bitstreams that behave identically**
Classic VLNV collision: the variants shared an IP name and Vivado reused a
cached copy. Re-export with per-variant names and delete the Vivado project
directories before rebuilding.

**`validate_bd_design` reports no clock or an unassigned address**
Connection Automation was not run, or not run on everything. Re-run it and
select all interfaces, then check the Address Editor has an entry for the IP.

**Implementation fails timing (negative WNS)**
At this design size that means something is wrong rather than tight — an
unrealistic pragma combination, or a clock constraint that does not match the
PS `FCLK_CLK0` setting. Do not ship a negative-WNS bitstream; it may appear to
work and fail intermittently.

**The XSA has no bitstream in it**
`write_hw_platform` needs `-include_bit`, and the Export Hardware wizard needs
the *Include bitstream* option. An XSA without a bitstream produces a platform
that builds fine and a board that was never programmed.

## Stage C — Vitis

**"Binary File not Found" although the ELF exists**
A copied workspace with absolute paths from another machine. Delete it and
rebuild from the XSA (C.2). This is the single most common Vitis failure in
this lab.

**Platform build fails immediately**
Version mismatch between the XSA's Vivado and this Vitis, or a corrupted
workspace. Check `xsct -eval "puts [version]"` reads 2022.2 and rebuild the
workspace from scratch.

**`xparameters.h` has no `XPAR_DNN_KERNEL_AXI_...` symbol**
The block design instance is named something else. Grep the file for
`BASEADDR`, take the real name, and fix the `#define` in `dnn_app.c`.

**Application builds but `ap_int.h` is not found**
An HLS `.cpp` ended up in the ARM project. Remove everything but the `.c`
files.

**`stdin`/`stdout` cannot be set to a UART**
There is no UART in the hardware design. Back to B.2.6 — this needs a new
bitstream and a new XSA, not a Vitis setting.

**"Create Application Project" cannot see any platform**
The platform was never built. Select it in Explorer and build it first: an
unbuilt platform has no `.xpfm` to point at.

**The application builds but nothing changed in the ELF**
You built the wrong node. `dnn_app` and `dnn_app_system` have separate Build
buttons; check which one was selected in Explorer. Watch the ELF's timestamp:
`ls -la ~/vitis/lab3_ws/dnn_app/Debug/dnn_app.elf`.

**Two `main()` symbols, or "multiple definition of main"**
The Empty Application template's own `main.c` is still in `src/` next to
`dnn_app.c`. Delete the template's file.

**`xparameters.h` not found, or the IP has no driver**
Open the platform editor (`platform.spr`) → `standalone_domain` → **Drivers**.
If `dnn_kernel_axi_0` shows `none`, the IP was not recognised from the XSA.
That usually means the XSA was exported without the block design's IP properly
packaged — go back to Stage B and re-export.

**The platform still has the old address map after a new XSA**
"Update Hardware Specification" left something stale. Delete the workspace and
rerun C.2 — it takes under a minute and this class of bug costs hours.

**Run Configurations has no "Single Application Debug" entry**
It is a category, not an entry. Double-click the category name to create a
configuration under it.

## Stage D — Board

**`cannot reset APU`, `APB-AP transaction error`, `DAP status ...`**
The debugger reached the CPU's debug registers through the APB-AP and the port
refused. Almost always: a system reset dropped the DAP and something touched a
target before it came back. Uncheck "Reset entire system", or use
`debug_init.tcl`, which waits. If it persists, power-cycle the board — a
half-awake DAP does not recover on its own.

**`memory write error at 0x100000`**
`0x100000` is the first DDR word, where the ELF is loaded. Failing there means
the DDR controller is not configured: `ps7_init` did not run, or ran and was
undone by a later reset. Check the ordering — `ps7_init` must come *after* the
last reset.

**`MMU section translation fault`**
The A9 still has its MMU enabled from an earlier run. Bare-metal Xilinx
applications *do* enable the MMU, and `SCTLR.M` survives `stop`, so every
physical address the debugger uses is translated through page tables that no
longer describe anything. Halting does not clear it. `rst -processor` does, and
`debug_init.tcl` issues it. A power cycle also works.

**Everything fails after one successful run**
The same MMU problem. The first run works, the second does not, which makes it
look random. It is not.

**The application hangs on its first register write**
The PL is not programmed, or is programmed with a different design. An AXI
write to an address with no slave stalls `M_AXI_GP0` and the CPU with it — no
exception, no message. Remember that a system reset erases the fabric, so
"I programmed it in Hardware Manager" is not sufficient if a reset happened
afterwards. Run menu option 5 before anything else.

**Nothing appears on the terminal**
In order: wrong `ttyUSB` node (try the other one), wrong baud (115200), no
`dialout` membership, `stdin`/`stdout` not set to `ps7_uart_1` in the BSP, or
no UART in the hardware at all. The last one is the expensive case — it needs a
new bitstream.

**Garbage on the terminal**
Baud mismatch, or the PS clock configuration does not match what the UART
divisor assumed. Confirm 115200 at both ends.

**`scanf` never returns**
Input is not reaching the UART. Same list as "nothing appears", plus: check the
terminal is actually connected to the second FTDI channel and not to the JTAG
one.

**Checksum reads `0x00000000` or `0xFFFFFFFF`**
Not a computation error — those are the signatures of an absent slave. `0` for
a read that never happened, all-ones for a read from an unmapped address. Menu
option 5 and the base address.

**Checksum is wrong but stable**
The hardware and the software golden model disagree. The operand generator in
`dnn_kernel_axi.cpp` and the one in `dnn_app.c` are a contract — if one was
edited without the other, this is what it looks like. Menu option 3 prints the
operands the software believes in.

**All five variants give the same wrong answer**
Suspect the generator contract above, not the pragmas. The variants share the
generator.

**Different variants give different checksums**
This is the one result that would matter. Re-run C simulation for those
variants (`run_hls_axi.tcl` does it automatically) before believing it — a
mismatch that reproduces in simulation is a real bug in the kernel; one that
appears only on the board points at timing (check WNS) or at the wrong
bitstream being loaded.

## When nothing makes sense

In this order:

1. Power-cycle the board. Clears the MMU, the DAP and the fabric at once.
2. Re-open the terminal after the power cycle — the `ttyUSB` node can move.
3. Rebuild the Vitis workspace from the XSA. Cheap, and eliminates every stale
   path.
4. Confirm you are programming the bitstream you think you are, with
   `ls -la` on its timestamp.
5. Fall back to the known-good design at
   `~/archive/lab3-5in1-known-good/` on the build server — one bitstream, all
   five variants behind a `variant` register at offset `0x18`, 11 checks
   passing. It proves the board and the cable are fine, which narrows the
   problem to your build.
