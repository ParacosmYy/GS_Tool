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

REM 检查Qt依赖是否已部署（以Qt6Core.dll为标志）
if not exist "%cd%\Qt6Core.dll" (
    echo Deploying Qt dependencies...
    "E:\Tool\DevEnv\Qt\6.8.3\mingw_64\bin\windeployqt.exe" "%cd%\EmbedDebug.exe" >nul 2>&1
    if errorlevel 1 (
        echo Warning: windeployqt failed, app may not start correctly
    )
)

start "" "EmbedDebug.exe"
