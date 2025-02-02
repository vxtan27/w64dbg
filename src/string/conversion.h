/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

/*
// Convert long to string
static __forceinline char *_ultoa10(unsigned long value, char *str)
{
    unsigned long num = value;

    do ++str; while ((num /= 10)); // Advance past last digit

    char *ptr = str;

    do *--ptr = (value % 10) + '0'; // Reverse convert
    while ((value /= 10));

    return str;
}
*/

// Convert non-negative long [0, 999996] to string
static __forceinline char *_ltoa10(long value, char *str)
{
    unsigned _int64 num = value;

    do ++str; while ((num = (num * 0xCCCD) >> 19)); // Advance past last digit

    char *ptr = str;

    do *--ptr = (value - ((value * 0xCCCDULL) >> 19) * 10) + '0'; // Reverse convert
    while ((value = (value * 0xCCCDULL) >> 19));

    return str;
}

// Convert unsigned long to string
static __forceinline char *_ultoa10(unsigned long value, char *str)
{
    unsigned long num = value;

    do ++str; while ((num /= 10)); // Advance past last digit

    char *ptr = str;

    do *--ptr = (value % 10) + '0'; // Reverse convert
    while ((value /= 10));

    return str;
}

// Uppercase hex lookup table
static const char HexTableUpper[16] = "0123456789ABCDEF";

// Convert unsigned long to uppercase hex string
static __forceinline char *_ultoa16u(unsigned long value, char *str)
{
    unsigned long num = value;

    do ++str; while ((num >>= 4)); // Advance past last digit

    char *ptr = str;

    do *--ptr = HexTableUpper[value & 0xF]; // Reverse convert
    while ((value >>= 4));

    return str;
}

// Lowercase hex lookup table
static const char HexTableLower[16] = "0123456789abcdef";

// Convert unsigned long long to lowercase memory address string
static __forceinline char *_ui64toaddr(unsigned _int64 value, char *str, BOOL is_64bit)
{
    *str++ = '0'; *str++ = 'x'; // Address prefix

    // Zero-pad
    if (is_64bit)
    { // 64-bit hex (16 digits)
        memset(str, '0', 16);
        str += 16; // Advance past last digit
    } else
    { // 32-bit hex (8 digits)
        memset(str, '0', 8);
        str += 8; // Advance past last digit
    }

    char *ptr = str;

    do *--ptr = HexTableLower[value & 0xF]; // Reverse convert
    while ((value >>= 4));

    return str;
}