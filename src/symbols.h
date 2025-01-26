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
    DWORD64 *Offset;
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
// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code#showing-variables-using-the-windows-debugging-api

static
__forceinline
DWORD64 GetSymbolValue(PSYMBOL_INFOW pSymInfo, PVOID ContextRecord, DWORD bx64win)
{
    if (bx64win)
    {
        PCONTEXT pContext = (PCONTEXT) ContextRecord;

        switch (pSymInfo->Register)
        { // 64-bit regular registers
            case 1: // CV_AMD64_AL
            case 2: // CV_AMD64_CL
            case 3: // CV_AMD64_DL
            case 4: // CV_AMD64_BL
                return (&pContext->Rax)[pSymInfo->Register - 1] & 0xFF;
            case 5: // CV_AMD64_AH
            case 6: // CV_AMD64_CH
            case 7: // CV_AMD64_DH
            case 8: // CV_AMD64_BH
                return ((&pContext->Rax)[pSymInfo->Register - 5] >> 8) & 0xFF;
            case 9: // CV_AMD64_AX
            case 10: // CV_AMD64_CX
            case 11: // CV_AMD64_DX
            case 12: // CV_AMD64_BX
                return (&pContext->Rax)[pSymInfo->Register - 9] & 0xFFFF;
            case 13: // CV_AMD64_SP
            case 14: // CV_AMD64_BP
            case 15: // CV_AMD64_SI
            case 16: // CV_AMD64_DI
                return (&pContext->Rsp)[pSymInfo->Register - 13];
            case 17: // CV_AMD64_EAX
            case 18: // CV_AMD64_ECX
            case 19: // CV_AMD64_EDX
            case 20: // CV_AMD64_EBX
            case 21: // CV_AMD64_ESP
            case 22: // CV_AMD64_EBP
            case 23: // CV_AMD64_ESI
            case 24: // CV_AMD64_EDI
                return (&pContext->Rax)[pSymInfo->Register - 17] & 0xFFFFFFFF;
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
            case 128: // CV_AMD64_ST0
            case 129: // CV_AMD64_ST1
            case 130: // CV_AMD64_ST2
            case 131: // CV_AMD64_ST3
            case 132: // CV_AMD64_ST4
            case 133: // CV_AMD64_ST5
            case 134: // CV_AMD64_ST6
            case 135: // CV_AMD64_ST7
                return (&pContext->Legacy[0])[pSymInfo->Register - 128].Low;
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
            case 146: // CV_AMD64_MM0
            case 147: // CV_AMD64_MM1
            case 148: // CV_AMD64_MM2
            case 149: // CV_AMD64_MM3
            case 150: // CV_AMD64_MM4
            case 151: // CV_AMD64_MM5
            case 152: // CV_AMD64_MM6
            case 153: // CV_AMD64_MM7
            case 154: // CV_AMD64_XMM0
            case 155: // CV_AMD64_XMM1
            case 156: // CV_AMD64_XMM2
            case 157: // CV_AMD64_XMM3
            case 158: // CV_AMD64_XMM4
            case 159: // CV_AMD64_XMM5
            case 160: // CV_AMD64_XMM6
            case 161: // CV_AMD64_XMM7
                return (&pContext->Legacy[0])[pSymInfo->Register - 146].Low;
            case 162: // CV_AMD64_XMM0_0
            case 166: // CV_AMD64_XMM1_0
            case 170: // CV_AMD64_XMM2_0
            case 174: // CV_AMD64_XMM3_0
            case 178: // CV_AMD64_XMM4_0
            case 182: // CV_AMD64_XMM5_0
            case 186: // CV_AMD64_XMM6_0
            case 190: // CV_AMD64_XMM7_0
                return (&pContext->Xmm0)[(pSymInfo->Register - 162) >> 2].Low & 0xFFFFFFFF;
            case 163: // CV_AMD64_XMM0_1
            case 167: // CV_AMD64_XMM1_1
            case 171: // CV_AMD64_XMM2_1
            case 175: // CV_AMD64_XMM3_1
            case 179: // CV_AMD64_XMM4_1
            case 183: // CV_AMD64_XMM5_1
            case 187: // CV_AMD64_XMM6_1
            case 191: // CV_AMD64_XMM7_1
                return ((&pContext->Xmm0)[(pSymInfo->Register - 163) >> 2].Low >> 32) & 0xFFFFFFFF;
            case 164: // CV_AMD64_XMM0_2
            case 168: // CV_AMD64_XMM1_2
            case 172: // CV_AMD64_XMM2_2
            case 176: // CV_AMD64_XMM3_2
            case 180: // CV_AMD64_XMM4_2
            case 184: // CV_AMD64_XMM5_2
            case 188: // CV_AMD64_XMM6_2
            case 192: // CV_AMD64_XMM7_2
                return (&pContext->Xmm0)[(pSymInfo->Register - 164) >> 2].High & 0xFFFFFFFF;
            case 165: // CV_AMD64_XMM0_3
            case 169: // CV_AMD64_XMM1_3
            case 173: // CV_AMD64_XMM2_3
            case 177: // CV_AMD64_XMM3_3
            case 181: // CV_AMD64_XMM4_3
            case 185: // CV_AMD64_XMM5_3
            case 189: // CV_AMD64_XMM6_3
            case 193: // CV_AMD64_XMM7_3
                return ((&pContext->Xmm0)[(pSymInfo->Register - 165) >> 2].High >> 32) & 0xFFFFFFFF;
            case 194: // CV_AMD64_XMM0L
            case 195: // CV_AMD64_XMM1L
            case 196: // CV_AMD64_XMM2L
            case 197: // CV_AMD64_XMM3L
            case 198: // CV_AMD64_XMM4L
            case 199: // CV_AMD64_XMM5L
            case 200: // CV_AMD64_XMM6L
            case 201: // CV_AMD64_XMM7L
                return (&pContext->Xmm0)[pSymInfo->Register - 194].Low;
            case 202: // CV_AMD64_XMM0H
            case 203: // CV_AMD64_XMM1H
            case 204: // CV_AMD64_XMM2H
            case 205: // CV_AMD64_XMM3H
            case 206: // CV_AMD64_XMM4H
            case 207: // CV_AMD64_XMM5H
            case 208: // CV_AMD64_XMM6H
            case 209: // CV_AMD64_XMM7H
                return (&pContext->Xmm0)[pSymInfo->Register - 202].High;
            case 211: // CV_AMD64_MXCSR
                return pContext->MxCsr;
            case 220: // CV_AMD64_EMM0L
            case 221: // CV_AMD64_EMM1L
            case 222: // CV_AMD64_EMM2L
            case 223: // CV_AMD64_EMM3L
            case 224: // CV_AMD64_EMM4L
            case 225: // CV_AMD64_EMM5L
            case 226: // CV_AMD64_EMM6L
            case 227: // CV_AMD64_EMM7
                return (&pContext->Xmm0)[pSymInfo->Register - 220].Low;
            case 228: // CV_AMD64_EMM0H
            case 229: // CV_AMD64_EMM1H
            case 230: // CV_AMD64_EMM2H
            case 231: // CV_AMD64_EMM3H
            case 232: // CV_AMD64_EMM4H
            case 233: // CV_AMD64_EMM5H
            case 234: // CV_AMD64_EMM6H
            case 235: // CV_AMD64_EMM7H
                return (&pContext->Xmm0)[pSymInfo->Register - 228].High;
            case 236: // CV_AMD64_MM00
            case 238: // CV_AMD64_MM10
            case 240: // CV_AMD64_MM20
            case 242: // CV_AMD64_MM30
            case 244: // CV_AMD64_MM40
            case 246: // CV_AMD64_MM50
            case 248: // CV_AMD64_MM60
            case 250: // CV_AMD64_MM70
                return (&pContext->Legacy[0])[(pSymInfo->Register - 236) >> 1].Low;
            case 237: // CV_AMD64_MM01
            case 239: // CV_AMD64_MM11
            case 241: // CV_AMD64_MM21
            case 243: // CV_AMD64_MM31
            case 245: // CV_AMD64_MM41
            case 247: // CV_AMD64_MM51
            case 249: // CV_AMD64_MM61
            case 251: // CV_AMD64_MM71
                return (&pContext->Legacy[0])[(pSymInfo->Register - 237) >> 1].High;
            case 252: // CV_AMD64_XMM8
            case 253: // CV_AMD64_XMM9
            case 254: // CV_AMD64_XMM10
            case 255: // CV_AMD64_XMM11
            case 256: // CV_AMD64_XMM12
            case 257: // CV_AMD64_XMM13
            case 258: // CV_AMD64_XMM14
            case 259: // CV_AMD64_XMM15
                return (&pContext->Xmm8)[pSymInfo->Register - 252].Low;
            case 260: // CV_AMD64_XMM8_0
            case 264: // CV_AMD64_XMM9_0
            case 268: // CV_AMD64_XMM10_0
            case 272: // CV_AMD64_XMM11_0
            case 276: // CV_AMD64_XMM12_0
            case 280: // CV_AMD64_XMM13_0
            case 284: // CV_AMD64_XMM14_0
            case 288: // CV_AMD64_XMM15_0
                return (&pContext->Xmm8)[(pSymInfo->Register - 260) >> 2].Low & 0xFFFFFFFF;
            case 261: // CV_AMD64_XMM8_1
            case 265: // CV_AMD64_XMM9_1
            case 269: // CV_AMD64_XMM10_1
            case 273: // CV_AMD64_XMM11_1
            case 277: // CV_AMD64_XMM12_1
            case 281: // CV_AMD64_XMM13_1
            case 285: // CV_AMD64_XMM14_1
            case 289: // CV_AMD64_XMM15_1
                return ((&pContext->Xmm8)[(pSymInfo->Register - 261) >> 2].Low >> 32) & 0xFFFFFFFF;
            case 262: // CV_AMD64_XMM8_2
            case 266: // CV_AMD64_XMM9_2
            case 270: // CV_AMD64_XMM10_2
            case 274: // CV_AMD64_XMM11_2
            case 278: // CV_AMD64_XMM12_2
            case 282: // CV_AMD64_XMM13_2
            case 286: // CV_AMD64_XMM14_2
            case 290: // CV_AMD64_XMM15_2
                return (&pContext->Xmm8)[(pSymInfo->Register - 262) >> 2].High & 0xFFFFFFFF;
            case 263: // CV_AMD64_XMM8_3
            case 267: // CV_AMD64_XMM9_3
            case 271: // CV_AMD64_XMM10_3
            case 275: // CV_AMD64_XMM11_3
            case 279: // CV_AMD64_XMM12_3
            case 283: // CV_AMD64_XMM13_3
            case 291: // CV_AMD64_XMM15_3
                return ((&pContext->Xmm8)[(pSymInfo->Register - 263) >> 2].High >> 32) & 0xFFFFFFFF;
            case 292: // CV_AMD64_XMM8L
            case 293: // CV_AMD64_XMM9L
            case 294: // CV_AMD64_XMM10L
            case 295: // CV_AMD64_XMM11L
            case 296: // CV_AMD64_XMM12L
            case 297: // CV_AMD64_XMM13L
            case 298: // CV_AMD64_XMM14L
            case 299: // CV_AMD64_XMM15L
                return (&pContext->Xmm8)[pSymInfo->Register - 292].Low;
            case 300: // CV_AMD64_XMM8H
            case 301: // CV_AMD64_XMM9H
            case 302: // CV_AMD64_XMM10H
            case 303: // CV_AMD64_XMM11H
            case 304: // CV_AMD64_XMM12H
            case 305: // CV_AMD64_XMM13H
            case 306: // CV_AMD64_XMM14H
            case 307: // CV_AMD64_XMM15H
                return (&pContext->Xmm8)[pSymInfo->Register - 300].High;
            case 308: // CV_AMD64_EMM8L
            case 309: // CV_AMD64_EMM9L
            case 310: // CV_AMD64_EMM10L
            case 311: // CV_AMD64_EMM11L
            case 312: // CV_AMD64_EMM12L
            case 313: // CV_AMD64_EMM13L
            case 314: // CV_AMD64_EMM14L
            case 315: // CV_AMD64_EMM15L
                return (&pContext->Xmm8)[pSymInfo->Register - 308].Low;
            case 316: // CV_AMD64_EMM8H
            case 317: // CV_AMD64_EMM9H
            case 318: // CV_AMD64_EMM10H
            case 319: // CV_AMD64_EMM11H
            case 320: // CV_AMD64_EMM12H
            case 321: // CV_AMD64_EMM13H
            case 322: // CV_AMD64_EMM14H
            case 323: // CV_AMD64_EMM15H
                return (&pContext->Xmm8)[pSymInfo->Register - 316].High;
            case 324: // CV_AMD64_SIL
                return pContext->Rsi & 0xFF;
            case 325: // CV_AMD64_DIL
                return pContext->Rdi & 0xFF;
            case 326: // CV_AMD64_BPL
                return pContext->Rbp & 0xFF;
            case 327: // CV_AMD64_SPL
                return pContext->Rsp & 0xFF;
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
                return (&pContext->R8)[pSymInfo->Register - 360] & 0xFFFFFFFF;
            case 368: // CV_AMD64_YMM0
            case 369: // CV_AMD64_YMM1
            case 370: // CV_AMD64_YMM2
            case 371: // CV_AMD64_YMM3
            case 372: // CV_AMD64_YMM4
            case 373: // CV_AMD64_YMM5
            case 374: // CV_AMD64_YMM6
            case 375: // CV_AMD64_YMM7
            case 376: // CV_AMD64_YMM8
            case 377: // CV_AMD64_YMM9
            case 378: // CV_AMD64_YMM10
            case 379: // CV_AMD64_YMM11
            case 380: // CV_AMD64_YMM12
            case 381: // CV_AMD64_YMM13
            case 382: // CV_AMD64_YMM14
            case 383: // CV_AMD64_YMM15
                return (&pContext->Xmm0)[pSymInfo->Register - 368].Low;
            case 384: // CV_AMD64_YMM0H
            case 385: // CV_AMD64_YMM1H
            case 386: // CV_AMD64_YMM2H
            case 387: // CV_AMD64_YMM3H
            case 388: // CV_AMD64_YMM4H
            case 389: // CV_AMD64_YMM5H
            case 390: // CV_AMD64_YMM6H
            case 391: // CV_AMD64_YMM7H
            case 392: // CV_AMD64_YMM8H
            case 393: // CV_AMD64_YMM9H
            case 394: // CV_AMD64_YMM10H
            case 395: // CV_AMD64_YMM11H
            case 396: // CV_AMD64_YMM12H
            case 397: // CV_AMD64_YMM13H
            case 398: // CV_AMD64_YMM14H
            case 399: // CV_AMD64_YMM15H
                return (&pContext->Xmm0)[pSymInfo->Register - 384].High;
            case 400: // CV_AMD64_XMM0IL
            case 401: // CV_AMD64_XMM1IL
            case 402: // CV_AMD64_XMM2IL
            case 403: // CV_AMD64_XMM3IL
            case 404: // CV_AMD64_XMM4IL
            case 405: // CV_AMD64_XMM5IL
            case 406: // CV_AMD64_XMM6IL
            case 407: // CV_AMD64_XMM7IL
            case 408: // CV_AMD64_XMM8IL
            case 409: // CV_AMD64_XMM9IL
            case 410: // CV_AMD64_XMM10IL
            case 411: // CV_AMD64_XMM11IL
            case 412: // CV_AMD64_XMM12IL
            case 413: // CV_AMD64_XMM13IL
            case 414: // CV_AMD64_XMM14IL
            case 415: // CV_AMD64_XMM15IL
                return (&pContext->Xmm0)[pSymInfo->Register - 400].Low;
            case 416: // CV_AMD64_XMM0IH
            case 417: // CV_AMD64_XMM1IH
            case 418: // CV_AMD64_XMM2IH
            case 419: // CV_AMD64_XMM3IH
            case 420: // CV_AMD64_XMM4IH
            case 421: // CV_AMD64_XMM5IH
            case 422: // CV_AMD64_XMM6IH
            case 423: // CV_AMD64_XMM7IH
            case 424: // CV_AMD64_XMM8IH
            case 425: // CV_AMD64_XMM9IH
            case 426: // CV_AMD64_XMM10IH
            case 427: // CV_AMD64_XMM11IH
            case 428: // CV_AMD64_XMM12IH
            case 429: // CV_AMD64_XMM13IH
            case 430: // CV_AMD64_XMM14IH
            case 431: // CV_AMD64_XMM15IH
                return (&pContext->Xmm0)[pSymInfo->Register - 416].High;
            default:
                printf("%ws %u\n\n\n\n", pSymInfo->Name, pSymInfo->Register);
                return 0;
        }
    } else
    {
        PWOW64_CONTEXT pContext = (PWOW64_CONTEXT) ContextRecord;

        switch (pSymInfo->Register)
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
                printf("%ws %u\n\n\n\n", pSymInfo->Name, pSymInfo->Register);
                return 0;
        }
    }
}

// https://accu.org/journals/overload/29/165/orr
// https://github.com/rogerorr/NtTrace/blob/main/src/SymbolEngine.cpp#L1185

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
            DWORD64 Offset;
            LONG_DOUBLE value = {};
            if (pSymInfo->Flags & SYMFLAG_REGREL)
                Offset = GetSymbolValue(pSymInfo, User->pContext, User->bx64win);
            else if (pSymInfo->Flags & SYMFLAG_FRAMEREL)
                Offset = *User->Offset;
            else Offset = 0;

            if (DTag == SymTagArrayType ||
                DTag == SymTagUDT ||
                DTag == SymTagTaggedUnionCase)
            {
                *User->p++ = '0';
                *User->p++ = 'x';
                value.u.LowPart.QuadPart = pSymInfo->Address + Offset;
                _Format = LOW_HEX_FORMAT;
            } else
            {
                ULONG64 Len;

                // add SYMFLAG_VALUEPRESENT
                SymGetTypeInfo(User->hProcess, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_LENGTH, &Len);
                NtReadVirtualMemory(User->hProcess, (PVOID) (pSymInfo->Address + Offset), &value, Len, NULL);

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