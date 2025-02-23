/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Example
{
public:
    // Public method to initiate recursion
    void CauseException(void)
    {
        InfiniteRecursive();  // Start the infinite recursion
    }

private:
    // Private method that recursively calls itself
    void InfiniteRecursive(void)
    {
        // Cause a stack overflow by calling the method indefinitely
        InfiniteRecursive();
    }
};

int main(void)
{
    // Create an instance of the Example class
    Example example;

    // Trigger the recursive method that will eventually cause a stack overflow
    example.CauseException();

    return 0;  // Return 0 to indicate successful execution (although this won't be reached due to the stack overflow)
}