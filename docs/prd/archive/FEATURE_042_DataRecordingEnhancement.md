# FEATURE-042C: Data Recording Enhancement -- Bookmarks, Time-Range Export, Multi-Stream Sync

## Motivation

The DataLogger (EDL format) and RecordingController are implemented and functional,
supporting basic record/playback with variable speed. However, the current system has
three critical gaps that limit its usefulness for real debugging workflows:

1. **No bookmarks**: When reviewing a 10-minute recording, users cannot mark "the
   interesting part" and jump back to it. They must replay linearly or guess at
   progress percentages.
2. **No time-range export**: Users can export all data to TXT/CSV/BIN, but cannot
   export only the segment between bookmark A and bookmark B. For a 50MB recording
   of which 3 seconds are relevant, this is a showstopper.
3. **No seek/scrub**: Playback offers only progress percentage, not a timestamp
   display or direct seek-to-time control. Users cannot answer "what happened at
   02:34?" without trial-and-error playback speed adjustment.

These three capabilities form the minimum viable recording tool for embedded debugging,
and their absence means users fall back to external tools (PulseView, Wireshark) for
even basic post-capture analysis.

## User Story

As an embedded developer who has recorded a 5-minute serial session with an
intermittent bug, I want to place bookmarks at moments when the bug manifests, then
export only the data between two bookmarks to a CSV file for analysis in a
spreadsheet, so that I can isolate the failure without manually trimming a large
binary file.

## Technical Approach

### Modules Affected
- `utils/DataLogger.h/cpp` -- add bookmark storage, seek-to-time, range export
- `core/RecordingController.h/cpp` -- add bookmark button, timestamp display, seek slider
- `utils/DataExporter.h/cpp` -- add time-range export overload
- New file: `utils/DataBookmark.h` -- bookmark data structure (timestamp + label + direction)
- EDL format: v2 header with bookmark count field (backward compatible)

### Implementation Strategy

**Phase 1: Bookmark Infrastructure**
1. Extend EDL format to v2: same header structure, append bookmark records after data
   records. Each bookmark: `{magic 'B', uint64 timestamp, uint16 labelLength, label}`.
   Old readers skip unknown records; new readers handle both v1 and v2.
2. `DataLogger` gains: `addBookmark(label)`, `bookmarks()`, `removeBookmark(index)`.
   Bookmarks are written immediately to the file (append-only).
3. During playback, `DataLogger` emits `bookmarksLoaded()` so UI can populate list.

**Phase 2: Seek & Timestamp Display**
1. RecordingController adds a `QSlider` showing playback position with a `QLabel`
   showing current timestamp (MM:SS.mmm format).
2. Slider drag calls `DataLogger::seekTo(timestamp)` which binary-searches the EDL
   file for the nearest record and resumes playback from that point.
3. Bookmark list in UI: clicking a bookmark seeks to its timestamp.

**Phase 3: Range Export**
1. `DataExporter` gains `exportRange(filePath, format, startTime, endTime)` overload.
2. DataLogger provides `iterateRange(startMs, endMs, callback)` that reads records
   in the specified time window without loading the entire file into memory.
3. RecordingController adds "Export Range" button that opens a dialog with start/end
   time pickers (pre-populated from selected bookmarks or manual input).

### Complexity Estimate
- Medium. ~350 lines of new/modified code. The EDL format change is the most
  delicate part (backward compatibility), but the append-only bookmark design
  minimizes risk.

## Architecture Impact

### New Classes
| Class | Layer | Responsibility |
|-------|-------|---------------|
| `DataBookmark` | Data | Value object: timestamp + label + direction filter |

### Pattern Changes
- No new design patterns. DataLogger extends its existing record/playback state machine.
- DataExporter gains a second export method (overloaded), no pattern change.

### Existing Component Reuse
- `DataLogger` -- extended in place, no replacement
- `RecordingController` -- UI additions only
- `DataExporter` -- add range export method
- `SettingsManager` -- bookmark persistence hint
- `ByteFormat` -- timestamp formatting

## Development Cost

| Item | Estimate |
|------|----------|
| DataBookmark struct | ~25 lines |
| DataLogger bookmark + seek | ~120 lines |
| RecordingController UI additions | ~80 lines |
| DataExporter range export | ~60 lines |
| EDL v2 format changes | ~40 lines |
| QSS + theme additions | ~15 lines |
| **Total** | **~340 lines** |
| **Iterations** | **1-2 iterations** |

## Priority Justification

P1 from the candidate pool ("Data Recording Enhancement"). The DataLogger already
exists but is a skeleton without analytical value -- you can record and replay, but
you cannot navigate or extract useful information from the recording. Bookmarks and
range export transform it from a demo feature into a professional debugging tool.

This is higher priority than it might appear because:
1. It requires no new external dependencies.
2. It completes an existing feature rather than starting a new one.
3. The recording workflow (capture, bookmark, export range) is the exact workflow
   PulseView users follow, making EmbedDebug a credible alternative for serial
   protocol debugging.
4. The EDL v2 format extension is low-risk (append-only bookmarks, backward compatible)
   and sets up the foundation for future multi-stream sync (serial + RTT alignment),
   which is explicitly listed as a P1 candidate.
