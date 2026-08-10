<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Package the verified onedir EXE into a user-shareable ZIP.
Entry: package-exe release command.
Module: Delivery / Windows artifact packaging boundary

The script only reads the generated dist directory and writes ignored release
artifacts. It does not sign binaries, install services, or alter user data.
#>

[CmdletBinding()]
param(
    [ValidatePattern("^[0-9A-Za-z][0-9A-Za-z.-]{0,30}$")]
    [string]$Version = "0.1.0"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$windowsRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$distDirectory = Join-Path $windowsRoot "dist\AI-Token-Tracker"
$releaseRoot = Join-Path $windowsRoot "release"
$packageName = "AI-Token-Tracker-windows-x64-$Version"
$stagingDirectory = Join-Path $releaseRoot $packageName
$zipPath = Join-Path $releaseRoot "$packageName.zip"
$quickStart = Join-Path $PSScriptRoot "EXE-QUICKSTART.txt"
$packageVerifier = Join-Path $PSScriptRoot "verify-package.ps1"

if (-not (Test-Path -LiteralPath (Join-Path $distDirectory "AI-Token-Tracker.exe") -PathType Leaf)) {
    throw "EXE is missing. Run packaging\build.ps1 before packaging the release ZIP."
}
if (-not (Test-Path -LiteralPath $quickStart -PathType Leaf)) {
    throw "End-user quick start is missing: $quickStart"
}
if (-not (Test-Path -LiteralPath $packageVerifier -PathType Leaf)) {
    throw "Package verifier is missing: $packageVerifier"
}

New-Item -ItemType Directory -Path $releaseRoot -Force | Out-Null
if (Test-Path -LiteralPath $stagingDirectory) {
    Remove-Item -LiteralPath $stagingDirectory -Recurse -Force
}
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}

New-Item -ItemType Directory -Path $stagingDirectory -Force | Out-Null
Copy-Item -Path (Join-Path $distDirectory "*") -Destination $stagingDirectory -Recurse -Force
Copy-Item -LiteralPath $quickStart -Destination (Join-Path $stagingDirectory "README.txt") -Force
$stagedExecutable = Join-Path $stagingDirectory "AI-Token-Tracker.exe"
$manifest = [ordered]@{
    manifest_version = 1
    product = "AI Token Tracker"
    version = $Version
    platform = "windows-x64"
    executable = "AI-Token-Tracker.exe"
    executable_sha256 = (Get-FileHash -LiteralPath $stagedExecutable -Algorithm SHA256).Hash.ToUpperInvariant()
    data_directory = "%LOCALAPPDATA%\AITokenTracker"
    data_policy = "Keep user data outside the extracted package; do not delete it during upgrade or rollback."
    rollback_policy = "Stop the current EXE, retain LocalAppData, and launch the previous verified package."
}
$manifest | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $stagingDirectory "RELEASE-MANIFEST.json") -Encoding utf8
Copy-Item -LiteralPath $packageVerifier -Destination (Join-Path $stagingDirectory "VERIFY-PACKAGE.ps1") -Force
Compress-Archive -Path (Join-Path $stagingDirectory "*") -DestinationPath $zipPath -CompressionLevel Optimal
Remove-Item -LiteralPath $stagingDirectory -Recurse -Force

Write-Host "Windows package created: $zipPath"
Write-Host "The archive contains the onedir runtime, README.txt, RELEASE-MANIFEST.json and VERIFY-PACKAGE.ps1; it does not contain user data or secrets."
