@echo off
if exist "%~dp0build\EmbedDebug.exe" (
    cd /d "%~dp0build"
) else if exist "%~dp0build2\EmbedDebug.exe" (
    cd /d "%~dp0build2"
) else (
    echo Error: EmbedDebug.exe not found in build or build2
    pause
    exit /b 1
)
start "" "EmbedDebug.exe"
