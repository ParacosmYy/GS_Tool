# Specs - PRD_096 Serial Station Replay Service

## 1. Scope

Implement a service-layer replay planner for Serial Station logs.

Allowed paths:

- `src/apps/serial_station/services/SerialReplayService.h`
- `src/apps/serial_station/services/SerialReplayService.cpp`
- `tests/serial_station/test_serial_replay_service.cpp`
- `CMakeLists.txt`
- `tests/CMakeLists.txt`
- `docs/tracking/SCORE_TRACKING.md`

No UI, controller, core serial transport, worker, protocol, launch script, or theme changes.

## 2. Public API

`SerialReplayService` must provide:

- `struct SerialReplayOptions`
- `struct SerialReplayEvent`
- `struct SerialReplayPlan`
- `SerialReplayPlan buildPlan(const QVector<SerialLogRecord>&, const SerialReplayOptions&) const`
- `SerialReplayPlan buildPlanFromJsonLines(const QString&, const SerialReplayOptions&) const`
- `QString eventSummary(const SerialReplayEvent&) const`
- `QString planSummary(const SerialReplayPlan&) const`

Required option fields:

- `bool includeTx = true`
- `bool includeRx = true`
- `bool includeSystem = false`
- `bool includeError = false`
- `double speedMultiplier = 1.0`
- `qint64 maxDelayMs = 5000`

## 3. Data Contract

`SerialReplayEvent` fields:

- `SerialLogDirection direction`
- `QDateTime timestamp`
- `QString source`
- `QString text`
- `QByteArray payload`
- `qint64 delayMs`

`SerialReplayPlan` fields:

- `bool ok`
- `QString errorMessage`
- `QVector<SerialReplayEvent> events`
- `qint64 totalDurationMs`
- `int skippedRecords`

Failures must have `ok == false`, non-empty `errorMessage`, no events, and zero duration.

## 4. Behavior

- Empty input fails.
- Records filtered out by options count as skipped.
- If all records are filtered out, fail with a clear message.
- Events preserve input order after filtering.
- First event delay is 0.
- Later delays use timestamp difference from previous included event.
- Negative timestamp deltas are clamped to 0.
- `speedMultiplier <= 0` is treated as 1.0.
- `maxDelayMs < 0` means no max-delay clamp.
- `maxDelayMs >= 0` clamps each delay to that value.
- `totalDurationMs` is the sum of event delays.

## 5. JSON Lines Parsing

`buildPlanFromJsonLines` must parse one JSON object per non-empty line.

Required fields:

- `direction`: `tx`, `rx`, `system`, or `error`
- `text`: string, non-empty after trim

Optional fields:

- `timestamp`: ISO date/time string
- `source`: string
- `payloadHex`: space-separated or compact hex

Invalid JSON, unknown direction, invalid payload hex, or missing required fields must fail.

## 6. Tests

Add QTest target `test_serial_replay_service`.

Required cases:

- default plan keeps TX/RX and skips System/Error
- include flags can keep all directions
- delays are computed from timestamps
- speed multiplier scales delays
- max delay clamps long gaps
- invalid speed falls back to 1.0
- negative timestamp deltas clamp to zero
- empty records fail
- all-filtered records fail
- JSON Lines from `SerialLogService` round trips into a plan
- invalid JSON line fails with line number
- missing direction/text fails
- invalid payload hex fails
- summaries include direction, delay, source, text, and duration

## 7. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_replay_service EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R SerialReplayService
ctest --test-dir build --output-on-failure -R "Serial|Ascii|Custom|Modbus"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 8. BATCH / LOOP

- BATCH: no. Single service plus one test target; shared interface is not stable enough for parallel writes.
- Parallel sub agents: not used.
- LOOP Doctor: run after implementation.
- LOOP Debug: route reproducible build/test failures here.
- LOOP Simplify: use if service or test files exceed file-size limits.

## 9. Commit Plan

1. PRD/Specs/score tracking.
2. Service/test/CMake/score tracking after validation.
