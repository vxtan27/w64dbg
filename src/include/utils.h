// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#undef NtCurrentPeb
FORCEINLINE
PPEB
NtCurrentPeb(
    VOID
)
{
#if defined(_M_AMD64)
    return (PPEB)__readgsqword(FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_IX86)
    return (PPEB)__readfsdword(FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_ARM)
    return (PPEB)(((PTEB)(ULONG_PTR)_MoveFromCoprocessor(CP15_TPIDRURW))->ProcessEnvironmentBlock);
#elif defined(_M_ARM64)
    return (PPEB)(((PTEB)__getReg(18))->ProcessEnvironmentBlock);
#elif defined(_M_IA64)
    return *(PPEB*)((size_t)_rdteb() + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_ALPHA)
    return *(PPEB*)((size_t)_rdteb() + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_MIPS)
    return *(PPEB*)((*(size_t*)(0x7ffff030)) + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#elif defined(_M_PPC)
    return *(PPEB*)(__gregister_get(13) + FIELD_OFFSET(TEB, ProcessEnvironmentBlock));
#else
#error "Unsupported architecture"
#endif
}

EXTERN_C_START

#if _WIN32_WINNT >= _WIN32_WINNT_WIN7
NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToUTF8N(
    _Out_writes_bytes_to_(UTF8StringMaxByteCount, *UTF8StringActualByteCount) PCHAR UTF8StringDestination,
    _In_ ULONG UTF8StringMaxByteCount,
    _Out_opt_ PULONG UTF8StringActualByteCount,
    _In_reads_bytes_(UnicodeStringByteCount) PCWCH UnicodeStringSource,
    _In_ ULONG UnicodeStringByteCount
);
#endif

#if _WIN32_WINNT >= _WIN32_WINNT_WS03
NTSYSAPI
NTSTATUS
NTAPI
RtlDosPathNameToNtPathName_U_WithStatus(
    _In_ PCWSTR DosFileName,
    _Out_ PUNICODE_STRING NtFileName,
    _Out_opt_ PWSTR *FilePart,
    _Out_opt_ PRTL_RELATIVE_NAME_U RelativeName
);
#endif

EXTERN_C_END

#define RtlGetProcessHeap() (NtCurrentPeb()->ProcessHeap)

#define GetStandardInput() (NtCurrentPeb()->ProcessParameters->StandardInput)
#define GetStandardOutput() (NtCurrentPeb()->ProcessParameters->StandardOutput)
#define GetStandardError() (NtCurrentPeb()->ProcessParameters->StandardError)

#define GetCurrentDirectory() (&NtCurrentPeb()->ProcessParameters->CurrentDirectory)
#define GetCurrentDirectoryDosPath() (&GetCurrentDirectory()->DosPath)
#define GetCommandLine() (&NtCurrentPeb()->ProcessParameters->CommandLine)
#define GetEnvironment() (NtCurrentPeb()->ProcessParameters->Environment)

ULONG ConvertUnicodeToUTF8(
    PWCH pUnicodeString,
    ULONG cchUnicodeString,
    PCH pUTF8String,
    ULONG cbUTF8String
) {
    ULONG cchUTF8String;

    RtlUnicodeToUTF8N(pUTF8String, cbUTF8String,
        &cchUTF8String, pUnicodeString, cchUnicodeString);
    return cchUTF8String;
}

BOOL WriteDataA(
    HANDLE hHandle,
    PCH pBuffer,
    ULONG uLength,
    BOOL bConsole
) {
    if (bConsole) {
        return WriteConsoleA(hHandle, pBuffer, uLength, NULL, NULL);
    } else {
        IO_STATUS_BLOCK IoStatus;
        return NT_SUCCESS(NtWriteFile(hHandle, NULL, NULL,
            NULL, &IoStatus, pBuffer, uLength, NULL, NULL));
    }
}

BOOL WriteDataW(
    HANDLE hHandle,
    PWCH pBuffer,
    ULONG uLength,
    BOOL bConsole
) {
    if (bConsole) {
        return WriteConsoleW(hHandle, pBuffer, uLength, NULL, NULL);
    } else {
        ULONG cchUTF8String;
        char cBuffer[WBUFLEN * 3];

        if (NT_SUCCESS(RtlUnicodeToUTF8N(cBuffer, sizeof(cBuffer),
            &cchUTF8String, pBuffer, uLength)))
            return WriteDataA(hHandle, cBuffer, cchUTF8String, bConsole);
        else return FALSE;
    }
}

//
//  Retrieves a localized system message from ntdll.dll
//  Leverages direct access to the second loaded module in the PEB loader data
//

NTSTATUS LookupNtdllMessage(DWORD dwMessageId, DWORD dwLanguageId, PMESSAGE_RESOURCE_ENTRY *MessageEntry) {
    return RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink)->DllBase,
        PtrToUlong(RT_MESSAGETABLE), dwLanguageId, dwMessageId, MessageEntry);
}

//
//  Retrieves a localized system message from KernelBase.dll
//  Leverages direct access to the fourth loaded module in the PEB loader data
//

NTSTATUS LookupSystemMessage(DWORD dwMessageId, DWORD dwLanguageId, PMESSAGE_RESOURCE_ENTRY *MessageEntry) {
    return RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink->Flink->Flink)->DllBase,
        PtrToUlong(RT_MESSAGETABLE), dwLanguageId, dwMessageId, MessageEntry);
}

// Retrieves the text of a message resource entry
PWCH GetMessageEntryText(PMESSAGE_RESOURCE_ENTRY MessageEntry) {
    return (PWCH) MessageEntry->Text;
}

// Retrieves the length of a message resource entry's text
WORD GetMessageEntryLength(PMESSAGE_RESOURCE_ENTRY MessageEntry) {
    // Compute text length, excluding trailing null character
    WORD wLength = MessageEntry->Length - FIELD_OFFSET(MESSAGE_RESOURCE_ENTRY, Text) - sizeof(wchar_t);
    PCWSTR pText = GetMessageEntryText(MessageEntry);

    // Trim additional trailing nulls
    while (pText[wLength] == L'0') --wLength;

    return wLength;
}

// Determines whether a given handle is associated with a console
NTSTATUS IsConsoleHandle(HANDLE hHandle, PBOOL pResult) {
    IO_STATUS_BLOCK IoStatus;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

    NTSTATUS NtStatus = NtQueryVolumeInformationFile(hHandle, &IoStatus,
        &DeviceInfo, sizeof(DeviceInfo), FileFsDeviceInformation);

    *pResult = (DeviceInfo.DeviceType == FILE_DEVICE_CONSOLE) ||
               (DeviceInfo.DeviceType == FILE_DEVICE_TERMSRV);
    return NtStatus;
}

// Retrieves the size of the specified module
NTSTATUS GetModuleSize(HANDLE hModule, PDWORD pFileSize) {
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileInfo;

    NTSTATUS NtStatus = NtQueryInformationFile(hModule, &IoStatus,
        &FileInfo, sizeof(FileInfo), FileStandardInformation);

    *pFileSize = FileInfo.EndOfFile.LowPart;
    return NtStatus;
}

_Success_(return >= 0)
_Check_return_opt_
int fast_sprintf(
    _Pre_notnull_ _Always_(_Post_z_) char*       const _Buffer,
    _In_z_ _Printf_format_string_    char const* const _Format,
    ...) {
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);

    _Result = __stdio_common_vsprintf(0,
        _Buffer, (size_t) -1, _Format, NULL, _ArgList);

    __crt_va_end(_ArgList);
    return _Result;
}