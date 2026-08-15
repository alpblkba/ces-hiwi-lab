# Task 3.1 - the unoptimised baseline.
#
#   cd labs/lab3/task1/hls && vitis_hls -f run_hls.tcl
#
# C simulation runs first on purpose. A synthesis figure for a kernel that does
# not compute the right thing is not a baseline, it is a number.

open_project -reset dnn_task1_prj
set_top dnn_kernel
add_files ../src/dnn_task1.cpp
add_files -tb ../src/dnn_task1_tb.cpp

open_solution -reset "baseline" -flow_target vivado
set_part {xc7z007sclg400-1}
create_clock -period 10 -name default

# Vitis HLS optimises even with no pragmas in the source: at 4x4 it unrolls the
# inner loops on its own. That is left ON here, because these are the numbers
# the board actually produces and Task 3.2 is measured against them.
#
# Uncomment to see the kernel with the tool's own optimisation switched off.
# The difference is itself worth looking at once.
#   config_compile -pipeline_loops 0

csim_design
csynth_design
exit
