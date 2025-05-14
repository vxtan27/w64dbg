// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include "ntdll.h"

HANDLE GetMountMgrHandle() {
    UNICODE_STRING MountMgrDevice;
    MountMgrDevice.Length = wcslen(MOUNTMGR_DEVICE_NAME) * sizeof(WCHAR);
    // MountMgrDevice.MaximumLength = MountMgrDevice.Length + sizeof(UNICODE_NULL);
    MountMgrDevice.Buffer = (PWCH) &MOUNTMGR_DEVICE_NAME;

    NTSTATUS NtStatus;
    HANDLE MountMgrHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes = RTL_INIT_OBJECT_ATTRIBUTES(&MountMgrDevice, 0);
    NtStatus = NtOpenFile(&MountMgrHandle,
                          SYNCHRONIZE,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          FILE_SYNCHRONOUS_IO_NONALERT);

    if (NtStatus != STATUS_SUCCESS)
        DEBUG("NtOpenFile failed (0x%x)\n", NtStatus);

    return MountMgrHandle;
}

NTSTATUS GetDosPathNameFromVolumeDeviceName(
	PMOUNTMGR_TARGET_NAME VolumeDeviceName,
	PCH DosPathName,
    ULONG DosPathNameMaxLength,
    PULONG DosPathNameLength
) {
    PMOUNTMGR_VOLUME_PATHS VolumePath;
    BYTE VolumePathBuffer[sizeof(VolumePath->MultiSzLength) + MAX_PATH * sizeof(WCHAR)];
    VolumePath = (PMOUNTMGR_VOLUME_PATHS) &VolumePathBuffer;

    NTSTATUS NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE MountMgrHandle = GetMountMgrHandle();
    NtStatus = NtDeviceIoControlFile(MountMgrHandle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH,
                                     VolumeDeviceName,
                                     VolumeDeviceName->DeviceNameLength + sizeof(VolumeDeviceName->DeviceNameLength),
                                     VolumePath,
                                     sizeof(VolumePathBuffer));

    if (NtStatus != STATUS_SUCCESS)
        DEBUG("NtDeviceIoControlFile failed (0x%x)\n", NtStatus);

    NtClose(MountMgrHandle);

    NtStatus = RtlUnicodeToUTF8N(DosPathName,
                                 DosPathNameMaxLength,
                                 DosPathNameLength,
                                 VolumePath->MultiSz,
                                 VolumePath->MultiSzLength / sizeof(WCHAR));

    if (NtStatus != STATUS_SUCCESS)
        DEBUG("RtlUnicodeToUTF8N failed (0x%x)\n", NtStatus);

    return NtStatus;
}

NTSTATUS GetDosPathFromHandle(
    HANDLE hFile,
    PCH DosPath,
    ULONG DosPathMaxLength,
    PULONG DosPathLength
) {
    BYTE ObjectNameInfoBuffer[sizeof(OBJECT_NAME_INFORMATION) + UNICODE_STRING_MAX_BYTES];
    POBJECT_NAME_INFORMATION ObjectNameInfo = (POBJECT_NAME_INFORMATION) &ObjectNameInfoBuffer;

    NTSTATUS NtStatus;
    NtStatus = NtQueryObject(hFile,
                             ObjectNameInformation,
                             ObjectNameInfo,
                             sizeof(ObjectNameInfoBuffer),
                             NULL);

    if (NtStatus != STATUS_SUCCESS)
        DEBUG("NtQueryObject failed (0x%x)\n", NtStatus);

    IO_STATUS_BLOCK IoStatusBlock;
    FILE_NAME_INFORMATION FileNameInfo;
    NtStatus = NtQueryInformationFile(hFile,
                                      &IoStatusBlock,
                                      &FileNameInfo,
                                      sizeof(FileNameInfo),
                                      FileNameInformation);

    if (NtStatus != STATUS_BUFFER_OVERFLOW)
        DEBUG("NtQueryInformationFile failed (0x%x)\n", NtStatus);

    PMOUNTMGR_TARGET_NAME VolumeDeviceName = (PMOUNTMGR_TARGET_NAME) (&ObjectNameInfo->Name.Length + 7);
    VolumeDeviceName->DeviceNameLength = ObjectNameInfo->Name.Length - FileNameInfo.FileNameLength;

    ULONG DosPathNameLength;
    NtStatus = GetDosPathNameFromVolumeDeviceName(VolumeDeviceName,
                                                  DosPath,
                                                  DosPathMaxLength,
                                                  &DosPathNameLength);

    NTSTATUS NtStatus2;
    NtStatus2 = RtlUnicodeToUTF8N(DosPath + DosPathNameLength,
                                  DosPathMaxLength - DosPathNameLength,
                                  DosPathLength,
                                  (PWCH) &ObjectNameInfo->Name.Length + 8 + (VolumeDeviceName->DeviceNameLength / sizeof(WCHAR)),
                                  FileNameInfo.FileNameLength);

    if (NtStatus2 != STATUS_SUCCESS)
        DEBUG("RtlUnicodeToUTF8N failed (0x%x)\n", NtStatus2);

    *DosPathLength += DosPathNameLength;

    return NtStatus;
}