/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int numerator = 100;
    int denominator = 0;

    // Use div function to compute quotient and remainder
    div_t res = div(numerator, denominator);

    // Output the results (although in case of zero denominator, this code won't execute)
    printf("Quotient of %d / %d = %d\n", numerator, denominator, res.quot);
    printf("Remainder of %d / %d = %d\n", numerator, denominator, res.rem);

    return 0;  // Return 0 to indicate successful execution
}