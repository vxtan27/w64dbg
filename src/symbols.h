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
    HANDLE hProcess;
    PVOID pContext;
    char *p;
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

// https://github.com/microsoft/microsoft-pdb/blob/master/include/cvconst.h

static
__forceinline
DWORD64 GetSymbolOffset(PSYMBOL_INFOW pSymbol, PVOID ContextRecord, DWORD bx64win)
{
    if (bx64win)
    {
        PCONTEXT pContext = (PCONTEXT) ContextRecord;

        switch (pSymbol->Register)
        { // 64-bit regular registers
            case 1: // CV_AMD64_AL
                return pContext->Rax & 0xFF;
            case 2: // CV_AMD64_CL
                return pContext->Rcx & 0xFF;
            case 3: // CV_AMD64_DL
                return pContext->Rdx & 0xFF;
            case 4: // CV_AMD64_BL
                return pContext->Rbx & 0xFF;
            case 5: // CV_AMD64_AH
                return (pContext->Rax >> 8) & 0xFF;
            case 6: // CV_AMD64_CH
                return (pContext->Rcx >> 8) & 0xFF;
            case 7: // CV_AMD64_DH
                return (pContext->Rdx >> 8) & 0xFF;
            case 8: // CV_AMD64_BH
                return (pContext->Rbx >> 8) & 0xFF;
            case 9: // CV_AMD64_AX
                return pContext->Rax & 0xFFFF;
            case 10: // CV_AMD64_CX
                return pContext->Rcx & 0xFFFF;
            case 11: // CV_AMD64_DX
                return pContext->Rdx & 0xFFFF;
            case 12: // CV_AMD64_BX
                return pContext->Rbx & 0xFFFF;
            case 13: // CV_AMD64_SP
            case 335: // CV_AMD64_RSP
                return pContext->Rsp;
            case 14: // CV_AMD64_BP
            case 334: // CV_AMD64_RBP
                return pContext->Rbp;
            case 15: // CV_AMD64_SI
            case 332: // CV_AMD64_RSI
                return pContext->Rsi;
            case 16: // CV_AMD64_DI
            case 333: // CV_AMD64_RDI
                return pContext->Rdi;
            case 17: // CV_AMD64_EAX
                return pContext->Rax & 0xFFFFFFFF;
            case 18: // CV_AMD64_ECX
                return pContext->Rcx & 0xFFFFFFFF;
            case 19: // CV_AMD64_EDX
                return pContext->Rdx & 0xFFFFFFFF;
            case 20: // CV_AMD64_EBX
                return pContext->Rbx & 0xFFFFFFFF;
            case 21: // CV_AMD64_ESP
                return pContext->Rsp & 0xFFFFFFFF;
            case 22: // CV_AMD64_EBP
                return pContext->Rbp & 0xFFFFFFFF;
            case 23: // CV_AMD64_ESI
                return pContext->Rsi & 0xFFFFFFFF;
            case 24: // CV_AMD64_EDI
                return pContext->Rdi & 0xFFFFFFFF;
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
            // case 80: // CV_AMD64_CR0
            // case 81: // CV_AMD64_CR1
            // case 82: // CV_AMD64_CR2
            // case 83: // CV_AMD64_CR3
            // case 84: // CV_AMD64_CR4
            // case 88: // CV_AMD64_CR8
            case 90: // CV_AMD64_DR0
            case 236: // CV_AMD64_MM00
                return pContext->Dr0;
            case 91: // CV_AMD64_DR1
            case 237: // CV_AMD64_MM01
                return pContext->Dr1;
            case 92: // CV_AMD64_DR2
            case 238: // CV_AMD64_MM10
                return pContext->Dr2;
            case 93: // CV_AMD64_DR3
            case 239: // CV_AMD64_MM11
                return pContext->Dr3;
            // case 94: // CV_AMD64_DR4
            // case 240: // CV_AMD64_MM20
            // case 95: // CV_AMD64_DR5
            // case 241: // CV_AMD64_MM21
            case 96: // CV_AMD64_DR6
            case 242: // CV_AMD64_MM30
                return pContext->Dr6;
            case 97: // CV_AMD64_DR7
            case 243: // CV_AMD64_MM31
                return pContext->Dr7;
            // case 98: // CV_AMD64_DR8
            // case 244: // CV_AMD64_MM40
            // case 99: // CV_AMD64_DR9
            // case 245: // CV_AMD64_MM41
            // case 100: // CV_AMD64_DR10
            // case 246: // CV_AMD64_MM50
            // case 101: // CV_AMD64_DR11
            // case 247: // CV_AMD64_MM51
            // case 102: // CV_AMD64_DR12
            // case 248: // CV_AMD64_MM60
            // case 103: // CV_AMD64_DR13
            // case 249: // CV_AMD64_MM61
            // case 104: // CV_AMD64_DR14
            // case 250: // CV_AMD64_MM70
            // case 105: // CV_AMD64_DR15
            // case 251: // CV_AMD64_MM71
            // case 110: // CV_AMD64_GDTR
            // case 111: // CV_AMD64_GDTL
            // case 112: // CV_AMD64_IDTR
            // case 113: // CV_AMD64_IDTL
            // case 114: // CV_AMD64_LDTR
            // case 115: //  CV_AMD64_TR
            case 128: // CV_AMD64_ST0
            case 146: // CV_AMD64_MM0
                return pContext->FltSave.FloatRegisters[0].Low;
            case 129: // CV_AMD64_ST1
            case 147: // CV_AMD64_MM1
                return pContext->FltSave.FloatRegisters[1].Low;
            case 130: // CV_AMD64_ST2
            case 148: // CV_AMD64_MM2
                return pContext->FltSave.FloatRegisters[2].Low;
            case 131: // CV_AMD64_ST3
            case 149: // CV_AMD64_MM3
                return pContext->FltSave.FloatRegisters[3].Low;
            case 132: // CV_AMD64_ST4
            case 150: // CV_AMD64_MM4
                return pContext->FltSave.FloatRegisters[4].Low;
            case 133: // CV_AMD64_ST5
            case 151: // CV_AMD64_MM5
                return pContext->FltSave.FloatRegisters[5].Low;
            case 134: // CV_AMD64_ST6
            case 152: // CV_AMD64_MM6
                return pContext->FltSave.FloatRegisters[6].Low;
            case 135: // CV_AMD64_ST7
            case 153: // CV_AMD64_MM7
                return pContext->FltSave.FloatRegisters[7].Low;
            case 136: // CV_AMD64_CTRL
                return pContext->FltSave.ControlWord;
            case 137: // CV_AMD64_STAT
                return pContext->FltSave.StatusWord;
            case 138: // CV_AMD64_TAG
                return pContext->FltSave.TagWord;
            case 139: // CV_AMD64_FPIP
                return pContext->FltSave.ErrorOpcode;
            case 140: // CV_AMD64_FPCS
                return pContext->FltSave.ErrorSelector;
            case 141: // CV_AMD64_FPDO
                return pContext->FltSave.DataOffset;
            case 142: // CV_AMD64_FPDS
                return pContext->FltSave.DataSelector;
            case 143: // CV_AMD64_ISEM
                return pContext->FltSave.MxCsr;
            case 144: // CV_AMD64_FPEIP
                return pContext->LastBranchToRip;
            case 145: // CV_AMD64_FPEDO
                return pContext->LastExceptionToRip;
            case 154: // CV_AMD64_XMM0
            case 194: // CV_AMD64_XMM0L
            case 220: // CV_AMD64_EMM0L
                return pContext->FltSave.XmmRegisters[0].Low;
            case 155: // CV_AMD64_XMM1
            case 195: // CV_AMD64_XMM1L
            case 221: // CV_AMD64_EMM1L
                return pContext->FltSave.XmmRegisters[1].Low;
            case 156: // CV_AMD64_XMM2
            case 196: // CV_AMD64_XMM2L
            case 222: // CV_AMD64_EMM2L
                return pContext->FltSave.XmmRegisters[2].Low;
            case 157: // CV_AMD64_XMM3
            case 197: // CV_AMD64_XMM3L
            case 223: // CV_AMD64_EMM3L
                return pContext->FltSave.XmmRegisters[3].Low;
            case 158: // CV_AMD64_XMM4
            case 198: // CV_AMD64_XMM4L
            case 224: // CV_AMD64_EMM4L
                return pContext->FltSave.XmmRegisters[4].Low;
            case 159: // CV_AMD64_XMM5
            case 199: // CV_AMD64_XMM5L
            case 225: // CV_AMD64_EMM5L
                return pContext->FltSave.XmmRegisters[5].Low;
            case 160: // CV_AMD64_XMM6
            case 200: // CV_AMD64_XMM6L
            case 226: // CV_AMD64_EMM6L
                return pContext->FltSave.XmmRegisters[6].Low;
            case 161: // CV_AMD64_XMM7
            case 201: // CV_AMD64_XMM7L
            case 227: // CV_AMD64_EMM7
                return pContext->FltSave.XmmRegisters[7].Low;
            case 162: // CV_AMD64_XMM0_0
                return pContext->FltSave.XmmRegisters[0].Low & 0xFFFFFFFF;
            case 163: // CV_AMD64_XMM0_1
                return (pContext->FltSave.XmmRegisters[0].Low >> 32) & 0xFFFFFFFF;
            case 164: // CV_AMD64_XMM0_2
                return pContext->FltSave.XmmRegisters[0].High & 0xFFFFFFFF;
            case 165: // CV_AMD64_XMM0_3
                return (pContext->FltSave.XmmRegisters[0].High >> 32) & 0xFFFFFFFF;
            case 166: // CV_AMD64_XMM1_0
                return pContext->FltSave.XmmRegisters[1].Low & 0xFFFFFFFF;
            case 167: // CV_AMD64_XMM1_1
                return (pContext->FltSave.XmmRegisters[1].Low >> 32) & 0xFFFFFFFF;
            case 168: // CV_AMD64_XMM1_2
                return pContext->FltSave.XmmRegisters[1].High & 0xFFFFFFFF;
            case 169: // CV_AMD64_XMM1_3
                return (pContext->FltSave.XmmRegisters[1].High >> 32) & 0xFFFFFFFF;
            case 170: // CV_AMD64_XMM2_0
                return pContext->FltSave.XmmRegisters[2].Low & 0xFFFFFFFF;
            case 171: // CV_AMD64_XMM2_1
                return (pContext->FltSave.XmmRegisters[2].Low >> 32) & 0xFFFFFFFF;
            case 172: // CV_AMD64_XMM2_2
                return pContext->FltSave.XmmRegisters[2].High & 0xFFFFFFFF;
            case 173: // CV_AMD64_XMM2_3
                return (pContext->FltSave.XmmRegisters[2].High >> 32) & 0xFFFFFFFF;
            case 174: // CV_AMD64_XMM3_0
                return pContext->FltSave.XmmRegisters[3].Low & 0xFFFFFFFF;
            case 175: // CV_AMD64_XMM3_1
                return (pContext->FltSave.XmmRegisters[3].Low >> 32) & 0xFFFFFFFF;
            case 176: // CV_AMD64_XMM3_2
                return pContext->FltSave.XmmRegisters[3].High & 0xFFFFFFFF;
            case 177: // CV_AMD64_XMM3_3
                return (pContext->FltSave.XmmRegisters[3].High >> 32) & 0xFFFFFFFF;
            case 178: // CV_AMD64_XMM4_0
                return pContext->FltSave.XmmRegisters[4].Low & 0xFFFFFFFF;
            case 179: // CV_AMD64_XMM4_1
                return (pContext->FltSave.XmmRegisters[4].Low >> 32) & 0xFFFFFFFF;
            case 180: // CV_AMD64_XMM4_2
                return pContext->FltSave.XmmRegisters[4].High & 0xFFFFFFFF;
            case 181: // CV_AMD64_XMM4_3
                return (pContext->FltSave.XmmRegisters[4].High >> 32) & 0xFFFFFFFF;
            case 182: // CV_AMD64_XMM5_0
                return pContext->FltSave.XmmRegisters[5].Low & 0xFFFFFFFF;
            case 183: // CV_AMD64_XMM5_1
                return (pContext->FltSave.XmmRegisters[5].Low >> 32) & 0xFFFFFFFF;
            case 184: // CV_AMD64_XMM5_2
                return pContext->FltSave.XmmRegisters[5].High & 0xFFFFFFFF;
            case 185: // CV_AMD64_XMM5_3
                return (pContext->FltSave.XmmRegisters[5].High >> 32) & 0xFFFFFFFF;
            case 186: // CV_AMD64_XMM6_0
                return pContext->FltSave.XmmRegisters[6].Low & 0xFFFFFFFF;
            case 187: // CV_AMD64_XMM6_1
                return (pContext->FltSave.XmmRegisters[6].Low >> 32) & 0xFFFFFFFF;
            case 188: // CV_AMD64_XMM6_2
                return pContext->FltSave.XmmRegisters[6].High & 0xFFFFFFFF;
            case 189: // CV_AMD64_XMM6_3
                return (pContext->FltSave.XmmRegisters[6].High >> 32) & 0xFFFFFFFF;
            case 190: // CV_AMD64_XMM7_0
                return pContext->FltSave.XmmRegisters[7].Low & 0xFFFFFFFF;
            case 191: // CV_AMD64_XMM7_1
                return (pContext->FltSave.XmmRegisters[7].Low >> 32) & 0xFFFFFFFF;
            case 192: // CV_AMD64_XMM7_2
                return pContext->FltSave.XmmRegisters[7].High & 0xFFFFFFFF;
            case 193: // CV_AMD64_XMM7_3
                return (pContext->FltSave.XmmRegisters[7].High >> 32) & 0xFFFFFFFF;
            case 202: // CV_AMD64_XMM0H
            case 228: // CV_AMD64_EMM0H
                return pContext->FltSave.XmmRegisters[0].High;
            case 203: // CV_AMD64_XMM1H
            case 229: // CV_AMD64_EMM1H
                return pContext->FltSave.XmmRegisters[1].High;
            case 204: // CV_AMD64_XMM2H
            case 230: // CV_AMD64_EMM2H
                return pContext->FltSave.XmmRegisters[2].High;
            case 205: // CV_AMD64_XMM3H
            case 231: // CV_AMD64_EMM3H
                return pContext->FltSave.XmmRegisters[3].High;
            case 206: // CV_AMD64_XMM4H
            case 232: // CV_AMD64_EMM4H
                return pContext->FltSave.XmmRegisters[4].High;
            case 207: // CV_AMD64_XMM5H
            case 233: // CV_AMD64_EMM5H
                return pContext->FltSave.XmmRegisters[5].High;
            case 208: // CV_AMD64_XMM6H
            case 234: // CV_AMD64_EMM6H
                return pContext->FltSave.XmmRegisters[6].High;
            case 209: // CV_AMD64_XMM7H
            case 235: // CV_AMD64_EMM7H
                return pContext->FltSave.XmmRegisters[7].High;
            case 211: // CV_AMD64_MXCSR
                return pContext->MxCsr;
            case 252: // CV_AMD64_XMM8
                return pContext->FltSave.XmmRegisters[8].Low;
            case 253: // CV_AMD64_XMM9
                return pContext->FltSave.XmmRegisters[9].Low;
            case 254: // CV_AMD64_XMM10
                return pContext->FltSave.XmmRegisters[10].Low;
            case 255: // CV_AMD64_XMM11
                return pContext->FltSave.XmmRegisters[11].Low;
            case 256: // CV_AMD64_XMM12
                return pContext->FltSave.XmmRegisters[12].Low;
            case 257: // CV_AMD64_XMM13
                return pContext->FltSave.XmmRegisters[13].Low;
            case 258: // CV_AMD64_XMM14
                return pContext->FltSave.XmmRegisters[14].Low;
            case 259: // CV_AMD64_XMM15
                return pContext->FltSave.XmmRegisters[15].Low;
            case 328: // CV_AMD64_RAX
                return pContext->Rax;
            case 329: // CV_AMD64_RBX
                return pContext->Rbx;
            case 330: // CV_AMD64_RCX
                return pContext->Rcx;
            case 331: // CV_AMD64_RDX
                return pContext->Rdx;
            default:
                printf("%ws %u\n\n\n\n", pSymbol->Name, pSymbol->Register);
                return 0;
        }
    } else
    {
        PWOW64_CONTEXT pContext = (PWOW64_CONTEXT) ContextRecord;

        switch (pSymbol->Register)
        { // AMD64 registers
            case 17: // CV_AMD64_EAX
                return pContext->Eax;
            case 18: // CV_AMD64_ECX
                return pContext->Ecx;
            case 19: // CV_AMD64_EDX
                return pContext->Edx;
            case 20: // CV_AMD64_EBX
                return pContext->Ebx;
            case 21: // CV_AMD64_ESP
                return pContext->Esp;
            case 22: // CV_AMD64_EBP
                return pContext->Ebp;
            case 23: // CV_AMD64_ESI
                return pContext->Esi;
            case 24: // CV_AMD64_EDI
                return pContext->Edi;
            default:
                printf("%ws %u\n\n\n\n", pSymbol->Name, pSymbol->Register);
                return 0;
        }
    }
}

// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code#showing-variables-using-the-windows-debugging-api
// https://accu.org/journals/overload/29/165/orr
// https://github.com/rogerorr/NtTrace/blob/main/src/SymbolEngine.cpp#L1185

static BOOL CALLBACK EnumCallbackProc(PSYMBOL_INFOW pSymbol, ULONG SymbolSize, PVOID UserContext)
{
    (void) SymbolSize;

    PUSERCONTEXT User = (PUSERCONTEXT) UserContext;

    if ((User->DataIsLocal && !(pSymbol->Flags & SYMFLAG_PARAMETER)) ||
        (!User->DataIsLocal && pSymbol->Flags & SYMFLAG_PARAMETER))
    {
        DWORD DTag;
        DWORD64 Offset = GetSymbolOffset(pSymbol, User->pContext, User->bx64win);
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
            memcpy(User->p, CONSOLE_CYAN_FORMAT, 5);
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
        LONG_DOUBLE value = {};

        *User->p++ = '=';

        if (DTag == SymTagArrayType ||
            DTag == SymTagUDT ||
            DTag == SymTagTaggedUnionCase)
        {
            *User->p++ = '0';
            *User->p++ = 'x';
            value.u.LowPart.QuadPart = pSymbol->Address + Offset;
            p = LOW_HEX_FORMAT;
        } else
        {
            ULONG64 Len;

            SymGetTypeInfo(User->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_LENGTH, &Len);
            NtReadVirtualMemory(User->hProcess, (PVOID) (pSymbol->Address + Offset), &value, Len, NULL);

            if (DTag == SymTagPointerType)
            {
                *User->p++ = '0';
                *User->p++ = 'x';
                p = LOW_HEX_FORMAT;
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
                        p = LONGLONG_FORMAT;
                        break;
                    case btUInt:
                    case btBool:
                    case btULong:
                        p = ULONGLONG_FORMAT;
                        break;
                    case btFloat:
                        p = LONGDOUBLE_FORMAT;
                        User->p += __builtin_sprintf(User->p, p, value.QuadPart);
                        if (User->DataIsLocal) *User->p++ = '\n';
                        return TRUE;
                    case btHresult:
                        p = UP_HEX_FORMAT;
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
                        p = LOW_HEX_FORMAT;
                }
            }

        }

        User->p += __builtin_sprintf(User->p, p, value.u.LowPart);

        if (User->DataIsLocal) *User->p++ = '\n';
    }

    return TRUE; // Continue enumeration
}