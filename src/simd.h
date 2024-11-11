#pragma once

#if defined(__GNUC__) || defined(__clang__)
#ifdef __cplusplus
template <typename T>
static inline constexpr int ffs(T x)
{
    if (sizeof(decltype(x)) == 8) return __builtin_ffsll(x);
    else return __builtin_ffs(x);
}
template <typename T>
static inline constexpr int clz(T x)
{
    if (sizeof(decltype(x)) == 8) return __builtin_clzll(x);
    else return __builtin_clz(x);
}
template <typename T>
static inline constexpr int ctz(T x)
{
    if (sizeof(decltype(x)) == 8) return __builtin_ctzll(x);
    else return __builtin_ctz(x);
}
template <typename T>
static inline constexpr int clrsb(T x)
{
    if (sizeof(decltype(x)) == 8) return __builtin_clrsbll(x);
    else return __builtin_clrsb(x);
}
template <typename T>
static inline constexpr int popcount(T x)
{
    if (sizeof(decltype(x)) == 8) return __builtin_popcountll(x);
    else return __builtin_popcount(x);
}
template <typename T>
static inline constexpr int parity(T x)
{
    if (sizeof(decltype(x)) == 8) return __builtin_parityll(x);
    else return __builtin_parity(x);
}
#else
#define ffs(x) _Generic((x), \
    long long: __builtin_ffsll, \
    unsigned long long: __builtin_ffsll, \
    default: __builtin_ffs \
)(x)
#define clz(x) _Generic((x), \
    long long: __builtin_clzll, \
    unsigned long long: __builtin_clzll, \
    default: __builtin_clz \
)(x)
#define ctz(x) _Generic((x), \
    long long: __builtin_ctzll, \
    unsigned long long: __builtin_ctzll, \
    default: __builtin_ctz \
)(x)
#define clrsb(x) _Generic((x), \
    long long: __builtin_clrsbll, \
    unsigned long long: __builtin_clrsbll, \
    default: __builtin_clrsb \
)(x)
#define popcount(x) _Generic((x), \
    long long: __builtin_popcountll, \
    unsigned long long: __builtin_popcountll, \
    default: __builtin_popcount \
)(x)
#define parity(x) _Generic((x), \
    long long: __builtin_parityll, \
    unsigned long long: __builtin_parityll, \
    default: __builtin_parity \
)(x)
#endif
#else
#error "MSVC compiler not supported!"
#endif