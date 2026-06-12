# Specs - PRD_092 Serial Station Log Service

## 1. Scope

Implement a service-layer log store for Serial Station.

Allowed paths:

- `src/apps/serial_station/services/SerialLogService.h`
- `src/apps/serial_station/services/SerialLogService.cpp`
- `tests/serial_station/test_serial_log_service.cpp`
- `CMakeLists.txt`
- `tests/CMakeLists.txt`
- `docs/tracking/SCORE_TRACKING.md`

No UI integration in this phase.

## 2. Public API Requirements

`SerialLogService` must provide:

- `append(const SerialLogRecord&)`
- convenience append methods for TX/RX/System/Error
- `clear()`
- `count()`
- `isEmpty()`
- `records()`
- `records(const SerialLogFilter&)`
- `setMaxRecords(int)`
- `maxRecords()`
- `toPlainText(...)`
- `toJsonLines(...)`

The service must be usable without `QApplication`.

## 3. Data Model

`SerialLogRecord` fields:

- timestamp
- direction
- source
- text
- payload
- fields

`SerialLogFilter` fields:

- accepted directions
- source contains
- text contains
- from timestamp
- to timestamp

Direction names must be stable:

- `rx`
- `tx`
- `system`
- `error`

## 4. Behavior

- Empty timestamp must be normalized to current UTC/local `QDateTime`.
- Whitespace-only text with empty payload must not be appended.
- Payload hex must use uppercase bytes separated by one space.
- Max record count must clamp to at least 1.
- Reducing max record count immediately evicts oldest records.
- Filtering must preserve original order.
- JSON Lines must escape strings through Qt JSON APIs, not manual concatenation.

## 5. Tests

Add QTest cases:

- appending records preserves order and directions
- convenience methods normalize records
- max record count evicts oldest records
- filtering by direction/source/text/time works
- plain text formatting is stable enough for export preview
- JSON Lines contains stable keys and hex payload
- clear removes all records

## 6. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_log_service EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R SerialLogService
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 7. Commit Rule

This feature must be committed in two phases:

1. PRD/Specs/score tracking.
2. Production code/test/CMake/score tracking after validation.
