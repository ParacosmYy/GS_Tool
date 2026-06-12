# Specs - PRD_095 Serial Station UI Export Flow

## 1. Scope

Wire the existing Serial Station export service into the user-visible log export flow.

Allowed paths:

- `src/apps/serial_station/SerialStationController.h`
- `src/apps/serial_station/SerialStationController.cpp`
- `src/apps/serial_station/SerialStationWindow.cpp`
- `tests/serial_station/test_serial_station_export_flow.cpp`
- `tests/CMakeLists.txt`
- `docs/tracking/SCORE_TRACKING.md`

No protocol, core serial transport, worker, theme, or launch script changes in this phase.

## 2. Public Contract

`SerialStationController` must expose:

- `SerialExportResult exportLogRecords(const SerialExportRequest& request)`
- `QString suggestedExportFileName(SerialExportFormat format) const`

The export method must:

- take a snapshot from `SerialLogService`
- call `SerialExportService::exportRecords`
- log success through existing system log path
- log failure through existing error log path
- return the service result unchanged enough for tests and callers to inspect

## 3. UI Flow

`SerialStationWindow` must handle `SerialLogPanel::exportRequested`:

1. Build a default filename from `SerialStationController::suggestedExportFileName`.
2. Open `QFileDialog::getSaveFileName` with JSON Lines, Text, and CSV filters.
3. Treat cancel as a system log message, not an error dialog.
4. Normalize missing suffix to `.jsonl`.
5. Infer export format from file suffix.
6. Call `SerialStationController::exportLogRecords`.

The window may open the dialog because that is UI behavior. It must not write the file.

## 4. Format Mapping

- `.jsonl` and `.json` -> `SerialExportFormat::JsonLines`
- `.txt` and `.log` -> `SerialExportFormat::PlainText`
- `.csv` -> `SerialExportFormat::Csv`
- no suffix -> append `.jsonl` and use JSON Lines
- unknown suffix -> keep path and use JSON Lines

## 5. Tests

Add QTest target `test_serial_station_export_flow`.

Required cases:

- controller exports existing log records to JSON Lines
- controller export success emits a system log
- empty log export fails and emits an error log
- suffix-independent service result remains inspectable by caller
- suggested file names use the requested format suffix
- CSV export through controller writes the CSV header and rows
- failure does not create a target file for missing parent directory

Tests must not instantiate a native file dialog and must not depend on real serial ports.

## 6. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_station_export_flow EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R SerialStationExportFlow
ctest --test-dir build --output-on-failure -R "Serial|Ascii|Custom|Modbus"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 7. BATCH / LOOP

- BATCH: no. This is a small cross-layer wiring task with shared controller/window files.
- Parallel sub agents: not used.
- LOOP Doctor: run after implementation for environment and launch verification.
- LOOP Debug: only if a test or build failure is reproducible.
- LOOP Simplify: use if controller/window line counts exceed target thresholds.

## 8. Commit Plan

1. PRD/Specs/score tracking.
2. Controller/UI/test/CMake/score tracking after validation.
