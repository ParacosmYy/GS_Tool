@echo off
rem Author: AI Token Tracker Engineering Team | Maintainer: Project Owner
rem Purpose: Stable Windows wrapper for the read-only release doctor.
rem Module: Release governance / Windows command boundary

setlocal EnableExtensions
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0release-doctor.ps1" %*
exit /b %errorlevel%
