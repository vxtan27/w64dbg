// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include <ntdll.h>
#include <conapi.h>
#include <kernelbase.h>

ULONG ConvertUnicodeToUTF8(
    _In_reads_bytes_(cbUnicodeString) PCVOID pUnicodeString,
    _In_ ULONG cbUnicodeString,
    _Out_ PVOID pUTF8String,
    _In_ ULONG cbUTF8String
) {
    ULONG cchUTF8String;

    RtlUnicodeToUTF8N((PCH) pUTF8String, cbUTF8String, &cchUTF8String,
        (PCWCH) pUnicodeString, cbUnicodeString);
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
    _In_ BOOL fConsole
) {
    return fConsole ? WriteConsoleDevice(hHandle, pBuffer, uLength, fUnicode)
                    : WriteFileData(hHandle, pBuffer, uLength, fUnicode);
}

NTSTATUS WriteInvalidArgument(
    _In_ HANDLE hStdout,
    _In_reads_(nLength) PCWCH pBuffer,
    _In_ ULONG nLength,
    _In_ BOOL fConsole
) {
    if (fConsole) {
        BYTE Buffer[sizeof(CD_USER_DEFINED_IO) + sizeof(CD_IO_BUFFER) * 3];
        PCD_USER_DEFINED_IO pIoBuffer = (PCD_USER_DEFINED_IO) &Buffer;

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

NTSTATUS
InitializeDebugProcess(
    _Out_ PHANDLE hProcess,
    _Out_ PHANDLE hThread,
    _In_ PWCH pApplicationName,
    _In_ USHORT szApplicationName,
    _In_ PWCH pCommandLine,
    _In_ USHORT szCommandLine,
    PSECTION_IMAGE_INFORMATION SectionImageInfomation
) {
    UNICODE_STRING CapturedDosName;
    CapturedDosName.Length = szApplicationName;
    CapturedDosName.Buffer = pApplicationName;

    WCHAR Buffer[MAX_PATH + 4];
    UNICODE_STRING ApplicationName;
    ApplicationName.MaximumLength = sizeof(Buffer) - 8;
    ApplicationName.Buffer = Buffer + 4;
    memcpy(Buffer, "\\\0?\0?\0\\", 8); // NT prefix

    RTL_PATH_TYPE InputPathType;
    RtlGetFullPathName_UstrEx(&CapturedDosName,
                              &ApplicationName,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              &InputPathType,
                              NULL);

    UNICODE_STRING CommandLine;
    CommandLine.Length = szCommandLine;
    CommandLine.MaximumLength = CommandLine.Length;
    CommandLine.Buffer = pCommandLine;

    PS_CREATE_INFO CreateInfo;
    CreateInfo.State = PsCreateInitialState;
    CreateInfo.Size = sizeof(CreateInfo);
    CreateInfo.InitState.WriteOutputOnExit = TRUE;
    CreateInfo.InitState.DetectManifest = TRUE;
    CreateInfo.InitState.IFEOSkipDebugger = TRUE;
    CreateInfo.InitState.IFEODoNotPropagateKeyState = TRUE;
    CreateInfo.InitState.ProhibitedImageCharacteristics = IMAGE_FILE_DLL;
    CreateInfo.InitState.AdditionalFileAccess = FILE_READ_ATTRIBUTES | FILE_READ_DATA;

    BYTE _AttributeList[sizeof(PS_ATTRIBUTE_LIST) + sizeof(PS_ATTRIBUTE) * 3];
    PPS_ATTRIBUTE_LIST AttributeList = (PPS_ATTRIBUTE_LIST) &_AttributeList;
    AttributeList->TotalLength = sizeof(_AttributeList);

    AttributeList->Attributes[0].Attribute = PS_ATTRIBUTE_IMAGE_NAME;
    AttributeList->Attributes[0].Size = ApplicationName.Length + 8;
    AttributeList->Attributes[0].ValuePtr = Buffer; // ApplicationName.Buffer - 8
    AttributeList->Attributes[0].ReturnLength = 0;

    // CLIENT_ID ClientId;
    // AttributeList->Attributes[1].Attribute = PS_ATTRIBUTE_CLIENT_ID;
    // AttributeList->Attributes[1].Size = sizeof(ClientId);
    // AttributeList->Attributes[1].ValuePtr = &ClientId;
    // AttributeList->Attributes[1].ReturnLength = 0;

    AttributeList->Attributes[1].Attribute = PS_ATTRIBUTE_IMAGE_INFO;
    AttributeList->Attributes[1].Size = sizeof(SECTION_IMAGE_INFORMATION);
    AttributeList->Attributes[1].ValuePtr = SectionImageInfomation;
    AttributeList->Attributes[1].ReturnLength = 0;

    DbgConnectToDbg();
    AttributeList->Attributes[2].Attribute = PS_ATTRIBUTE_DEBUG_OBJECT;
    AttributeList->Attributes[2].Size = sizeof(HANDLE);
    AttributeList->Attributes[2].ValuePtr = DbgGetThreadDebugObject();
    AttributeList->Attributes[2].ReturnLength = 0;

    PS_STD_HANDLE_INFO StdHandle;
    StdHandle.StdHandleState = PsRequestDuplicate;
    StdHandle.PseudoHandleMask = 0;
    StdHandle.StdHandleSubsystemType = IMAGE_SUBSYSTEM_WINDOWS_CUI;
    AttributeList->Attributes[3].Attribute = PS_ATTRIBUTE_STD_HANDLE_INFO;
    AttributeList->Attributes[3].Size = sizeof(PS_STD_HANDLE_INFO);
    AttributeList->Attributes[3].ValuePtr = &StdHandle;
    AttributeList->Attributes[3].ReturnLength = 0;

    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    RtlCreateProcessParametersEx(&ProcessParameters, &ApplicationName, NULL, NULL,
        &CommandLine, NULL, NULL, NULL, NULL, NULL, RTL_USER_PROC_PARAMS_NORMALIZED);
    ProcessParameters->ProcessGroupId = NtCurrentPeb()->ProcessParameters->ProcessGroupId;
    ProcessParameters->ConsoleHandle = BaseGetConsoleReference();

    NTSTATUS NtStatus = NtCreateUserProcess(hProcess,
        hThread, MAXIMUM_ALLOWED, MAXIMUM_ALLOWED, NULL, NULL,
        PROCESS_CREATE_FLAGS_BREAKAWAY | PROCESS_CREATE_FLAGS_NO_DEBUG_INHERIT,
        THREAD_CREATE_FLAGS_NONE, ProcessParameters, &CreateInfo, AttributeList);

    RtlDestroyProcessParameters(ProcessParameters);

    if (NT_SUCCESS(NtStatus)) {
        NtClose(CreateInfo.SuccessState.FileHandle);
        NtClose(CreateInfo.SuccessState.SectionHandle);
        return STATUS_SUCCESS;
    }

    if (CreateInfo.State == PsCreateFailOnSectionCreate)
        NtClose(CreateInfo.FailSection.FileHandle);

    return NtStatus;
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
FORCEINLINE VOID WaitForKeyPress(HANDLE hStdout, BOOL fConsole) {
    // Display prompt
    WriteHandle(hStdout, (PCVOID) InfiniteMessage,
        strlen(InfiniteMessage), FALSE, fConsole);

    if (fConsole) SetConsoleDeviceMode(RtlStandardInput(), 0);

    BYTE Buffer;
    IO_STATUS_BLOCK IoStatus;

    // Block until input
    NtReadFile(RtlStandardInput(), NULL, NULL, NULL,
        &IoStatus, &Buffer, sizeof(Buffer), NULL, NULL);

    // Mimic input echo
    WriteHandle(hStdout, (PCVOID) InfiniteMessage, 1, FALSE, fConsole);
}