/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

/*
// Convert non-negative long to string
static __forceinline char *_ltoa10(unsigned long value, char *str)
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
    unsigned long cal = value;

    do ++str; while ((cal = (cal * 0xCCCDULL) >> 19)); // Advance past last digit

    char *ptr = str;
    unsigned long num = value;

    do *--ptr = (num - ((num * 0xCCCDULL) >> 19) * 10) + '0'; // Reverse convert
    while ((num = (num * 0xCCCDULL) >> 19));

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

// Reverse convert unsigned long to lowercase hex string
static __forceinline char *__ultoa16l(unsigned long value, char *str)
{
    do *--str = HexTableLower[value & 0xF];
    while ((value >>= 4));

    return str;
}

// Reverse convert unsigned long to lowercase hex string
static __forceinline char *__ui64toa(unsigned _int64 value, char *str)
{
    do *--str = HexTableLower[value & 0xF];
    while ((value >>= 4));

    return str;
}

// Convert unsigned long long to lowercase memory address string
static __forceinline char *_ui64toaddr(unsigned _int64 value, char *str, BOOL is_64bit)
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
        stop = __ultoa16l(value, str);
    }

    // Zero-pad
    while (ptr < stop) *ptr++ = '0';

    return str;
}