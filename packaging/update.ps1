#requires -Version 5.1
[CmdletBinding(SupportsShouldProcess = $true, ConfirmImpact = "Medium")]
param(
    [string]$ArtifactPath,
    [string]$ExpectedSha256,
    [string]$InstallRoot = (Join-Path $env:LOCALAPPDATA "Programs\QuillForge"),
    [switch]$Rollback
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
$backup = if ([string]::IsNullOrWhiteSpace([string]$state.backup_path)) {
    $null
} else {
    ConvertTo-QfFullPath ([string]$state.backup_path)
}

if ($Rollback) {
    if ($null -eq $backup -or -not (Test-Path -LiteralPath $backup -PathType Leaf)) {
        throw "No retained QuillForge rollback copy is available."
    }
    if ([string]::Equals($backup, $target, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Install state rollback copy must differ from the executable path."
    }
    if (-not $PSCmdlet.ShouldProcess($target, "Rollback QuillForge to $backup")) {
        return
    }
    $null = Get-QfArtifactInfo $target ([string]$state.sha256)
    $null = Get-QfArtifactInfo $backup ([string]$state.previous_sha256)
    $originalTargetMoved = $false
    $originalBackupMoved = $false
    $stateCommitted = $false
    $originalTargetSha256 = ([string]$state.sha256).ToUpperInvariant()
    $originalBackupSha256 = ([string]$state.previous_sha256).ToUpperInvariant()
    $rollbackOld = Get-QfBackupPath $target
    Move-Item -LiteralPath $target -Destination $rollbackOld -Force
    $originalTargetMoved = $true
    try {
        Move-Item -LiteralPath $backup -Destination $target -Force
        $originalBackupMoved = $true
        $restored = Get-QfArtifactInfo $target ([string]$state.previous_sha256)
        $state.previous_sha256 = $state.sha256
        $state.sha256 = $restored.sha256
        $state.bytes = $restored.bytes
        $state.backup_path = $rollbackOld
        $state.updated_at_utc = [DateTime]::UtcNow.ToString("o")
        Write-QfState $root $state
        $stateCommitted = $true
        Write-Output "Rolled back QuillForge at $target"
    } catch {
        if (-not $stateCommitted -and ($originalTargetMoved -or $originalBackupMoved)) {
            $restorePossible = $true
            try {
                if ($originalBackupMoved -and (Test-Path -LiteralPath $target -PathType Leaf)) {
                    $currentHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash.ToUpperInvariant()
                    if ($currentHash -ne $originalBackupSha256) {
                        $restorePossible = $false
                        Write-Warning "Rollback recovery found a changed target; preserving it at $target."
                    } elseif (Test-Path -LiteralPath $backup) {
                        $restorePossible = $false
                        Write-Warning "Rollback recovery found an occupied backup path; preserving files."
                    } else {
                        Move-Item -LiteralPath $target -Destination $backup -Force
                    }
                }
                if ($restorePossible -and $originalTargetMoved -and (Test-Path -LiteralPath $rollbackOld -PathType Leaf)) {
                    if (Test-Path -LiteralPath $target -PathType Leaf) {
                        $currentHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash.ToUpperInvariant()
                        if ($currentHash -ne $originalTargetSha256) {
                            $restorePossible = $false
                            Write-Warning "Rollback recovery found a changed target; preserving it at $target."
                        }
                    }
                    if ($restorePossible -and -not (Test-Path -LiteralPath $target -PathType Leaf)) {
                        Move-Item -LiteralPath $rollbackOld -Destination $target -Force
                    }
                }
            } catch {
                Write-Warning "Rollback recovery was incomplete; inspect $target and $rollbackOld before retrying."
            }
        }
        throw
    }
    return
}

if ([string]::IsNullOrWhiteSpace($ArtifactPath) -or [string]::IsNullOrWhiteSpace($ExpectedSha256)) {
    throw "ArtifactPath and ExpectedSha256 are required unless -Rollback is used."
}
$artifact = Get-QfArtifactInfo $ArtifactPath $ExpectedSha256
if (-not $PSCmdlet.ShouldProcess($target, "Update QuillForge to $($artifact.sha256)")) {
    return
}
$null = Get-QfArtifactInfo $target ([string]$state.sha256)

$temporary = Join-Path $root ".QuillForge.$PID.update.tmp.exe"
$newBackup = Get-QfBackupPath $target
$previousTargetMoved = $false
$stateCommitted = $false
$previousSha256 = ([string]$state.sha256).ToUpperInvariant()
try {
    Copy-Item -LiteralPath $artifact.path -Destination $temporary -Force
    $copied = Get-QfArtifactInfo $temporary $artifact.sha256
    Move-Item -LiteralPath $target -Destination $newBackup -Force
    $previousTargetMoved = $true
    try {
        Move-Item -LiteralPath $temporary -Destination $target -Force
    } catch {
        Move-Item -LiteralPath $newBackup -Destination $target -Force -ErrorAction SilentlyContinue
        throw
    }
    $state.backup_path = $newBackup
    $state.previous_sha256 = [string]$state.sha256
    $state.sha256 = $copied.sha256
    $state.bytes = $copied.bytes
    $state.updated_at_utc = [DateTime]::UtcNow.ToString("o")
    Write-QfState $root $state
    $stateCommitted = $true
    Write-Output "Updated QuillForge at $target"
    Write-Output "SHA-256: $($copied.sha256)"
} catch {
    if (Test-Path -LiteralPath $temporary -PathType Leaf) {
        Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
    }
    if ($previousTargetMoved -and -not $stateCommitted) {
        try {
            $restorePossible = $true
            if (Test-Path -LiteralPath $target -PathType Leaf) {
                $currentHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $target).Hash.ToUpperInvariant()
                if ($currentHash -ne ([string]$copied.sha256).ToUpperInvariant() -and $currentHash -ne $previousSha256) {
                    $restorePossible = $false
                    Write-Warning "Update recovery found a changed target; preserving it at $target."
                }
                if ($restorePossible -and $currentHash -eq ([string]$copied.sha256).ToUpperInvariant()) {
                    Remove-Item -LiteralPath $target -Force
                }
            }
            if ($restorePossible -and -not (Test-Path -LiteralPath $target -PathType Leaf) -and (Test-Path -LiteralPath $newBackup -PathType Leaf)) {
                Move-Item -LiteralPath $newBackup -Destination $target -Force
            }
        } catch {
            Write-Warning "Update recovery was incomplete; inspect $target and $newBackup before retrying."
        }
    }
    throw
}
