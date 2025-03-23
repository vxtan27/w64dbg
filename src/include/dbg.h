// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

typedef struct _DBGSS_THREAD_DATA {
    struct _DBGSS_THREAD_DATA *Next;
    HANDLE ThreadHandle;
    HANDLE ProcessHandle;
    DWORD ProcessId;
    DWORD ThreadId;
    BOOLEAN HandleMarked;
} DBGSS_THREAD_DATA, *PDBGSS_THREAD_DATA;

#define DbgSsSetThreadData(d) \
    NtCurrentTeb()->DbgSsReserved[0] = d

#define DbgSsGetThreadData() \
    ((PDBGSS_THREAD_DATA) NtCurrentTeb()->DbgSsReserved[0])

// Retrieves the debug object handle of the current thread
#define DbgGetThreadDebugObject() \
    (NtCurrentTeb()->DbgSsReserved[1])

DWORD DbgGetProcessId(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    return HandleToUlong(pStateChange->AppClientId.UniqueProcess);
}

DWORD DbgGetThreadId(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    return HandleToUlong(pStateChange->AppClientId.UniqueThread);
}

// Saves the thread handle of the process being debugged on the current thread
VOID DbgSaveThreadHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    // Allocate a thread structure
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        GetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));

    // Fill it out
    ThreadData->ThreadHandle = pStateChange->StateInfo.CreateThread.HandleToThread;
    ThreadData->ProcessHandle = NULL;
    ThreadData->ProcessId = DbgGetProcessId(pStateChange);
    ThreadData->ThreadId = DbgGetThreadId(pStateChange);
    ThreadData->HandleMarked = FALSE;

    // Link it
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Saves the process and initial thread handle of the process being debugged on the current thread
VOID DbgSaveProcessHandle(PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    // Allocate a thread structure
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        GetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));

    // Fill it out
    ThreadData->ThreadHandle = NULL;
    ThreadData->ProcessHandle = pStateChange->StateInfo.CreateProcessInfo.HandleToProcess;
    ThreadData->ProcessId = DbgGetProcessId(pStateChange);
    ThreadData->ThreadId = 0;
    ThreadData->HandleMarked = FALSE;

    // Link it
    ThreadData->Next = DbgSsGetThreadData();

    // Allocate another thread structure
    PDBGSS_THREAD_DATA ThisData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        GetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));

    // Fill it out
    ThisData->ThreadHandle = pStateChange->StateInfo.CreateProcessInfo.HandleToThread;
    ThisData->ProcessHandle = NULL;
    ThisData->ProcessId = DbgGetProcessId(pStateChange);
    ThisData->ThreadId = DbgGetThreadId(pStateChange);
    ThisData->HandleMarked = FALSE;

    // Link it
    ThisData->Next = ThreadData;
    DbgSsSetThreadData(ThisData);
}

// Marks the thread handle of the process being debugged on the current thread
VOID DbgMarkThreadHandle(DWORD dwProcessId, DWORD dwThreadId) {
    // Loop all thread data events
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
        ThreadData; ThreadData = ThreadData->Next) {
        // Check if this one matches
        if (ThreadData->ThreadId == dwThreadId &&
            ThreadData->ProcessId == dwProcessId) {
            // Mark the structure and break out
            ThreadData->HandleMarked = TRUE;
            break;
        }
    }
}

// Marks the process handle of the process being debugged on the current thread
VOID DbgMarkProcessHandle(DWORD dwProcessId) {
    // Loop all thread data events
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
        ThreadData; ThreadData = ThreadData->Next) {
        // Check if this one matches
        if (!ThreadData->ThreadId && ThreadData->ProcessId == dwProcessId) {
            // Mark the structure and break out
            ThreadData->HandleMarked = TRUE;
            break;
        }
    }
}

// Removes the handles of the process being debugged on the current thread
VOID DbgRemoveHandles(DWORD dwProcessId, DWORD dwThreadId) {
    PDBGSS_THREAD_DATA *ThreadData;
    PDBGSS_THREAD_DATA ThisData;

    // Loop all thread data events
    ThreadData = (PDBGSS_THREAD_DATA*) NtCurrentTeb()->DbgSsReserved;
    ThisData = *ThreadData;

    while (ThisData) {
        // Check if this one matches
        if (ThisData->HandleMarked && ThisData->ProcessId == dwProcessId &&
            (ThisData->ThreadId == dwThreadId || !ThisData->ThreadId)) {
            // Close open handles
            if (ThisData->ThreadHandle) CloseHandle(ThisData->ThreadHandle);
            else if (ThisData->ProcessHandle) CloseHandle(ThisData->ProcessHandle);

            // Unlink the thread data
            *ThreadData = ThisData->Next;

            // Free it
            RtlFreeHeap(GetProcessHeap(), 0, ThisData);
        } else {
            // Move to the next one
            ThreadData = &ThisData->Next;
        }

        ThisData = *ThreadData;
    }
}

// Close all the handles of the process being debugged on the current thread
VOID DbgCloseAllProcessHandles(DWORD dwProcessId) {
    PDBGSS_THREAD_DATA *ThreadData;
    PDBGSS_THREAD_DATA ThisData;

    // Loop all thread data events
    ThreadData = (PDBGSS_THREAD_DATA*) NtCurrentTeb()->DbgSsReserved;
    ThisData = *ThreadData;

    while (ThisData) {
        // Check if this one matches
        if (ThisData->ProcessId == dwProcessId) {
            // Close open handles
            if (ThisData->ThreadHandle) CloseHandle(ThisData->ThreadHandle);
            if (ThisData->ProcessHandle) CloseHandle(ThisData->ProcessHandle);

            // Unlink the thread data
            *ThreadData = ThisData->Next;

            // Free it
            RtlFreeHeap(GetProcessHeap(), 0, ThisData);
        } else {
            // Move to the next one
            ThreadData = &ThisData->Next;
        }

        ThisData = *ThreadData;
    }
}

// Waits for a debugging event to occur in a process being debugged
NTSTATUS DbgWaitStateChange(
    PDBGUI_WAIT_STATE_CHANGE pStateChange, BOOLEAN bAlertable, PLARGE_INTEGER pTimeout) {
    NTSTATUS NtStatus;

    // Loop while we keep getting interrupted
    if (bAlertable) {
        do {
            // Call the native API
            NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(), TRUE, pTimeout, pStateChange);
        } while (NtStatus == STATUS_ALERTED || NtStatus == STATUS_USER_APC);
    } else {
        // Call the native API
        NtStatus = NtWaitForDebugEvent(DbgGetThreadDebugObject(), FALSE, pTimeout, pStateChange);
    }

    // Check if the wait failed or we timed out
    if (!NT_SUCCESS(NtStatus) || NtStatus == DBG_UNABLE_TO_PROVIDE_HANDLE ||
        (pTimeout && NtStatus == STATUS_TIMEOUT)) return NtStatus;

    // Check what kind of event this was
    switch (pStateChange->NewState) {
    // New thread was created
    case DbgCreateThreadStateChange:
        // Setup the thread data
        DbgSaveThreadHandle(pStateChange);
        break;

    // Thread was exited
    case DbgExitThreadStateChange:
        // Mark the thread data
        DbgMarkThreadHandle(DbgGetProcessId(pStateChange),
            DbgGetThreadId(pStateChange));
        break;

    // New process was created
    case DbgCreateProcessStateChange:
        // Setup the process and initial thread data
        DbgSaveProcessHandle(pStateChange);
        break;

    // Process was exited
    case DbgExitProcessStateChange:
        // Mark the thread data
        DbgMarkThreadHandle(DbgGetProcessId(pStateChange),
            DbgGetThreadId(pStateChange));
        // Mark the process data
        DbgMarkProcessHandle(DbgGetProcessId(pStateChange));
        break;

    // Nothing to do
    case DbgLoadDllStateChange:
    case DbgUnloadDllStateChange:
    case DbgExceptionStateChange:
    case DbgBreakpointStateChange:
    case DbgSingleStepStateChange:
        break;

    // Fail anything else
    default:
        break;
    }

    return NtStatus;
}

// Enables a debugger to continue a thread that previously reported a debugging event
NTSTATUS DbgContinue(PDBGUI_WAIT_STATE_CHANGE pStateChange, NTSTATUS dwContinueStatus) {
    // Continue debugging
    NTSTATUS NtStatus = NtDebugContinue(DbgGetThreadDebugObject(),
        &pStateChange->AppClientId, dwContinueStatus);

    // Remove the process/thread handles
    if (NT_SUCCESS(NtStatus)) DbgRemoveHandles(DbgGetProcessId(pStateChange),
        DbgGetThreadId(pStateChange));

    return NtStatus;
}

// Stops the debugger from debugging the specified process
NTSTATUS DbgStopDebugging(HANDLE hProcess, PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    // Close all the process handles
    DbgCloseAllProcessHandles(DbgGetProcessId(pStateChange));

    // Now stop debgging the process
    return NtRemoveProcessDebug(hProcess, DbgGetThreadDebugObject());
}