############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
############################################################
open_project seven_segment_hls
set_top seven_segment
add_files seven_segment.cpp
add_files ../../work/ces-hiwi-lab/labs/lab1/task2/src/seven_segment.cpp
add_files -tb seven_segment_tb.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
add_files -tb ../../work/ces-hiwi-lab/labs/lab1/task2/src/seven_segment_tb.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
config_interface -m_axi_latency 0
config_export -format ip_catalog -rtl verilog
source "./seven_segment_hls/solution1/directives.tcl"
csim_design -quiet
