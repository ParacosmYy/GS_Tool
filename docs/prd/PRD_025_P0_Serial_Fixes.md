# PRD-025: P0 串口功能修复与增强

## 背景

commit #24 已完成 ProtocolBridgeManager 协议源选择器、objectName 审计补全、QSS 交互状态补全、ConnectionController 悬空指针修复和桥接单元测试，当前评分 26 分。

本次迭代 #25 是 Bug 修复冲刺迭代（按 CLAUDE.md 3.4.5 节规定，每 3 次迭代执行一次全面 Bug 审查和修复）。经全面扫描终端功能、串口功能、发送功能三大核心区域后，发现以下 P0/P1 级别问题:

**P0 Bug 列表**:

1. **终端复制功能完全失效** -- `TerminalWidget::selectedText()` 原实现访问 `m_model->lineAt()` 取文本，但 commit #22 环形缓存重构后 `lineAt()` 返回的是原始 `TerminalLine`（含 `QByteArray data`），而非格式化后的显示文本。选中后 Ctrl+C 复制到剪贴板的内容为空字符串，用户无法从终端复制任何内容。

2. **搜索栏与终端完全断开** -- `TerminalSearchBar` 的 `searchRequested` 信号虽已连接到 MainWindow 槽函数，但 TerminalWidget 缺少 `setSearchHighlight()`/`clearSearchHighlight()`/`gotoNextMatch()`/`gotoPrevMatch()` 等搜索方法的完整实现，搜索栏形同虚设，输入关键词后终端无任何高亮反馈。

3. **DTR/RTS 连接后不可控** -- `SerialConfigPanel::setConnected()` 在连接后将所有控件（包括 DTR/RTS 复选框）设置为 `setEnabled(false)`，导致用户无法在运行时切换 DTR/RTS 电平。嵌入式开发中 DTR/RTS 常用于复位控制（如 ESP32 进入下载模式），连接后不可控是功能完全不可用的 P0 问题。

**P1 功能增强**:

4. **发送数据无换行符选项** -- 串口通信中绝大多数设备需要 `\r\n` 或 `\n` 作为命令终止符，但发送栏缺少自动追加换行符的选项，用户每次手动输入换行符极易遗漏且低效。

5. **CLAUDE.md 流程更新** -- Bug 修复冲刺机制需要在项目约束文档中正式确立，同时补充 VOFA+ 超越目标和串口驱动检测需求到新特性候选池。

**审查基准**: commit #24, score 26。

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | TerminalWidget::selectedText() 修复 -- 从缓存行提取格式化文本 | P0 | terminal/TerminalWidget.h/cpp |
| R2 | 搜索高亮完整实现 -- 匹配追踪、正则/纯文本/HEX 模式、F3 导航、匹配计数显示、自动滚动、新数据刷新 | P0 | terminal/TerminalWidget.h/cpp, terminal/TerminalSearchBar.h/cpp |
| R3 | DTR/RTS 运行时控制 -- 连接后保持可切换，信号透传至 SerialConnection | P0 | serial/SerialConfigPanel.cpp, core/ConnectionController.h/cpp, core/MainWindow.cpp |
| R4 | 自动追加换行符 -- NewlineCombo 下拉框，选项: 无/\r\n/\n/\r | P1 | core/SendController.h/cpp |
| R5 | CLAUDE.md 更新 -- Bug 修复冲刺机制、VOFA+ 超越目标、串口驱动检测需求 | P1 | CLAUDE.md |

---

## 需求详细说明

---

### R1: TerminalWidget::selectedText() 修复 (P0)

#### 问题分析

原实现:

```cpp
// 旧代码（commit #22 之前）
QString TerminalWidget::selectedText() const
{
    // ... 遍历 m_model->lineAt(i) 取文本
}
```

commit #22 环形缓存重构后，`m_model->lineAt(i)` 返回 `TerminalLine` 结构体（含 `QByteArray data` 和 `DataDirection direction`），而非格式化后的显示文本。此外 `selectedText()` 直接从 model 取数据，没有经过 `DisplayMode`（Text/Hex/Mixed）格式化，导致:

1. HEX 模式下选中后复制的是原始二进制（不可读），而非 HEX 字符串
2. 混合模式下选中后复制的是原始数据，而非 "文本 | HEX" 的混合格式
3. 时间戳不会被包含在复制内容中

#### 方案设计

`TerminalWidget` 已维护 `m_cachedLines`（`QVector<CachedLine>`），其中 `CachedLine::text` 字段即为经过 `DisplayMode` 格式化后的文本。`selectedText()` 直接从缓存行提取即可:

```cpp
QString TerminalWidget::selectedText() const
{
    if (m_selectionStartLine < 0 || m_selectionEndLine < 0) return {};
    if (m_cachedLines.isEmpty()) return {};

    int start = qMin(m_selectionStartLine, m_selectionEndLine);
    int end = qMax(m_selectionStartLine, m_selectionEndLine);
    end = qMin(end, m_cachedLines.size() - 1);
    if (start >= m_cachedLines.size()) return {};

    QStringList lines;
    for (int i = start; i <= end; ++i) {
        lines << m_cachedLines[i].text;
    }
    return lines.join('\n');
}
```

**设计决策**:

1. **从缓存读取而非 model**: 缓存中的 `text` 已经过 `formatToCache()` 格式化，与 paintEvent 绘制的内容完全一致，所见即所得。避免 `selectedText()` 重复实现格式化逻辑。
2. **边界安全检查**: `start`/`end` 越界时返回空串，不会崩溃。环形缓冲区回绕时缓存会被清空重建（见 `paintEvent` 中的回绕检测逻辑），此处 `m_cachedLines` 始终是有效的。

---

### R2: 搜索高亮完整实现 (P0)

#### 问题分析

TerminalSearchBar 已有完整的 UI 和信号:

- `searchRequested(pattern, regex, hex)` -- 搜索请求信号
- `searchCleared()` -- 清除信号
- `setResultText(text)` -- 设置结果标签

但 TerminalWidget 缺少完整的搜索后端:

1. 无匹配存储结构 -- 无法记录哪些行哪些列有匹配
2. 无高亮渲染 -- paintEvent 中没有搜索高亮绘制逻辑
3. 无导航机制 -- 无法 F3 跳转到上/下一个匹配
4. 无自动刷新 -- 新数据到达后不更新搜索结果
5. 无自动滚动 -- 匹配项不在可见区域时不会自动滚动

#### 方案设计

##### 数据结构

```cpp
// TerminalWidget.h 新增:

// 搜索高亮
struct SearchMatch { int line; int startCol; int length; };
QVector<SearchMatch> m_searchMatches;
int m_currentMatchIndex = -1;
QString m_searchPattern;
bool m_searchRegex = false;
bool m_searchHex = false;
QColor m_searchHighlightColor;      // 所有匹配背景色
QColor m_currentMatchColor;         // 当前匹配背景色（更亮）
```

##### 搜索方法

```cpp
// 设置搜索高亮 -- 根据模式扫描所有缓存行
void TerminalWidget::setSearchHighlight(const QString& pattern, bool regex, bool hex);

// 清除搜索高亮
void TerminalWidget::clearSearchHighlight();

// 跳转到下一个/上一个匹配（F3/Shift+F3）
void TerminalWidget::gotoNextMatch();
void TerminalWidget::gotoPrevMatch();

// 滚动到指定匹配行（不在可见区域时自动滚动）
void TerminalWidget::scrollToMatch(int line);

// 缓存更新后重新搜索
void TerminalWidget::refreshSearch();
```

##### 搜索逻辑 -- 三种模式

**纯文本模式** (`regex=false, hex=false`):
- 对每个 `m_cachedLines[i].text` 执行 `QString::indexOf()` 全文扫描
- 记录每个匹配的 `{line, startCol, length}`
- 大小写敏感（串口协议通常区分大小写）

**正则模式** (`regex=true`):
- 使用 `QRegularExpression::globalMatch()` 扫描每个缓存行
- 支持所有标准正则语法（量词、字符类、分组等）
- 正则不合法时不执行搜索，保持上次结果

**HEX 模式** (`hex=true`):
- 使用 `HexConverter::fromHexString()` 将用户输入转为字节
- 在每行原始数据（`m_model->lineAt(i).data`）的 HEX 表示中搜索
- 支持 HEX 输入验证（非法 HEX 显示"非法HEX"提示）

##### 高亮渲染

在 `paintEvent` 中，绘制文字前先绘制搜索高亮背景:

```cpp
// 搜索高亮: 绘制匹配区域背景
for (int mi = 0; mi < m_searchMatches.size(); ++mi) {
    const auto& match = m_searchMatches[mi];
    if (match.line != i) continue;
    int xStart = xOffset + 4 + fm.horizontalAdvance(cached.text.left(match.startCol));
    int matchWidth = fm.horizontalAdvance(cached.text.mid(match.startCol, match.length));
    QColor color = (mi == m_currentMatchIndex)
        ? m_currentMatchColor : m_searchHighlightColor;
    painter.fillRect(xStart, y + 2, matchWidth, m_lineHeight - 4, color);
}
```

**配色方案**:
- `m_searchHighlightColor`: `rgba(249, 226, 175, 80)` -- 半透明黄色，不遮挡文字
- `m_currentMatchColor`: `rgba(249, 226, 175, 180)` -- 更高不透明度的黄色，当前匹配更醒目

##### 信号连接链路

```
TerminalSearchBar::searchRequested(pattern, regex, hex)
    -> MainWindow::onSearchRequested()
    -> TerminalWidget::setSearchHighlight(pattern, regex, hex)
    -> TerminalWidget::searchMatchesChanged(total, current)  // 信号
    -> MainWindow::lambda [lambda 转发匹配计数]
    -> TerminalSearchBar::setResultText("3/15")

TerminalSearchBar::searchCleared()
    -> MainWindow::onSearchCleared()
    -> TerminalWidget::clearSearchHighlight()

TerminalSearchBar::closed()
    -> TerminalWidget::clearSearchHighlight()

F3 / Shift+F3 (TerminalWidget::keyPressEvent)
    -> TerminalWidget::gotoNextMatch() / gotoPrevMatch()
    -> TerminalWidget::searchMatchesChanged(total, current)
    -> MainWindow -> TerminalSearchBar::setResultText()
```

##### 自动刷新机制

新数据到达时 `onDataAppended()` 触发，检测是否有活跃搜索模式:

```cpp
void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    // ... 滚动更新 ...

    // 有活跃搜索时，缓存更新后重新搜索
    if (!m_searchPattern.isEmpty()) {
        refreshSearch();
    }

    update();
}
```

`refreshSearch()` 保存当前搜索参数，调用 `setSearchHighlight()` 重新扫描全部缓存行，保持 `m_currentMatchIndex` 不变（如果仍在有效范围内）。

---

### R3: DTR/RTS 运行时控制 (P0)

#### 问题分析

`SerialConfigPanel::setConnected()` 原实现:

```cpp
void SerialConfigPanel::setConnected(bool connected)
{
    // ... 其他控件 setEnabled(!connected) ...
    m_dtrCheck->setEnabled(!connected);   // BUG: 连接后 DTR 灰掉
    m_rtsCheck->setEnabled(!connected);   // BUG: 连接后 RTS 灰掉
}
```

在嵌入式开发中，DTR/RTS 信号常用于:
- **ESP32/ESP8266**: DTR+RTS 组合控制 EN/IO0 引脚，进入下载模式或复位
- **STM32**: DTR 控制 BOOT0 引脚，RTS 控制复位引脚
- **Arduino**: DTR 触发自动复位

连接后 DTR/RTS 不可控意味着这些功能完全无法使用。

#### 方案设计

##### SerialConfigPanel 修复

```cpp
void SerialConfigPanel::setConnected(bool connected)
{
    // ... 其他控件 setEnabled(!connected) ...
    // DTR/RTS 保持可用，允许连接后实时切换
    m_dtrCheck->setEnabled(true);
    m_rtsCheck->setEnabled(true);
}
```

##### 信号连接链路

```
SerialConfigPanel::dtrChanged(bool enabled)   // QCheckBox::toggled 信号
    -> ConnectionController::setDtr(bool enabled)
    -> qobject_cast<SerialConnection*>(m_currentConn)
    -> SerialConnection::setDtr(bool enabled)
    -> QSerialPort::setDataTerminalReady(enabled)

SerialConfigPanel::rtsChanged(bool enabled)
    -> ConnectionController::setRts(bool enabled)
    -> qobject_cast<SerialConnection*>(m_currentConn)
    -> SerialConnection::setRts(bool enabled)
    -> QSerialPort::setRequestToSend(enabled)
```

MainWindow::connectSignals() 中新增连接:

```cpp
// DTR/RTS运行时控制
connect(m_serialConfig, &SerialConfigPanel::dtrChanged,
        m_connController, &ConnectionController::setDtr);
connect(m_serialConfig, &SerialConfigPanel::rtsChanged,
        m_connController, &ConnectionController::setRts);
```

ConnectionController 新增方法:

```cpp
void ConnectionController::setDtr(bool enabled)
{
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial) {
        auto* serial = qobject_cast<SerialConnection*>(m_currentConn);
        if (serial) serial->setDtr(enabled);
    }
}

void ConnectionController::setRts(bool enabled)
{
    if (m_currentConn && m_currentConn->type() == ConnectionType::Serial) {
        auto* serial = qobject_cast<SerialConnection*>(m_currentConn);
        if (serial) serial->setRts(enabled);
    }
}
```

**安全性**: `setDtr`/`setRts` 仅对串口连接生效，网络连接（TCP/UDP）下 `m_currentConn->type() != Serial`，条件判断直接跳过，不会触发错误。

---

### R4: 自动追加换行符 (P1)

#### 问题分析

串口通信中绝大多数设备需要换行符作为命令终止符:

- **AT 指令**: `AT\r\n`（大多数调制解调器和蓝牙模块）
- **Linux 设备**: `ls\n`（Unix 换行）
- **旧式设备**: `command\r`（Mac 经典换行）

当前用户每次发送都需要手动在输入框中输入 `\r\n` 或 `\n`，容易遗漏且操作低效。

#### 方案设计

##### NewlineCombo 下拉框

在 SendController 的发送栏中新增 `m_newlineCombo`（`QComboBox`），位于发送模式选择框和输入框之间:

```
[文本 v] [无 v] [输入要发送的数据...            ] [发送]
```

选项:

| 索引 | 显示文本 | 追加内容 | 说明 |
|------|---------|---------|------|
| 0 | 无 | (不追加) | 默认，保持原有行为 |
| 1 | \r\n | `\r\n` | Windows/AT 指令标准换行 |
| 2 | \n | `\n` | Unix/Linux 换行 |
| 3 | \r | `\r` | 旧式 Mac 换行 |

##### 追加逻辑

```cpp
void SendController::onSendData()
{
    // ... 解析输入为 data ...

    // 追加换行符（仅文本模式下生效）
    if (!isHex && m_newlineCombo && m_newlineCombo->currentIndex() > 0) {
        switch (m_newlineCombo->currentIndex()) {
        case 1: data.append("\r\n"); break;
        case 2: data.append("\n"); break;
        case 3: data.append("\r"); break;
        }
    }

    // ... sendAndRecord(data) ...
}
```

**设计决策**:

1. **仅文本模式下追加**: HEX 模式下用户输入的是精确的字节序列，自动追加换行符会破坏数据的精确性。
2. **追加而非替换**: 在用户输入的文本末尾追加换行符，不修改输入框内容。
3. **默认"无"**: 向后兼容，不改变现有用户行为。

---

### R5: CLAUDE.md 更新 (P1)

#### 3.4.5 节: Bug 修复冲刺机制

在 CLAUDE.md 第 3.4 节之后新增 3.4.5 小节，正式确立 Bug 修复冲刺流程:

- 每 3 次 commit 执行一次全面 Bug 审查
- 优先级分类: P0(功能完全不可用) > P1(功能受限) > P2(体验不佳)
- P0 和 P1 必须修复，新特性开发暂停
- 审查范围覆盖: 终端功能、串口功能、发送功能、波形图、OTA、UI/UX、配置持久化
- 记录到 `docs/prd/BUG_AUDIT_xxx.md`

#### 新特性候选池更新

1. **VOFA+ 超越目标**: 在 3.4.4 参考标杆产品对照表中，将 VOFA+ 的借鉴说明更新为包含"**目标: 超越 VOFA+ 的波形体验**"
2. **串口驱动检测**: 在候选池中将"串口驱动检测"从 P2 提升描述，明确需求: 启动时检测系统是否安装了串口驱动（CH340/CP2102/FT232/PL2303 等），无驱动时给出安装提示

---

## 接口设计

### 新增接口

| 接口 | 文件 | 说明 |
|------|------|------|
| `TerminalWidget::setSearchHighlight()` | terminal/TerminalWidget.h/cpp (修改) | 设置搜索高亮模式并扫描匹配 |
| `TerminalWidget::clearSearchHighlight()` | terminal/TerminalWidget.h/cpp (修改) | 清除搜索高亮 |
| `TerminalWidget::searchMatchCount()` | terminal/TerminalWidget.h/cpp (修改) | 返回匹配总数 |
| `TerminalWidget::currentMatchIndex()` | terminal/TerminalWidget.h/cpp (修改) | 返回当前匹配索引 |
| `TerminalWidget::gotoNextMatch()` | terminal/TerminalWidget.h/cpp (修改) | 跳转到下一个匹配 |
| `TerminalWidget::gotoPrevMatch()` | terminal/TerminalWidget.h/cpp (修改) | 跳转到上一个匹配 |
| `TerminalWidget::searchMatchesChanged` | terminal/TerminalWidget.h (修改) | 匹配结果变更信号 |
| `TerminalWidget::scrollToMatch()` | terminal/TerminalWidget.cpp (修改) | 滚动到指定匹配行 |
| `TerminalWidget::refreshSearch()` | terminal/TerminalWidget.cpp (修改) | 缓存更新后重新搜索 |
| `TerminalWidget::formatToCache()` | terminal/TerminalWidget.cpp (修改) | 将数据行转换为缓存结构 |
| `ConnectionController::setDtr()` | core/ConnectionController.h/cpp (修改) | 运行时 DTR 控制 |
| `ConnectionController::setRts()` | core/ConnectionController.h/cpp (修改) | 运行时 RTS 控制 |
| `SerialConfigPanel::dtrChanged` | serial/SerialConfigPanel.h (修改) | DTR 变更信号 |
| `SerialConfigPanel::rtsChanged` | serial/SerialConfigPanel.h (修改) | RTS 变更信号 |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| `TerminalWidget::selectedText()` | 逻辑重写: 从缓存行提取而非 model | 无外部影响，返回值语义不变 |
| `TerminalWidget::paintEvent()` | 新增搜索高亮渲染代码段 | 无外部影响，不影响现有渲染流程 |
| `TerminalWidget::keyPressEvent()` | 新增 F3/Shift+F3 搜索导航 | 无外部影响，不影响现有键盘事件 |
| `TerminalWidget::onDataAppended()` | 新增搜索自动刷新逻辑 | 无外部影响 |
| `SerialConfigPanel::setConnected()` | DTR/RTS 保持启用 | 行为变更: 连接后 DTR/RTS 不再灰掉（预期） |
| `SendController::createSendBar()` | 新增 NewlineCombo 控件 | UI 变更: 发送栏多一个下拉框 |
| `SendController::onSendData()` | 新增换行符追加逻辑 | 行为变更: 文本模式下自动追加选定换行符 |
| `MainWindow::connectSignals()` | 新增搜索/DTR/RTS 信号连接 | 无外部影响 |

### 移除接口

无。所有现有接口保留，仅变更内部实现。

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| `HexConverter` | utils/HexConverter.h | HEX 搜索模式下的编解码和验证 | R2 |
| `TerminalModel` | terminal/TerminalModel.h | 搜索时访问原始数据（HEX 模式） | R1, R2 |
| `CachedLine` | terminal/TerminalWidget.h | selectedText 和搜索扫描的数据源 | R1, R2 |
| `SerialConnection` | connection/SerialConnection.h | DTR/RTS 底层控制 | R3 |
| `ConnectionController` | core/ConnectionController.h | DTR/RTS 信号中转 | R3 |
| `SendController` | core/SendController.h | 换行符追加逻辑宿主 | R4 |
| `TerminalSearchBar` | terminal/TerminalSearchBar.h | 搜索信号源和结果展示 | R2 |
| `ThemeManager` | core/ThemeManager.h | 搜索高亮配色获取 | R2 |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **观察者模式 (Observer)** | 搜索信号链: SearchBar -> MainWindow -> TerminalWidget -> searchMatchesChanged -> MainWindow -> SearchBar | R2 | Qt 信号/槽链路，MainWindow 充当中介者 |
| **状态模式 (State)** | 搜索模式切换（纯文本/正则/HEX）影响搜索行为 | R2 | 通过 `m_searchRegex`/`m_searchHex` 标志位驱动分支 |
| **策略模式 (Strategy)** | DTR/RTS 控制通过 IConnection 多态分发，仅 Serial 类型执行 | R3 | ConnectionController::setDtr() 使用 qobject_cast 类型检查 |

---

## 影响范围

### 文件变更矩阵

| 文件 | 变更类型 | R1 | R2 | R3 | R4 | R5 |
|------|---------|-----|-----|-----|-----|-----|
| `src/terminal/TerminalWidget.h` | 修改 | +5 行 | +30 行 | -- | -- | -- |
| `src/terminal/TerminalWidget.cpp` | 修改 | 重写 10 行 | +200 行 | -- | -- | -- |
| `src/terminal/TerminalSearchBar.h` | 无变更 | -- | -- | -- | -- | -- |
| `src/terminal/TerminalSearchBar.cpp` | 无变更 | -- | -- | -- | -- | -- |
| `src/serial/SerialConfigPanel.h` | 修改 | -- | -- | +2 行 | -- | -- |
| `src/serial/SerialConfigPanel.cpp` | 修改 | -- | -- | +5 行 | -- | -- |
| `src/core/ConnectionController.h` | 修改 | -- | -- | +3 行 | -- | -- |
| `src/core/ConnectionController.cpp` | 修改 | -- | -- | +15 行 | -- | -- |
| `src/core/SendController.h` | 修改 | -- | -- | -- | +2 行 | -- |
| `src/core/SendController.cpp` | 修改 | -- | -- | -- | +20 行 | -- |
| `src/core/MainWindow.cpp` | 修改 | -- | +10 行 | +5 行 | -- | -- |
| `CLAUDE.md` | 修改 | -- | -- | -- | -- | +40 行 |

### 预计变更量

| 类别 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| TerminalWidget.h | 35 行 | -- | -- |
| TerminalWidget.cpp | 200 行 | 15 行 | 10 行 |
| SerialConfigPanel.h/cpp | 7 行 | 2 行 | -- |
| ConnectionController.h/cpp | 18 行 | -- | -- |
| SendController.h/cpp | 22 行 | -- | -- |
| MainWindow.cpp | 15 行 | -- | -- |
| CLAUDE.md | 40 行 | 5 行 | -- |
| **合计** | **约 337 行** | **约 22 行** | **约 10 行** |

### 跨模块影响评估

- **TerminalWidget 与 TerminalSearchBar**: TerminalWidget 新增搜索方法后，TerminalSearchBar 的信号（`searchRequested`/`searchCleared`）通过 MainWindow 槽函数连接到 TerminalWidget 的搜索方法。TerminalSearchBar 本身不变更。
- **TerminalWidget 与 TerminalModel**: 搜索的 HEX 模式需要访问 `m_model->lineAt(i).data` 获取原始字节，纯文本和正则模式仅使用 `m_cachedLines[i].text`。TerminalModel 不变更。
- **SerialConfigPanel 与 ConnectionController**: DTR/RTS 的 `toggled` 信号通过 `dtrChanged`/`rtsChanged` 信号传递到 ConnectionController，再通过 `qobject_cast<SerialConnection*>` 透传到 QSerialPort。
- **SendController 的 NewlineCombo**: 纯 UI 层变更，不影响数据层的 `sendAndRecord()` 方法签名。

---

## 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | 终端选中后 Ctrl+C 可复制文本 | 手动: 接收数据 -> 鼠标选中多行 -> Ctrl+C -> 粘贴到记事本 | 粘贴内容与终端显示一致 |
| R1-AC2 | HEX 模式下复制的是 HEX 文本 | 手动: 切换到 HEX 显示 -> 选中 -> Ctrl+C | 粘贴内容为 HEX 格式字符串 |
| R1-AC3 | 混合模式下复制的是混合格式 | 手动: 切换到混合显示 -> 选中 -> Ctrl+C | 粘贴内容为 "文本 \| HEX" 格式 |
| R1-AC4 | 无数据时复制不崩溃 | 手动: 终端无数据 -> Ctrl+C | 无崩溃，无异常 |
| R2-AC1 | Ctrl+F 激活搜索栏 | 手动: Ctrl+F | 搜索栏展开动画，输入框获得焦点 |
| R2-AC2 | 纯文本搜索显示高亮 | 手动: 输入关键词 -> 回车 | 匹配项显示黄色高亮背景 |
| R2-AC3 | 正则搜索正常工作 | 手动: 勾选"正则" -> 输入 `\d+` | 所有数字序列高亮 |
| R2-AC4 | HEX 搜索正常工作 | 手动: 勾选"HEX" -> 输入 "AA 55" | 匹配字节序列高亮 |
| R2-AC5 | F3 跳转到下一个匹配 | 手动: 有多个匹配时按 F3 | 当前匹配切换，视图跟随滚动 |
| R2-AC6 | Shift+F3 跳转到上一个匹配 | 手动: 按 Shift+F3 | 当前匹配向上切换 |
| R2-AC7 | 匹配计数显示正确 | 手动: 搜索 "hello" 出现 5 次 | 结果标签显示 "1/5" -> "2/5" ... |
| R2-AC8 | 新数据到达自动刷新搜索 | 手动: 搜索关键词 -> 继续接收数据 | 高亮结果实时更新 |
| R2-AC9 | Esc 关闭搜索栏并清除高亮 | 手动: Esc | 搜索栏收起动画，高亮消失 |
| R2-AC10 | 非法 HEX 输入显示错误提示 | 手动: 勾选 HEX -> 输入 "GG" | 显示"非法HEX"提示 |
| R2-AC11 | 非法正则不崩溃 | 手动: 勾选正则 -> 输入 `[` | 无崩溃，无搜索结果 |
| R3-AC1 | 连接后 DTR/RTS 复选框保持可用 | 手动: 连接串口 | DTR/RTS 复选框非灰色 |
| R3-AC2 | 运行时切换 DTR 生效 | 手动: 连接后取消/勾选 DTR | 目标设备 DTR 电平变化（如 ESP32 复位） |
| R3-AC3 | 运行时切换 RTS 生效 | 手动: 连接后取消/勾选 RTS | 目标设备 RTS 电平变化 |
| R3-AC4 | 网络连接下 DTR/RTS 不报错 | 手动: TCP 连接 -> 点击 DTR | 无崩溃，静默忽略 |
| R4-AC1 | NewlineCombo 显示在发送栏 | 手动: 查看发送区域 | 看到"无/\r\n/\n/\r"下拉框 |
| R4-AC2 | 选择 \r\n 发送带换行 | 手动: 选择 "\r\n" -> 发送 "AT" | 串口实际发出 "AT\r\n" |
| R4-AC3 | 选择"无"不追加换行 | 手动: 选择"无" -> 发送 "test" | 串口实际发出 "test"（无换行符） |
| R4-AC4 | HEX 模式下不追加换行 | 手动: 切换 HEX 模式 -> 选择 "\r\n" -> 发送 "AA 55" | 串口实际发出 `0xAA 0x55`（无追加） |
| R5-AC1 | CLAUDE.md 包含 3.4.5 Bug 修复冲刺 | 代码检查 | 存在 3.4.5 节及完整流程描述 |
| R5-AC2 | VOFA+ 参考包含超越目标 | 代码检查 | 标杆表 VOFA+ 行包含"目标: 超越" |
| R5-AC3 | 串口驱动检测需求已补充 | 代码检查 | 候选池中描述包含驱动检测需求 |
| AC-1 | 编译零错误零警告 | `cmake --build build` | 0 error, 0 warning |
| AC-2 | EmbedDebug.bat 正常启动 | 双击 EmbedDebug.bat | 应用窗口正常显示 |
| AC-3 | 现有功能回归: 终端收发 | 手动: 连接串口 -> 收发数据 | 功能正常 |
| AC-4 | 现有功能回归: 搜索栏展开/收起 | 手动: Ctrl+F / Esc | 动画正常 |
| AC-5 | 现有功能回归: OTA 传输 | 手动: XMODEM 传输 | 功能正常 |
| AC-6 | 现有功能回归: 协议解析 | 手动: 帧定义模式 -> 接收数据 | ProtocolView 和 ChartWidget 正常 |

---

## 实施优先级

| 顺序 | 步骤 | 理由 |
|------|------|------|
| 1 | TerminalWidget::selectedText() 修复 (R1) | 最小改动，独立修复，消除 P0 复制失效 |
| 2 | TerminalWidget 搜索高亮实现 (R2) | 核心功能，涉及 TerminalWidget 大量新增代码 |
| 3 | TerminalSearchBar 与 TerminalWidget 信号连接 (R2) | 依赖步骤 2 的搜索方法 |
| 4 | DTR/RTS 运行时控制 (R3) | 独立修复，涉及 SerialConfigPanel + ConnectionController |
| 5 | NewlineCombo 自动换行 (R4) | 独立功能，涉及 SendController |
| 6 | MainWindow 信号连接更新 (R2+R3) | 串联所有组件 |
| 7 | CLAUDE.md 更新 (R5) | 文档更新，最后处理 |
| 8 | 编译验证 | 确保零错误 |
| 9 | 手动功能测试 | 验证所有 P0 修复和功能增强 |

---

## 验证度量指标

### 代码度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| TerminalWidget.h 行数 | wc -l | 100 行 | < 130 行 |
| TerminalWidget.cpp 行数 | wc -l | 280 行 | < 500 行（上限） |
| ConnectionController.cpp 新增行数 | diff | 0 | < 20 行 |
| SendController.cpp 新增行数 | diff | 0 | < 25 行 |
| MainWindow.cpp 行数变化 | diff | 580 行 | 净增 < 20 行 |

### 架构度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| TerminalWidget 新增的 UI 组件指针数 | 代码检查 | 0（搜索逻辑在内部处理，不新增 QWidget） |
| ConnectionController 新增的依赖 | 代码检查 | 0（仅使用 qobject_cast，不引入新头文件） |
| SendController NewlineCombo 影响范围 | 代码检查 | 仅影响 onSendData() 方法，不影响 sendAndRecord() 签名 |
| 分层依赖方向 | 代码检查 | 无反向依赖（表现层 -> 业务层 -> 数据层） |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|---------|
| 终端复制功能 | 手动测试: 选中 100+ 行后 Ctrl+C | 复制内容与显示一致 |
| 搜索性能: 1000 行数据 | 手动测试: 搜索常见关键词 | < 100ms 响应，无明显卡顿 |
| 搜索性能: 正则复杂模式 | 手动测试: `\d+\.\d+` 搜索 | < 200ms 响应 |
| DTR/RTS 实时切换 | 示波器/逻辑分析仪验证 | 信号电平即时变化 |
| 自动换行符正确性 | 串口助手回显验证 | 追加字符与选择一致 |
| 编译零错误零警告 | cmake --build | 0 error, 0 warning |
| EmbedDebug.bat 启动 | 双击 bat 文件 | 正常启动 |
