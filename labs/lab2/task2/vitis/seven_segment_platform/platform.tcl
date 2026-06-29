# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct /home/urelg/vitis/lab2_task2/seven_segment_platform/platform.tcl
# 
# OR launch xsct and run below command.
# source /home/urelg/vitis/lab2_task2/seven_segment_platform/platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {seven_segment_platform}\
-hw {/home/urelg/vivado/lab2/seven_segment_wrapper.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {/home/urelg/vitis/lab2_task2}

platform write
platform generate -domains 
platform active {seven_segment_platform}
platform generate
