// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#include <iostream>

class Example {
public:
    // Public method that triggers an exception
    void CauseException(void) {
        RootException();  // Call the private method to cause an exception
    }

private:
    // Static inline method that handles the exception
    static inline void RootException(void) {
        // Cause a C++ exception and handle it using try-catch
        try {
            // Throwing a runtime error to simulate an exception
            throw std::runtime_error("C++ EH Exception triggered");
        } catch (const std::exception &e) {
            // Catch the exception and output the error message
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    }
};

int main(void) {
    // Create an instance of Example
    Example example;

    // Notify that we're about to throw an exception
    std::cout << "Throwing a C++ exception...\n";

    // Call the method to cause an exception
    example.CauseException();

    return 0;  // Return 0 to indicate successful completion
}