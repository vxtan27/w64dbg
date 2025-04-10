# Building

## 🛠️ Prerequisites

Ensure the following dependencies are installed and correctly configured in your environment:

- **Git**: Version control system; essential for repository cloning and submodule management.
- **C++ Compiler**: C++20 compliant, with support for MSVC extensions when using MSVC headers.
- **Windows SDK**: Latest version recommended for optimal compatibility and access to necessary headers and libraries.

## 📥 Repository Setup

Clone the repository, including all necessary submodules:

```sh
git clone --recursive [https://github.com/vxtan27/w64dbg.git](https://github.com/vxtan27/w64dbg.git)
```

## ⚙️ Compilation

Follow these steps to build the project:

1. Launch the **Microsoft Visual Studio Developer Command Prompt**.
2. Navigate to the root directory of the cloned project.
4. Generate the import library for `KernelBase.dll`:

    ```sh
    lib /DEF:lib/KernelBase.def /MACHINE:X64 /OUT:lib/KernelBase.lib
    ```

4. Compile the project using the following command:

    ```sh
    cl /I "%VSINSTALLDIR%/DIA SDK/include" /I external/phnt /I external/dragonbox/include /I external/dragonbox/source /I src/include /std:c++latest src/main.cpp /link /LIBPATH:lib /ENTRY:wmain
    ```

## ⚡ Build Configuration

Enhance performance by enabling compiler optimizations:

```sh
/O2 # Optimize for speed
/Ox # Aggressive optimizations
```

Refer to the official [MSVC Compiler Options](https://learn.microsoft.com/cpp/build/reference/compiler-options) and [MSVC Linker Options](https://learn.microsoft.com/cpp/build/reference/linker-options) documentation for a comprehensive list of available flags and their descriptions.