/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define _VERSION_TO_WSTR(a, b, c, d) L###a "." #b "." #c "." #d
#define VERSION_TO_WSTR(a, b, c, d) _VERSION_TO_WSTR(a, b, c, d)
