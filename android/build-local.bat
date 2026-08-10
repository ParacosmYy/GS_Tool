@echo off
rem Author: AI Token Tracker Engineering Team ^| Maintainer: Project Owner
rem Purpose: Keep Gradle distributions and dependency caches inside android/.
rem Module: Android local build entry point

setlocal EnableExtensions
cd /d "%~dp0"
set "GRADLE_USER_HOME=%~dp0.gradle\user-home"
set "ANDROID_USER_HOME=%~dp0.gradle\android-user"
set "ANDROID_SDK_ROOT=%~dp0.toolchain\android-sdk"
set "ANDROID_HOME=%ANDROID_SDK_ROOT%"
set "PROJECT_JDK=%~dp0.toolchain\jdk-17"
if exist "%PROJECT_JDK%\bin\java.exe" set "JAVA_HOME=%PROJECT_JDK%"

if exist "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" (
    powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0toolchain-doctor.ps1"
    if errorlevel 3 exit /b 3
)

if not exist "%~dp0gradlew.bat" (
    echo [AI Token Tracker] Android Gradle Wrapper is not provisioned yet.
    echo Run provision-toolchain.ps1 -GenerateGradleWrapper after the local Gradle is ready.
    exit /b 2
)

if not exist "%ANDROID_SDK_ROOT%\platforms\android-37\android.jar" (
    echo [AI Token Tracker] Project-local Android SDK platform 37 is not provisioned.
    echo Run provision-toolchain.ps1 -InstallSdkPackages after accepting the SDK licenses.
    exit /b 3
)

if not exist "%ANDROID_SDK_ROOT%\build-tools" (
    echo [AI Token Tracker] Project-local Android SDK build-tools are not provisioned.
    echo Run provision-toolchain.ps1 -InstallSdkPackages after accepting the SDK licenses.
    exit /b 3
)

call "%~dp0gradlew.bat" %*
exit /b %errorlevel%
