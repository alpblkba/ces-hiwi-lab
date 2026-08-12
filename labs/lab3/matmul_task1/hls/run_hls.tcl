open_project -reset p
set_top matmul
add_files matmul.cpp
open_solution -reset "s" -flow_target vivado
set_part {xc7z007sclg400-1}
create_clock -period 10 -name default
# disable the automatic loop pipelining that Vitis HLS applies by default
config_compile -pipeline_loops 0
csynth_design
exit
