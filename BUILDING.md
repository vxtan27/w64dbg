# __Building__

## __Prerequisites__

A supported C/C++ compiler is required. Fully supported compilers include:

* MSVC C/C++ Compiler
* MSVC Clang Compiler

## __Instructions__

Open the Microsoft Visual Studio Tools Command Prompt.

Navigate to the `src` directory:

    cd src

Compile the source code:

    cl main.c /link ntdll.lib kernel32.lib msvcrt.lib ucrt.lib vcruntime.lib dbghelp.lib /ENTRY:main

* Use `/ENTRY:main` to minimize CRT (C Runtime Library) dependencies.

## __Optimizations__

Add compiler and linker options based on your requirements to improve performance or customize the build process.