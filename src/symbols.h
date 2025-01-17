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

// https://github.com/microsoft/microsoft-pdb/blob/master/include/cvconst.h
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
                        User->p += sprintf(User->p, p, value.QuadPart);
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

        User->p += sprintf(User->p, p, value.u.LowPart);

        if (User->DataIsLocal) *User->p++ = '\n';
    }

    return TRUE; // Continue enumeration
}