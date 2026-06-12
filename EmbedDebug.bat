@echo off
setlocal EnableExtensions

title EmbedDebug Launcher
set "ROOT_DIR=%~dp0"
set "LAUNCHER=%ROOT_DIR%tools\launch_embeddebug.ps1"

if not exist "%LAUNCHER%" (
    echo [ERROR] Missing launcher: %LAUNCHER%
    echo [HINT] Please keep tools\launch_embeddebug.ps1 in the repository.
    pause
    exit /b 1
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%LAUNCHER%" -RootDir "%ROOT_DIR%."
set "EXIT_CODE=%ERRORLEVEL%"

if not "%EXIT_CODE%"=="0" (
    echo.
    echo [FAIL] EmbedDebug launcher stopped with exit code %EXIT_CODE%.
    echo [HINT] Read the messages above before closing this window.
    pause
    exit /b %EXIT_CODE%
)

exit /b 0
