# Code Review 048 -- DTR/RTS Buttons + EDL Range Export

**Reviewer**: Code Reviewer (self-review agent)
**Date**: 2026-06-01
**Score**: 47/1000, iteration 48
**Branch**: feat/embed-debug

---

## Files Reviewed

| File | Lines | Limit | Status |
|------|-------|-------|--------|
| `src/serial/SerialConfigPanel.h` | 109 | 200 | OK |
| `src/serial/SerialConfigPanel.cpp` | 434 | 500 | OK |
| `src/utils/DataExporter.h` | 165 | 200 | OK |
| `src/utils/DataExporter.cpp` | 498 | 500 | OK |

All files within line limits.

---

## 1. DTR/RTS Signal Wiring (SerialConfigPanel)

**Verdict: PASS with minor observations**

Correct patterns observed:
- `m_dtrBtn` and `m_rtsBtn` are `setCheckable(true)` with `setChecked(true)` default, matching `m_dtrState = true` / `m_rtsState = true` initial state.
- Signal wiring uses `&QPushButton::toggled` (not `&QPushButton::clicked`), which is correct for checkable buttons -- toggled fires on every state change including programmatic ones.
- The lambdas update `m_dtrState`/`m_rtsState`, button text, visual style, and emit `dtrChanged`/`rtsChanged` signals in a consistent order.
- `restoreConfig()` correctly uses `blockSignals(true)` before `setChecked()` and `blockSignals(false)` after, preventing spurious signal emission during config restoration.
- `setConnected()` correctly enables/disables DTR/RTS buttons based on connection state (`m_dtrBtn->setEnabled(connected)`, line 222-223).
- `dtrEnabled()` and `rtsEnabled()` accessors return the `m_dtrState`/`m_rtsState` bools (not the button check state), which is the right source of truth.

Observation (non-blocking):
- `restoreConfig()` sets `m_dtrState`/`m_rtsState` and calls `setChecked()` to sync the button, but if the button is disabled (not connected), the visual state won't reflect the restored value until `setConnected(true)` is called. This is acceptable since buttons are only visible when connected.

---

## 2. EDL Binary Parsing Robustness (DataExporter::readEdlRange)

**Verdict: PASS with one concern**

Correct patterns observed:
- Magic validation: reads exactly 3 bytes, checks `magic.size() != 3` before comparison. Good -- handles short reads.
- Version check: reads 1 byte and validates against `kEdlVersion`. Good.
- Header skip: `file.seek(kEdlHeaderSize)` after reading magic+version, ensuring consistent positioning.
- Record parsing: checks `stream.status() != QDataStream::Ok` after each field read (timestamp, direction, length). Any stream error breaks the loop.
- Record size guard: `if (length > kEdlMaxRecordSize) break;` prevents OOM on corrupted files (1MB cap defined as `kEdlMaxRecordSize`).
- Raw data read: `stream.readRawData(...)` return value checked against requested length.
- Early termination optimization: `if (toMs >= 0 && tsMs > toMs) break;` stops scanning once past the upper bound. This is correct assuming records are monotonically ordered by timestamp.
- `fromMs` skip: `if (fromMs >= 0 && tsMs < fromMs) continue;` correctly skips records before the lower bound without breaking.

Concern:
- **Monotonic timestamp assumption**: The early termination on `tsMs > toMs` assumes EDL records are written in monotonically increasing timestamp order. If the DataLogger ever produces out-of-order records (e.g., due to thread scheduling, pause/resume offset bugs, or file corruption), records after the first `tsMs > toMs` hit would be silently lost. The DataLogger uses `QElapsedTimer::elapsed()` which is monotonic, so this assumption holds for well-formed files. The risk is low but worth noting for defensive robustness.

EDL format consistency verified:
- `DataExporter` constants: `kEdlMagic = "EDL"`, `kEdlVersion = 1`, `kEdlHeaderSize = 8`
- `DataLogger` constants: `kMagic = "EDL"`, `kVersion = 1`, `kHeaderSize = 8`
- Record layout match: `DataLogger::RecordHeader` is `timestamp(quint64) + direction(quint8) + length(quint32)`, which matches `readEdlRange`'s `stream >> timestamp >> direction >> length` sequence.
- Direction encoding match: DataLogger uses `Direction::Received = 0` / `Direction::Sent = 1`; readEdlRange maps `direction == 0` to `DataDirection::Rx` and else to `DataDirection::Tx`. Consistent.

---

## 3. Memory Leaks and Ownership

**Verdict: PASS**

- All `new` calls in SerialConfigPanel pass `this` or a child layout widget as parent, falling under Qt's parent-child tree. No orphan allocations.
- `QGraphicsOpacityEffect` is created with `m_statusIndicator` as parent (line 276). The `delete m_breathAnim` at line 281 before creating a new one prevents leaks on repeated `setConnecting()` calls.
- `readEdlRange` uses stack-allocated `QFile` and `QDataStream`; `QVector<TerminalLine>` result is returned by value (RVO/elided).
- `exportRange` delegates to existing `exportPlain/exportHexDump/...` methods which all open/close `QFile` properly within scope. No file handle leaks.

---

## 4. Include Path Format

| File | Issue? |
|------|--------|
| `SerialConfigPanel.cpp` | **YES** -- uses `"SerialConfigPanel.h"` and `"SerialDriverDetector.h"` without module prefix |
| `DataExporter.cpp` | OK -- uses `"utils/DataExporter.h"`, `"utils/HexConverter.h"`, `"core/Constants.h"` |
| `DataExporter.h` | OK -- uses `"terminal/TerminalTypes.h"` |
| `SerialConfigPanel.h` | OK -- no project includes (only `<QWidget>`, etc.) |

**Finding**: SerialConfigPanel.cpp violates the CLAUDE.md rule: "Include paths use `module/File.h` format." It should be `"serial/SerialConfigPanel.h"` and `"serial/SerialDriverDetector.h"`. This is a formatting issue that should be corrected.

---

## 5. Summary Checklist

```
[PASS] DTR/RTS signals correctly wired: checkable buttons, toggled signal,
       blockSignals on restore, enabled/disabled by connection state
[PASS] EDL binary parsing: magic+version validation, stream status checks,
       record size cap (1MB), early termination optimization
[PASS] No memory leaks: Qt parent-child tree, stack-allocated file objects,
       animation lifecycle managed correctly
[PASS] File size limits: all files within .h <= 200 / .cpp <= 500 bounds
[WARN] Include path format: SerialConfigPanel.cpp uses bare filenames
       instead of "serial/File.h" format
[INFO] Monotonic timestamp assumption in readEdlRange early termination --
       safe for well-formed EDL files, fragile against corruption
```

---

## 6. Recommended Actions

1. **[Minor, should fix]** SerialConfigPanel.cpp includes: change `"SerialConfigPanel.h"` to `"serial/SerialConfigPanel.h"` and `"SerialDriverDetector.h"` to `"serial/SerialDriverDetector.h"` to comply with project conventions.

2. **[Optional, defensive]** Consider adding a `QDataStream::ReadPastEnd` check after the while loop in `readEdlRange` to distinguish "clean EOF" from "truncated record" -- currently both silently return whatever was accumulated.

3. **[No action needed]** DTR/RTS default state (HIGH) is a reasonable default for most USB-serial adapters. The tooltip text correctly documents the ESP32/STM32 reset/bootloader use case where users need to toggle LOW.
