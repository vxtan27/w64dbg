/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#include <iostream>

class Example
{
public:
    void CauseException(void)
    {
        RootException();
    }
private:
    static inline void RootException(void)
    {
        // Cause C++ exception handling exception
        try {
            throw std::runtime_error("C++ EH Exception triggered");
        } catch (const std::exception &e) {
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    }
};

int main(void) {
    Example example;
    std::cout << "Throwing a C++ exception...\n";
    example.CauseException();
}