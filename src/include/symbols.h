/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

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

#pragma warning(disable: 4715)

static __forceinline DWORD64 GetRegisterBase32(PSYMBOL_INFOW pSymInfo, PWOW64_CONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case CV_REG_AL:
        case CV_REG_CL:
        case CV_REG_DL:
        case CV_REG_BL:
            return (&pContext->Ebx)[CV_REG_BL - pSymInfo->Register] & 0xFF;

        case CV_REG_AX:
        case CV_REG_CX:
        case CV_REG_DX:
        case CV_REG_BX:
            return (&pContext->Ebx)[CV_REG_BX - pSymInfo->Register] & 0xFFFF;

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
            return (&pContext->Ebx)[CV_REG_EBX - pSymInfo->Register];

        case CV_REG_ESP:
            return pContext->Esp;
        case CV_REG_EBP:
        case CV_ALLREG_VFRAME:
            return pContext->Ebp;
        case CV_REG_ESI:
            return pContext->Esi;
        case CV_REG_EDI:
            return pContext->Edi;
    }
}

static __forceinline DWORD64 GetRegisterBase64(PSYMBOL_INFOW pSymInfo, PCONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case CV_AMD64_AL:
        case CV_AMD64_CL:
        case CV_AMD64_DL:
        case CV_AMD64_BL:
            return (&pContext->Rax)[pSymInfo->Register - CV_AMD64_AL] & 0xFF;

        case CV_AMD64_AX:
        case CV_AMD64_CX:
        case CV_AMD64_DX:
        case CV_AMD64_BX:
        case CV_AMD64_SP:
        case CV_AMD64_BP:
        case CV_AMD64_SI:
        case CV_AMD64_DI:
            return (&pContext->Rax)[pSymInfo->Register - CV_AMD64_AX] & 0xFFFF;

        case CV_AMD64_EAX:
        case CV_AMD64_ECX:
        case CV_AMD64_EDX:
        case CV_AMD64_EBX:
        case CV_AMD64_ESP:
        case CV_AMD64_EBP:
        case CV_AMD64_ESI:
        case CV_AMD64_EDI:
            return (&pContext->Rax)[pSymInfo->Register - CV_AMD64_EAX];

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
            return (&pContext->R8)[pSymInfo->Register - CV_AMD64_R8];

        case CV_AMD64_R8B:
        case CV_AMD64_R9B:
        case CV_AMD64_R10B:
        case CV_AMD64_R11B:
        case CV_AMD64_R12B:
        case CV_AMD64_R13B:
        case CV_AMD64_R14B:
        case CV_AMD64_R15B:
            return (&pContext->R8)[pSymInfo->Register - CV_AMD64_R8B] & 0xFF;

        case CV_AMD64_R8W:
        case CV_AMD64_R9W:
        case CV_AMD64_R10W:
        case CV_AMD64_R11W:
        case CV_AMD64_R12W:
        case CV_AMD64_R13W:
        case CV_AMD64_R14W:
        case CV_AMD64_R15W:
            return (&pContext->R8)[pSymInfo->Register - CV_AMD64_R8W] & 0xFFFF;

        case CV_AMD64_R8D:
        case CV_AMD64_R9D:
        case CV_AMD64_R10D:
        case CV_AMD64_R11D:
        case CV_AMD64_R12D:
        case CV_AMD64_R13D:
        case CV_AMD64_R14D:
        case CV_AMD64_R15D:
            return (&pContext->R8)[pSymInfo->Register - CV_AMD64_R8D] & 0xFFFFFFFF;
    }
}

static __forceinline DWORD64 GetRegisterBase(PSYMBOL_INFOW pSymInfo, PVOID ContextRecord, DWORD is_64bit)
{
    return is_64bit ? GetRegisterBase64(pSymInfo, (PCONTEXT) ContextRecord)
                    : GetRegisterBase32(pSymInfo, (PWOW64_CONTEXT) ContextRecord);
}

typedef struct
{
    LPSTR p;
    HANDLE hProcess;
    PVOID pContext;
    PDWORD64 pBase;
    DWORD is_64bit;
    BOOL Console;
    BOOL DataIsParam;
    BOOL IsFirst;
} USERCONTEXT, *PUSERCONTEXT;

typedef union
{
    char c;
    wchar_t wc;
    __int64 i64;
    unsigned __int64 ui64;
    bool b;
    long l;
    unsigned long ul;
    double d;
} BASIC_TYPE;

// Applies non-bold/bright cyan to foreground
#define CONSOLE_CYAN_FORMAT "\x1b[36m"

#define BOOL_TRUE "TRUE"
#define BOOL_FALSE "FALSE"
#define LONGDOUBLE_FORMAT "%Lg"

// https://accu.org/journals/overload/29/165/orr
// https://github.com/rogerorr/NtTrace/blob/main/src/SymbolEngine.cpp#L1185
// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code#showing-variables-using-the-windows-debugging-api

static BOOL CALLBACK EnumCallbackProc(PSYMBOL_INFOW pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    PUSERCONTEXT User = (PUSERCONTEXT) UserContext;

    if (pSymInfo->Flags & SYMFLAG_NULL ||
        (User->DataIsParam && !(pSymInfo->Flags & SYMFLAG_PARAMETER)) ||
        (!User->DataIsParam && pSymInfo->Flags & SYMFLAG_PARAMETER)) return FALSE; // Stop enumeration

    if (User->DataIsParam)
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
            strlen(CONSOLE_DEFAULT_FORMAT));
        User->p += strlen(CONSOLE_DEFAULT_FORMAT);
    }

    DWORD64 Base;

    if (pSymInfo->Flags & SYMFLAG_REGREL)
        Base = GetRegisterBase(pSymInfo, User->pContext, User->is_64bit);
    else if (pSymInfo->Flags & SYMFLAG_FRAMEREL)
        Base = *User->pBase;

    DWORD DTag;

    *User->p++ = '=';
    SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_SYMTAG, &DTag);

    if (DTag == SymTagArrayType ||
        DTag == SymTagUDT ||
        DTag == SymTagTaggedUnionCase)
        User->p = _ui64toaddr(pSymInfo->Address + Base, User->p, User->is_64bit);
    else
    {
        BASIC_TYPE bt = {};

        // add SYMFLAG_VALUEPRESENT
        NtReadVirtualMemory(User->hProcess, (PVOID) (pSymInfo->Address + Base), &bt, SymbolSize, NULL); // pSymInfo->Size

        if (DTag == SymTagPointerType) User->p = _ui64toaddr(bt.ui64, User->p, User->is_64bit);
        else
        {
            DWORD BaseType;
            SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_BASETYPE, &BaseType);

            switch (BaseType)
            {
                case btChar:
                    User->p = dtoa(bt.c, User->p);
                    break;
                case btWChar:
                    User->p = dtoa(bt.wc, User->p);
                    break;
                case btInt:
                    User->p = dtoa(bt.i64, User->p);
                    break;
                case btUInt:
                    User->p = dtoa(bt.ui64, User->p);
                    break;
                case btFloat:
                    User->p += __builtin_sprintf(User->p, LONGDOUBLE_FORMAT, bt.d);
                    break;
                case btBool:
                    if (bt.b)
                    {
                        memcpy(User->p, BOOL_TRUE, strlen(BOOL_TRUE));
                        User->p += strlen(BOOL_TRUE);
                    } else
                    {
                        memcpy(User->p, BOOL_FALSE, strlen(BOOL_FALSE));
                        User->p += strlen(BOOL_FALSE);
                    }
                    break;
                case btLong:
                    User->p = dtoa(bt.l, User->p);
                    break;
                case btULong:
                    User->p = dtoa(bt.ul, User->p);
                    break;
                case btHresult:
                    User->p = _ultoa16u(bt.ul, User->p);
                    break;
            }
        }
    }

    if (!User->DataIsParam) *User->p++ = '\n';

    return TRUE; // Continue enumeration
}

BOOL NTAPI ReadMemoryRoutineLocal(HANDLE hProcess, DWORD64 qwBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    SIZE_T r;

    if (!NT_SUCCESS(NtReadVirtualMemory(hProcess, (PVOID) qwBaseAddress, lpBuffer, nSize, &r))) return FALSE;

    *lpNumberOfBytesRead = r;

    return TRUE;
}