// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
/***************************** Include Files *********************************/
#include "xseven_segment_axi.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XSeven_segment_axi_CfgInitialize(XSeven_segment_axi *InstancePtr, XSeven_segment_axi_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XSeven_segment_axi_Set_op1(XSeven_segment_axi *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP1_DATA, Data);
}

u32 XSeven_segment_axi_Get_op1(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP1_DATA);
    return Data;
}

void XSeven_segment_axi_Set_op2(XSeven_segment_axi *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP2_DATA, Data);
}

u32 XSeven_segment_axi_Get_op2(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP2_DATA);
    return Data;
}

void XSeven_segment_axi_Set_op_sel(XSeven_segment_axi *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP_SEL_DATA, Data);
}

u32 XSeven_segment_axi_Get_op_sel(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_OP_SEL_DATA);
    return Data;
}

