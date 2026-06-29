// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#include "xparameters.h"
#include "xseven_segment_axi.h"

extern XSeven_segment_axi_Config XSeven_segment_axi_ConfigTable[];

XSeven_segment_axi_Config *XSeven_segment_axi_LookupConfig(u16 DeviceId) {
	XSeven_segment_axi_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XSEVEN_SEGMENT_AXI_NUM_INSTANCES; Index++) {
		if (XSeven_segment_axi_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XSeven_segment_axi_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XSeven_segment_axi_Initialize(XSeven_segment_axi *InstancePtr, u16 DeviceId) {
	XSeven_segment_axi_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XSeven_segment_axi_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XSeven_segment_axi_CfgInitialize(InstancePtr, ConfigPtr);
}

#endif

