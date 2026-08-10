<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Report Android build prerequisites without downloading or mutating tools.
Module: Android delivery / toolchain gate
#>

[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$androidRoot = (Resolve-Path (Join-Path $PSScriptRoot ".")).Path
$failures = [System.Collections.Generic.List[string]]::new()

function Add-Check([string]$Name, [bool]$Passed, [string]$Detail) {
    if ($Passed) {
        Write-Host "[PASS] ${Name}: $Detail" -ForegroundColor Green
        return
    }
    Write-Host "[PENDING] ${Name}: $Detail" -ForegroundColor Yellow
    $failures.Add($Name)
}

function Resolve-Tool([string]$Name, [string]$HomeVariable, [string]$RelativePath) {
    $homePath = [Environment]::GetEnvironmentVariable($HomeVariable)
    if ($homePath) {
        $candidate = Join-Path $homePath $RelativePath
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return $candidate
        }
    }
    $command = Get-Command $Name -CommandType Application -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }
    return $null
}

$java = Resolve-Tool "java.exe" "JAVA_HOME" "bin\java.exe"
$javac = Resolve-Tool "javac.exe" "JAVA_HOME" "bin\javac.exe"
$jdkDetail = if ($java -and $javac) { "java/javac found" } else { "JDK 17 required; set JAVA_HOME or PATH" }
Add-Check "JDK 17" ($java -and $javac) $jdkDetail

$wrapper = Join-Path $androidRoot "gradlew.bat"
$wrapperJar = Join-Path $androidRoot "gradle\wrapper\gradle-wrapper.jar"
$wrapperProperties = Join-Path $androidRoot "gradle\wrapper\gradle-wrapper.properties"
$wrapperReady = (Test-Path -LiteralPath $wrapper -PathType Leaf) -and
    (Test-Path -LiteralPath $wrapperJar -PathType Leaf) -and
    (Test-Path -LiteralPath $wrapperProperties -PathType Leaf)
$wrapperDetail = if ($wrapperReady) { "project wrapper files are complete" } else { "approved Gradle environment must generate gradlew.bat and wrapper jar" }
Add-Check "Gradle Wrapper" $wrapperReady $wrapperDetail

$sdkRoot = Join-Path $androidRoot ".toolchain\android-sdk"
$platformJar = Join-Path $sdkRoot "platforms\android-37\android.jar"
$aapt2 = Get-ChildItem -LiteralPath (Join-Path $sdkRoot "build-tools") -Filter "aapt2.exe" -File -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
$sdkReady = (Test-Path -LiteralPath $platformJar -PathType Leaf) -and ($null -ne $aapt2)
$sdkDetail = if ($sdkReady) { "project API 37 platform and build-tools are available" } else { "install API 37 and build-tools under android\\.toolchain\\android-sdk" }
Add-Check "Android SDK API 37" $sdkReady $sdkDetail

$adb = Resolve-Tool "adb.exe" "ANDROID_SDK_ROOT" "platform-tools\adb.exe"
$adbDetail = if ($adb) { "device inspection command is available" } else { "optional: install Android Platform Tools for device integration" }
Add-Check "ADB" ($null -ne $adb) $adbDetail

if ($failures.Count -gt 0) {
    Write-Host "Android toolchain pending: $($failures -join ', ')" -ForegroundColor Yellow
    exit 3
}
Write-Host "Android toolchain ready: build-local.bat can run." -ForegroundColor Green
exit 0
