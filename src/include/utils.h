// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"
#include "conapi.h"

NTSTATUS WriteFileData(
    _In_ HANDLE hFile,
    _In_reads_bytes_(Length) PVOID pBuffer,
    _In_ ULONG uLength,
    _In_ BOOL fUnicode
) {
    IO_STATUS_BLOCK IoStatus;

    if (fUnicode) {
        NTSTATUS NtStatus;
        ULONG cchUTF8String;
        PCH pUTF8String = (PCH) _malloca((uLength >> 1) * 3);

        NtStatus = RtlUnicodeToUTF8N(pUTF8String,
            (uLength >> 1) * 3, &cchUTF8String, (PCWCH) pBuffer, uLength);

        if (NT_SUCCESS(NtStatus)) NtWriteFile(hFile, NULL, NULL,
            NULL, &IoStatus, pBuffer, uLength, NULL, NULL);

        return NtStatus;
    }

    return NtWriteFile(hFile, NULL, NULL, NULL, &IoStatus,
        pBuffer, uLength, NULL, NULL);
}

NTSTATUS WriteHandle(
    _In_ HANDLE hHandle,
    _In_reads_bytes_(Length) PVOID pBuffer,
    _In_ ULONG uLength,
    _In_ BOOL fUnicode,
    _In_ BOOL bConsole
) {
    return bConsole ? WriteConsoleDevice(hHandle, pBuffer, uLength, fUnicode)
                    : WriteFileData(hHandle, pBuffer, uLength, fUnicode);
}

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

BOOL DoesFileExists(
    PCWSTR pDosName,
    PUNICODE_STRING pNtName
) {
    NTSTATUS NtStatus;
    RTL_RELATIVE_NAME_U RelativeName;

    // Get the NT Path
    NtStatus = RtlDosPathNameToNtPathName_U_WithStatus(pDosName, pNtName, NULL, &RelativeName);

    if (!NT_SUCCESS(NtStatus)) return FALSE;

    // Save the buffer
    PWCH pBuffer = pNtName->Buffer;

    // Check if we have a relative name
    if (RelativeName.RelativeName.Length) {
        // Use it
        *pNtName = RelativeName.RelativeName;
    } else {
        // Otherwise ignore it
        RelativeName.ContainingDirectory = NULL;
    }

    OBJECT_ATTRIBUTES ObjectAttributes;
    FILE_BASIC_INFORMATION BasicInformation;

    // Initialize the object attributes
    InitializeObjectAttributes(&ObjectAttributes,
                               pNtName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

    // Query the attributes and free the buffer now
    NtStatus = NtQueryAttributesFile(&ObjectAttributes, &BasicInformation);
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(GetProcessHeap(), 0, pBuffer);

    // Check if we failed
    if (!NT_SUCCESS(NtStatus)) {
        // Check if we failed because the file is in use
        if (NtStatus == STATUS_SHARING_VIOLATION ||
            NtStatus == STATUS_ACCESS_DENIED) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    return TRUE;
}

//
//  Retrieves a localized system message from ntdll.dll
//  Leverages direct access to the second loaded module in the PEB loader data
//

NTSTATUS LookupNtdllMessage(DWORD dwMessageId, DWORD dwLanguageId, PMESSAGE_RESOURCE_ENTRY *MessageEntry) {
    return RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink->Flink)->DllBase,
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
    while (pText[(wLength - 1) >> 1] == L'\0') wLength -= 2;

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