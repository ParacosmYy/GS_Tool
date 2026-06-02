# ARCH-040: NavIndicatorWidget + ToastWidget Integration

> Status: Design | Author: System Architect | Date: 2026-06-01

## 1. NavIndicatorWidget Integration

### 1.1 Current State (Already Done)

NavIndicatorWidget is already partially integrated. The code audit shows:

- **Created in** `MainWindow::setupUI()` (line 174): `new NavIndicatorWidget(m_navTree)`
- **Stored in** `MainWindow.h` (line 136): `NavIndicatorWidget* m_navIndicator`
- **Include in** `MainWindow.h` (line 30): `#include "core/NavIndicatorWidget.h"`
- **Animated in** `MainWindowSignalConnect.cpp` (line 192): `m_navIndicator->moveToIndex(index)`

### 1.2 Remaining Work

| Item | Detail | Location |
|------|--------|----------|
| Session restore | Call `jumpToIndex()` when restoring last panel from session | `MainWindow.cpp` constructor, after `restorePanelByIndex()` |
| Network nav clicks | `moveToIndex()` already fires for all nav clicks (line 192) -- no gap | -- |

**Session restore insertion point** (`MainWindow.cpp`, around line 119):

```cpp
if (lastPanel >= 0) {
    m_navController->restorePanelByIndex(lastPanel);
    // NEW: sync indicator to restored panel
    QModelIndex restoredIdx = m_navTree->model()->index(lastPanel, 0);
    m_navIndicator->jumpToIndex(restoredIdx);
}
```

Note: The exact index mapping requires traversing the tree model to find the
leaf node corresponding to `lastPanel` in the NavPanelMapping table. This is
a lookup, not a direct model index. NavigationController should expose a
helper: `QModelIndex leafIndexForMapping(int mappingIndex) const`.

### 1.3 Geometry & Resize

NavIndicatorWidget installs an event filter on `m_navTree` (its own constructor,
line 72). On `QEvent::Resize`, it calls `setGeometry(0, 0, width, height)`. No
work needed in MainWindow -- the widget is fully self-managing.

### 1.4 Theme Change

The constructor connects `ThemeManager::themeChanged` to `update()` (line 75-76).
Each `paintEvent` fetches the accent color live from ThemeManager. No additional
wiring required.

### 1.5 Lifecycle

- **Owner**: `m_navTree` (QObject parent passed in constructor)
- **Created**: `setupUI()`, after m_navTree exists, before splitter assembly
- **Destroyed**: Automatically by Qt parent-child tree when navTree is destroyed

---

## 2. ToastWidget Integration

### 2.1 Current State (Already Done)

ToastWidget is already integrated for three event sources:

| Event Source | Signal | Toast Call | File:Line |
|-------------|--------|-----------|-----------|
| Connection failed | `ConnectionController::connectionFailed` | `ToastWidget::show(this, msg, Error)` | `MainWindowSignalConnect.cpp:86-89` |
| Send status | `SendController::statusMessage` | `ToastWidget::show(this, msg, Info)` | `MainWindowSignalConnect.cpp:103-106` |
| Recording status | `RecordingController::statusMessage` | `ToastWidget::show(this, msg, Info, timeout)` | `MainWindowSignalConnect.cpp:140-144` |

### 2.2 Remaining Event Sources

| Event Source | Signal | Toast Type | Rationale |
|-------------|--------|-----------|-----------|
| Connection success | `ConnectionController::connectionStateChanged` with `Connected` | `Success` | User feedback for successful connect |
| Connection disconnected | Same signal with `Disconnected` | `Info` | User feedback for disconnect |
| OTA transfer complete | `OtaManager::transferComplete` | `Success` | Background panel may not be visible |
| OTA transfer error | `OtaManager::transferError` | `Error` | User may be on a different panel |
| Port hot-plug | `ConnectionController::portAdded` | `Info` | Already in status bar, toast adds visibility |
| Port removed | `PortWatcher::portRemoved` | `Error` | Unplugged device warrants prominent notice |

### 2.3 Avoiding Duplicate Toasts

Problem: Rapid connect/disconnect cycles (e.g., auto-reconnect enabled) can
flood the screen with toasts.

Solution: Add a debounce map in MainWindowSignalConnect.cpp:

```cpp
// File-scope debounce helper (anonymous namespace in MainWindowSignalConnect.cpp)
namespace {
    QMap<QString, QElapsedTimer> g_toastDebounce;
    bool shouldShowToast(const QString& key, int cooldownMs = 2000) {
        if (g_toastDebounce.contains(key)
            && !g_toastDebounce[key].hasExpired(cooldownMs)) {
            return false;
        }
        g_toastDebounce[key].start();
        return true;
    }
}
```

Usage in signal connections:

```cpp
// Connection state toast with debounce
connect(m_connController, &ConnectionController::connectionStateChanged,
        this, [this](ConnectionState state, const QString& connName) {
    if (state == ConnectionState::Connected
        && shouldShowToast("conn_state")) {
        ToastWidget::show(this, tr("已连接: %1").arg(connName),
                          ToastWidget::ToastType::Success);
    }
});
```

This keeps the debounce logic local to the signal-connect file, not leaking
into MainWindow.h or any controller.

### 2.4 ToastWidget Static API Design

ToastWidget uses a static `show()` factory method. Each call creates a new
`ToastWidget` instance with `parent = MainWindow`. The widget:

- Self-positions in parent's bottom-right corner
- Auto-stacks vertically when multiple toasts are active
- Self-destructs via `deleteLater()` after dismiss animation
- Uses a static `QMap<QWidget*, QList<ToastWidget*>>` to track active toasts

No members needed in MainWindow.h for ToastWidget. Zero footprint on the header.

### 2.5 Lifecycle

- **Owner**: ToastWidget instances parent themselves to the `parent` widget (MainWindow)
- **Created**: On-demand by `ToastWidget::show()`
- **Destroyed**: `deleteLater()` after dismiss animation finishes

---

## 3. MainWindow Impact

### 3.1 MainWindow.h Changes

**Already present (no new members needed):**

```cpp
// Line 30 - already included
#include "core/NavIndicatorWidget.h"

// Line 136 - already declared
NavIndicatorWidget* m_navIndicator;
```

**ToastWidget**: `#include "core/ToastWidget.h"` already present in
`MainWindowSignalConnect.cpp` (line 23). No include needed in MainWindow.h
because ToastWidget is only used via its static `show()` method in the
signal-connect file.

**Net new lines in MainWindow.h**: 0

**Net new lines in MainWindow.cpp**: ~5 (session restore jumpToIndex call)

**Net new lines in MainWindowSignalConnect.cpp**: ~35 (new toast connections
+ debounce helper)

### 3.2 New Signal Connections

All additions go into `MainWindow::connectSignals()` in
`MainWindowSignalConnect.cpp`:

```
ConnectionController::connectionStateChanged  -> toast (Connected/Disconnected)
OtaManager::transferComplete                  -> toast (Success)
OtaManager::transferError                     -> toast (Error)
ConnectionController::portAdded               -> toast (Info)
PortWatcher::portRemoved                      -> toast (Error)
```

### 3.3 Philosophy Compliance

The "embedded main" philosophy is fully preserved:

- MainWindow.h: +0 new members
- MainWindow.cpp: +5 lines (session restore sync)
- All toast logic lives in signal-connect lambdas
- NavIndicatorWidget manages its own geometry, resize, and theme
- ToastWidget manages its own positioning, stacking, and destruction

---

## 4. Dependency Diagram

```
MainWindow
  |
  |-- owns (QObject parent-child) --> NavIndicatorWidget
  |       |-- reads geometry from --> QTreeView (m_navTree)
  |       |-- reads color from --> ThemeManager
  |       `-- animated by --> QPropertyAnimation (internal)
  |
  |-- owns (via constructor) --> NavigationController
  |       `-- panel switching, tree model
  |
  |-- signal-connect (no ownership) --> ToastWidget::show()
  |       |-- parent = MainWindow (QObject parent-child for lifetime)
  |       |-- reads colors from --> ThemeManager
  |       `-- animated by --> QPropertyAnimation (internal)
  |
  |-- signal source --> ConnectionController
  |       `-- connectionStateChanged / connectionFailed / portAdded
  |
  |-- signal source --> OtaManager (via OtaWidget)
  |       `-- transferComplete / transferError
  |
  |-- signal source --> PortWatcher
  |       `-- portRemoved
  |
  `-- signal source --> SendController / RecordingController
          `-- statusMessage (already connected)
```

**Coupling direction**: All arrows point inward or sideways. ToastWidget and
NavIndicatorWidget depend on ThemeManager (stable singleton). Neither widget
depends on any controller. Controllers do not know about these widgets.

```
ThemeManager (singleton, stable)
    ^        ^
    |        |
NavIndicatorWidget   ToastWidget
    ^                  ^
    |                  |
  MainWindow (mediator, creates + connects only)
    |
    v
ConnectionController / OtaManager / NavigationController (business layer)
```

---

## 5. File Change Summary

| File | Change | Lines Added |
|------|--------|-------------|
| `MainWindow.h` | No change needed | 0 |
| `MainWindow.cpp` | Add `jumpToIndex()` after session restore | ~5 |
| `MainWindowSignalConnect.cpp` | Add toast connections + debounce helper | ~35 |
| `NavigationController.h/cpp` | Add `leafIndexForMapping()` helper (optional) | ~12 |
| **Total** | | **~52** |

No new files. No new classes. No header growth. All within the 200-line
MainWindow.h limit (currently 197 lines, delta = 0).
