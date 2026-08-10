<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Verify an extracted Windows EXE package before launch or rollback.
Module: Delivery / package integrity boundary

This script is read-only. It verifies the package manifest and executable
hash, and rejects user-data directories placed beside the application.
#>

[CmdletBinding()]
param(
    [string]$PackageDirectory = $PSScriptRoot,
    [string]$ExpectedVersion = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$packageRoot = (Resolve-Path -LiteralPath $PackageDirectory).Path
$manifestPath = Join-Path $packageRoot "RELEASE-MANIFEST.json"
if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
    throw "Package manifest is missing: $manifestPath"
}

try {
    $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
} catch {
    throw "Package manifest is not valid JSON: $manifestPath"
}

if ([int]$manifest.manifest_version -ne 1) {
    throw "Unsupported package manifest version: $($manifest.manifest_version)"
}
if ([string]$manifest.platform -ne "windows-x64") {
    throw "Unsupported package platform: $($manifest.platform)"
}
if (-not [string]::IsNullOrWhiteSpace($ExpectedVersion) -and [string]$manifest.version -ne $ExpectedVersion) {
    throw "Package version mismatch. Expected $ExpectedVersion, got $($manifest.version)."
}

$artifactName = [string]$manifest.executable
if ([string]::IsNullOrWhiteSpace($artifactName) -or [IO.Path]::IsPathRooted($artifactName)) {
    throw "Package executable must be a relative file name."
}
$resolvedArtifact = (Resolve-Path -LiteralPath (Join-Path $packageRoot $artifactName)).Path
$packagePrefix = $packageRoot.TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
if (-not $resolvedArtifact.StartsWith($packagePrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Package executable escapes the extracted package: $artifactName"
}

$actualHash = (Get-FileHash -LiteralPath $resolvedArtifact -Algorithm SHA256).Hash.ToUpperInvariant()
$expectedHash = [string]$manifest.executable_sha256
if ($actualHash -ne $expectedHash.ToUpperInvariant()) {
    throw "Package executable SHA-256 mismatch. Expected $expectedHash, got $actualHash."
}
if ([string]$manifest.data_directory -ne "%LOCALAPPDATA%\AITokenTracker") {
    throw "Package data boundary is not the user-scoped LocalAppData directory."
}
if (Test-Path -LiteralPath (Join-Path $packageRoot "data")) {
    throw "Package contains a data directory; user data must stay outside the package."
}

Write-Host "Package verified: version=$($manifest.version) platform=$($manifest.platform)"
Write-Host "Executable SHA-256: $actualHash"
Write-Host "User data boundary: $($manifest.data_directory)"
