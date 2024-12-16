/* Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License. */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int numerator = 100;
    int denominator = 0;

    // Use div function
    div_t res = div(numerator, denominator);

    printf("Quotient of 100/0 = %d\n", res.quot);
    printf("Remainder of 100/0 = %d\n", res.rem);
}