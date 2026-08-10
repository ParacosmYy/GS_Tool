@echo off
rem Author: AI Token Tracker Engineering Team ^| Maintainer: Project Owner
rem Purpose: Canonical trusted-LAN entry; delegate policy to one reviewed script.
rem Security: The delegated script requires explicit SHARE confirmation before
rem binding to all local interfaces over plain HTTP.
setlocal EnableExtensions
call "%~dp0deployment\start-lan-preview.bat"
exit /b %errorlevel%
