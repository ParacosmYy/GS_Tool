@echo off
rem Author: AI Token Tracker Engineering Team | Maintainer: Project Owner
rem Purpose: Keep the local Gateway launch accessible from Explorer and shells.
setlocal EnableExtensions
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0start-gateway.ps1" %*
set "EXITCODE=%ERRORLEVEL%"
if not "%EXITCODE%"=="0" pause
exit /b %EXITCODE%
