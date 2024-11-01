@echo off

for /f "tokens=4-5 delims=. " %%i in ('ver') do (
    set "VERSION=%%i.%%j"
    set "MAJOR=%%i"
    set "MINOR=%%j"
)

if "%VERSION%" == "10.0" (
    rem Windows 10
) else if "%VERSION%" == "6.3" (
    rem Windows 8.1
) else if "%VERSION%" == "6.2" (
    rem Windows 8
) else if "%VERSION%" == "6.1" (
    rem Windows 7
) else if "%VERSION%" == "6.0" (
    rem Windows Vista
) else if "%MAJOR%"=="5" (
    if "%MINOR%"=="1" (
        rem Windows XP
    ) else if "%MINOR%"=="0" (
        echo [ERROR] Your Windows version is not supported.
    )
) else if %MAJOR% LSS 5 (
    echo [ERROR] Your Windows version is not supported.
)

if "%PROCESSOR_ARCHITECTURE%"=="x86" (
    if defined PROCESSOR_ARCHITEW6432 (
        rem x64 Windows
    ) else (
        echo [ERROR] Your system type is not supported.
    )
) else (
    rem x64 Windows
)

rem get administrator privilege
@fltmc > nul || (echo CreateObject^("Shell.Application"^).ShellExecute "conhost.exe", "%~s0", , "runas", 1 >> "%tmp%\GetAdmin.vbs" && "%tmp%\GetAdmin.vbs" && del "%tmp%\GetAdmin.vbs" & @exit)

rem clean setup
del /f /q /ah %temp%\gdbinit >nul 2>&1

setx _NO_DEBUG_HEAP 1 /M

where gdb.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [WARNING] gdb.exe not found, cannot use /G options.

where ntdll.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] ntdll.dll missing. Install it here with your corresponding Windows version: https://fix4dll.com/ntdll.dll

where kernel32.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] kernel32.dll missing. Install it here with your corresponding Windows version: https://fix4dll.com/kernel32.dll

where ucrtbase.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] ucrtbase.dll missing. Install it here with your corresponding Windows version: https://fix4dll.com/ucrtbase.dll

where dbghelp.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] dbghelp.dll missing. Install it here with your corresponding Windows version: https://fix4dll.com/dbghelp.dll

timeout -1