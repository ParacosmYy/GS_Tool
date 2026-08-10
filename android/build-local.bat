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

if not exist "%~dp0gradlew.bat" (
    echo [AI Token Tracker] Android Gradle Wrapper is not provisioned yet.
    echo Ask for approval before installing JDK/Gradle or generating wrapper files.
    exit /b 2
)

if not exist "%ANDROID_SDK_ROOT%\platforms\android-37\android.jar" (
    echo [AI Token Tracker] Project-local Android SDK platform 37 is not provisioned.
    echo Ask for approval before installing Android SDK packages into android\.toolchain\.
    exit /b 3
)

if not exist "%ANDROID_SDK_ROOT%\build-tools" (
    echo [AI Token Tracker] Project-local Android SDK build-tools are not provisioned.
    echo Ask for approval before installing Android SDK packages into android\.toolchain\.
    exit /b 3
)

call "%~dp0gradlew.bat" %*
exit /b %errorlevel%
