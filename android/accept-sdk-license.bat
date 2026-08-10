@echo off
rem Author: AI Token Tracker Engineering Team ^| Maintainer: Project Owner
rem Purpose: Obtain explicit Android SDK license confirmation and install pinned packages.
rem Module: Android delivery / user-authorized SDK bootstrap

setlocal EnableExtensions
cd /d "%~dp0"

set "SDK_ROOT=%~dp0.toolchain\android-sdk"
set "SDK_MANAGER=%SDK_ROOT%\cmdline-tools\latest\bin\sdkmanager.bat"

if not exist "%SDK_MANAGER%" (
    echo [AI Token Tracker] Project-local sdkmanager is missing.
    echo Run provision-toolchain.ps1 -GenerateGradleWrapper first.
    exit /b 2
)

echo.
echo This action opens the official Android SDK license prompt.
echo It will not silently accept licenses or change the global Android SDK.
echo.
set "CONFIRM="
set /p "CONFIRM=Type ACCEPT to continue, or press Enter to cancel: "
if /I not "%CONFIRM%"=="ACCEPT" (
    echo [AI Token Tracker] SDK license flow cancelled by the user.
    exit /b 4
)

call "%SDK_MANAGER%" --sdk_root="%SDK_ROOT%" --licenses
if errorlevel 1 (
    echo [AI Token Tracker] SDK license command failed or was declined.
    exit /b 5
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0provision-toolchain.ps1" -InstallSdkPackages
if errorlevel 1 (
    echo [AI Token Tracker] API 37 / Build Tools installation failed.
    exit /b 6
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0toolchain-doctor.ps1"
if errorlevel 1 (
    echo [AI Token Tracker] Toolchain doctor did not pass after installation.
    exit /b 7
)

echo.
echo [AI Token Tracker] Android SDK is ready. Build with:
echo   build-local.bat assembleDebug
exit /b 0
