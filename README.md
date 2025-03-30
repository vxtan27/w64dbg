# w64dbg

A native Windows debugging utility for x64 systems.

## 📥 Download

- [Latest Release](https://github.com/vxtan27/w64dbg/releases/latest)
- [Source Code](./)

## 🚀 Usage

```sh
Usage: W64DBG [options] <executable> [exec-args]

Description:
    A native debugging utility for x64 Windows.

Entries:
    options       Control debugging behavior.
    executable    Target executable file.
    exec-args     Arguments passed to the target executable.

Options:
    /B            Ignore breakpoints.
    /O            Suppress OutputDebugString.
    /T            Wait for a key press before exiting.
    /V{0|1|2}     Set verbosity level.
```

## 🔧 System Requirements

Architecture: 64-bit

Supported OS: Windows 10 or later

## ⚠️ Limitations

- Supports debugging of executables with up to **32 DLLs**, covering most common debugging scenarios.

## ❓ Frequently Asked Questions

<details>
<summary><b>How do I configure MSVC for debugging?</b></summary>

Use these options when compiling with MSVC:

| Option          | Optimized | Debug Info Level     |
| -------------- | --------- | -------------------- |
| /DEBUG          | Yes       | Standard            |
| /DEBUG:FULL     | Yes       | Maximum             |
| /DEBUG:FASTLINK | Yes       | Reduced             |
| /Z7             | Yes       | Basic               |
| /Zi             | Yes       | Standard            |
| /OPT:NOREF      | No        | Enhanced symbols    |
| /OPT:NOICF      | No        | Enhanced symbols    |

For further details:

- [Generate Debug Info](https://learn.microsoft.com/cpp/build/reference/debug-generate-debug-info)
- [Debug Information Format](https://learn.microsoft.com/cpp/build/reference/z7-zi-zi-debug-information-format)
- [MSVC Optimizations](https://learn.microsoft.com/cpp/build/reference/opt-optimizations)

</details>

<details>
<summary><b>Where should I place *.pdb files?</b></summary>

Place `*.pdb` files alongside the executable or configure symbol paths using:

```sh
set _NT_ALT_SYMBOL_PATH=C:\Symbols
set _NT_SYMBOL_PATH=SRV*C:\Symbols*https://msdl.microsoft.com/download/symbols
```

Refer to [Microsoft Documentation](https://learn.microsoft.com/windows-hardware/drivers/debugger/general-environment-variables) for more details.

</details>

<details>
<summary><b>How do I pause execution inside my code?</b></summary>

Use either of the following break instructions:

```cpp
__debugbreak();  // MSVC intrinsic
DebugBreak();    // Windows API
```

More details: [Using Breakpoints](https://learn.microsoft.com/visualstudio/debugger/using-breakpoints)

</details>

<details>
<summary><b>How can I access errno for a specific thread?</b></summary>

`errno` is thread-local and should be accessed via `_errno()`, as shown:

```cpp
_ACRTIMP int* __cdecl _errno(void);
#define errno (*_errno())
```

Direct access to another thread's `errno` is not possible.

</details>

## 📚 Additional Resources

- [PDB Format](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md)
- [Debug Help Library](https://learn.microsoft.com/windows/win32/debug/debug-help-library)
- [DbgHelp Experimentation](https://debuginfo.com/articles.html)
- [Variable Display with Windows Debug API](https://accu.org/journals/overload/29/165/orr)
- [Debugging Tools for Windows](https://learn.microsoft.com/windows-hardware/drivers/debugger/debugger-download-tools)

## 📜 License

w64dbg is licensed under the **BSD-3-Clause**.

See [LICENSE](LICENSE) for more details.

## ©️ Copyright

Copyright (c) 2024-2025 Xuan Tan. All rights reserved.