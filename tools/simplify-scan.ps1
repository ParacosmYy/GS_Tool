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

function Get-SourceFiles([string]$Root) {
    $rg = Get-Command rg -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($rg) {
        Push-Location -LiteralPath $Root
        try {
            return @(& $rg.Source --files src -g '*.cpp' -g '*.h' 2>$null)
        } finally {
            Pop-Location
        }
    }

    $src = Join-Path $Root "src"
    if (-not (Test-Path -LiteralPath $src -PathType Container)) { return @() }
    return @(Get-ChildItem -LiteralPath $src -Recurse -File -Include *.cpp,*.h | ForEach-Object {
        Format-RelativePath $Root $_.FullName
    })
}

function Get-LineCountRows([string]$Root) {
    $rg = Get-Command rg -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($rg) {
        Push-Location -LiteralPath $Root
        try {
            $output = & $rg.Source --count '^' src -g '*.cpp' -g '*.h' 2>$null
            return @($output | ForEach-Object {
                if ($_ -match '^(.*):(\d+)$') {
                    [pscustomobject]@{ Path = $matches[1].Replace('\', '/'); Lines = [int]$matches[2] }
                }
            })
        } finally {
            Pop-Location
        }
    }

    $files = Get-SourceFiles $Root
    return @($files | ForEach-Object {
        $full = Join-Path $Root $_
        $count = 0
        foreach ($line in [System.IO.File]::ReadLines($full)) { $count++ }
        [pscustomobject]@{ Path = $_; Lines = $count }
    })
}

function New-FileRows([object[]]$LineRows, [string]$Extension, [int]$Limit, [int]$MaxFiles) {
    $rows = $LineRows |
        Where-Object { $_.Path.EndsWith($Extension, [System.StringComparison]::OrdinalIgnoreCase) -and $_.Lines -gt $Limit } |
        Sort-Object Lines -Descending |
        Select-Object -First $MaxFiles

    if (-not $rows) {
        return "- none"
    }

    return ($rows | ForEach-Object { "- $($_.Path) - $($_.Lines) lines" }) -join "`r`n"
}

function New-FrozenRows([string]$Root) {
    $frozen = @(
        'src/core/animation2',
        'src/core/widgets2',
        'src/plugin/loader2',
        'src/core/fonts',
        'src/core/icons',
        'src/core/responsive',
        'src/core/font',
        'src/core/icon',
        'src/core/shortcut',
        'src/core/managers'
    )

    $rows = foreach ($path in $frozen) {
        $full = Join-Path $Root $path
        if (Test-Path -LiteralPath $full -PathType Container) {
            $count = (Get-ChildItem -LiteralPath $full -Recurse -File | Measure-Object).Count
            "- $path - present, $count files"
        }
    }

    if (-not $rows) {
        return "- none"
    }
    return $rows -join "`r`n"
}

function New-ForbiddenBuildRows([string]$Root) {
    $bad = Get-ChildItem -LiteralPath $Root -Force -Directory |
        Where-Object { $_.Name -match '^(build2|build-debug|build-release|cmake-build)' }

    if (-not $bad) {
        return "- none"
    }
    return ($bad | ForEach-Object { "- " + (Format-RelativePath $Root $_.FullName) }) -join "`r`n"
}

function New-DuplicateFamilyRows([string]$Root) {
    $families = @(
        @('src/core/font', 'src/core/fonts'),
        @('src/core/icon', 'src/core/icons'),
        @('src/core/widgets', 'src/core/widgets2'),
        @('src/plugin/loader', 'src/plugin/loader2'),
        @('src/core/shortcut', 'src/core/managers')
    )

    $rows = foreach ($family in $families) {
        $present = @()
        foreach ($path in $family) {
            if (Test-Path -LiteralPath (Join-Path $Root $path) -PathType Container) {
                $present += $path
            }
        }
        if ($present.Count -gt 1) {
            "- " + ($present -join " vs ")
        }
    }

    if (-not $rows) {
        return "- none"
    }
    return $rows -join "`r`n"
}

function New-RiskKeywordRows([string]$Root) {
    $keywords = @('TODO', 'FIXME', 'setStyleSheet')
    $rows = foreach ($keyword in $keywords) {
        $count = 0
        $rg = Get-Command rg -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($rg) {
            Push-Location -LiteralPath $Root
            try {
                $output = & $rg.Source --count-matches --fixed-strings $keyword src 2>$null
                foreach ($line in $output) {
                    if ($line -match ':(\d+)$') {
                        $count += [int]$matches[1]
                    }
                }
            } finally {
                Pop-Location
            }
        } else {
            $files = Get-SourceFiles $Root
            foreach ($file in $files) {
                $matches = Select-String -LiteralPath (Join-Path $Root $file) -Pattern $keyword -SimpleMatch -ErrorAction SilentlyContinue
                if ($matches) {
                    $count += @($matches).Count
                }
            }
        }
        "- $keyword - $count occurrence(s)"
    }
    return $rows -join "`r`n"
}

function New-SimplifyReport([string]$Root, [int]$MaxFiles) {
    $files = @(Get-SourceFiles $Root)
    $lineRows = @(Get-LineCountRows $Root)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

    $cppRows = New-FileRows $lineRows ".cpp" 500 $MaxFiles
    $headerRows = New-FileRows $lineRows ".h" 200 $MaxFiles
    $frozenRows = New-FrozenRows $Root
    $badBuildRows = New-ForbiddenBuildRows $Root
    $duplicateRows = New-DuplicateFamilyRows $Root
    $riskRows = New-RiskKeywordRows $Root

    return @"
# Simplify Scan Report

## 1. Metadata

| Field | Value |
|------|-------|
| Time | $timestamp |
| Source files scanned | $($files.Count) |
| Max rows per section | $MaxFiles |

## 2. Oversized Cpp Files

$cppRows

## 3. Oversized Header Files

$headerRows

## 4. Frozen Directories Present

$frozenRows

## 5. Duplicate Directory Families

$duplicateRows

## 6. Forbidden Parallel Build Directories

$badBuildRows

## 7. Risk Keyword Counts

$riskRows

## 8. Recommended Next Step

Pick one small cleanup target and route it through the technical debt workflow. Do not refactor directly from this scan without a focused Specs or PRD.
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
