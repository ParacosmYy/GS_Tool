<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Validate the Caddy edge boundary before an approved HTTPS launch.
Module: Deployment / edge preflight boundary

This script is read-only. It does not install Caddy, request certificates,
change firewall rules, change ACLs, or start a server.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$ConfigPath,

    [Parameter(Mandatory = $true)]
    [string]$LogsDirectory,

    [string]$CaddyPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-ExistingFile([string]$PathValue, [string]$Label) {
    $resolved = Resolve-Path -LiteralPath $PathValue -ErrorAction SilentlyContinue
    if (-not $resolved -or -not (Test-Path -LiteralPath $resolved.Path -PathType Leaf)) {
        throw "$Label must be an existing file: $PathValue"
    }
    return $resolved.Path
}

function Resolve-ExistingDirectory([string]$PathValue, [string]$Label) {
    $resolved = Resolve-Path -LiteralPath $PathValue -ErrorAction SilentlyContinue
    if (-not $resolved -or -not (Test-Path -LiteralPath $resolved.Path -PathType Container)) {
        throw "$Label must be an existing directory: $PathValue"
    }
    return $resolved.Path
}

function Assert-NoBroadWriteAcl([string]$PathValue) {
    $broadIdentities = @(
        "S-1-1-0",             # Everyone
        "S-1-5-32-545",        # BUILTIN\Users
        "Everyone",
        "BUILTIN\Users"
    )
    # Keep FullControl out of the bitmask: it contains read bits too, so
    # including it would classify ReadAndExecute as writable. FullControl
    # still matches because it contains each explicit write flag below.
    $writeRights = [System.Security.AccessControl.FileSystemRights]::WriteData -bor `
        [System.Security.AccessControl.FileSystemRights]::AppendData -bor `
        [System.Security.AccessControl.FileSystemRights]::CreateFiles -bor `
        [System.Security.AccessControl.FileSystemRights]::CreateDirectories -bor `
        [System.Security.AccessControl.FileSystemRights]::WriteAttributes -bor `
        [System.Security.AccessControl.FileSystemRights]::WriteExtendedAttributes -bor `
        [System.Security.AccessControl.FileSystemRights]::Delete -bor `
        [System.Security.AccessControl.FileSystemRights]::DeleteSubdirectoriesAndFiles -bor `
        [System.Security.AccessControl.FileSystemRights]::ChangePermissions -bor `
        [System.Security.AccessControl.FileSystemRights]::TakeOwnership
    $acl = Get-Acl -LiteralPath $PathValue
    foreach ($entry in $acl.Access) {
        $identity = $entry.IdentityReference.Value
        $rights = $entry.FileSystemRights.ToString()
        $isBroad = $broadIdentities -contains $identity
        $canWrite = (($entry.FileSystemRights -band $writeRights) -ne 0)
        if ($entry.AccessControlType -eq "Allow" -and $isBroad -and $canWrite) {
            throw "Log path has broad write permission: identity=$identity rights=$rights path=$PathValue"
        }
    }
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectCaddyPath = Join-Path $projectRoot ".cache\caddy\2.11.4\caddy.exe"
$caddyExecutable = $null
if (-not [string]::IsNullOrWhiteSpace($CaddyPath)) {
    $resolvedCaddy = Resolve-Path -LiteralPath $CaddyPath -ErrorAction SilentlyContinue
    if ($resolvedCaddy -and (Test-Path -LiteralPath $resolvedCaddy.Path -PathType Leaf)) {
        $caddyExecutable = $resolvedCaddy.Path
    }
} else {
    $caddyCommand = Get-Command caddy -CommandType Application -ErrorAction SilentlyContinue
    if ($caddyCommand) {
        $caddyExecutable = $caddyCommand.Source
    } elseif (Test-Path -LiteralPath $projectCaddyPath -PathType Leaf) {
        $caddyExecutable = $projectCaddyPath
    }
}
if (-not $caddyExecutable) {
    throw "Caddy is missing. Run deployment\provision-caddy.ps1 or install and verify Caddy on the deployment host."
}

$config = Resolve-ExistingFile $ConfigPath "Caddy configuration"
$logs = Resolve-ExistingDirectory $LogsDirectory "Caddy log directory"
Assert-NoBroadWriteAcl $logs
Get-ChildItem -LiteralPath $logs -File -Force | ForEach-Object {
    Assert-NoBroadWriteAcl $_.FullName
}

Push-Location (Split-Path -Parent $config)
try {
    # Caddy resolves relative log/certificate paths from its working directory.
    # Match start-edge.ps1 so validation cannot create files in the caller's
    # current directory or validate a different relative-path boundary.
    & $caddyExecutable validate --config $config --adapter caddyfile
    if ($LASTEXITCODE -ne 0) {
        throw "Caddyfile validation failed; HTTPS startup is rejected."
    }
}
finally {
    Pop-Location
}

Write-Host "Edge preflight passed: config=$config logs=$logs"
Write-Host "No certificate request, ACL/firewall change, or Caddy startup was performed."
