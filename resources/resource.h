// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once
#pragma code_page(65001) // UTF-8

#include <verrsrc.h>

// Version string macros
#define _VER_STRINGIFY_W(a, b, c, d) L###a "." #b "." #c "." #d
#define VER_STRINGIFY_W(a, b, c, d) _VER_STRINGIFY_W(a, b, c, d)

// Version information
#define VER_MAJOR                2
#define VER_MINOR                0
#define VER_PATCH                0
#define VER_BUILD                0

#ifdef _DEBUG
#define VER_FILEFLAGS            VS_FF_DEBUG
#else
#define VER_FILEFLAGS            0x00000000L
#endif

#define VER_FILEVERSION          VER_MAJOR, VER_MINOR, VER_PATCH, VER_BUILD
#define VER_PRODUCTVERSION       VER_MAJOR, VER_MINOR, VER_PATCH, VER_BUILD
#define VER_FILEFLAGSMASK        VS_FFI_FILEFLAGSMASK
#define VER_FILEOS               VOS_NT_WINDOWS32
#define VER_FILETYPE             VFT_APP
#define VER_FILESUBTYPE          VFT2_UNKNOWN

// String information block
#define VER_VERSION_UNICODE_LANG L"040904B0" // LANG_ENGLISH/SUBLANG_ENGLISH_US, Unicode CP
#define VER_COMPANYNAME_STR      L"Xuan Tan"
#define VER_FILEDESCRIPTION_STR  L"Native x64 Windows Debugging Utility"
#define VER_FILEVERSION_STR      VER_STRINGIFY_W(VER_MAJOR, VER_MINOR, VER_PATCH, VER_BUILD)
#define VER_INTERNALNAME_STR     L"w64dbg"
#define VER_LEGALCOPYRIGHT_STR   L"© 2024-2025 Xuan Tan. All rights reserved."
#define VER_LEGALTRADEMARKS_STR  L"Licensed under the BSD-3-Clause."
#define VER_ORIGINALFILENAME_STR L"w64dbg.exe"
#define VER_PRODUCTNAME_STR      L"w64dbg"
#define VER_PRODUCTVERSION_STR   VER_FILEVERSION_STR

// Variable information block
#define VER_VERSION_TRANSLATION  0x0400, 65001 // LANG_NEUTRAL/SUBLANG_DEFAULT, UTF-8 CP
