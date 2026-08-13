$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location -LiteralPath $root

$projectVersionMatch = [regex]::Match(
    (Get-Content -Raw -LiteralPath (Join-Path $root "pyproject.toml")),
    '(?m)^version\s*=\s*"([^"]+)"'
)
if (-not $projectVersionMatch.Success) {
    throw "Project version is missing from pyproject.toml."
}
$version = $projectVersionMatch.Groups[1].Value
$noticeSource = Join-Path $root "docs\third_party\NOTICE.md"
if (-not (Test-Path -LiteralPath $noticeSource)) {
    throw "Third-party notice inventory is missing: $noticeSource"
}

uv run pyinstaller --noconfirm --clean packaging/quillforge.spec
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$distExe = Join-Path $root "dist\QuillForge.exe"
$rootExe = Join-Path $root "QuillForge.exe"
if (-not (Test-Path -LiteralPath $distExe)) {
    throw "PyInstaller did not produce the expected artifact: $distExe"
}

$stagedRootExe = "$rootExe.staging"
$backupRootExe = "$rootExe.backup"
try {
    Copy-Item -LiteralPath $distExe -Destination $stagedRootExe -Force
    if (Test-Path -LiteralPath $rootExe) {
        if (Test-Path -LiteralPath $backupRootExe) {
            Remove-Item -LiteralPath $backupRootExe -Force
        }
        [System.IO.File]::Replace($stagedRootExe, $rootExe, $backupRootExe, $true)
    } else {
        [System.IO.File]::Move($stagedRootExe, $rootExe)
    }
} finally {
    if (Test-Path -LiteralPath $stagedRootExe) {
        Remove-Item -LiteralPath $stagedRootExe -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path -LiteralPath $backupRootExe) {
        Remove-Item -LiteralPath $backupRootExe -Force -ErrorAction SilentlyContinue
    }
}

$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $distExe).Hash
$rootHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $rootExe).Hash
if ($rootHash -ne $hash) {
    throw "Root test copy hash differs from the packaged artifact."
}
$manifest = Join-Path $root "dist\QuillForge.sha256"
"$hash *QuillForge.exe" | Set-Content -LiteralPath $manifest -Encoding ascii

$noticeDestination = Join-Path $root "dist\NOTICE.md"
Copy-Item -LiteralPath $noticeSource -Destination $noticeDestination -Force
$noticeHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $noticeDestination).Hash
$noticeBytes = (Get-Item -LiteralPath $noticeDestination).Length
$lockfileHash = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $root "uv.lock")).Hash
$sourceFiles = @()
foreach ($sourceRoot in @("src", "scripts", "packaging")) {
    $sourceFiles += Get-ChildItem -LiteralPath (Join-Path $root $sourceRoot) -File -Recurse |
        Where-Object { $_.FullName -notmatch "\\__pycache__\\" }
}
$sourceFiles += Get-Item -LiteralPath (Join-Path $root "pyproject.toml")
$sourceFiles += Get-Item -LiteralPath (Join-Path $root "uv.lock")
$rootPath = [System.IO.Path]::GetFullPath($root)
$rootUri = New-Object System.Uri(($rootPath.TrimEnd('\') + '\'))
$sourceLines = New-Object 'System.Collections.Generic.List[string]'
foreach ($sourceFile in $sourceFiles) {
    $fileUri = New-Object System.Uri($sourceFile.FullName)
    $relative = [System.Uri]::UnescapeDataString(
        $rootUri.MakeRelativeUri($fileUri).ToString()
    )
    $fileHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $sourceFile.FullName).Hash.ToUpperInvariant()
    [void]$sourceLines.Add("$relative=$fileHash")
}
$sourceLines.Sort([System.StringComparer]::Ordinal)
$sourceRevisionBytes = [System.Text.Encoding]::UTF8.GetBytes(($sourceLines -join "`n"))
$sourceRevisionHasher = [System.Security.Cryptography.SHA256]::Create()
try {
    $sourceRevisionHash = $sourceRevisionHasher.ComputeHash($sourceRevisionBytes)
} finally {
    $sourceRevisionHasher.Dispose()
}
$sourceRevisionHex = [System.BitConverter]::ToString($sourceRevisionHash).Replace("-", "").ToLowerInvariant()
$sourceRevision = "tree-sha256:" + $sourceRevisionHex
$architecture = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString()
$pythonVersion = ((& uv run python -c "import platform; print(platform.python_version())") | Select-Object -Last 1).Trim()
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$pyinstallerVersion = ((& uv run pyinstaller --version) | Select-Object -Last 1).Trim()
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$manifestArgs = @(
    "scripts/write_release_manifest.py",
    "--manifest", "dist/QuillForge.release.json",
    "--version", $version,
    "--architecture", "Windows $architecture",
    "--packaging", "PyInstaller one-file portable candidate",
    "--source-revision", $sourceRevision,
    "--lockfile-sha256", $lockfileHash,
    "--built-at-utc", [DateTime]::UtcNow.ToString("o"),
    "--python", $pythonVersion,
    "--pyinstaller", $pyinstallerVersion,
    "--version-resource", "packaging/version_info.txt",
    "--warning-file", "build/quillforge/warn-quillforge.txt",
    "--artifact-path", "dist/QuillForge.exe",
    "--artifact-bytes", (Get-Item -LiteralPath $distExe).Length,
    "--artifact-sha256", $hash,
    "--root-path", "QuillForge.exe",
    "--root-bytes", (Get-Item -LiteralPath $rootExe).Length,
    "--root-sha256", $rootHash,
    "--support-owner", "Project Manager (QuillForge)",
    "--support-issue-route", "docs/support/ISSUES.md",
    "--notice", "dist/NOTICE.md", $noticeBytes, $noticeHash
)
& uv run python @manifestArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Package output: $distExe"
Write-Host "Root test copy: $rootExe"
Write-Host "SHA-256: $hash"
Write-Host "Release manifest: $root\dist\QuillForge.release.json"
