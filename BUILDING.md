# Building

## Requirements

System type: **64-bit**

Windows version: **10+**

## Prerequisites

Ensure the following dependencies are installed:

- A C/C++ compiler compatible with Microsoft Visual C++ (MSVC).

- Windows SDK.

## Instructions

Open the Microsoft Visual Studio Developer Command Prompt.

Navigate to the source directory:

    cd src

Compile the source code:

    cl /std:c++latest main.cpp /link ntdll.lib kernel32.lib ucrt.lib vcruntime.lib dbghelp.lib /ENTRY:main

## Customization

Adjust compiler and linker options as needed to align with specific performance, compatibility, or debugging requirements.