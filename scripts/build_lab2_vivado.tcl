# ---------------------------------------------------------------------------
# Rebuilds the Lab 2.1 Vivado project from scratch: Zynq PS + AXI interconnect
# + the exported HLS calculator IP, with seg/an brought out to the board pins.
#
# Built from scratch on purpose. The HLS IP keeps the same VLNV
# (ces.kit.edu:hls:seven_segment_axi:1.0) across re-exports, so Vivado will
# silently reuse a cached copy of the OLD IP if an existing project is simply
# refreshed. A fresh project guarantees the current IP is used.
#
#   vivado -mode batch -source scripts/build_lab2_vivado.tcl
# ---------------------------------------------------------------------------

set proj_dir  /home/urelg/vivado/lab2_calc
set proj_name lab2_task1
set part      xc7z007sclg400-1
set ip_repo   /home/urelg/vitis/lab2_task1/lab2_task1_hls/solution1/impl/ip
set xdc       /home/urelg/vivado/lab2/lab2_task1.srcs/constrs_1/new/session2_task1.xdc
set bd_name   seven_segment

file delete -force $proj_dir
create_project $proj_name $proj_dir -part $part

# Point at the freshly exported HLS IP and make it visible in the catalog.
set_property ip_repo_paths [list $ip_repo] [current_project]
update_ip_catalog -rebuild

add_files -fileset constrs_1 -norecurse $xdc

create_bd_design $bd_name

# --- Zynq processing system -------------------------------------------------
set ps [create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0]
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \
    -config {make_external "FIXED_IO, DDR" apply_board_preset "0" \
             Master "Disable" Slave "Disable"} $ps
set_property CONFIG.PCW_FPGA_FCLK0_ENABLE {1} $ps

# --- HLS calculator IP ------------------------------------------------------
create_bd_cell -type ip -vlnv ces.kit.edu:hls:seven_segment_axi:1.0 seven_segment_axi_0

# --- AXI4-Lite: PS master -> HLS slave, clocks and resets automatic ---------
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config [list Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} \
                  Master {/processing_system7_0/M_AXI_GP0} \
                  Slave {/seven_segment_axi_0/s_axi_CTRL} \
                  ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}] \
    [get_bd_intf_pins seven_segment_axi_0/s_axi_CTRL]

# --- Display outputs to real pins ------------------------------------------
# The port names must be exactly seg and an so the XDC constraints apply.
make_bd_pins_external -name seg [get_bd_pins seven_segment_axi_0/seg]
make_bd_pins_external -name an  [get_bd_pins seven_segment_axi_0/an]

assign_bd_address
regenerate_bd_layout
validate_bd_design
save_bd_design

# --- Wrapper and build ------------------------------------------------------
set bd_file [get_files ${bd_name}.bd]
make_wrapper -files $bd_file -top
add_files -norecurse ${proj_dir}/${proj_name}.gen/sources_1/bd/${bd_name}/hdl/${bd_name}_wrapper.v
set_property top ${bd_name}_wrapper [current_fileset]
update_compile_order -fileset sources_1

launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1

if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    puts "BUILD_RESULT: FAILED"
    exit 1
}

# --- Hardware handoff for Vitis --------------------------------------------
write_hw_platform -fixed -include_bit -force ${proj_dir}/${bd_name}_wrapper.xsa

puts "BUILD_RESULT: OK"
puts "XSA: ${proj_dir}/${bd_name}_wrapper.xsa"
exit 0
