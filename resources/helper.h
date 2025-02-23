/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#define _VER_STRINGIFY_W(a, b, c, d) L###a "." #b "." #c "." #d
#define VER_STRINGIFY_W(a, b, c, d) _VER_STRINGIFY_W(a, b, c, d)
