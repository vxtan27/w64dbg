// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

typedef struct _DBGSS_THREAD_DATA
{
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
#define DbgGetThreadDebugObject() (NtCurrentTeb()->DbgSsReserved[1])

// Saves the thread handle of the process being debugged on the current thread
VOID DbgSaveThreadHandle(DWORD dwProcessId, DWORD dwThreadId, HANDLE hThread)
{
    // Allocate a thread structure
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlGetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));

    if (!ThreadData) return;

    // Fill it out
    ThreadData->ThreadHandle = hThread;
    ThreadData->ProcessHandle = NULL;
    ThreadData->ProcessId = dwProcessId;
    ThreadData->ThreadId = dwThreadId;
    ThreadData->HandleMarked = FALSE;

    // Link it
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Saves the process handle of the process being debugged on the current thread
VOID DbgSaveProcessHandle(DWORD dwProcessId, HANDLE hProcess)
{
    // Allocate a thread structure
    PDBGSS_THREAD_DATA ThreadData = (PDBGSS_THREAD_DATA) RtlAllocateHeap(
        RtlGetProcessHeap(), 0, sizeof(DBGSS_THREAD_DATA));

    if (!ThreadData) return;

    // Fill it out
    ThreadData->ThreadHandle = NULL;
    ThreadData->ProcessHandle = hProcess;
    ThreadData->ProcessId = dwProcessId;
    ThreadData->ThreadId = 0;
    ThreadData->HandleMarked = FALSE;

    // Link it
    ThreadData->Next = DbgSsGetThreadData();
    DbgSsSetThreadData(ThreadData);
}

// Marks the thread handle of the process being debugged on the current thread
VOID DbgMarkThreadHandle(DWORD dwThreadId)
{
    // Loop all thread data events
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
        ThreadData; ThreadData = ThreadData->Next)
    {
        // Check if this one matches
        if (ThreadData->ThreadId == dwThreadId)
        {
            // Mark the structure and break out
            ThreadData->HandleMarked = TRUE;
            break;
        }
    }
}

// Marks the process handle of the process being debugged on the current thread
VOID DbgMarkProcessHandle(DWORD dwProcessId)
{
    // Loop all thread data events
    for (PDBGSS_THREAD_DATA ThreadData = DbgSsGetThreadData();
        ThreadData; ThreadData = ThreadData->Next)
    {
        // Check if this one matches
        if ((ThreadData->ProcessId == dwProcessId) && !(ThreadData->ThreadId))
        {
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
        if ((ThisData->HandleMarked) &&
            ((ThisData->ProcessId == dwProcessId) || (ThisData->ThreadId == dwThreadId))) {
            // Close open handles
            if (ThisData->ThreadHandle) CloseHandle(ThisData->ThreadHandle);
            if (ThisData->ProcessHandle) CloseHandle(ThisData->ProcessHandle);

            // Unlink the thread data
            *ThreadData = ThisData->Next;

            // Free it
            RtlFreeHeap(RtlGetProcessHeap(), 0, ThisData);
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
            RtlFreeHeap(RtlGetProcessHeap(), 0, ThisData);
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
        DbgSaveThreadHandle(HandleToUlong(pStateChange->AppClientId.UniqueProcess),
            HandleToUlong(pStateChange->AppClientId.UniqueThread),
            pStateChange->StateInfo.CreateThread.HandleToThread);
        break;

    // Thread was exited
    case DbgExitThreadStateChange:
        // Mark the thread data
        DbgMarkThreadHandle(HandleToUlong(pStateChange->AppClientId.UniqueThread));
        break;

    // New process was created
    case DbgCreateProcessStateChange:
        // Setup the process data
        DbgSaveThreadHandle(HandleToUlong(pStateChange->AppClientId.UniqueProcess),
            HandleToUlong(pStateChange->AppClientId.UniqueThread),
            pStateChange->StateInfo.CreateProcessInfo.HandleToThread);
        // Setup the thread data
        DbgSaveThreadHandle(HandleToUlong(pStateChange->AppClientId.UniqueProcess),
            HandleToUlong(pStateChange->AppClientId.UniqueThread),
            pStateChange->StateInfo.CreateProcessInfo.HandleToThread);
        break;

    // Process was exited
    case DbgExitProcessStateChange:
        // Mark the thread data as such and fall through
        DbgMarkProcessHandle(HandleToUlong(pStateChange->AppClientId.UniqueThread));
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
    if (NT_SUCCESS(NtStatus)) DbgRemoveHandles(HandleToUlong(pStateChange->AppClientId.UniqueProcess),
        HandleToUlong(pStateChange->AppClientId.UniqueThread));

    return NtStatus;
}

// Stops the debugger from debugging the specified process
NTSTATUS DbgStopDebugging(HANDLE hProcess, PDBGUI_WAIT_STATE_CHANGE pStateChange) {
    // Close all the process handles
    DbgCloseAllProcessHandles(HandleToUlong(pStateChange->AppClientId.UniqueProcess));

    // Now stop debgging the process
    return NtRemoveProcessDebug(hProcess, DbgGetThreadDebugObject());
}