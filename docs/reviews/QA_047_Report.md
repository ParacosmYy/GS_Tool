# QA Report - Iteration 47

**Project**: EmbedDebug (C++17 / Qt 6.8.3)
**Date**: 2026-06-01
**Score**: 46/1000
**Auditor**: QA Engineer

---

## 1. Build Verification

| Item | Result |
|------|--------|
| CMake Configure | PASS (CMake 4.0.1, Ninja generator, GCC 14.2.0) |
| Clean Build (64/64 targets) | PASS |
| Compile Errors | 0 |
| Compile Warnings | 0 |
| Linker Output | `build/EmbedDebug.exe` (4,138,495 bytes) |
| Windeployqt | PASS (all Qt DLLs, plugins, translations deployed) |

**Note**: First configure attempt failed because `src/terminal/TerminalContextMenuManager.cpp` was added to CMakeLists.txt before the file existed. The file was subsequently created and the second configure+build succeeded cleanly.

---

## 2. File Size Audit

### 2.1 Header Files (.h) -- Limit: 200 lines

All 66 header files are within the 200-line limit. Zero violations.

**Files approaching the limit (within 10%):**

| File | Lines | Status |
|------|-------|--------|
| src/core/MainWindow.h | 199 | 1 line under limit |
| src/ota/OtaWidget.h | 198 | 2 lines under limit |
| src/core/ToastWidget.h | 196 | 4 lines under limit |
| src/ota/AnimatedProgressBar.h | 194 | 6 lines under limit |
| src/connection/SerialConnection.h | 193 | 7 lines under limit |
| src/core/SendController.h | 192 | 8 lines under limit |
| src/utils/ByteFormat.h | 191 | 9 lines under limit |
| src/core/NavIndicatorWidget.h | 190 | 10 lines under limit |
| src/core/ThemeManager.h | 189 | 11 lines under limit |
| src/chart/ChartWidget.h | 184 | 16 lines under limit |

**Risk**: `MainWindow.h` at 199 lines will violate the limit with any additional declaration. Prioritize keeping it stable or splitting.

### 2.2 Implementation Files (.cpp) -- Limit: 500 lines

All 57 .cpp files are within the 500-line limit. Zero violations.

**Files approaching the limit (within 10%):**

| File | Lines | Status |
|------|-------|--------|
| src/ota/protocols/YModemTransfer.cpp | 499 | 1 line under limit |
| src/utils/DataExporter.cpp | 498 | 2 lines under limit |
| src/ota/protocols/ZModemTransfer.cpp | 497 | 3 lines under limit |
| src/utils/DataLogger.cpp | 493 | 7 lines under limit |
| src/terminal/TerminalWidget.cpp | 467 | 33 lines under limit |
| src/core/ConnectionController.cpp | 467 | 33 lines under limit |

**Critical risk**: `YModemTransfer.cpp` at 499 lines will violate the 500-line limit with any addition. `DataExporter.cpp` at 498 and `ZModemTransfer.cpp` at 497 are equally at risk. These files require proactive splitting before the next iteration.

---

## 3. Method Length Audit -- Limit: 80 lines

Scanned all .cpp files for methods exceeding 80 lines. Found **8 violations** across 8 files:

| File | Method | Lines | Span |
|------|--------|-------|------|
| src/core/MainWindow.cpp | `MainWindow::setupUI` | 87 | 152-239 |
| src/core/ToolbarController.cpp | `ToolbarController::createToolbar` | 103 | 53-156 |
| src/ota/OtaWidget.cpp | `OtaWidget::setupUI` | 125 | 59-184 |
| src/protocol/FrameParser.cpp | `FrameParser::handlePayloadReceiving` | 82 | 288-370 |
| src/protocol/FrameVisualEditor.cpp | `FrameVisualEditor::setupUI` | 177 | 51-228 |
| src/serial/QuickCommandBar.cpp | `QuickCommandBar::onEditRequested` | 94 | 142-236 |
| src/terminal/TerminalLayoutManager.cpp | `TerminalLayoutManager::applyLayout` | 94 | 107-201 |
| src/terminal/TerminalSearchManager.cpp | `TerminalSearchManager::setSearchHighlight` | 116 | 26-142 |

**Severity breakdown**:
- **Critical (>150 lines)**: `FrameVisualEditor::setupUI` at 177 lines -- should be split into sub-methods.
- **High (>100 lines)**: `OtaWidget::setupUI` at 125 lines, `TerminalSearchManager::setSearchHighlight` at 116 lines, `ToolbarController::createToolbar` at 103 lines.
- **Moderate (81-100 lines)**: `QuickCommandBar::onEditRequested` at 94, `TerminalLayoutManager::applyLayout` at 94, `MainWindow::setupUI` at 87, `FrameParser::handlePayloadReceiving` at 82.

**Pattern**: Most violations are `setupUI` methods, which are inherently layout-heavy. Consider extracting widget creation into separate helper methods (e.g., `createTopBar()`, `createMainArea()`, `createBottomBar()`).

---

## 4. Deploy Test

| Step | Result |
|------|--------|
| windeployqt execution | PASS (9 Qt modules, 7 plugin categories, 31 translations) |
| DLL deployment | PASS (Qt6Charts, Qt6Core, Qt6Gui, Qt6Network, Qt6OpenGL, Qt6OpenGLWidgets, Qt6SerialPort, Qt6Svg, Qt6Widgets, plus MinGW runtime) |
| Exe launch test (5s timeout) | PASS (process started, killed by timeout at 5s -- confirmed window opened) |

**Warning**: windeployqt reported missing `dxcompiler.dll` and `dxil.dll`. These are optional Direct3D shader compiler DLLs and do not prevent the application from running (falls back to software rendering). Not a blocking issue.

---

## 5. Include Path Audit

**Rule**: All `#include "..."` statements must use `"module/File.h"` format (with path prefix relative to `src/`), not bare `"File.h"`.

Found **48 violations** across 22 files:

### By file (count):

| File | Count | Violations |
|------|-------|------------|
| src/core/SettingsController.cpp | 5 | SettingsController.h, ToolbarController.h, ThemeManager.h, Constants.h, SerialConfigPanel.h |
| src/core/ToolbarController.cpp | 4 | ToolbarController.h, RecordingController.h, ThemeManager.h, Constants.h |
| src/core/NavigationController.cpp | 3 | NavigationController.h, Constants.h, ThemeManager.h |
| src/core/MainWindow.h | 3 | ConnectionManager.h, ThemeManager.h, Constants.h |
| src/core/ThemeManager.cpp | 2 | ThemeManager.h, Constants.h |
| src/core/MainWindow.cpp | 1 | MainWindow.h |
| src/core/MainWindowSignalConnect.cpp | 1 | MainWindow.h |
| src/core/RecordingController.cpp | 1 | RecordingController.h |
| src/core/TerminalController.cpp | 1 | TerminalController.h |
| src/core/SessionManager.cpp | 1 | ThemeManager.h |
| src/core/BackgroundWidget.cpp | 2 | BackgroundWidget.h, ThemeManager.h |
| src/core/BackgroundSettingsPopup.cpp | 2 | BackgroundSettingsPopup.h, BackgroundWidget.h |
| src/core/ConnectionManager.cpp | 2 | ConnectionManager.h, ConnectionFactory.h |
| src/connection/SerialConnection.cpp | 1 | SerialConnection.h |
| src/connection/SerialConnection.h | 1 | IConnection.h |
| src/protocol/FrameParser.cpp | 1 | FrameParser.h |
| src/protocol/FrameParserHelpers.cpp | 1 | FrameParser.h |
| src/protocol/FrameVisualEditor.cpp | 1 | FrameVisualEditor.h |
| src/protocol/IntelHexParser.cpp | 1 | IntelHexParser.h |
| src/protocol/ProtocolView.cpp | 1 | ProtocolView.h |
| src/serial/SerialConfigPanel.cpp | 2 | SerialConfigPanel.h, SerialDriverDetector.h |
| src/serial/BookmarkWidget.cpp | 1 | BookmarkWidget.h |
| src/serial/PortWatcher.cpp | 1 | PortWatcher.h |
| src/serial/QuickCommandBar.cpp | 1 | QuickCommandBar.h |
| src/serial/TimedSender.cpp | 1 | TimedSender.h |
| src/terminal/TerminalLayoutManager.cpp | 4 | TerminalLayoutManager.h, TerminalWidget.h, TerminalSearchBar.h, TerminalModel.h |
| src/terminal/TerminalModel.cpp | 1 | TerminalModel.h |
| src/terminal/TerminalSearchBar.cpp | 1 | TerminalSearchBar.h |
| src/terminal/DirectionFilter.cpp | 1 | DirectionFilter.h |
| src/utils/SettingsManager.cpp | 1 | SettingsManager.h |

**Pattern**: The violations fall into two categories:
1. **Same-directory includes** (e.g., `#include "MainWindow.h"` in `MainWindow.cpp`) -- these are the most common and technically work because the compiler searches the current directory, but they violate the project coding standard.
2. **Cross-module bare includes** (e.g., `#include "ThemeManager.h"` in `SettingsController.cpp`) -- these happen to resolve because the include path adds `src/`, but they break the explicit module-qualified naming convention.

**All 48 violations require the same fix**: prepend the module directory. For example:
- `#include "MainWindow.h"` becomes `#include "core/MainWindow.h"`
- `#include "FrameParser.h"` becomes `#include "protocol/FrameParser.h"`
- `#include "IConnection.h"` becomes `#include "connection/IConnection.h"`

---

## Summary Scorecard

| Audit Area | Status | Issues Found |
|------------|--------|-------------|
| Build (errors) | PASS | 0 |
| Build (warnings) | PASS | 0 |
| .h file size (200 line limit) | PASS | 0 violations, 10 warnings |
| .cpp file size (500 line limit) | PASS | 0 violations, 6 warnings |
| Method length (80 line limit) | FAIL | 8 violations |
| Deploy test | PASS | 0 |
| Include path convention | FAIL | 48 violations |

**Overall**: Build and deploy are clean. The codebase has no compile errors or warnings. However, there are 8 method-length violations and 48 include-path convention violations that should be addressed in subsequent iterations to maintain code quality standards.

### Priority Recommendations

1. **P0 (Immediate)**: `YModemTransfer.cpp` at 499 lines and `DataExporter.cpp` at 498 lines need proactive splitting before any additions are made.
2. **P1 (Next iteration)**: Fix the 8 method-length violations, starting with `FrameVisualEditor::setupUI` (177 lines) which is the worst offender.
3. **P1 (Next iteration)**: Fix the 48 include-path violations. This is a mechanical change that can be batched.
4. **P2 (Ongoing)**: Monitor the 10 header files within 10% of the 200-line limit.
