# ---------------------------------------------------------------------------
# One Lab 3 bitstream: Zynq PS + AXI interconnect + the exported dnn_kernel_axi
# IP for one pragma variant.
#
#   vivado -mode batch -source scripts/build_lab3_vivado.tcl -tclargs 0
#   vivado -mode batch -source scripts/build_lab3_vivado.tcl -tclargs 2 labs/lab3/task2/hls ~/vivado/lab3_dnn_bits
#
# All five, one after the other:
#   for v in 0 1 2 3 4; do
#       vivado -mode batch -source scripts/build_lab3_vivado.tcl -tclargs $v
#   done
#
# Nothing here is machine specific: the two paths default to the usual
# locations and can be overridden on the command line, so the same script runs
# on a lab machine and on a build server.
#
# The design has no board I/O at all - everything goes in and out over
# AXI4-Lite - so there are no external ports and no pin constraints.
# ---------------------------------------------------------------------------

set variant  0

# run_hls_axi.tcl creates its projects in its own directory, so that is where the
# exported IP is. Both paths can be overridden on the command line.
set here     [file dirname [file normalize [info script]]]
set hls_root [file normalize "$here/../labs/lab3/task2/hls"]

# NOT ~/vivado/lab3_dnn - that is the superseded 5-in-1 project, kept as a
# fallback. These are the new per-variant builds.
set out_root [file normalize "~/vivado/lab3_dnn_bits"]

if {[llength $argv] > 0} { set variant  [lindex $argv 0] }
if {[llength $argv] > 1} { set hls_root [file normalize [lindex $argv 1]] }
if {[llength $argv] > 2} { set out_root [file normalize [lindex $argv 2]] }

set part      xc7z007sclg400-1
set ip_name   dnn_kernel_axi_v$variant
set ip_repo   $hls_root/dnn_kernel_axi_V$variant/sol/impl/ip
set proj_dir  $out_root/v$variant
set proj_name lab3_v$variant
set bd_name   dnn_system

if {![file isdirectory $ip_repo]} {
    puts "BUILD_RESULT: FAILED - no exported IP at $ip_repo"
    puts "Run labs/lab3/task2/hls/run_hls_axi.tcl for variant $variant first."
    exit 1
}

puts "variant   $variant"
puts "ip_repo   $ip_repo"
puts "project   $proj_dir"

# Built from scratch each time. The exported IP keeps its VLNV across
# re-exports, and Vivado will otherwise reuse a cached older copy - which shows
# up as a bitstream that behaves like the previous variant.
file delete -force $proj_dir
create_project $proj_name $proj_dir -part $part

set_property ip_repo_paths [list $ip_repo] [current_project]
update_ip_catalog -rebuild

create_bd_design $bd_name

# --- Zynq processing system -------------------------------------------------
set ps [create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0]
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \
    -config {make_external "FIXED_IO, DDR" apply_board_preset "0" \
             Master "Disable" Slave "Disable"} $ps
set_property CONFIG.PCW_FPGA_FCLK0_ENABLE {1} $ps

# --- the HLS accelerator ----------------------------------------------------
create_bd_cell -type ip -vlnv [format "xilinx.com:hls:%s:1.0" $ip_name] dnn_kernel_axi_0

# --- AXI4-Lite: PS master -> HLS slave, clock and reset automatic -----------
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config [list Clk_master {Auto} Clk_slave {Auto} Clk_xbar {Auto} \
                  Master {/processing_system7_0/M_AXI_GP0} \
                  Slave {/dnn_kernel_axi_0/s_axi_CTRL} \
                  ddr_seg {Auto} intc_ip {New AXI Interconnect} master_apm {0}] \
    [get_bd_intf_pins dnn_kernel_axi_0/s_axi_CTRL]

assign_bd_address
regenerate_bd_layout
validate_bd_design
save_bd_design

# The application needs this as its base address. It also goes in
# xparameters.h, but printing it here means a mismatch is visible in the build
# log rather than three steps later as a bus hang.
foreach seg [get_bd_addr_segs -of_objects [get_bd_addr_spaces processing_system7_0/Data]] {
    puts "ADDR_SEG: $seg -> [get_property OFFSET $seg] size [get_property RANGE $seg]"
}

set bd_file [get_files ${bd_name}.bd]
make_wrapper -files $bd_file -top
add_files -norecurse ${proj_dir}/${proj_name}.gen/sources_1/bd/${bd_name}/hdl/${bd_name}_wrapper.v
set_property top ${bd_name}_wrapper [current_fileset]
update_compile_order -fileset sources_1

launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1

if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    puts "BUILD_RESULT: FAILED - variant $variant"
    exit 1
}

# Post-implementation utilisation. These are the numbers worth putting in the
# report: the C-synthesis estimate for this kernel was out by 2x on DSP and
# nearly 3x on LUT, in opposite directions.
open_run impl_1
puts "UTILISATION variant $variant:"
report_utilization -return_string

write_hw_platform -fixed -include_bit -force ${proj_dir}/${bd_name}_wrapper_v${variant}.xsa

puts "BUILD_RESULT: OK - variant $variant"
puts "XSA: ${proj_dir}/${bd_name}_wrapper_v${variant}.xsa"
exit 0
