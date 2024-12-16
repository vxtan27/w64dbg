/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#include "resource.h"

static __FORCEINLINE char *__builtin_hextoa(
    _In_ unsigned int value,
    _Out_writes_(8) char *p
    )
{
    unsigned int num = value;

    // Pre-compute number length
    do ++p; while ((num >>= 4));

    char *ptr = p;

    do
    {
        *(--ptr) = (value & 0xF) + '0';
        if (*ptr >= 10 + '0') *ptr += 'a' - '0' - 10;
        value >>= 4;
    } while (value);

    return p;
}

static __FORCEINLINE char *__builtin_ulltoa(
    _In_ unsigned int value,
    _Out_writes_(10) char *p
    )
{
    unsigned int num = value;

    // Pre-compute number length
    do ++p; while ((num /= 10));

    char *ptr = p;

    do
    {
        *(--ptr) = (value % 10) + '0';
        value /= 10;
    } while (value);

    return p;
}

static __FORCEINLINE int __builtin_wcstol(
    _In_z_ wchar_t *p
    )
{
    if (*p == '-')
    {
        if (*(p + 1) == '1' && *(p + 2) == '\0')
            return -1;
        else return 100000;
    }

    char c;
    int value = 0;

    while (TRUE)
    {
        if ((c = *p - '0') > 9) return 100000;
        value = value * 10 + c;
        if (!*++p) break;
    }

    return value;
}

static __FORCEINLINE int __builtin_wmemcmp(
    _In_reads_(_N) wchar_t const* _S1,
    _In_reads_(_N) wchar_t const* _S2,
    _In_           size_t         _N
    )
{
    for (; 0 < _N; ++_S1, ++_S2, --_N)
        if (*_S1 != *_S2)
            return *_S1 < *_S2 ? -1 : 1;

    return 0;
}

static __FORCEINLINE char *line_ultoa(
    _In_ unsigned int value,
    _Out_writes_(9) char *p
    )
{ // Undefine behavior for more than 99 999 999 lines
    memset(p, ' ', 8);
    p += 8;
    *p = '|';

    unsigned char i = 1;
    unsigned int num = value;

    // Pre-compute number length
    do ++i; while ((num /= 10));

    char *ptr = p - ((8 - --i) >> 1);

    do
    {
        *(--ptr) = (value % 10) + '0';
        value /= 10;
    } while (value);

    return p + 1;
}

static __FORCEINLINE char *debug_ultoa(
    _In_ unsigned int value,
    _Out_writes_(5) char *p
    )
{
    unsigned int num = value;

    // Pre-compute number length
    do ++p; while ((num /= 10));

    char *ptr = p;

    *(--ptr) = (value % 10) + '0';
    value /= 10;
    *(--ptr) = (value % 10) + '0';
    value /= 10;
    *(--ptr) = (value % 10) + '0';

    if (value >= 10)
    {
        value /= 10;
        *(--ptr) = (value % 10) + '0';
        if (value >= 10) *(--ptr) = value / 10 + '0';
    }

    return p;
}

static __FORCEINLINE void space_ultoa(
    _In_ unsigned int value,
    _Out_writes_(6) char *p
    )
{
    memset(p, ' ', 3);
    p += 5;

    *p = '\0';
    *(--p) = (value % 10) + '0';
    value /= 10;
    *(--p) = (value % 10) + '0';
    value /= 10;
    *(--p) = (value % 10) + '0';

    if (value >= 10)
    {
        value /= 10;
        *(--p) = (value % 10) + '0';
        if (value >= 10) *(--p) = value / 10 + '0';
    }
}

static __FORCEINLINE char *ulltoaddr(
    _In_ unsigned long long value,
    _Out_writes_(16) char *p,
    _In_ char bWow64
    )
{
    *p++ = '0';
    *p++ = 'x';

    if (!bWow64)
    { // FFFFFFFFFFFFFFFF
        memset(p, '0', 16);
        p += 16;
    } else
    { // FFFFFFFF
        memset(p, '0', 8);
        p += 8;
    }

    char *ptr = p;

    do
    {
        *(--ptr) = (value & 0xF) + '0';
        if (*ptr >= 10 + '0') *ptr += 'a' - '0' - 10;
        value >>= 4;
    } while (value);

    return p;
}