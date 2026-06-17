# PRD-135 B13 - Python Serial Frame Settings Specs

## Scope

Extend the Python/PyQt real-UART path beyond port and baud rate by carrying
serial frame settings from the UI through the controller into `QSerialPort`.

## Requirements

- `SerialPortConfig` carries data bits, parity, and stop bits with defaults of
  `8`, `none`, and `1`.
- `QtSerialPortTransport.configure()` applies port name, baud rate, data bits,
  parity, and stop bits to the underlying `QSerialPort`.
- The controller exposes these settings as simple Python values and records them
  in saved profiles.
- The PyQt window exposes stable controls:
  - `serialStationDataBitsCombo`
  - `serialStationParityCombo`
  - `serialStationStopBitsCombo`
- UI code still calls only the controller and does not import or configure
  `QSerialPort` directly.

## Non-Goals

- No hardware D4 claim.
- No flow-control UI yet.
- No per-profile auto-restore into the UI controls yet.

## Verification

```powershell
uv run pytest tests/python/unit/test_transports.py tests/python/unit/test_workbench_controller.py
uv run pytest tests/python/ui_smoke/test_serial_station_mvp.py
uv run test-embeddebug-py
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
