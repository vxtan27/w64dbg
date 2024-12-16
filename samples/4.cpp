/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#include <iostream>

class Example
{
public:
    void CauseException(void)
    {
        RootException(4, 5.6f);
    }
private:
    static inline void RootException(int i, double j)
    {
        // Cause access violation
        std::sscanf("12345", "%d", reinterpret_cast<int *>(i));
    }
};

int main(void) {
    Example example;
    std::cout << "Accessing invalid memory...\n";
    example.CauseException();
}