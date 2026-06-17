# PRD-135 B18 - Python/PyQt Workbench Shortcuts

## Scope

- Add keyboard shortcuts to the Python/PyQt Serial Station workbench.
- Route shortcuts through existing UI intent methods.
- Keep the default `uv run start-embeddebug` Python/PyQt lane unchanged.

## User Story

As a Serial Station user, I can use common keyboard shortcuts in the PyQt workbench so frequent UART debugging actions do not require the legacy C++ UI or repetitive mouse clicks.

## Acceptance Criteria

- `Ctrl+Enter` sends the current command text.
- `Ctrl+L` clears the serial log.
- `Ctrl+R` refreshes the serial port list.
- Shortcut tooltips are discoverable on the relevant controls.
- The implementation stays in `python/embeddebug/serial_station/ui/` and uses existing controller-facing methods.

## Non-Goals

- No global command palette in this batch.
- No user-editable shortcut map.
- No C++ shortcut changes.
