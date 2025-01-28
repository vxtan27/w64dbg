_Post_writable_byte_size_(sizeof(wchar_t))
static
__forceinline
wchar_t *
__builtin__wmemchr(
    _In_reads_(n) const wchar_t *_S,
    wchar_t _C,
    size_t _N
    )
{
    // Unroll the loop for performance optimization.
    while (_N >= 4)
    {
        if (_S[0] == _C)
            return (wchar_t *)_S;
        if (_S[1] == _C)
            return (wchar_t *)&_S[1];
        if (_S[2] == _C)
            return (wchar_t *)&_S[2];
        if (_S[3] == _C)
            return (wchar_t *)&_S[3];

        _S += 4;
        _N -= 4;
    }

    // Process the remaining characters.
    while (_N)
    {
        if (*_S == _C)
            break;

        ++_S;
        --_N;
    }

    // Character not found.
    return (wchar_t *)_S;
}

_Success_(return != NULL)
_Ret_maybenull_
_Post_writable_byte_size_(sizeof(wchar_t))
static
__forceinline
wchar_t *
__builtin_wmemchr(
    _In_reads_(n) const wchar_t *_S,
    wchar_t _C,
    size_t _N
    )
{
    // Unroll the loop for performance optimization.
    while (_N >= 4)
    {
        if (_S[0] == _C)
            return (wchar_t *)_S;
        if (_S[1] == _C)
            return (wchar_t *)&_S[1];
        if (_S[2] == _C)
            return (wchar_t *)&_S[2];
        if (_S[3] == _C)
            return (wchar_t *)&_S[3];

        _S += 4;
        _N -= 4;
    }

    // Process the remaining characters.
    while (_N)
    {
        if (*_S == _C)
            return (wchar_t *)_S;

        ++_S;
        --_N;
    }

    // Character not found.
    return NULL;
}