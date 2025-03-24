// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

//------------------------------------------------------------------------------
// Data Structures & Macros
//------------------------------------------------------------------------------

// Debug thread data structure
typedef struct _DBGSS_THREAD_DATA {
    struct _DBGSS_THREAD_DATA *Next;   // Next thread data in the linked list
    HANDLE Handle;                     // Debugged thread/process handle
    DWORD ProcessId;                   // Process identifier
    DWORD ThreadId;                    // Thread identifier
    BOOL Marked;                       // Flag for processed state
} DBGSS_THREAD_DATA, *PDBGSS_THREAD_DATA;

// Set/Get per-thread debug data (stored in the TEB reserved field)
#define DbgSsSetThreadData(d) \
    NtCurrentTeb()->DbgSsReserved[0] = d

#define DbgSsGetThreadData() \
    ((PDBGSS_THREAD_DATA) NtCurrentTeb()->DbgSsReserved[0])

// Retrieve the thread's debug object handle
#define DbgGetThreadDebugObject() \
    (NtCurrentTeb()->DbgSsReserved[1])

//------------------------------------------------------------------------------
// Utility Functions: Process & Thread ID Extraction
//------------------------------------------------------------------------------

// Extract process ID from wait state change event
DWORD DbgGetProcessId(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    return HandleToUlong(pStateChange->AppClientId.UniqueProcess);
}

// Extract thread ID from wait state change event
DWORD DbgGetThreadId(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    return HandleToUlong(pStateChange->AppClientId.UniqueThread);
}

//------------------------------------------------------------------------------
// Handle Management: Save, Mark, Remove, Close
//------------------------------------------------------------------------------

// Save the thread handle from a debug event onto the current thread's list
VOID DbgSaveThreadHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    // Allocate and fill a new thread data structure
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        GetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));
    ThreadData->Handle = pStateChange->StateInfo.CreateThread.HandleToThread;
    ThreadData->ProcessId = DbgGetProcessId(pStateChange);
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Marked = FALSE;

    // Link new data to list head
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Save process handle and its initial thread handle from a debug event
VOID DbgSaveProcessHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    // Allocate and fill process handle structure
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        GetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));
    ThreadData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToProcess;
    ThreadData->ProcessId = DbgGetProcessId(pStateChange);
    ThreadData->ThreadId = 0;
    ThreadData->Marked = FALSE;

    // Link process data into list
    ThreadData->Next = DbgSsGetThreadData();

    // Allocate and fill initial thread handle structure
    PDBGSS_THREAD_DATA ThisData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        GetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));
    ThisData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToThread;
    ThisData->ProcessId = DbgGetProcessId(pStateChange);
    ThisData->ThreadId = DbgGetThreadId(pStateChange);
    ThisData->Marked = FALSE;

    // Link initial thread data before process data
    ThisData->Next = ThreadData;
    DbgSsSetThreadData(ThisData);
}

// Mark thread handle in list that matches the debug event
VOID DbgMarkThreadHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (ThreadData->ThreadId == DbgGetThreadId(pStateChange) &&
            ThreadData->ProcessId == DbgGetProcessId(pStateChange)) {
            ThreadData->Marked = TRUE;
            break;
        }
    }
}

// Mark process handle in list that matches the debug event
VOID DbgMarkProcessHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (!ThreadData->ThreadId &&
            ThreadData->ProcessId == DbgGetProcessId(pStateChange)) {
            ThreadData->Marked = TRUE;
            break;
        }
    }
}

// Remove marked handles matching the debug event, freeing their resources
VOID DbgRemoveHandles(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    PDBGSS_THREAD_DATA *ThreadData = (PDBGSS_THREAD_DATA*) NtCurrentTeb()->DbgSsReserved;
    PDBGSS_THREAD_DATA ThisData = *ThreadData;

    while (ThisData) {
        if (ThisData->Marked && ThisData->ProcessId == DbgGetProcessId(pStateChange) &&
           (ThisData->ThreadId == DbgGetThreadId(pStateChange) || !ThisData->ThreadId)) {
            CloseHandle(ThisData->Handle);  // Close handle
            *ThreadData = ThisData->Next;     // Unlink from list
            RtlFreeHeap(GetProcessHeap(), 0, ThisData);  // Free memory
        } else {
            ThreadData = &ThisData->Next;
        }
        ThisData = *ThreadData;
    }
}

// Close all handles for the process matching the debug event
VOID DbgCloseAllProcessHandles(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    PDBGSS_THREAD_DATA *ThreadData = (PDBGSS_THREAD_DATA*) NtCurrentTeb()->DbgSsReserved;
    PDBGSS_THREAD_DATA ThisData = *ThreadData;

    while (ThisData) {
        if (ThisData->ProcessId == DbgGetProcessId(pStateChange)) {
            CloseHandle(ThisData->Handle);
            *ThreadData = ThisData->Next;
            RtlFreeHeap(GetProcessHeap(), 0, ThisData);
        } else {
            ThreadData = &ThisData->Next;
        }
        ThisData = *ThreadData;
    }
}

//------------------------------------------------------------------------------
// Debug Event Handling
//------------------------------------------------------------------------------

// Wait for a debug event; process thread/process creation/exits accordingly
NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    BOOL bAlertable,
    PLARGE_INTEGER pTimeout
) {
    NTSTATUS NtStatus;

    if (bAlertable) {
        // Retry if interrupted by alert/APC
        do {
            NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(), TRUE, pTimeout, pStateChange);
        } while (NtStatus == STATUS_ALERTED || NtStatus == STATUS_USER_APC);
    } else {
        NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(), FALSE, pTimeout, pStateChange);
    }

    // Return early on error/timeout
    if (!NT_SUCCESS(NtStatus) || NtStatus == DBG_UNABLE_TO_PROVIDE_HANDLE ||
       (pTimeout && NtStatus == STATUS_TIMEOUT))
        return NtStatus;

    // Process event based on type
    switch (pStateChange->NewState) {
        case DbgCreateThreadStateChange:
            DbgSaveThreadHandle(pStateChange);  // New thread created
            break;
        case DbgExitThreadStateChange:
            DbgMarkThreadHandle(pStateChange);    // Thread exit
            break;
        case DbgCreateProcessStateChange:
            DbgSaveProcessHandle(pStateChange);   // New process created
            break;
        case DbgExitProcessStateChange:
            DbgMarkThreadHandle(pStateChange);      // Process exit (thread)
            DbgMarkProcessHandle(pStateChange);       // Process exit (process)
            break;
        // Other state changes: no action needed
        case DbgLoadDllStateChange:
        case DbgUnloadDllStateChange:
        case DbgExceptionStateChange:
        case DbgBreakpointStateChange:
        case DbgSingleStepStateChange:
            break;
        default:
            break;
    }
    return NtStatus;
}

// Continue a debug event and clean up associated handles
NTSTATUS DbgContinue(
    PDBGUI_WAIT_STATE_CHANGE pStateChange,
    NTSTATUS dwContinueStatus
) {
    NTSTATUS NtStatus = NtDebugContinue(DbgGetThreadDebugObject(),
        &pStateChange->AppClientId, dwContinueStatus);
    DbgRemoveHandles(pStateChange);  // Clean up after continuation
    return NtStatus;
}

// Stop debugging a process by closing all handles and removing the debug object
NTSTATUS DbgStopDebugging(
    HANDLE hProcess,
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    DbgCloseAllProcessHandles(pStateChange);  // Clean up process handles
    return NtRemoveProcessDebug(hProcess, DbgGetThreadDebugObject());
}