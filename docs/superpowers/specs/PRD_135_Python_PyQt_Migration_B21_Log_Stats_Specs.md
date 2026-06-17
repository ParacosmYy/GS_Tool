# PRD-135 B21 - Python/PyQt Log Statistics

## Scope

- Add visible/total log statistics to the Python/PyQt Serial Station log view.
- Count total TX and RX entries from controller state.
- Update visible count after direction and search filters change.

## User Story

As a Serial Station user, I can see how many log entries are visible and how many TX/RX entries exist in the session, so filtered PyQt debugging remains understandable without falling back to the legacy C++ UI.

## Acceptance Criteria

- `serialStationLogStatsLabel` exists and defaults to zero counts.
- Sending and receiving updates total, TX, RX, and visible counts.
- Direction/search filters update visible count while preserving total counts.
- Clearing the log resets all counts.
- Statistics are derived from controller entries and do not alter export/replay data.

## Non-Goals

- No byte counters in this batch.
- No rate/throughput graphs in this batch.
- No persistent statistics in profiles.
