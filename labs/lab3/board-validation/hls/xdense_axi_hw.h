// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of seed
//        bit 31~0 - seed[31:0] (Read/Write)
// 0x14 : reserved
// 0x18 : Data signal of variant
//        bit 2~0 - variant[2:0] (Read/Write)
//        others  - reserved
// 0x1c : reserved
// 0x20 : Data signal of checksum
//        bit 31~0 - checksum[31:0] (Read)
// 0x24 : Control signal of checksum
//        bit 0  - checksum_ap_vld (Read/COR)
//        others - reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XDENSE_AXI_CTRL_ADDR_AP_CTRL       0x00
#define XDENSE_AXI_CTRL_ADDR_GIE           0x04
#define XDENSE_AXI_CTRL_ADDR_IER           0x08
#define XDENSE_AXI_CTRL_ADDR_ISR           0x0c
#define XDENSE_AXI_CTRL_ADDR_SEED_DATA     0x10
#define XDENSE_AXI_CTRL_BITS_SEED_DATA     32
#define XDENSE_AXI_CTRL_ADDR_VARIANT_DATA  0x18
#define XDENSE_AXI_CTRL_BITS_VARIANT_DATA  3
#define XDENSE_AXI_CTRL_ADDR_CHECKSUM_DATA 0x20
#define XDENSE_AXI_CTRL_BITS_CHECKSUM_DATA 32
#define XDENSE_AXI_CTRL_ADDR_CHECKSUM_CTRL 0x24

