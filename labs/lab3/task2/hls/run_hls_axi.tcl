# Task 3.2, board half - export one AXI IP per pragma variant.
#
#   cd labs/lab3/task2/hls && vitis_hls -f run_hls_axi.tcl
#   vitis_hls -f run_hls_axi.tcl -tclargs 0 4        # only these variants
#
# Each variant is exported under its own IP name. Without that the five builds
# would share a VLNV and Vivado would silently reuse whichever copy it cached
# first - a failure that looks like "the pragmas do nothing on the board".
#
# C simulation runs before every export. The testbench checks the checksums the
# ARM application expects, so a mismatch is caught here rather than after a
# forty-minute implementation run.

set variants {0 1 2 3 4}
if {$argc > 0} { set variants $argv }

foreach v $variants {
    puts "=========== VARIANT $v ==========="

    open_project -reset dnn_kernel_axi_V$v
    set_top dnn_kernel_axi
    add_files    ../src/dnn_kernel_axi.cpp    -cflags "-DVARIANT=$v"
    add_files -tb ../src/dnn_kernel_axi_tb.cpp -cflags "-DVARIANT=$v"

    open_solution -reset "sol" -flow_target vivado
    set_part {xc7z007sclg400-1}
    create_clock -period 10 -name default

    csim_design
    csynth_design
    export_design -format ip_catalog -ipname dnn_kernel_axi_v$v

    file copy -force \
        dnn_kernel_axi_V$v/sol/syn/report/dnn_kernel_axi_csynth.rpt \
        [file join [pwd] V0${v}_axi_csynth.rpt]
}

puts "\nIP repositories:"
foreach v $variants {
    puts "  variant $v  [pwd]/dnn_kernel_axi_V$v/sol/impl/ip"
}
puts "\nFeed each one to scripts/build_lab3_vivado.tcl to get its bitstream."
exit
