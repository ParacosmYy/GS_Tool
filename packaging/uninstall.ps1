#requires -Version 5.1
[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = "High")]
param(
    [string]$InstallRoot = (Join-Path $env:LOCALAPPDATA "Programs\QuillForge")
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
Import-Module (Join-Path $PSScriptRoot "QuillForge.Distribution.psm1") -Force

$root = Assert-QfInstallRoot $InstallRoot
$state = Read-QfState $root
$target = ConvertTo-QfFullPath ([string]$state.executable_path)
if (-not (Test-QfPathWithin $root $target)) {
    throw "Install state executable is outside the install root."
}
if (-not $PSCmdlet.ShouldProcess($root, "Uninstall QuillForge and only owned associations")) {
    return
}

$associations = @($state.file_associations | Where-Object { $null -ne $_ })
$allAssociationsRemoved = $true
foreach ($association in $associations) {
    if ($null -ne $association) {
        $removed = Remove-QfFileAssociation $association
        if (-not $removed) {
            $allAssociationsRemoved = $false
            Write-Warning "Association changed by the user; leaving it untouched: $($association.extension)"
        }
    }
}
if ($allAssociationsRemoved -and $associations.Count -gt 0) {
    if (-not (Remove-QfProgId $associations[0])) {
        $allAssociationsRemoved = $false
        Write-Warning "QuillForge ProgId changed by the user; leaving it untouched."
    }
}
if (-not $allAssociationsRemoved) {
    Write-Warning "Association cleanup was incomplete; preserving the installed executable and state."
    return
}

$paths = @($target)
if (-not [string]::IsNullOrWhiteSpace([string]$state.backup_path)) {
    $backup = ConvertTo-QfFullPath ([string]$state.backup_path)
    if (Test-QfPathWithin $root $backup) {
        $paths += $backup
    }
}
$paths += (Get-QfStatePath $root)
foreach ($path in $paths) {
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        Remove-Item -LiteralPath $path -Force
    }
}
$remaining = @(Get-ChildItem -LiteralPath $root -Force -ErrorAction SilentlyContinue)
if ($remaining.Count -eq 0) {
    Remove-Item -LiteralPath $root -Force
}
Write-Output "Uninstalled QuillForge from $root"
