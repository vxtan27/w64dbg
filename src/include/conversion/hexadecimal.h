// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#pragma once

// Uppercase hex lookup table
const char HexTableUpper[16] = {
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
};

// Convert unsigned long to uppercase hex string
char *_ulto16a(unsigned long value, char *str) {
    unsigned long num = value;

    do ++str; while ((num >>= 4)); // Advance past last digit

    char *ptr = str;

    do *--ptr = HexTableUpper[value & 0xF]; // Reverse convert
    while ((value >>= 4));

    return str;
}