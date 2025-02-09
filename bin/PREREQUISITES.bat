::
::  Copyright (c) 2024, vxtan27, all rights reserved.
::  Licensed under the BSD-3-Clause.
::

@echo off

:: Check Windows type
if "%PROCESSOR_ARCHITECTURE%"=="x86" if not defined PROCESSOR_ARCHITEW6432 (
    echo [ERROR] Unsupported architecture.
    timeout -1 & exit /B
)

:: Check Windows version
for /f "tokens=4-5 delims=. " %%i in ('ver') do if %%i LSS 5 (
    echo [ERROR] Unsupported Windows.
    timeout -1 & exit /B
)

:: Check DLLs
for %%f in (ucrtbase.dll vcruntime140.dll dbghelp.dll) do (
    where %%f || (
        echo [ERROR] %%f missing.
        timeout -1 & exit /B
    )
)

timeout -1