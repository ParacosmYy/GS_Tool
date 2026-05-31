# CODE REVIEW 049 -- Iteration 48 Refactoring

**Reviewer**: Code Reviewer (self-review)
**Date**: 2026-06-01
**Score**: 48/1000
**Scope**: setupUI method extraction (6 files), shared constants, escapeZdle helper

---

## 1. File Line Limits

| File | Type | Lines | Limit | Status |
|------|------|------:|-------:|--------|
| `src/protocol/FrameVisualEditor.cpp` | .cpp | 486 | 500 | PASS (14 lines margin) |
| `src/ota/OtaWidget.cpp` | .cpp | 455 | 500 | PASS |
| `src/core/ToolbarController.cpp` | .cpp | 264 | 500 | PASS |
| `src/terminal/TerminalSearchManager.cpp` | .cpp | 228 | 500 | PASS |
| `src/ota/protocols/BaseTransfer.h` | .h | 106 | 200 | PASS |
| `src/ota/protocols/ZModemTransfer.cpp` | .cpp | 498 | 500 | PASS (2 lines margin -- DANGER) |
| `src/protocol/FrameVisualEditor.h` | .h | 149 | 200 | PASS |
| `src/ota/OtaWidget.h` | .h | 175 | 200 | PASS |
| `src/core/ToolbarController.h` | .h | 171 | 200 | PASS |
| `src/terminal/TerminalSearchManager.h` | .h | 166 | 200 | PASS |
| `src/ota/protocols/ZModemTransfer.h` | .h | 124 | 200 | PASS |

**WARNING**: ZModemTransfer.cpp at 498 lines has only 2 lines of margin before breaching the 500-line limit. Any further additions to this file will likely require another extraction round.

## 2. Extracted Method Line Counts (80-line limit)

### FrameVisualEditor.cpp -- setupUI split into 6 sub-methods

| Method | Lines | Count | Status |
|--------|-------|------:|--------|
| `setupUI()` | 51-69 | 19 | PASS |
| `setupHeaderGroup()` | 75-91 | 17 | PASS |
| `setupLengthGroup()` | 97-122 | 26 | PASS |
| `setupChecksumGroup()` | 128-149 | 22 | PASS |
| `setupFieldsGroup()` | 155-238 | **84** | **FAIL** |
| `setupPreviewGroup()` | 244-258 | 15 | PASS |
| `setupConnections()` | 263-273 | 11 | PASS |

**DEFECT**: `setupFieldsGroup()` is 84 lines, exceeding the 80-line method limit by 4 lines. The excess comes from inline lambda definitions for the moveUp/moveDown button handlers (lines 203-235). These lambdas should be extracted into named private slots (e.g., `onMoveFieldUp()`, `onMoveFieldDown()`) to bring the method under the limit.

### OtaWidget.cpp -- setupUI split into 5 sub-methods

| Method | Lines | Count | Status |
|--------|-------|------:|--------|
| `setupUI()` | 59-70 | 12 | PASS |
| `setupFileGroup()` | 76-99 | 24 | PASS |
| `setupConfigGroup()` | 105-135 | 31 | PASS |
| `setupProgressGroup()` | 141-168 | 28 | PASS |
| `setupLogGroup()` | 174-185 | 12 | PASS |
| `setupHistoryGroup()` | 191-221 | 31 | PASS |

All methods well under the limit. Clean extraction.

### ToolbarController.cpp -- createToolbar split

| Method | Lines | Count | Status |
|--------|-------|------:|--------|
| `createToolbar()` | 53-88 | 36 | PASS |
| `createDisplayModeGroup()` | 98-131 | 34 | PASS |
| `createConnectionGroup()` | 141-180 | 40 | PASS |

All methods under limit. The split leaves `createToolbar()` as a clean orchestrator: create groups + wire signals + initialize themes.

### TerminalSearchManager.cpp -- setSearchHighlight refactored

| Method | Lines | Count | Status |
|--------|-------|------:|--------|
| `setSearchHighlight()` | 26-104 | 79 | PASS (boundary) |
| `buildPlainSearch()` | 106-119 | 14 | PASS |
| `buildRegexSearch()` | 121-139 | 19 | PASS |
| `buildHexSearch()` | 141-158 | 18 | PASS |

`setSearchHighlight()` at 79 lines is at the boundary. The lineProvider lambda construction (lines 54-81) accounts for most of the length. Acceptable for now but monitor.

### ZModemTransfer.cpp -- escapeZdle helper

| Method | Lines | Count | Status |
|--------|-------|------:|--------|
| `escapeZdle()` | 485-498 | 14 | PASS |

Trivial extraction. Correctly handles 0x18, 0x0D, 0x0A, 0x11, 0x13, 0x2A per ZMODEM spec.

## 3. Signal Connection Integrity

### FrameVisualEditor.cpp
- `setupConnections()` (lines 263-273): All 5 connections intact.
  - `m_applyBtn->clicked` -> `onApply`
  - `m_addFieldBtn->clicked` -> `onAddField`
  - `m_removeFieldBtn->clicked` -> `onRemoveField`
  - `m_fieldTable->cellChanged` -> `onFieldChanged`
  - `m_fieldTable->cellChanged` -> lambda (updateBinaryPreview)
- Inline lambda in `setupFieldsGroup()` (lines 203-235): moveUp/moveDown connections defined locally. No signal breakage risk since the lambdas capture `this`.

### OtaWidget.cpp
- Constructor connections (lines 33-37): 5 OtaManager signal connections intact.
- `setupConfigGroup()` inline connections (lines 132-133): start/cancel buttons wired correctly.

### ToolbarController.cpp
- `createToolbar()` lines 65-82: All 8 signal forwarding connections intact and correctly wired.
  - `m_displayModeCombo`, `m_layoutCombo`, `m_timestampAction`, `m_dirPrefixAction`, `m_clearAction`, `m_exportAction`, `m_bgAction`, `m_themeCombo`, `m_langCombo` all connected.

### TerminalSearchManager.cpp
- Constructor `themeChanged` connection (line 20): Intact.
- `searchMatchesChanged` signal emitted at: line 40, 94, 102, 165, 182, 189. All paths covered.

**Verdict**: No broken signal connections detected from the refactoring.

## 4. Shared Constants via BaseTransfer:: Prefix

| Constant | Location in BaseTransfer.h | Referenced in |
|----------|--------------------------|---------------|
| `kMaxFileSize` | line 73 (protected, constexpr) | ZModemTransfer.cpp:52,54; XModemTransfer.cpp:63,65,69; YModemTransfer.cpp:35,37,39 |
| `SOH` | line 67 | XMODEM protocol byte |
| `EOT` | line 68 | XMODEM/YMODEM end marker |
| `ACK` | line 69 | XMODEM acknowledgement |
| `NAK` | line 70 | XMODEM negative ack |
| `CAN` | line 71 | Cancel byte |
| `CRC_CHAR` | line 72 | CRC mode request |

All references in ZModemTransfer.cpp use `BaseTransfer::kMaxFileSize` correctly (lines 52, 54). Same pattern verified in XModemTransfer.cpp and YModemTransfer.cpp.

**Note**: The constants `SOH`, `EOT`, `ACK`, `NAK`, `CAN`, `CRC_CHAR` are declared in BaseTransfer.h but are XMODEM-specific. ZModemTransfer does not reference any BaseTransfer protocol constants (it defines its own `ZRQINIT`, `ZRINIT`, etc.), which is correct since ZMODEM uses a different frame format.

## 5. Include Path Format

All files use the required `"module/File.h"` format:

| File | Includes | Format |
|------|----------|--------|
| FrameVisualEditor.cpp | `"protocol/FrameVisualEditor.h"`, `"utils/HexConverter.h"` | PASS |
| OtaWidget.cpp | `"ota/OtaWidget.h"`, `"core/ThemeManager.h"`, `"utils/ByteFormat.h"` | PASS |
| ToolbarController.cpp | `"core/ToolbarController.h"`, `"core/RecordingController.h"`, `"core/ThemeManager.h"`, `"core/Constants.h"` | PASS |
| TerminalSearchManager.cpp | `"terminal/TerminalSearchManager.h"`, `"terminal/DirectionFilter.h"`, `"utils/HexConverter.h"`, `"core/ThemeManager.h"` | PASS |
| ZModemTransfer.cpp | `"ota/protocols/ZModemTransfer.h"`, `"utils/CRC.h"` | PASS |

Include ordering (Qt -> STL -> project) is correct in all files.

## 6. Doxygen Comments on New Methods

| Method | Has Doxygen | Quality |
|--------|-------------|---------|
| `FrameVisualEditor::setupHeaderGroup()` | Yes (line 71-74) | Good: @return described |
| `FrameVisualEditor::setupLengthGroup()` | Yes (line 93-96) | Good: @return described |
| `FrameVisualEditor::setupChecksumGroup()` | Yes (line 124-127) | Good: @return described |
| `FrameVisualEditor::setupFieldsGroup()` | Yes (line 151-154) | Good: @return described |
| `FrameVisualEditor::setupPreviewGroup()` | Yes (line 240-243) | Good: @return described |
| `FrameVisualEditor::setupConnections()` | Yes (line 260-262) | Adequate: brief description |
| `OtaWidget::setupFileGroup()` | Yes (line 72-75) | Good |
| `OtaWidget::setupConfigGroup()` | Yes (line 101-104) | Good |
| `OtaWidget::setupProgressGroup()` | Yes (line 137-140) | Good |
| `OtaWidget::setupLogGroup()` | Yes (line 170-173) | Good |
| `OtaWidget::setupHistoryGroup()` | Yes (line 187-190) | Good |
| `ToolbarController::createDisplayModeGroup()` | Yes (line 90-97) | Good: @param documented |
| `ToolbarController::createConnectionGroup()` | Yes (line 133-140) | Good: @param documented |
| `TerminalSearchManager::buildPlainSearch()` | Yes (header line 140-145) | Good |
| `TerminalSearchManager::buildRegexSearch()` | Yes (header line 147-153) | Good |
| `TerminalSearchManager::buildHexSearch()` | Yes (header line 155-161) | Good |
| `ZModemTransfer::escapeZdle()` | Yes (header line 83) | Adequate: brief description |

All new methods have Doxygen comments. Both header declarations and cpp definitions carry comments.

## 7. Summary of Findings

### PASS
- [x] All .h files under 200 lines
- [x] All .cpp files under 500 lines (ZModemTransfer.cpp at 498 is dangerously close)
- [x] No broken signal connections from refactoring
- [x] Constants correctly referenced via `BaseTransfer::` prefix
- [x] Include paths all use `"module/File.h"` format
- [x] Doxygen comments on all new methods
- [x] 5 out of 6 extracted method groups fully under 80-line limit

### DEFECTS (1)

**D49-1 [Minor] `setupFieldsGroup()` exceeds 80-line method limit (84 lines)**
- File: `src/protocol/FrameVisualEditor.cpp`, lines 155-238
- Root cause: Inline lambda handlers for moveUp/moveDown buttons (lines 203-235) add ~30 lines
- Recommendation: Extract lambdas into named private slots `onMoveFieldUp()` and `onMoveFieldDown()` in the header. This would reduce `setupFieldsGroup()` to approximately 54 lines and improve testability.

### WARNINGS (1)

**W49-1 [Watch] ZModemTransfer.cpp at 498/500 lines**
- Only 2 lines of margin remain. Any future additions (e.g., ZMODEM receiver support, windowing protocol) will breach the limit.
- Proactive suggestion: Extract the state handler methods (handleStateWaitingRinit through handleStateSendingFin, lines 174-279) into a separate `ZModemStateHandlers.cpp` or similar file in the next iteration that touches this file.

## 8. Reflexion Pattern

The setupUI extraction pattern is sound: each sub-method returns a QGroupBox and the parent `setupUI()` acts as an orchestrator. This is the correct approach per the "MainWindow embedded main philosophy" in CLAUDE.md section 5.1.1.

The inline lambda pattern in `setupFieldsGroup()` should be avoided in future extractions -- prefer named private slots for any handler exceeding 5 lines, as it makes the method line count predictable and supports future unit testing.
