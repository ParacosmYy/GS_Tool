# Specs - PRD_098 Serial Station UART Basic Usability

## 1. Scope

Improve the basic UART usability of the Serial Station port panel.

Allowed paths:

- `src/apps/serial_station/ui/SerialPortPanel.h`
- `src/apps/serial_station/ui/SerialPortPanel.cpp`
- `tests/serial_station/test_serial_port_panel.cpp`
- `README.md`
- `docs/tracking/SCORE_TRACKING.md`

No controller/core/protocols/services/workers changes unless a value-propagation bug is proven first.

## 2. UI Requirements

Add a summary label:

- objectName: `serialUartSummaryLabel`
- text: current `SerialPortConfig::summary()`
- visible near the status label

Port Combo behavior:

- listed ports display a friendly label using port name plus optional description/manufacturer/serial/VID/PID information
- item data stores the actual port name
- `currentConfig().portName` uses item data when available
- manual editable input remains supported when no ports exist or when tests set editable mode

Signal behavior:

- `refreshRequested()` still emits after a refresh
- summary updates when any UART field changes
- `connectRequested(config)` emits the current normalized user-visible config

## 3. Tests

Extend `test_serial_port_panel`.

Required cases:

- default summary shows 115200 8N1
- connect signal uses item data as port name when display text differs
- manual editable input still works
- baud change updates summary
- frame format change updates summary
- flow control change updates summary
- DTR/RTS changes update summary
- refresh button emits `refreshRequested`
- closed state enables UART controls
- open state disables UART controls

Tests must not require physical serial ports.

## 4. Validation Commands

```powershell
$env:PATH='C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CMAKE_PREFIX_PATH='C:/msys64/mingw64'
cmake --build build --target test_serial_port_panel test_serial_station_workbench EmbedDebug -j 4
ctest --test-dir build --output-on-failure -R "SerialPortPanel|SerialStationWorkbench|SerialStationConfig"
ctest --test-dir build --output-on-failure -R "Serial|Ascii|Custom|Modbus"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
```

## 5. Commit Plan

1. PRD/Specs/score tracking.
2. UI/test/README/score tracking after validation.
