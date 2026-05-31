# CODE REVIEW 043 - P0 Bug Fixes + DataBookmark

**Date**: 2026-06-01 | **Verdict**: REJECT -- 3 critical issues

## File Size Compliance

| File | Lines | Limit | Status |
|------|-------|-------|--------|
| TerminalWidget.cpp | 539 | 500 | FAIL |
| AnimatedProgressBar.h | 188 | 200 | PASS |
| XModemTransfer.cpp | 405 | 500 | PASS |
| ConnectionController.cpp | 455 | 500 | PASS |
| TerminalModel.cpp | 182 | 500 | PASS |
| DataBookmark.h | 96 | 200 | PASS |
| DataLogger.h | 131 | 200 | PASS |
| DataLogger.cpp | 333 | 500 | PASS |

## Critical Issues (blocks commit)

### C1: AnimatedProgressBar.h:109 -- stopShimmer use-after-free

`stopShimmer()` calls `m_shimmerAnim->stop()` but no longer nulls the pointer, relying on Qt's async `DeleteWhenStopped`. If `stopShimmer()` is called twice in quick succession (rapid start/stop clicks), the second call will dereference a deleted object. Fix: use `deleteLater()` + nullptr in both `startShimmer()` and `stopShimmer()`.

### C2: XModemTransfer.cpp:173 -- NAK mode downgrade removed (regression)

The old `WaitingForStart` handler downgraded `m_mode = Checksum` on NAK receipt per XMODEM protocol. The fix removed this entirely. Now when a Checksum-only receiver sends NAK, the sender still uses CRC16 checksums (`buildBlock` checks `m_mode == Checksum`), causing repeated NAKs and transfer failure. The header comment at XModemTransfer.h:41 contradicts the new code. Restore the downgrade in WaitingForStart; if the original bug was in SendingBlock, fix it there.

### C3: DataLogger.cpp -- bookmark methods declared but not implemented

`DataLogger.h` declares `addBookmark()`, `removeBookmark()`, `clearBookmarks()`, `bookmarks()`, plus `m_bookmarks` member and `bookmarksChanged()` signal. DataLogger.cpp has zero implementations. This will produce linker errors. Either implement all 4 methods or remove declarations.

## Bug Fix Assessment

| Fix | File:Line | Root cause resolved? | Notes |
|-----|-----------|---------------------|-------|
| Ctrl+C modifier | TerminalWidget.cpp:389 | YES | `== Qt::ControlModifier` exact match correct |
| Search highlight offset | TerminalWidget.cpp:234-260 | YES | Prefix subtracted from startCol before pixel calc |
| stopShimmer race | AnimatedProgressBar.h:109 | NO | New use-after-free risk (C1) |
| NAK CRC mode | XModemTransfer.cpp:173 | NO | Protocol regression (C2) |
| Network reconnect params | ConnectionController.cpp:215 | YES | m_lastConnectType now saved for network |
| setMaxLines re-entrancy | TerminalModel.cpp:127-156 | YES | Mutex held through entire resize |

## DataBookmark Review

DataBookmark.h: clean struct with `toJson()`/`fromJson()`, comparison operators, proper naming (PascalCase struct, camelCase members). No QObject dependency -- correct for data layer. Note: verify `QJsonValue::toInteger()` compiles on Qt 6.8.3; consider adding explicit `operator!=`.

Include order violation: `#include "utils/DataBookmark.h"` appears before Qt headers in DataLogger.h -- per CLAUDE.md 5.3, Qt must come first.

## Major Issues

M1: TerminalWidget.cpp at 539 lines exceeds 500-line limit. Extract search highlight painting from `paintLine()` into `TerminalSearchManager`.

M2: Include order violations in DataLogger.h, DataLogger.cpp, TerminalWidget.cpp, XModemTransfer.cpp -- project headers appear before Qt headers.

## Required Actions

1. Fix C1: `deleteLater()` + nullptr pattern for shimmer animation
2. Fix C2: Restore NAK-to-Checksum downgrade in WaitingForStart
3. Fix C3: Implement or remove bookmark method declarations
4. Fix M1: Extract search painting to reduce TerminalWidget.cpp below 500 lines
5. Fix M2/m2: Reorder includes: Qt -> STL -> project across all files
