// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// -----------------------------------------------------------------------------
// Data Structure
// -----------------------------------------------------------------------------

// Debug thread data structure (single-process mode)
typedef struct _DBGSS_THREAD_DATA {
    struct _DBGSS_THREAD_DATA *Next; // Next in linked list
    HANDLE Handle;                   // Thread/process handle
    DWORD ThreadId;                  // Thread ID
} DBGSS_THREAD_DATA, *PDBGSS_THREAD_DATA;

// -----------------------------------------------------------------------------
//  Handle Management: Save, Remove, Close
// -----------------------------------------------------------------------------

// Save thread handle from debug event into thread-local storage
DBGAPI VOID DbgSaveThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(DBGSS_THREAD_DATA));
    ThreadData->Handle = pStateChange->StateInfo.CreateThread.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Save process and initial thread handles from debug event
DBGAPI VOID DbgSaveProcessHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    PDBGSS_THREAD_DATA ProcessData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(DBGSS_THREAD_DATA));
    ProcessData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToProcess;
    ProcessData->ThreadId = 0;
    ProcessData->Next = DbgSsGetThreadData();

    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlProcessHeap(), HEAP_NO_SERIALIZE, sizeof(DBGSS_THREAD_DATA));
    ThreadData->Handle = pStateChange->StateInfo.CreateProcessInfo.HandleToThread;
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->Next = ProcessData;
    DbgSsSetThreadData(ThreadData);
}

// Remove and free thread handle matching the debug event
DBGAPI VOID DbgFreeThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
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
DBGAPI VOID DbgCloseAllProcessHandles(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
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

// Retrieve process handle from the list for the debug event
DBGAPI HANDLE DbgGetProcessHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    (void) pStateChange;

    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (!ThreadData->ThreadId)
            return ThreadData->Handle;
    }
}

// Retrieve thread handle from the list for the debug event
DBGAPI HANDLE DbgGetThreadHandle(
    PDBGUI_WAIT_STATE_CHANGE pStateChange
) {
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
         ThreadData; ThreadData = ThreadData->Next) {
        if (ThreadData->ThreadId == DbgGetThreadId(pStateChange))
            return ThreadData->Handle;
    }
}