# ---------------------------------------------------------------------------
# SUPERSEDED. Builds the old 5-in-1 design: one bitstream containing all five
# pragma variants behind a `variant` register.
#
# The lab now builds one bitstream per variant - use build_lab3_vivado.tcl.
# This is kept only to rebuild the fallback that is known to pass on hardware;
# the bitstream and XSA it produced are already saved at
# ~/archive/lab3-5in1-known-good/ and need no rebuild to be used.
#
# The project name inside stays lab3_dense: renaming it would mean renaming the
# whole .cache/.gen/.runs tree, which is not a risk worth taking on a fallback.
# ---------------------------------------------------------------------------
set proj_dir  /home/urelg/vivado/lab3_dnn
set part      xc7z007sclg400-1
file delete -force $proj_dir
create_project lab3_dense $proj_dir -part $part
set_property ip_repo_paths [list /home/urelg/vitis/lab3_dnn_axi/dense_axi_prj/solution1/impl/ip] [current_project]
update_ip_catalog -rebuild
create_bd_design dense_system
set ps [create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0]
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \
    -config {make_external "FIXED_IO, DDR" apply_board_preset "0" Master "Disable" Slave "Disable"} $ps
set_property CONFIG.PCW_FPGA_FCLK0_ENABLE {1} $ps
create_bd_cell -type ip -vlnv xilinx.com:hls:dense_axi:1.0 dense_axi_0
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config [list Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} \
                  Master {/processing_system7_0/M_AXI_GP0} Slave {/dense_axi_0/s_axi_CTRL} \
                  ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}] \
    [get_bd_intf_pins dense_axi_0/s_axi_CTRL]
assign_bd_address
validate_bd_design
save_bd_design
foreach seg [get_bd_addr_segs -of_objects [get_bd_addr_spaces processing_system7_0/Data]] {
    puts "ADDR_SEG: $seg -> [get_property OFFSET $seg]"
}
make_wrapper -files [get_files dense_system.bd] -top
add_files -norecurse ${proj_dir}/lab3_dense.gen/sources_1/bd/dense_system/hdl/dense_system_wrapper.v
set_property top dense_system_wrapper [current_fileset]
update_compile_order -fileset sources_1
launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] != "100%"} { puts "BUILD_RESULT: FAILED"; exit 1 }
write_hw_platform -fixed -include_bit -force ${proj_dir}/dense_system_wrapper.xsa
puts "BUILD_RESULT: OK"
exit 0
