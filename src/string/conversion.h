/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

// Optimized for non-negative long
// values in range from 0 to 99999
static
__forceinline
char *_ltoa10(long value, char *p)
{
    unsigned long long num = value;

    // Move to new position
    do ++p; while ((num = (num * 0xCCCD) >> 19));

    char *ptr = p;

    // Convert digits to characters in reverse order
    do *--ptr = (value - ((value * 0xCCCDULL) >> 19) * 10) + '0';
    while ((value = (value * 0xCCCDULL) >> 19));

    return p; // Return pointer to next position
}

static
__forceinline
char *_ultoa10(unsigned long value, char *p)
{
    unsigned long num = value;

    // Move to new position
    do ++p; while ((num /= 10));

    char *ptr = p;

    // Convert digits to characters in reverse order
    do *--ptr = (value % 10) + '0';
    while ((value /= 10));

    return p; // Return pointer to next position
}

static const char hex_table_upper[16] = "0123456789ABCDEF";

static
__forceinline
char *_ultoa16(unsigned long value, char *p)
{
    unsigned long num = value;

    // Move to new position
    do ++p; while ((num >>= 4));

    char *ptr = p;

    // Convert digits to hexadecimal characters in reverse order
    do *--ptr = hex_table_upper[value & 0xF];
    while ((value >>= 4));

    return p; // Return pointer to next position
}

static const char hex_table_lower[16] = "0123456789abcdef";

static
__forceinline
char *_ui64toaddr(unsigned long long value, char *p, BOOL bx64win)
{
    *p++ = '0';
    *p++ = 'x';

    if (bx64win)
    { // Format as 64-bit hexadecimal (16 digits)
        memset(p, '0', 16);
        p += 16; // Move to new position
    } else
    { // Format as 32-bit hexadecimal (8 digits)
        memset(p, '0', 8);
        p += 8; // Move to new position
    }

    char *ptr = p;

    // Convert digits to hexadecimal characters in reverse order
    do *--ptr = hex_table_lower[value & 0xF];
    while ((value >>= 4));

    return p; // Return pointer to next position
}