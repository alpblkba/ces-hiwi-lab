open_project -reset dense_axi_prj
set_top dense_axi
add_files src/dense_axi.cpp
add_files -tb src/dense_axi_tb.cpp
open_solution -reset "solution1" -flow_target vivado
set_part {xc7z007sclg400-1}
create_clock -period 10 -name default
csim_design
csynth_design
export_design -format ip_catalog
exit
