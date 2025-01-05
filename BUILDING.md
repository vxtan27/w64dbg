# Building

## Prerequisites

A compatible C/C++ compiler is required:

- MSVC C/C++ Compiler
- MSVC Clang Compiler

## Instructions

Launch the Microsoft Visual Studio Tools Command Prompt.

Change to the `src` directory:

    cd src

Compile the source code:

    cl /std:clatest main.c /link ntdll.lib kernel32.lib ucrt.lib vcruntime.lib dbghelp.lib /ENTRY:main

## Optimizations

Customize the build process by adding compiler and linker options as needed to enhance performance or meet specific requirements.