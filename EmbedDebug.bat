@echo off
setlocal EnableExtensions

title EmbedDebug Launcher
set "ROOT_DIR=%~dp0"

where uv >nul 2>nul
if errorlevel 1 (
    echo [ERROR] uv was not found on PATH.
    echo [HINT] Install uv or start from a uv-enabled terminal.
    if /I not "%~1"=="--smoke" pause
    exit /b 2
)

cd /d "%ROOT_DIR%"
echo -------------------------------------------------
echo EmbedDebug Python/PyQt Launcher
echo Root : %CD%
echo -------------------------------------------------
echo [INFO] uv run start-embeddebug %*

uv run start-embeddebug %*
set "EXIT_CODE=%ERRORLEVEL%"

if not "%EXIT_CODE%"=="0" (
    echo.
    echo [FAIL] EmbedDebug launcher stopped with exit code %EXIT_CODE%.
    echo [HINT] Read the messages above before closing this window.
    if /I not "%~1"=="--smoke" pause
    exit /b %EXIT_CODE%
)

exit /b 0
