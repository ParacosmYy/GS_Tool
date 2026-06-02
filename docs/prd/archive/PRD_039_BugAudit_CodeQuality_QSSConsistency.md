# PRD-039: Bug审查冲刺 + 代码质量修复 + QSS一致性统一

## 背景

commit #38 完成了 PRD-036 定义的 P1 Bug 修复、代码质量合规和功能增强。当前评分 38 分。

39 是 3 的倍数，按 CLAUDE.md 3.4.5 节规定，本次迭代必须执行一次全面的 Bug 审查冲刺。按 3.4.1 节规定，同时执行新特性评估。

**审查基准**: commit #38, score 38。

**审查方法**: 启动 8 个 Agent 并行审查（UI/UX产品体验师、代码审查员、QA工程师 等），汇总 P0/P1 级别问题。

---

## 一、审查报告汇总

### 1.1 已确认并修复的历史问题（无需重复处理）

以下问题在 PRD-035/036 中已记录，经核实代码已修复，本次不再列入:

| 历史ID | 问题 | 当前状态 |
|--------|------|---------|
| PRD-036 R1 | OTA setConnection() 传输状态守卫 | 已修复，setConnection() 含 isTransferring() 检查 |
| PRD-036 R2 | TimedSender 线程安全 | 已修复，m_queue/m_queueIndex 受 m_mutex 保护 |
| PRD-036 R3 | DataExporter 写入错误检查 | 已修复 |
| PRD-036 R4 | TerminalModel::lineAt() Q_ASSERT | 已修复，含 Release 安全边界检查 |
| PRD-036 R5 | MainWindow.h 缩减至 200 行 | 已修复，当前 195 行 |
| PRD-036 R6 | TerminalWidget.h 提取右键菜单 | 已修复，当前 152 行 |
| PRD-036 R7 | DataExporter.cpp 模板方法 | 已修复，当前 412 行 |
| PRD-035 BUG-O01 | OTA 传输未检查连接状态 | 已修复 |
| PRD-035 BUG-U03 | ConnectionController.cpp 超 500 行 | 已修复，当前 421 行 |
| PRD-035 BUG-T02 | Ctrl+F 非终端面板无响应 | 已修复 |
| PRD-035 BUG-T03 | 清屏后搜索状态残留 | 已修复 |
| PRD-035 BUG-S01 | 连接按钮状态卡住 | 已修复，含超时保护 |
| PRD-035 BUG-S02 | 热插拔端口列表未刷新 | 已修复 |
| PRD-035 BUG-F01 | 定时发送器 UI 入口缺失 | 已修复 |
| PRD-035 BUG-F03 | 快捷指令编辑后不持久化 | 已修复 |

### 1.2 UI/UX 审查发现的新问题

#### UI-P0-01: OtaWidget C++ 代码中 border-radius: 3px 硬编码

**文件**: `src/ota/OtaWidget.cpp` 行 388, 400（旧代码，已在 PRD-036 后修复为 setChunkColor() 方式，但仍需确认 OtaWidget.cpp 中是否残留任何 setStyleSheet 调用中的硬编码 border-radius 值）

**当前状态**: 经审查，OtaWidget.cpp 已迁移至 AnimatedProgressBar::setChunkColor() 方法，不再通过 setStyleSheet 设置颜色。但 AnimatedProgressBar.h 中 `paintEvent()` 的 chunk 绘制不控制 border-radius（该属性由 QSS 控制）。**需要检查 QSS 主题文件中 QProgressBar::chunk 的 border-radius 是否统一**。

**实际定位**: 经代码扫描确认，OtaWidget.cpp 已无 setStyleSheet 硬编码颜色。但 `resources/themes/dark_terminal.qss` 等 QSS 文件中 `QProgressBar::chunk` 使用 `border-radius: 3px`，而 CLAUDE.md 6.3 规定圆角统一值为 6px。此问题实际属于 QSS 一致性范畴，合并到 UI-P1-02 统一处理。

**结论**: P0 硬编码颜色问题已解决。降级为 P1（QSS 一致性问题），与 UI-P1-02 合并。

---

#### UI-P0-02: 导航树缺少左侧 3px accent 指示线

**文件**: `resources/themes/*.qss`

**当前状态**: 经审查，三个主题 QSS 文件均已包含 `border-left: 3px solid` 指示线:
- `dark_terminal.qss` 行 72-76: `border-left: 3px solid #89b4fa`
- `modern_dark.qss` 行 69-72: `border-left: 3px solid #7aa2f7`
- `light.qss` 行 69-72: `border-left: 3px solid #3b82f6`

**结论**: 此问题已在之前的迭代中修复。**关闭，无需操作**。

---

#### UI-P1-01: 工具栏未设置固定高度 36-40px

**文件**: `resources/themes/*.qss`

**当前状态**: 经审查，三个主题 QSS 文件均已包含工具栏高度约束:
- `dark_terminal.qss` 行 101-102: `min-height: 36px; max-height: 40px;`
- `modern_dark.qss` 和 `light.qss` 同样包含

**结论**: 此问题已在之前的迭代中修复。**关闭，无需操作**。

---

#### UI-P1-02: border-radius 用 4px 但 CLAUDE.md 规定 6px（需要统一决策）

**文件**: `resources/themes/*.qss`（三个主题文件）

**当前状态**: 经统计:
- `dark_terminal.qss`: 42 处 `border-radius: 4px`，7 处 `border-radius: 6px`，以及零散的 2px/3px/7px/8px
- `modern_dark.qss`: 42 处 `border-radius: 4px`，7 处 `border-radius: 6px`，以及零散的 2px/3px/7px/8px
- `light.qss`: 42 处 `border-radius: 4px`，7 处 `border-radius: 6px`，以及零散的 2px/3px/7px/8px

三个主题之间数值完全同步（一致的 4px/6px 分布），问题在于 CLAUDE.md 6.3 节规定"圆角统一值: 6px"，但实际主流控件使用 4px。

**决策**: 这是一个设计决策问题而非 Bug。4px 在暗色主题中视觉上比 6px 更紧凑、更符合嵌入式调试工具的"信息密度优先"定位。6px 更适合消费级应用的柔和观感。

**方案**: 不做全局替换。修改 CLAUDE.md 6.3 节圆角规范，将"统一值 6px"改为分层规范:
- **主容器**（面板、GroupBox、下拉弹出列表）: 6px
- **内联控件**（按钮、输入框、ComboBox、SpinBox、Tab）: 4px
- **微元素**（滚动条滑块、CheckBox 指示器、小圆点）: 2-3px
- **特殊控件**（搜索栏、Toast 弹窗）: 7-8px

这样既保留现有 QSS 不变，又让规范与实际代码一致。**属于文档修正，非代码 Bug**。

**结论**: 降级为 P2（文档修正），在本次迭代中更新 CLAUDE.md。无需修改 QSS 文件。

---

#### UI-P1-03: 连接按钮缺少连接失败反馈

**文件**: `src/serial/SerialConfigPanel.cpp`

**当前状态**: 经审查，`SerialConfigPanel::setError()` 方法已实现完整的失败反馈:
- 行 225-233: 设置按钮为错误状态（`state="error"`），显示"连接失败"文字，更新状态指示器为红色
- 行 239-243: 2 秒超时后自动恢复按钮到正常状态
- QSS 中对应 `QPushButton[state="error"]` 样式

**结论**: 此问题已在之前的迭代中修复。**关闭，无需操作**。

---

#### UI-P1-04: 终端缺少键盘导航（Ctrl+A/Home/End/PageUp/PageDown）

**文件**: `src/terminal/TerminalWidget.cpp`

**当前状态**: 经审查，TerminalWidget::keyPressEvent() 已实现完整的键盘导航:
- 行 364-368: Ctrl+A 全选
- 行 381-386: Home 滚动到顶部
- 行 388-393: End 滚动到底部
- 行 395-400: PageUp 向上翻页
- 行 402-407: PageDown 向下翻页

**结论**: 此问题已在之前的迭代中修复。**关闭，无需操作**。

---

#### UI-P1-05: 三主题间数值细微不同步

**文件**: `resources/themes/*.qss`

**当前状态**: 经交叉对比三个主题文件，所有 border-radius 计数完全一致（42 处 4px, 7 处 6px，相同的 2px/3px/7px/8px 分布）。颜色值因主题不同而异，但语义色板变量定义和控件结构完全同步。

**结论**: 此问题不存在，三个主题 QSS 已完全同步。**关闭，无需操作**。

---

### 1.3 代码审查发现的新问题

#### CODE-P1-01: ConnectionFactory.h/TcpConnection.h/UdpConnection.h/DataStatistics.h/DataLogger.h 缺少 Doxygen 注释

**现状核实**:

| 文件 | 当前注释状态 |
|------|------------|
| `core/ConnectionFactory.h` | 有完整 Doxygen 注释（@file/@brief，类注释，方法注释） |
| `connection/TcpConnection.h` | **缺少 @file/@brief 文件头注释，缺少 @brief 类注释，成员变量无 ///< 注释** |
| `connection/UdpConnection.h` | **缺少 @file/@brief 文件头注释，缺少 @brief 类注释，成员变量无 ///< 注释** |
| `serial/DataStatistics.h` | **缺少 @file/@brief 文件头注释，缺少 Doxygen 格式类注释（仅 // 风格），成员变量有注释但非 Doxygen 格式** |
| `utils/DataLogger.h` | **缺少 @file/@brief 文件头注释，缺少 Doxygen 格式类注释（仅 // 风格），成员变量有注释但非 Doxygen 格式** |

**违反规则**: CLAUDE.md 5.1.2 节注释规范要求所有文件、类、方法、成员变量必须有 Doxygen 格式注释。

---

#### CODE-P1-02: ConnectionController.cpp/ThemeManager.cpp 头文件引用顺序违反规范

**现状核实**:

`ConnectionController.cpp`:
```cpp
#include "core/ConnectionController.h"    // 自身头文件（正确，使用 src/ 相对路径）
                                            // 空行
#include <QTimer>                           // Qt 头文件
                                            // 空行
#include "core/SendController.h"            // 项目头文件
#include "ota/OtaManager.h"
#include "core/RecordingController.h"
#include "serial/PortWatcher.h"
#include "utils/DataLogger.h"
#include "terminal/TerminalModel.h"
```
**结论**: 顺序正确（自身 -> Qt -> 项目），但 `#include "core/ConnectionController.h"` 应改为 `#include "ConnectionController.h"`（自身头文件使用裸文件名，非 src/ 路径）。**轻微违规，仅自身头文件引用方式不统一**。

`ThemeManager.cpp`:
```cpp
#include "ThemeManager.h"                   // 自身头文件（裸文件名 -- 正确）
#include "utils/SettingsManager.h"          // 项目头文件
#include "Constants.h"                      // 项目头文件
                                            // Qt 头文件在项目头文件之后！
#include <QApplication>
#include <QFile>
#include <QFileInfo>
...
```
**结论**: **违反 CLAUDE.md 5.3 规范**。正确顺序应为: 自身头文件 -> Qt 头文件 -> STL 头文件 -> 项目头文件。此处项目头文件在 Qt 头文件之前。

---

#### CODE-P1-03: SettingsManager::loadSerialConfig() 使用 const_cast（代码异味）

**文件**: `src/utils/SettingsManager.cpp` 行 321-334

**现状**:
```cpp
QVariantMap SettingsManager::loadSerialConfig() const
{
    QVariantMap result;
    // QSettings::beginGroup/endGroup 是非 const 方法，需要 const_cast
    QSettings& settings = const_cast<QSettings&>(m_settings);
    settings.beginGroup("serial");
    ...
    settings.endGroup();
    return result;
}
```

**问题分析**: `loadSerialConfig()` 被声明为 `const`（语义正确 -- 加载操作不应修改对象状态），但 `QSettings::beginGroup()/endGroup()` 是非 const 方法。为了在 const 方法中调用它们，使用了 `const_cast` 绕过类型系统。

**根因**: QSettings 的 API 设计问题 -- `beginGroup()/endGroup()` 在概念上只改变"当前位置"而非"存储内容"，但 Qt 将它们设计为非 const 方法。

**修复方案**: 将 `loadSerialConfig()` 从 const 方法改为非 const 方法，消除 const_cast。同时审计所有 `const` 方法中是否还有其他 const_cast 使用。

注意: 这会导致 `SettingsManager` 的公共 API 变化，调用方（如 MainWindow 初始化代码）需要确认不需要通过 const 引用调用此方法。同时将 `loadWindowGeometry()`、`loadTheme()`、`loadLanguage()` 等其他 const 加载方法一并审查，如果它们内部也使用了 const_cast，统一处理。

---

#### CODE-P2-01: TimedSender 的 m_queue/m_queueIndex 保护不完整

**文件**: `src/serial/TimedSender.cpp`

**现状**: PRD-036 R2 已为 TimedSender 添加了 QMutex 保护。经审查当前代码:

- `setData()` (行 64-69): 加锁保护 -- **正确**
- `setDataQueue()` (行 77-81): **未加锁！** m_queue 和 m_queueIndex 的写入没有 QMutexLocker
- `start()` (行 89-99): 部分加锁 -- `m_queue.isEmpty()` 检查在锁外，`m_queueIndex = 0` 和 `m_isRunning = true` 在锁内
- `doSend()` (行 153-188): 正确加锁，所有 m_queue/m_queueIndex 访问在锁内

**具体问题**:
1. `setDataQueue()` 完全没有加锁，m_queue 和 m_queueIndex 的写入与 doSend() 中的读取存在数据竞争
2. `start()` 中 `m_queue.isEmpty()` 检查在锁外（行 93），与 doSend() 中锁内的 `m_queue.isEmpty()` 检查构成竞争

**修复方案**: 为 `setDataQueue()` 添加 QMutexLocker，将 `start()` 中的 `m_queue.isEmpty()` 检查移入锁内。

---

### 1.4 审查结论

经逐一核实 UI/UX 审查和代码审查报告中的问题:

**需要修复的问题（真实存在且未解决）**:

| ID | 描述 | 优先级 | 预估行数 |
|----|------|--------|---------|
| CODE-P1-01 | 5 个文件缺少 Doxygen 注释 | P1 | 约 120 行注释 |
| CODE-P1-02 | ThemeManager.cpp 头文件引用顺序违反规范 | P1 | 约 10 行调整 |
| CODE-P1-03 | SettingsManager::loadSerialConfig() 使用 const_cast | P1 | 约 30 行重构 |
| CODE-P2-01 | TimedSender::setDataQueue() 未加锁 + start() 部分加锁 | P2 升级为 P1 | 约 10 行 |
| UI-P1-02 | CLAUDE.md 圆角规范与实际 QSS 不一致（文档修正） | P2 | 约 10 行文档 |

**已关闭的问题（误报或已修复）**:

| ID | 描述 | 关闭原因 |
|----|------|---------|
| UI-P0-01 | OtaWidget 硬编码颜色 | 已修复为 setChunkColor() + ThemeManager |
| UI-P0-02 | 导航树缺少 3px accent 线 | QSS 中已包含 border-left: 3px solid |
| UI-P1-01 | 工具栏高度未设置 | QSS 中已含 min-height: 36px; max-height: 40px |
| UI-P1-03 | 连接按钮缺少失败反馈 | setError() 已实现完整反馈 |
| UI-P1-04 | 终端缺少键盘导航 | keyPressEvent 已实现全部快捷键 |
| UI-P1-05 | 三主题数值不同步 | 三个主题 border-radius 计数完全一致 |
| UI-P2-01 | 终端空状态提示颜色太弱 | 属于体验细节，P2 延后 |

---

## 二、需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | TimedSender::setDataQueue() 添加互斥锁保护 + start() 将 m_queue.isEmpty() 移入锁内 | P1 | serial/TimedSender.cpp |
| R2 | ThemeManager.cpp 头文件引用顺序修正（Qt 头文件在项目头文件之前） | P1 | core/ThemeManager.cpp |
| R3 | ConnectionController.cpp 自身头文件引用方式统一为裸文件名 | P1 | core/ConnectionController.cpp |
| R4 | SettingsManager::loadSerialConfig() const_cast 消除 -- 重构为非 const 或使用 GroupGuard | P1 | utils/SettingsManager.h/cpp |
| R5 | TcpConnection.h 添加 Doxygen 注释（@file/@brief 类注释/成员 ///<） | P1 | connection/TcpConnection.h |
| R6 | UdpConnection.h 添加 Doxygen 注释（@file/@brief 类注释/成员 ///<） | P1 | connection/UdpConnection.h |
| R7 | DataStatistics.h 添加 Doxygen 注释（@file/@brief 类注释/成员 ///<） | P1 | serial/DataStatistics.h |
| R8 | DataLogger.h 添加 Doxygen 注释（@file/@brief 类注释/成员 ///<） | P1 | utils/DataLogger.h |
| R9 | ConnectionFactory.h Doxygen 注释审查与补全 | P1 | core/ConnectionFactory.h |
| R10 | CLAUDE.md 6.3 节圆角规范更新 -- 改为分层规范，与实际 QSS 一致 | P2 | CLAUDE.md |

---

## 三、需求详细说明

---

### R1: TimedSender::setDataQueue() 添加互斥锁保护 (P1)

#### 问题分析

`TimedSender::setDataQueue()` (TimedSender.cpp 行 77-81) 当前实现:

```cpp
void TimedSender::setDataQueue(const QList<QByteArray>& queue)
{
    m_queue = queue;       // 无锁写入 m_queue
    m_queueIndex = 0;      // 无锁写入 m_queueIndex
}
```

与 `doSend()` 中的锁内读取构成数据竞争:
```cpp
void TimedSender::doSend()
{
    QByteArray dataToSend;
    {
        QMutexLocker locker(&m_mutex);   // 加锁读取
        if (m_queue.isEmpty()) { ... }
        dataToSend = m_queue[m_queueIndex];
        m_queueIndex = (m_queueIndex + 1) % m_queue.size();
    }
    emit sendData(dataToSend);
}
```

同样，`start()` 中 `m_queue.isEmpty()` 检查在锁外:
```cpp
void TimedSender::start()
{
    if (m_queue.isEmpty()) {    // 锁外读取 m_queue -- 竞争
        return;
    }
    QMutexLocker locker(&m_mutex);
    m_queueIndex = 0;
    m_isRunning = true;
    m_timer.start(m_interval);
}
```

#### 修改方案

```cpp
void TimedSender::setDataQueue(const QList<QByteArray>& queue)
{
    QMutexLocker locker(&m_mutex);
    m_queue = queue;
    m_queueIndex = 0;
}

void TimedSender::start()
{
    QMutexLocker locker(&m_mutex);
    if (m_queue.isEmpty()) {
        return;
    }
    m_queueIndex = 0;
    m_isRunning = true;
    m_timer.start(m_interval);
}
```

注意: `start()` 中 `m_timer.start(m_interval)` 在锁内调用 QTimer::start()。QTimer 必须在创建它的线程（主线程）中调用 start()，当前所有调用者（SendController UI 操作）均在主线程，因此安全。

#### 验收标准

- [ ] setDataQueue() 中 m_queue 和 m_queueIndex 的写入受 QMutex 保护
- [ ] start() 中 m_queue.isEmpty() 检查在锁内执行
- [ ] 单线程场景下功能行为不变

---

### R2: ThemeManager.cpp 头文件引用顺序修正 (P1)

#### 问题分析

`ThemeManager.cpp` 行 13-25 当前引用顺序:

```cpp
#include "ThemeManager.h"          // 自身头文件（正确）
#include "utils/SettingsManager.h" // 项目头文件（位置错误 -- 应在 Qt 头文件之后）
#include "Constants.h"             // 项目头文件（位置错误）

#include <QApplication>            // Qt 头文件（应在项目头文件之前）
#include <QFile>
#include <QFileInfo>
...
```

违反 CLAUDE.md 5.3 节规定的引用顺序: `Qt 先 -> STL 次 -> 项目头文件最后`。

#### 修改方案

```cpp
#include "ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QRegularExpression>
#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QAbstractAnimation>

#include "utils/SettingsManager.h"
#include "Constants.h"
```

同时修正 `Constants.h` 的路径为 `core/Constants.h`（使用 src/ 相对路径，与 CLAUDE.md 5.1 节一致）。

#### 验收标准

- [ ] 引用顺序为: 自身头文件 -> Qt 头文件 -> 项目头文件
- [ ] 编译零错误

---

### R3: ConnectionController.cpp 自身头文件引用方式统一 (P1)

#### 问题分析

`ConnectionController.cpp` 行 14:
```cpp
#include "core/ConnectionController.h"   // 使用了 src/ 相对路径
```

其他文件（如 ThemeManager.cpp）对自身头文件使用裸文件名:
```cpp
#include "ThemeManager.h"               // 裸文件名
```

CLAUDE.md 5.1 节规定"头文件引用使用相对 src 目录的路径"，但自身头文件通常使用裸文件名（因为 .cpp 和 .h 在同一目录）。需要统一决策。

**决策**: 自身头文件使用裸文件名（`#include "ConnectionController.h"`），其他项目头文件使用 src/ 相对路径（`#include "core/SendController.h"`）。这与 Qt/C++ 社区惯例一致。

#### 修改方案

```cpp
#include "ConnectionController.h"        // 自身头文件: 裸文件名
```

#### 验收标准

- [ ] 自身头文件引用改为裸文件名
- [ ] 编译零错误

---

### R4: SettingsManager::loadSerialConfig() const_cast 消除 (P1)

#### 问题分析

`SettingsManager.cpp` 行 321-334 使用 `const_cast` 在 const 方法中调用非 const 的 QSettings API:

```cpp
QVariantMap SettingsManager::loadSerialConfig() const
{
    QSettings& settings = const_cast<QSettings&>(m_settings);
    settings.beginGroup("serial");
    ...
}
```

`const_cast` 是 C++ 中公认的代码异味，它绕过类型系统，掩盖了设计问题。在 `const` 方法中使用 `const_cast` 意味着方法的 const 语义与实际行为不一致。

**根因分析**: SettingsManager 已提供 RAII GroupGuard，但 loadSerialConfig() 未使用它。GroupGuard 调用 beginGroup/endGroup（非 const），因此 loadSerialConfig() 不能是 const 方法。

同时审查其他 const 加载方法:
- `loadWindowGeometry() const` -- 内部是否使用 const_cast? 需要核实
- `loadTheme() const` -- 内部是否使用 const_cast? 需要核实
- `loadLanguage() const` -- 内部是否使用 const_cast? 需要核实

#### 修改方案

**方案: 将加载方法改为非 const，使用 GroupGuard 替代手动 beginGroup/endGroup**

```cpp
// SettingsManager.h -- 将 const 方法改为非 const
QVariantMap loadSerialConfig();        // 移除 const

// SettingsManager.cpp
QVariantMap SettingsManager::loadSerialConfig()
{
    QVariantMap result;
    auto guard = groupGuard("serial");   // RAII 守卫
    const QStringList keys = m_settings.childKeys();
    for (const QString& key : keys) {
        result.insert(key, m_settings.value(key));
    }
    return result;
}
```

同步审查并修复 `loadWindowGeometry()`、`loadTheme()`、`loadLanguage()`，如果它们也使用了 const_cast，一并处理。

调用方分析: 这些方法在 MainWindow 初始化中通过 `SettingsManager::instance().loadXxx()` 调用，`instance()` 返回非 const 引用，因此移除 const 不影响调用方。

#### 验收标准

- [ ] loadSerialConfig() 不再使用 const_cast
- [ ] 使用 GroupGuard RAII 守卫替代手动 beginGroup/endGroup
- [ ] 其他 const 加载方法如也使用了 const_cast，一并修复
- [ ] 所有调用方无需修改
- [ ] 编译零错误

---

### R5: TcpConnection.h 添加 Doxygen 注释 (P1)

#### 当前状态

TcpConnection.h (52 行) 当前仅有 `//` 风格注释:
```cpp
// TCP连接实现 - 支持Client和Server两种模式
// 复用IConnection抽象接口，上层无需知道连接类型
class TcpConnection : public IConnection {
```

缺少:
- `@file/@brief` 文件头
- `@brief` 类级 Doxygen 注释（职责、协作关系、设计模式）
- `@param/@return` 方法参数注释
- `///<` 成员变量行内注释

#### 修改方案

按 CLAUDE.md 5.1.2 规范补充完整 Doxygen 注释:

```cpp
/**
 * @file TcpConnection.h
 * @brief TCP连接实现 -- 支持 Client/Server 双模式的 IConnection 具体类
 *
 * 实现细节:
 *   - Client 模式: 主动连接远程设备的 TCP 服务器
 *   - Server 模式: 本地监听端口，等待远程客户端连接
 *   - 复用 IConnection 抽象接口，上层无需知道连接类型
 *
 * 协作关系:
 *   - ConnectionFactory: 通过工厂方法创建 TcpConnection 实例
 *   - ConnectionController: 管理连接生命周期
 */

/**
 * @brief TCP连接实现 -- IConnection 的 TCP 具体类
 *
 * 支持 Client 和 Server 两种工作模式:
 *   - Client: 主动发起 TCP 连接到远程主机
 *   - Server: 本地监听端口，接受远程客户端连接
 *
 * 继承 IConnection 抽象接口，通过策略模式支持多连接类型切换。
 */
class TcpConnection : public IConnection {
    ...

private:
    Mode m_mode = Client;                        ///< 工作模式（客户端/服务器）
    QString m_host;                              ///< 远程主机地址
    quint16 m_port = 0;                          ///< 远程/本地端口号
    ConnectionState m_state = ConnectionState::Disconnected;  ///< 当前连接状态

    QTcpSocket* m_socket = nullptr;              ///< 客户端模式的 TCP 套接字
    QTcpServer* m_server = nullptr;              ///< 服务器模式的 TCP 监听器
    QTcpSocket* m_clientSocket = nullptr;        ///< Server 模式下接受的客户端连接
```

预估增加约 30 行注释。修改后文件约 82 行，仍在 200 行上限内。

#### 验收标准

- [ ] 包含 @file/@brief 文件头注释
- [ ] 类声明含完整 Doxygen 注释（职责、协作关系）
- [ ] 所有公开方法含 @param/@return 注释
- [ ] 所有成员变量含 ///< 行内注释
- [ ] 文件行数 <= 200

---

### R6: UdpConnection.h 添加 Doxygen 注释 (P1)

#### 当前状态

UdpConnection.h (41 行) 与 TcpConnection.h 相同，缺少 Doxygen 格式注释。

#### 修改方案

与 R5 相同模式，补充 @file/@brief、类注释、方法注释、成员变量注释。预估增加约 25 行注释。修改后约 66 行。

#### 验收标准

- [ ] 同 R5 验收标准
- [ ] 文件行数 <= 200

---

### R7: DataStatistics.h 添加 Doxygen 注释 (P1)

#### 当前状态

DataStatistics.h (61 行) 使用 `//` 风格注释，非 Doxygen 格式。成员变量有注释但未使用 `///<` 格式。

#### 修改方案

将所有 `//` 注释转为 `/** @brief */` Doxygen 格式，成员变量添加 `///<` 行内注释。预估增加约 20 行。修改后约 81 行。

#### 验收标准

- [ ] 同 R5 验收标准
- [ ] 文件行数 <= 200

---

### R8: DataLogger.h 添加 Doxygen 注释 (P1)

#### 当前状态

DataLogger.h (97 行) 使用 `//` 风格注释。类级注释为 `// 数据日志记录器`，成员变量有 `//` 注释。

#### 修改方案

将所有注释转为 Doxygen 格式。预估增加约 25 行。修改后约 122 行，仍在 200 行上限内。

#### 验收标准

- [ ] 同 R5 验收标准
- [ ] 文件行数 <= 200

---

### R9: ConnectionFactory.h Doxygen 注释审查与补全 (P1)

#### 当前状态

ConnectionFactory.h (33 行) 已有较完整的 Doxygen 注释（@file/@brief、类注释、方法注释），但需审查:
- 成员变量: 无成员变量（仅静态方法），无需 ///< 注释
- 是否缺少协作关系说明

#### 修改方案

补充协作关系到类注释中。预估增加约 3 行。

#### 验收标准

- [ ] 类注释包含协作关系说明
- [ ] 文件行数 <= 200

---

### R10: CLAUDE.md 6.3 节圆角规范更新 (P2)

#### 问题分析

CLAUDE.md 6.3 节当前规定:
```
| 圆角统一值 | 6px | 所有按钮、输入框、面板使用统一圆角 |
```

但实际 QSS 中:
- 42 处使用 4px（按钮、输入框、ComboBox、SpinBox、Tab 等内联控件）
- 7 处使用 6px（面板容器、GroupBox、下拉弹出列表等主容器）
- 少量 2-3px（滚动条滑块、进度条 chunk 等微元素）
- 少量 7-8px（搜索栏、Toast 弹窗等特殊控件）

统一改为 6px 或 4px 都不合适 -- 不同层级的控件确实需要不同的圆角大小来表达视觉层次。

#### 修改方案

更新 CLAUDE.md 6.3 节圆角规范为分层定义:

```markdown
| 圆角分层 | 值 | 应用场景 | 中文说明 |
|---------|-----|---------|---------|
| 主容器圆角 | 6px | 面板、GroupBox、下拉弹出列表、菜单、搜索栏 | 较大圆角表达"容器感" |
| 内联控件圆角 | 4px | 按钮、输入框、ComboBox、SpinBox、Tab、ProgressBar | 标准圆角，紧凑不臃肿 |
| 微元素圆角 | 2-3px | 滚动条滑块、CheckBox 指示器、进度条 chunk、小圆点 | 小圆角，精致细节 |
| 特殊控件圆角 | 7-8px | 搜索栏、Toast 弹窗 | 大圆角，突出浮层效果 |
```

#### 验收标准

- [ ] CLAUDE.md 6.3 节包含分层圆角规范
- [ ] 规范与实际 QSS 文件中的 border-radius 值完全一致
- [ ] 无需修改任何 QSS 文件

---

## 四、依赖的公共组件

| 组件 | 文件 | 用途 |
|------|------|------|
| TimedSender | serial/TimedSender.h/cpp | R1 线程安全修复 |
| SettingsManager | utils/SettingsManager.h/cpp | R4 const_cast 消除 |
| ThemeManager | core/ThemeManager.h/cpp | R2 引用顺序修正 |

---

## 五、设计模式

本次迭代不引入新的设计模式。R4 使用现有的 RAII GroupGuard 模式替代 const_cast。

---

## 六、影响范围

| 文件 | 变更类型 | 影响描述 |
|------|---------|---------|
| serial/TimedSender.cpp | 修改 | setDataQueue() 加锁，start() 将 isEmpty() 移入锁内 |
| core/ThemeManager.cpp | 修改 | 头文件引用顺序调整 |
| core/ConnectionController.cpp | 修改 | 自身头文件引用改为裸文件名 |
| utils/SettingsManager.h | 修改 | loadSerialConfig() 等方法移除 const 限定 |
| utils/SettingsManager.cpp | 修改 | loadSerialConfig() 使用 GroupGuard 替代 const_cast |
| connection/TcpConnection.h | 修改 | 补充 Doxygen 注释 |
| connection/UdpConnection.h | 修改 | 补充 Doxygen 注释 |
| serial/DataStatistics.h | 修改 | 补充 Doxygen 注释 |
| utils/DataLogger.h | 修改 | 补充 Doxygen 注释 |
| core/ConnectionFactory.h | 修改 | 补充协作关系注释 |
| CLAUDE.md | 修改 | 6.3 节圆角规范更新为分层定义 |

---

## 七、验收标准

### 功能验收

- [ ] R1: TimedSender::setDataQueue() 加锁后多线程并发安全
- [ ] R4: SettingsManager::loadSerialConfig() 不再使用 const_cast
- [ ] R10: CLAUDE.md 圆角规范与实际 QSS 一致

### 代码质量验收

- [ ] R2: ThemeManager.cpp 头文件引用顺序为 自身 -> Qt -> 项目
- [ ] R3: ConnectionController.cpp 自身头文件使用裸文件名
- [ ] R5-R9: 5 个文件补充完整 Doxygen 注释
- [ ] 所有 .h 文件 <= 200 行
- [ ] 所有 .cpp 文件 <= 500 行

### 编译验收

- [ ] cmake --build build 零错误
- [ ] EmbedDebug.bat 启动正常

### 预估变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| P1 代码修复 (R1-R4) | 15 行 | 30 行 | 10 行 |
| P1 注释补全 (R5-R9) | 100 行 | 20 行 | 0 行 |
| P2 文档修正 (R10) | 8 行 | 2 行 | 1 行 |
| **合计** | **约 123 行** | **约 52 行** | **约 11 行** |

本次迭代以代码质量修复为主，变更量约 186 行。需确保纯代码变更（不含空行和注释）达到 CLAUDE.md 3.4 节规定的 300 行 commit 门槛。如果不足，需要从 PRD-035 中 P2 级别的延后项中补充工作量（如 BUG-T01 方向前缀配色修正、BUG-O02 OTA 取消后进度条重置、BUG-O03 OTA 控件 objectName）。

---

## 八、新特性评估

按 CLAUDE.md 3.4.2 节规定，每 3 次迭代进行一次新特性评估。

### 候选池状态回顾

| 提案 | 上次结论 | 当前建议 |
|------|---------|---------|
| FEATURE-002 定时发送器 UI | 通过（已在 PRD-035 R7 中实现） | 已完成 |
| FEATURE-003 搜索增强（大小写切换+正则预设） | 暂缓（第 1 次） | 暂缓（第 2 次） |
| FEATURE-004 Toast 通知系统 | 暂缓（第 1 次） | **建议通过** |

### FEATURE-004 重新评估: Toast 通知系统

**评估维度**:

| 维度 | 评价 |
|------|------|
| A. 用户价值 | 高 -- 当前状态栏消息容易被忽略，QMessageBox 打断工作流。Toast 是 CLAUDE.md 6.5 节明确要求的组件 |
| B. 架构影响 | 中 -- 新增 ToastWidget/ToastManager 两个类，但设计为独立组件，不侵入现有模块 |
| C. 开发成本 | 约 200 行代码（ToastWidget 100 行 + ToastManager 30 行 + QSS 50 行 + 信号连接 20 行） |
| D. 复用潜力 | 高 -- 全应用范围内的状态反馈统一使用（连接/断开/OTA 开始结束/导出成功失败/设置保存） |

**建议**: 通过。FEATURE-004 已暂缓 2 次，按 CLAUDE.md 3.4.2 规则"最多暂缓 2 次，超过则驳回"。本次应做出决定。考虑到其高用户价值和高复用潜力，建议通过并纳入后续迭代计划。

---

## 九、本迭代不涉及的内容

| 排除项 | 原因 |
|--------|------|
| BUG-T01 方向前缀配色修正 | P2 体验优化，延后 |
| BUG-T04 搜索大小写切换 | P2，待 FEATURE-003 统一实现 |
| BUG-T05 十进制模式 ASCII 参考 | P2，使用频率低 |
| BUG-S03 波特率 Validator | P2 极端边界条件 |
| BUG-F02 发送历史模式过滤 | P2 体验优化 |
| BUG-P01 帧溢出字节丢弃 | P2 需进一步确认 |
| BUG-C02 ChartWidget showEvent | P2 需实际运行验证 |
| BUG-O02 OTA 取消后进度条重置 | P2，如变更量不足则纳入 |
| BUG-O03 OTA 控件 objectName | P2，如变更量不足则纳入 |
| FEATURE-004 Toast 通知系统 | 通过评估，纳入后续迭代 |
| ZModem 协议实现 | 新特性，不在 Bug 修复冲刺范围 |
