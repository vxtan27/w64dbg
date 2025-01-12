/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#if WINVER < 0x0601
#error "Unsupported Windows version"
#endif

#include <winternl.h>

typedef enum _WAIT_TYPE {
    WaitAll,
    WaitAny,
    WaitNotification
} WAIT_TYPE;

typedef struct _CURDIR
{
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    UNICODE_STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

#define RTL_MAX_DRIVE_LETTERS 32

typedef struct
{
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    HANDLE ConsoleHandle;
    ULONG ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;

    CURDIR CurrentDirectory;
    UNICODE_STRING DllPath;
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
    PWCHAR Environment;

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;
    UNICODE_STRING DesktopInfo;
    UNICODE_STRING ShellInfo;
    UNICODE_STRING RuntimeData;
    RTL_DRIVE_LETTER_CURDIR CurrentDirectories[RTL_MAX_DRIVE_LETTERS];

    ULONG_PTR EnvironmentSize;
    ULONG_PTR EnvironmentVersion;
    PVOID PackageDependencyData;
    ULONG ProcessGroupId;
    ULONG LoaderThreads;
} ZWRTL_USER_PROCESS_PARAMETERS, *PZWRTL_USER_PROCESS_PARAMETERS;

typedef struct _FILE_STANDARD_INFORMATION
{
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

NTSYSAPI NTSTATUS  WINAPI LdrFindEntryForAddress(const void*, PLDR_DATA_TABLE_ENTRY*);
NTSYSAPI void      WINAPI LdrShutdownProcess(void);
NTSYSAPI void      WINAPI LdrShutdownThread(void);
NTSYSAPI NTSTATUS  WINAPI NtDelayExecution(BOOLEAN,const LARGE_INTEGER*);
NTSYSAPI NTSTATUS  WINAPI NtGetContextThread(HANDLE,CONTEXT*);
NTSYSAPI NTSTATUS  WINAPI NtQueryInformationFile(HANDLE,PIO_STATUS_BLOCK,PVOID,ULONG,FILE_INFORMATION_CLASS);
NTSYSAPI NTSTATUS  WINAPI NtReadFile(HANDLE,HANDLE,PIO_APC_ROUTINE,PVOID,PIO_STATUS_BLOCK,PVOID,ULONG,PLARGE_INTEGER,PULONG);
NTSYSAPI NTSTATUS  WINAPI NtReadVirtualMemory(HANDLE,const void*,void*,SIZE_T,SIZE_T*);
NTSYSAPI NTSTATUS  WINAPI NtResumeProcess(HANDLE);
NTSYSAPI NTSTATUS  WINAPI NtSetInformationFile(HANDLE,PIO_STATUS_BLOCK,PVOID,ULONG,FILE_INFORMATION_CLASS);
NTSYSAPI NTSTATUS  WINAPI NtSuspendProcess(HANDLE);
NTSYSAPI NTSTATUS  WINAPI NtTerminateProcess(HANDLE,LONG);
NTSYSAPI NTSTATUS  WINAPI NtTerminateThread(HANDLE,LONG);
NTSYSAPI NTSTATUS  WINAPI NtWaitForMultipleObjects(ULONG,const HANDLE*,BOOLEAN,BOOLEAN,const LARGE_INTEGER*);
NTSYSAPI LONG      WINAPI RtlCompareUnicodeStrings(const WCHAR*,SIZE_T,const WCHAR*,SIZE_T,BOOLEAN);
NTSYSAPI ULONG     WINAPI RtlDosSearchPath_U(LPCWSTR, LPCWSTR, LPCWSTR, ULONG, LPWSTR, LPWSTR*);
NTSYSAPI ULONG     WINAPI RtlGetCurrentDirectory_U(ULONG, LPWSTR);
NTSYSAPI NTSTATUS  WINAPI RtlQueryEnvironmentVariable(WCHAR*,const WCHAR*,SIZE_T,WCHAR*,SIZE_T,SIZE_T*);
NTSYSAPI NTSTATUS  WINAPI RtlUnicodeToUTF8N(LPSTR,DWORD,LPDWORD,LPCWSTR,DWORD);
NTSYSAPI NTSTATUS  WINAPI NtWriteFile(HANDLE,HANDLE,PIO_APC_ROUTINE,PVOID,PIO_STATUS_BLOCK,const void*,ULONG,PLARGE_INTEGER,PULONG);
int                WINAPI sprintf( char *str, const char *format, ... );

__declspec(noreturn)
static __forceinline void CleanProcess(UINT uExitCode)
{
    NtTerminateProcess(0, uExitCode);
    LdrShutdownProcess();
    while (TRUE) NtTerminateProcess((HANDLE) -1, uExitCode);
}

__declspec(noreturn)
static __forceinline void CleanThread(UINT uExitCode)
{
    LdrShutdownThread();
    while (TRUE) NtTerminateThread((HANDLE) -1, uExitCode);
}