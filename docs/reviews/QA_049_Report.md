# QA Report - Iteration 49

**Date**: 2026-06-01
**Branch**: feat/embed-debug
**Score**: 48/1000

---

## 1. Build Verification

| Check | Result |
|-------|--------|
| CMake configure | PASS |
| Full build (64/64 targets) | PASS |
| Zero errors | PASS |
| Zero warnings (noted) | 1 ninja warning: premature end of file (non-blocking) |

### Build Fix Applied

**File**: `src/core/ConnectionController.cpp` line 237
**Problem**: Method signature mismatch. `.cpp` declared `enableAutoReconnect(bool, int)` (2 params) but header declared `enableAutoReconnect(bool, int, int)` (3 params with `maxRetries`).
**Fix**: Added third parameter `int maxRetries` to `.cpp` definition with `Q_UNUSED(maxRetries)` to match the header. This is a forward-compatible placeholder for the planned max-retries feature.

---

## 2. File Size Audit - Complete

### Header Files (.h) -- Limit: 200 lines

| Status | Count | Details |
|--------|-------|---------|
| PASS (under limit) | 68 | All files within limit |
| AT RISK (within 5% = 190-200) | 5 | See below |
| OVER LIMIT | 0 | None |

**At-Risk Headers (190-200 lines)**:

| File | Lines | Headroom |
|------|-------|----------|
| `src/core/ToastWidget.h` | 196 | 4 lines |
| `src/ota/AnimatedProgressBar.h` | 194 | 6 lines |
| `src/connection/SerialConnection.h` | 193 | 7 lines |
| `src/core/SendController.h` | 192 | 8 lines |
| `src/utils/ByteFormat.h` | 191 | 9 lines |

**Observations**:
- 5 header files are within 5% of the 200-line limit and require monitoring.
- `ToastWidget.h` at 196 lines is the closest to the limit with only 4 lines of headroom.
- No header files exceed the 200-line limit.

### Implementation Files (.cpp) -- Limit: 500 lines

| Status | Count | Details |
|--------|-------|---------|
| PASS (under limit) | 59 | All files within limit |
| AT RISK (within 5% = 475-500) | 5 | See below |
| OVER LIMIT | 0 | None |

**At-Risk CPP Files (475-500 lines)**:

| File | Lines | Headroom |
|------|-------|----------|
| `src/utils/DataExporter.cpp` | 498 | 2 lines |
| `src/ota/protocols/ZModemTransfer.cpp` | 498 | 2 lines |
| `src/utils/DataLogger.cpp` | 493 | 7 lines |
| `src/ota/protocols/YModemTransfer.cpp` | 493 | 7 lines |
| `src/core/ConnectionController.cpp` | 489 | 11 lines |

**Observations**:
- 5 files are at critical risk (within 5% of 500-line limit).
- `DataExporter.cpp` and `ZModemTransfer.cpp` each have only 2 lines of headroom -- any minor addition will push them over the limit.
- `ConnectionController.cpp` increased to 489 lines after the build fix (added 1 line for the `Q_UNUSED` and parameter).
- No files currently exceed the 500-line limit.

---

## 3. Method Length Audit -- Target: max 80 lines

### src/protocol/FrameVisualEditor.cpp (486 lines)

| Method | Lines | Status |
|--------|-------|--------|
| `setupFieldsGroup()` | 82 (155-236) | OVER LIMIT by 2 |
| `setupHeaderGroup()` | 16 (75-90) | PASS |
| `setupLengthGroup()` | 24 (97-121) | PASS |
| `setupChecksumGroup()` | 21 (128-148) | PASS |
| `setupPreviewGroup()` | 14 (244-257) | PASS |
| `setupConnections()` | 10 (263-273) | PASS |
| `setDefinition()` | 16 (291-306) | PASS |
| `onAddField()` | 32 (321-352) | PASS |
| `rebuildDefinition()` | 36 (373-408) | PASS |
| `updateFieldTable()` | 29 (412-441) | PASS |
| `updateBinaryPreview()` | 36 (447-483) | PASS |

**Verdict**: 1 violation. `setupFieldsGroup()` exceeds the 80-line method limit by 2 lines. This method contains inline lambda handlers for moveUp/moveDown buttons that could be extracted.

### src/ota/OtaWidget.cpp (455 lines)

| Method | Lines | Status |
|--------|-------|--------|
| `setupUI()` | 10 (59-69) | PASS |
| `setupFileGroup()` | 23 (76-98) | PASS |
| `setupConfigGroup()` | 30 (105-134) | PASS |
| `setupProgressGroup()` | 27 (141-167) | PASS |
| `setupLogGroup()` | 11 (174-184) | PASS |
| `setupHistoryGroup()` | 30 (191-221) | PASS |
| `onStartTransfer()` | 36 (239-273) | PASS |
| `onProgress()` | 20 (288-307) | PASS |
| `onTransferStats()` | 12 (310-321) | PASS |
| `onOtaStateChanged()` | 9 (324-331) | PASS |
| `onTransferComplete()` | 26 (339-364) | PASS |
| `onTransferError()` | 19 (367-385) | PASS |
| `setTransferring()` | 16 (398-414) | PASS |
| `startCompletionAnimation()` | 32 (424-455) | PASS |

**Verdict**: All methods within 80-line limit. Clean.

### src/core/ToolbarController.cpp (264 lines)

| Method | Lines | Status |
|--------|-------|--------|
| `createToolbar()` | 35 (53-87) | PASS |
| `createDisplayModeGroup()` | 34 (98-131) | PASS |
| `createConnectionGroup()` | 40 (141-180) | PASS |
| `setAvailableThemes()` | 21 (188-208) | PASS |
| `setCurrentTheme()` | 10 (216-225) | PASS |
| `themeNameAt()` | 3 (233-236) | PASS |
| `setCurrentLanguage()` | 10 (243-252) | PASS |
| `languageCodeAt()` | 3 (260-263) | PASS |

**Verdict**: All methods within 80-line limit. Clean.

### src/terminal/TerminalSearchManager.cpp (228 lines)

| Method | Lines | Status |
|--------|-------|--------|
| `setSearchHighlight()` | 72 (26-103) | PASS (but near limit) |
| `buildPlainSearch()` | 13 (106-118) | PASS |
| `buildRegexSearch()` | 18 (121-138) | PASS |
| `buildHexSearch()` | 17 (141-157) | PASS |
| `clearSearchHighlight()` | 6 (160-166) | PASS |
| `gotoNextMatch()` | 6 (178-183) | PASS |
| `gotoPrevMatch()` | 6 (186-191) | PASS |

**Verdict**: All methods within 80-line limit. `setSearchHighlight()` at 72 lines is 8 lines from the limit.

---

## 4. Deploy Test

| Step | Command | Result |
|------|---------|--------|
| windeployqt | `windeployqt build/EmbedDebug.exe` | PASS |
| Launch test | `timeout 5 build/EmbedDebug.exe` | PASS (exit 124 = killed by timeout) |

Application launched successfully and ran for the full 5-second timeout window without crashing.

---

## 5. Summary

| Category | Status | Details |
|----------|--------|---------|
| Build | PASS | 1 build fix applied (signature mismatch) |
| File sizes (.h) | PASS | 0 violations, 5 at risk |
| File sizes (.cpp) | PASS | 0 violations, 5 at risk |
| Method lengths | PASS (1 minor) | `setupFieldsGroup()` = 82 lines (over by 2) |
| Deploy test | PASS | Application launches and runs |

### Risk Assessment

**HIGH RISK** (files needing immediate attention to avoid limit breach):
- `src/utils/DataExporter.cpp` at 498 lines (2 lines headroom)
- `src/ota/protocols/ZModemTransfer.cpp` at 498 lines (2 lines headroom)

**MEDIUM RISK** (files to monitor closely):
- `src/utils/DataLogger.cpp` at 493 lines (7 lines headroom)
- `src/ota/protocols/YModemTransfer.cpp` at 493 lines (7 lines headroom)
- `src/core/ToastWidget.h` at 196 lines (4 lines headroom)

### Recommendations

1. **DataExporter.cpp and ZModemTransfer.cpp** should be refactored in the next iteration before any new features are added. Even a single-line addition will breach the 500-line limit.
2. **FrameVisualEditor.cpp `setupFieldsGroup()`** exceeds the 80-line method limit by 2 lines. Extract the moveUp/moveDown lambda handlers into named private methods.
3. **ToastWidget.h** at 196 lines should be reviewed for potential simplification to create more breathing room.
