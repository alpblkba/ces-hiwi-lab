set task_dir [file normalize [file join [pwd]]]
set workspace_dir [file normalize "$env(HOME)/vitis/lab1_task2"]

file mkdir $workspace_dir
cd $workspace_dir

open_project seven_segment_hls
set_top seven_segment

add_files $task_dir/src/seven_segment.cpp
add_files -tb $task_dir/src/seven_segment_tb.cpp

open_solution "solution1" -flow_target vivado
set_part {xc7z020clg400-1}
create_clock -period 10 -name default

csim_design
csynth_design
export_design -format ip_catalog

exit
