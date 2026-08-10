@echo off
rem Author: AI Token Tracker Engineering Team ^| Maintainer: Project Owner
rem Purpose: Windows source launcher; the repository root delegates here.
setlocal EnableExtensions
cd /d "%~dp0"

set "PYTHON=%~dp0.venv\Scripts\python.exe"
set "REQUIREMENTS=%~dp0requirements.lock"
set "PIP_CACHE_DIR=%~dp0.cache\pip"
set "PIP_DISABLE_PIP_VERSION_CHECK=1"

if not exist "%PIP_CACHE_DIR%" mkdir "%PIP_CACHE_DIR%"

if not exist "%PYTHON%" (
    echo [AI Token Tracker] Creating local Python environment...
    where py >nul 2>&1
    if errorlevel 1 (
        echo Python Launcher was not found. Install Python 3.11+ first.
        pause
        exit /b 1
    )
    py -3 -m venv "%~dp0.venv"
    if errorlevel 1 (
        echo Failed to create the virtual environment.
        pause
        exit /b 1
    )
)

if not exist "%REQUIREMENTS%" set "REQUIREMENTS=%~dp0requirements.txt"

if not exist "%~dp0.env" (
    echo [AI Token Tracker] Creating .env from .env.example...
    copy /Y "%~dp0.env.example" "%~dp0.env" >nul
)

"%PYTHON%" -c "import flask, requests, dotenv, waitress" >nul 2>&1
if errorlevel 1 (
    echo [AI Token Tracker] Installing dependencies...
    "%PYTHON%" -m pip install -r "%REQUIREMENTS%"
    if errorlevel 1 (
        echo Dependency installation failed.
        pause
        exit /b 1
    )
)

echo.
echo AI Token Tracker is starting. Close this window to stop it.
echo The default local port is 5000. If it is occupied, the current source
echo will select the next available local port and open that address.
echo.
"%PYTHON%" run.py
pause
