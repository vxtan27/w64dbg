# Building

## 🔧 Prerequisites

Ensure the following dependencies are installed:

- **Git** – Required for cloning the repository and submodules.
- **A compatible C++ compiler** – Must support **C++20** and MSVC extensions while using MSVC headers.
- **Windows SDK** – Latest version recommended for best compatibility.

## 📥 Cloning the Repository

This project depends on external submodules. Clone it using:

```sh
git clone --recursive https://github.com/vxtan27/w64dbg.git
```

If you already cloned the repository without submodules, initialize and update them separately:

```sh
git submodule update --init --recursive
```

Alternatively, if you prefer manual dependency management, clone the main repository and place the required dependencies in the `external` directory.

## ⚙️ Build Instructions

1. Open the **Microsoft Visual Studio Developer Command Prompt**.
2. Navigate to the project root directory.
3. Compile using the following command:

    ```sh
    cl /I "%VSINSTALLDIR%/DIA SDK/include" /I external/phnt /I external/dragonbox/include /I external/dragonbox/source /I src/include /std:c++latest src/main.cpp /link /SUBSYSTEM:CONSOLE /ENTRY:wmain
    ```

## ⚡ Customization & Optimization

### 🚀 Performance Optimization
Enable compiler optimizations for speed:

```sh
/O2   # Optimize for speed
/Ox   # Maximum optimizations (aggressive)
```

### 🐞 Debugging Support
Use debugging flags for an improved debugging experience:

```sh
/Zi    # Generate debugging information
/DEBUG # Enable debug mode in the linker
```

### 🔄 Compatibility Adjustments
Modify subsystem settings or use a specific Windows SDK version:

```sh
/SUBSYSTEM:CONSOLE /ENTRY:wmain  # Console application entry point
```

For a complete list of compiler and linker options, refer to the [MSVC documentation](https://learn.microsoft.com/cpp/build/reference/compiler-options).