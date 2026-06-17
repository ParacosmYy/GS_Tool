# PRD-135 B14 - Python Profile Restore Specs

## Scope

Close the Python/PyQt Serial Station profile loop by restoring saved serial
settings back into visible UI controls after loading a profile.

## Requirements

- Loading a profile restores the active protocol selector.
- Loading a serial profile restores port name, baud rate, data bits, parity, and
  stop bits into the corresponding controls.
- If a saved port or baud rate is not currently present in the selector, the UI
  adds that value before selecting it so historical profiles remain usable.
- File I/O remains in the service/controller path; UI only applies already
  loaded profile values to controls.

## Non-Goals

- No automatic connection on profile load.
- No D4 real-device validation claim.
- No profile-driven waveform or layout restoration yet.

## Verification

```powershell
uv run pytest tests/python/ui_smoke/test_serial_station_workflow.py
uv run test-embeddebug-py
uv run start-embeddebug --smoke
cmd /c EmbedDebug.bat --smoke
```
