# PRD-135 B20 - Python/PyQt Log Text Search

## Scope

- Add a text search filter to the Python/PyQt Serial Station log view.
- Combine text search with the existing `All` / `TX` / `RX` direction filter.
- Keep controller entries, replay data, and export data unfiltered.

## User Story

As a Serial Station user, I can search visible PyQt log entries by text so noisy UART sessions can be inspected without falling back to the legacy C++ UI or external editors.

## Acceptance Criteria

- `serialStationLogSearchEdit` exists in the PyQt UI.
- Typing text re-renders the visible log entries to matching lines only.
- Search matching is case-insensitive.
- Clearing search text restores entries allowed by the current direction filter.
- Direction filter and search filter combine predictably.

## Non-Goals

- No regex search in this batch.
- No highlight styling in this batch.
- No persistent search state in profiles.
