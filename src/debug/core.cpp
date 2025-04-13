// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "api.h"
#include <utility>

// Enable a debugger to attach to an active process and debug it
NTSTATUS WDbgDebugActiveProcess(HANDLE hProcess) {
    DbgUiConnectToDbg();
    return DbgUiDebugActiveProcess(hProcess);
}

// Wait for a debugging event to occur in a process being debugged
NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
) {
    NTSTATUS NtStatus;

    if (bAlertable) do {
        NtStatus = NtWaitForDebugEvent(DbgUiGetThreadDebugObject(),
            (BOOLEAN) bAlertable, pTimeout, pStateChange);
    } while (NtStatus == STATUS_ALERTED || NtStatus == STATUS_USER_APC);
    else NtStatus = NtWaitForDebugEvent(DbgUiGetThreadDebugObject(),
        (BOOLEAN) bAlertable, pTimeout, pStateChange);

    return NtStatus;
}

// Get PID from current debug event
FORCEINLINE DWORD DbgGetProcessId(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    return HandleToUlong(pStateChange->AppClientId.UniqueProcess);
}

// Get TID from current debug event
FORCEINLINE DWORD DbgGetThreadId(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    return HandleToUlong(pStateChange->AppClientId.UniqueThread);
}

// Save thread handle from current debug event
VOID SaveThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PWDBGSS_DATA ThreadData = (PWDBGSS_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(WDBGSS_DATA));
    if (!ThreadData) std::unreachable();

    ThreadData->Handle = pStateChange->StateInfo.CreateThread.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Save process and initial thread handles from current debug event
VOID SaveProcessHandles(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PWDBGSS_DATA ProcessData = (PWDBGSS_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(WDBGSS_DATA));
    if (!ProcessData) std::unreachable();

    ProcessData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToProcess;
    ProcessData->ThreadId = 0;
    ProcessData->Next = DbgSsGetThreadData();

    PWDBGSS_DATA ThreadData = (PWDBGSS_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(WDBGSS_DATA));
    if (!ThreadData) std::unreachable();

    ThreadData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = ProcessData;
    DbgSsSetThreadData(ThreadData);
}

// Free thread handle matching current debug event
VOID FreeThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PWDBGSS_DATA *ThreadData = (PWDBGSS_DATA*) NtCurrentTeb()->DbgSsReserved;
    PWDBGSS_DATA ThisData = *ThreadData;

    while (ThisData) {
        if (ThisData->ThreadId == DbgGetThreadId(pStateChange)) {
            NtClose(ThisData->Handle);
            *ThreadData = ThisData->Next;
            RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ThisData);
            return;
        }

        ThisData = ThisData->Next;
    }

    std::unreachable();
}

// Free process handles matching current debug event
VOID FreeProcessHandles(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    (void) pStateChange;

    PWDBGSS_DATA *ThreadData = (PWDBGSS_DATA*) NtCurrentTeb()->DbgSsReserved;
    PWDBGSS_DATA ThisData = *ThreadData;

    while (ThisData) {
        NtClose(ThisData->Handle);
        *ThreadData = ThisData->Next;
        RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ThisData);
        ThisData = *ThreadData;
    }
}

// Continue a thread after handling a debug event
NTSTATUS WDbgContinueDebugEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    NTSTATUS dwContinueStatus
) {
    NTSTATUS NtStatus = NtDebugContinue(DbgUiGetThreadDebugObject(),
        &pStateChange->AppClientId, dwContinueStatus);

    // Process state-specific actions
    switch (pStateChange->NewState) {
    case DbgCreateThreadStateChange:
        SaveThreadHandle(pStateChange); // New thread
        break;
    case DbgExitThreadStateChange:
        FreeThreadHandle(pStateChange); // Thread exit
        break;
    case DbgCreateProcessStateChange:
        SaveProcessHandles(pStateChange); // New process
        break;
    case DbgExitProcessStateChange:
        FreeProcessHandles(pStateChange); // Process exit
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
        // Invariant violation: Unexpected state
        std::unreachable();
    }

    return NtStatus;
}

// Get process handle from current debug event
HANDLE WDbgGetProcessHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    (void) pStateChange;

    for (PWDBGSS_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (!ThreadData->ThreadId)
            return ThreadData->Handle;
    }

    std::unreachable();
}

// Get thread handle from current debug event
HANDLE WDbgGetThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    for (PWDBGSS_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (ThreadData->ThreadId == DbgGetThreadId(pStateChange))
            return ThreadData->Handle;
    }

    std::unreachable();
}

// Stop the debugger from debugging the specified process
NTSTATUS WDbgDebugActiveProcessStop(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    HANDLE hProcess
) {
    FreeProcessHandles(pStateChange);
    return DbgUiStopDebugging(hProcess);
}