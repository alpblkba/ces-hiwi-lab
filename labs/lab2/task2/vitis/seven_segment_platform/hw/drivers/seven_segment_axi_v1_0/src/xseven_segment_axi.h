// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2022.2 (64-bit)
// Tool Version Limit: 2019.12
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// ==============================================================
#ifndef XSEVEN_SEGMENT_AXI_H
#define XSEVEN_SEGMENT_AXI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xseven_segment_axi_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
    u16 DeviceId;
    u64 Ctrl_BaseAddress;
} XSeven_segment_axi_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XSeven_segment_axi;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XSeven_segment_axi_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XSeven_segment_axi_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XSeven_segment_axi_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XSeven_segment_axi_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XSeven_segment_axi_Initialize(XSeven_segment_axi *InstancePtr, u16 DeviceId);
XSeven_segment_axi_Config* XSeven_segment_axi_LookupConfig(u16 DeviceId);
int XSeven_segment_axi_CfgInitialize(XSeven_segment_axi *InstancePtr, XSeven_segment_axi_Config *ConfigPtr);
#else
int XSeven_segment_axi_Initialize(XSeven_segment_axi *InstancePtr, const char* InstanceName);
int XSeven_segment_axi_Release(XSeven_segment_axi *InstancePtr);
#endif

void XSeven_segment_axi_Start(XSeven_segment_axi *InstancePtr);
u32 XSeven_segment_axi_IsDone(XSeven_segment_axi *InstancePtr);
u32 XSeven_segment_axi_IsIdle(XSeven_segment_axi *InstancePtr);
u32 XSeven_segment_axi_IsReady(XSeven_segment_axi *InstancePtr);
void XSeven_segment_axi_EnableAutoRestart(XSeven_segment_axi *InstancePtr);
void XSeven_segment_axi_DisableAutoRestart(XSeven_segment_axi *InstancePtr);

void XSeven_segment_axi_Set_digit(XSeven_segment_axi *InstancePtr, u32 Data);
u32 XSeven_segment_axi_Get_digit(XSeven_segment_axi *InstancePtr);

void XSeven_segment_axi_InterruptGlobalEnable(XSeven_segment_axi *InstancePtr);
void XSeven_segment_axi_InterruptGlobalDisable(XSeven_segment_axi *InstancePtr);
void XSeven_segment_axi_InterruptEnable(XSeven_segment_axi *InstancePtr, u32 Mask);
void XSeven_segment_axi_InterruptDisable(XSeven_segment_axi *InstancePtr, u32 Mask);
void XSeven_segment_axi_InterruptClear(XSeven_segment_axi *InstancePtr, u32 Mask);
u32 XSeven_segment_axi_InterruptGetEnabled(XSeven_segment_axi *InstancePtr);
u32 XSeven_segment_axi_InterruptGetStatus(XSeven_segment_axi *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
