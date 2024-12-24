/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma section(".copr", read, discard)
__declspec(allocate(".copr"))
const char copyright[27] = "Copyright (c) 2024, vxtan27";

#pragma comment(linker, "/INCLUDE:copyright")