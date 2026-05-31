# PRD-045: 架构审查里程碑 + Bug修复冲刺

## 背景

本次迭代 #45 是架构审查里程碑（每5次commit执行一次），审查 commit #41~#44 的架构合规性；同时也是 Bug 修复冲刺（每3次迭代执行一次），需全面扫描当前工具中的已知 Bug。

当前评分: 44/1000
审查范围: commit #41 `e1707b6` ~ commit #44 `4c84ab9`（共4次commit，含1次架构审查触发）
变更统计: 67 files, +7368 insertions, -853 deletions

---

## 第一部分: 架构审查（Commit #41~#44）

### 审查提交清单

| # | Hash | 内容摘要 |
|---|------|---------|
| #41 | `e1707b6` | NavIndicatorWidget+ToastWidget集成: 连接状态吐司通知+OTA传输吐司+NavIndicator主题联动+QSS三主题吐司样式+OTA错误消息中文化+XModem Doxygen注释 |
| #42 | `e900854` | Bug审计冲刺#42: 呼吸动画QSequentialAnimationGroup+Toast向上飘出+teardownConnection提取+Toast防抖连接+QSS全局字体+ZModem/BaseTransfer/YModem Doxygen注释+Bug审计25项+新特性评审 |
| #43 | `2f054c2` | P0 Bug修复#43: Ctrl+C精确修饰符匹配+搜索高亮偏移修正+AnimatedProgressBar竞态修复+XModem模式保留+网络重连参数持久化+setMaxLines重入守卫+DataBookmark数据结构+QSS语义色注释包裹修复主题加载 |
| #44 | `4c84ab9` | TerminalWidget拆分+BookmarkWidget书签面板+DataLogger seek跳转+OTA进度条objectName修复 |

---

### 1. 分层架构合规性

**审查结论: 合格，有1处轻微问题需关注**

| 层级 | 文件 | 合规性 | 说明 |
|------|------|--------|------|
| **表现层** | `BookmarkWidget.h/cpp` | 合格 | 纯 UI 控件，书签操作通过信号委托给上层，不直接调用 DataLogger |
| **表现层** | `TerminalSearchRenderer.h/cpp` | 合格 | 无状态渲染辅助类，从 TerminalWidget::paintLine 提取，不持有数据 |
| **表现层** | `OtaWidget.h/cpp` | 合格 | 仅展示进度/状态，业务逻辑委托给 OtaManager |
| **表现层** | `NavIndicatorWidget.h` | 合格 | 纯 UI 动画控件，颜色从 ThemeManager 获取 |
| **业务层** | `ConnectionController.h/cpp` | 合格 | 中介者模式协调上下游，无反向依赖 |
| **业务层** | `NavigationController.h/cpp` | 合格 | 管理面板切换动画和导航树，依赖合理 |
| **业务层** | `OtaManager.h/cpp` | 合格 | 协调传输协议和文件验证 |
| **数据层** | `TerminalModel.h/cpp` | 合格 | 数据模型不依赖 UI |
| **数据层** | `DataLogger.h/cpp` | 合格 | 录制/回放引擎，新增 seek/bookmark 功能保持层内 |
| **数据层** | `DataBookmark.h` | 合格 | 纯数据结构（struct），无依赖 |
| **基础设施层** | `ToastWidget.h` | 合格 | 通知组件，颜色从 ThemeManager 获取，无反向依赖 |
| **基础设施层** | `BaseTransfer/XModem/YModem/ZModem` | 合格 | OTA协议层，仅依赖 IConnection 接口 |

**问题 ARCH-045-01 [轻微]**: `DataLogger.h` 中 `#include` 顺序违反 CLAUDE.md 5.3 规范。当前将项目头文件 `"utils/DataBookmark.h"` 放在第4行（Qt头文件之前），应调整为: Qt头文件 -> STL头文件 -> 项目头文件。

```cpp
// 当前（违规）:
#include "utils/DataBookmark.h"    // 项目头文件在最前
#include <QObject>
#include <QFile>
...

// 应改为:
#include <QObject>                 // Qt头文件在前
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include "utils/DataBookmark.h"    // 项目头文件在最后
```

**严重程度**: 轻微（不影响编译和运行，但违反编码规范）

---

### 2. 公共组件复用检查

**审查结论: 合格，无重复造轮子**

| 新增组件 | 是否复用已有组件 | 说明 |
|---------|-----------------|------|
| `DataBookmark` (utils/DataBookmark.h) | 复用 Qt 类型 | QJsonObject/QVector/QDateTime，未重写序列化逻辑 |
| `BookmarkWidget` (serial/BookmarkWidget.h) | 复用 QListWidget | 使用 Qt 标准列表控件，未自绘 |
| `TerminalSearchRenderer` (terminal/TerminalSearchRenderer.h) | 复用 TerminalSearchManager | 委托给已有搜索管理器获取匹配数据 |
| `ToastWidget` (core/ToastWidget.h) | 复用 ThemeManager | 颜色从语义色板获取 |
| `NavIndicatorWidget` (core/NavIndicatorWidget.h) | 复用 ThemeManager + QPropertyAnimation | 无重复实现 |

---

### 3. 设计模式合规性（8大模式）

| 模式 | 应用位置 | 合规性 | 说明 |
|------|---------|--------|------|
| **策略模式** | OTA协议切换 (IProtocol/XModem/YModem/ZModem) | 合格 | OtaManager通过协议名选择实例 |
| **观察者模式** | 信号/槽分发 | 合格 | 新增 DataLogger::bookmarksChanged/seekCompleted 信号 |
| **工厂模式** | ConnectionFactory | 合格 | 未受本次变更影响 |
| **状态模式** | ConnectionState | 合格 | BaseTransfer双状态机架构清晰 |
| **单例模式** | SettingsManager/ThemeManager | 合格 | ToastWidget通过ThemeManager::instance()获取颜色 |
| **模板方法** | BaseTransfer::start()/cancel() | 合格 | 钩子清晰: onStartInit/sendCancelBytes/processReceivedData/handleTimeout |
| **适配器模式** | JLinkBridge | 合格 | 未受本次变更影响 |
| **命令模式** | QuickCommand/BookmarkWidget | 合格 | 书签操作通过信号封装为命令 |

---

### 4. 模块间耦合度评估

**耦合度: 低~中等，架构健康**

| 模块对 | 耦合类型 | 评估 |
|--------|---------|------|
| BookmarkWidget -> DataBookmark | 数据依赖（只读） | 低耦合: 仅读取 struct 字段 |
| BookmarkWidget -> DataLogger | 信号委托 | 低耦合: 通过信号间接调用，MainWindow 路由 |
| DataLogger -> DataBookmark | 数据持有 | 低耦合: 持有 QVector<DataBookmark> |
| OtaWidget -> OtaManager | 接口依赖 | 中等: OtaWidget 持有 OtaManager 指针，但仅通过信号通信 |
| ToastWidget -> ThemeManager | 单例依赖 | 低耦合: 颜色获取，无业务逻辑耦合 |
| TerminalSearchRenderer -> TerminalSearchManager | 只读依赖 | 低耦合: 静态方法，仅读取搜索数据 |
| ConnectionController -> 各下游Controller | 中介者模式 | 低耦合: 通过 setConnection 注入，接口清晰 |
| MainWindowSignalConnect -> 各模块 | 信号路由 | 中等: 这是信号路由的集中连接点，职责明确 |

---

### 5. 头文件引用规范

**问题汇总**:

| 文件 | 问题 | 严重程度 |
|------|------|---------|
| `DataLogger.h` | `#include "utils/DataBookmark.h"` 在 Qt 头文件之前，违反 Qt->STL->项目 顺序 | 轻微 |
| `BookmarkWidget.h` | Qt 头文件 -> 项目头文件顺序正确 | 合格 |
| `TerminalSearchRenderer.h` | Qt -> 项目头文件顺序正确 | 合格 |
| `ConnectionController.h` | Qt -> 项目头文件顺序正确 | 合格 |
| `OtaWidget.h` | Qt -> 项目头文件顺序正确 | 合格 |
| `XModemTransfer.h` | 项目头文件 -> Qt 头文件（`#include "utils/CRC.h"` 在 `<QElapsedTimer>` 之前） | 轻微 |

**问题 ARCH-045-02 [轻微]**: `XModemTransfer.h` 中 `#include "utils/CRC.h"` 和 `#include "ota/protocols/BaseTransfer.h"` 排在 `<QElapsedTimer>` 之前，违反 Qt->STL->项目 的 include 顺序规范。

---

### 6. 新增类登记检查

以下新增类需登记到 CLAUDE.md 公共组件清单:

| 类名 | 文件 | 用途 | 是否已登记 |
|------|------|------|-----------|
| `DataBookmark` | `utils/DataBookmark.h` | 数据书签纯数据结构 | 否，需登记 |
| `BookmarkWidget` | `serial/BookmarkWidget.h/cpp` | 书签管理面板 | 否，需登记 |
| `TerminalSearchRenderer` | `terminal/TerminalSearchRenderer.h/cpp` | 终端搜索高亮渲染器 | 否，需登记 |

---

### 7. 目录结构组织

**审查结论: 合格**

新增文件放置位置正确:
- `src/utils/DataBookmark.h` -- 数据层工具，位置正确
- `src/serial/BookmarkWidget.h/cpp` -- 表现层串口功能UI，位置正确
- `src/terminal/TerminalSearchRenderer.h/cpp` -- 表现层终端辅助，位置正确
- `src/core/NavIndicatorWidget.h` -- 核心层导航辅助，位置正确
- `src/core/ToastWidget.h` -- 核心层通知组件，位置正确

---

### 8. 文件体积检查

| 文件 | 行数 | 上限 | 状态 |
|------|------|------|------|
| TerminalWidget.cpp | 498 | 500 | 临界（差2行） |
| DataLogger.cpp | 489 | 500 | 合格 |
| ConnectionController.cpp | 468 | 500 | 合格 |
| NavigationController.cpp | 427 | 500 | 合格 |
| OtaWidget.cpp | 418 | 500 | 合格 |
| MainWindow.cpp | 361 | 500 | 合格 |
| MainWindow.h | 199 | 200 | 合格 |
| MainWindowSignalConnect.cpp | 390 | 500 | 合格 |
| ToastWidget.h | 196 | 200 | 合格 |

**关注点 ARCH-045-03**: `TerminalWidget.cpp` 当前 498 行，距 500 行上限仅余 2 行。后续若需新增终端功能，需继续拆分提取。`TerminalSearchRenderer` 的提取是正确的瘦身方向。

---

### 架构审查结论

| 编号 | 问题 | 严重程度 | 建议处理 |
|------|------|---------|---------|
| ARCH-045-01 | DataLogger.h include 顺序违规 | 轻微 | 下次迭代修复 |
| ARCH-045-02 | XModemTransfer.h include 顺序违规 | 轻微 | 下次迭代修复 |
| ARCH-045-03 | TerminalWidget.cpp 行数临界(498/500) | 关注 | 持续监控，优先提取 |
| ARCH-045-04 | DataBookmark/BookmarkWidget/TerminalSearchRenderer 未登记到公共组件清单 | 轻微 | 本迭代登记 |

---

## 第二部分: Bug扫描与修复

### Bug扫描方法论

扫描范围覆盖 CLAUDE.md 3.4.5 规定的全部功能域:
1. 终端功能: 复制/粘贴/搜索/滚轮/选中/清屏
2. 串口功能: 连接/断开/DTR/RTS
3. OTA功能: 文件选择/传输/进度/错误处理
4. 书签功能: 添加/删除/清空/跳转
5. UI/UX: 主题切换/面板切换/动画

### Bug清单

#### P0 -- 功能完全不可用

无 P0 级 Bug 发现。

#### P1 -- 功能受限

**BUG-045-01 [P1]**: DataLogger::seekToBookmark 时间戳坐标系不一致

- **位置**: `src/utils/DataLogger.cpp` 第 446~458 行
- **描述**: `addBookmark()` 使用 `QDateTime::currentDateTime().toMSecsSinceEpoch()` 记录 Unix 纪元时间戳，而录制文件 `.edl` 中的时间戳是距录制开始的偏移量（0起始的毫秒数）。`seekToBookmark()` 直接将书签的 Unix 纪元时间戳传递给 `seekToTimestamp()`，但 `seekToTimestamp()` 期望的是录制文件内部的相对时间偏移。代码注释已承认此问题（"调用者需确保书签时间戳与录制文件时间戳处于同一时间参考系"），但未实际解决。
- **影响**: 在录制回放期间双击书签跳转功能完全失效，跳转到的时间点不正确（跳到文件开头或错误位置）。
- **修复方案**: `addBookmark()` 应记录相对于录制开始时间的偏移量，而非 Unix 纪元时间。在录制模式下，时间戳应为 `m_recordTimer.elapsed() - m_pauseOffset`；在非录制模式下，保留 Unix 纪元时间戳。或者 `seekToBookmark()` 中进行时间坐标转换。

**BUG-045-02 [P1]**: DataBookmark 书签在录制停止后不持久化

- **位置**: `src/utils/DataLogger.h/cpp`
- **描述**: `DataLogger::m_bookmarks` 仅保存在内存中的 `QVector<DataBookmark>`，录制停止（`stopRecording()`）后书签集合清空或不保存到 `.edl` 文件中。用户在录制过程中添加的书签在下次打开文件时丢失。DataBookmark 已实现 `toJson()/fromJson()` 序列化，但未被 DataLogger 使用。
- **影响**: 书签功能无法跨会话使用，用户每次打开录制文件都需重新标记书签。
- **修复方案**: 在 `stopRecording()` 中将书签序列化追加到 `.edl` 文件尾部（或在文件头中扩展书签区域），在 `startPlayback()` 中反序列化恢复书签。

**BUG-045-03 [P1]**: OtaWidget::startCompletionAnimation 使用 QPropertyAnimation("value") 作为定时器

- **位置**: `src/ota/OtaWidget.cpp` 第 407~417 行
- **描述**: 完成变色动画使用 `QPropertyAnimation(m_progressBar, "value")` 且 startValue=endValue=100，将 QPropertyAnimation 用作 400ms 定时器来延迟设置 success 颜色。这是一个 hack -- QPropertyAnimation 设计用于驱动属性变化，而非延迟执行。如果 `AnimatedProgressBar` 的 `value` 属性 setter 包含副作用（如触发 repaint），这个 hack 可能导致不必要的重绘。
- **影响**: 功能上基本正常，但代码可读性差且存在潜在的重绘浪费。
- **修复方案**: 替换为 `QTimer::singleShot(400, ...)` 或使用自定义的 `QPropertyAnimation(m_progressBar, "chunkColor")` 属性动画（AnimatedProgressBar 需要添加 chunkColor 为 Q_PROPERTY）。

**BUG-045-04 [P1]**: BookmarkWidget 清空操作无 undo 能力

- **位置**: `src/serial/BookmarkWidget.cpp` 第 217~234 行
- **描述**: 清空操作弹出 `QMessageBox::question` 确认，但 DataLogger::clearBookmarks() 直接清空 `m_bookmarks` 集合，无法恢复。如果用户误操作确认了清空，所有书签永久丢失。
- **影响**: 用户误操作清空书签后无法恢复。
- **修复方案**: 在 `clearBookmarks()` 中缓存旧书签集合，提供 undo-last-clear 能力，或至少在 DataLogger 层保留最后一次清空的数据。

#### P2 -- 体验不佳

**BUG-045-05 [P2]**: ToastWidget::dismiss 动画完成后 deleteLater 时机问题

- **位置**: `src/core/ToastWidget.h` 第 147~153 行
- **描述**: `dismiss()` 中在 `QParallelAnimationGroup::finished` 回调内调用 `deleteLater()`。但如果父窗口在动画进行中被销毁，`activeToastsMap()` 中的 key 仍指向已销毁的 parent。虽然 `ToastWidget::show()` 中连接了 `parent->destroyed` 信号清理 map，但 `repositionToasts` 仍可能在 destroyed 信号之后、deleteLater 之前被调用。
- **影响**: 极端场景下可能导致访问已销毁的 parent widget，但概率很低（需要父窗口正好在 dismiss 动画期间被销毁）。
- **修复方案**: 在 `dismiss()` 中验证 `parentWidget()` 非空后再操作 activeToasts。

**BUG-045-06 [P2]**: BookmarkWidget 列表项索引在排序后可能错位

- **位置**: `src/utils/DataLogger.cpp` 第 462~469 行 + `src/serial/BookmarkWidget.cpp` 第 100~133 行
- **描述**: `DataLogger::addBookmark()` 添加书签后调用 `std::sort(m_bookmarks.begin(), m_bookmarks.end())`，书签按时间戳升序排列。但 `BookmarkWidget::refreshBookmarks()` 中 `item->setData(Qt::UserRole, i)` 使用的是排序后的索引。如果用户快速连续添加书签（间隔很短），排序可能导致索引与添加顺序不一致，`removeBookmarkRequested(index)` 删除的可能不是用户期望的书签。
- **影响**: 快速连续添加书签时，删除操作可能删除错误的书签。
- **修复方案**: 改用书签的唯一标识（如时间戳+标签组合）而非索引来标识书签，或在 DataBookmark 中增加唯一 ID 字段。

**BUG-045-07 [P2]**: ConnectionController::connectSerial 中 DTR/RTS 设置时机

- **位置**: `src/core/ConnectionController.cpp` 第 120~123 行
- **描述**: DTR/RTS 在 `open()` 成功后立即设置，但 `open()` 可能会触发 STM32 等芯片的自动复位（DTR/RTS 下降沿）。当前实现先 `open()` 再 `setDtr/setRts`，可能导致芯片复位后立即又被 DTR/RTS 设置影响。SerialConfigPanel 中的 DTR/RTS checkbox 状态在 `connectSerial` 前 UI 层已经传递了正确的 enabled 状态。
- **影响**: 某些 STM32 开发板在连接瞬间可能因 DTR/RTS 时序问题产生不预期的复位行为。
- **修复方案**: 当前实现已将 DTR/RTS 参数从 configure 中分离，open 后再设置。如果仍有问题，可考虑在 open 前先设置线路信号。

**BUG-045-08 [P2]**: QSS 三主题文件中 BookmarkWidget 样式可能缺失

- **位置**: `resources/themes/dark_terminal.qss`, `resources/themes/light.qss`, `resources/themes/modern_dark.qss`
- **描述**: commit #44 新增了 BookmarkWidget（含 bookmarkTitleLabel/bookmarkAddBtn/bookmarkRemoveBtn/bookmarkClearBtn/bookmarkList/bookmarkWidget objectName），需确认三个主题文件都已添加对应样式。虽然 commit 记录显示 QSS 有更新（+166/+164/+168 行），但需验证 bookmarkAddDlg/bookmarkLabelInput 等对话框控件的样式是否覆盖完整。
- **影响**: 如果对话框控件样式缺失，在亮色主题下可能出现输入框/按钮视觉不一致。
- **修复方案**: 检查并补全三个主题文件中的 BookmarkWidget 相关控件样式。

**BUG-045-09 [P2]**: TerminalWidget 搜索刷新性能 -- refreshSearchAfterCacheUpdate 在 paintEvent 中调用

- **位置**: `src/terminal/TerminalWidget.cpp` 第 176~183 行
- **描述**: `refreshSearchAfterCacheUpdate()` 在 `paintEvent()` 内部被调用（第 277~278 行和第 300~301 行）。每次 paintEvent 都可能触发完整的搜索重新计算，当终端数据量大（50000行）且搜索模式为正则表达式时，可能导致帧间卡顿。
- **影响**: 大数据量+正则搜索时，滚动和数据显示可能出现帧率下降。
- **修复方案**: 将搜索刷新逻辑从 paintEvent 中移出，改为在 dataAppended 回调中增量更新搜索结果。或使用 dirty flag 标记搜索结果是否需要重新计算。

**BUG-045-10 [P2]**: NavigationController::switchToPanel 面板父容器宽度获取可能为 0

- **位置**: `src/core/NavigationController.cpp`
- **描述**: `parentContainerWidth()` 获取面板父容器宽度用于计算滑动距离。如果面板尚未被添加到布局中（如首次显示），`parentWidget()` 可能返回 nullptr 或 width() 为 0，导致面板从 (0,0) 滑到 (0,0)，无动画效果。
- **影响**: 首次启动应用时第一个面板切换可能没有滑动动画。
- **修复方案**: 在 `parentContainerWidth()` 中增加 width==0 的回退逻辑（使用窗口宽度或其他合理的默认值）。

---

### Bug修复优先级排序

| 优先级 | Bug编号 | 描述 | 建议迭代 |
|--------|---------|------|---------|
| P1 | BUG-045-01 | seekToBookmark 时间戳坐标系不一致 | 下次迭代（#46） |
| P1 | BUG-045-02 | 书签录制停止后不持久化 | 下次迭代（#46） |
| P1 | BUG-045-03 | OtaWidget 完成 Animation hack | 下次迭代（#46） |
| P1 | BUG-045-04 | BookmarkWidget 清空无 undo | 迭代 #47 |
| P2 | BUG-045-05 | ToastWidget dismiss 动画 parent 安全 | 迭代 #47 |
| P2 | BUG-045-06 | BookmarkWidget 索引排序错位 | 迭代 #47 |
| P2 | BUG-045-07 | DTR/RTS 设置时机 | 迭代 #48 |
| P2 | BUG-045-08 | QSS 三主题 BookmarkWidget 样式补全 | 迭代 #48 |
| P2 | BUG-045-09 | paintEvent 搜索刷新性能 | 迭代 #48 |
| P2 | BUG-045-10 | 面板切换首次宽度为 0 | 迭代 #48 |

---

## 第三部分: 验收标准

### 架构审查验收

- [x] 分层架构无反向依赖（确认合格，1处 include 顺序轻微违规）
- [x] 公共组件无重复造轮子（确认合格）
- [x] 8大设计模式正确应用（确认合格）
- [x] 模块间耦合度低~中等（确认合格）
- [ ] 头文件 include 顺序规范（2处轻微违规，需修复）
- [ ] 新增类登记到公共组件清单（3个新类需登记）
- [x] 目录结构按规划组织（确认合格）
- [x] 文件体积在限制内（TerminalWidget.cpp 临界，需关注）

### Bug修复验收

- [ ] BUG-045-01: seekToBookmark 时间坐标转换逻辑正确
- [ ] BUG-045-02: 书签持久化到 .edl 文件并可恢复
- [ ] BUG-045-03: OtaWidget 完成 Animation 使用正确方式实现
- [ ] BUG-045-04: clearBookmarks 支持 undo 或确认机制增强
- [ ] BUG-045-05~10: P2 Bug 按优先级逐步修复

---

## 影响范围

### 需要修改的文件

| 文件 | 修改原因 |
|------|---------|
| `src/utils/DataLogger.h` | 修复 include 顺序 + seekToBookmark 时间坐标转换 |
| `src/utils/DataLogger.cpp` | 实现书签持久化 + seekToBookmark 修复 |
| `src/ota/protocols/XModemTransfer.h` | 修复 include 顺序 |
| `src/ota/OtaWidget.cpp` | 替换 completion animation hack |
| `src/ota/AnimatedProgressBar.h` | 可选: 添加 chunkColor Q_PROPERTY |
| `CLAUDE.md` | 登记新组件到公共组件清单 |
| `resources/themes/*.qss` | 补全 BookmarkWidget 对话框控件样式 |

### 对现有功能的影响

- DataLogger 修改影响录制/回放功能，需回归测试
- OtaWidget 修改仅影响传输完成后的动画表现，不影响传输逻辑
- CLAUDE.md 变更为文档更新，不影响运行时行为

---

## 评分

当前: 44/1000
本迭代为架构审查+Bug扫描，输出 PRD 和问题清单，不产生代码变更，不加分。
下次迭代实现 Bug 修复后 +1 分。
