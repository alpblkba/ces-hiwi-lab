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

void XSeven_segment_axi_Start(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL) & 0x80;
    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XSeven_segment_axi_IsDone(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XSeven_segment_axi_IsIdle(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XSeven_segment_axi_IsReady(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XSeven_segment_axi_EnableAutoRestart(XSeven_segment_axi *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL, 0x80);
}

void XSeven_segment_axi_DisableAutoRestart(XSeven_segment_axi *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_AP_CTRL, 0);
}

void XSeven_segment_axi_Set_digit(XSeven_segment_axi *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_DIGIT_DATA, Data);
}

u32 XSeven_segment_axi_Get_digit(XSeven_segment_axi *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_DIGIT_DATA);
    return Data;
}

void XSeven_segment_axi_InterruptGlobalEnable(XSeven_segment_axi *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_GIE, 1);
}

void XSeven_segment_axi_InterruptGlobalDisable(XSeven_segment_axi *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_GIE, 0);
}

void XSeven_segment_axi_InterruptEnable(XSeven_segment_axi *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER);
    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER, Register | Mask);
}

void XSeven_segment_axi_InterruptDisable(XSeven_segment_axi *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER);
    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER, Register & (~Mask));
}

void XSeven_segment_axi_InterruptClear(XSeven_segment_axi *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XSeven_segment_axi_WriteReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_ISR, Mask);
}

u32 XSeven_segment_axi_InterruptGetEnabled(XSeven_segment_axi *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_IER);
}

u32 XSeven_segment_axi_InterruptGetStatus(XSeven_segment_axi *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XSeven_segment_axi_ReadReg(InstancePtr->Ctrl_BaseAddress, XSEVEN_SEGMENT_AXI_CTRL_ADDR_ISR);
}

