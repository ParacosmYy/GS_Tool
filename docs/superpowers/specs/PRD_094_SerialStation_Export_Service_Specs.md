# Specs - PRD_094 Serial Station Export Service

## 1. Scope

Implement a service-layer exporter for Serial Station logs.

Allowed paths:

- `src/apps/serial_station/services/SerialExportService.h`
- `src/apps/serial_station/services/SerialExportService.cpp`
- `tests/serial_station/test_serial_export_service.cpp`
- `CMakeLists.txt`
- `tests/CMakeLists.txt`
- `docs/tracking/SCORE_TRACKING.md`

No UI/controller/core/protocol changes in this phase.

## 2. Public API

`SerialExportService` must provide:

- `enum class SerialExportFormat`
- `struct SerialExportRequest`
- `struct SerialExportResult`
- `QString formatRecords(const QVector<SerialLogRecord>&, SerialExportFormat) const`
- `SerialExportResult exportRecords(const QVector<SerialLogRecord>&, const SerialExportRequest&) const`
- `QString formatName(SerialExportFormat) const`
- `QString defaultSuffix(SerialExportFormat) const`

Required formats:

- PlainText
- JsonLines
- Csv

## 3. Result Contract

`SerialExportResult` fields:

- `bool ok`
- `QString filePath`
- `QString format`
- `qint64 bytesWritten`
- `QString errorMessage`

Failures must not leave partial success state.

## 4. Behavior

- Empty records are invalid for file export.
- Empty file path is invalid.
- Parent directory must exist.
- File writes use `QSaveFile` for atomic replacement.
- UTF-8 is the only encoding.
- Optional UTF-8 BOM is supported by request flag.
- Plain Text and JSON Lines may delegate to `SerialLogService` formatting.
- CSV must escape fields according to standard quote-doubling rules.

## 5. Tests

Add QTest target `test_serial_export_service`.

Required cases:

- plain text formatting contains direction/source/text/payload
- JSON Lines parses as one JSON object per line
- CSV header and rows are stable
- CSV escapes comma, quote and newline
- export writes UTF-8 file and reports byte count
- BOM option writes UTF-8 BOM
- empty path fails
- empty records fail
- missing parent directory fails
- unsupported format returns clear failure

## 6. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_export_service EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R SerialExportService
ctest --test-dir build --output-on-failure -R "Serial|Ascii|Custom|Modbus"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 7. Commit Plan

1. PRD/Specs/score tracking.
2. Production service/test/CMake/score tracking after validation.
