Set-StrictMode -Version Latest

$script:QfStateSchemaVersion = 1
$script:QfProgId = "QuillForge.Document"

function ConvertTo-QfFullPath {
    param([Parameter(Mandatory = $true)][string]$Path)

    if ([string]::IsNullOrWhiteSpace($Path)) {
        throw "Path must not be empty."
    }
    try {
        return [System.IO.Path]::GetFullPath($Path)
    } catch {
        throw "Path is invalid: $Path"
    }
}

function Assert-QfInstallRoot {
    param([Parameter(Mandatory = $true)][string]$InstallRoot)

    $full = ConvertTo-QfFullPath $InstallRoot
    $root = [System.IO.Path]::GetPathRoot($full)
    if ([string]::IsNullOrWhiteSpace($root) -or
        $full.TrimEnd('\') -eq $root.TrimEnd('\')) {
        throw "Refusing to use a filesystem root as the QuillForge install root: $full"
    }
    return $full.TrimEnd('\')
}

function Test-QfPathWithin {
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [Parameter(Mandatory = $true)][string]$Candidate
    )

    $rootFull = (ConvertTo-QfFullPath $Root).TrimEnd('\') + '\'
    $candidateFull = ConvertTo-QfFullPath $Candidate
    return $candidateFull.StartsWith($rootFull, [System.StringComparison]::OrdinalIgnoreCase)
}

function Get-QfArtifactInfo {
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$ExpectedSha256
    )

    $full = ConvertTo-QfFullPath $Path
    if (-not (Test-Path -LiteralPath $full -PathType Leaf)) {
        throw "Artifact does not exist as a file: $full"
    }
    if ([System.IO.Path]::GetExtension($full) -ine ".exe") {
        throw "Only a QuillForge .exe artifact may be installed: $full"
    }
    $expected = $ExpectedSha256.Trim().ToUpperInvariant()
    if ($expected -notmatch '^[0-9A-F]{64}$') {
        throw "ExpectedSha256 must be a 64-character SHA-256 digest."
    }
    $item = Get-Item -LiteralPath $full
    $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $full).Hash.ToUpperInvariant()
    if ($actual -ne $expected) {
        throw "Artifact SHA-256 mismatch for $full. Expected $expected, got $actual."
    }
    return [pscustomobject]@{
        path = $full
        bytes = [int64]$item.Length
        sha256 = $actual
    }
}

function Get-QfStatePath {
    param([Parameter(Mandatory = $true)][string]$InstallRoot)

    return Join-Path (Assert-QfInstallRoot $InstallRoot) "install-state.json"
}

function Write-QfState {
    param(
        [Parameter(Mandatory = $true)][string]$InstallRoot,
        [Parameter(Mandatory = $true)][hashtable]$State
    )

    $statePath = Get-QfStatePath $InstallRoot
    $temporary = "$statePath.$PID.tmp"
    try {
        $State | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $temporary -Encoding UTF8
        Move-Item -LiteralPath $temporary -Destination $statePath -Force
    } finally {
        if (Test-Path -LiteralPath $temporary -PathType Leaf) {
            Remove-Item -LiteralPath $temporary -Force -ErrorAction SilentlyContinue
        }
    }
}

function Read-QfState {
    param([Parameter(Mandatory = $true)][string]$InstallRoot)

    $root = Assert-QfInstallRoot $InstallRoot
    $statePath = Get-QfStatePath $root
    if (-not (Test-Path -LiteralPath $statePath -PathType Leaf)) {
        throw "QuillForge is not installed at $root."
    }
    try {
        $state = Get-Content -Raw -LiteralPath $statePath | ConvertFrom-Json
    } catch {
        throw "QuillForge install state is unreadable: $statePath"
    }
    if ($state.schema_version -ne $script:QfStateSchemaVersion) {
        throw "Unsupported QuillForge install-state schema: $($state.schema_version)"
    }
    if ([string]::IsNullOrWhiteSpace([string]$state.executable_path) -or
        -not (Test-QfPathWithin $root ([string]$state.executable_path))) {
        throw "Install state points outside its install root."
    }
    if (-not [string]::IsNullOrWhiteSpace([string]$state.backup_path) -and
        -not (Test-QfPathWithin $root ([string]$state.backup_path))) {
        throw "Install state rollback copy points outside its install root."
    }
    return $state
}

function Assert-QfExtension {
    param([Parameter(Mandatory = $true)][string]$Extension)

    $normalized = $Extension.Trim().ToLowerInvariant()
    if ($normalized -notmatch '^\.[a-z0-9][a-z0-9.-]{0,15}$') {
        throw "File extension is invalid or unsafe: $Extension"
    }
    return $normalized
}

function Get-QfRegistryPath {
    param([Parameter(Mandatory = $true)][string]$RelativePath)

    if ([string]::IsNullOrWhiteSpace($RelativePath) -or
        $RelativePath -match '(^|[\\/])\.\.?([\\/]|$)' -or
        $RelativePath -match '[:"]' -or
        $RelativePath.StartsWith('\\')) {
        throw "Registry path is invalid or unsafe: $RelativePath"
    }
    return "Registry::HKEY_CURRENT_USER\Software\Classes\$RelativePath"
}

function Get-QfRegistryDefault {
    param([Parameter(Mandatory = $true)][string]$Path)

    try {
        return [string](Get-ItemPropertyValue -LiteralPath $Path -Name "(default)" -ErrorAction Stop)
    } catch {
        return $null
    }
}

function Get-QfOpenCommand {
    param([Parameter(Mandatory = $true)][string]$ExecutablePath)

    return '"' + $ExecutablePath + '" "%1"'
}

function New-QfFileAssociation {
    param(
        [Parameter(Mandatory = $true)][string]$Extension,
        [Parameter(Mandatory = $true)][string]$ExecutablePath,
        [switch]$ReuseOwnedProgId
    )

    $normalized = Assert-QfExtension $Extension
    $extensionPath = Get-QfRegistryPath $normalized
    if (Test-Path -LiteralPath $extensionPath) {
        throw "Refusing to overwrite an existing user association: $normalized"
    }
    $executable = ConvertTo-QfFullPath $ExecutablePath
    if ([System.IO.Path]::GetExtension($executable) -ine ".exe") {
        throw "Only a QuillForge .exe may be registered: $executable"
    }
    $command = Get-QfOpenCommand $executable
    $progIdPath = Get-QfRegistryPath $script:QfProgId
    $progIdExists = Test-Path -LiteralPath $progIdPath
    if ($progIdExists -and -not $ReuseOwnedProgId) {
        throw "Refusing to overwrite an existing QuillForge ProgId registration."
    }
    if ($ReuseOwnedProgId -and -not $progIdExists) {
        throw "The owned QuillForge ProgId is missing while registering $normalized."
    }
    $commandPath = Join-Path $progIdPath "shell\open\command"
    $iconPath = Join-Path $progIdPath "DefaultIcon"
    $shellPath = Join-Path $progIdPath "shell"
    $openPath = Join-Path $shellPath "open"
    if ($ReuseOwnedProgId -and (Get-QfRegistryDefault $commandPath) -ne $command) {
        throw "Refusing to reuse a QuillForge ProgId with a changed open command."
    }
    $createdProgId = $false
    $createdExtension = $false
    try {
        if (-not $progIdExists) {
            New-Item -Path $progIdPath -ErrorAction Stop | Out-Null
            $createdProgId = $true
            New-Item -Path $shellPath -ErrorAction Stop | Out-Null
            New-Item -Path $openPath -ErrorAction Stop | Out-Null
            New-Item -Path $commandPath -ErrorAction Stop | Out-Null
            New-Item -Path $iconPath -ErrorAction Stop | Out-Null
            New-ItemProperty -LiteralPath $progIdPath -Name "(default)" -Value "QuillForge document" -PropertyType String -ErrorAction Stop | Out-Null
            New-ItemProperty -LiteralPath $commandPath -Name "(default)" -Value $command -PropertyType String -ErrorAction Stop | Out-Null
            New-ItemProperty -LiteralPath $iconPath -Name "(default)" -Value ('"' + $executable + '",0') -PropertyType String -ErrorAction Stop | Out-Null
        }
        New-Item -Path $extensionPath -ErrorAction Stop | Out-Null
        $createdExtension = $true
        New-ItemProperty -LiteralPath $extensionPath -Name "(default)" -Value $script:QfProgId -PropertyType String -ErrorAction Stop | Out-Null
    } catch {
        if ($createdExtension -and (Test-Path -LiteralPath $extensionPath)) {
            Remove-Item -LiteralPath $extensionPath -Recurse -Force -ErrorAction SilentlyContinue
        }
        if ($createdProgId -and (Test-Path -LiteralPath $progIdPath)) {
            Remove-Item -LiteralPath $progIdPath -Recurse -Force -ErrorAction SilentlyContinue
        }
        throw
    }
    return [ordered]@{
        extension = $normalized
        prog_id = $script:QfProgId
        command = $command
    }
}

function Remove-QfFileAssociation {
    param([Parameter(Mandatory = $true)]$Association)

    $extension = Assert-QfExtension ([string]$Association.extension)
    if ([string]$Association.prog_id -ne $script:QfProgId) {
        return $false
    }
    $extensionPath = Get-QfRegistryPath $extension
    $progIdPath = Get-QfRegistryPath $script:QfProgId
    $commandPath = Join-Path $progIdPath "shell\open\command"
    if (-not (Test-Path -LiteralPath $extensionPath)) {
        return $true
    }
    $currentExtension = Get-QfRegistryDefault $extensionPath
    $currentCommand = Get-QfRegistryDefault $commandPath
    if ($currentExtension -ne [string]$Association.prog_id -or
        $currentCommand -ne [string]$Association.command) {
        return $false
    }
    if (Test-Path -LiteralPath $extensionPath) {
        Remove-Item -LiteralPath $extensionPath -Recurse -Force
    }
    return $true
}

function Remove-QfProgId {
    param([Parameter(Mandatory = $true)]$Association)

    if ([string]$Association.prog_id -ne $script:QfProgId) {
        return $false
    }
    $progIdPath = Get-QfRegistryPath $script:QfProgId
    $commandPath = Join-Path $progIdPath "shell\open\command"
    if (-not (Test-Path -LiteralPath $progIdPath)) {
        if (@(Get-QfProgIdReferencePath).Count -gt 0) {
            return $false
        }
        return $true
    }
    $currentCommand = Get-QfRegistryDefault $commandPath
    if ($currentCommand -ne [string]$Association.command) {
        return $false
    }
    if (@(Get-QfProgIdReferencePath).Count -gt 0) {
        return $false
    }
    if (Test-Path -LiteralPath $progIdPath) {
        Remove-Item -LiteralPath $progIdPath -Recurse -Force
    }
    return $true
}

function Get-QfProgIdReferencePath {
    $classesPath = "Registry::HKEY_CURRENT_USER\Software\Classes"
    if (-not (Test-Path -LiteralPath $classesPath)) {
        return @()
    }
    $references = @()
    foreach ($key in @(Get-ChildItem -LiteralPath $classesPath -ErrorAction Stop)) {
        if ($key.PSChildName -eq $script:QfProgId) {
            continue
        }
        if ((Get-QfRegistryDefault $key.PSPath) -eq $script:QfProgId) {
            $references += $key.PSPath
        }
    }
    return $references
}

function Get-QfBackupPath {
    param([Parameter(Mandatory = $true)][string]$ExecutablePath)

    $base = "$ExecutablePath.previous"
    if (-not (Test-Path -LiteralPath $base)) {
        return $base
    }
    for ($index = 1; $index -le 20; $index++) {
        $candidate = "$base.$index"
        if (-not (Test-Path -LiteralPath $candidate)) {
            return $candidate
        }
    }
    throw "Too many retained QuillForge rollback copies near $ExecutablePath"
}

Export-ModuleMember -Function @(
    "Assert-QfExtension",
    "Assert-QfInstallRoot",
    "ConvertTo-QfFullPath",
    "Get-QfArtifactInfo",
    "Get-QfBackupPath",
    "Get-QfOpenCommand",
    "Get-QfStatePath",
    "Get-QfRegistryPath",
    "New-QfFileAssociation",
    "Read-QfState",
    "Remove-QfFileAssociation",
    "Remove-QfProgId",
    "Test-QfPathWithin",
    "Write-QfState"
)
