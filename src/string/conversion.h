/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once

static __forceinline long _wtol_timeout(wchar_t* p) {
    if (*p == '-') {
        if (*(p + 2) == ' ') {
            if (*(p + 1) == '1') {
                return -1;
            }
            if (*(p + 1) == '0') {
                return 0;
            }
        }
        return 100000;
    }

    if (*p == '+') {
        ++p;
    }

    unsigned char c;
    long value = 0;

    do {
        c = *p - '0';
        if (c > 9) {
            return 100000;
        }
        value = value * 10 + c;
    } while (*++p != ' ');

    return value;
}

static __forceinline char* _ltoa10(long value, char* p) {
    long num = value;

    // Pre-compute the number of digits
    while ((num /= 10)) {
        ++p;
    }

    char* ptr = p;

    do {
        *ptr-- = (value % 10) + '0';
    } while ((value /= 10));

    return p + 1;
}

static __forceinline char* _ultoa10(unsigned long value, char* p) {
    unsigned long num = value;

    // Pre-compute the number of digits
    while ((num /= 10)) {
        ++p;
    }

    char* ptr = p;

    do {
        *ptr-- = (value % 10) + '0';
    } while ((value /= 10));

    return p + 1;
}

static const char hex_table[16] = "0123456789abcdef";

static __forceinline char* _ultoa16(unsigned long value, char* p) {
    unsigned long num = value;

    // Pre-compute the number of hexadecimal digits
    while ((num >>= 4)) {
        ++p;
    }

    char* ptr = p;

    do {
        *ptr-- = hex_table[value & 0xF];
    } while ((value >>= 4));

    return p + 1;
}

static __forceinline char* _ui64toa16(unsigned long long value, char* p, unsigned long bx64win) {
    *p++ = '0';
    *p++ = 'x';

    if (bx64win) {
        // 64-bit format: 16 hexadecimal digits
        memset(p, '0', 16);
        p += 15; // Move to the last position of the 16-digit field
    } else {
        // 32-bit format: 8 hexadecimal digits
        memset(p, '0', 8);
        p += 7; // Move to the last position of the 8-digit field
    }

    char* ptr = p;

    do {
        *ptr-- = hex_table[value & 0xF];
    } while ((value >>= 4));

    return p + 1;
}