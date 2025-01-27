/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

enum SymTagEnum {
    SymTagNull,
    SymTagExe,
    SymTagCompiland,
    SymTagCompilandDetails,
    SymTagCompilandEnv,
    SymTagFunction,
    SymTagBlock,
    SymTagData,
    SymTagAnnotation,
    SymTagLabel,
    SymTagPublicSymbol,
    SymTagUDT,
    SymTagEnum,
    SymTagFunctionType,
    SymTagPointerType,
    SymTagArrayType,
    SymTagBaseType,
    SymTagTypedef,
    SymTagBaseClass,
    SymTagFriend,
    SymTagFunctionArgType,
    SymTagFuncDebugStart,
    SymTagFuncDebugEnd,
    SymTagUsingNamespace,
    SymTagVTableShape,
    SymTagVTable,
    SymTagCustom,
    SymTagThunk,
    SymTagCustomType,
    SymTagManagedType,
    SymTagDimension,
    SymTagCallSite,
    SymTagInlineSite,
    SymTagBaseInterface,
    SymTagVectorType,
    SymTagMatrixType,
    SymTagHLSLType,
    SymTagCaller,
    SymTagCallee,
    SymTagExport,
    SymTagHeapAllocationSite,
    SymTagCoffGroup,
    SymTagInlinee,
    SymTagTaggedUnionCase,
};

enum BasicType
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

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

[[deprecated]]
static
__forceinline
DWORD64 _GetRegisterBase32(PSYMBOL_INFOW pSymInfo, PWOW64_CONTEXT pContext)
{
    switch (pSymInfo->Register)
    {
        case 5: // CV_REG_AH
        case 6: // CV_REG_CH
        case 7: // CV_REG_DH
        case 8: // CV_REG_BH
            return ((&pContext->Ebx)[8 - pSymInfo->Register] >> 8) & 0xFF;

        case 25: // CV_REG_ES
            return pContext->SegEs;
        case 26: // CV_REG_CS
            return pContext->SegCs;
        case 27: // CV_REG_SS
            return pContext->SegSs;
        case 28: // CV_REG_DS
            return pContext->SegDs;
        case 29: // CV_REG_FS
            return pContext->SegFs;
        case 30: // CV_REG_GS
            return pContext->SegGs;
        case 32: // CV_REG_FLAGS
            return pContext->EFlags & 0xFFFF;
        case 33: // CV_REG_EIP
            return pContext->Eip;
        case 34: // CV_REG_EFLAGS
            return pContext->EFlags;

        case 90: // CV_REG_DR0
        case 91: // CV_REG_DR1
        case 92: // CV_REG_DR2
        case 93: // CV_REG_DR3
            return (&pContext->Dr0)[pSymInfo->Register - 90];

        case 96: // CV_REG_DR6
            return pContext->Dr6;
        case 97: // CV_REG_DR7
            return pContext->Dr7;

        case 136: // CV_REG_CTRL
            return pContext->FloatSave.ControlWord;
        case 137: // CV_REG_STAT
            return pContext->FloatSave.StatusWord;
        case 138: // CV_REG_TAG
            return pContext->FloatSave.TagWord;
        case 139: // CV_REG_FPIP
        case 144: // CV_REG_FPEIP
            return pContext->FloatSave.ErrorOffset;
        case 140: // CV_REG_FPCS
            return pContext->FloatSave.ErrorSelector;
        case 141: // CV_REG_FPDO
        case 145: // CV_REG_FPEDO
            return pContext->FloatSave.DataOffset;
        case 142: // CV_REG_FPDS
            return pContext->FloatSave.DataSelector;
        case 143: // CV_REG_ISEM
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
        case 5: // CV_AMD64_AH
        case 6: // CV_AMD64_CH
        case 7: // CV_AMD64_DH
        case 8: // CV_AMD64_BH
            return ((&pContext->Rax)[pSymInfo->Register - 5] >> 8) & 0xFF;

        case 25: // CV_AMD64_ES
            return pContext->SegEs;
        case 26: // CV_AMD64_CS
            return pContext->SegCs;
        case 27: // CV_AMD64_SS
            return pContext->SegSs;
        case 28: // CV_AMD64_DS
            return pContext->SegDs;
        case 29: // CV_AMD64_FS
            return pContext->SegFs;
        case 30: // CV_AMD64_GS
            return pContext->SegGs;
        case 32: // CV_AMD64_FLAGS
            return pContext->EFlags & 0xFFFF;
        case 33: // CV_AMD64_RIP
            return pContext->Rip;
        case 34: // CV_AMD64_EFLAGS
            return pContext->EFlags;

        case 90: // CV_AMD64_DR0
        case 91: // CV_AMD64_DR1
        case 92: // CV_AMD64_DR2
        case 93: // CV_AMD64_DR3
            return (&pContext->Dr0)[pSymInfo->Register - 90];

        case 96: // CV_AMD64_DR6
            return pContext->Dr6;
        case 97: // CV_AMD64_DR7
            return pContext->Dr7;

        case 136: // CV_AMD64_CTRL
            return pContext->FltSave.ControlWord;
        case 137: // CV_AMD64_STAT
            return pContext->FltSave.StatusWord;
        case 138: // CV_AMD64_TAG
            return pContext->FltSave.TagWord;
        case 139: // CV_AMD64_FPIP
        case 144: // CV_AMD64_FPEIP
            return pContext->FltSave.ErrorOffset;
        case 140: // CV_AMD64_FPCS
            return pContext->FltSave.ErrorSelector;
        case 141: // CV_AMD64_FPDO
        case 145: // CV_AMD64_FPEDO
            return pContext->FltSave.DataOffset;
        case 142: // CV_AMD64_FPDS
            return pContext->FltSave.DataSelector;
        case 143: // CV_AMD64_ISEM
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
    { // AMD64 registers
        case 1: // CV_REG_AL
        case 2: // CV_REG_CL
        case 3: // CV_REG_DL
        case 4: // CV_REG_BL
            return (&pContext->Ebx)[4 - pSymInfo->Register] & 0xFF;

        case 9: // CV_REG_AX
        case 10: // CV_REG_CX
        case 11: // CV_REG_DX
        case 12: // CV_REG_BX
            return (&pContext->Ebx)[12 - pSymInfo->Register] & 0xFFFF;

        case 13: // CV_REG_SP
            return pContext->Esp & 0xFFFF;
        case 14: // CV_REG_BP
            return pContext->Ebp & 0xFFFF;
        case 15: // CV_REG_SI
            return pContext->Esi & 0xFFFF;
        case 16: // CV_REG_DI
            return pContext->Edi & 0xFFFF;

        case 17: // CV_REG_EAX
        case 18: // CV_REG_ECX
        case 19: // CV_REG_EDX
        case 20: // CV_REG_EBX
            return (&pContext->Ebx)[20 - pSymInfo->Register];

        case 21: // CV_REG_ESP
            return pContext->Esp;
        case 22: // CV_REG_EBP
        case 30006: // CV_ALLREG_VFRAME
            return pContext->Ebp;
        case 23: // CV_REG_ESI
            return pContext->Esi;
        case 24: // CV_REG_EDI
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
    { // 64-bit regular registers
        case 1: // CV_AMD64_AL
        case 2: // CV_AMD64_CL
        case 3: // CV_AMD64_DL
        case 4: // CV_AMD64_BL
            return (&pContext->Rax)[pSymInfo->Register - 1] & 0xFF;

        case 9: // CV_AMD64_AX
        case 10: // CV_AMD64_CX
        case 11: // CV_AMD64_DX
        case 12: // CV_AMD64_BX
        case 13: // CV_AMD64_SP
        case 14: // CV_AMD64_BP
        case 15: // CV_AMD64_SI
        case 16: // CV_AMD64_DI
            return (&pContext->Rax)[pSymInfo->Register - 9] & 0xFFFF;

        case 17: // CV_AMD64_EAX
        case 18: // CV_AMD64_ECX
        case 19: // CV_AMD64_EDX
        case 20: // CV_AMD64_EBX
        case 21: // CV_AMD64_ESP
        case 22: // CV_AMD64_EBP
        case 23: // CV_AMD64_ESI
        case 24: // CV_AMD64_EDI
            return (&pContext->Rax)[pSymInfo->Register - 17];

        case 328: // CV_AMD64_RAX
            return pContext->Rax;
        case 329: // CV_AMD64_RBX
            return pContext->Rbx;
        case 330: // CV_AMD64_RCX
            return pContext->Rcx;
        case 331: // CV_AMD64_RDX
            return pContext->Rdx;
        case 332: // CV_AMD64_RSI
            return pContext->Rsi;
        case 333: // CV_AMD64_RDI
            return pContext->Rdi;
        case 334: // CV_AMD64_RBP
            return pContext->Rbp;
        case 335: // CV_AMD64_RSP
            return pContext->Rsp;

        case 336: // CV_AMD64_R8
        case 337: // CV_AMD64_R9
        case 338: // CV_AMD64_R10
        case 339: // CV_AMD64_R11
        case 340: // CV_AMD64_R12
        case 341: // CV_AMD64_R13
        case 342: // CV_AMD64_R14
        case 343: // CV_AMD64_R15
            return (&pContext->R8)[pSymInfo->Register - 336];

        case 344: // CV_AMD64_R8B
        case 345: // CV_AMD64_R9B
        case 346: // CV_AMD64_R10B
        case 347: // CV_AMD64_R11B
        case 348: // CV_AMD64_R12B
        case 349: // CV_AMD64_R13B
        case 350: // CV_AMD64_R14B
        case 351: // CV_AMD64_R15B
            return (&pContext->R8)[pSymInfo->Register - 344] & 0xFF;

        case 352: // CV_AMD64_R8W
        case 353: // CV_AMD64_R9W
        case 354: // CV_AMD64_R10W
        case 355: // CV_AMD64_R11W
        case 356: // CV_AMD64_R12W
        case 357: // CV_AMD64_R13W
        case 358: // CV_AMD64_R14W
        case 359: // CV_AMD64_R15W
            return (&pContext->R8)[pSymInfo->Register - 352] & 0xFFFF;

        case 360: // CV_AMD64_R8D
        case 361: // CV_AMD64_R9D
        case 362: // CV_AMD64_R10D
        case 363: // CV_AMD64_R11D
        case 364: // CV_AMD64_R12D
        case 365: // CV_AMD64_R13D
        case 366: // CV_AMD64_R14D
        case 367: // CV_AMD64_R15D
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
            const char* _Format;
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