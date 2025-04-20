// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "api.h"
#include <utility>

// Enable a debugger to attach to an active process and debug it
NTSTATUS WdbgDebugActiveProcess(HANDLE hProcess) {
    DbgUiConnectToDbg(); // Connect to the debugger
    return DbgUiDebugActiveProcess(hProcess); // Now debug the process
}

// Wait for a debugging event to occur in a process being debugged
NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
) {
    NTSTATUS NtStatus;

    if (bAlertable) do { // Loop while we keep getting interrupted
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
    // Allocate a thread structure
    PWDBGSS_DATA ThreadData = (PWDBGSS_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(WDBGSS_DATA));
    if (!ThreadData) std::unreachable();

    // Fill it out
    ThreadData->Handle = pStateChange->StateInfo.CreateThread.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = DbgSsGetThreadData();

    // Link it
    DbgSsSetThreadData(ThreadData);
}

// Save process and initial thread handles from current debug event
VOID SaveProcessHandles(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    // Allocate a process structure
    PWDBGSS_DATA ProcessData = (PWDBGSS_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(WDBGSS_DATA));
    if (!ProcessData) std::unreachable();

    // Fill it out
    ProcessData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToProcess;
    ProcessData->ThreadId = 0;
    ProcessData->Next = NULL; // DbgSsGetThreadData()

    // Allocate a thread structure
    PWDBGSS_DATA ThreadData = (PWDBGSS_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(WDBGSS_DATA));
    if (!ThreadData) std::unreachable();

    // Fill it out
    ThreadData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = ProcessData;

    // Link it
    DbgSsSetThreadData(ThreadData);
}

// Free thread handle matching current debug event
VOID FreeThreadHandle(DWORD dwThreadId) {
    PWDBGSS_DATA *ThreadData = (PWDBGSS_DATA*) NtCurrentTeb()->DbgSsReserved;
    PWDBGSS_DATA ThisData = *ThreadData;

    while (ThisData) { // Loop all thread data events
        if (ThisData->ThreadId == dwThreadId) { // Check if this one matches
            NtClose(ThisData->Handle); // Close open thread handle
            *ThreadData = ThisData->Next; // Unlink the thread data
            RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ThisData); // Free it
            return;
        }

        // Move to the next one
        ThreadData = &ThisData->Next;
        ThisData = *ThreadData;
    }

    std::unreachable();
}

// Free process and initial thread handles
VOID FreeProcessHandles() {
    PWDBGSS_DATA ProcessData = DbgSsGetThreadData()->Next; // Move to the next one
    NtClose(DbgSsGetThreadData()->Handle); // Close open thread handle
    RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, DbgSsGetThreadData()); // Free it
    NtClose(ProcessData->Handle); // Close open process handle
    RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ProcessData); // Free it
}

// Free process handles being debugged
VOID FreeAllProcessHandles() {
    PWDBGSS_DATA *ThreadData = (PWDBGSS_DATA*) NtCurrentTeb()->DbgSsReserved;
    PWDBGSS_DATA ThisData = *ThreadData;

    while (ThisData) { // Loop all data events
        NtClose(ThisData->Handle); // Close open handle
        *ThreadData = ThisData->Next; // Unlink the data
        RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, ThisData); // Free it

        // Move to the next one
        ThreadData = &ThisData->Next;
        ThisData = *ThreadData;
    }
}

// Continue a thread after handling a debug event
NTSTATUS WdbgContinueDebugEvent(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    NTSTATUS dwContinueStatus
) {
    NTSTATUS NtStatus = NtDebugContinue(DbgUiGetThreadDebugObject(),
        &pStateChange->AppClientId, dwContinueStatus);

    // Check what kind of event this was
    switch (pStateChange->NewState) {
    case DbgCreateThreadStateChange: // New thread was created
        SaveThreadHandle(pStateChange); // Save the thread data
        break;

    case DbgExitThreadStateChange: // Thread was exited
        FreeThreadHandle(DbgGetThreadId(pStateChange)); // Free the thread data
        break;

    case DbgCreateProcessStateChange: // New process was created
        SaveProcessHandles(pStateChange); // Save the process and thread data
        break;

    case DbgExitProcessStateChange: // Process was exited
        FreeProcessHandles(); // Free the thread and process data
        break;

    // Nothing to do
    case DbgLoadDllStateChange:
    case DbgUnloadDllStateChange:
    case DbgExceptionStateChange:
    case DbgBreakpointStateChange:
    case DbgSingleStepStateChange:
        break;

    // Interruption occured
    case DbgIdle:
    case DbgReplyPending:
        break;

    default: // Fail anything else
        std::unreachable();
    }

    return NtStatus;
}

// Get process handle being debugged
HANDLE WdbgGetProcessHandle() {
    for (PWDBGSS_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) { // Loop all data events
        if (!ThreadData->ThreadId) // Check if this one matches
            return ThreadData->Handle;
    }

    std::unreachable();
}

// Get thread handle from current debug event
HANDLE WdbgGetThreadHandle(DWORD dwThreadId) {
    for (PWDBGSS_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) { // Loop all thread data events
        if (ThreadData->ThreadId == dwThreadId) // Check if this one matches
            return ThreadData->Handle;
    }

    std::unreachable();
}

// Stop the debugger from debugging the specified process
NTSTATUS WdbgDebugActiveProcessStop(
    HANDLE hProcess
) {
    FreeAllProcessHandles(); // Close all the process handles
    return DbgUiStopDebugging(hProcess); // Now stop debugging the process
}