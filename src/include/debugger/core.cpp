// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "api.h"

// Set/Get per-thread debug data
#define DbgSsSetThreadData(d) NtCurrentTeb()->DbgSsReserved[0] = d
#define DbgSsGetThreadData() ((PDBGSS_THREAD_DATA) NtCurrentTeb()->DbgSsReserved[0])

// Get thread's debug object handle
#define DbgGetThreadDebugObject() (NtCurrentTeb()->DbgSsReserved[1])

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244 4715 28182)
#endif

#ifdef DBG_DEBUG_SINGLE_PROCESS
#include "single.cpp"
#else
#include "multiple.cpp"
#endif

// Wait for a debug event, retrying if alert/APC interrupts
DBGAPI NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
) {
    NTSTATUS NtStatus;
    if (bAlertable) {
        do {
            NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(), bAlertable, pTimeout, pStateChange);
        } while (NtStatus == STATUS_ALERTED || NtStatus == STATUS_USER_APC);
    } else {
        NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(), bAlertable, pTimeout, pStateChange);
    }
    return NtStatus;
}

// Continue a thread after handling a debug event
DBGAPI NTSTATUS DbgContinue(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    NTSTATUS dwContinueStatus
) {
    NTSTATUS NtStatus = NtDebugContinue(DbgGetThreadDebugObject(),
        &pStateChange->AppClientId, dwContinueStatus);

    // Process state-specific actions
    switch (pStateChange->NewState) {
    case DbgCreateThreadStateChange:
        DbgSaveThreadHandle(pStateChange); // New thread
        break;
    case DbgExitThreadStateChange:
        DbgFreeThreadHandle(pStateChange); // Thread exit
        break;
    case DbgCreateProcessStateChange:
        DbgSaveProcessHandle(pStateChange); // New process
        break;
    case DbgExitProcessStateChange:
        DbgCloseAllProcessHandles(pStateChange); // Process exit
        break;
    case DbgLoadDllStateChange:
    case DbgUnloadDllStateChange:
    case DbgExceptionStateChange:
    case DbgBreakpointStateChange:
    case DbgSingleStepStateChange:
        break;
    case DbgIdle:
    case DbgReplyPending:
        break;
    default:
        // Unhandled state, no action
        break;
    }
    return NtStatus;
}

// Stop debugging a process and clean up handles
DBGAPI NTSTATUS DbgStopDebugging(
    HANDLE hProcess,
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    DbgCloseAllProcessHandles(pStateChange);
    return NtRemoveProcessDebug(hProcess, DbgGetThreadDebugObject());
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif