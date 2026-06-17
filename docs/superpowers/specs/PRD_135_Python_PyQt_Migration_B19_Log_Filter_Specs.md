# PRD-135 B19 - Python/PyQt Log Direction Filter

## Scope

- Add a direction filter to the Python/PyQt Serial Station log view.
- Support `All`, `TX`, and `RX` without changing controller log semantics.
- Re-render visible log lines from existing controller entries when the filter changes.

## User Story

As a Serial Station user, I can filter the PyQt log by transmitted or received lines so debugging noisy UART sessions does not require the legacy C++ UI or external log tools.

## Acceptance Criteria

- `serialStationLogFilterCombo` exists in the PyQt UI and defaults to `All`.
- Selecting `TX` shows only transmitted log lines.
- Selecting `RX` shows only received log lines.
- Selecting `All` restores both transmitted and received log lines.
- Export, replay, and controller entries remain unfiltered; the filter only changes UI rendering.

## Non-Goals

- No text search in this batch.
- No regex, severity, timestamp, or channel filters.
- No C++ UI changes.
