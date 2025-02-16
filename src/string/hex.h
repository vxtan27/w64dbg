/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

// Uppercase hex lookup table
static const char HexTableUpper[16] =
{
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
};

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
static const char HexTableLower[16] =
{
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f'
};

// Convert unsigned long to lowercase hex string
static __forceinline char *_ultoa16(unsigned long value, char *str)
{
    unsigned long num = value;

    do ++str; while ((num >>= 4)); // Advance past last digit

    char *ptr = str;

    do *--ptr = HexTableLower[value & 0xF]; // Reverse convert
    while ((value >>= 4));

    return str;
}

// Reverse convert unsigned long long to lowercase hex string
static __forceinline char *_ui64toa(unsigned _int64 value, char *str)
{
    unsigned long long num = value;

    do ++str; while ((num >>= 4)); // Advance past last digit

    char *ptr = str;

    do *--ptr = HexTableLower[value & 0xF]; // Reverse convert
    while ((value >>= 4));

    return str;

}

// Convert value to lowercase memory address string
static __forceinline char *_ui64toaddr(unsigned _int64 value, char *str, BOOL is_64bit)
{
    *str++ = '0'; *str++ = 'x'; // Address prefix
    return is_64bit ? _ui64toa(value, str) : _ultoa16(value, str);
}

// Reverse convert unsigned long to lowercase hex string
static __forceinline char *__ultoa16(unsigned long value, char *str)
{
    do *--str = HexTableLower[value & 0xF];
    while ((value >>= 4));

    return str;
}

// Reverse convert unsigned long long to lowercase hex string
static __forceinline char *__ui64toa(unsigned _int64 value, char *str)
{
    do *--str = HexTableLower[value & 0xF];
    while ((value >>= 4));

    return str;
}

// Convert value to lowercase memory address string
static __forceinline char *__ui64toaddr(unsigned _int64 value, char *str, BOOL is_64bit)
{
    *str++ = '0'; *str++ = 'x'; // Address prefix

    char *stop, *ptr = str;

    if (is_64bit)
    { // 64-bit hex (16 digits)
        str += 16; // Advance past last digit
        stop = __ui64toa(value, str);
    } else
    { // 32-bit hex (8 digits)
        str += 8; // Advance past last digit
        stop = __ultoa16(value, str);
    }

    // Zero-pad
    while (ptr < stop) *ptr++ = '0';

    return str;
}