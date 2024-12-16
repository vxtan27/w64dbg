/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#include "symbol.h"
#include "ntdll.h"
#include <stdio.h>

// https://github.com/microsoft/microsoft-pdb/blob/master/include/cvconst.h
// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code
// https://gist.github.com/HoShiMin/779d1c5e96e50a653ca43511b7bcb69a
// https://accu.org/journals/overload/29/165/orr

BOOL CALLBACK EnumCallbackProc(PSYMBOL_INFOW pSymbol, ULONG SymbolSize, PVOID UserContext)
{
    (void) SymbolSize;

    USERCONTEXT *User = (USERCONTEXT *) UserContext;

    if ((User->DataIsLocal && !(pSymbol->Flags & SYMFLAG_PARAMETER)) ||
        (!User->DataIsLocal && pSymbol->Flags & SYMFLAG_PARAMETER))
    {
        DWORD64 Offset;

        if (!User->bWow64) switch (pSymbol->Register)
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
            memset(User->p, ' ', 9);
            User->p += 9;
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
            memcpy(User->p, "\x1b[m", 3);
            User->p += 3;
        }

        char *p;
        SIZE_T Len;

        *User->p++ = '=';

        if (DTag == SymTagArrayType ||
            DTag == SymTagUDT ||
            DTag == SymTagTaggedUnionCase)
        {
            *User->p++ = '0';
            *User->p++ = 'x';
            Len = pSymbol->Address + Offset;

            if (!User->bWow64) p = "%I64x";
            else p = "%I32x";
        } else
        {
            SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_LENGTH, &Len);
            NtReadVirtualMemory(User->hProcess, (PVOID) (pSymbol->Address + Offset), &Len, Len, NULL);

            if (DTag == SymTagPointerType)
            {
                *User->p++ = '0';
                *User->p++ = 'x';
                if (!User->bWow64) p = "%I64x";
                else p = "%I32x";
            } else
            {
                DWORD BaseType;
                SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_BASETYPE, &BaseType);
                switch (BaseType)
                {
                    case btChar:
                    case btWChar: // Do not native support
                    case btBit: // Do not native support
                        p = "'%c'";
                        break;
                    case btInt:
                    case btBool:
                    case btHresult:
                        p = "%d";
                        break;
                    case btUInt:
                        p = "%u";
                        break;
                    case btFloat:
                        p = "%f";
                        break;
                    case btLong:
                    case btCurrency:
                        p = "%ld";
                        break;
                    case btULong:
                        p = "%lu";
                        break;
                    case btDate:
                        p = "%Lf";
                        break;
                    /*
                    case btBCD:
                    case btVariant:
                    case btComplex:
                    case btBSTR:
                    */
                    default:
                        *User->p++ = '0';
                        *User->p++ = 'x';
                        if (User->bWow64) p = "%I64x";
                        else p = "%I32x";
                }
            }

        }

        User->p += sprintf(User->p, p, Len);

        if (User->DataIsLocal) *User->p++ = '\n';
    }

    return TRUE; // Continue enumeration
}