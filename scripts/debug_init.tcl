# ---------------------------------------------------------------------------
# debug_init.tcl - bring a Zynq-7000 board up and run an application, without
# the Vitis GUI launch flow.
#
#   xsct scripts/debug_init.tcl [bit] [ps7_init.tcl] [elf]
#
# Written because the GUI launcher fails on the lab machines in three ways that
# are all sequencing problems, not build problems:
#
#   * "cannot reset APU, APB-AP transaction error, DAP status ..."
#     The launch config has "reset entire system" enabled. On Zynq-7000 that
#     asserts PS_SRST, which drops the DAP for a moment. Anything issued before
#     it comes back fails on the DAP, not on the design. Fix: wait after the
#     reset before touching a target.
#
#   * "memory write error at 0x100000 / MMU section translation fault"
#     Two independent causes with the same symptom. (a) 0x100000 is the first
#     DDR word, so a failure there means the DDR controller is not configured -
#     ps7_init did not run, or a system reset undid it. (b) xsdb reaches memory
#     through the selected A9's MMU, and SCTLR.M survives `stop`. If anything
#     ran on the core earlier - a previous ELF, or a bootloader when the board
#     is not strapped for JTAG boot - translation is still enabled and every
#     physical address faults. Halting does not clear it; a core reset does.
#
#   * PS_SRST also clears the PL configuration. A bitstream programmed earlier
#     in the Hardware Manager is gone by the time the ELF runs, and the first
#     write to the accelerator then goes to an unmapped AXI address, stalls
#     M_AXI_GP0 and hangs the CPU. The bitstream must be (re)programmed AFTER
#     the system reset, which is why the order below is not negotiable.
#
# Part is xc7z007sclg400-1 - Zynq-7000, so ps7_init, NOT psu_init (that is
# UltraScale+ MPSoC).
# ---------------------------------------------------------------------------

set here [file dirname [file normalize [info script]]]
set repo [file dirname $here]
set ws   $repo/labs/lab2/task2/vitis

set BIT     [expr {[llength $argv] > 0 ? [lindex $argv 0] : "$ws/seven_segment_platform/hw/seven_segment_wrapper.bit"}]
set PS7INIT [expr {[llength $argv] > 1 ? [lindex $argv 1] : "$ws/seven_segment_platform/hw/ps7_init.tcl"}]
set ELF     [expr {[llength $argv] > 2 ? [lindex $argv 2] : "$ws/seven_segment_app/Debug/seven_segment_app.elf"}]

# First DDR word. The linker script maps ps7_ddr_0 at 0x100000, so this is
# where the ELF download starts and where it fails when DDR is not up.
set DDR_TEST 0x100000

proc step {name body} {
    puts -nonewline "  $name ... "
    flush stdout
    if {[catch {uplevel 1 $body} e]} {
        puts "FAILED"
        puts "    $e"
        return 0
    }
    puts "ok"
    return 1
}

proc die {msg} { puts "\n$msg"; exit 1 }

foreach {label path} [list bitstream $BIT ps7_init $PS7INIT elf $ELF] {
    if {![file exists $path]} { die "$label not found: $path" }
}

puts "bit      $BIT"
puts "ps7_init $PS7INIT"
puts "elf      $ELF"
puts ""

# --- 1. connect --------------------------------------------------------------
step "connect to hw_server" { connect }

# 10 MHz. Higher is tempting but the on-board FTDI plus a long USB cable is
# where marginal JTAG shows up as intermittent DAP errors.
step "jtag frequency 10 MHz" {
    jtag targets -set -filter {name =~ "*DAP*"}
    jtag frequency 10000000
}

# --- 2. system reset ---------------------------------------------------------
# Clears any hung AXI transaction and any leftover PS state. It also drops the
# DAP and wipes the PL, both of which the next two steps account for.
if {![step "select APU" { targets -set -filter {name =~ "APU*"} }]} {
    die "No APU target. The DAP is not responding: check the USB cable, the\
board power switch, and that no other tool (Hardware Manager, another xsct)\
holds the cable."
}

step "system reset" { rst -system }

# The DAP needs time to come back. Resetting the APU before it does is exactly
# what produces "cannot reset APU / APB-AP transaction error".
step "wait for the DAP" { after 3000 }

# --- 3. program the PL -------------------------------------------------------
# After the reset the fabric is blank, whatever the Hardware Manager did
# earlier. Program it before any code can touch the accelerator.
if {![step "program PL" {
        targets -set -filter {name =~ "xc7z*"}
        fpga -file $BIT
    }]} {
    die "Programming the PL failed. If the bitstream is for a different part,\
this is where it shows."
}

# --- 4. core reset, then PS configuration ------------------------------------
if {![step "select Cortex-A9 #0" { targets -set -filter {name =~ "*A9*#0"} }]} {
    die "APU is present but no A9 core context: the DAP is only half awake.\
Power-cycle the board and run again."
}

# Clears SCTLR.M. Without this, a core left with the MMU enabled by an earlier
# run makes every physical address below fault, and `stop` alone does not undo
# it. Narrower than `rst -system`: the DAP and the JTAG chain are untouched.
step "reset the core (clears the MMU)" {
    if {[catch {rst -processor} e]} { puts -nonewline "(rst -processor unavailable: $e) " }
}

# ps7_init writes PS configuration registers through the DAP, and the DAP
# refuses register access on a running core. Already-halted is the normal case.
step "halt the core" {
    if {[catch {stop} e] && ![string match "*Already stopped*" $e]} { error $e }
}

# Route xsdb's memory accesses through the DAP's own path rather than the
# core's, so a stale translation cannot break the download either.
step "force-mem-access on" { configparams force-mem-access 1 }

if {![step "ps7_init + ps7_post_config" {
        source $PS7INIT
        ps7_init
        ps7_post_config
    }]} {
    die "PS initialisation failed. Everything below depends on it - the DDR\
controller and the PL clocks are configured here."
}

# --- 5. is DDR actually there? ----------------------------------------------
# Do this before the download, so a DDR failure is reported as a DDR failure
# instead of as "memory write error at 0x100000" in the middle of `dow`.
if {![step "DDR read/write at $DDR_TEST" {
        mwr $DDR_TEST 0xDEADBEEF
        set got [lindex [mrd -value $DDR_TEST] 0]
        if {$got != 0xDEADBEEF} {
            error [format "wrote 0xDEADBEEF, read back 0x%08X" $got]
        }
        mwr $DDR_TEST 0x00000000
    }]} {
    die "DDR is not usable. ps7_init reported success, so suspect the board's\
boot-mode straps (JP2) or a PS7 preset that does not match this board."
}

# --- 6. download and run -----------------------------------------------------
if {![step "download ELF" { dow $ELF }]} {
    die "Download failed even though DDR is writable. Check that the ELF was\
linked against this platform - a stale lscript.ld from another design is the\
usual cause."
}

step "force-mem-access off" { configparams force-mem-access 0 }
step "run" { con }

puts "\nRunning. Open the UART terminal (115200 8N1) to interact with the application."
