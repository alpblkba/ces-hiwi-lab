# Task 3.2 - synthesize all five pragma variants.
#
#   cd labs/lab3/task2/hls && vitis_hls -f run_hls.tcl
#   vitis_hls -f run_hls.tcl -tclargs 2        # just one variant
#
# All five come from ONE source file. dnn_task2.cpp guards each pragma with
# #if VARIANT == n, and the -cflags below pick which ones survive the
# preprocessor. Five copies of the same code would drift apart within a week.
#
# Each variant gets its own solution, so all five reports sit side by side in
# dnn_task2_prj/ and the copies land here as V0n_csynth.rpt.

set variants {0 1 2 3 4}
if {$argc > 0} { set variants $argv }

foreach v $variants {
    puts "=========== VARIANT $v ==========="

    open_project -reset dnn_task2_prj_V$v
    set_top dnn_kernel
    add_files ../src/dnn_task2.cpp -cflags "-DVARIANT=$v"
    add_files -tb ../src/dnn_task2_tb.cpp -cflags "-DVARIANT=$v"

    open_solution -reset "V$v" -flow_target vivado
    set_part {xc7z007sclg400-1}
    create_clock -period 10 -name default

    # C simulation before every synthesis. The claim being tested is that the
    # pragmas do not change the result, so proving it once is not enough - it
    # has to hold for the variant actually being built.
    csim_design
    csynth_design

    file copy -force \
        dnn_task2_prj_V$v/V$v/syn/report/dnn_kernel_csynth.rpt \
        [file join [pwd] V0${v}_csynth.rpt]
}

puts "\nreports written to [pwd]/V0n_csynth.rpt"
puts "run  python3 summarise.py  to turn them into results.md"
exit
