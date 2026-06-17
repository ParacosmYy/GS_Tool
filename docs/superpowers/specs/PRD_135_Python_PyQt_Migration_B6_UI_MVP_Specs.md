# PRD-135 B6 - Python/PyQt Serial Station UI MVP Specs

## Scope

B6 turns the Python/PyQt skeleton into a local Serial Station MVP path:

- fake connect
- send text command
- inject fake received bytes
- render TX/RX log
- clear log and keep the window usable

## Non-Goals

- No real COM port open path.
- No PyInstaller packaging.
- No C++ launcher cutover.
- No CMake registration for Python files.
- No pyqtgraph or waveform view.

## Architecture

Files must stay in the Python lane:

- Runtime: `python/embeddebug/serial_station/`
- Tests: `tests/python/ui_smoke/`

Layering:

- `ui/` owns PyQt widgets and sends user intent to controller.
- `controllers/` orchestrates fake transport, dispatcher, protocol registry and logs.
- `drivers/`, `core/`, `protocols/`, `services/` remain UI-free.

The default C++ baseline remains `EmbedDebug.bat -> build/EmbedDebug.exe`.

## Acceptance

- `uv run pytest tests/python/ui_smoke/test_serial_station_mvp.py`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug-py --smoke`

## E/U/D Target

- Engineering: `E4` for the Python lane, covered by pytest-qt smoke.
- User: `U2`, local fake loop path is usable from the PyQt window.
- Device: `D2`, substitute transport verification only; no physical serial device.
