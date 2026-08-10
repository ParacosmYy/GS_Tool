<#
Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Run the repository's deterministic source-quality gate.
Module: CI/CD / source quality boundary

This gate is intentionally source-only. It does not create databases, users,
backups, release archives, test-only assets, or deployment state. External
toolchain and credential gates remain visible through the release doctor.
#>

[CmdletBinding()]
param(
    [string]$PythonPath = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..\")).Path
$windowsRoot = Join-Path $repositoryRoot "windows"
$python = if ([string]::IsNullOrWhiteSpace($PythonPath)) {
    (Get-Command python -CommandType Application -ErrorAction Stop).Source
} else {
    (Resolve-Path -LiteralPath $PythonPath).Path
}

if (-not (Test-Path -LiteralPath $python -PathType Leaf)) {
    throw "Python executable is missing: $python"
}

function Invoke-Checked([string]$Label, [scriptblock]$Command) {
    Write-Host "[quality-gate] $Label"
    & $Command
    if ($LASTEXITCODE -ne 0) {
        throw "$Label failed with exit code $LASTEXITCODE"
    }
}

Push-Location $repositoryRoot
try {
    Invoke-Checked "git diff --check" { git diff --check }
    Push-Location $windowsRoot
    try {
        Invoke-Checked "Python compileall" { & $python -m compileall -q token_tracker }

        $auditRaw = & $python -m token_tracker audit --json 2>&1
        if ($LASTEXITCODE -ne 0) {
            throw "token_tracker audit failed with exit code $LASTEXITCODE"
        }
        $audit = ($auditRaw -join [Environment]::NewLine) | ConvertFrom-Json
        if ([int]$audit.summary.fail -gt 0) {
            throw "token_tracker audit reported $($audit.summary.fail) failing checks"
        }
        Write-Host "[quality-gate] release audit: pass=$($audit.summary.pass) pending=$($audit.summary.pending) fail=$($audit.summary.fail)"

        Invoke-Checked "CLI help" { & $python -m token_tracker --help }
    }
    finally {
        Pop-Location
    }

    $ignoredFragments = @(
        "\.venv\",
        "\.cache\",
        "\android\.gradle\",
        "\.toolchain\",
        "\build\",
        "\dist\",
        "\release\"
    )
    $parseFailures = [System.Collections.Generic.List[string]]::new()
    $powershellFiles = @(Get-ChildItem -LiteralPath $repositoryRoot -Recurse -File -Filter "*.ps1" |
        Where-Object {
            $fullName = $_.FullName
            -not ($ignoredFragments | Where-Object { $fullName.IndexOf($_, [System.StringComparison]::OrdinalIgnoreCase) -ge 0 })
        })
    foreach ($path in $powershellFiles) {
        $tokens = $null
        $errors = $null
        [System.Management.Automation.Language.Parser]::ParseFile($path.FullName, [ref]$tokens, [ref]$errors) | Out-Null
        if ($errors.Count -gt 0) {
            $parseFailures.Add($path.FullName)
        }
    }
    if ($parseFailures.Count -gt 0) {
        throw "PowerShell parse failures: $($parseFailures -join ', ')"
    }
    Write-Host "[quality-gate] PowerShell AST parse: $($powershellFiles.Count) files passed"
}
finally {
    Pop-Location
}

Write-Host "Source quality gate passed. Pending external gates remain explicit in release-doctor."
