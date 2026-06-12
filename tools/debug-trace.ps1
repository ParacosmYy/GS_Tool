param(
    [Parameter(Mandatory = $true)]
    [string]$Title,

    [ValidateSet("P0", "P1", "P2", "P3")]
    [string]$Severity = "P2",

    [Parameter(Mandatory = $true)]
    [string]$ReproCommand,

    [Parameter(Mandatory = $true)]
    [string]$ErrorSummary,

    [Parameter(Mandatory = $true)]
    [string]$ImpactScope,

    [string[]]$CandidateFiles = @(),

    [Parameter(Mandatory = $true)]
    [string]$RegressionCommand,

    [string]$OutFile = ""
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

function Escape-Markdown([string]$Value) {
    if ([string]::IsNullOrWhiteSpace($Value)) {
        return "not provided"
    }
    return $Value
}

function Assert-DebugOutFile([string]$Root, [string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }

    $debugRoot = [System.IO.Path]::GetFullPath((Join-Path $Root 'docs\reviews\debug'))
    $full = [System.IO.Path]::GetFullPath((Join-Path $Root $Path))

    if (-not $full.StartsWith($debugRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw 'OutFile must be inside docs\reviews\debug.'
    }
    return $full
}

function New-DebugReport {
    param(
        [string]$Title,
        [string]$Severity,
        [string]$ReproCommand,
        [string]$ErrorSummary,
        [string]$ImpactScope,
        [string[]]$CandidateFiles,
        [string]$RegressionCommand
    )

    $files = if ($CandidateFiles.Count -eq 0) {
        "- not narrowed"
    } else {
        ($CandidateFiles | ForEach-Object { "- " + (Escape-Markdown $_) }) -join "`r`n"
    }

    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

    return @"
# Debug Trace - $(Escape-Markdown $Title)

## 1. Metadata

| Field | Value |
|------|-------|
| Time | $timestamp |
| Severity | $Severity |
| Impact Scope | $(Escape-Markdown $ImpactScope) |

## 2. Reproduce

~~~powershell
$(Escape-Markdown $ReproCommand)
~~~

## 3. Error Summary

$(Escape-Markdown $ErrorSummary)

## 4. Candidate Files

$files

## 5. Regression Command

~~~powershell
$(Escape-Markdown $RegressionCommand)
~~~

## 6. LOOP Route

- Doctor: run powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1 before fixing environment-sensitive failures.
- Debug: keep the fix scoped to the candidate files or update this trace with a narrower root cause.
- Simplify: if the root cause is duplicated logic, an oversized file, or unclear ownership, open a Simplify task before expanding the fix.

## 7. Closeout Checklist

- [ ] Reproduce command was run and captured.
- [ ] Root cause was narrowed to the smallest practical file range.
- [ ] Fix did not touch files outside the approved scope.
- [ ] Regression command passed.
- [ ] EmbedDebug.bat was verified if build, launch, runtime path, resources, or dependencies were affected.
"@
}

$root = Resolve-RepoRoot
if (-not (Test-Path -LiteralPath (Join-Path $root "CLAUDE.md") -PathType Leaf)) {
    throw "Repository root could not be resolved."
}

$report = New-DebugReport `
    -Title $Title `
    -Severity $Severity `
    -ReproCommand $ReproCommand `
    -ErrorSummary $ErrorSummary `
    -ImpactScope $ImpactScope `
    -CandidateFiles $CandidateFiles `
    -RegressionCommand $RegressionCommand

if ([string]::IsNullOrWhiteSpace($OutFile)) {
    Write-Output $report
    exit 0
}

$fullOut = Assert-DebugOutFile $root $OutFile
$outDir = Split-Path -Parent $fullOut
if (-not (Test-Path -LiteralPath $outDir -PathType Container)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

Set-Content -LiteralPath $fullOut -Value $report -Encoding UTF8
Write-Host "[OK] Debug trace written: $fullOut"
exit 0
