# PRD-135 B11 - Python Real Serial Connect Specs

## Scope

Move the Python/PyQt Serial Station from fake-only interaction toward a real
UART user path by exposing serial port enumeration and selected-port connection
through the existing controller boundary.

## Requirements

- The controller exposes available serial port names without requiring UI code
  to import or instantiate `QSerialPort`.
- The controller can switch from fake loopback transport to a `QSerialPort`
  transport created by a factory and open it with a selected port and baud rate.
- The PyQt window shows a serial port selector, baud-rate selector, and
  `Connect Serial` action with stable `objectName` values.
- The UI still supports the existing fake loopback path for D2 substitute
  verification.
- Profiles save the active transport mode, port name, baud rate, connection
  state, and active protocol.

## Non-Goals

- No claim of real-device D4 validation.
- No virtual COM pair automation yet.
- No advanced serial parameters beyond port name and baud rate.

## Verification

```powershell
uv run pytest tests/python/unit/test_workbench_controller.py
uv run pytest tests/python/ui_smoke/test_serial_station_mvp.py
uv run test-embeddebug-py
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
