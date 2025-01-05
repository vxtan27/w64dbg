/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "enum.h"
#include "..\resrc.h"
#include "..\ntdll.h"
#include <dbghelp.h>
#include <stdio.h>

_Success_(return >= 0)
static
int __builtin_snprintf(
    char*       const _Buffer,
    size_t      const _BufferCount,
    char const* const _Format,
    ...)
{
    int _Result;
    va_list _ArgList;

    __crt_va_start(_ArgList, _Format);
    _Result = __stdio_common_vsprintf(0,
        _Buffer, _BufferCount, _Format, NULL, _ArgList);
    __crt_va_end(_ArgList);

    return _Result;
}

// https://github.com/microsoft/microsoft-pdb/blob/master/include/cvconst.h
// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code#showing-variables-using-the-windows-debugging-api
// https://accu.org/journals/overload/29/165/orr
// https://github.com/rogerorr/NtTrace/blob/main/src/SymbolEngine.cpp#L1185

static
BOOL
CALLBACK
EnumCallbackProc(PSYMBOL_INFOW pSymbol, ULONG SymbolSize, PVOID UserContext)
{
    (void) SymbolSize;

    USERCONTEXT* User = (USERCONTEXT*) UserContext;

    if ((User->DataIsLocal && !(pSymbol->Flags & SYMFLAG_PARAMETER)) ||
        (!User->DataIsLocal && pSymbol->Flags & SYMFLAG_PARAMETER))
    {
        DWORD64 Offset;

        if (User->bx64win) switch (pSymbol->Register)
        { // 64-bit regular registers
            case 328: // CV_AMD64_RAX
                Offset = ((PCONTEXT) User->pContext)->Rax;
                break;
            case 329: // CV_AMD64_RBX
                Offset = ((PCONTEXT) User->pContext)->Rbx;
                break;
            case 330: // CV_AMD64_RCX
                Offset = ((PCONTEXT) User->pContext)->Rcx;
                break;
            case 331: // CV_AMD64_RDX
                Offset = ((PCONTEXT) User->pContext)->Rdx;
                break;
            case 332: // CV_AMD64_RSI
                Offset = ((PCONTEXT) User->pContext)->Rsi;
                break;
            case 333: // CV_AMD64_RDI
                Offset = ((PCONTEXT) User->pContext)->Rdi;
                break;
            case 334: // CV_AMD64_RBP
                Offset = ((PCONTEXT) User->pContext)->Rbp;
                break;
            case 335: // CV_AMD64_RSP
                Offset = ((PCONTEXT) User->pContext)->Rsp;
                break;
            default: // Unsupport
                Offset = 0;
                break;
        } else switch (pSymbol->Register)
        { // AMD64 registers
            case 17: // CV_AMD64_EAX
                Offset = ((PWOW64_CONTEXT) User->pContext)->Eax;
                break;
            case 18: // CV_AMD64_ECX
                Offset = ((PWOW64_CONTEXT) User->pContext)->Ecx;
                break;
            case 19: // CV_AMD64_EDX
                Offset = ((PWOW64_CONTEXT) User->pContext)->Edx;
                break;
            case 20: // CV_AMD64_EBX
                Offset = ((PWOW64_CONTEXT) User->pContext)->Ebx;
                break;
            case 21: // CV_AMD64_ESP
                Offset = ((PWOW64_CONTEXT) User->pContext)->Esp;
                break;
            case 22: // CV_AMD64_EBP
                Offset = ((PWOW64_CONTEXT) User->pContext)->Ebp;
                break;
            case 23: // CV_AMD64_ESI
                Offset = ((PWOW64_CONTEXT) User->pContext)->Esi;
                break;
            case 24: // CV_AMD64_EDI
                Offset = ((PWOW64_CONTEXT) User->pContext)->Edi;
                break;
            default: // Unsupport
                Offset = 0;
                break;
        }

        DWORD DTag;

        SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_SYMTAG, &DTag);

        if (DTag == SymTagTypedef)
        {
            // Resolve the base type from the underlying type
            SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_TYPE, &DTag);
            SymGetTypeInfo(User->hProcess, pSymbol->ModBase, DTag, TI_GET_SYMTAG, &DTag);
        }

        if (!User->DataIsLocal)
        {
            if (!User->IsFirst)
            {
                *User->p++ = ',';
                *User->p++ = ' ';
            } else User->IsFirst = FALSE;
        } else
        {
            memset(User->p, ' ', 8);
            User->p += 8;
        }

        if (User->Console)
        {
            memcpy(User->p, "\x1b[36m", 5);
            User->p += 5;
        }

        ULONG UTF8StringActualByteCount;
        RtlUnicodeToUTF8N(User->p, BUFLEN,
            &UTF8StringActualByteCount, pSymbol->Name, (pSymbol->NameLen - 1) << 1);
        User->p += UTF8StringActualByteCount;

        if (User->Console)
        {
            memcpy(User->p, CONSOLE_DEFAULT_FORMAT,
                sizeof(CONSOLE_DEFAULT_FORMAT));
            User->p += sizeof(CONSOLE_DEFAULT_FORMAT);
        }

        char* p;
        long double value = 0;

        *User->p++ = '=';

        if (DTag == SymTagArrayType ||
            DTag == SymTagUDT ||
            DTag == SymTagTaggedUnionCase)
        {
            *User->p++ = '0';
            *User->p++ = 'x';
            value = pSymbol->Address + Offset;
            p = "%Ix";
        } else
        {
            ULONG64 Len;

            SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_LENGTH, &Len);
            NtReadVirtualMemory(User->hProcess, (PVOID) (pSymbol->Address + Offset), &value, Len, NULL);

            if (DTag == SymTagPointerType)
            {
                *User->p++ = '0';
                *User->p++ = 'x';
                p = "%Ix";
            } else
            {
                DWORD BaseType;
                SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_BASETYPE, &BaseType);
                switch (BaseType)
                {
                    case btChar:
                    case btWChar:
                    case btInt:
                    case btLong:
                        p = "%lld";
                        break;
                    case btUInt:
                    case btBool:
                    case btULong:
                        p = "%llu";
                        break;
                    case btFloat:
                        p = "%Lg";
                        break;
                    case btHresult:
                        p = "%IX";
                        break;
                    /*
                    case btNoType:
                    case btVoid:
                    case btBCD:
                    case btCurrency:
                    case btDate:
                    case btVariant:
                    case btComplex:
                    case btBit:
                    case btBSTR:
                    */
                    default:
                        p = "%Ix";
                }
            }

        }

        User->p += __builtin_snprintf(User->p, PAGESIZE, p, value);

        if (User->DataIsLocal) *User->p++ = '\n';
    }

    return TRUE; // Continue enumeration
}