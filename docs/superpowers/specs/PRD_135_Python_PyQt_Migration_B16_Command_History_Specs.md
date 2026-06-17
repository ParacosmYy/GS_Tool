# PRD-135 B16 - Python/PyQt Command History

## Scope

- Add a Python/PyQt command history path for the Serial Station send workflow.
- Record only successfully sent commands.
- Let the UI refill the command input from prior commands without using the legacy C++ station.

## User Story

As a Serial Station user, I can resend or edit a previously sent command from the PyQt window so routine UART debugging does not require retyping frequent commands.

## Acceptance Criteria

- `SerialWorkbenchController.command_history` exposes successful commands.
- Failed sends before connection do not enter history.
- Duplicate commands move to the most recent position instead of creating repeated entries.
- `serialStationCommandHistoryCombo` exists in the PyQt UI.
- Sending a command refreshes the history combo.
- Selecting a history item fills `serialStationSendEdit`.

## Non-Goals

- No persistent command history across app restarts in this batch.
- No command grouping, favorites, or scripting.
- No C++ UI changes.
