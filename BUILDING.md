# Building

## System Requirements

Architecture: 64-bit

Supported OS: Windows 10 or later

## Prerequisites

Ensure the following dependencies are installed:

- Microsoft Visual C++ (MSVC) or a compatible C/C++ compiler.

- Windows SDK (latest version recommended).

## Build Instructions

Open the Microsoft Visual Studio Developer Command Prompt.

Compile the source code using the following command:

    cl /I external /I "%VSINSTALLDIR%\DIA SDK\include" /std:c++20 src\main.cpp /link /SUBSYSTEM:CONSOLE /ENTRY:wmain

## Customization & Optimization

You can modify compiler and linker options to optimize for specific use cases:

* Performance tuning: Enable optimization flags such as /O2 or /Ox.

* Debugging support: Use /Zi for debug symbols and /DEBUG for better debugging experience.

* Compatibility adjustments: Modify subsystem settings or use specific Windows SDK versions.

Refer to the MSVC documentation for a comprehensive list of available options.