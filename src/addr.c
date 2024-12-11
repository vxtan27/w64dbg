#include "resource.h"

static __FORCEINLINE char *__builtin_hextoa(unsigned int value, char *p)
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

static __FORCEINLINE char *__builtin_ulltoa(unsigned int value, char *p)
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

static __FORCEINLINE char *line_ultoa(unsigned int value, char *p)
{
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

static __FORCEINLINE int __builtin_wcstod(wchar_t *p)
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

static inline char *debug_ultoa(unsigned int value, char *p)
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

static __FORCEINLINE void space_ultoa(unsigned int value, char *p)
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

static inline char *ulltoaddr(unsigned long long value, char *p, char bWow64)
{
    *p++ = '0';
    *p++ = 'x';

    if (!bWow64)
    {
        memset(p, '0', 16);
        p += 16;
    } else
    {
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