# __Requirements__

## __Device specification__

System type: **64-bit operating system, x64-based processor**

## __Windows specifications__

Minimum Windows version (ARM): **Windows 10**

Minimum Windows version (x86-64): **Windows XP with SP2** (**Windows 10** if cross debug x86 executable)

## __Dynamic link libraries__

|       DLL        |       Location       |               Package                |
| ---------------- | -------------------- | ------------------------------------ |
| kernel32.dll     | C:\Windows\System32  | Windows Core API                     |
| msvcrt.dll       | C:\Windows\System32  | Microsoft Visual C++ Runtime         |
| ucrtbase.dll     | C:\Windows\System32  | Universal C Runtime                  |
| vcruntime140.dll | C:\Windows\System32  | Microsoft Visual C++ Redistributable |
| dbghelp.dll      | C:\Windows\System32  | Debugging Tools For Windows          |

**NOTE**: If your Windows version is missing one of these DLL files or does not meet its version requirements, download it [__here__](https://www.dll-files.com) with your corresponding Windows version.

Read more: [__Deployment on Microsoft Windows XP__](https://learn.microsoft.com/en-us/cpp/windows/universal-crt-deployment#deployment-on-microsoft-windows-xp)