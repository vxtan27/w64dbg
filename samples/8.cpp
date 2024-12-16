/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Example
{
public:
    // Start the recursion
    void CauseException(void)
    {
        InfiniteRecursive();
    }
private:
    void InfiniteRecursive(void)
    {
        // Cause stack overflow
        InfiniteRecursive();
    }
};

int main(void)
{
    Example example;
    example.CauseException();
}