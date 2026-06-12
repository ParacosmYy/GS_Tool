# Specs - PRD_093 Serial Station Controller Log Service Integration

## 1. Scope

Integrate `SerialLogService` into `SerialStationController` without changing UI behavior.

Allowed paths:

- `src/apps/serial_station/SerialStationController.h`
- `src/apps/serial_station/SerialStationController.cpp`
- `tests/serial_station/test_serial_station_controller_log_service.cpp`
- `tests/CMakeLists.txt`
- `docs/tracking/SCORE_TRACKING.md`

No production UI file changes in this phase.

## 2. Public Controller API

Add controller methods:

- `SerialLogService& logService()`
- `const SerialLogService& logService() const`
- `QVector<SerialLogRecord> logRecords() const`
- `QVector<SerialLogRecord> logRecords(const SerialLogFilter&) const`
- `QString logPlainText() const`
- `QString logPlainText(const SerialLogFilter&) const`
- `QString logJsonLines() const`
- `QString logJsonLines(const SerialLogFilter&) const`
- `void clearLogRecords()`

The API is for controller/service consumers and tests. UI integration remains signal-based.

## 3. Required Internal Logging Paths

Controller must append structured records for:

- invalid config validation failure: `Error`
- connect attempt summary: `System`
- send failure: `Error`
- send success: `Tx`
- receive parsed frame: `Rx`
- receive buffered partial bytes: `System`
- missing protocol receive drop: `Error`
- unknown receive event: `Error`
- protocol log event: `System`

## 4. Record Field Conventions

Use stable source names:

- `controller`
- `serial_manager`
- `codec`
- `dispatcher`
- `protocol`

For TX/RX records, payload should contain the frame/input bytes when available.

For command records, fields should include:

- `command`
- `mode`
- `bytesWritten` when send succeeds

## 5. Tests

Add QTest target `test_serial_station_controller_log_service`.

Required tests:

- send failure is stored as Error and visible in JSON Lines
- receive line is stored as Rx with payload hex
- partial receive is stored as System
- invalid connect config stores validation error
- clear removes stored records and subsequent records can be appended
- filters returned through controller preserve service behavior

Do not add new tests to the already oversized `test_serial_station_controller.cpp`.

## 6. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_station_controller_log_service EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R SerialStationControllerLogService
ctest --test-dir build --output-on-failure -R "Serial|Ascii|Custom|Modbus"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 7. Commit Plan

1. PRD/Specs/score tracking.
2. Controller integration/test/CMake/score tracking after validation.
