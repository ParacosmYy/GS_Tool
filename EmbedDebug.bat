@echo off
setlocal EnableExtensions EnableDelayedExpansion

title EmbedDebug Launcher

set "ROOT_DIR=%~dp0"
set "BUILD_DIR=%ROOT_DIR%build"
set "EXE_PATH=%BUILD_DIR%\EmbedDebug.exe"
set "ENV_FILE=%ROOT_DIR%local_env.bat"

set "QT_PREFIX="
set "MINGW_BIN="

if exist "%ENV_FILE%" call "%ENV_FILE%"

call :banner
call :ensure_environment
if errorlevel 1 goto :error_exit

if not exist "%BUILD_DIR%" (
    echo [INFO] create build directory: %BUILD_DIR%
    mkdir "%BUILD_DIR%"
)

if not exist "%EXE_PATH%" (
    echo [INFO] EmbedDebug.exe not found, start configure & build.
    cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_PREFIX_PATH="%QT_PREFIX%"
    if errorlevel 1 (
        echo [ERROR] cmake configure failed.
        echo [HINT] check QT_PREFIX=%QT_PREFIX%
        goto :error_exit
    )

    cmake --build "%BUILD_DIR%" --config Release
    if errorlevel 1 (
        echo [ERROR] build failed, fix compile errors first.
        goto :error_exit
    )
)

if not exist "%EXE_PATH%" (
    echo [ERROR] build done but EmbedDebug.exe still missing.
    goto :error_exit
)

if not exist "%BUILD_DIR%\Qt6Core.dll" (
    if exist "%QT_PREFIX%\bin\windeployqt.exe" (
        echo [INFO] run windeployqt...
        "%QT_PREFIX%\bin\windeployqt.exe" "%EXE_PATH%"
        if errorlevel 1 echo [WARN] windeployqt failed, runtime might be missing.
    ) else (
        echo [WARN] windeployqt not found, continue to start directly.
    )
)

pushd "%BUILD_DIR%"
if exist "EmbedDebug.exe" (
    echo [INFO] start EmbedDebug ...
    start "" "EmbedDebug.exe"
    if errorlevel 1 (
        echo [ERROR] launch failed.
        popd
        goto :error_exit
    )
    echo [OK] launch request sent.
) else (
    echo [ERROR] executable vanished: %EXE_PATH%
    popd
    goto :error_exit
)
popd

echo [OK] done.
exit /b 0

:ensure_environment
if defined QT_PREFIX (
    if not exist "%QT_PREFIX%" set "QT_PREFIX="
)
if not defined QT_PREFIX (
    if exist "E:\Tool\DevEnv\Qt\6.8.3\mingw_64\bin\windeployqt.exe" set "QT_PREFIX=E:\Tool\DevEnv\Qt\6.8.3\mingw_64"
)
if not defined QT_PREFIX (
    where windeployqt >nul 2>&1
    if not errorlevel 1 (
        for /f "delims=" %%F in ('where windeployqt 2^>nul') do (
            set "QT_BIN_DIR=%%~dpF"
            for /f "delims=" %%Q in ("!QT_BIN_DIR!..\") do set "QT_PREFIX=%%~fQ"
            if exist "!QT_PREFIX!\bin\windeployqt.exe" goto :qt_ready
        )
    )
)

:qt_ready
if not exist "%QT_PREFIX%\bin\windeployqt.exe" (
    echo [ERROR] windeployqt not found. Set QT_PREFIX to your Qt root.
    echo [HINT] example: set QT_PREFIX=E:\Tool\DevEnv\Qt\6.8.3\mingw_64
    exit /b 1
)

if not defined MINGW_BIN (
    if exist "E:\Tool\DevEnv\x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2\mingw64\bin\gcc.exe" set "MINGW_BIN=E:\Tool\DevEnv\x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2\mingw64\bin"
)
if not defined MINGW_BIN (
    where gcc >nul 2>&1
    if not errorlevel 1 (
        for /f "delims=" %%F in ('where gcc 2^>nul') do (
            set "MINGW_BIN=%%~dpF"
            set "MINGW_BIN=!MINGW_BIN:~0,-1!"
            goto :mingw_found
        )
    )
)
:mingw_found
if not exist "%MINGW_BIN%\gcc.exe" (
    echo [ERROR] gcc not found. Install MinGW and set MINGW_BIN.
    exit /b 1
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake not found in PATH.
    exit /b 1
)
where ninja >nul 2>&1
if errorlevel 1 (
    echo [ERROR] ninja not found in PATH.
    exit /b 1
)

set "PATH=%MINGW_BIN%;%PATH%"
exit /b 0

:banner
echo -------------------------------------------------
echo EmbedDebug Launcher

echo Root : %ROOT_DIR%
echo -------------------------------------------------

goto :eof

:error_exit
pause
exit /b 1
