:: Copyright (c) 2024, vxtan27. Licensed under the BSD-3-Clause License.

@echo off

:: Check Windows version
for /f "tokens=4-5 delims=. " %%i in ('ver') do if %%i LSS 5 (
    echo [ERROR] Unsupported Windows version.
    timeout -1 & exit /B
)

:: Check Windows type
if "%PROCESSOR_ARCHITECTURE%"=="x86" if not defined PROCESSOR_ARCHITEW6432 (
    echo [ERROR] Unsupported system architecture.
    timeout -1 & exit /B
)

:: Check DLLs
for %%f in (msvcrt.dll ucrtbase.dll vcruntime140.dll dbghelp.dll) do (
    where %%f || (
        echo [ERROR] %%f missing.
        timeout -1 & exit /B
    )
)

timeout -1