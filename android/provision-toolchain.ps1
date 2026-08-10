<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Provision the pinned Android build tools inside the project boundary.
Module: Android delivery / reproducible toolchain bootstrap
#>

[CmdletBinding()]
param(
    [switch]$GenerateGradleWrapper,
    [switch]$InstallSdkPackages
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$androidRoot = (Resolve-Path $PSScriptRoot).Path
$toolchainRoot = Join-Path $androidRoot ".toolchain"
$downloadRoot = Join-Path $toolchainRoot "downloads"
$sdkRoot = Join-Path $toolchainRoot "android-sdk"
$gradleCache = Join-Path $androidRoot ".gradle\user-home"
$androidUserHome = Join-Path $androidRoot ".gradle\android-user"

$jdkArchive = "OpenJDK17U-jdk_x64_windows_hotspot_17.0.20_8.zip"
$jdkUri = "https://github.com/adoptium/temurin17-binaries/releases/download/jdk-17.0.20%2B8/$jdkArchive"
$jdkSha256 = "418497be5cf585bdd2203d6486a565d66d3f5e992d5630d45104cb873fab8122"
$jdkRoot = Join-Path $toolchainRoot "jdk-17"

$gradleArchive = "gradle-9.5.0-bin.zip"
$gradleUri = "https://services.gradle.org/distributions/$gradleArchive"
$gradleSha256 = "553c78f50dafcd54d65b9a444649057857469edf836431389695608536d6b746"
$gradleRoot = Join-Path $toolchainRoot "gradle-9.5.0"

$cmdlineArchive = "commandlinetools-win-15859902_latest.zip"
$cmdlineUri = "https://dl.google.com/android/repository/$cmdlineArchive"
$cmdlineSha256 = "90ae805d20434428bffcb699c290860f19bb5f66a67e6b330067e3de801fb04a"
$cmdlineRoot = Join-Path $sdkRoot "cmdline-tools\latest"

function Ensure-Directory([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        New-Item -ItemType Directory -Path $Path -Force | Out-Null
    }
}

function Assert-Hash([string]$Path, [string]$Expected) {
    $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $Expected.ToLowerInvariant()) {
        throw "SHA-256 mismatch for $Path. Expected $Expected, got $actual."
    }
}

function Download-Verified([string]$Uri, [string]$Path, [string]$Sha256) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        Write-Host "[DOWNLOAD] $Uri"
        & curl.exe --fail --location --retry 3 --retry-all-errors --output $Path $Uri
        if ($LASTEXITCODE -ne 0) {
            throw "Download failed for $Uri."
        }
    }
    Assert-Hash $Path $Sha256
    Write-Host "[PASS] verified $([IO.Path]::GetFileName($Path))"
}

function Expand-PinnedArchive([string]$Archive, [string]$Destination, [string]$Marker, [string]$ExpectedTopLevel) {
    if (Test-Path -LiteralPath $Marker -PathType Leaf) {
        Write-Host "[PASS] already provisioned $Destination"
        return
    }
    $extractRoot = Join-Path $downloadRoot ("extract-" + [IO.Path]::GetFileNameWithoutExtension($Archive))
    if (Test-Path -LiteralPath $extractRoot) {
        throw "Incomplete extraction exists at $extractRoot; inspect it before retrying."
    }
    Ensure-Directory $extractRoot
    Expand-Archive -LiteralPath $Archive -DestinationPath $extractRoot
    $topLevel = Join-Path $extractRoot $ExpectedTopLevel
    if (-not (Test-Path -LiteralPath $topLevel -PathType Container)) {
        throw "Archive $Archive did not contain expected directory $ExpectedTopLevel."
    }
    if (Test-Path -LiteralPath $Destination) {
        throw "Destination already exists but marker is missing: $Destination"
    }
    Move-Item -LiteralPath $topLevel -Destination $Destination
    Remove-Item -LiteralPath $extractRoot -Recurse -Force
    if (-not (Test-Path -LiteralPath $Marker -PathType Leaf)) {
        throw "Provisioning marker is missing after extracting $Destination."
    }
}

Ensure-Directory $downloadRoot
Ensure-Directory $sdkRoot

$jdkFile = Join-Path $downloadRoot $jdkArchive
$gradleFile = Join-Path $downloadRoot $gradleArchive
$cmdlineFile = Join-Path $downloadRoot $cmdlineArchive
Download-Verified $jdkUri $jdkFile $jdkSha256
Download-Verified $gradleUri $gradleFile $gradleSha256
Download-Verified $cmdlineUri $cmdlineFile $cmdlineSha256

Expand-PinnedArchive $jdkFile $jdkRoot (Join-Path $jdkRoot "bin\javac.exe") "jdk-17.0.20+8"
Expand-PinnedArchive $gradleFile $gradleRoot (Join-Path $gradleRoot "bin\gradle.bat") "gradle-9.5.0"
Expand-PinnedArchive $cmdlineFile $cmdlineRoot (Join-Path $cmdlineRoot "bin\sdkmanager.bat") "cmdline-tools"

$env:JAVA_HOME = $jdkRoot
$env:GRADLE_USER_HOME = $gradleCache
$env:ANDROID_USER_HOME = $androidUserHome
$env:ANDROID_SDK_ROOT = $sdkRoot
$env:ANDROID_HOME = $sdkRoot

if ($GenerateGradleWrapper) {
    Write-Host "[ACTION] generating the official Gradle Wrapper"
    Push-Location $androidRoot
    try {
        & (Join-Path $gradleRoot "bin\gradle.bat") wrapper --gradle-version 9.5.0 --distribution-type bin
        if ($LASTEXITCODE -ne 0) {
            throw "Gradle Wrapper generation failed."
        }
    } finally {
        Pop-Location
    }
}

if ($InstallSdkPackages) {
    $sdkManager = Join-Path $cmdlineRoot "bin\sdkmanager.bat"
    $licenseRoot = Join-Path $sdkRoot "licenses"
    $licenseFile = Join-Path $licenseRoot "android-sdk-license"
    $hasLicenseFile = Test-Path -LiteralPath $licenseFile -PathType Leaf
    $hasLicenseContent = if ($hasLicenseFile) {
        @(Get-Content -LiteralPath $licenseFile -ErrorAction Stop |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) }).Count -gt 0
    } else {
        $false
    }
    if (-not $hasLicenseContent) {
        throw "Android SDK license is not ready. Run '$sdkManager --sdk_root=$sdkRoot --licenses' interactively, accept the required terms, then rerun with -InstallSdkPackages."
    }
    Write-Host "[ACTION] installing platform-tools, API 37 and Build Tools 37.0.0"
    & $sdkManager --sdk_root=$sdkRoot "platform-tools" "platforms;android-37" "build-tools;37.0.0"
    if ($LASTEXITCODE -ne 0) {
        throw "Android SDK package installation failed."
    }
}

Write-Host "Project-local Android toolchain paths are ready."
Write-Host "Next: powershell -NoProfile -ExecutionPolicy Bypass -File .\toolchain-doctor.ps1"
