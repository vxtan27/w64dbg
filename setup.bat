@echo off

rem Enable delayed variable expansion
setlocal enabledelayedexpansion

rem Get Windows major and minor version
for /f "tokens=4-5 delims=. " %%i in ('ver') do (set MAJOR=%%i & set MINOR=%%j)

rem Check Windows version
if %MAJOR% LSS 5 (
    rem Unknown / Unsupported version
    echo [ERROR] Your Windows version is not supported.
    timeout -1 & exit /B
) else if %MAJOR% == 5 (
    if MINOR == 1 (
        rem Windows XP & Check if Windows XP has SP2 or not
        for /f "tokens=3" %%a in ('systeminfo ^| find "Service Pack"') do (
            if not %%a == 2 (
                rem Unknown / Unsupported version
                echo [ERROR] Your Windows version is not supported.
                timeout -1 & exit /B
            )
        )
    ) else (
        rem Unknown / Unsupported version
        echo [ERROR] Your Windows version is not supported.
        timeout -1 & exit /B
    )
)

if "%PROCESSOR_ARCHITECTURE%" == "x86" (
    if defined PROCESSOR_ARCHITEW6432 (
        rem x64 Windows
    ) else (
        echo [ERROR] Your system type is not supported.
        timeout -1 & exit /B
    )
) else (
    rem x64 Windows
)

endlocal

rem get administrator privilege
@fltmc > nul || (echo CreateObject^("Shell.Application"^).ShellExecute "conhost.exe", "%~s0", , "runas", 1 >> "%tmp%\GetAdmin.vbs" && "%tmp%\GetAdmin.vbs" && del "%tmp%\GetAdmin.vbs" & exit /B)

rem https://www.codeproject.com/Tips/1032786/Visual-Studio-Slow-Debugging-and-NO-DEBUG-HEAP
setx /M _NO_DEBUG_HEAP 1

where gdb.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [WARNING] gdb.exe not found, cannot use /G options.
where msvcrt.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] msvcrt.dll missing. Install it here: https://www.dll-files.com/msvcrt.dll.html
where ucrtbase.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] ucrtbase.dll missing. Install it here: https://www.dll-files.com/ucrtbase.dll.html
where vcruntime140.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] vcruntime140.dll missing. Install it here: https://www.dll-files.com/vcruntime140.dll.html
where dbghelp.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] dbghelp.dll missing. Install it here: https://www.dll-files.com/dbghelp.dll.html

timeout -1