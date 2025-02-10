/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#include <winternl.h>

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

typedef struct _ZWPEB_LDR_DATA
{
    ULONG Length;
    BOOLEAN Initialized;
    HANDLE SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID EntryInProgress;
    BOOLEAN ShutdownInProgress;
    HANDLE ShutdownThreadId;
} ZWPEB_LDR_DATA, *PZWPEB_LDR_DATA;

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

typedef struct _FILE_FS_DEVICE_INFORMATION
{
    DEVICE_TYPE DeviceType;
    ULONG Characteristics;
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;

typedef enum _FSINFOCLASS
{
    FileFsVolumeInformation = 1, // q: FILE_FS_VOLUME_INFORMATION
    FileFsLabelInformation, // s: FILE_FS_LABEL_INFORMATION (requires FILE_WRITE_DATA to volume)
    FileFsSizeInformation, // q: FILE_FS_SIZE_INFORMATION
    FileFsDeviceInformation, // q: FILE_FS_DEVICE_INFORMATION
    FileFsAttributeInformation, // q: FILE_FS_ATTRIBUTE_INFORMATION
    FileFsControlInformation, // q, s: FILE_FS_CONTROL_INFORMATION  (q: requires FILE_READ_DATA; s: requires FILE_WRITE_DATA to volume)
    FileFsFullSizeInformation, // q: FILE_FS_FULL_SIZE_INFORMATION
    FileFsObjectIdInformation, // q; s: FILE_FS_OBJECTID_INFORMATION (s: requires FILE_WRITE_DATA to volume)
    FileFsDriverPathInformation, // q: FILE_FS_DRIVER_PATH_INFORMATION
    FileFsVolumeFlagsInformation, // q; s: FILE_FS_VOLUME_FLAGS_INFORMATION (q: requires FILE_READ_ATTRIBUTES; s: requires FILE_WRITE_ATTRIBUTES to volume) // 10
    FileFsSectorSizeInformation, // q: FILE_FS_SECTOR_SIZE_INFORMATION // since WIN8
    FileFsDataCopyInformation, // q: FILE_FS_DATA_COPY_INFORMATION
    FileFsMetadataSizeInformation, // q: FILE_FS_METADATA_SIZE_INFORMATION // since THRESHOLD
    FileFsFullSizeInformationEx, // q: FILE_FS_FULL_SIZE_INFORMATION_EX // since REDSTONE5
    FileFsGuidInformation, // q: FILE_FS_GUID_INFORMATION // since 23H2
    FileFsMaximumInformation
} FSINFOCLASS, *PFSINFOCLASS;
typedef enum _FSINFOCLASS FS_INFORMATION_CLASS;

/**
 * The FILE_BASIC_INFORMATION structure contains timestamps and basic attributes of a file.
 * \li If you specify a value of zero for any of the XxxTime members, the file system keeps a file's current value for that time.
 * \li If you specify a value of -1 for any of the XxxTime members, time stamp updates are disabled for I/O operations preformed on the file handle.
 * \li If you specify a value of -2 for any of the XxxTime members, time stamp updates are enabled for I/O operations preformed on the file handle.
 * \remarks To set the members of this structure, the caller must have FILE_WRITE_ATTRIBUTES access to the file.
 */
typedef struct _FILE_BASIC_INFORMATION
{
    LARGE_INTEGER CreationTime;         // Specifies the time that the file was created.
    LARGE_INTEGER LastAccessTime;       // Specifies the time that the file was last accessed.
    LARGE_INTEGER LastWriteTime;        // Specifies the time that the file was last written to.
    LARGE_INTEGER ChangeTime;           // Specifies the last time the file was changed.
    ULONG FileAttributes;               // Specifies one or more FILE_ATTRIBUTE_XXX flags.
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _PS_ATTRIBUTE
{
    ULONG_PTR Attribute;
    SIZE_T    Size;
    union
    {
        ULONG_PTR Value;
        void     *ValuePtr;
    };
    SIZE_T *ReturnLength;
} PS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST
{
    SIZE_T       TotalLength;
    PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

#define THREAD_CREATE_FLAGS_NONE 0x00000000
#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED 0x00000001 // NtCreateUserProcess & NtCreateThreadEx
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH 0x00000002 // NtCreateThreadEx only
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004 // NtCreateThreadEx only
#define THREAD_CREATE_FLAGS_LOADER_WORKER 0x00000010 // NtCreateThreadEx only // since THRESHOLD
#define THREAD_CREATE_FLAGS_SKIP_LOADER_INIT 0x00000020 // NtCreateThreadEx only // since REDSTONE2
#define THREAD_CREATE_FLAGS_BYPASS_PROCESS_FREEZE 0x00000040 // NtCreateThreadEx only // since 19H1

typedef void (CALLBACK *PUSER_THREAD_START_ROUTINE)(LPVOID);

typedef enum _WAIT_TYPE
{
    WaitAll,
    WaitAny,
    WaitNotification,
    WaitDequeue,
    WaitDpc,
} WAIT_TYPE;

#define RTL_DOS_SEARCH_PATH_FLAG_APPLY_ISOLATION_REDIRECTION 0x00000001
#define RTL_DOS_SEARCH_PATH_FLAG_DISALLOW_DOT_RELATIVE_PATH_SEARCH 0x00000002
#define RTL_DOS_SEARCH_PATH_FLAG_APPLY_DEFAULT_EXTENSION_WHEN_NOT_RELATIVE_PATH_EVEN_IF_FILE_HAS_EXTENSION 0x00000004

typedef struct _FILE_STANDARD_INFORMATION
{
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

#ifdef __cplusplus
extern "C" {
#endif

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateJobObject(
    _Out_ PHANDLE JobHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateThreadEx(
    _Out_ PHANDLE ThreadHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_ HANDLE ProcessHandle,
    _In_ PUSER_THREAD_START_ROUTINE StartRoutine,
    _In_opt_ PVOID Argument,
    _In_ ULONG CreateFlags, // THREAD_CREATE_FLAGS_*
    _In_ SIZE_T ZeroBits,
    _In_ SIZE_T StackSize,
    _In_ SIZE_T MaximumStackSize,
    _In_opt_ PPS_ATTRIBUTE_LIST AttributeList
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDelayExecution(
    _In_ BOOLEAN Alertable,
    _In_ PLARGE_INTEGER DelayInterval
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetContextThread(
    _In_ HANDLE ThreadHandle,
    _Inout_ PCONTEXT ThreadContext
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryAttributesFile(
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _Out_ PFILE_BASIC_INFORMATION FileInformation
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationFile(
    _In_ HANDLE FileHandle,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryVolumeInformationFile(
    _In_ HANDLE FileHandle,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID FsInformation,
    _In_ ULONG Length,
    _In_ FSINFOCLASS FsInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadFile(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _Out_writes_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
    _In_opt_ PULONG Key
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadVirtualMemory(
    _In_ HANDLE ProcessHandle,
    _In_opt_ PVOID BaseAddress,
    _Out_writes_bytes_to_(NumberOfBytesToRead, *NumberOfBytesRead) PVOID Buffer,
    _In_ SIZE_T NumberOfBytesToRead,
    _Out_opt_ PSIZE_T NumberOfBytesRead
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtResumeProcess(
    _In_ HANDLE ProcessHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationFile(
    _In_ HANDLE FileHandle,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID FileInformation,
    _In_ ULONG Length,
    _In_ FILE_INFORMATION_CLASS FileInformationClass
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationJobObject(
    _In_ HANDLE JobHandle,
    _In_ JOBOBJECTINFOCLASS JobObjectInformationClass,
    _In_reads_bytes_(JobObjectInformationLength) PVOID JobObjectInformation,
    _In_ ULONG JobObjectInformationLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSuspendProcess(
    _In_ HANDLE ProcessHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtTerminateProcess(
    _In_opt_ HANDLE ProcessHandle,
    _In_ NTSTATUS ExitStatus
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitForMultipleObjects(
    _In_ ULONG Count,
    _In_reads_(Count) HANDLE Handles[],
    _In_ WAIT_TYPE WaitType,
    _In_ BOOLEAN Alertable,
    _In_opt_ PLARGE_INTEGER Timeout
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteFile(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_reads_bytes_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_opt_ PLARGE_INTEGER ByteOffset,
    _In_opt_ PULONG Key
    );

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
_Must_inspect_result_
NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeStrings(
    _In_reads_(String1Length) PCWCH String1,
    _In_ SIZE_T String1Length,
    _In_reads_(String2Length) PCWCH String2,
    _In_ SIZE_T String2Length,
    _In_ BOOLEAN CaseInSensitive
    );
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlDosSearchPath_Ustr(
    _In_ ULONG Flags,
    _In_ PUNICODE_STRING Path,
    _In_ PUNICODE_STRING FileName,
    _In_opt_ PUNICODE_STRING DefaultExtension,
    _Out_opt_ PUNICODE_STRING StaticString,
    _Out_opt_ PUNICODE_STRING DynamicString,
    _Out_opt_ PCUNICODE_STRING *FullFileNameOut,
    _Out_opt_ SIZE_T *FilePartPrefixCch,
    _Out_opt_ SIZE_T *BytesRequired
    );

#if _WIN32_WINNT >= _WIN32_WINNT_WINXP
_Analysis_noreturn_
DECLSPEC_NORETURN
NTSYSAPI
VOID
NTAPI
RtlExitUserThread(
    _In_ NTSTATUS ExitStatus
    );
#endif

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
_Analysis_noreturn_
DECLSPEC_NORETURN
NTSYSAPI
VOID
NTAPI
RtlExitUserProcess(
    _In_ NTSTATUS ExitStatus
    );
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlFindMessage(
    _In_ PVOID DllHandle,
    _In_ ULONG MessageTableId,
    _In_ ULONG MessageLanguageId,
    _In_ ULONG MessageId,
    _Out_ PMESSAGE_RESOURCE_ENTRY *MessageEntry
    );

#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
// private
NTSYSAPI
NTSTATUS
NTAPI
RtlQueryEnvironmentVariable(
    _In_opt_ PVOID Environment,
    _In_reads_(NameLength) PCWSTR Name,
    _In_ SIZE_T NameLength,
    _Out_writes_opt_(ValueLength) PWSTR Value,
    _In_opt_ SIZE_T ValueLength,
    _Out_ PSIZE_T ReturnLength
    );
#endif

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

#ifdef __cplusplus
}
#endif

static __forceinline VOID FindNativeMessage(
    PPEB ProcessEnvironmentBlock,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    PMESSAGE_RESOURCE_ENTRY *Entry
    )
{
    RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) ((PZWPEB_LDR_DATA) ProcessEnvironmentBlock->Ldr)->InLoadOrderModuleList.
        Flink->Flink)->DllBase, (ULONG)(ULONG_PTR) RT_MESSAGETABLE, dwLanguageId, dwMessageId, Entry);
}

static __forceinline VOID FindCoreMessage(
    PPEB ProcessEnvironmentBlock,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    PMESSAGE_RESOURCE_ENTRY *Entry
    )
{
    RtlFindMessage(((PLDR_DATA_TABLE_ENTRY) ((PZWPEB_LDR_DATA) ProcessEnvironmentBlock->Ldr)->InLoadOrderModuleList.
        Flink->Flink->Flink->Flink)->DllBase, (ULONG)(ULONG_PTR) RT_MESSAGETABLE, dwLanguageId, dwMessageId, Entry);
}