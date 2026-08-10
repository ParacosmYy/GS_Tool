@echo off
rem Author: AI Token Tracker Engineering Team | Maintainer: Project Owner
rem Purpose: Explicitly launch a trusted-LAN preview without changing the personal launcher.
rem Module: Deployment / local network preview boundary

setlocal EnableExtensions
cd /d "%~dp0.."
set "PYTHON=%~dp0..\.venv\Scripts\python.exe"

if not exist "%PYTHON%" (
    echo [AI Token Tracker] Local Python environment not found: windows\.venv
    echo Run windows\start.bat once before starting a LAN preview.
    exit /b 1
)

echo.
echo WARNING: this preview listens on every network interface over plain HTTP.
echo Use only on a trusted LAN. It is not a public or HTTPS deployment.
echo Configure TOKEN_TRACKER_SECRET_KEY in windows\.env before sharing.
echo.
set /p "CONFIRM=Type SHARE to continue, or press Enter to cancel: "
if /I not "%CONFIRM%"=="SHARE" (
    echo LAN preview cancelled.
    exit /b 0
)

set "TOKEN_TRACKER_HOST=0.0.0.0"
set "TOKEN_TRACKER_PORT=5000"
"%PYTHON%" -m token_tracker serve --host 0.0.0.0 --port 5000 --lan-preview
exit /b %errorlevel%
