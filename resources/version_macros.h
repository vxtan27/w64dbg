// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// Macros for version string formatting
#define STR(x)      #x
#define WSTR(x)     L## #x
#define VER_WSTR(x) WSTR(x)

#define VER_NUM_WSTR(maj, min, pat, bld) \
    VER_WSTR(maj) L"." VER_WSTR(min) L"." VER_WSTR(pat) L"." VER_WSTR(bld)
