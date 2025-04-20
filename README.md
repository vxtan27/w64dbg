# w64dbg

A native x64 Windows debugging utility.

## 📥 Download

- [Latest Release](https://github.com/vxtan27/w64dbg/releases/latest)
- [Source Code](./)

## 🚀 Usage

```sh
Usage: W64DBG [options] <executable> [exec-args]

Description:
    A native debugging utility for x64 Windows.

Entries:
    options       Options control behavior.
    executable    Target executable file.
    exec-args     Target executable arguments.

Options:
    /B            Ignore breakpoints.
    /O            Suppress OutputDebugString.
    /T            Wait for a key press to exit.
    /V{0|1|2}     Set output verbosity.
    /?            Display this help message.
```

## 🔧 System Requirements

Architecture: x64

Operating System: Windows 10, version 1607 or later

## ⚠️ Limitations

- Supports debugging executables with a maximum of **32 loaded DLLs**. This covers the majority of typical debugging scenarios.

## ❓ Frequently Asked Questions

<details>
<summary><b>How do I configure MSVC for debugging?</b></summary>

Recommended MSVC compiler and linker options for debugging:

| Option          | Optimized | Debug Info Level     |
| --------------  | --------- | -------------------- |
| /DEBUG          | Yes       | Standard             |
| /DEBUG:FULL     | Yes       | Maximum              |
| /DEBUG:FASTLINK | Yes       | Reduced              |
| /Z7             | Yes       | Basic                |
| /Zi             | Yes       | Standard             |
| /OPT:NOREF      | No        | Enhanced symbols     |
| /OPT:NOICF      | No        | Enhanced symbols     |

**References**:

- [Generate Debug Info](https://learn.microsoft.com/cpp/build/reference/debug-generate-debug-info)
- [Debug Information Format](https://learn.microsoft.com/cpp/build/reference/z7-zi-zi-debug-information-format)
- [MSVC Optimizations](https://learn.microsoft.com/cpp/build/reference/opt-optimizations)

</details>

<details>
<summary><b>Where should I place *.pdb files?</b></summary>

Place `*.pdb`files alongside the target executable or configure symbol paths using environment variables:

```sh
set /M _NT_ALT_SYMBOL_PATH="%temp%/SymbolCache"
set /M _NT_SYMBOL_PATH=SRV*"%temp%/SymbolCache"*https://msdl.microsoft.com/download/symbols
```

**Reference**: [General Environment Variables](https://learn.microsoft.com/windows-hardware/drivers/debugger/general-environment-variables).

</details>

<details>
<summary><b>How do I pause execution inside my code?</b></summary>

Use the following intrinsic or Windows API function to trigger a debugger break:

```cpp
__debugbreak(); // MSVC intrinsic for breakpoint
DebugBreak();   // Windows API breakpoint
```

**Reference**: [Using Breakpoints](https://learn.microsoft.com/visualstudio/debugger/using-breakpoints)

</details>

<details>
<summary><b>How can I access errno for a specific thread?</b></summary>

`errno` is thread-local. Accesse its value using the provided macro:

```cpp
_ACRTIMP int* __cdecl _errno(void);
#define errno (*_errno())
```

Direct access to another thread's `errno` is not supported due to its thread-local nature.

</details>

## 📚 Further Reading

- [PDB File Format](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md)
- [Debug Help Library (DbgHelp)](https://learn.microsoft.com/windows/win32/debug/debug-help-library)
- [DbgHelp Experimentation and Articles](https://debuginfo.com/articles.html)
- [Displaying Variables with Windows Debugging API](https://accu.org/journals/overload/29/165/orr)
- [Debugging Tools for Windows (WinDbg)](https://learn.microsoft.com/windows-hardware/drivers/debugger/debugger-download-tools)

## 📜 License

This project is licensed under the **BSD-3-Clause**.

See [LICENSE](LICENSE) for the full license text.

## ©️ Copyright

Copyright (c) 2024-2025 Xuan Tan. All rights reserved.