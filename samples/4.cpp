// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2024-2025 Xuan Tan. All rights reserved.

#include <iostream>
#include <cstdio>

class Example {
public:
    // Public method to trigger an exception
    void CauseException(void) {
        // Calling the private method with specific parameters
        RootException(4, 5.6f);
    }

private:
    // Static inline method that intentionally causes an access violation
    static inline void RootException(int i, double j) {
        // Print the received parameters (for debugging, if necessary)
        std::cout << "Received values: i = " << i << ", j = " << j << std::endl;

        // Cause an access violation by incorrectly using std::sscanf
        // Attempting to dereference an invalid pointer (reinterpret_cast)
        std::sscanf("12345", "%d", reinterpret_cast<int *>(i));
    }
};

int main(void) {
    // Create an instance of Example
    Example example;

    // Notify that we are about to cause an access violation
    std::cout << "Accessing invalid memory...\n";

    // Call the method that triggers the access violation
    example.CauseException();

    return 0;  // Return 0 to indicate successful execution (although it won't be reached)
}