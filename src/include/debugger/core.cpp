// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "api.h"

// Set/Get per-thread debug data
#define DbgSsSetThreadData(d) NtCurrentTeb()->DbgSsReserved[0] = d
#define DbgSsGetThreadData() ((PDBGSS_THREAD_DATA) NtCurrentTeb()->DbgSsReserved[0])

// Get thread's debug object handle
#define DbgGetThreadDebugObject() (NtCurrentTeb()->DbgSsReserved[1])

// =====================================================================================
//  Data Structure
// =====================================================================================

// Debug thread data structure (single-process mode)
typedef struct _DBGSS_THREAD_DATA {
    struct _DBGSS_THREAD_DATA *Next; // Next in linked list
    HANDLE Handle;                   // Thread/process handle
    DWORD ThreadId;                  // Thread ID
} DBGSS_THREAD_DATA, *PDBGSS_THREAD_DATA;

#include <utility>

// =====================================================================================
//  Handle Management
// =====================================================================================

// Save thread handle from debug event into thread-local storage
VOID DbgSaveThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(DBGSS_THREAD_DATA));
    if (!ThreadData) std::unreachable();

    ThreadData->Handle = pStateChange->StateInfo.CreateThread.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Save process and initial thread handles from debug event
VOID DbgSaveProcessHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PDBGSS_THREAD_DATA ProcessData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(DBGSS_THREAD_DATA));
    if (!ProcessData) std::unreachable();

    ProcessData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToProcess;
    ProcessData->ThreadId = 0;
    ProcessData->Next = DbgSsGetThreadData();

    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(DBGSS_THREAD_DATA));
    if (!ThreadData) std::unreachable();

    ThreadData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = ProcessData;
    DbgSsSetThreadData(ThreadData);
}

// Remove and free thread handle matching the debug event
VOID DbgFreeThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PDBGSS_THREAD_DATA *ThreadData = (PDBGSS_THREAD_DATA*) NtCurrentTeb()->DbgSsReserved;
    PDBGSS_THREAD_DATA ThisData = *ThreadData;

    while (ThisData) {
        if (ThisData->ThreadId == DbgGetThreadId(pStateChange)) {
            NtClose(ThisData->Handle);
            *ThreadData = ThisData->Next;
            RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ThisData);
            break;
        }

        ThisData = ThisData->Next;
    }
}

// Close and free all handles for the process in the debug event
VOID DbgCloseAllProcessHandles(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    (void) pStateChange;

    PDBGSS_THREAD_DATA *ThreadData = (PDBGSS_THREAD_DATA*) NtCurrentTeb()->DbgSsReserved;
    PDBGSS_THREAD_DATA ThisData = *ThreadData;

    while (ThisData) {
        NtClose(ThisData->Handle);
        *ThreadData = ThisData->Next;
        RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ThisData);
        ThisData = *ThreadData;
    }
}

// Attach debugger to an active process
NTSTATUS DbgDebugActiveProcess(
    HANDLE hProcess,
    ULONG uFlags
) {
    NtCreateDebugObject(&DbgGetThreadDebugObject(), DEBUG_ALL_ACCESS, NULL, uFlags);
    return DbgUiDebugActiveProcess(hProcess);
}

// Retrieve process handle from the list for the debug event
HANDLE DbgGetProcessHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    (void) pStateChange;

    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (!ThreadData->ThreadId)
            return ThreadData->Handle;
    }

    std::unreachable();
}

// Retrieve thread handle from the list for the debug event
HANDLE DbgGetThreadHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (ThreadData->ThreadId == DbgGetThreadId(pStateChange))
            return ThreadData->Handle;
    }

    std::unreachable();
}

// Wait for a debug event, retrying if alert/APC interrupts
NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
) {
    NTSTATUS NtStatus;

    if (bAlertable) {
        do {
            NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(),
                (BOOLEAN) bAlertable, pTimeout, pStateChange);
        } while (NtStatus == STATUS_ALERTED || NtStatus == STATUS_USER_APC);
    } else {
        NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(),
            (BOOLEAN) bAlertable, pTimeout, pStateChange);
    }

    return NtStatus;
}

// Continue a thread after handling a debug event
NTSTATUS DbgContinue(
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
NTSTATUS DbgStopDebugging(
    HANDLE hProcess,
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    DbgCloseAllProcessHandles(pStateChange);
    return NtRemoveProcessDebug(hProcess, DbgGetThreadDebugObject());
}