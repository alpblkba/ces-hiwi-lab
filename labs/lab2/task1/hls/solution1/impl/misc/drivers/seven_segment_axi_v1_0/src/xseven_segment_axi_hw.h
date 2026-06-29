// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// CTRL
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/SC)
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
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        others - reserved
// 0x10 : Data signal of digit
//        bit 3~0 - digit[3:0] (Read/Write)
//        others  - reserved
// 0x14 : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL    0x00
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_GIE        0x04
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER        0x08
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_ISR        0x0c
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_DIGIT_DATA 0x10
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_DIGIT_DATA 4

