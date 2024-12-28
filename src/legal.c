/*
    Copyright (c) 2024, vxtan27, all rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma section(".legal", read, discard)
__declspec(allocate(".legal"))
const char license[32] = "Licensed under the BSD-3-Clause.";

#pragma section(".legal", read, discard)
__declspec(allocate(".legal"))
const char copyright[48] = "Copyright (c) 2024, vxtan27, allrights reserved.";

#pragma comment(linker, "/INCLUDE:license")
#pragma comment(linker, "/INCLUDE:copyright")