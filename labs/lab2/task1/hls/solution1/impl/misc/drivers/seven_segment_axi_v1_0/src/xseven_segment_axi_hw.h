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
// 0x10 : Data signal of op1
//        bit 6~0 - op1[6:0] (Read/Write)
//        others  - reserved
// 0x14 : reserved
// 0x18 : Data signal of op2
//        bit 6~0 - op2[6:0] (Read/Write)
//        others  - reserved
// 0x1c : reserved
// 0x20 : Data signal of op_sel
//        bit 1~0 - op_sel[1:0] (Read/Write)
//        others  - reserved
// 0x24 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL     0x00
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_GIE         0x04
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER         0x08
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_ISR         0x0c
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP1_DATA    0x10
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_OP1_DATA    7
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP2_DATA    0x18
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_OP2_DATA    7
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP_SEL_DATA 0x20
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_OP_SEL_DATA 2

