############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
############################################################
open_project lab2_task1_hls
set_top seven_segment_axi
add_files lab2_task1_hls/seven_segment_axi.cpp
add_files lab2_task1_hls/seven_segment_axi.h
add_files -tb lab2_task1_hls/seven_segment_axi_tb.cpp -cflags "-Wno-unknown-pragmas -Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xc7z007s-clg400-1}
create_clock -period 10 -name default
config_export -description {AXI4-Lite controlled seven-segment display HLS IP} -display_name seven_segment_axi -format ip_catalog -library hls -rtl verilog -taxonomy /UserIP -vendor ces.kit.edu -version 1.0
source "./lab2_task1_hls/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -rtl verilog -format ip_catalog
