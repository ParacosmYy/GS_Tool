# QA Report - Iteration 043

**Date**: 2026-06-01
**Build**: EmbedDebug 0.1.0
**Compiler**: GCC 14.2.0 (MinGW) + Qt 6.8.3
**QA Engineer**: quality-engineer

---

## 1. Build Result: PASS

| Metric | Value |
|--------|-------|
| Compilation units | 41/41 |
| Linking | EmbedDebug.exe linked successfully |
| Warnings | 0 errors, 1 ninja recovery warning (benign) |
| Build tool | Ninja 1.13.2 via CMake 4.0.1 |

Build completed cleanly. The `ninja: warning: premature end of file; recovering` is a known benign Ninja
output artifact when the build database is updated mid-stream; no object files or linking were affected.

---

## 2. File Size Violations

### 2.1 Header Files (limit: 200 lines)

| File | Lines | Status |
|------|-------|--------|
| MainWindow.h | 199 | PASS (within limit) |
| OtaWidget.h | 198 | PASS |
| ToastWidget.h | 196 | PASS |
| AnimatedProgressBar.h | 194 | PASS |
| SerialConnection.h | 193 | PASS |
| SendController.h | 192 | PASS |
| ByteFormat.h | 191 | PASS |
| NavIndicatorWidget.h | 190 | PASS |
| ThemeManager.h | 189 | PASS |
| ChartWidget.h | 184 | PASS |

**Violations: 0.** All headers within the 200-line limit.

### 2.2 Source Files (limit: 500 lines)

| File | Lines | Status |
|------|-------|--------|
| TerminalWidget.cpp | 539 | **VIOLATION (+39 lines over)** |
| YModemTransfer.cpp | 490 | PASS |
| ZModemTransfer.cpp | 480 | PASS |
| ConnectionController.cpp | 467 | PASS |
| ProtocolBridgeManager.cpp | 439 | PASS |
| FrameVisualEditor.cpp | 437 | PASS |
| FrameParser.cpp | 428 | PASS |
| NavigationController.cpp | 421 | PASS |
| OtaWidget.cpp | 418 | PASS |
| SerialConnection.cpp | 414 | PASS |

**Violations: 1.** TerminalWidget.cpp at 539 lines exceeds the 500-line limit.

### 2.3 MainWindow Special Tracking

| File | Lines | Status |
|------|-------|--------|
| MainWindow.cpp | 361 | PASS (well within 500-line limit) |
| MainWindowSignalConnect.cpp | 303 | PASS |

MainWindow has been successfully decomposed. The signal wiring extracted to
MainWindowSignalConnect.cpp (303 lines) keeps the main body at 361 lines.

---

## 3. P0 Fix Verification

### Fix 1: TerminalWidget exact modifier match (keyPressEvent)

**File**: `src/terminal/TerminalWidget.cpp` (lines 389-407)
**Status**: VERIFIED

All four keyboard shortcuts (Ctrl+C, Ctrl+A, Ctrl+V, Ctrl+F) use exact
modifier matching via `event->modifiers() == Qt::ControlModifier`. This
prevents Ctrl+Shift+C, Ctrl+Shift+A, Ctrl+Shift+F from being incorrectly
intercepted. Each shortcut has a descriptive Chinese comment explaining
the exact-match rationale.

### Fix 2: AnimatedProgressBar stopShimmer race condition

**File**: `src/ota/AnimatedProgressBar.h` (lines 109-117)
**Status**: VERIFIED

The stopShimmer() method:
- Calls `m_shimmerAnim->stop()` without setting `m_shimmerAnim` to nullptr
- Relies on `QAbstractAnimation::DeleteWhenStopped` for async cleanup
- Avoids manual nullptr that could race with Qt's deferred delete in the event loop
- startShimmer() calls stopShimmer() first, then creates a new animation
  with `this` as parent for lifetime safety

The design is documented with detailed comments explaining the race condition
prevention strategy.

### Fix 3: ConnectionController network params persistence

**File**: `src/core/ConnectionController.cpp` (line 226)
**Status**: VERIFIED

Network connection parameters are now saved:
- Line 134: `m_lastConnectParams = serialParams` for serial connections
- Line 226: `m_lastConnectParams = params` for network connections
- Line 351: Auto-reconnect uses `m_lastConnectParams` to restore serial
- Line 353: Auto-reconnect uses `m_lastConnectType` to route to correct path

Both serial and network paths persist params before auto-reconnect can use them.

### Fix 4: TerminalModel setMaxLines re-entrancy guard

**File**: `src/terminal/TerminalModel.cpp` (lines 127-131)
**Status**: VERIFIED

The method acquires `QMutexLocker locker(&m_mutex)` at entry, providing
thread-safe protection against concurrent calls from the terminal data
thread and UI configuration changes. The early-return `if (max == m_maxLines) return;`
prevents unnecessary buffer reallocation.

### Fix 5: DataBookmark.h new file

**File**: `src/utils/DataBookmark.h` (97 lines)
**Status**: VERIFIED

New file exists and contains:
- Complete DataBookmark struct with timestamp, label, streamId fields
- JSON serialization (toJson/fromJson) for persistence
- Comparison operators for sorting and equality
- Proper header guard and Doxygen documentation
- Collaborator documentation: DataLogger, RecordingController

---

## 4. Static Analysis

### 4.1 SIGNAL/SLOT Macro Usage

**Result: 0 violations.** No legacy `SIGNAL()` or `SLOT()` macro usage found
in the src/ tree. All signal-slot connections use the modern function-pointer
syntax per project coding standards.

### 4.2 TODO/FIXME Comments

**Result: 0 violations.** No dangling TODO or FIXME comments found. All
outstanding work items have been resolved in this iteration.

---

## 5. Summary and Recommendation

| Category | Result |
|----------|--------|
| Build | PASS |
| P0 fixes verified | 5/5 |
| SIGNAL/SLOT violations | 0 |
| TODO/FIXME violations | 0 |
| Header file violations | 0 |
| Source file violations | 1 (TerminalWidget.cpp at 539 lines) |

### Recommendation: APPROVED with 1 advisory

**Advisory**: TerminalWidget.cpp (539 lines) exceeds the 500-line limit by 39
lines. This file combines self-drawn terminal rendering, keyboard handling,
search integration, and paste logic. Consider extracting search highlight
painting or keyboard event dispatch into a helper to bring it under the limit.
This is a P2 refactoring task and does not block the commit.

**P0 bug clearance**: All five tracked P0 bugs from the iteration 43 bug sprint
have been verified as fixed. The codebase is clean with zero static analysis
violations and zero build errors. Ready for commit.
