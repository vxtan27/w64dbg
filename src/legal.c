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

/*
  4C 69 63 65 6E 73 65 64 20 75 6E 64 65 72 20 74  Licensed under t
  68 65 20 42 53 44 2D 33 2D 43 6C 61 75 73 65 2E  he BSD-3-Clause.
  43 6F 70 79 72 69 67 68 74 20 28 63 29 20 32 30  Copyright (c) 20
  32 34 2C 20 76 78 74 61 6E 32 37 2C 20 61 6C 6C  24, vxtan27, all
  72 69 67 68 74 73 20 72 65 73 65 72 76 65 64 2E  rights reserved.
*/