# PRD-135 B17 - Profile Command History Restore

## Scope

- Persist Python/PyQt Serial Station command history into profile JSON.
- Restore command history when loading a profile.
- Refresh the PyQt command history selector after profile load.

## User Story

As a Serial Station user, I can save a profile with frequently used commands and restore them later in the PyQt application, so repeated device debugging sessions do not depend on the legacy C++ station or manual retyping.

## Acceptance Criteria

- Saved profiles contain `commandHistory` as an ordered list of successful commands.
- Loading a profile restores `SerialWorkbenchController.command_history`.
- Loading a profile refreshes `serialStationCommandHistoryCombo`.
- Selecting a restored history item fills `serialStationSendEdit`.
- Missing or malformed `commandHistory` remains backward compatible and behaves as an empty history.

## Non-Goals

- No global command history database outside profiles.
- No command favorites or grouping.
- No C++ profile migration in this batch.
