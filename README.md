# [__w64dbg__](#w64dbg)

## __About__

w64dbg is a small and efficient open-source binary tool for debugging live user-mode instructions on x64 Windows. It can read executable's debug information in [__PDB__](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md) and [__DWARF__](https://dwarfstd.org) format generated using MinGW.

## __Download__

* [__Releases__](https://github.com/vxtan27/w64dbg/releases/latest)

* [__Git repository__](https://github.com/vxtan27/w64dbg)

## __How to use__

### __Install__

1. Download the compiled tool from [__Releases__](https://github.com/vxtan27/w64dbg/releases/latest) corresponding to your **CPU** architecture.

* AMD **`\*-amd.tar.gz`**
* ARM **`\*-arm.tar.gz`**
* INTEL **`\*-intel.tar.gz`**
* Others **`\*.tar.gz`**

2. [__Untar__](https://monsterhost.com/how-to-extract-or-untar-tar-gz-files/#How_to_untar_targz_archives_in_their_entirety) and run **`setup.bat`** to check for compatibility.

3. [__Set system environment variables__](https://www3.ntu.edu.sg/home/ehchua/programming/howto/Environment_Variables.html).

### __Uninstall__

Run **`cleanup.bat`**.

## __Usage__

    
    Usage: w64dbg [...] executable [...]
    
    Description:
    This tool is used to debug an executable on x64 Windows.
    
    Parameter List:
    /B Ignore breakpoints.
    /D Load the executable's PDB debug symbols.
    /G Load the executable's DWARF debug symbols.
    /Q Do not display verbose exception information.
    /O Do not display OutputDebugString string.
    /S Start the executable in a new console.
    /T Wait for the specified period (in seconds).
    /V Display verbose debug information.
    

## __Samples__

<details><summary><b>MSVC Compiler</b></summary>

    
    **********************************************************************
    ** Visual Studio 2022 Developer Command Prompt v17.11.5
    ** Copyright (c) 2022 Microsoft Corporation
    **********************************************************************
    [vcvarsall.bat] Environment initialized for: 'x64'
    
    C:\w64dbg>cl /Zi sample.cpp
    Microsoft (R) C/C++ Optimizing Compiler Version 19.41.34123 for x64
    Copyright (C) Microsoft Corporation.  All rights reserved.
    
    sample.cpp
    Microsoft (R) Incremental Linker Version 14.41.34123.0
    Copyright (C) Microsoft Corporation.  All rights reserved.
    
    /out:sample.exe
    /debug
    sample.obj
    
    C:\w64dbg>w64dbg /D /V sample.exe
    CreateProcess 9572x6656
    LoadDll \\?\C:\Windows\System32\ntdll.dll
    LoadDll \\?\C:\Windows\System32\kernel32.dll
    LoadDll \\?\C:\Windows\System32\KernelBase.dll
    Thread #01 caused access violation writing to location 0x0000000000000001
    The thread tried to read from or write to a virtual address for which it does not have the appropriate access
    #0  0x00007ff6109d21a1 in sample!__crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::write_integer () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1573
    minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h: No such file or directory.
    #1  0x00007ff6109cf0f5 in sample!__crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process_integer_specifier () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1523
    minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h: No such file or directory.
    #2  0x00007ff6109cea37 in sample!__crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process_conversion_specifier () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1226
    minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h: No such file or directory.
    #3  0x00007ff6109cf68f in sample!__crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process_state () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1144
    minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h: No such file or directory.
    #4  0x00007ff6109ce47d in sample!__crt_stdio_input::input_processor<char,__crt_stdio_input::string_input_adapter<char> >::process () at minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h:1100
    minkernel\crts\ucrt\inc\corecrt_internal_stdio_input.h: No such file or directory.
    #5  0x00007ff6109d25da in sample!__stdio_common_vsscanf () at minkernel\crts\ucrt\src\appcrt\stdio\input.cpp:122
    minkernel\crts\ucrt\src\appcrt\stdio\input.cpp: No such file or directory.
    #6  0x00007ff6109a850a in sample!_vsscanf_l () at C:\Program Files (x86)\Windows Kits\10\include\10.0.26100.0\ucrt\stdio.h:2156
     2156  |    }
    #7  0x00007ff6109a8569 in sample!sscanf () at C:\Program Files (x86)\Windows Kits\10\include\10.0.26100.0\ucrt\stdio.h:2251
     2251  |        _Result = _vsscanf_l(_Buffer, _Format, NULL, _ArgList);
    #8  0x00007ff6109a83fc in sample!Exception () at C:\w64dbg\sample.cpp:6
       6   |    sscanf("12345", "%d", (int *) 1);
    #9  0x00007ff6109a849b in sample!Example::RootException () at C:\w64dbg\sample.cpp:13
      13   |        Exception(i * 2, j, "Hello");
    #10 0x00007ff6109a845b in sample!Example::CauseException () at C:\w64dbg\sample.cpp:18
      18   |        RootException(4, 5.6f);
    #11 0x00007ff6109a841e in sample!main () at C:\w64dbg\sample.cpp:25
      25   |    example.CauseException();
    #12 0x00007ff6109a87d0 in sample!__scrt_common_main_seh () at D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl:288
    D:\a\_work\1\s\src\vctools\crt\vcstartup\src\startup\exe_common.inl: No such file or directory.
    #13 0x00007ffa5b5fdbe7 in sample!BaseThreadInitThunk () from C:\w64dbg\sample.exe
    #14 0x00007ffa5cd1fbec in sample!RtlUserThreadStart () from C:\w64dbg\sample.exe
    ExitProcess 9572x6656
    
    C:\w64dbg>
    

</details>

<details><summary><b>MinGW Compiler</b></summary>

    
    Microsoft Windows [Version 10.0.26100.2161]
    (c) Microsoft Corporation. All rights reserved.
    
    C:\w64dbg>g++ -g sample.cpp -o sample.exe
    
    C:\w64dbg>w64dbg /B /G sample.exe
    Thread #01 caused stack overflow
    The thread used up its stack
    #0  0x00007ff60a7429b3 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #1  0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #2  0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #3  0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #4  0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #5  0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #6  0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #7  0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #8  0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #9  0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #10 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #11 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #12 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #13 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #14 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #15 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #16 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #17 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #18 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #19 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #20 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #21 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #22 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #23 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #24 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #25 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #26 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #27 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #28 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #29 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #30 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #31 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #32 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #33 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #34 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #35 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #36 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #37 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #38 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #39 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #40 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #41 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #42 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #43 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #44 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #45 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #46 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #47 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #48 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #49 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #50 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #51 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #52 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #53 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #54 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #55 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #56 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #57 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #58 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #59 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #60 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #61 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #62 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #63 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #64 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #65 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #66 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #67 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #68 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #69 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #70 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #71 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #72 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #73 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #74 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #75 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #76 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #77 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #78 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #79 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #80 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #81 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #82 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #83 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #84 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #85 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #86 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #87 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #88 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #89 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #90 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #91 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #92 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #93 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #94 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #95 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #96 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #97 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
    #98 0x00007ff60a7429b8 in Example::RootException (this=0x5ffecf) at sample.cpp:8
       8   |        CauseException();
    #99 0x00007ff60a7429d8 in Example::CauseException (this=0x5ffecf) at sample.cpp:13
      13   |        RootException();
      
    C:\w64dbg>
    

</details>

See [__here__](samples) for more details.

## __Requirements__

### __Device specification__

System type: **64-bit operating system, x64-based processor**

### __Windows specifications__

Minium Windows version (ARM): **Windows 10**

Minium Windows version (x86-64): **Windows XP**

### __Dynamic link libraries__

|      DLL      |       Location       |           Package           | Minimum Windows version | Minimum DLL version |
| ------------- | -------------------- | --------------------------- | ----------------------- | ------------------- |
| ntdll.dll     | C:\Windows\System32  | Windows NT Kernel           | Windows XP              | 5.1                 |
| kernel32.dll  | C:\Windows\System32  | Windows Core API            | Windows XP              | 5.1                 |
| ucrtbase.dll  | C:\Windows\System32  | Universal C Runtime         | Windows XP              | 10.0                |
| dbghelp.dll   | C:\Windows\System32  | Debugging Tools For Windows | Windows XP              | 6.0                 |

**NOTE**: If your Windows version is missing one of these DLL files or does not meet its version requirements, download it [__here__](https://fix4dll.com) with your corresponding Windows version.

## __Limitations__

w64dbg cannot handle more than **`MAX_THREAD`** and **`MAX_DLL`** (64 and 16 defined [__here__](src/main.c#L14)) as it is abnormal for a process to overcome these limits. If you want to adjust them, change the definition in the source files and then rebuild your own. By the way, w64dbg will not print out more than 100 thread context frames as the only way to reach that unbelievable number is infinite recursive.

## __Frequently Asked Questions__

<details><summary><b>How can I get the errno value of a specific thread?</b></summary>

Currently, there is no way to get **`errno`** value as it's not a variable. It's definition is:

    
    _CRTIMP extern int *__cdecl _errno(void);
    #define errno (*_errno())
    

References to **`errno`** actually call the internal **`_errno()`** function, which returns the error value for the caller thread.

</details>

<details><summary><b>Which options should I pass to MSVC when compiling?</b></summary>

|     Option      | Macro info | MSVC-Optimized |   Debug info level   |
| --------------- | ---------- | -------------- | -------------------- |
| /DEBUG          | No         | Yes            | Standard             |
| /DEBUG:FULL     | No         | Yes            | Maximum              |
| /DEBUG:FASTLINK | No         | Yes            | Reduced              |
| /Z7             | No         | Yes            | Basic                |
| /Zi             | No         | Yes            | Standard             |
| /OPT:NOREF      | No         | No             | Enhanced symbol info |
| /OPT:NOICF      | No         | No             | Enhanced symbol info |

Read more:

[__Generate debug info__](https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info)

[__Debug Information Format__](https://learn.microsoft.com/en-us/cpp/build/reference/z7-zi-zi-debug-information-format)

[__Optimizations__](https://learn.microsoft.com/en-us/cpp/build/reference/opt-optimizations)

</details>

<details><summary><b>Which options should I pass to MinGW when compiling?</b></summary>

| Option | Macro info | GDB-Optimized | Debug info level |
| ------ | ---------- | ------------- | ---------------- |
| -g     | No         | No            | Basic            |
| -g3    | Yes        | No            | Maximum          |
| -ggdb  | No         | Yes           | Basic            |
| -ggdb3 | Yes        | Yes           | Maximum          |

See [__this__](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#index-g) for more details.

* **`-fno-omit-frame-pointer`** : Do not omit the frame pointer in functions that don’t need one

See [__this__](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fomit-frame-pointer) for more details.

</details>

<details><summary><b>Where should I put the *.pdb files?</b></summary>

**w64dbg** has default behavior as same as **DbgHelp** library. This mean you can put `*.pdb` files in the same directory of debugged executable or you can [__set system environment variables__](https://www3.ntu.edu.sg/home/ehchua/programming/howto/Environment_Variables.html) **`_NT_ALT_SYMBOL_PATH`** and **`_NT_SYMBOL_PATH`**

Read more:

[__Symbol Paths__](https://learn.microsoft.com/en-us/windows/win32/debug/symbol-paths#specifying-search-paths)

[__Control the symbol path__](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/symbol-path#control-the-symbol-path)

</details>

<details><summary><b>What are breakpoints and how do I use them effectively?</b></summary>

Breakpoints are one of the most important debugging techniques in your developer's toolbox. You set breakpoints wherever you want to pause debugger execution. On Windows you can call **`DebugBreak()`**. Don't forget to add **`/B`** options to **w64dbg**.

See [__this__](https://learn.microsoft.com/en-us/visualstudio/debugger/using-breakpoints) for more details.

</details>

## __Other links__

[__Microsoft Exception Handling__](https://learn.microsoft.com/en-us/windows/win32/debug/exception-handling)

[__Microsoft DbgHelp library__](https://learn.microsoft.com/en-us/windows/win32/debug/debug-help-library)

[__Microsoft PDB format__](https://github.com/Microsoft/microsoft-pdb/blob/master/docs/ExternalResources.md)

[__Experimental use of DbgHelp__](https://www.debuginfo.com/articles.html)

[__Other Debugging Tools for Windows__](https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/debugger-download-tools)

## __License__

w64dbg is licensed under the BSD-3-Clause license.

See [__LICENSE__](LICENSE) for more details.