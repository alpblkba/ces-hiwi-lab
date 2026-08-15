# ---------------------------------------------------------------------------
# Build a Vitis workspace from an XSA, from the command line.
#
#   xsct scripts/create_vitis_workspace.tcl <xsa> <src-dir> [workspace] [app]
#
#   xsct scripts/create_vitis_workspace.tcl \
#        ~/vivado/lab3_dnn_bits/v0/dnn_system_wrapper_v0.xsa \
#        labs/lab3/task2/src \
#        ~/vitis/lab3_ws  dnn_app
#
# Why this exists, and why a workspace should never be copied between machines:
#
# A Vitis .prj records the platform by ABSOLUTE path, and the generated
# _ide/launch/*.launch does the same. Copy that workspace to another machine
# and Vitis cannot resolve the platform, decides the application was never
# built, and reports "Binary File not Found" while the ELF sits in Debug/ where
# it always was. The XSA itself is portable; the workspace around it is not.
#
# So: move the XSA, rebuild the workspace here. It takes seconds.
#
# The XSA and the Vitis install have to be the same release (2022.2 for this
# lab). A newer XSA in an older Vitis fails in ways that read like corruption.
# ---------------------------------------------------------------------------

if {[llength $argv] < 2} {
    puts "usage: xsct create_vitis_workspace.tcl <xsa> <src-dir> \[workspace\] \[app-name\]"
    exit 1
}

set xsa  [file normalize [lindex $argv 0]]
set src  [file normalize [lindex $argv 1]]
set ws   [expr {[llength $argv] > 2 ? [file normalize [lindex $argv 2]] : [file normalize "~/vitis/lab3_ws"]}]
set app  [expr {[llength $argv] > 3 ? [lindex $argv 3] : "dnn_app"}]
set plat "${app}_platform"

foreach {label path} [list XSA $xsa sources $src] {
    if {![file exists $path]} {
        puts "ERROR: $label not found: $path"
        exit 1
    }
}

puts "xsa        $xsa"
puts "sources    $src"
puts "workspace  $ws"
puts "app        $app"
puts ""

# A fresh workspace every time. Reusing one is how a stale platform survives an
# XSA change and the application silently keeps the old address map.
file delete -force $ws
file mkdir $ws
setws $ws

puts "--- platform ---"
platform create -name $plat -hw $xsa -proc ps7_cortexa9_0 -os standalone -out $ws
platform write
platform generate

puts "--- application ---"
app create -name $app -platform $plat -domain standalone_domain \
           -template {Empty Application(C)}

# Vitis creates its own empty main.c; the imported sources replace it.
file delete -force $ws/$app/src
importsources -name $app -path $src -target-path src

# The source directory holds the HLS kernel next to the ARM application. Only
# the .c files belong in an ARM project: a stray .cpp pulls in ap_int.h and the
# build fails on something that is not an error at all, just the wrong compiler.
foreach stray [glob -nocomplain $ws/$app/src/*] {
    if {[file extension $stray] ne ".c"} {
        puts "  (skipping [file tail $stray] - not part of the ARM build)"
        file delete -force $stray
    }
}

app build -name $app

set elf $ws/$app/Debug/$app.elf
if {![file exists $elf]} {
    puts "\nBUILD_RESULT: FAILED - no ELF at $elf"
    exit 1
}

puts "\nBUILD_RESULT: OK"
puts "elf  $elf"
puts ""
puts "Run it without the GUI launcher:"
puts "  xsct scripts/debug_init.tcl <bit> <ps7_init.tcl> $elf"
exit 0
