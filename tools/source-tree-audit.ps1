param(
    [string]$OutFile
)

$ErrorActionPreference = "Stop"

function Get-RepoRoot {
    $root = git rev-parse --show-toplevel 2>$null
    if (-not $root) {
        throw "Unable to resolve repository root. Run this script inside the repository."
    }
    return (Resolve-Path $root).Path
}

function Convert-ToRepoPath {
    param([string]$Path)
    return ($Path -replace "\\", "/")
}

function Assert-ReportPath {
    param(
        [string]$RepoRoot,
        [string]$Path
    )

    if ([string]::IsNullOrWhiteSpace($Path)) {
        return $null
    }

    $target = [System.IO.Path]::GetFullPath((Join-Path (Get-Location).Path $Path))
    $allowed = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot "docs/reviews/simplify"))

    if (-not $target.StartsWith($allowed, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "OutFile must stay under docs/reviews/simplify."
    }

    $parent = Split-Path -Parent $target
    if (-not (Test-Path $parent)) {
        New-Item -ItemType Directory -Path $parent | Out-Null
    }

    return $target
}

function Add-Section {
    param(
        [System.Collections.Generic.List[string]]$Lines,
        [string]$Title
    )
    $Lines.Add("")
    $Lines.Add("## $Title")
    $Lines.Add("")
}

function Get-TopPythonModule {
    param([string]$RepoPath)
    if ($RepoPath -match "^python/embeddebug/([^/]+)/") {
        return $Matches[1]
    }
    if ($RepoPath -match "^tests/python/([^/]+)/") {
        return "tests/$($Matches[1])"
    }
    if ($RepoPath -match "^tools/([^/]+)/") {
        return "tools/$($Matches[1])"
    }
    if ($RepoPath -like "tools/*.py") {
        return "tools"
    }
    return "(root)"
}

$repoRoot = Get-RepoRoot
Set-Location $repoRoot

$resolvedOutFile = Assert-ReportPath -RepoRoot $repoRoot -Path $OutFile

$trackedFiles = @(git ls-files)
$untrackedFiles = @(git ls-files --others --exclude-standard)
$repoFiles = @(
    $trackedFiles + $untrackedFiles |
        Sort-Object -Unique |
        Where-Object { Test-Path -LiteralPath (Join-Path $repoRoot $_) }
)
$pythonFiles = @(
    $repoFiles |
        ForEach-Object { Convert-ToRepoPath $_ } |
        Where-Object {
            ($_ -like "python/embeddebug/*.py" -or
             $_ -like "python/embeddebug/*/*.py" -or
             $_ -like "python/embeddebug/*/*/*.py" -or
             $_ -like "tests/python/*.py" -or
             $_ -like "tests/python/*/*.py" -or
             $_ -like "tests/python/*/*/*.py" -or
             $_ -like "tools/*.py" -or
             $_ -like "tools/*/*.py")
        }
)

$moduleGroups = $pythonFiles |
    Group-Object { Get-TopPythonModule $_ } |
    Sort-Object Count -Descending

$entrypoints = @(
    "EmbedDebug.bat",
    "tools/launch_embeddebug.ps1",
    "tools/start_embeddebug.py",
    "python/embeddebug/app/main.py",
    "python/embeddebug/devtools/package_pyinstaller.py",
    "python/embeddebug/devtools/verify_pyinstaller_package.py"
)

$entryRows = foreach ($path in $entrypoints) {
    [pscustomobject]@{
        Path = $path
        Exists = Test-Path (Join-Path $repoRoot $path)
    }
}

$serialStationFiles = @($pythonFiles | Where-Object { $_ -like "python/embeddebug/serial_station/*" })
$pythonTests = @($pythonFiles | Where-Object { $_ -like "tests/python/*" })
$toolFiles = @($pythonFiles | Where-Object { $_ -like "tools/*" })

$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add("# Python Source Tree Audit")
$lines.Add("")
$lines.Add("- Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$lines.Add("- Repo: $repoRoot")
$lines.Add("- Mode: read-only audit")

Add-Section $lines "Summary"
$lines.Add("| Metric | Value |")
$lines.Add("|--------|-------|")
$lines.Add("| Python runtime/test/tool files | $($pythonFiles.Count) |")
$lines.Add("| Serial Station Python files | $($serialStationFiles.Count) |")
$lines.Add("| Python test files | $($pythonTests.Count) |")
$lines.Add("| Python tool files | $($toolFiles.Count) |")

Add-Section $lines "Python Files By Module"
$lines.Add("| Module | Files |")
$lines.Add("|--------|-------|")
foreach ($group in $moduleGroups) {
    $lines.Add("| $($group.Name) | $($group.Count) |")
}

Add-Section $lines "Active Entrypoints"
$lines.Add("| Path | Exists |")
$lines.Add("|------|--------|")
foreach ($row in $entryRows) {
    $lines.Add("| $($row.Path) | $($row.Exists) |")
}

Add-Section $lines "Serial Station Python Files"
if ($serialStationFiles.Count -eq 0) {
    $lines.Add("No Python Serial Station files found.")
} else {
    foreach ($path in $serialStationFiles | Sort-Object) {
        $lines.Add("- ``$path``")
    }
}

Add-Section $lines "Interpretation"
$lines.Add("1. Active product code should land under `python/embeddebug/`.")
$lines.Add("2. Python tests should land under `tests/python/` or shared fixtures under `tests/fixtures/`.")
$lines.Add("3. Startup, package, and verification behavior should remain exposed through `uv run ...` scripts.")
$lines.Add("4. Generated packages, caches, and temporary work directories must stay out of git.")

$output = $lines -join [Environment]::NewLine

if ($resolvedOutFile) {
    Set-Content -Path $resolvedOutFile -Value $output -Encoding UTF8
    Write-Host "Report written: $resolvedOutFile"
} else {
    Write-Output $output
}
