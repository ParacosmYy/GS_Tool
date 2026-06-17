param(
    [int]$MaxFiles = 20,
    [string]$OutFile = ""
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

function Assert-SimplifyOutFile([string]$Root, [string]$Path) {
    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }

    $reportRoot = [System.IO.Path]::GetFullPath((Join-Path $Root 'docs\reviews\simplify'))
    $full = [System.IO.Path]::GetFullPath((Join-Path $Root $Path))

    $reportPrefix = $reportRoot.TrimEnd('\') + '\'
    if (-not $full.StartsWith($reportPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw 'OutFile must be inside docs\reviews\simplify.'
    }
    return $full
}

function Format-RelativePath([string]$Root, [string]$Path) {
    $rootPrefix = $Root.TrimEnd('\') + '\'
    if ($Path.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $Path.Substring($rootPrefix.Length).Replace('\', '/')
    }
    return $Path.Replace('\', '/')
}

function Get-PythonFiles([string]$Root) {
    $roots = @('python', 'tests\python', 'tools')
    $files = foreach ($relativeRoot in $roots) {
        $fullRoot = Join-Path $Root $relativeRoot
        if (Test-Path -LiteralPath $fullRoot -PathType Container) {
            Get-ChildItem -LiteralPath $fullRoot -Recurse -File -Filter *.py | ForEach-Object {
                Format-RelativePath $Root $_.FullName
            }
        }
    }
    return @($files | Sort-Object -Unique)
}

function Get-LineCountRows([string]$Root) {
    return @(Get-PythonFiles $Root | ForEach-Object {
        $full = Join-Path $Root $_
        $count = 0
        foreach ($line in [System.IO.File]::ReadLines($full)) { $count++ }
        [pscustomobject]@{ Path = $_; Lines = $count }
    })
}

function New-FileRows([object[]]$LineRows, [int]$Limit, [int]$MaxFiles) {
    $rows = $LineRows |
        Where-Object { $_.Lines -gt $Limit } |
        Sort-Object Lines -Descending |
        Select-Object -First $MaxFiles

    if (-not $rows) {
        return "- none"
    }

    return ($rows | ForEach-Object { "- $($_.Path) - $($_.Lines) lines" }) -join "`r`n"
}

function New-ForbiddenBuildRows([string]$Root) {
    $bad = Get-ChildItem -LiteralPath $Root -Force -Directory |
        Where-Object {
            $_.Name -in @('build2', 'build-debug', 'build-release') -or
            ($_.Name -like '*-build' -and $_.Name -ne 'build')
        }

    if (-not $bad) {
        return "- none"
    }
    return ($bad | ForEach-Object { "- " + (Format-RelativePath $Root $_.FullName) }) -join "`r`n"
}

function New-RiskKeywordRows([string]$Root) {
    $keywords = @('TODO', 'FIXME', 'Any', 'type: ignore')
    $files = Get-PythonFiles $Root
    $rows = foreach ($keyword in $keywords) {
        $count = 0
        foreach ($file in $files) {
            $matches = Select-String -LiteralPath (Join-Path $Root $file) -Pattern $keyword -SimpleMatch -ErrorAction SilentlyContinue
            if ($matches) {
                $count += @($matches).Count
            }
        }
        "- $keyword - $count occurrence(s)"
    }
    return $rows -join "`r`n"
}

function New-SimplifyReport([string]$Root, [int]$MaxFiles) {
    $files = @(Get-PythonFiles $Root)
    $lineRows = @(Get-LineCountRows $Root)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

    $oversizedRows = New-FileRows $lineRows 300 $MaxFiles
    $badBuildRows = New-ForbiddenBuildRows $Root
    $riskRows = New-RiskKeywordRows $Root

    return @"
# Python Simplify Scan Report

## 1. Metadata

| Field | Value |
|------|-------|
| Time | $timestamp |
| Python files scanned | $($files.Count) |
| Max rows per section | $MaxFiles |

## 2. Oversized Python Files

$oversizedRows

## 3. Forbidden Parallel Build Directories

$badBuildRows

## 4. Risk Keyword Counts

$riskRows

## 5. Recommended Next Step

Pick one small cleanup target and route it through the Python/PyQt Specs workflow. Do not refactor directly from this scan without a focused PRD or Specs batch.
"@
}

$root = Resolve-RepoRoot
if (-not (Test-Path -LiteralPath (Join-Path $root "CLAUDE.md") -PathType Leaf)) {
    throw "Repository root could not be resolved."
}

$fullOut = Assert-SimplifyOutFile $root $OutFile
$report = New-SimplifyReport $root $MaxFiles

if ([string]::IsNullOrWhiteSpace($OutFile)) {
    Write-Output $report
    exit 0
}

$outDir = Split-Path -Parent $fullOut
if (-not (Test-Path -LiteralPath $outDir -PathType Container)) {
    New-Item -ItemType Directory -Path $outDir | Out-Null
}

Set-Content -LiteralPath $fullOut -Value $report -Encoding UTF8
Write-Host "[OK] Simplify scan written: $fullOut"
exit 0
