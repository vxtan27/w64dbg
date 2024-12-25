# __w64dbg__

A native debugging utility for x64 Windows with PDB and DWARF format support.

## __Download__

* [__Latest release__](https://github.com/vxtan27/w64dbg/releases/latest)
* [__Source code__](./)

## __Installation__

1. Download and extract the archive.
2. Run the included `bin\PREREQUISITES.bat` script.

## __Usage__

    
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
        /T            Wait for input (seconds).
        /V<n>         Set output verbosity.
    

## __Examples__

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

See [__here__](samples) for more samples.

## __Requirements__

### __System Requirements__

System type: **64-bit**

Windows version: **7+** / **10+** (x86 debugging)

### __Dynamic link libraries__

|       DLL        |       Location       |             Description              |
| ---------------- | -------------------- | ------------------------------------ |
| ntdll.dll        | C:\Windows\System32  | Windows Native API                   |
| kernel32.dll     | C:\Windows\System32  | Windows Core API                     |
| ucrtbase.dll     | C:\Windows\System32  | Universal C Runtime                  |
| vcruntime140.dll | C:\Windows\System32  | Microsoft Visual C++ Redistributable |
| dbghelp.dll      | C:\Windows\System32  | Debugging Tools For Windows          |

Missing DLLs can be downloaded from trusted sources.

## __Limitations__

Supports up to 32 threads and 16 DLLs, meeting typical debugging needs.

## __Frequently Asked Questions__

<details>
    <summary>
        <b>
            How do I configure MSVC for debugging?
        </b>
    </summary>

Use these options with MSVC:

|     Option      | MSVC-Optimized |   Debug info level   |
| --------------- | -------------- | -------------------- |
| /DEBUG          | Yes            | Standard             |
| /DEBUG:FULL     | Yes            | Maximum              |
| /DEBUG:FASTLINK | Yes            | Reduced              |
| /Z7             | Yes            | Basic                |
| /Zi             | Yes            | Standard             |
| /OPT:NOREF      | No             | Enhanced symbol info |
| /OPT:NOICF      | No             | Enhanced symbol info |

For more information:

* [__Generate debug info__](https://learn.microsoft.com/cpp/build/reference/debug-generate-debug-info)
* [__Debug Information Format__](https://learn.microsoft.com/cpp/build/reference/z7-zi-zi-debug-information-format)
* [__Optimizations__](https://learn.microsoft.com/cpp/build/reference/opt-optimizations)

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

Add `-fno-omit-frame-pointer` to retain frame pointers.

For more information:

* [__Debugging Options__](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html)
* [__Optimize Options__](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fomit-frame-pointer)

</details>

<details>
    <summary>
        <b>
            Where should I place *.pdb files?
        </b>
    </summary>

Place `*.pdb` files alongside the executable or configure symbol paths via `_NT_ALT_SYMBOL_PATH` or `_NT_SYMBOL_PATH`

See [__this__](https://learn.microsoft.com/windows-hardware/drivers/debugger/general-environment-variables) for more information.

</details>

<details>
    <summary>
        <b>
            How do I pause the execution of my code?
        </b>
    </summary>

You can use the `__debugbreak()` or `DebugBreak()` API to pause execution at specific points.

See [__this__](https://learn.microsoft.com/visualstudio/debugger/using-breakpoints) for more information.

</details>

<details>
    <summary>
        <b>
            How can I access errno for a specific thread?
        </b>
    </summary>

`errno` is thread-local and accessed via `_errno()`, defined as:

    
    _ACRTIMP int* __cdecl _errno(void);
    #define errno (*_errno())
    

Direct access to another thread's errno is not possible.

</details>

## __Other links__

* [__PDB format__](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md)
* [__Debug Help Library__](https://learn.microsoft.com/windows/win32/debug/debug-help-library)
* [__DbgHelp experimentation__](https://debuginfo.com/articles.html)
* [__Variable Display with Windows Debug API__](https://accu.org/journals/overload/29/165/orr)
* [__Debugging Tools for Windows__](https://learn.microsoft.com/windows-hardware/drivers/debugger/debugger-download-tools)

## __Copyright__

Copyright (c) 2024, vxtan27, all rights reserved.

## __License__

w64dbg is licensed under the BSD-3-Clause.

See [__LICENSE__](LICENSE) for more details.