#requires -Version 5.1
[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = "Medium")]
param(
    [Parameter(Mandatory = $true)]
    [string]$ArtifactPath,
    [Parameter(Mandatory = $true)]
    [string]$ExpectedSha256,
    [string]$InstallRoot = (Join-Path $env:LOCALAPPDATA "Programs\QuillForge"),
    [switch]$RegisterFileAssociations,
    [string[]]$FileExtensions = @(".qf")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
Import-Module (Join-Path $PSScriptRoot "QuillForge.Distribution.psm1") -Force

$root = Assert-QfInstallRoot $InstallRoot
$artifact = Get-QfArtifactInfo $ArtifactPath $ExpectedSha256
$target = Join-Path $root "QuillForge.exe"
if (Test-Path -LiteralPath (Get-QfStatePath $root) -PathType Leaf) {
    throw "QuillForge is already installed at $root; use update.ps1 instead."
}
if (Test-Path -LiteralPath $target -PathType Leaf) {
    throw "Refusing to overwrite an existing executable; use update.ps1 instead."
}

if (-not $PSCmdlet.ShouldProcess($root, "Install QuillForge $($artifact.sha256)")) {
    return
}

$temporary = Join-Path $root ".QuillForge.$PID.tmp.exe"
$associations = @()
$progIdAssociation = $null
try {
    New-Item -ItemType Directory -Path $root -Force | Out-Null
    Copy-Item -LiteralPath $artifact.path -Destination $temporary -Force
    $copied = Get-QfArtifactInfo $temporary $artifact.sha256
    Move-Item -LiteralPath $temporary -Destination $target -Force
    if ($RegisterFileAssociations) {
        foreach ($extension in $FileExtensions) {
            $association = New-QfFileAssociation $extension $target -ReuseOwnedProgId:($null -ne $progIdAssociation)
            $associations += $association
            if ($null -eq $progIdAssociation) {
                $progIdAssociation = [ordered]@{
                    prog_id = $association.prog_id
                    command = $association.command
                }
            }
        }
    }
    $state = [ordered]@{
        schema_version = 1
        installed_at_utc = [DateTime]::UtcNow.ToString("o")
        install_root = $root
        executable_path = $target
        sha256 = $copied.sha256
        bytes = $copied.bytes
        backup_path = $null
        previous_sha256 = $null
        file_associations = @($associations)
    }
    Write-QfState $root $state
    Write-Output "Installed QuillForge to $target"
    Write-Output "SHA-256: $($copied.sha256)"
} catch {
    $allAssociationsRemoved = $true
    foreach ($association in @($associations)) {
        try {
            if (-not (Remove-QfFileAssociation $association)) {
                $allAssociationsRemoved = $false
            }
        } catch {
            $allAssociationsRemoved = $false
        }
    }
    if ($allAssociationsRemoved -and $null -ne $progIdAssociation) {
        try {
            if (-not (Remove-QfProgId $progIdAssociation)) {
                $allAssociationsRemoved = $false
            }
        } catch {
            $allAssociationsRemoved = $false
        }
    }
    if (Test-Path -LiteralPath $temporary -PathType Leaf) {
        Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
    }
    if ($allAssociationsRemoved -and (Test-Path -LiteralPath $target -PathType Leaf)) {
        Remove-Item -LiteralPath $target -Force -ErrorAction SilentlyContinue
    } elseif (Test-Path -LiteralPath $target -PathType Leaf) {
        Write-Warning "Association cleanup was incomplete; preserving the installed executable at $target."
    }
    throw
}
