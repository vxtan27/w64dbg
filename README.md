# w64dbg

A native debugging utility for x64 Windows with PDB and DWARF format support.

## Download

- [Latest release](https://github.com/vxtan27/w64dbg/releases/latest)
- [Source code](./)

## Installation

1. Download and extract the archive.
2. Run `bin\PREREQUISITES.bat` script.

## Usage

    
    Usage: W64DBG [options] <executable> [exec-args]
    
    Description:
        A native debugging utility for x64 Windows.
        
    Entries:
        options       Options control behavior.
        executable    Target executable file.
        exec-args     Target executable arguments.
        
    Options:
        /B<n>         Set breakpoints ignorance.
        /D            Load PDB debug symbols.
        /G[+]         Load DWARF debug symbols.
        /O            Suppress OutputDebugString.
        /S            Open in a new console window.
        /T<n>          Wait for input (seconds).
        /V<n>         Set output verbosity.
        

## Examples

<details>
    <summary>
        <b>
            MSVC Compiler
        </b>
    </summary>
    <br>
    <img src="./samples/4.png"/>
</details>

<details>
    <summary>
        <b>
            MinGW Compiler
        </b>
    </summary>
    <br>
    <img src="./samples/8.png"/>
</details>

Refer to [samples](samples) for additional examples.

## Requirements

### System Requirements

System type: **64-bit**

Windows version: **10+**

### Dynamic link libraries

| DLL              | Location            | Description                          |
| ---------------- | ------------------- | ------------------------------------ |
| ntdll.dll        | C:\Windows\System32 | Windows Native API                   |
| kernel32.dll     | C:\Windows\System32 | Windows Core API                     |
| ucrtbase.dll     | C:\Windows\System32 | Universal C Runtime                  |
| vcruntime140.dll | C:\Windows\System32 | Microsoft Visual C++ Redistributable |
| dbghelp.dll      | C:\Windows\System32 | Debugging Tools For Windows          |

Missing DLLs can be downloaded from trusted sources.

## Limitations

Supports a maximum of 32 threads and 32 DLLs, sufficient for most debugging scenarios.

## Frequently Asked Questions

<details>
    <summary>
        <b>
            How do I configure MSVC for debugging?
        </b>
    </summary>

Use these options with MSVC:

| Option          | MSVC-Optimized | Debug info level     |
| --------------- | -------------- | -------------------- |
| /DEBUG          | Yes            | Standard             |
| /DEBUG:FULL     | Yes            | Maximum              |
| /DEBUG:FASTLINK | Yes            | Reduced              |
| /Z7             | Yes            | Basic                |
| /Zi             | Yes            | Standard             |
| /OPT:NOREF      | No             | Enhanced symbol info |
| /OPT:NOICF      | No             | Enhanced symbol info |

For more information:

- [Generate debug info](https://learn.microsoft.com/cpp/build/reference/debug-generate-debug-info)
- [Debug Information Format](https://learn.microsoft.com/cpp/build/reference/z7-zi-zi-debug-information-format)
- [Optimizations](https://learn.microsoft.com/cpp/build/reference/opt-optimizations)

</details>

<details>
    <summary>
        <b>
            How do I configure MinGW for debugging?
        </b>
    </summary>

Use these options with MinGW:

| Option | Macro info | GDB-Optimized | Debug info level |
| ------ | ---------- | ------------- | ---------------- |
| -g     | No         | No            | Basic            |
| -g3    | Yes        | No            | Maximum          |
| -ggdb  | No         | Yes           | Basic            |
| -ggdb3 | Yes        | Yes           | Maximum          |

Include `-fno-omit-frame-pointer` to preserve frame pointers.

For more information:

- [Debugging Options](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html)
- [Optimize Options](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fomit-frame-pointer)

</details>

<details>
    <summary>
        <b>
            Where should I place *.pdb files?
        </b>
    </summary>

Place `*.pdb` files alongside the executable or configure symbol paths via `_NT_ALT_SYMBOL_PATH` or `_NT_SYMBOL_PATH`

Refer to [this documentation](https://learn.microsoft.com/windows-hardware/drivers/debugger/general-environment-variables) for additional information.

</details>

<details>
    <summary>
        <b>
            How do I pause the execution of my code?
        </b>
    </summary>

You can use the `__debugbreak()` or `DebugBreak()` API to pause execution at specific points.

Refer to [this documentation](https://learn.microsoft.com/visualstudio/debugger/using-breakpoints) for additional information.

</details>

<details>
    <summary>
        <b>
            How can I access errno value for a specific thread?
        </b>
    </summary>

`errno` is thread-local and accessed via `_errno()`:

    _ACRTIMP int* __cdecl _errno(void);
    #define errno (*_errno())

Direct access to another thread's `errno` is not possible.

</details>

## Additional Resources

- [PDB format](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md)
- [Debug Help Library](https://learn.microsoft.com/windows/win32/debug/debug-help-library)
- [DbgHelp experimentation](https://debuginfo.com/articles.html)
- [Variable Display with Windows Debug API](https://accu.org/journals/overload/29/165/orr)
- [Debugging Tools for Windows](https://learn.microsoft.com/windows-hardware/drivers/debugger/debugger-download-tools)

## License

w64dbg is licensed under the BSD-3-Clause.

See [LICENSE](LICENSE) for more details.

## Copyright

Copyright (c) 2024, vxtan27, all rights reserved.