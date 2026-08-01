// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
// CTRL
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
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

#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP1_DATA    0x10
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_OP1_DATA    7
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP2_DATA    0x18
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_OP2_DATA    7
#define XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP_SEL_DATA 0x20
#define XSEVEN_SEGMENT_AXI_CTRL_BITS_OP_SEL_DATA 2

