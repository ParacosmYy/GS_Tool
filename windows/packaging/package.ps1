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

if (-not (Test-Path -LiteralPath (Join-Path $distDirectory "AI-Token-Tracker.exe") -PathType Leaf)) {
    throw "EXE is missing. Run packaging\build.ps1 before packaging the release ZIP."
}
if (-not (Test-Path -LiteralPath $quickStart -PathType Leaf)) {
    throw "End-user quick start is missing: $quickStart"
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
Compress-Archive -Path (Join-Path $stagingDirectory "*") -DestinationPath $zipPath -CompressionLevel Optimal
Remove-Item -LiteralPath $stagingDirectory -Recurse -Force

Write-Host "Windows package created: $zipPath"
Write-Host "The archive contains the onedir runtime and README.txt; it does not contain user data or secrets."
