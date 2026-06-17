# PRD-135 B12 - Python Serial Port Refresh Specs

## Scope

Improve the Python/PyQt Serial Station real-UART path by allowing users to
refresh available serial ports from the main window after plugging or removing
devices.

## Requirements

- The PyQt window exposes a `Refresh Ports` action with a stable
  `serialStationRefreshPortsButton` object name.
- Refreshing ports re-queries the controller, updates `serialStationPortCombo`,
  preserves the previous selection when it still exists, and shows a visible
  status message.
- `Connect Serial` remains disabled when no serial ports are available and is
  enabled after refresh finds at least one port.
- UI code continues to call only the controller for port enumeration; it does
  not directly instantiate or configure `QSerialPort`.

## Non-Goals

- No D4 real-device validation claim.
- No virtual COM pair automation.
- No advanced serial settings beyond the existing port and baud selectors.

## Verification

```powershell
uv run pytest tests/python/ui_smoke/test_serial_station_mvp.py
uv run test-embeddebug-py
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
