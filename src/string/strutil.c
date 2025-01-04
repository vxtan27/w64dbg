/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include "resrc.h" // Resource

// Modified for processing command-line arguments

static
__forceinline
long __builtin_wcstol(
    _In_z_ wchar_t *p
    )
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
wchar_t *__builtin_wmemchr(
    _In_reads_(_N) wchar_t const *_S,
    _In_           wchar_t        _C,
    _In_           size_t         _N
    )
{
    for (; 0 < _N; ++_S, --_N)
        if (*_S == _C)
            return (wchar_t *)_S;

    return 0;
}

static
__forceinline
char *__builtin_ultoa(
    _In_ unsigned long value,
    _Out_writes_(10) char *p
    )
{
    unsigned long num = value;

    // Pre-compute number length
    while ((num /= 10)) ++p;

    char *ptr = p;

    do *ptr-- = (value % 10) + '0';
    while ((value /= 10));

    return p + 1;
}

static const char hex_table[16] = "0123456789abcdef";

static
__forceinline
char *__builtin_hextoa(
    _In_ unsigned int value,
    _Out_writes_(8) char *p
    )
{
    unsigned int num = value;

    // Pre-compute number length
    while ((num >>= 4)) ++p;

    char *ptr = p;

    do *ptr-- = hex_table[value & 0xF];
    while ((value >>= 4));

    return p + 1;
}

static
__forceinline
char *ulltoaddr(
    _In_ unsigned long long value,
    _Out_writes_(16) char *p,
    _In_ DWORD bx64win
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

    char *ptr = p;

    do *ptr-- = hex_table[value & 0xF];
    while ((value >>= 4));

    return p + 1;
}