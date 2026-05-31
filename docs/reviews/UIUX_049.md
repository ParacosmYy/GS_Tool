# UI/UX Review 049 -- Key Component Spot Check

**Reviewer**: UI/UX Product Experience (frontend-architect)
**Date**: 2026-06-01
**Scope**: SerialConfigPanel, ToolbarController, OtaWidget, FrameVisualEditor
**Score baseline**: 48/1000, iteration 49

---

## 1. objectName Compliance

### SerialConfigPanel.cpp -- PASS
All QPushButtons and interactive widgets have objectName set:
- `dtrBtn`, `rtsBtn` -- with `signalState` property for QSS targeting
- `refreshBtn`, `connectBtn` -- with `state` property
- `statusIndicator` -- with `state` property
- `portCombo`, `baudCombo`, `dataBitsCombo`, `parityCombo`, `stopBitsCombo`, `flowControlCombo`
- `portGroup`, `paramGroup`, `signalGroup`
- `driverInfoLbl`

### ToolbarController.cpp -- PASS
All controls have objectName:
- `mainToolbar`, `displayModeCombo`, `layoutCombo`
- `timestampAction`, `dirPrefixAction`, `clearAction`, `exportAction`, `bgSettingsAction`
- `themeLabel`, `themeCombo`, `langLabel`, `langCombo`

### OtaWidget.cpp -- PASS
All controls have objectName:
- `otaFileGroup`, `otaConfigGroup`, `otaProgressGroup`, `otaLogGroup`, `otaHistoryGroup`
- `otaFileLabel`, `otaBrowseBtn`, `otaFileInfo`
- `otaProtocolCombo`, `otaStartBtn`, `otaCancelBtn`
- `otaProgress`, `otaStatusLbl`, `otaSpeedLbl`, `otaEtaLbl`
- `otaLogView`, `otaHistoryView`, `otaClearHistory`

### FrameVisualEditor.cpp -- PASS
All controls have objectName:
- `frameHeaderGroup`, `frameLengthGroup`, `frameChecksumGroup`, `frameFieldGroup`, `framePreviewGroup`
- `frameHeaderEdit`, `frameFooterEdit`
- `frameLengthOffsetSpin`, `frameLengthSizeCombo`, `frameLengthEndianCheck`, `frameLengthAdjustSpin`
- `frameChecksumTypeCombo`, `frameChecksumOffsetSpin`, `frameChecksumStartSpin`
- `frameFieldTable`, `frameAddFieldBtn`, `frameRemoveFieldBtn`, `frameMoveUpBtn`, `frameMoveDownBtn`
- `framePreviewLabel`, `applyDefBtn`
- Dynamic cell widgets: `fieldTypeCombo`, `fieldEndianCombo`

---

## 2. Hardcoded Colors

**Result: ALL CLEAR** -- No `#RRGGBB`, `rgb()`, `rgba()`, or `setStyleSheet()` calls found in any of the four files. All color logic is driven through:
- QSS property selectors (`signalState`, `state`) with unpolish/polish refresh
- ThemeManager::instance().color() for self-painted controls (OtaWidget completion animation)

---

## 3. Spacing Consistency

### Within-spec values (good)
| File | Margins | Spacing | Notes |
|------|---------|---------|-------|
| SerialConfigPanel | 12,12,12,12 | 12 | Matches panel padding spec (8-12px) |
| OtaWidget | 12,12,12,12 | 12 | Consistent with SerialConfigPanel |
| OtaWidget sub-groups | 4, 6, 8 | | All within spec (4-8px intra-group) |

### ISSUE: FrameVisualEditor asymmetric margins
- **Line 54**: `mainLayout->setContentsMargins(12, 8, 12, 8)` -- top/bottom are 8px while left/right are 12px.
- SerialConfigPanel and OtaWidget both use uniform 12px margins.
- This is a **minor inconsistency**. The 8px vertical is technically within spec (8-12px range), but the asymmetry breaks visual consistency when panels are compared side-by-side. All other panels use 12px uniform.

### ISSUE: FrameVisualEditor missing layout spacing
- `setupUI()` sets margins but does **not** call `mainLayout->setSpacing()`. The default QVBoxLayout spacing is implementation-dependent (typically ~9px on Windows). All other panels explicitly set spacing to 12px.

### ISSUE: OtaWidget button heights inconsistent with panel standard
- `otaStartBtn`/`otaCancelBtn`: `setFixedHeight(32)`
- `otaClearHistory`: `setFixedHeight(28)`
- `connectBtn`: `setMinimumHeight(36)`
- `applyDefBtn`: `setMinimumHeight(32)`
- The connect button at 36px is noticeably taller than the OTA start button at 32px. While both are within spec (28-36px range), the inconsistency between equivalent "primary action" buttons is visible.

---

## 4. QSS Coverage Across All Three Themes

Cross-referenced every objectName in the four C++ files against all three QSS theme files:

| Theme File | Coverage | Status |
|-----------|----------|--------|
| `dark_terminal.qss` | All objectNames found | PASS |
| `modern_dark.qss` | All objectNames found | PASS |
| `light.qss` | All objectNames found | PASS |

Verified that all state-based property selectors are present:
- `QPushButton#dtrBtn[signalState="high"/"low"]` -- present in all 3 themes
- `QPushButton#connectBtn[state="connected"/"connecting"]` -- present in all 3 themes
- `QLabel#statusIndicator[state="connected"/"connecting"/"disconnected"/"error"]` -- present in all 3 themes
- `QPushButton#applyDefBtn` -- present in all 3 themes

---

## 5. Findings Summary

| # | Severity | File | Issue |
|---|----------|------|-------|
| F1 | Minor | FrameVisualEditor.cpp:54 | Asymmetric margins (12,8,12,8) differ from panel standard (12,12,12,12) |
| F2 | Minor | FrameVisualEditor.cpp:53 | No explicit `setSpacing()` call; default is platform-dependent, not guaranteed to be 12px |
| F3 | Cosmetic | OtaWidget.cpp vs SerialConfigPanel.cpp | Primary action button heights differ (32px vs 36px) between OTA start and connect buttons |

**Critical issues**: 0
**Blocked items**: 0

---

## 6. Recommended Fixes

1. **FrameVisualEditor.cpp line 54**: Change to `mainLayout->setContentsMargins(12, 12, 12, 12)` for consistency.
2. **FrameVisualEditor.cpp line 55** (add after margins): Add `mainLayout->setSpacing(12)` for explicit control.
3. **Button height alignment**: Decide on a single standard for primary action buttons. Recommendation: use `setMinimumHeight(36)` for all primary actions (connect, start transfer, apply definition) to maintain visual weight, and `setFixedHeight(32)` for secondary/cancel buttons.
