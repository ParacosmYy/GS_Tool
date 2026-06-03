$ErrorActionPreference = 'Stop'
Set-Location E:\Embedded\Tool\Serial_tool\User_Serial

# Fix CMakeLists.txt - add interfaces entries
$c = [System.IO.File]::ReadAllText((Join-Path (Get-Location) 'CMakeLists.txt'))
$old = "set(HEADERS`r`n    src/core/mainwindow/MainWindow.h"
$new = "set(HEADERS`r`n    src/interfaces/IPanelProvider.h`r`n    src/interfaces/IDataSink.h`r`n    src/interfaces/IProtocolParser.h`r`n    src/interfaces/IDevice.h`r`n`r`n    src/core/mainwindow/MainWindow.h"
$c = $c.Replace($old, $new)
[System.IO.File]::WriteAllText((Join-Path (Get-Location) 'CMakeLists.txt'), $c, (New-Object System.Text.UTF8Encoding $false))
Write-Output "CMakeLists.txt updated"

# Verify
$lines = [System.IO.File]::ReadAllLines((Join-Path (Get-Location) 'CMakeLists.txt'))
foreach ($line in $lines) {
    if ($line -match 'interfaces') {
        Write-Output "  FOUND: $line"
    }
}
