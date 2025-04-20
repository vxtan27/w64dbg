// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"
#include <utility>

HANDLE GetMountMgrHandle() {
    HANDLE MountMgrHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING MountMgrDevice = RTL_CONSTANT_STRING(MOUNTMGR_DEVICE_NAME);
    OBJECT_ATTRIBUTES ObjectAttributes = RTL_INIT_OBJECT_ATTRIBUTES(&MountMgrDevice, 0);

    NTSTATUS NtStatus = NtOpenFile(&MountMgrHandle, SYNCHRONIZE, 
        &ObjectAttributes, &IoStatusBlock, FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (NtStatus != STATUS_SUCCESS) {
        printf("NtOpenFile failed (0x%x)", NtStatus);
        std::unreachable();
    }

    return MountMgrHandle;
}

NTSTATUS GetDosPathNameFromVolumeDeviceName(
	PUNICODE_STRING VolumeDeviceName,
	PCH DosPathName,
    ULONG DosPathNameMaxLength,
    PULONG DosPathNameLength
) {
    PMOUNTMGR_VOLUME_PATHS VolumePath;
    BYTE VolumePathBuffer[MAX_PATH * sizeof(WCHAR) + sizeof(VolumePath->MultiSzLength)];
    VolumePath = (PMOUNTMGR_VOLUME_PATHS) &VolumePathBuffer;

    PMOUNTMGR_TARGET_NAME TargetName = (PMOUNTMGR_TARGET_NAME) &VolumeDeviceName->MaximumLength + 3;
    TargetName->DeviceNameLength = VolumeDeviceName->Length;

    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE MountMgrHandle = GetMountMgrHandle();
    NTSTATUS NtStatus = NtDeviceIoControlFile(MountMgrHandle, NULL,
    	NULL, NULL, &IoStatusBlock, IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH,
    	TargetName, TargetName->DeviceNameLength + sizeof(TargetName->DeviceNameLength),
        VolumePath, sizeof(VolumePathBuffer));

    if (NtStatus != STATUS_SUCCESS) {
        printf("NtDeviceIoControlFile failed (0x%x)", NtStatus);
        std::unreachable();
    }

    NtClose(MountMgrHandle);
    RtlUnicodeToUTF8N(DosPathName, DosPathNameMaxLength, DosPathNameLength,
        VolumePath->MultiSz, VolumePath->MultiSzLength - 4);

    return NtStatus;
}

NTSTATUS GetDosPathFromHandle(
    HANDLE hFile,
    PCH DosPath,
    ULONG DosPathMaxLength,
    PULONG DosPathLength
) {
    BYTE NameInfoBuffer[sizeof(OBJECT_NAME_INFORMATION) + UNICODE_STRING_MAX_BYTES];
    POBJECT_NAME_INFORMATION NameInfo = (POBJECT_NAME_INFORMATION) &NameInfoBuffer;
    NtQueryObject(hFile, ObjectNameInformation, NameInfo, sizeof(NameInfoBuffer), NULL);

    IO_STATUS_BLOCK IoStatusBlock;
    FILE_NAME_INFORMATION FileNameInfo;
    NtQueryInformationFile(hFile, &IoStatusBlock, &FileNameInfo,
        sizeof(FileNameInfo), FileNameInformation);
    NameInfo->Name.Length -= FileNameInfo.FileNameLength;

    ULONG DosPathNameLength;
    NTSTATUS NtStatus = GetDosPathNameFromVolumeDeviceName(&NameInfo->Name,
                                                       DosPath,
                                                       DosPathMaxLength,
                                                       &DosPathNameLength);

    RtlUnicodeToUTF8N(DosPath + DosPathNameLength,
                      DosPathMaxLength - DosPathNameLength,
                      DosPathLength,
                      (PWCH) &NameInfo->Name.Length + 8 + (NameInfo->Name.Length >> 1),
                      FileNameInfo.FileNameLength);
    *DosPathLength += DosPathNameLength;

    return NtStatus;
}