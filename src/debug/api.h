// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"

// Debug data structure
typedef struct _WDBGSS_DATA {
    struct _WDBGSS_DATA *Next; // Next in linked list
    HANDLE Handle;             // Thread/process handle
    DWORD ThreadId;            // Thread ID
} WDBGSS_DATA, *PWDBGSS_DATA;

// Set/Get debug data from the TEB
#define DbgSsSetThreadData(d) NtCurrentTeb()->DbgSsReserved[0] = d
#define DbgSsGetThreadData() ((PWDBGSS_DATA) NtCurrentTeb()->DbgSsReserved[0])

// Return the debug object handle from the TEB
#define DbgUiGetThreadDebugObject() (NtCurrentTeb()->DbgSsReserved[1])

// Call the kernel to create / remove the debug object
#define DbgUiConnectToDbg() NtCreateDebugObject(&DbgUiGetThreadDebugObject(), DEBUG_ALL_ACCESS, NULL, DebugObjectKillProcessOnExitInformation)
#define DbgUiStopDebugging(hProcess) NtRemoveProcessDebug(hProcess, DbgUiGetThreadDebugObject())

// Enable a debugger to attach to an active process and debug it
NTSTATUS
WDbgDebugActiveProcess(
    HANDLE hProcess
);

// Wait for a debugging event to occur in a process being debugged
NTSTATUS
DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
);

// Get PID from current debug event
FORCEINLINE
DWORD
DbgGetProcessId(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);

// Get TID from current debug event
FORCEINLINE
DWORD
DbgGetThreadId(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);

// Continue a thread after handling a debug event
NTSTATUS
WDbgContinueDebugEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    NTSTATUS dwContinueStatus
);

// Get process handle from current debug event
HANDLE
WDbgGetProcessHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);

// Get thread handle from current debug event
HANDLE
WDbgGetThreadHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);

// Stop the debugger from debugging the specified process
NTSTATUS
WDbgDebugActiveProcessStop(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE hProcess
);