# __Requirements__

## __Device specification__

System type: **64-bit operating system, x64-based processor**

## __Windows specifications__

ARM (Minimum Version): **Windows 10**

x86-64 (Minimum Version):
* **Windows XP with SP2**
* **Windows 10** (debug x86 executable)

## __Dynamic link libraries__

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