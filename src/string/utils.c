/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

#include <vcruntime_string.h>

// Modified for processing command-line arguments

static
__forceinline
long wtol_timeout(wchar_t* p)
{
    if (*p == '-')
    {
        if (*(p + 2) == ' ')
        {
            if (*(p + 1) == '1')
                return -1;
            if (*(p + 1) == '0')
                return 0;
        }

        return 100000;
    }

    if (*p == '+') ++p;

    unsigned char c;
    long value = 0;

    do
    {
        c = *p - '0';
        if (c > 9) return 100000;
        value = value * 10 + c;
    } while (*++p != ' ');

    return value;
}

static
__forceinline
char* __builtin_ltoa(long value, char* p)
{
    long num = value;

    // Pre-compute number Count
    while ((num /= 10)) ++p;

    char* ptr = p;

    do *ptr-- = (value % 10) + '0';
    while ((value /= 10));

    return p + 1;
}

static
__forceinline
char* __builtin_ultoa(
    unsigned long value,
    char*             p
)
{
    unsigned long num = value;

    // Pre-compute number length
    while ((num /= 10)) ++p;

    char* ptr = p;

    do *ptr-- = (value % 10) + '0';
    while ((value /= 10));

    return p + 1;
}

static const char hex_table[16] = "0123456789abcdef";

static
__forceinline
char* __builtin_hextoa(
    unsigned int value,
    char*            p
)
{
    unsigned int num = value;

    // Pre-compute number length
    while ((num >>= 4)) ++p;

    char* ptr = p;

    do *ptr-- = hex_table[value & 0xF];
    while ((value >>= 4));

    return p + 1;
}

static
__forceinline
char* ulltoaddr(
    unsigned long long value,
    char*                  p,
    unsigned long    bx64win
)
{
    *p++ = '0';
    *p++ = 'x';

    if (bx64win)
    { // FFFFFFFFFFFFFFFF
        memset(p, '0', 16);
        p += 16 - 1;
    } else
    { // FFFFFFFF
        memset(p, '0', 8);
        p += 8 - 1;
    }

    char* ptr = p;

    do *ptr-- = hex_table[value & 0xF];
    while ((value >>= 4));

    return p + 1;
}