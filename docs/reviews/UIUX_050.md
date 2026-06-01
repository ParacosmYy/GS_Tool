# UI/UX Review 050 -- Connection & Export Layer Spot Check

**Reviewer**: UI/UX Product Experience (frontend-architect)
**Date**: 2026-06-01
**Scope**: ConnectionController.cpp, MainWindowSignalConnect.cpp, TcpConnection.cpp, UdpConnection.cpp, DataExporter.cpp
**Score baseline**: 49/1000, iteration 50

---

## 1. Hardcoded Colors

### Result: ALL CLEAR

Scanned all five files for `#RRGGBB`, `#RRGGBBAA`, `rgb()`, `rgba()`, and `setStyleSheet()` patterns. **Zero matches found.**

- `ConnectionController.cpp`: No color references. All status-driven visual changes go through QSS property selectors (`state` property + unpolish/polish).
- `MainWindowSignalConnect.cpp`: Uses `setProperty("state", ...)` + `style()->unpolish/polish()` pattern for `m_connStatusLbl`. No inline styles.
- `TcpConnection.cpp`: Pure networking logic. No UI code.
- `UdpConnection.cpp`: Pure networking logic. No UI code.
- `DataExporter.cpp`: Pure data processing. No UI code.

---

## 2. objectName Compliance

### Scope Clarification

Four of the five files (ConnectionController, TcpConnection, UdpConnection, DataExporter) are business/infrastructure layer code with no QWidget creation. objectName requirements apply only to QWidgets (Section 6.8 rule 2).

**MainWindowSignalConnect.cpp** does not create widgets (it connects signals on existing widgets created in MainWindow.cpp). However, it manipulates `m_connStatusLbl` which was created with `setObjectName("connStatus")` in MainWindow.cpp:248.

### ISSUE F1: Stale QSS selector `connStatusLbl`

All three QSS theme files contain a **dead selector** `QLabel#connStatusLbl` that does not match any widget:

| Theme File | Line | Dead Selector |
|-----------|------|---------------|
| `dark_terminal.qss` | 223 | `QLabel#connStatusLbl { font-size: 12px; padding: 0 4px; }` |
| `modern_dark.qss` | 220 | `QLabel#connStatusLbl { font-size: 12px; padding: 0 4px; }` |
| `light.qss` | 220 | `QLabel#connStatusLbl { font-size: 12px; padding: 0 4px; }` |

The actual objectName is `connStatus` (set at MainWindow.cpp:248). The state-based selectors `QLabel#connStatus[state="..."]` at lines 642-646 (dark_terminal), 634-638 (modern_dark, light) **do** match and are correct.

This means the font-size and padding rules in the stale selectors are **not applied** to the connection status label. The `connStatus` label only gets the state-color rules; it inherits its font-size from a broader QLabel selector.

---

## 3. tr() Usage Compliance

### MainWindowSignalConnect.cpp -- PASS (Chinese source strings)

All 22 user-visible strings use `tr()` with Chinese source text. Verified:
- Toast messages: `tr("已连接: %1")`, `tr("已断开: %1")`, `tr("连接错误: %1\n%2")`, `tr("重连成功: %1")`, etc.
- Status bar: `tr("重连中... (第 %1/%2 次)")`, `tr("检测到新端口: %1")`, `tr("端口已拔出: %1")`
- Navigation: `tr("数据导出")`, `tr("TCP客户端")`, `tr("TCP服务端")`, `tr("UDP")`
- OTA: `tr("开始传输: %1")`, `tr("传输完成: %1 (%2, %3)")`, `tr("传输失败: %1\n%2")`
- Bookmarks: `tr("书签已添加: %1")`, `tr("书签列表已更新 (%1)")`

### TcpConnection.cpp -- PASS (Chinese source strings)

The `translateNetworkError()` method uses `TcpConnection::tr()` with explicit class qualifier (correct for static/friend context). All 15 error messages are Chinese source strings wrapped in tr():
- `tr("连接被拒绝，请检查目标地址和端口是否正确")`
- `tr("远程主机已关闭连接")`
- etc.

One additional tr() in `open()`: `tr("TCP服务器监听失败: %1")` at line 67.

### UdpConnection.cpp -- PASS (Chinese source strings)

Same pattern as TcpConnection. `translateNetworkError()` uses `UdpConnection::tr()` with explicit class qualifier. All 13 error messages are Chinese source strings wrapped in tr():
- `tr("连接被拒绝，请检查目标地址和端口是否正确")`
- `tr("通信超时，请检查目标主机是否可达")` (UDP-specific wording vs TCP's "连接超时")
- etc.

One additional tr() in `open()`: `tr("UDP绑定端口失败: %1")` at line 56.

### DataExporter.cpp -- PASS (Chinese source strings)

All 7 user-visible error messages use `tr()`:
- `tr("无法打开文件: %1")` (3 occurrences, consistent)
- `tr("写入文件失败: %1")` (3 occurrences, consistent)
- `tr("无有效数据可导出")` (1 occurrence)

### ISSUE F2: ConnectionController.cpp language inconsistency

ConnectionController.cpp uses **English** tr() source strings, while the entire rest of the codebase uses **Chinese** tr() source strings. The translation file (`embeddebug_en.ts`) has `sourcelanguage="zh_CN"`, confirming Chinese is the source language convention.

Affected strings (18 occurrences across the file):

| Line | Current (English) | Expected Convention (Chinese) |
|------|-------------------|-------------------------------|
| 94 | `tr("Not Supported")` | `tr("不支持")` |
| 94 | `tr("Serial connection not available")` | `tr("串口连接不可用")` |
| 110 | `tr("Connection Failed")` | `tr("连接失败")` |
| 110 | `tr("Cannot open serial port")` | `tr("无法打开串口")` |
| 152 | `tr("user disconnect")` (log string) | `tr("用户断开")` |
| 194 | `tr("This connection type is not yet available")` | `tr("该连接类型暂不可用")` |
| 208-209 | `tr("Cannot establish network connection")` | `tr("无法建立网络连接")` |
| 231 | `tr("Network")` | `tr("网络")` |
| 286 | `tr("Connection error occurred")` | `tr("连接发生错误")` |
| 327 | `tr("Unknown")` | `tr("未知")` |
| 332 | `tr("connection timeout")` (log string) | `tr("连接超时")` |
| 334 | `tr("Connection Timeout")` | `tr("连接超时")` |
| 335 | `tr("Connection timed out after %1 seconds...")` | `tr("连接在 %1 秒后超时，请检查设备后重试")` |
| 340 | `tr("Connection timed out")` | `tr("连接超时")` |
| 361 | `tr("Max reconnect attempts reached (%1)")` | `tr("已达最大重连次数 (%1)")` |
| 391 | `tr("port removed: %1")` (log string) | `tr("端口拔出: %1")` |
| 395-397 | `tr("Port Removed")`, `tr("Serial port %1 was disconnected...")` | `tr("端口已拔出")`, `tr("串口 %1 已断开，请重新连接设备")` |
| 400 | `tr("Port was physically removed")` | `tr("端口被物理拔出")` |
| 422 | `tr("Unknown")` | `tr("未知")` |
| 424 | `tr("Connection Error")` | `tr("连接错误")` |

**Note**: Some of these (lines 152, 332, 391) are log/debug strings passed to `teardownConnection()`. They appear in `qInfo()` output but not directly in the UI. Whether these need tr() is debatable, but consistency with the rest of the codebase argues for Chinese source strings.

### ISSUE F3: TcpConnection/UdpConnection `name()` returns non-tr()-wrapped strings

`TcpConnection::name()` (lines 21-25) returns:
```cpp
return QString("TCP:%1:%2").arg(m_host).arg(m_port);
return QString("TCP Server:%1").arg(m_port);
```

`UdpConnection::name()` (lines 21-27) returns:
```cpp
return QString("UDP:%1:broadcast").arg(m_localPort);
return QString("UDP:%1>%2:%3").arg(m_localPort).arg(m_remoteHost.toString()).arg(m_remotePort);
```

These strings are displayed to users via `connectionSucceeded`, `connectionError`, Toast messages, and status bar text (all routed through `m_currentConn->name()` in ConnectionController). They contain the arrow character `->` which is a developer convention, not a user-friendly separator.

These are **technical identifiers** (like "COM3:115200") and could reasonably stay untranslated. However, the arrow operator `->` should use a more readable separator for end users, and the strings should use `tr()` to allow future localization of the "TCP Server" label.

---

## 4. Spacing Consistency

None of the five files create QWidget layouts. Spacing compliance is N/A for this review scope.

---

## 5. Button Height Consistency

None of the five files create QPushButtons. Button compliance is N/A for this review scope.

---

## 6. QSS Coverage Audit (connStatus Follow-up)

Verified the `QLabel#connStatus` selectors across all three themes:

| State | dark_terminal.qss | modern_dark.qss | light.qss |
|-------|------------------|-----------------|-----------|
| default | `font-size: 12px` | `font-size: 12px` | `font-size: 12px` |
| `connected` | `color: #a6e3a1` (green) | `color: #9ece6a` (green) | `color: #22c55e` (green) |
| `disconnected` | `color: #f38ba8` (red) | `color: #f7768e` (red) | `color: #dc2626` (red) |
| `connecting` | `color: #f9e2af` (yellow) | `color: #e0af68` (yellow) | `color: #f59e0b` (yellow) |
| `error` | `color: #f38ba8` (red) | `color: #f7768e` (red) | `color: #dc2626` (red) |

All state selectors are present and correctly typed. No missing states.

---

## 7. Findings Summary

| # | Severity | File | Issue |
|---|----------|------|-------|
| F1 | Minor | All 3 QSS theme files | Dead selector `QLabel#connStatusLbl` does not match actual objectName `connStatus`. The font-size/padding rules in this selector are not applied. Should be renamed to `QLabel#connStatus`. |
| F2 | Major | ConnectionController.cpp (18 locations) | All tr() source strings are in English, violating the project convention of Chinese source strings. The translation file `embeddebug_en.ts` expects `sourcelanguage="zh_CN"`. |
| F3 | Minor | TcpConnection.cpp:21-25, UdpConnection.cpp:21-27 | `name()` returns non-tr()-wrapped strings displayed to users. The "->" separator is a developer convention. Should use tr() and a more readable separator. |

**Critical issues**: 0
**Blocked items**: 0

---

## 8. Recommended Fixes

### F1: Remove stale QSS selector

In all three theme files, rename `QLabel#connStatusLbl` to `QLabel#connStatus`:

**dark_terminal.qss line 223:**
```
- QLabel#connStatusLbl {
+ QLabel#connStatus {
```

Apply the same change at:
- `modern_dark.qss` line 220
- `light.qss` line 220

### F2: Convert ConnectionController.cpp tr() to Chinese source strings

Replace all 18 English tr() calls with Chinese equivalents (see table in Section 3, issue F2). This is a bulk find-and-replace operation with no logic changes. The English translations will be provided by the existing `embeddebug_en.ts` translation file after running `lupdate`.

Example:
```cpp
// Before:
emit connectionFailed(tr("Connection Failed"), tr("Cannot open serial port"));
// After:
emit connectionFailed(tr("连接失败"), tr("无法打开串口"));
```

### F3: Wrap connection name strings with tr()

In `TcpConnection::name()`:
```cpp
// Before:
return QString("TCP:%1:%2").arg(m_host).arg(m_port);
return QString("TCP Server:%1").arg(m_port);

// After:
return tr("TCP:%1:%2").arg(m_host).arg(m_port);
return tr("TCP服务器:%1").arg(m_port);
```

In `UdpConnection::name()`:
```cpp
// Before:
return QString("UDP:%1:broadcast").arg(m_localPort);
return QString("UDP:%1->%2:%3").arg(...)

// After:
return tr("UDP:%1:广播").arg(m_localPort);
return tr("UDP:%1->%2:%3").arg(...)  // keep -> as technical notation
```

The arrow separator `->` is acceptable for technical port displays but could be improved to a Unicode arrow `→` for better readability.
