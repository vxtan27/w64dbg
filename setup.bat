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

rem Check Windows type
if "%PROCESSOR_ARCHITECTURE%" == "x86" (
    if defined PROCESSOR_ARCHITEW6432 (
        rem x64 Windows
    ) else (
        rem x86 Windows
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

rem Check if any DLL miss
where gdb.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [WARNING] gdb.exe not found, cannot use /G options.
where msvcrt.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] msvcrt.dll missing, download them from official and trusted sources.
where ucrtbase.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] ucrtbase.dll missing, download them from official and trusted sources.
where vcruntime140.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] vcruntime140.dll missing, download them from official and trusted sources.
where dbghelp.dll >nul 2>&1
if %ERRORLEVEL% NEQ 0 echo [ERROR] dbghelp.dll missing, download them from official and trusted sources.

timeout -1