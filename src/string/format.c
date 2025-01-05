/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "..\ntdll.h"
#include "..\resrc.h"
#include "format.h"
#include "conversion.c"

static
inline
char* FormatDebugException(
    const PEXCEPTION_RECORD64 ExceptionRecord,
    char* p,
    DWORD bx64win
)
{
    switch (ExceptionRecord->ExceptionCode)
    {
        case 0xC0000005: // EXCEPTION_ACCESS_VIOLATION
        case 0xC0000006: // EXCEPTION_IN_PAGE_ERROR
            if (ExceptionRecord->ExceptionCode == 0xC0000005)
            { // EXCEPTION_ACCESS_VIOLATION
                memcpy(p, _EXCEPTION_ACCESS_VIOLATION,
                    sizeof(_EXCEPTION_ACCESS_VIOLATION));
                p += sizeof(_EXCEPTION_ACCESS_VIOLATION);
            } else
            { // EXCEPTION_IN_PAGE_ERROR
                memcpy(p, _EXCEPTION_IN_PAGE_ERROR,
                    sizeof(_EXCEPTION_IN_PAGE_ERROR));
                p += sizeof(_EXCEPTION_IN_PAGE_ERROR);
            }

            *p++ = ' ';

            if (!ExceptionRecord->ExceptionInformation[0])
            {
                memcpy(p, EXCEPTION_READING_FROM,
                    sizeof(EXCEPTION_READING_FROM));
                p += sizeof(EXCEPTION_READING_FROM);
            } else if (ExceptionRecord->ExceptionInformation[0] == 1)
            {
                memcpy(p, EXCEPTION_WRITING_TO,
                    sizeof(EXCEPTION_WRITING_TO));
                p += sizeof(EXCEPTION_WRITING_TO);
            } else
            {
                memcpy(p, EXCEPTION_DEP_VIOLATION,
                    sizeof(EXCEPTION_DEP_VIOLATION));
                p += sizeof(EXCEPTION_DEP_VIOLATION);
            }

            memcpy(p, EXCEPTION_LOCATION,
                sizeof(EXCEPTION_LOCATION));
            p = _ui64toa16(ExceptionRecord->ExceptionInformation[1],
                p + sizeof(EXCEPTION_LOCATION), bx64win);
            break;
        case 0xC0000008: // EXCEPTION_INVALID_HANDLE
            memcpy(p, _EXCEPTION_INVALID_HANDLE,
                sizeof(_EXCEPTION_INVALID_HANDLE));
            p += sizeof(_EXCEPTION_INVALID_HANDLE);
            break;
        case 0xC000001D: // EXCEPTION_ILLEGAL_INSTRUCTION
            memcpy(p, _EXCEPTION_ILLEGAL_INSTRUCTION,
                sizeof(_EXCEPTION_ILLEGAL_INSTRUCTION));
            p += sizeof(_EXCEPTION_ILLEGAL_INSTRUCTION);
            break;
        case 0xC0000026: // EXCEPTION_INVALID_DISPOSITION
            memcpy(p, _EXCEPTION_INVALID_DISPOSITION,
                sizeof(_EXCEPTION_INVALID_DISPOSITION));
            p += sizeof(_EXCEPTION_INVALID_DISPOSITION);
            break;
        case 0xC000008C: // EXCEPTION_ARRAY_BOUNDS_EXCEEDED
            memcpy(p, _EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
                sizeof(_EXCEPTION_ARRAY_BOUNDS_EXCEEDED));
            p += sizeof(_EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
            break;
        case 0xC000008D: // EXCEPTION_FLT_DENORMAL_OPERAND
            memcpy(p, _EXCEPTION_FLT_DENORMAL_OPERAND,
                sizeof(_EXCEPTION_FLT_DENORMAL_OPERAND));
            p += sizeof(_EXCEPTION_FLT_DENORMAL_OPERAND);
            break;
        case 0xC000008E: // EXCEPTION_FLT_DIVIDE_BY_ZERO
            memcpy(p, _EXCEPTION_FLT_DIVIDE_BY_ZERO,
                sizeof(_EXCEPTION_FLT_DIVIDE_BY_ZERO));
            p += sizeof(_EXCEPTION_FLT_DIVIDE_BY_ZERO);
            break;
        case 0xC000008F: // EXCEPTION_FLT_INEXACT_RESULT
            memcpy(p, _EXCEPTION_FLT_INEXACT_RESULT,
                sizeof(_EXCEPTION_FLT_INEXACT_RESULT));
            p += sizeof(_EXCEPTION_FLT_INEXACT_RESULT);
            break;
        case 0xC0000090: // EXCEPTION_FLT_INVALID_OPERATION
            memcpy(p, _EXCEPTION_FLT_INVALID_OPERATION,
                sizeof(_EXCEPTION_FLT_INVALID_OPERATION));
            p += sizeof(_EXCEPTION_FLT_INVALID_OPERATION);
            break;
        case 0xC0000091: // EXCEPTION_FLT_OVERFLOW
            memcpy(p, _EXCEPTION_FLT_OVERFLOW,
                sizeof(_EXCEPTION_FLT_OVERFLOW));
            p += sizeof(_EXCEPTION_FLT_OVERFLOW);
            break;
        case 0xC0000092: // EXCEPTION_FLT_STACK_CHECK
            memcpy(p, _EXCEPTION_FLT_STACK_CHECK,
                sizeof(_EXCEPTION_FLT_STACK_CHECK));
            p += sizeof(_EXCEPTION_FLT_STACK_CHECK);
            break;
        case 0xC0000093: // EXCEPTION_FLT_UNDERFLOW
            memcpy(p, _EXCEPTION_FLT_UNDERFLOW,
                sizeof(_EXCEPTION_FLT_UNDERFLOW));
            p += sizeof(_EXCEPTION_FLT_UNDERFLOW);
            break;
        case 0xC0000094: // EXCEPTION_INT_DIVIDE_BY_ZERO
            memcpy(p, _EXCEPTION_INT_DIVIDE_BY_ZERO,
                sizeof(_EXCEPTION_INT_DIVIDE_BY_ZERO));
            p += sizeof(_EXCEPTION_INT_DIVIDE_BY_ZERO);
            break;
        case 0xC0000095: // EXCEPTION_INT_OVERFLOW
            memcpy(p, _EXCEPTION_INT_OVERFLOW,
                sizeof(_EXCEPTION_INT_OVERFLOW));
            p += sizeof(_EXCEPTION_INT_OVERFLOW);
            break;
        case 0xC0000096: // EXCEPTION_PRIV_INSTRUCTION
            memcpy(p, _EXCEPTION_PRIV_INSTRUCTION,
                sizeof(_EXCEPTION_PRIV_INSTRUCTION));
            p += sizeof(_EXCEPTION_PRIV_INSTRUCTION);
            break;
        case 0xC00000FD: // EXCEPTION_STACK_OVERFLOW
            memcpy(p, _EXCEPTION_STACK_OVERFLOW,
                sizeof(_EXCEPTION_STACK_OVERFLOW));
            p += sizeof(_EXCEPTION_STACK_OVERFLOW);
            break;
        case 0xC0000194: // EXCEPTION_POSSIBLE_DEADLOCK
            memcpy(p, _EXCEPTION_POSSIBLE_DEADLOCK,
                sizeof(_EXCEPTION_POSSIBLE_DEADLOCK));
            p += sizeof(_EXCEPTION_POSSIBLE_DEADLOCK);
            break;
        case 0xC0000409: // STATUS_STACK_BUFFER_OVERRUN
            memcpy(p, _STATUS_STACK_BUFFER_OVERRUN,
                sizeof(_STATUS_STACK_BUFFER_OVERRUN));
            p += sizeof(_STATUS_STACK_BUFFER_OVERRUN);
            break;
        case 0xC000041D: // STATUS_FATAL_USER_CALLBACK_EXCEPTION
            memcpy(p, _STATUS_FATAL_USER_CALLBACK_EXCEPTION,
                sizeof(_STATUS_FATAL_USER_CALLBACK_EXCEPTION));
            p += sizeof(_STATUS_FATAL_USER_CALLBACK_EXCEPTION);
            break;
        case 0xC0000420: // STATUS_ASSERTION_FAILURE
            memcpy(p, _STATUS_ASSERTION_FAILURE,
                sizeof(_STATUS_ASSERTION_FAILURE));
            p += sizeof(_STATUS_ASSERTION_FAILURE);
            break;
        case 0xE0434f4D: // STATUS_CLR_EXCEPTION
            memcpy(p, _STATUS_CLR_EXCEPTION,
                sizeof(_STATUS_CLR_EXCEPTION));
            p += sizeof(_STATUS_CLR_EXCEPTION);
            break;
        case 0xE06D7363: // STATUS_CPP_EH_EXCEPTION
            memcpy(p, _STATUS_CPP_EH_EXCEPTION,
                sizeof(_STATUS_CPP_EH_EXCEPTION));
            p += sizeof(_STATUS_CPP_EH_EXCEPTION);
            break;
        case 0x80000001: // EXCEPTION_GUARD_PAGE
            memcpy(p, _EXCEPTION_GUARD_PAGE,
                sizeof(_EXCEPTION_GUARD_PAGE));
            p += sizeof(_EXCEPTION_GUARD_PAGE);
            break;
        case 0x80000002: // EXCEPTION_DATATYPE_MISALIGNMENT
            memcpy(p, _EXCEPTION_DATATYPE_MISALIGNMENT,
                sizeof(_EXCEPTION_DATATYPE_MISALIGNMENT));
            p += sizeof(_EXCEPTION_DATATYPE_MISALIGNMENT);
            break;
        case 0x80000003: // EXCEPTION_BREAKPOINT
        case 0x4000001F: // STATUS_WX86_BREAKPOINT
            memcpy(p, _EXCEPTION_BREAKPOINT,
                sizeof(_EXCEPTION_BREAKPOINT));
            p += sizeof(_EXCEPTION_BREAKPOINT);
            break;
        case 0x80000004: // EXCEPTION_SINGLE_STEP
            memcpy(p, _EXCEPTION_SINGLE_STEP,
                sizeof(_EXCEPTION_SINGLE_STEP));
            p += sizeof(_EXCEPTION_SINGLE_STEP);
            break;
        case 0x40000015: // STATUS_FATAL_APP_EXIT
            memcpy(p, _STATUS_FATAL_APP_EXIT,
                sizeof(_STATUS_FATAL_APP_EXIT));
            p += sizeof(_STATUS_FATAL_APP_EXIT);
            break;
        case 0x406D1388: // https://learn.microsoft.com/visualstudio/debugger/tips-for-debugging-threads
            memcpy(p, _MS_VC_EXCEPTION,
                sizeof(_MS_VC_EXCEPTION));
            p += sizeof(_MS_VC_EXCEPTION);
            break;
        case 1717: // RPC_S_UNKNOWN_IF
            memcpy(p, _RPC_S_UNKNOWN_IF,
                sizeof(_RPC_S_UNKNOWN_IF));
            p += sizeof(_RPC_S_UNKNOWN_IF);
            break;
        case 1722: // RPC_S_SERVER_UNAVAILABLE
            memcpy(p, _RPC_S_SERVER_UNAVAILABLE,
                sizeof(_RPC_S_SERVER_UNAVAILABLE));
            p += sizeof(_RPC_S_SERVER_UNAVAILABLE);
            break;
        default:
            memcpy(p, _EXCEPTION_UNKNOWN,
                sizeof(_EXCEPTION_UNKNOWN));
            p = _ultoa16(ExceptionRecord->ExceptionCode,
                p + sizeof(_EXCEPTION_UNKNOWN));
            *p++ = ')';
    }

    return p;
}

static
inline
char* FormatVerboseDebugException(
    char* p,
    DWORD ExceptionCode
)
{
    switch (ExceptionCode)
    {
        case 0xC0000005: // EXCEPTION_ACCESS_VIOLATION
            memcpy(p, EXCEPTION_ACCESS_VIOLATION_,
                sizeof(EXCEPTION_ACCESS_VIOLATION_));
            p += sizeof(EXCEPTION_ACCESS_VIOLATION_);
            break;
        case 0xC0000006: // EXCEPTION_IN_PAGE_ERROR
            memcpy(p, _EXCEPTION_IN_PAGE_ERROR,
                sizeof(_EXCEPTION_IN_PAGE_ERROR));
            p += sizeof(_EXCEPTION_IN_PAGE_ERROR);
            break;
        case 0xC0000008: // EXCEPTION_INVALID_HANDLE
            memcpy(p, EXCEPTION_INVALID_HANDLE_,
                sizeof(EXCEPTION_INVALID_HANDLE_));
            p += sizeof(EXCEPTION_INVALID_HANDLE_);
            break;
        case 0xC000001D: // EXCEPTION_ILLEGAL_INSTRUCTION
            memcpy(p, EXCEPTION_ILLEGAL_INSTRUCTION_,
                sizeof(EXCEPTION_ILLEGAL_INSTRUCTION_));
            p += sizeof(EXCEPTION_ILLEGAL_INSTRUCTION_);
            break;
        case 0xC0000026: // EXCEPTION_INVALID_DISPOSITION
            memcpy(p, EXCEPTION_INVALID_DISPOSITION_,
                sizeof(EXCEPTION_INVALID_DISPOSITION_));
            p += sizeof(EXCEPTION_INVALID_DISPOSITION_);
            break;
        case 0xC000008C: // EXCEPTION_ARRAY_BOUNDS_EXCEEDED
            memcpy(p, EXCEPTION_ARRAY_BOUNDS_EXCEEDED_,
                sizeof(EXCEPTION_ARRAY_BOUNDS_EXCEEDED_));
            p += sizeof(EXCEPTION_ARRAY_BOUNDS_EXCEEDED_);
            break;
        case 0xC000008D: // EXCEPTION_FLT_DENORMAL_OPERAND
            memcpy(p, EXCEPTION_FLT_DENORMAL_OPERAND_,
                sizeof(EXCEPTION_FLT_DENORMAL_OPERAND_));
            p += sizeof(EXCEPTION_FLT_DENORMAL_OPERAND_);
            break;
        case 0xC000008E: // EXCEPTION_FLT_DIVIDE_BY_ZERO
            memcpy(p, EXCEPTION_FLT_DIVIDE_BY_ZERO_,
                sizeof(EXCEPTION_FLT_DIVIDE_BY_ZERO_));
            p += sizeof(EXCEPTION_FLT_DIVIDE_BY_ZERO_);
            break;
        case 0xC000008F: // EXCEPTION_FLT_INEXACT_RESULT
            memcpy(p, EXCEPTION_FLT_INEXACT_RESULT_,
                sizeof(EXCEPTION_FLT_INEXACT_RESULT_));
            p += sizeof(EXCEPTION_FLT_INEXACT_RESULT_);
            break;
        case 0xC0000090: // EXCEPTION_FLT_INVALID_OPERATION
            memcpy(p, EXCEPTION_FLT_INVALID_OPERATION_,
                sizeof(EXCEPTION_FLT_INVALID_OPERATION_));
            p += sizeof(EXCEPTION_FLT_INVALID_OPERATION_);
            break;
        case 0xC0000091: // EXCEPTION_FLT_OVERFLOW
            memcpy(p, EXCEPTION_FLT_OVERFLOW_,
                sizeof(EXCEPTION_FLT_OVERFLOW_));
            p += sizeof(EXCEPTION_FLT_OVERFLOW_);
            break;
        case 0xC0000092: // EXCEPTION_FLT_STACK_CHECK
            memcpy(p, EXCEPTION_FLT_STACK_CHECK_,
                sizeof(EXCEPTION_FLT_STACK_CHECK_));
            p += sizeof(EXCEPTION_FLT_STACK_CHECK_);
            break;
        case 0xC0000093: // EXCEPTION_FLT_UNDERFLOW
            memcpy(p, EXCEPTION_FLT_UNDERFLOW_,
                sizeof(EXCEPTION_FLT_UNDERFLOW_));
            p += sizeof(EXCEPTION_FLT_UNDERFLOW_);
            break;
        case 0xC0000094: // EXCEPTION_INT_DIVIDE_BY_ZERO
            memcpy(p, EXCEPTION_INT_DIVIDE_BY_ZERO_,
                sizeof(EXCEPTION_INT_DIVIDE_BY_ZERO_));
            p += sizeof(EXCEPTION_INT_DIVIDE_BY_ZERO_);
            break;
        case 0xC0000095: // EXCEPTION_INT_OVERFLOW
            memcpy(p, EXCEPTION_INT_OVERFLOW_,
                sizeof(EXCEPTION_INT_OVERFLOW_));
            p += sizeof(EXCEPTION_INT_OVERFLOW_);
            break;
        case 0xC0000096: // EXCEPTION_PRIV_INSTRUCTION
            memcpy(p, EXCEPTION_PRIV_INSTRUCTION_,
                sizeof(EXCEPTION_PRIV_INSTRUCTION_));
            p += sizeof(EXCEPTION_PRIV_INSTRUCTION_);
            break;
        case 0xC00000FD: // EXCEPTION_STACK_OVERFLOW
            memcpy(p, EXCEPTION_STACK_OVERFLOW_,
                sizeof(EXCEPTION_STACK_OVERFLOW_));
            p += sizeof(EXCEPTION_STACK_OVERFLOW_);
            break;
        case 0xC0000194: // EXCEPTION_POSSIBLE_DEADLOCK
            memcpy(p, EXCEPTION_POSSIBLE_DEADLOCK_,
                sizeof(EXCEPTION_POSSIBLE_DEADLOCK_));
            p += sizeof(EXCEPTION_POSSIBLE_DEADLOCK_);
            break;
        case 0xC0000409: // STATUS_STACK_BUFFER_OVERRUN
            memcpy(p, STATUS_STACK_BUFFER_OVERRUN_,
                sizeof(STATUS_STACK_BUFFER_OVERRUN_));
            p += sizeof(STATUS_STACK_BUFFER_OVERRUN_);
            break;
        case 0xC000041D: // STATUS_FATAL_USER_CALLBACK_EXCEPTION
            memcpy(p, STATUS_FATAL_USER_CALLBACK_EXCEPTION_,
                sizeof(STATUS_FATAL_USER_CALLBACK_EXCEPTION_));
            p += sizeof(STATUS_FATAL_USER_CALLBACK_EXCEPTION_);
            break;
        case 0xC0000420: // STATUS_ASSERTION_FAILURE
            memcpy(p, STATUS_ASSERTION_FAILURE_,
                sizeof(STATUS_ASSERTION_FAILURE_));
            p += sizeof(STATUS_ASSERTION_FAILURE_);
            break;
        case 0xE0434f4D: // STATUS_CLR_EXCEPTION
            memcpy(p, STATUS_CLR_EXCEPTION_,
                sizeof(STATUS_CLR_EXCEPTION_));
            p += sizeof(STATUS_CLR_EXCEPTION_);
            break;
        case 0xE06D7363: // STATUS_CPP_EH_EXCEPTION
            memcpy(p, STATUS_CPP_EH_EXCEPTION_,
                sizeof(STATUS_CPP_EH_EXCEPTION_));
            p += sizeof(STATUS_CPP_EH_EXCEPTION_);
            break;
        case 0x80000001: // EXCEPTION_GUARD_PAGE
            memcpy(p, EXCEPTION_GUARD_PAGE_,
                sizeof(EXCEPTION_GUARD_PAGE_));
            p += sizeof(EXCEPTION_GUARD_PAGE_);
            break;
        case 0x80000002: // EXCEPTION_DATATYPE_MISALIGNMENT
            memcpy(p, EXCEPTION_DATATYPE_MISALIGNMENT_,
                sizeof(EXCEPTION_DATATYPE_MISALIGNMENT_));
            p += sizeof(EXCEPTION_DATATYPE_MISALIGNMENT_);
            break;
        case 0x80000003: // EXCEPTION_BREAKPOINT
            memcpy(p, EXCEPTION_BREAKPOINT_,
                sizeof(EXCEPTION_BREAKPOINT_));
            p += sizeof(EXCEPTION_BREAKPOINT_);
            break;
        case 0x80000004: // EXCEPTION_SINGLE_STEP
            memcpy(p, EXCEPTION_SINGLE_STEP_,
                sizeof(EXCEPTION_SINGLE_STEP_));
            p += sizeof(EXCEPTION_SINGLE_STEP_);
            break;
        case 0x4000001F: // STATUS_WX86_BREAKPOINT
            memcpy(p, STATUS_WX86_BREAKPOINT_,
                sizeof(STATUS_WX86_BREAKPOINT_));
            p += sizeof(STATUS_WX86_BREAKPOINT_);
            break;
        case 0x40000015: // STATUS_FATAL_APP_EXIT
            memcpy(p, STATUS_FATAL_APP_EXIT_,
                sizeof(STATUS_FATAL_APP_EXIT_));
            p += sizeof(STATUS_FATAL_APP_EXIT_);
            break;
        case 0x406D1388: // https://learn.microsoft.com/visualstudio/debugger/tips-for-debugging-threads
            memcpy(p, MS_VC_EXCEPTION_,
                sizeof(MS_VC_EXCEPTION_));
            p += sizeof(MS_VC_EXCEPTION_);
            break;
        case 1717: // RPC_S_UNKNOWN_IF
            memcpy(p, RPC_S_UNKNOWN_IF_,
                sizeof(RPC_S_UNKNOWN_IF_));
            p += sizeof(RPC_S_UNKNOWN_IF_);
            break;
        case 1722: // RPC_S_SERVER_UNAVAILABLE
            memcpy(p, RPC_S_SERVER_UNAVAILABLE_,
                sizeof(RPC_S_SERVER_UNAVAILABLE_));
            p += sizeof(RPC_S_SERVER_UNAVAILABLE_);
            break;
    }

    return p;
}

static
__forceinline
char* FormatFileLine(
    PWSTR FileName,
    DWORD LineNumber,
    ULONG len,
    char* p,
    char Color
)
{
    ULONG UTF8StringActualByteCount;

    RtlUnicodeToUTF8N(p, BUFLEN,
        &UTF8StringActualByteCount, FileName, len << 1);
    p += UTF8StringActualByteCount;

    if (Color)
    {
        memcpy(p, CONSOLE_DEFAULT_FORMAT,
            sizeof(CONSOLE_DEFAULT_FORMAT));
        p += sizeof(CONSOLE_DEFAULT_FORMAT);
    }

    *p = ':';
    p = _ultoa10(LineNumber, p + 1);
    *p = '\n';

    return p + 1;
}

static
__forceinline
char* FormatSourceCode(
    PWSTR FileName,
    DWORD LineNumber,
    size_t len,
    char* _buffer,
    char* p,
    char verbose
)
{
    HANDLE hFile;
    UNICODE_STRING String;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    // Windows Object Manager namespace
    memcpy(FileName - 4, OBJECT_MANAGER_NAMESPACE,
        sizeof(OBJECT_MANAGER_NAMESPACE));
    String.Length = (len << 1) + sizeof(OBJECT_MANAGER_NAMESPACE);
    String.Buffer = FileName - 4;
    InitializeObjectAttributes(&ObjectAttributes,
        &String, OBJ_CASE_INSENSITIVE, NULL, NULL);
    NtCreateFile(&hFile, FILE_READ_DATA | SYNCHRONIZE, &ObjectAttributes,
        &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
        FILE_SEQUENTIAL_ONLY | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (IoStatusBlock.Information == FILE_OPENED)
    {
        char* ptr;
        char buffer[PAGESIZE];
        DWORD line = 1;

        while (TRUE)
        {
            if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock,
                buffer, PAGESIZE, NULL, NULL)) break;

            ptr = buffer;

            while ((ptr = (char* ) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr) + 1) > (char* ) 1)
            {
                if (++line == LineNumber)
                {
                    char* _ptr;
                    size_t temp;

                    p = _ultoa10(line, p);
                    memset(p, ' ', 6);
                    p += 6;
                    _ptr = (char* ) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr);

                    if (_ptr) temp = _ptr - ptr;
                    else
                    {
                        temp = buffer + IoStatusBlock.Information - ptr;
                        memcpy(p, ptr, temp);
                        p += temp;
                        if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock, buffer,
                            PAGESIZE, NULL, NULL)) break;
                        ptr = buffer;
                        _ptr = (char* ) memchr(buffer, '\n', IoStatusBlock.Information);
                        if (_ptr) temp = buffer + IoStatusBlock.Information - _ptr;
                        else temp = IoStatusBlock.Information;
                    }

                    ++temp;
                    memcpy(p, ptr, temp);
                    p += temp;
                    break;
                }
            }

            if (line == LineNumber) break;
        }

        NtClose(hFile);
    } else if (verbose >= 3)
    {
        wchar_t temp[WBUFLEN];
        ULONG UTF8StringActualByteCount;

        DWORD len = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
            ERROR_FILE_NOT_FOUND, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), temp, WBUFLEN, NULL);
        RtlUnicodeToUTF8N(p, _buffer + BUFLEN - p,
            &UTF8StringActualByteCount, temp, len << 1);
        p += UTF8StringActualByteCount;
    }

    return p;
}