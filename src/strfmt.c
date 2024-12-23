/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "strutil.c" // String utilities
#include "ntdll.h" // Native
#include <wchar.h>

static inline char *FormatDebugException(
    _In_ const EXCEPTION_RECORD *ExceptionRecord,
    _Out_writes_(48) char *p,
    _In_ char bWow64
    )
{
    switch (ExceptionRecord->ExceptionCode)
    {
        case 0xC0000005: // EXCEPTION_ACCESS_VIOLATION
        case 0xC0000006: // EXCEPTION_IN_PAGE_ERROR
            if (ExceptionRecord->ExceptionCode == 0xC0000005)
            { // EXCEPTION_ACCESS_VIOLATION
                memcpy(p, "access violation", 16);
                p += 16;
            } else
            { // EXCEPTION_IN_PAGE_ERROR
                memcpy(p, "in page error", 13);
                p += 13;
            }

            *p++ = ' ';

            if (!ExceptionRecord->ExceptionInformation[0])
            {
                memcpy(p, "reading from", 12);
                p += 12;
            } else if (ExceptionRecord->ExceptionInformation[0] == 1)
            {
                memcpy(p, "writing to", 10);
                p += 10;
            } else
            {
                memcpy(p, "DEP violation at", 16);
                p += 16;
            }

            memcpy(p, " location ", 10);
            p = ulltoaddr(ExceptionRecord->ExceptionInformation[1], p + 10, bWow64);
            break;
        case 0xC0000008: // EXCEPTION_INVALID_HANDLE
            memcpy(p, "invalid handle", 14);
            p += 14;
            break;
        case 0xC000001D: // EXCEPTION_ILLEGAL_INSTRUCTION
            memcpy(p, "illegal instruction", 19);
            p += 19;
            break;
        case 0xC0000026: // EXCEPTION_INVALID_DISPOSITION
            memcpy(p, "invalid disposition", 19);
            p += 19;
            break;
        case 0xC000008C: // EXCEPTION_ARRAY_BOUNDS_EXCEEDED
            memcpy(p, "array bounds exceeded", 21);
            p += 21;
            break;
        case 0xC000008D: // EXCEPTION_FLT_DENORMAL_OPERAND
            memcpy(p, "floating-point denormal operand", 31);
            p += 31;
            break;
        case 0xC000008E: // EXCEPTION_FLT_DIVIDE_BY_ZERO
            memcpy(p, "floating-point division by zero", 31);
            p += 31;
            break;
        case 0xC000008F: // EXCEPTION_FLT_INEXACT_RESULT
            memcpy(p, "floating-point inexact result", 29);
            p += 29;
            break;
        case 0xC0000090: // EXCEPTION_FLT_INVALID_OPERATION
            memcpy(p, "floating-point invalid operation", 32);
            p += 32;
            break;
        case 0xC0000091: // EXCEPTION_FLT_OVERFLOW
            memcpy(p, "floating-point overflow", 23);
            p += 23;
            break;
        case 0xC0000092: // EXCEPTION_FLT_STACK_CHECK
            memcpy(p, "floating-point stack check", 26);
            p += 26;
            break;
        case 0xC0000093: // EXCEPTION_FLT_UNDERFLOW
            memcpy(p, "floating-point underflow", 24);
            p += 24;
            break;
        case 0xC0000094: // EXCEPTION_INT_DIVIDE_BY_ZERO
            memcpy(p, "integer division by zero", 24);
            p += 24;
            break;
        case 0xC0000095: // EXCEPTION_INT_OVERFLOW
            memcpy(p, "integer overflow", 16);
            p += 16;
            break;
        case 0xC0000096: // EXCEPTION_PRIV_INSTRUCTION
            memcpy(p, "privileged instruction", 22);
            p += 22;
            break;
        case 0xC00000FD: // EXCEPTION_STACK_OVERFLOW
            memcpy(p, "stack overflow", 14);
            p += 14;
            break;
        case 0xC0000194: // EXCEPTION_POSSIBLE_DEADLOCK
            memcpy(p, "possible deadlock condition", 27);
            p += 27;
            break;
        case 0xC0000409: // STATUS_STACK_BUFFER_OVERRUN
            memcpy(p, "fast fail", 9);
            p += 9;
            break;
        case 0xC000041D: // STATUS_FATAL_USER_CALLBACK_EXCEPTION
            memcpy(p, "fatal user callback exception", 29);
            p += 29;
            break;
        case 0xC0000420: // STATUS_ASSERTION_FAILURE
            memcpy(p, "assertion failure", 17);
            p += 17;
            break;
        case 0xE0434f4D: // STATUS_CLR_EXCEPTION
            memcpy(p, "CLR exception", 13);
            p += 13;
            break;
        case 0xE06D7363: // STATUS_CPP_EH_EXCEPTION
            memcpy(p, "C++ exception handling exception", 32);
            p += 32;
            break;
        case 0x80000001: // EXCEPTION_GUARD_PAGE
            memcpy(p, "guard page exception", 20);
            p += 20;
            break;
        case 0x80000002: // EXCEPTION_DATATYPE_MISALIGNMENT
            memcpy(p, "alignment fault", 15);
            p += 15;
            break;
        case 0x80000003: // EXCEPTION_BREAKPOINT
        case 0x4000001F: // STATUS_WX86_BREAKPOINT
            memcpy(p, "breakpoint", 10);
            p += 10;
            break;
        case 0x80000004: // EXCEPTION_SINGLE_STEP
            memcpy(p, "trace trap or other single-instruction mechanism", 48);
            p += 48;
            break;
        case 0x40000015: // STATUS_FATAL_APP_EXIT
            memcpy(p, "fast application exit", 21);
            p += 21;
            break;
        case 0x406D1388: // https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads
            memcpy(p, "thread name exception", 21);
            p += 21;
            break;
        case 1717: // RPC_S_UNKNOWN_IF
            memcpy(p, "unknown interface", 17);
            p += 17;
            break;
        case 1722: // RPC_S_SERVER_UNAVAILABLE
            memcpy(p, "server unavailable", 18);
            p += 18;
            break;
        default:
            memcpy(p, "unknown exception (0x", 21);
            p = __builtin_hextoa(ExceptionRecord->ExceptionCode, p + 21);
            *p++ = ')';
    }

    return p;
}

static inline char *FormatVerboseDebugException(
    _Out_writes_(152) char *p,
    _In_ DWORD ExceptionCode
    )
{
    switch (ExceptionCode)
    {
        case 0xC0000005: // EXCEPTION_ACCESS_VIOLATION
            memcpy(p, "The thread tried to read from or write to a virtual address for which it does not have the appropriate access", 109);
            p += 109;
            break;
        case 0xC0000006: // EXCEPTION_IN_PAGE_ERROR
            memcpy(p, "The thread tried to access a page that was not present, and the system was unable to load the page", 98);
            p += 98;
            break;
        case 0xC0000008: // EXCEPTION_INVALID_HANDLE
            memcpy(p, "The thread used a handle to a kernel object that was invalid", 60);
            p += 60;
            break;
        case 0xC000001D: // EXCEPTION_ILLEGAL_INSTRUCTION
            memcpy(p, "The thread tried to execute an invalid instruction", 50);
            p += 50;
            break;
        case 0xC0000026: // EXCEPTION_INVALID_DISPOSITION
            memcpy(p, "An exception handler returned an invalid disposition to the exception dispatcher", 80);
            p += 80;
            break;
        case 0xC000008C: // EXCEPTION_ARRAY_BOUNDS_EXCEEDED
            memcpy(p, "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking", 118);
            p += 118;
            break;
        case 0xC000008D: // EXCEPTION_FLT_DENORMAL_OPERAND
            memcpy(p, "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value", 152);
            p += 152;
            break;
        case 0xC000008E: // EXCEPTION_FLT_DIVIDE_BY_ZERO
            memcpy(p, "The thread tried to divide a floating-point value by a floating-point divisor of zero", 85);
            p += 85;
            break;
        case 0xC000008F: // EXCEPTION_FLT_INEXACT_RESULT
            memcpy(p, "The result of a floating-point operation cannot be represented exactly as a decimal fraction", 92);
            p += 92;
            break;
        case 0xC0000090: // EXCEPTION_FLT_INVALID_OPERATION
            memcpy(p, "This exception represents any floating-point exception not included", 67);
            p += 67;
            break;
        case 0xC0000091: // EXCEPTION_FLT_OVERFLOW
            memcpy(p, "The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type", 106);
            p += 106;
            break;
        case 0xC0000092: // EXCEPTION_FLT_STACK_CHECK
            memcpy(p, "The stack overflowed or underflowed as the result of a floating-point operation", 79);
            p += 79;
            break;
        case 0xC0000093: // EXCEPTION_FLT_UNDERFLOW
            memcpy(p, "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type", 103);
            p += 103;
            break;
        case 0xC0000094: // EXCEPTION_INT_DIVIDE_BY_ZERO
            memcpy(p, "The thread tried to divide an integer value by an integer divisor of zero", 73);
            p += 73;
            break;
        case 0xC0000095: // EXCEPTION_INT_OVERFLOW
            memcpy(p, "The result of an integer operation caused a carry out of the most significant bit of the result", 95);
            p += 95;
            break;
        case 0xC0000096: // EXCEPTION_PRIV_INSTRUCTION
            memcpy(p, "The thread tried to execute an instruction whose operation is not allowed in the current machine mode", 101);
            p += 101;
            break;
        case 0xC00000FD: // EXCEPTION_STACK_OVERFLOW
            memcpy(p, "The thread used up its stack", 28);
            p += 28;
            break;
        case 0xC0000194: // EXCEPTION_POSSIBLE_DEADLOCK
            memcpy(p, "The wait operation on the critical section times out", 52);
            p += 52;
            break;
        case 0xC0000409: // STATUS_STACK_BUFFER_OVERRUN
            memcpy(p, "The system detected an overrun of a stack-based buffer in this application", 74);
            p += 74;
            break;
        case 0xC000041D: // STATUS_FATAL_USER_CALLBACK_EXCEPTION
            memcpy(p, "An unhandled exception was encountered during a user callback", 61);
            p += 61;
            break;
        case 0xC0000420: // STATUS_ASSERTION_FAILURE
            memcpy(p, "An assertion failure has occurred", 33);
            p += 33;
            break;
        case 0xE0434f4D: // STATUS_CLR_EXCEPTION
            memcpy(p, "A managed code exception was encountered within .NET Common Language Runtime", 76);
            p += 76;
            break;
        case 0xE06D7363: // STATUS_CPP_EH_EXCEPTION
            memcpy(p, "A C++ exception has been thrown and is being handled or caught", 62);
            p += 62;
            break;
        case 0x80000001: // EXCEPTION_GUARD_PAGE
            memcpy(p, "The thread accessed memory allocated with the PAGE_GUARD modifier", 65);
            p += 65;
            break;
        case 0x80000002: // EXCEPTION_DATATYPE_MISALIGNMENT
            memcpy(p, "The thread tried to read or write data that is misaligned on hardware that does not provide alignment", 101);
            p += 101;
            break;
        case 0x80000003: // EXCEPTION_BREAKPOINT
            memcpy(p, "A breakpoint was encountered", 28);
            p += 28;
            break;
        case 0x80000004: // EXCEPTION_SINGLE_STEP
            memcpy(p, "A trace trap or other single-instruction mechanism signaled that one instruction has been executed", 98);
            p += 98;
            break;
        case 0x4000001F: // STATUS_WX86_BREAKPOINT
            memcpy(p, "A Win32 x86 breakpoint was encountered", 38);
            p += 38;
            break;
        case 0x40000015: // STATUS_FATAL_APP_EXIT
            memcpy(p, "The application caused an unhandled runtime exception during shutdown", 69);
            p += 69;
            break;
        case 0x406D1388: // https://learn.microsoft.com/en-us/visualstudio/debugger/tips-for-debugging-threads
            memcpy(p, "The thread set its own name by raising exception", 48);
            p += 48;
            break;
        case 1717: // RPC_S_UNKNOWN_IF
            memcpy(p, "The interface is unknown", 24);
            p += 24;
            break;
        case 1722: // RPC_S_SERVER_UNAVAILABLE
            memcpy(p, "The RPC server is unavailable", 29);
            p += 29;
            break;
    }

    return p;
}

static __forceinline char *FormatFileLine(
    _In_reads_or_z_(len) wchar_t *fname,
    _In_ unsigned int lnum,
    _In_ ULONG len,
    _Out_writes_(len + 10) char *p,
    _In_ char Color
    )
{
    ULONG UTF8StringActualByteCount;

    RtlUnicodeToUTF8N(p, BUFLEN,
        &UTF8StringActualByteCount, fname, len << 1);
    p += UTF8StringActualByteCount;

    if (Color)
    {
        memcpy(p, "\x1b[m", 3);
        p += 3;
    }

    *p = ':';
    p = __builtin_ulltoa(lnum, p + 1);
    *p = '\n';

    return p + 1;
}

static __forceinline char *FormatSourceCode(
    _Inout_updates_bytes_all_(4) wchar_t *fname,
    _In_ unsigned int lnum,
    _Out_ char *p,
    _In_ char verbose
    )
{
    HANDLE hFile;
    UNICODE_STRING String;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

    memcpy(fname - 4, L"\\??\\", 8);
    String.Length = (wcslen(fname) << 1) + 8;
    String.Buffer = fname - 4;
    InitializeObjectAttributes(&ObjectAttributes,
        &String, OBJ_CASE_INSENSITIVE, NULL, NULL);
    NtCreateFile(&hFile,
        GENERIC_READ | SYNCHRONIZE, &ObjectAttributes,
        &IoStatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN,
        FILE_SEQUENTIAL_ONLY | FILE_NO_INTERMEDIATE_BUFFERING |
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);

    if (IoStatusBlock.Information == FILE_OPENED)
    {
        char *ptr;
        DWORD line;
        char buffer[PAGESIZE];
        IO_STATUS_BLOCK IoStatusBlock;

        line = 1;

        while (TRUE)
        {
            if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock,
                buffer, PAGESIZE, NULL, NULL)) break;

            ptr = buffer;

            while ((ptr = (char *) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr) + 1) > (char *) 1)
            {
                if (++line == lnum)
                {
                    char *_ptr;
                    size_t temp;

                    p = line_ultoa(line, p);
                    _ptr = (char *) memchr(ptr, '\n', buffer + IoStatusBlock.Information - ptr);

                    if (_ptr) temp = _ptr - ptr;
                    else
                    {
                        temp = buffer + IoStatusBlock.Information - ptr;
                        memcpy(p, ptr, temp);
                        p += temp;
                        if (NtReadFile(hFile, NULL, NULL, NULL, &IoStatusBlock, buffer,
                            PAGESIZE, NULL, NULL)) break;
                        ptr = buffer;
                        _ptr = (char *) memchr(buffer, '\n', IoStatusBlock.Information);
                        if (_ptr) temp = buffer + IoStatusBlock.Information - _ptr;
                        else temp = IoStatusBlock.Information;
                    }

                    ++temp;
                    memcpy(p, ptr, temp);
                    p += temp;
                    break;
                }
            }

            if (line == lnum) break;
        }

        NtClose(hFile);
    } else if (verbose >= 3)
    {
        memcpy(p, W64DBG_FILE_NOT_FOUND,
            strlen(W64DBG_FILE_NOT_FOUND));
        p += strlen(W64DBG_FILE_NOT_FOUND);
    }

    return p;
}