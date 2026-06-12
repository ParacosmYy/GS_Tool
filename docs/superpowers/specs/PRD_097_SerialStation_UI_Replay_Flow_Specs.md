# Specs - PRD_097 Serial Station UI Replay Flow

## 1. Scope

Implement a UI-visible replay preview flow for Serial Station logs.

Allowed production paths:

- `src/apps/serial_station/SerialStationController.h`
- `src/apps/serial_station/SerialStationController.cpp`
- `src/apps/serial_station/SerialStationControllerExport.cpp`
- `src/apps/serial_station/SerialStationWindow.cpp`
- `src/apps/serial_station/ui/SerialLogPanel.h`
- `src/apps/serial_station/ui/SerialLogPanel.cpp`
- `README.md`
- `CMakeLists.txt`

Allowed test/doc paths:

- `tests/serial_station/test_serial_station_replay_flow.cpp`
- `tests/serial_station/test_serial_station_workbench.cpp`
- `tests/CMakeLists.txt`
- `docs/tracking/SCORE_TRACKING.md`

No `core/`, `protocols/`, `workers/`, launch script, or theme changes.

## 2. UI Contract

`SerialLogPanel` must expose:

- `void replayRequested()` signal.
- A replay button with objectName `serialLogReplayButton`.
- Button text must use `tr("回放")`.

The panel must not include or instantiate `SerialReplayService`.

## 3. Controller Contract

`SerialStationController` must expose a user-triggerable replay preview method:

- `SerialReplayPlan previewReplayPlan(const SerialReplayOptions& options = SerialReplayOptions())`

Behavior:

- Use current `m_logService.records()`.
- Use `m_replayService.buildPlan(...)`.
- On success, emit system log with plan summary.
- On success, emit a bounded preview of event summaries.
- On failure, log an error/system message that is visible in the UI.
- Return the plan for tests and later orchestration.

The method must not send bytes to `SerialManager`.

## 4. Window Wiring

`SerialStationWindow` must connect:

```cpp
SerialLogPanel::replayRequested
  -> SerialStationController::previewReplayPlan
```

The connection may use a lambda only for UI-level text routing; replay planning must stay in Controller.

## 5. README Requirement

Because this is user-visible Serial Station functionality, update README capability copy so the enterprise landing page mentions:

- structured logging
- log export
- replay preview

Do not add screenshots or generated assets in this phase.

## 6. Tests

Add QTest target `test_serial_station_replay_flow`.

Required cases:

- controller replay fails clearly when log is empty
- controller replay succeeds after TX/RX records exist
- controller preview logs include summary and event details
- UI button exists with objectName `serialLogReplayButton`
- clicking the button routes through the window and appends replay preview feedback
- clearing logs before replay causes an empty-log failure

Existing `test_serial_station_workbench` should keep passing.

## 7. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_station_replay_flow test_serial_station_workbench EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R "SerialStationReplayFlow|SerialStationWorkbench|SerialReplayService"
ctest --test-dir build --output-on-failure -R "Serial|Ascii|Custom|Modbus"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 8. BATCH / LOOP

- BATCH: no for code edits, because controller/header/test target CMake changes are tightly coupled.
- Parallel sub agents: allowed only for read-only inspection or independent validation, not simultaneous writes to the same files.
- LOOP Doctor: run after implementation.
- LOOP Debug: use for reproducible build/test failures.
- LOOP Simplify: use if any touched file exceeds the file-size limits.

## 9. Commit Plan

1. PRD/Specs/score tracking.
2. Controller/UI/test/README/CMake/score tracking after validation.
