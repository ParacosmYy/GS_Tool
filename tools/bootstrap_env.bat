@echo off
setlocal EnableExtensions

set "SCRIPT_DIR=%~dp0"
set "ROOT_DIR=%SCRIPT_DIR%.."

echo ==========================================
echo EmbedDebug - Python/PyQt bootstrap
echo Root : %ROOT_DIR%
echo ==========================================

where uv >nul 2>&1
if errorlevel 1 (
    echo [ERROR] uv was not found on PATH.
    echo [INFO] Install uv, then run this script again.
    exit /b 1
)

pushd "%ROOT_DIR%" >nul
uv sync
if errorlevel 1 (
    popd >nul
    echo [ERROR] uv sync failed.
    exit /b 1
)

uv run start-embeddebug --smoke
if errorlevel 1 (
    popd >nul
    echo [ERROR] Python/PyQt smoke failed.
    exit /b 1
)
popd >nul

echo [PASS] Python/PyQt environment is ready.
echo [PASS] Start with EmbedDebug.bat or uv run start-embeddebug.
exit /b 0
