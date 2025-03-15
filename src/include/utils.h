//
//  SPDX-License-Identifier: BSD-3-Clause
//  Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
//

#pragma once

// Retrieves a pointer to the Process Environment Block (PEB) of the current process

static
FORCEINLINE
PPEB
GetCurrentPeb(
    VOID
    )
{
#ifdef _M_X64
    return (PPEB) __readgsqword(0x60); // GS:[0x60]
#elif defined(_M_IX86)
    return (PPEB) __readfsdword(0x30); // FS:[0x30]
#elif defined(_M_ARM)
    return *(PPEB*) (_MoveFromCoprocessor(15, 0, 13, 0, 2) + 0x30); // CP15 + 0x30
#elif defined(_M_ARM64)
    return *(PPEB*) (__getReg(18) + 0x60); // x18 + 0x60
#elif defined(_M_IA64)
    return *(PPEB*) ((size_t)_rdteb() + 0x60); // r13 + 0x60
#elif defined(_M_ALPHA)
    return *(PPEB*) ((size_t)_rdteb() + 0x30); // callpal 0xAB + 0x30
#elif defined(_M_MIPS)
    return *(PPEB*) ((*(size_t*) (0x7ffff030)) + 0x30); // PCR[0x30]
#elif defined(_M_PPC)
    return *(PPEB*) (__gregister_get(13) + 0x30); // r13 + 0x30
#else
#error Unsupported architecture
#endif
}

// Retrieves the locale identifier (LCID) of the current thread

static
FORCEINLINE
LCID
GetCurrentLCID(
    VOID
    )
{
    // NtCurrentTeb()->CurrentLocale
    return PtrToUlong(NtCurrentTeb()->Reserved2[20]);
}

// Retrieves the language identifier (LANGID) of the current thread

static
FORCEINLINE
LANGID
GetCurrentLangID(
    VOID
    )
{
    return LANGIDFROMLCID(GetCurrentLCID());
}

// Determines whether a given handle is associated with a console

static
FORCEINLINE
NTSTATUS
IsConsoleHandle(HANDLE hHandle, PBOOL pResult)
{
    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

    NtStatus = NtQueryVolumeInformationFile(hHandle, &IoStatusBlock,
        &DeviceInfo, sizeof(DeviceInfo), FileFsDeviceInformation);

    *pResult = (DeviceInfo.DeviceType == FILE_DEVICE_CONSOLE) ||
               (DeviceInfo.DeviceType == FILE_DEVICE_TERMSRV);
    return NtStatus;
}

// Retrieves the size of the specified module
static FORCEINLINE DWORD GetModuleSize(HANDLE hModule) {
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION FileInfo;

    NtQueryInformationFile(hModule, &IoStatusBlock,
        &FileInfo, sizeof(FileInfo), FileStandardInformation);
    return FileInfo.EndOfFile.LowPart;
}