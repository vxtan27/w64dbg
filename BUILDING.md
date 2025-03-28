# Building

## System Requirements

Architecture: 64-bit

Supported OS: Windows 10 or later

## Prerequisites

Ensure the following dependencies are installed:

- Git (required to clone the repository and submodules).
- Microsoft Visual C++ (MSVC) or a compatible C++ compiler supporting C++20.
- Windows SDK (latest recommended for best compatibility).

## Cloning the Repository

This project depends on some external submodules. Clone it with the following commands to ensure all dependencies are included:

```sh
git clone --recursive https://github.com/vxtan27/w64dbg.git
```

If you have already cloned the repository without submodules, you can initialize and update them separately:

```sh
git submodule update --init --recursive
```

Alternatively, if you prefer to manage external dependencies manually, clone the main repository and place the required ones in the `external` directory.

## Build Instructions

1. Open the Microsoft Visual Studio Developer Command Prompt.
2. Navigate to the project root directory.
3. Compile using the following command:

    ```sh
    cl /I "%VSINSTALLDIR%/DIA SDK/include" /I external/phnt /I external/dragonbox/include /I external/dragonbox/source /I src/include /std:c++20 src/main.cpp /link /SUBSYSTEM:CONSOLE /ENTRY:wmain
    ```

## Customization & Optimization

### Performance Optimization
- Enable compiler optimizations for speed:
  ```sh
  /O2   # Optimize for speed
  /Ox   # Maximum optimizations (aggressive)
  ```

### Debugging Support
- Use debugging flags for better debugging experience:
  ```sh
  /Zi    # Generate debugging information
  /DEBUG # Enable debug mode in the linker
  ```

### Compatibility Adjustments
- Modify subsystem settings or use a specific Windows SDK version:
  ```sh
  /SUBSYSTEM:CONSOLE /ENTRY:wmain  # Console application entry point
  ```

For a complete list of available compiler and linker options, refer to the [MSVC documentation](https://learn.microsoft.com/cpp/build/reference/compiler-options).