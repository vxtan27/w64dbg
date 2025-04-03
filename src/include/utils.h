// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"
#include "conapi.h"

ULONG ConvertUnicodeToUTF8(
    _In_reads_bytes_(cchUnicodeString << 1) PCVOID pUnicodeString,
    _In_ ULONG cchUnicodeString,
    _Out_ PVOID pUTF8String,
    _In_ ULONG cbUTF8String
) {
    ULONG cchUTF8String;

    RtlUnicodeToUTF8N((PCH) pUTF8String, cbUTF8String, &cchUTF8String,
        (PCWCH) pUnicodeString, cchUnicodeString);
    return cchUTF8String;
}

NTSTATUS WriteFileData(
    _In_ HANDLE hFile,
    _In_reads_bytes_(uLength) PCVOID pBuffer,
    _In_ ULONG uLength,
    _In_ BOOL fUnicode
) {
    IO_STATUS_BLOCK IoStatus;

    if (fUnicode) {
        PCH pUTF8String = (PCH) _alloca((uLength >> 1) * 3);
        return NtWriteFile(hFile, NULL, NULL, NULL, &IoStatus,
            (PVOID) pBuffer, ConvertUnicodeToUTF8(pBuffer,
                uLength, pUTF8String, (uLength >> 1) * 3), NULL, NULL);
    }

    return NtWriteFile(hFile, NULL, NULL, NULL, &IoStatus,
        (PVOID) pBuffer, uLength, NULL, NULL);
}

NTSTATUS WriteHandle(
    _In_ HANDLE hHandle,
    _In_reads_bytes_(uLength) PCVOID pBuffer,
    _In_ ULONG uLength,
    _In_ BOOL fUnicode,
    _In_ BOOL bConsole
) {
    return bConsole ? WriteConsoleDevice(hHandle, pBuffer, uLength, fUnicode)
                    : WriteFileData(hHandle, pBuffer, uLength, fUnicode);
}

NTSTATUS WriteInvalidArgument(
    _In_ HANDLE hStdout,
    _In_reads_(nLength) PCWCH pBuffer,
    _In_ ULONG nLength,
    _In_ BOOL bConsole
) {
    if (bConsole) {
        BYTE Buffer[sizeof(CD_USER_DEFINED_IO) + sizeof(CD_IO_BUFFER) * 3];
        PCD_USER_DEFINED_IO pIoBuffer = (PCD_USER_DEFINED_IO) Buffer;

        pIoBuffer->InputCount = 4;
        pIoBuffer->OutputCount = 0;

        CONSOLE_MSG_WRITECONSOLE_L1 Msg;
        Msg.Header.ApiNumber = ConsolepWriteConsole;
        Msg.Header.ApiDescriptorSize = sizeof(CONSOLE_WRITECONSOLE_MSG);
        Msg.Msg.Unicode = FALSE;

        pIoBuffer->Buffers[0].Size = sizeof(Msg);
        pIoBuffer->Buffers[0].Buffer = &Msg;

        pIoBuffer->Buffers[1].Size = strlen(_INVALID_ARGUMENT);
        pIoBuffer->Buffers[1].Buffer = (PVOID) _INVALID_ARGUMENT;

        PCH pBuf = (PCH) _alloca(nLength * 3);
        pIoBuffer->Buffers[2].Size = ConvertUnicodeToUTF8(pBuffer,
            nLength << 1, pBuf, nLength * 3);
        pIoBuffer->Buffers[2].Buffer = pBuf;

        pIoBuffer->Buffers[3].Size = strlen(INVALID_ARGUMENT_);
        pIoBuffer->Buffers[3].Buffer = (PVOID) INVALID_ARGUMENT_;

        IO_STATUS_BLOCK IoStatus;
        return NtDeviceIoControlFile(hStdout, NULL, NULL, NULL, &IoStatus,
            IOCTL_CONDRV_ISSUE_USER_IO, pIoBuffer, sizeof(Buffer), NULL, 0);
    } else {
        PCH pBuf = (PCH) _alloca(strlen(_INVALID_ARGUMENT) +
            strlen(INVALID_ARGUMENT_) + nLength * 3);

        memcpy(pBuf, _INVALID_ARGUMENT, strlen(_INVALID_ARGUMENT));
        ULONG Length = ConvertUnicodeToUTF8(pBuffer,
            nLength << 1, &pBuf[strlen(_INVALID_ARGUMENT)], nLength * 3);
        memcpy(&pBuf[strlen(_INVALID_ARGUMENT) + Length],
            INVALID_ARGUMENT_, strlen(INVALID_ARGUMENT_));

        return WriteFileData(hStdout, pBuf,
            strlen(_INVALID_ARGUMENT) + Length + strlen(INVALID_ARGUMENT_), FALSE);
    }
}

BOOL DoesFileExists(
    PCWCH pDosName,
    PUNICODE_STRING pNtName,
    PRTL_RELATIVE_NAME_U pRelativeName
) {
    NTSTATUS NtStatus;

    // Get the NT Path
    NtStatus = RtlDosPathNameToNtPathName_U_WithStatus(pDosName, pNtName, NULL, pRelativeName);

    if (!NT_SUCCESS(NtStatus)) return FALSE;

    // Save the buffer
    PWCH pBuffer = pNtName->Buffer;

    // Check if we have a relative name
    if (pRelativeName->RelativeName.Length) {
        // Use it
        *pNtName = pRelativeName->RelativeName;
    } else {
        // Otherwise ignore it
        pRelativeName->ContainingDirectory = NULL;
    }

    OBJECT_ATTRIBUTES ObjectAttributes;

    // Initialize the object attributes
    InitializeObjectAttributes(&ObjectAttributes,
                               pNtName,
                               OBJ_CASE_INSENSITIVE,
                               pRelativeName->ContainingDirectory,
                               NULL);

    FILE_BASIC_INFORMATION BasicInformation;

    // Query the attributes and free the buffer now
    NtStatus = NtQueryAttributesFile(&ObjectAttributes, &BasicInformation);
    RtlReleaseRelativeName(pRelativeName);
    RtlFreeHeap(RtlProcessHeap(), HEAP_NO_SERIALIZE, pBuffer);

    // Check if we failed
    if (!NT_SUCCESS(NtStatus)) {
        // Check if we failed because the file is in use
        if (NtStatus == STATUS_SHARING_VIOLATION ||
            NtStatus == STATUS_ACCESS_DENIED) return TRUE;
        else return FALSE;
    }

    return TRUE;
}

//
//  Retrieve a localized system message from ntdll.dll
//  Leverage direct access to the second loaded module in the PEB loader data
//

NTSTATUS LookupNtdllMessage(DWORD dwMessageId, DWORD dwLanguageId, PMESSAGE_RESOURCE_ENTRY *MessageEntry) {
    return RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink->Flink)->DllBase,
        PtrToUlong(RT_MESSAGETABLE), dwLanguageId, dwMessageId, MessageEntry);
}

//
//  Retrieve a localized system message from KernelBase.dll
//  Leverage direct access to the fourth loaded module in the PEB loader data
//

NTSTATUS LookupSystemMessage(DWORD dwMessageId, DWORD dwLanguageId, PMESSAGE_RESOURCE_ENTRY *MessageEntry) {
    return RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) NtCurrentPeb()->Ldr->InLoadOrderModuleList.Flink->Flink->Flink)->DllBase,
        PtrToUlong(RT_MESSAGETABLE), dwLanguageId, dwMessageId, MessageEntry);
}

// Retrieve the text of a message resource entry
PCVOID GetMessageEntryText(PMESSAGE_RESOURCE_ENTRY MessageEntry) {
    return MessageEntry->Text;
}

// Retrieve the length of a message resource entry's text
WORD GetMessageEntryLength(PMESSAGE_RESOURCE_ENTRY MessageEntry) {
    // Compute text length, excluding trailing null character
    WORD wLength = MessageEntry->Length - FIELD_OFFSET(MESSAGE_RESOURCE_ENTRY, Text) - sizeof(wchar_t);
    PCWCH pText = (PCWCH) GetMessageEntryText(MessageEntry);

    // Trim additional trailing nulls
    while (pText[(wLength - 1) >> 1] == L'\0') wLength -= 2;

    return wLength;
}

// Determine whether a given handle is associated with a console
NTSTATUS IsConsoleHandle(HANDLE hHandle, PBOOL pResult) {
    IO_STATUS_BLOCK IoStatus;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

    NTSTATUS NtStatus = NtQueryVolumeInformationFile(hHandle, &IoStatus,
        &DeviceInfo, sizeof(DeviceInfo), FileFsDeviceInformation);

    *pResult = DeviceInfo.DeviceType == FILE_DEVICE_CONSOLE;
    return NtStatus;
}

// Retrieve the size of the specified module
NTSTATUS GetModuleSize(HANDLE hModule, PDWORD pFileSize) {
    IO_STATUS_BLOCK IoStatus;
    FILE_STANDARD_INFORMATION FileInfo;

    NTSTATUS NtStatus = NtQueryInformationFile(hModule, &IoStatus,
        &FileInfo, sizeof(FileInfo), FileStandardInformation);

    *pFileSize = FileInfo.EndOfFile.LowPart;
    return NtStatus;
}

// Standard pause message
#define InfiniteMessage "\nPress any key to continue ..."

// Wait for a key press, simulating standard console behavior
FORCEINLINE VOID WaitForKeyPress(HANDLE hStdout, BOOL bConsole) {
    // Display prompt
    WriteHandle(hStdout, (PCVOID) InfiniteMessage,
        strlen(InfiniteMessage), FALSE, bConsole);

    if (bConsole) SetConsoleDeviceMode(RtlStandardInput(), 0);

    BYTE Buffer;
    IO_STATUS_BLOCK IoStatus;

    // Block until input
    NtReadFile(RtlStandardInput(), NULL, NULL, NULL,
        &IoStatus, &Buffer, sizeof(Buffer), NULL, NULL);

    // Mimic input echo
    WriteHandle(hStdout, (PCVOID) InfiniteMessage, 1, FALSE, bConsole);
}