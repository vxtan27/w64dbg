/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

static
__forceinline
char* _ltoa10(long value, char* p)
{
    long num = value;

    // Determine the number of digits
    while ((num /= 10)) ++p;

    char* ptr = p;

    // Convert digits to characters in reverse order
    do *ptr-- = (value % 10) + '0';
    while ((value /= 10));

    return p + 1; // Return pointer to next position
}

static
__forceinline
char* _ultoa10(unsigned long value, char* p)
{
    unsigned long num = value;

    // Determine the number of digits
    while ((num /= 10)) ++p;

    char* ptr = p;

    // Convert digits to characters in reverse order
    do *ptr-- = (value % 10) + '0';
    while ((value /= 10));

    return p + 1; // Return pointer to next position
}

static const char hex_table[16] = "0123456789abcdef";

static
__forceinline
char* _ultoa16(unsigned long value, char* p)
{
    unsigned long num = value;

    // Determine the number of hexadecimal digits.
    while ((num >>= 4)) ++p;

    char* ptr = p;

    // Convert digits to hexadecimal characters in reverse order
    do *ptr-- = hex_table[value & 0xF];
    while ((value >>= 4));

    return p + 1; // Return pointer to next position
}

static
__forceinline
char* _ui64toa16(unsigned long long value, char* p, unsigned long bx64win)
{
    *p++ = '0';
    *p++ = 'x';

    if (bx64win)
    { // Format as 64-bit hexadecimal (16 digits)
        memset(p, '0', 16);
        p += 15; // Move to the last position
    } else
    { // Format as 32-bit hexadecimal (8 digits)
        memset(p, '0', 8);
        p += 7; // Move to the last position
    }

    char* ptr = p;

    // Convert digits to hexadecimal characters in reverse order
    do *ptr-- = hex_table[value & 0xF];
    while ((value >>= 4));

    return p + 1; // Return pointer to next position
}