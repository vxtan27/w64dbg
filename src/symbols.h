/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

typedef struct
{
    char *p;
    HANDLE hProcess;
    PVOID pContext;
    DWORD64 *pBase;
    DWORD bx64win;
    char Console;
    char DataIsLocal;
    char IsFirst;
} USERCONTEXT, *PUSERCONTEXT;

typedef union
{
    struct
    {
        ULARGE_INTEGER LowPart;
        ULARGE_INTEGER HighPart;
    } u;
    long double QuadPart;
} LONG_DOUBLE;

// Applies non-bold/bright cyan to foreground
static const char CONSOLE_CYAN_FORMAT[5] = "\x1b[36m";

static const char LOW_HEX_FORMAT[] = "%Ix";
static const char LONGLONG_FORMAT[] = "%lld";
static const char ULONGLONG_FORMAT[] = "%llu";
static const char LONGDOUBLE_FORMAT[] = "%Lg";
static const char UP_HEX_FORMAT[] = "%IX";

_Success_(return >= 0)
_Check_return_opt_
static
int __builtin_sprintf(
    _Pre_notnull_ _Always_(_Post_z_) char*       const _Buffer,
    _In_z_ _Printf_format_string_    char const* const _Format,
    ...)
{
    int _Result;
    va_list _ArgList;
    __crt_va_start(_ArgList, _Format);

    _Result = __stdio_common_vsprintf(0,
        _Buffer, (size_t) -1, _Format, NULL, _ArgList);

    __crt_va_end(_ArgList);
    return _Result;
}

#include "cvconst.h"

[[deprecated]]
static
__forceinline
DWORD64 _GetRegisterBase32(PSYMBOL_INFOW pSymInfo, PWOW64_CONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case CV_REG_AH:
        case CV_REG_CH:
        case CV_REG_DH:
        case CV_REG_BH:
            return ((&pContext->Ebx)[8 - pSymInfo->Register] >> 8) & 0xFF;

        case CV_REG_ES:
            return pContext->SegEs;
        case CV_REG_CS:
            return pContext->SegCs;
        case CV_REG_SS:
            return pContext->SegSs;
        case CV_REG_DS:
            return pContext->SegDs;
        case CV_REG_FS:
            return pContext->SegFs;
        case CV_REG_GS:
            return pContext->SegGs;
        case CV_REG_FLAGS:
            return pContext->EFlags & 0xFFFF;
        case CV_REG_EIP:
            return pContext->Eip;
        case CV_REG_EFLAGS:
            return pContext->EFlags;

        case CV_REG_DR0:
        case CV_REG_DR1:
        case CV_REG_DR2:
        case CV_REG_DR3:
            return (&pContext->Dr0)[pSymInfo->Register - 90];

        case CV_REG_DR6:
            return pContext->Dr6;
        case CV_REG_DR7:
            return pContext->Dr7;

        case CV_REG_CTRL:
            return pContext->FloatSave.ControlWord;
        case CV_REG_STAT:
            return pContext->FloatSave.StatusWord;
        case CV_REG_TAG:
            return pContext->FloatSave.TagWord;
        case CV_REG_FPIP:
        case CV_REG_FPEIP:
            return pContext->FloatSave.ErrorOffset;
        case CV_REG_FPCS:
            return pContext->FloatSave.ErrorSelector;
        case CV_REG_FPDO:
        case CV_REG_FPEDO:
            return pContext->FloatSave.DataOffset;
        case CV_REG_FPDS:
            return pContext->FloatSave.DataSelector;
        case CV_REG_ISEM:
            return pContext->FloatSave.Cr0NpxState;

        default:
            return 0;
    }
}

[[deprecated]]
static
__forceinline
DWORD64 _GetRegisterBase64(PSYMBOL_INFOW pSymInfo, PCONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case CV_AMD64_AH:
        case CV_AMD64_CH:
        case CV_AMD64_DH:
        case CV_AMD64_BH:
            return ((&pContext->Rax)[pSymInfo->Register - 5] >> 8) & 0xFF;

        case CV_AMD64_ES:
            return pContext->SegEs;
        case CV_AMD64_CS:
            return pContext->SegCs;
        case CV_AMD64_SS:
            return pContext->SegSs;
        case CV_AMD64_DS:
            return pContext->SegDs;
        case CV_AMD64_FS:
            return pContext->SegFs;
        case CV_AMD64_GS:
            return pContext->SegGs;
        case CV_AMD64_FLAGS:
            return pContext->EFlags & 0xFFFF;
        case CV_AMD64_RIP:
            return pContext->Rip;
        case CV_AMD64_EFLAGS:
            return pContext->EFlags;

        case CV_AMD64_DR0:
        case CV_AMD64_DR1:
        case CV_AMD64_DR2:
        case CV_AMD64_DR3:
            return (&pContext->Dr0)[pSymInfo->Register - 90];

        case CV_AMD64_DR6:
            return pContext->Dr6;
        case CV_AMD64_DR7:
            return pContext->Dr7;

        case CV_AMD64_CTRL:
            return pContext->FltSave.ControlWord;
        case CV_AMD64_STAT:
            return pContext->FltSave.StatusWord;
        case CV_AMD64_TAG:
            return pContext->FltSave.TagWord;
        case CV_AMD64_FPIP:
        case CV_AMD64_FPEIP:
            return pContext->FltSave.ErrorOffset;
        case CV_AMD64_FPCS:
            return pContext->FltSave.ErrorSelector;
        case CV_AMD64_FPDO:
        case CV_AMD64_FPEDO:
            return pContext->FltSave.DataOffset;
        case CV_AMD64_FPDS:
            return pContext->FltSave.DataSelector;
        case CV_AMD64_ISEM:
            return pContext->FltSave.MxCsr;

        default:
            return 0;
    }
}

static
__forceinline
DWORD64 GetRegisterBase32(PSYMBOL_INFOW pSymInfo, PWOW64_CONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case CV_REG_AL:
        case CV_REG_CL:
        case CV_REG_DL:
        case CV_REG_BL:
            return (&pContext->Ebx)[4 - pSymInfo->Register] & 0xFF;

        case CV_REG_AX:
        case CV_REG_CX:
        case CV_REG_DX:
        case CV_REG_BX:
            return (&pContext->Ebx)[12 - pSymInfo->Register] & 0xFFFF;

        case CV_REG_SP:
            return pContext->Esp & 0xFFFF;
        case CV_REG_BP:
            return pContext->Ebp & 0xFFFF;
        case CV_REG_SI:
            return pContext->Esi & 0xFFFF;
        case CV_REG_DI:
            return pContext->Edi & 0xFFFF;

        case CV_REG_EAX:
        case CV_REG_ECX:
        case CV_REG_EDX:
        case CV_REG_EBX:
            return (&pContext->Ebx)[20 - pSymInfo->Register];

        case CV_REG_ESP:
            return pContext->Esp;
        case CV_REG_EBP:
        case CV_ALLREG_VFRAME:
            return pContext->Ebp;
        case CV_REG_ESI:
            return pContext->Esi;
        case CV_REG_EDI:
            return pContext->Edi;

        default:
            /* return _GetRegisterBase32(pSymInfo, pContext); */
            return 0;
    }
}

static
__forceinline
DWORD64 GetRegisterBase64(PSYMBOL_INFOW pSymInfo, PCONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case CV_AMD64_AL:
        case CV_AMD64_CL:
        case CV_AMD64_DL:
        case CV_AMD64_BL:
            return (&pContext->Rax)[pSymInfo->Register - 1] & 0xFF;

        case CV_AMD64_AX:
        case CV_AMD64_CX:
        case CV_AMD64_DX:
        case CV_AMD64_BX:
        case CV_AMD64_SP:
        case CV_AMD64_BP:
        case CV_AMD64_SI:
        case CV_AMD64_DI:
            return (&pContext->Rax)[pSymInfo->Register - 9] & 0xFFFF;

        case CV_AMD64_EAX:
        case CV_AMD64_ECX:
        case CV_AMD64_EDX:
        case CV_AMD64_EBX:
        case CV_AMD64_ESP:
        case CV_AMD64_EBP:
        case CV_AMD64_ESI:
        case CV_AMD64_EDI:
            return (&pContext->Rax)[pSymInfo->Register - 17];

        case CV_AMD64_RAX:
            return pContext->Rax;
        case CV_AMD64_RBX:
            return pContext->Rbx;
        case CV_AMD64_RCX:
            return pContext->Rcx;
        case CV_AMD64_RDX:
            return pContext->Rdx;
        case CV_AMD64_RSI:
            return pContext->Rsi;
        case CV_AMD64_RDI:
            return pContext->Rdi;
        case CV_AMD64_RBP:
            return pContext->Rbp;
        case CV_AMD64_RSP:
            return pContext->Rsp;

        case CV_AMD64_R8:
        case CV_AMD64_R9:
        case CV_AMD64_R10:
        case CV_AMD64_R11:
        case CV_AMD64_R12:
        case CV_AMD64_R13:
        case CV_AMD64_R14:
        case CV_AMD64_R15:
            return (&pContext->R8)[pSymInfo->Register - 336];

        case CV_AMD64_R8B:
        case CV_AMD64_R9B:
        case CV_AMD64_R10B:
        case CV_AMD64_R11B:
        case CV_AMD64_R12B:
        case CV_AMD64_R13B:
        case CV_AMD64_R14B:
        case CV_AMD64_R15B:
            return (&pContext->R8)[pSymInfo->Register - 344] & 0xFF;

        case CV_AMD64_R8W:
        case CV_AMD64_R9W:
        case CV_AMD64_R10W:
        case CV_AMD64_R11W:
        case CV_AMD64_R12W:
        case CV_AMD64_R13W:
        case CV_AMD64_R14W:
        case CV_AMD64_R15W:
            return (&pContext->R8)[pSymInfo->Register - 352] & 0xFFFF;

        case CV_AMD64_R8D:
        case CV_AMD64_R9D:
        case CV_AMD64_R10D:
        case CV_AMD64_R11D:
        case CV_AMD64_R12D:
        case CV_AMD64_R13D:
        case CV_AMD64_R14D:
        case CV_AMD64_R15D:
            return (&pContext->R8)[pSymInfo->Register - 360];

        default:
            /* return _GetRegisterBase64(pSymInfo, pContext); */
            return 0;
    }
}

static
__forceinline
DWORD64 GetRegisterBase(PSYMBOL_INFOW pSymInfo, PVOID ContextRecord, DWORD bx64win)
{
    if (bx64win) return GetRegisterBase64(pSymInfo, ContextRecord);
    else return GetRegisterBase32(pSymInfo, ContextRecord);
}

// https://accu.org/journals/overload/29/165/orr
// https://github.com/rogerorr/NtTrace/blob/main/src/SymbolEngine.cpp#L1185
// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code#showing-variables-using-the-windows-debugging-api

static BOOL CALLBACK EnumCallbackProc(PSYMBOL_INFOW pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    (void) SymbolSize;

    PUSERCONTEXT User = (PUSERCONTEXT) UserContext;

    if ((User->DataIsLocal && !(pSymInfo->Flags & SYMFLAG_PARAMETER)) ||
        (!User->DataIsLocal && pSymInfo->Flags & SYMFLAG_PARAMETER))
    {
        DWORD DTag;
        SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_SYMTAG, &DTag);

        if (DTag == SymTagTypedef)
        {
            // Resolve the base type from the underlying type
            SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_TYPE, &DTag);
            SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, DTag, TI_GET_SYMTAG, &DTag);
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
            memcpy(User->p, CONSOLE_CYAN_FORMAT, 5);
            User->p += 5;
        }

        ULONG UTF8StringActualByteCount;
        RtlUnicodeToUTF8N(User->p, BUFLEN,
            &UTF8StringActualByteCount, pSymInfo->Name, (pSymInfo->NameLen - 1) << 1);
        User->p += UTF8StringActualByteCount;

        if (User->Console)
        {
            memcpy(User->p, CONSOLE_DEFAULT_FORMAT,
                sizeof(CONSOLE_DEFAULT_FORMAT));
            User->p += sizeof(CONSOLE_DEFAULT_FORMAT);
        }

        *User->p++ = '=';

        if (pSymInfo->Flags & SYMFLAG_NULL)
        {
            *User->p++ = '?';
            *User->p++ = '?';
        } else
        {
            const char *_Format;
            DWORD64 Base;
            LONG_DOUBLE value = {};
            if (pSymInfo->Flags & SYMFLAG_REGREL)
                Base = GetRegisterBase(pSymInfo, User->pContext, User->bx64win);
            else if (pSymInfo->Flags & SYMFLAG_FRAMEREL)
                Base = *User->pBase;
            else return TRUE;

            /*
            else if (pSymInfo->Flags & SYMFLAG_REGISTER)
                GetRegisterValue(pSymInfo, User->pContext, User->bx64win);
            */

            if (DTag == SymTagArrayType ||
                DTag == SymTagUDT ||
                DTag == SymTagTaggedUnionCase)
            {
                *User->p++ = '0';
                *User->p++ = 'x';
                value.u.LowPart.QuadPart = pSymInfo->Address + Base;
                _Format = LOW_HEX_FORMAT;
            } else
            {
                ULONG64 Len;

                // add SYMFLAG_VALUEPRESENT
                SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_LENGTH, &Len);
                NtReadVirtualMemory(User->hProcess, (PVOID) (pSymInfo->Address + Base), &value, Len, NULL);

                if (DTag == SymTagPointerType)
                {
                    *User->p++ = '0';
                    *User->p++ = 'x';
                    _Format = LOW_HEX_FORMAT;
                } else
                {
                    DWORD BaseType;
                    SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_BASETYPE, &BaseType);
                    switch (BaseType)
                    {
                        case btChar:
                        case btWChar:
                        case btInt:
                        case btLong:
                            _Format = LONGLONG_FORMAT;
                            break;
                        case btUInt:
                        case btBool:
                        case btULong:
                            _Format = ULONGLONG_FORMAT;
                            break;
                        case btFloat:
                            _Format = LONGDOUBLE_FORMAT;
                            User->p += __builtin_sprintf(User->p, _Format, value.QuadPart);
                            if (User->DataIsLocal) *User->p++ = '\n';
                            return TRUE;
                        case btHresult:
                            _Format = UP_HEX_FORMAT;
                            break;
                        default:
                            _Format = LOW_HEX_FORMAT;
                    }
                }
            }
            User->p += __builtin_sprintf(User->p, _Format, value.u.LowPart);
        }
        if (User->DataIsLocal) *User->p++ = '\n';
    }

    return TRUE; // Continue enumeration
}