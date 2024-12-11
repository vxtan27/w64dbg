@echo off
@fltmc > nul || (echo CreateObject^("Shell.Application"^).ShellExecute "conhost.exe", "%~s0", , "runas", 1 >> "%tmp%\GetAdmin.vbs" && "%tmp%\GetAdmin.vbs" && del "%tmp%\GetAdmin.vbs" & exit /B)
del /f %temp%\w64dbg >nul 2>&1
reg delete "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v _NO_DEBUG_HEAP /f >nul 2>&1
timeout -1