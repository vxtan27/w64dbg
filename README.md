# [__w64dbg__](#w64dbg)

A lightweight and efficient open-source debugging tool for live user-mode instructions on x64 Windows. It supports reading the executable's debug information in PDB and DWARF formats generated using MinGW.

## __Download__

* [__Latest Release__](https://github.com/vxtan27/w64dbg/releases/latest)
* [__Git repository__](https://github.com/vxtan27/w64dbg)

## __Getting Started__

### __Installation__

1. Download the compiled tool compatible with your **CPU** architecture.
2. Untar archive and run the **`setup.bat`** script to verify compatibility.

## __Usage__

    
    W64DBG [option...] <executable> [exec-arg...]
    
    Description:
        This tool is used to debug an executable on x64 Windows.
        
    Entries:
        Entry         Meaning
        -----         -------
        option        Options control debugging behavior.
        executable    Name of the executable.
        exec-arg      Arguments passed to the executable.
        
    Options:
        Option        Description
        ------        -----------
        /B            Ignore breakpoints.
        /D            Load the executable's PDB debug symbols.
        /G[+]         Load the executable's DWARF debug symbols.
        /O            Suppress messages from OutputDebugString.
        /S            Start the executable in a new console.
        /T            Wait for the specified period (in seconds).
        /V<n>         Display verbose debug information.
    

## __Examples__

<details>
    <summary>
        <b>MSVC Compiler</b>
    </summary>
    <br>

    
    **********************************************************************
    ** Visual Studio 2022 Developer Command Prompt v17.12.1
    ** Copyright (c) 2022 Microsoft Corporation
    **********************************************************************
    [vcvarsall.bat] Environment initialized for: 'x64'
    
    C:\w64dbg\samples>cl /EHsc /Z7 4.cpp
    Microsoft (R) C/C++ Optimizing Compiler Version 19.42.34433 for x64
    Copyright (C) Microsoft Corporation.  All rights reserved.
    
    4.cpp
    4.cpp(14): warning C4312: 'reinterpret_cast': conversion from 'int' to 'int *' of greater size
    Microsoft (R) Incremental Linker Version 14.42.34433.0
    Copyright (C) Microsoft Corporation.  All rights reserved.
    
    /out:4.exe
    /debug
    4.obj
    
    C:\w64dbg\samples>w64dbg /D /V2 4.exe
    Accessing invalid memory...
    Thread #01 caused access violation writing to location 0x0000000000000004
    The thread tried to read from or write to a virtual address for which it does not have the appropriate access
    #0  0x00007ff72f24bef9 in __crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::write_integer (value=5434512) at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1573
    #1  0x00007ff72f248e85 in __crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process_integer_specifier (base=1375552, is_signed=64) at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1523
    #2  0x00007ff72f2487c7 in __crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process_conversion_specifier () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1226
    #3  0x00007ff72f24941f in __crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process_state () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1144
    #4  0x00007ff72f24820d in __crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1100
    #5  0x00007ff72f24c316 in __stdio_common_vsscanf (options=5, buffer=0x5, buffer_count=5, format=0x5, locale=0x0, arglist=0x14fe90) at minkernel\crts\ucrt\src\appcrt\stdio\input.cpp:122
    #6  0x00007ff72f1c44ea in _vsscanf_l (_Buffer=0x7ff72f2c73d0, _Format=0x7ff72f2c73cc, _Locale=0x0, _ArgList=0x14fe90) at C:\Program Files (x86)\Windows Kits\10\include\10.0.26100.0\ucrt\stdio.h:2156
      2156  |    }
    #7  0x00007ff72f1c4549 in sscanf (_Buffer=0x7ff72f2c73d0, _Format=0x7ff72f2c73cc) at C:\Program Files (x86)\Windows Kits\10\include\10.0.26100.0\ucrt\stdio.h:2251
      2251  |        _Result = _vsscanf_l(_Buffer, _Format, NULL, _ArgList);
    #8  0x00007ff72f1c2dc9 in Example::RootException (i=4, j=5.600000) at 4.cpp:14
       14   |        std::sscanf("12345", "%d", reinterpret_cast<int *>(i));
    #9  0x00007ff72f1c2d8b in Example::CauseException () at 4.cpp:8
        8   |        RootException(4, 5.6f);
    #10 0x00007ff72f1c0c21 in main () at 4.cpp:21
       21   |    example.CauseException();
    #11 0x00007ff72f2216d0 in __scrt_common_main_seh () at D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl:288
    #12 0x00007ffc2770e8d7 in BaseThreadInitThunk () from C:\WINDOWS\System32\KERNEL32.DLL
    #13 0x00007ffc28c5fbcc in RtlUserThreadStart () from C:\WINDOWS\SYSTEM32\ntdll.dll
    
    C:\w64dbg\samples>
    

</details>

<details>
    <summary>
        <b>MinGW Compiler</b>
    </summary>
    <br>

    
    Microsoft Windows [Version 10.0.26100.2454]
    (c) Microsoft Corporation. All rights reserved.
    
    C:\w64dbg\samples>g++ -g 8.cpp -o 8.exe
    
    C:\w64dbg\samples>w64dbg /G 8.exe
    Thread #01 caused stack overflow
    The thread used up its stack
    #0  0x00007ff7cb7d29c3 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #1  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #2  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #3  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #4  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #5  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #6  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #7  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #8  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #9  0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #10 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #11 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #12 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #13 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #14 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #15 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #16 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #17 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #18 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #19 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #20 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #21 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #22 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #23 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #24 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #25 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #26 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #27 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #28 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #29 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #30 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #31 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #32 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #33 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #34 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #35 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #36 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #37 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #38 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #39 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #40 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #41 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #42 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #43 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #44 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #45 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #46 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #47 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #48 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #49 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #50 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #51 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #52 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #53 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #54 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #55 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #56 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #57 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #58 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #59 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #60 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #61 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #62 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #63 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #64 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #65 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #66 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #67 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #68 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #69 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #70 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #71 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #72 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #73 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #74 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #75 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #76 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #77 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #78 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #79 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #80 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #81 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #82 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #83 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #84 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #85 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #86 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #87 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #88 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #89 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #90 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #91 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #92 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #93 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #94 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #95 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #96 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #97 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #98 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    #99 0x00007ff7cb7d29c8 in Example::InfiniteRecursive (this=0x5ffecf) at 8.cpp:16
    
    C:\w64dbg\samples>
    

</details>

See [__here__](samples) for more details.

## __Requirements__

### __Device specification__

System type: **64-bit operating system, x64-based processor**

### __Windows specifications__

ARM (Minimum Version): **Windows 10**

x86-64 (Minimum Version):
* **Windows XP with SP2**
* **Windows 10** (debug x86 executable)

### __Dynamic link libraries__

|       DLL        |       Location       |             Description              |
| ---------------- | -------------------- | ------------------------------------ |
| ntdll.dll        | C:\Windows\System32  | Windows Native API                   |
| kernel32.dll     | C:\Windows\System32  | Windows Core API                     |
| msvcrt.dll       | C:\Windows\System32  | Microsoft Visual C++ Runtime         |
| ucrtbase.dll     | C:\Windows\System32  | Universal C Runtime                  |
| vcruntime140.dll | C:\Windows\System32  | Microsoft Visual C++ Redistributable |
| dbghelp.dll      | C:\Windows\System32  | Debugging Tools For Windows          |

**NOTE**:

* If any of these DLLs are missing, download them from official and trusted sources.
* For Windows XP, refer to [__Deployment on Microsoft Windows XP__](https://learn.microsoft.com/en-us/cpp/windows/universal-crt-deployment#deployment-on-microsoft-windows-xp)

## __Limitations__

w64dbg supports up to MAX_THREAD (64) and MAX_DLL (16), as defined [__here__](src/main.h#L21). These limits are based on typical process behaviors.

## __Frequently Asked Questions__

<details>
    <summary>
        <b>How can I get the errno value of a specific thread?</b>
    </summary>
    <br>

Currently, there is no way to get **`errno`** value as it is not a direct variable, but a function call. Its definition is:

    
    _ACRTIMP int* __cdecl _errno(void);
    #define errno (*_errno())
    

Thus, references to **`errno`** are calls to the internal **`_errno()`** function, which returns the error value for the caller thread.

</details>

<details>
    <summary>
        <b>Which options should I use when compiling with MSVC?</b>
    </summary>
    <br>

|     Option      | Macro info | MSVC-Optimized |   Debug info level   |
| --------------- | ---------- | -------------- | -------------------- |
| /DEBUG          | No         | Yes            | Standard             |
| /DEBUG:FULL     | No         | Yes            | Maximum              |
| /DEBUG:FASTLINK | No         | Yes            | Reduced              |
| /Z7             | No         | Yes            | Basic                |
| /Zi             | No         | Yes            | Standard             |
| /OPT:NOREF      | No         | No             | Enhanced symbol info |
| /OPT:NOICF      | No         | No             | Enhanced symbol info |

For more details:

* [__Generate debug info__](https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info)
* [__Debug Information Format__](https://learn.microsoft.com/en-us/cpp/build/reference/z7-zi-zi-debug-information-format)
* [__Optimizations__](https://learn.microsoft.com/en-us/cpp/build/reference/opt-optimizations)

</details>

<details>
    <summary>
        <b>Which options should I pass to MinGW when compiling?</b>
    </summary>
    <br>

| Option | Macro info | GDB-Optimized | Debug info level |
| ------ | ---------- | ------------- | ---------------- |
| -g     | No         | No            | Basic            |
| -g3    | Yes        | No            | Maximum          |
| -ggdb  | No         | Yes           | Basic            |
| -ggdb3 | Yes        | Yes           | Maximum          |

See [__this__](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#index-g) for more details.

**`-fno-omit-frame-pointer`** : Do not omit the frame pointer in functions that don’t need one

See [__this__](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fomit-frame-pointer) for more details.

</details>

<details>
    <summary>
        <b>Where should I place *.pdb files?</b>
    </summary>
    <br>

**w64dbg** behaves like the **DbgHelp** library, allowing you to:

* Place `*.pdb` files in the same directory as the debugged executable
* Configure system environment variables **`_NT_ALT_SYMBOL_PATH`** and **`_NT_SYMBOL_PATH`**

Read more:

* [__Symbol Paths__](https://learn.microsoft.com/en-us/windows/win32/debug/symbol-paths#specifying-search-paths)
* [__Control the symbol path__](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/symbol-path#control-the-symbol-path)

</details>

<details>
    <summary>
        <b>What are breakpoints, and how to use them effectively?</b>
    </summary>
    <br>

Breakpoints are a critical debugging technique, allowing you to pause execution at specific points in your code. On Windows, use the **`DebugBreak()`** API to trigger breakpoints.

See [__this__](https://learn.microsoft.com/en-us/visualstudio/debugger/using-breakpoints) for more details.

</details>

## __Other links__

* [__Microsoft Exception Handling__](https://learn.microsoft.com/en-us/windows/win32/debug/exception-handling)
* [__Microsoft DbgHelp library__](https://learn.microsoft.com/en-us/windows/win32/debug/debug-help-library)
* [__Microsoft PDB format__](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md)
* [__Experimental use of DbgHelp__](https://www.debuginfo.com/articles.html)
* [__Other Debugging Tools for Windows__](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)

## __License__

w64dbg is licensed under the BSD-3-Clause license.

See [__LICENSE__](LICENSE) for more details.