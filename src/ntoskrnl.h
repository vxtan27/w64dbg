#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>

NTSYSCALLAPI NTSTATUS NTAPI NtReadVirtualMemory(HANDLE ProcessHandle, PVOID BaseAddress, PVOID Buffer, SIZE_T BufferSize, PSIZE_T NumberOfBytesRead);
NTSYSCALLAPI NTSTATUS NTAPI NtSuspendProcess(HANDLE ProcessHandle);
NTSYSCALLAPI NTSTATUS NTAPI NtResumeProcess(HANDLE ProcessHandle);