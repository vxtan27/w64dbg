// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include <ntdll.h>

// Get process ID from debug event
FORCEINLINE DWORD DbgGetProcessId(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    return HandleToUlong(pStateChange->AppClientId.UniqueProcess);
}

// Get thread ID from debug event
FORCEINLINE DWORD DbgGetThreadId(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    return HandleToUlong(pStateChange->AppClientId.UniqueThread);
}

#ifndef DBGAPI
#ifdef _DEBUG
#define DBGAPI
#else
#define DBGAPI FORCEINLINE
#endif
#endif

// Attach debugger to an active process
DBGAPI NTSTATUS DbgDebugActiveProcess(
    HANDLE hProcess,
    ULONG uFlags
);

// Wait for a debug state change event
DBGAPI NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
);

// Continue a thread after a debug event
DBGAPI NTSTATUS DbgContinue(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    NTSTATUS dwContinueStatus
);

// Stop debugging a process
DBGAPI NTSTATUS DbgStopDebugging(
    HANDLE hProcess,
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);

// Get process handle from a debug event
DBGAPI HANDLE DbgGetProcessHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);

// Get thread handle from a debug event
DBGAPI HANDLE DbgGetThreadHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
);