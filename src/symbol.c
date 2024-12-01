#pragma once

#include "resource.h"
#include "ntoskrnl.h"
#include <dbghelp.h>
#include <stdio.h>

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

// https://github.com/rogerorr/articles/tree/main/Debugging_Optimised_Code
// https://gist.github.com/HoShiMin/779d1c5e96e50a653ca43511b7bcb69a

BOOL CALLBACK EnumCallbackProc(PSYMBOL_INFO pSymbol, ULONG SymbolSize, PVOID UserContext)
{
    (void) SymbolSize;
    if (pSymbol->Flags & SYMFLAG_PARAMETER)
    {
        DWORD DTag;
        USERCONTEXT *Context = (USERCONTEXT *) UserContext;

        SymGetTypeInfo(Context->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_SYMTAG, &DTag);

        if (DTag == SymTagTypedef)
        {
            // Resolve the base type from the underlying type
            SymGetTypeInfo(Context->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_TYPE, &DTag);
            SymGetTypeInfo(Context->hProcess, pSymbol->ModBase, DTag, TI_GET_SYMTAG, &DTag);
        }

        if (!Context->IsFirst)
        {
            *Context->p++ = ',';
            *Context->p++ = ' ';
        } else Context->IsFirst = FALSE;

        if (Context->Console)
        {
            memcpy(Context->p, "\x1b[36m", 5);
            Context->p += 5;
        }

        memcpy(Context->p, pSymbol->Name, pSymbol->NameLen);
        Context->p += pSymbol->NameLen;

        if (Context->Console)
        {
            memcpy(Context->p, W64DBG_IN, 3);
            Context->p += 3;
        }

        *Context->p++ = '=';

        if (DTag == SymTagArrayType ||
            DTag == SymTagUDT ||
            DTag == SymTagTaggedUnionCase)
        {
            Context->p += sprintf(Context->p, "0x%I64x", Context->Offset + pSymbol->Address);
            return TRUE;
        }

        SIZE_T Len;

        SymGetTypeInfo(Context->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_LENGTH, &Len);
        NtReadVirtualMemory(Context->hProcess, (PVOID)(Context->Offset + pSymbol->Address), &Len, Len, NULL);

        if (DTag == SymTagPointerType)
        {
            Context->p += sprintf(Context->p, "0x%I64x", Len);
        } else
        {
            char *p;
            DWORD BaseType;
            SymGetTypeInfo(Context->hProcess, pSymbol->ModBase, pSymbol->TypeIndex, TI_GET_BASETYPE, &BaseType);
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
                    p = "%lf";
                    break;
                /*
                case btBCD:
                case btVariant:
                case btComplex:
                case btBSTR:
                */
                default:
                    p = "0x%I64x";
            }
            Context->p += sprintf(Context->p, p, Len);
        }

    }

    return TRUE; // Continue enumeration
}