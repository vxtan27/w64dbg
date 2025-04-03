// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

#include <cvconst.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4715)
#endif

DWORD64 GetRegisterBase32(PSYMBOL_INFOW pSymInfo, PWOW64_CONTEXT pContext) {
    switch (pSymInfo->Register) {
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

DWORD64 GetRegisterBase64(PSYMBOL_INFOW pSymInfo, PCONTEXT pContext) {
    switch (pSymInfo->Register) {
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

DWORD64 GetRegisterBase(PSYMBOL_INFOW pSymInfo, PCVOID ContextRecord, DWORD b64bit) {
    return b64bit ? GetRegisterBase64(pSymInfo, (PCONTEXT) ContextRecord)
                    : GetRegisterBase32(pSymInfo, (PWOW64_CONTEXT) ContextRecord);
}

typedef struct {
    PSTR p;
    BOOL DataIsParam;
    BOOL bConsole;
    PDWORD64 pBase;
    PCVOID pContext;
    HANDLE hProcess;
    DWORD b64bit;
    PSTR pEnd;
} USERCONTEXT, *PUSERCONTEXT;

typedef union {
    bool b;
    char c;
    long l;
    double d;
    wchar_t wc;
    __int64 i64;
    unsigned long ul;
    unsigned __int64 ui64;
} BASIC_TYPE;

// Macro String
#define TRUE_STR _CRT_STRINGIZE_(TRUE)
#define FALSE_STR _CRT_STRINGIZE_(FALSE)
// Apply non-bold/bright cyan to foreground
#define CONSOLE_CYAN_FORMAT "\x1b[36m"

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif

#include <dragonbox_to_chars.cpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// https://accu.org/journals/overload/29/165/orr
// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code#showing-variables-using-the-windows-debugging-api

BOOL CALLBACK EnumSymbolsProcW(PSYMBOL_INFOW pSymInfo, ULONG SymbolSize, PVOID UserContext) {
    (void) SymbolSize;
    PUSERCONTEXT User = (PUSERCONTEXT) UserContext;

    if (pSymInfo->Flags & SYMFLAG_NULL ||
        (User->DataIsParam && !(pSymInfo->Flags & SYMFLAG_PARAMETER)) ||
        (!User->DataIsParam && pSymInfo->Flags & SYMFLAG_PARAMETER)) return FALSE; // Stop enumeration

    if (!User->DataIsParam) {
        memset(User->p, ' ', 8);
        User->p += 8;
    }

    if (User->bConsole) {
        memcpy(User->p, CONSOLE_CYAN_FORMAT, 5);
        User->p += 5;
    }

    User->p += ConvertUnicodeToUTF8(pSymInfo->Name,
        (pSymInfo->NameLen - 1) << 1, User->p, User->pEnd - User->p);

    if (User->bConsole) {
        memcpy(User->p, CONSOLE_DEFAULT_FORMAT,
            strlen(CONSOLE_DEFAULT_FORMAT));
        User->p += strlen(CONSOLE_DEFAULT_FORMAT);
    }

    DWORD64 Base;

    if (pSymInfo->Flags & SYMFLAG_REGREL)
        Base = GetRegisterBase(pSymInfo, User->pContext, User->b64bit);
    else if (pSymInfo->Flags & SYMFLAG_FRAMEREL)
        Base = *User->pBase;

    DWORD DTag;

    *User->p++ = '=';
    SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_SYMTAG, &DTag);

    if (DTag == SymTagArrayType ||
        DTag == SymTagUDT ||
        DTag == SymTagTaggedUnionCase) {
        if (User->b64bit)
            User->p = conversion::addr::from_int(User->p, pSymInfo->Address + Base);
        else User->p = conversion::addr::from_int(User->p, (DWORD) (pSymInfo->Address + Base));
    } else {
        ULONG64 Len;
        BASIC_TYPE bt = {};

        // add SYMFLAG_VALUEPRESENT
        SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_LENGTH, &Len);
        NtReadVirtualMemory(User->hProcess, (PVOID) (pSymInfo->Address + Base), &bt, Len, NULL); // pSymInfo->Size

        if (DTag == SymTagPointerType) {
            if (User->b64bit)
                User->p = conversion::addr::from_int(User->p, bt.ui64);
            else User->p = conversion::addr::from_int(User->p, bt.ul);
        } else {
            DWORD BaseType = 0;
            SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_BASETYPE, &BaseType);

            // https://github.com/rogerorr/NtTrace/blob/main/src/SymbolEngine.cpp#L1185
            switch (BaseType) {
            case btChar:
                User->p = conversion::dec::from_int(User->p, bt.c);
                break;
            case btWChar:
                User->p = conversion::dec::from_int(User->p, bt.wc);
                break;
            case btInt:
                User->p = conversion::dec::from_int(User->p, bt.i64);
                break;
            case btUInt:
                User->p = conversion::dec::from_int(User->p, bt.ui64);
                break;
            case btFloat:
                User->p = jkj::dragonbox::to_chars_n(bt.d, User->p);
                break;
            case btBool:
                if (bt.b) {
                    memcpy(User->p, TRUE_STR, strlen(TRUE_STR));
                    User->p += strlen(TRUE_STR);
                } else {
                    memcpy(User->p, FALSE_STR, strlen(FALSE_STR));
                    User->p += strlen(FALSE_STR);
                }
                break;
            case btLong:
                User->p = conversion::dec::from_int(User->p, bt.l);
                break;
            case btULong:
                User->p = conversion::dec::from_int(User->p, bt.ul);
                break;
            case btHresult:
                User->p[0] = '0';
                User->p[1] = 'x';
                User->p = conversion::status::from_int(User->p + 2, bt.ul);
                break;
            }
        }
    }

    if (User->DataIsParam) {
        *User->p++ = ',';
        *User->p++ = ' ';
    } else *User->p++ = '\n';

    return TRUE; // Continue enumeration
}