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

function Get-TopModule {
    param([string]$RepoPath)
    if ($RepoPath -match "^src/([^/]+)/") {
        return $Matches[1]
    }
    return "(root)"
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

$repoRoot = Get-RepoRoot
Set-Location $repoRoot

$resolvedOutFile = Assert-ReportPath -RepoRoot $repoRoot -Path $OutFile

$sourceExtensions = @(".cpp", ".cxx", ".cc", ".c", ".h", ".hpp", ".hh", ".ui", ".qrc")
$trackedFiles = @(git ls-files)
$untrackedFiles = @(git ls-files --others --exclude-standard)
$repoFiles = @(
    $trackedFiles + $untrackedFiles |
        Sort-Object -Unique |
        Where-Object { Test-Path -LiteralPath (Join-Path $repoRoot $_) }
)
$srcFiles = @(
    $repoFiles |
        ForEach-Object { Convert-ToRepoPath $_ } |
        Where-Object {
            $_ -like "src/*" -and $sourceExtensions -contains ([System.IO.Path]::GetExtension($_).ToLowerInvariant())
        }
)

$srcByModule = $srcFiles |
    Group-Object { Get-TopModule $_ } |
    Sort-Object Count -Descending

$cmakeFiles = @(
    $repoFiles |
        ForEach-Object { Convert-ToRepoPath $_ } |
        Where-Object { $_ -match "(^|/)CMakeLists\.txt$" -and $_ -notlike "build/*" }
)
$cmakeText = ($cmakeFiles | ForEach-Object { Get-Content -Raw -Path (Join-Path $repoRoot $_) }) -join [Environment]::NewLine
$absoluteStyleCmakeRefs = @(
    [regex]::Matches($cmakeText, "(?:src|tests)/[A-Za-z0-9_./+-]+\.(?:cpp|cxx|cc|c|h|hpp|hh|ui|qrc)") |
        ForEach-Object { Convert-ToRepoPath $_.Value } |
        Sort-Object -Unique
)
$testRelativeCmakeRefs = @(
    [regex]::Matches($cmakeText, "serial_station/[A-Za-z0-9_./+-]+\.(?:cpp|cxx|cc|c|h|hpp|hh|ui|qrc)") |
        ForEach-Object { Convert-ToRepoPath ("tests/" + $_.Value) } |
        Sort-Object -Unique
)
$cmakeRefs = @($absoluteStyleCmakeRefs + $testRelativeCmakeRefs | Sort-Object -Unique)

$cmakeByModule = $cmakeRefs |
    Group-Object { Get-TopModule $_ } |
    Sort-Object Count -Descending

$cmakeSet = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
foreach ($ref in $cmakeRefs) {
    [void]$cmakeSet.Add($ref)
}

$notDirectlyInCMake = @($srcFiles | Where-Object { -not $cmakeSet.Contains($_) })

$frozenPatterns = @(
    "src/core/animation2/",
    "src/core/widgets2/",
    "src/plugin/loader2/",
    "src/core/fonts/",
    "src/core/icons/",
    "src/core/responsive/",
    "src/core/font/",
    "src/core/icon/",
    "src/core/shortcut/",
    "src/core/managers/",
    "src/chart/heatmap2/",
    "src/serial/profiler2/"
)

$frozenHits = foreach ($pattern in $frozenPatterns) {
    $files = @($srcFiles | Where-Object { $_.StartsWith($pattern, [System.StringComparison]::OrdinalIgnoreCase) })
    [pscustomobject]@{
        Path = $pattern.TrimEnd("/")
        Count = $files.Count
        CMakeRefs = @($cmakeRefs | Where-Object { $_.StartsWith($pattern, [System.StringComparison]::OrdinalIgnoreCase) }).Count
    }
}

$utilsFileCounts = @{}
foreach ($path in $srcFiles) {
    if ($path -match "^src/utils/([^/]+)/") {
        $dir = $Matches[1]
        if (-not $utilsFileCounts.ContainsKey($dir)) {
            $utilsFileCounts[$dir] = 0
        }
        $utilsFileCounts[$dir]++
    }
}

$utilsCMakeCounts = @{}
foreach ($path in $cmakeRefs) {
    if ($path -match "^src/utils/([^/]+)/") {
        $dir = $Matches[1]
        if (-not $utilsCMakeCounts.ContainsKey($dir)) {
            $utilsCMakeCounts[$dir] = 0
        }
        $utilsCMakeCounts[$dir]++
    }
}

$utilsDirs = @($utilsFileCounts.Keys | Sort-Object)

$generatedUtilsDirs = @(
    $utilsDirs |
        Where-Object {
            $_ -match "\d{2,}$" -or
            $_ -match "^(c|poly|graph|tree|signal|matrix|vector|sort|search|hash|compress|crypto|math)\d+$"
        }
)

$generatedUtilsStats = foreach ($dir in $generatedUtilsDirs) {
    [pscustomobject]@{
        Path = "src/utils/$dir"
        Count = $utilsFileCounts[$dir]
        CMakeRefs = if ($utilsCMakeCounts.ContainsKey($dir)) { $utilsCMakeCounts[$dir] } else { 0 }
    }
}

$oldUartEvidence = @(
    "src/serial/config/SerialConfigPanel.h",
    "src/serial/config/SerialConfigPanelUI.cpp",
    "src/serial/config/SerialConfigPanelConfig.cpp",
    "src/connection/serial_port/SerialConnection.h",
    "src/connection/serial_port/SerialConnectionStats.cpp",
    "src/core/connect/ConnectionControllerLifecycle.cpp",
    "src/core/panels/PanelManagerQuery.cpp"
)

$oldUartRows = foreach ($path in $oldUartEvidence) {
    [pscustomobject]@{
        Path = $path
        Exists = Test-Path (Join-Path $repoRoot $path)
        InCMake = $cmakeSet.Contains($path)
    }
}

$serialStationExpected = @(
    "src/apps/serial_station/SerialStationApp.h",
    "src/apps/serial_station/SerialStationApp.cpp",
    "src/apps/serial_station/SerialStationWindow.h",
    "src/apps/serial_station/SerialStationWindow.cpp",
    "src/apps/serial_station/SerialStationController.h",
    "src/apps/serial_station/SerialStationController.cpp",
    "src/apps/serial_station/SerialStationConfig.h",
    "src/apps/serial_station/SerialStationConfig.cpp",
    "src/apps/serial_station/ui/SerialPortPanel.h",
    "src/apps/serial_station/ui/SerialPortPanel.cpp",
    "src/apps/serial_station/core/SerialPort.h",
    "src/apps/serial_station/core/SerialPort.cpp",
    "src/apps/serial_station/core/SerialManager.h",
    "src/apps/serial_station/core/SerialManager.cpp",
    "src/apps/serial_station/core/SerialSession.h",
    "src/apps/serial_station/core/SerialSession.cpp",
    "src/apps/serial_station/protocols/ISerialProtocol.h",
    "src/apps/serial_station/protocols/SerialProtocolRegistry.h",
    "src/apps/serial_station/protocols/SerialProtocolRegistry.cpp",
    "src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h",
    "src/apps/serial_station/protocols/ascii_text/AsciiTextProtocol.cpp",
    "tests/serial_station/test_serial_manager.cpp",
    "tests/serial_station/test_serial_protocol_registry.cpp",
    "tests/serial_station/test_ascii_text_protocol.cpp"
)

$serialStationRows = foreach ($path in $serialStationExpected) {
    [pscustomobject]@{
        Path = $path
        Exists = Test-Path (Join-Path $repoRoot $path)
        InCMake = $cmakeSet.Contains($path)
    }
}

$existingSerialStationFiles = @($repoFiles | ForEach-Object { Convert-ToRepoPath $_ } | Where-Object { $_ -like "src/apps/serial_station/*" })
$serialStationCMakeRefs = @($cmakeRefs | Where-Object { $_ -like "src/apps/serial_station/*" })

$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add("# Source Tree Audit")
$lines.Add("")
$lines.Add("- Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')")
$lines.Add("- Repo: $repoRoot")
$lines.Add("- Mode: read-only audit")

Add-Section $lines "Summary"
$lines.Add("| Metric | Value |")
$lines.Add("|--------|-------|")
$lines.Add("| Working tree source files under src | $($srcFiles.Count) |")
$lines.Add("| Direct CMake source references | $($cmakeRefs.Count) |")
$lines.Add("| Source files not directly referenced by CMake | $($notDirectlyInCMake.Count) |")
$lines.Add("| Existing src/apps/serial_station files | $($existingSerialStationFiles.Count) |")
$lines.Add("| CMake refs under src/apps/serial_station | $($serialStationCMakeRefs.Count) |")
$lines.Add("| Generated-looking utils directories | $($generatedUtilsDirs.Count) |")

Add-Section $lines "Source Files By Module"
$lines.Add("| Module | Files |")
$lines.Add("|--------|-------|")
foreach ($group in $srcByModule) {
    $lines.Add("| $($group.Name) | $($group.Count) |")
}

Add-Section $lines "CMake References By Module"
$lines.Add("| Module | References |")
$lines.Add("|--------|------------|")
foreach ($group in $cmakeByModule) {
    $lines.Add("| $($group.Name) | $($group.Count) |")
}

Add-Section $lines "Frozen Or Duplicate Directory Hits"
$lines.Add("| Path | Files | CMake refs |")
$lines.Add("|------|-------|------------|")
foreach ($row in ($frozenHits | Sort-Object Count -Descending)) {
    $lines.Add("| $($row.Path) | $($row.Count) | $($row.CMakeRefs) |")
}

Add-Section $lines "Generated-Looking Utils Directories"
$lines.Add("| Path | Files | CMake refs |")
$lines.Add("|------|-------|------------|")
foreach ($row in ($generatedUtilsStats | Sort-Object Count -Descending | Select-Object -First 80)) {
    $lines.Add("| $($row.Path) | $($row.Count) | $($row.CMakeRefs) |")
}
if ($generatedUtilsStats.Count -gt 80) {
    $lines.Add("")
    $lines.Add("Only the first 80 generated-looking utils directories are listed.")
}

Add-Section $lines "Old UART Configuration Evidence"
$lines.Add("| Path | Exists | In CMake |")
$lines.Add("|------|--------|----------|")
foreach ($row in $oldUartRows) {
    $lines.Add("| $($row.Path) | $($row.Exists) | $($row.InCMake) |")
}

Add-Section $lines "Serial Station Minimal UART Gap"
$lines.Add("| Path | Exists | In CMake |")
$lines.Add("|------|--------|----------|")
foreach ($row in $serialStationRows) {
    $lines.Add("| $($row.Path) | $($row.Exists) | $($row.InCMake) |")
}

Add-Section $lines "Existing Serial Station Files"
if ($existingSerialStationFiles.Count -eq 0) {
    $lines.Add('No tracked files found under `src/apps/serial_station/`.')
} else {
    foreach ($path in $existingSerialStationFiles | Sort-Object) {
        $lines.Add("- ``$path``")
    }
}

Add-Section $lines "Interpretation"
$lines.Add('1. A high `src/utils` count usually means historical generated or duplicate utility code should be frozen and audited before deletion.')
$lines.Add('2. Old UART configuration exists in the current mainline. The missing part is the new `src/apps/serial_station/` minimal UART loop.')
$lines.Add("3. Do not delete by directory name alone. First remove unneeded files from CMake, build, launch, then delete in a separate reviewed step.")

$output = $lines -join [Environment]::NewLine

if ($resolvedOutFile) {
    Set-Content -Path $resolvedOutFile -Value $output -Encoding UTF8
    Write-Host "Report written: $resolvedOutFile"
} else {
    Write-Output $output
}
