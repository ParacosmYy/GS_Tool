# PRD-135 B7 - Python/PyQt User Workflow Parity Specs

## Scope

B7 extends the PyQt MVP from fake serial traffic into a user workflow:

- Save the current TX/RX session to a JSON Lines log.
- Clear the visible log.
- Replay a saved JSON Lines log back into the window.
- Save a local profile.
- Load a local profile and show its name in the UI.

## Non-Goals

- No real COM port workflow.
- No modal file dialogs.
- No pyqtgraph or waveform view.
- No C++ launcher cutover.
- No public release claim.

## Architecture

Layering remains:

- `ui/` owns PyQt widgets, path fields, buttons, and status rendering.
- `controllers/` orchestrates services and exposes UI-safe methods.
- `services/` performs file persistence.
- `protocols/`, `drivers/`, and `core/` stay UI-free.

The first B7 UI uses explicit path fields instead of dialogs so pytest-qt can verify the workflow
deterministically.

## Acceptance

- `uv run pytest tests/python/unit/test_workbench_controller.py`
- `uv run pytest tests/python/ui_smoke/test_serial_station_workflow.py`
- `uv run test-embeddebug-py`
- `uv run test-embeddebug-tools`
- `uv run start-embeddebug-py --smoke`
- `uv run package-embeddebug-py --version b7-smoke --clean`
- `uv run verify-package-embeddebug-py --package-dir dist/EmbedDebugPy-b7-smoke-windows-x64`

## E/U/D Target

- Engineering: `E4`, controller and PyQt smoke tests cover the workflow.
- User: `U3`, local fake session can be saved, cleared, replayed, and profile-loaded from the UI.
- Device: `D2`, substitute serial verification only.
