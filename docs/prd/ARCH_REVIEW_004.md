# 架构审查报告 ARCH_REVIEW_004

> 审查人: 系统架构师 (system-architect)
> 审查日期: 2026-05-31
> 审查范围: 第16-17次commit后的架构状态
> 上次审查: ARCH_REVIEW_003 (第10次commit, commit #10)

---

## 一、分层架构合规性审查

### 1.1 分层定义回顾

```
表现层 (Presentation)  →  业务层 (Business)  →  数据层 (Data)  →  基础设施层 (Infra)
```

### 1.2 模块分层归属确认

| 文件 | 声明归属 | 实际归属 | 合规 |
|------|---------|---------|------|
| `core/MainWindow.cpp` | 表现层 | 表现层 | OK |
| `ota/OtaWidget.cpp` | 表现层 | 表现层 | OK |
| `ota/OtaManager.h` | 业务层 | 业务层 | OK |
| `protocol/FrameParser.h` | 数据层 | 数据层 | OK |
| `terminal/TerminalModel.h` | 数据层 | 数据层 | OK |
| `connection/SerialConnection.h` | 基础设施层 | 基础设施层 | OK |
| `connection/IConnection.h` | 基础设施层 | 基础设施层 | OK |
| `ota/protocols/BaseTransfer.h` | 业务层 | 业务层 | OK |
| `utils/RingBuffer.h` | 基础设施层 | 基础设施层 | OK |
| `utils/CRC.h` | 基础设施层 | 基础设施层 | OK |

### 1.3 分层违规列表

#### 违规 V-001: MainWindow.cpp 中 appendReceived/appendSent 的重复逻辑 [P1]

**位置**: `src/core/MainWindow.cpp` 第385-391行 (定时发送器回调) 与 第636-645行 (快捷指令回调) 与 第600-634行 (手动发送)

**问题描述**: 表现层 (MainWindow) 同时执行了三个本应由业务层编排的操作: `m_currentConn->write(data)` + `m_terminalModel->appendSent(data)` + `m_dataLogger->logData(...)` + `updateStatusBar()`。这三段代码在 `onSendData()`、`onQuickCommand()`、定时发送器回调中完全重复，说明"发送数据并记录"是一个跨层业务流程，不应由表现层直接编排。

**违规类型**: 表现层承担了业务逻辑编排职责，违反"表现层只做UI展示，不含业务逻辑"的原则。

**现状评估**: 功能正确，但不利于扩展。当新增连接类型或需要增加发送前校验时，需要在三处同步修改。

**建议**: 提取 `SendDataUseCase` 或在 `ConnectionManager` 上增加 `sendAndLog(data)` 方法，将 write + appendSent + logData + updateStatusBar 统一封装。MainWindow 只需调用一个方法。

---

#### 违规 V-002: MainWindow.cpp 中 onConnectionStateChanged 包含面板可见性控制逻辑 [P2]

**位置**: `src/core/MainWindow.cpp` 第767-801行

**问题描述**: `onConnectionStateChanged()` 在 Connected 状态时直接操作 `m_serialConfig->setVisible(false)` 和 `m_terminal->setVisible(true)` 等面板可见性。这是面板切换逻辑与连接状态管理的耦合，且与导航树点击的面板切换逻辑(第419-481行)存在重复。

**违规类型**: 表现层内部职责不清晰，连接状态管理与面板路由逻辑混合。

**建议**: 将面板路由逻辑抽取为独立的 `PanelRouter` 类或在 MainWindow 中封装统一的 `switchPanel(PanelId)` 方法，由连接状态回调和导航树点击共同调用。

---

#### 违规 V-003: MainWindow.h 中 m_frameParser 放在 "UI组件-协议解析" 分组下 [P2]

**位置**: `src/core/MainWindow.h` 第163行

**问题描述**: `FrameParser` 是数据层组件(协议帧解析器)，不属于UI组件。在 MainWindow.h 的注释分组中被归类为 "UI组件-协议解析"，与 `ProtocolView`(表现层) 混在一起。MainWindow 表现层直接持有数据层的 FrameParser，虽然依赖方向正确(表现层可依赖数据层)，但 MainWindow 同时承担了 FrameParser 的配置分发工作(第411-416行)。

**违规类型**: 表现层直接管理数据层组件的生命周期和配置。

**建议**: FrameParser 的创建、配置分发应由业务层(如 ProtocolEngine 或 ConnectionManager)负责，MainWindow 只接收解析结果信号。

---

#### 违规 V-004: OtaWidget::setConnection 绕过业务层直接设置连接 [P1]

**位置**: `src/ota/OtaWidget.cpp` 第26-29行

**问题描述**: `OtaWidget::setConnection(IConnection* conn)` 在表现层直接调用 `m_manager->setConnection(conn)`。这个方法被 `MainWindow::onConnectSerial()` (第588行) 调用。虽然调用链经过了 OtaManager(业务层)，但 setConnection 的触发权和调用时机完全由表现层控制。

**违规类型**: 连接生命周期管理分散在表现层中，业务层被动接受。

**现状评估**: 当前规模可接受。当支持多连接Tab切换时，需要由 ConnectionManager 统一管理所有业务模块的连接绑定。

---

### 1.4 依赖方向合规性总结

| 依赖路径 | 方向 | 合规 |
|---------|------|------|
| MainWindow -> OtaManager | 表现 -> 业务 | OK |
| MainWindow -> TerminalModel | 表现 -> 数据 | OK |
| MainWindow -> FrameParser | 表现 -> 数据 | OK |
| MainWindow -> IConnection | 表现 -> 基础设施 | OK (间接使用) |
| OtaWidget -> OtaManager | 表现 -> 业务 | OK |
| OtaManager -> BaseTransfer | 业务 -> 业务 | OK |
| OtaManager -> IConnection | 业务 -> 基础设施 | OK |
| FrameParser -> CRC | 数据 -> 基础设施 | OK |
| TerminalModel -> Constants | 数据 -> 基础设施 | OK |
| SerialConnection -> IConnection | 基础设施 -> 基础设施 | OK |

**结论**: 无反向依赖。所有模块遵守单向依赖规则: 表现层 -> 业务层 -> 数据层 -> 基础设施层。违规项均为表现层内部职责划分问题，不影响分层依赖方向。

---

## 二、性能瓶颈分析

### 2.1 TerminalModel::lines() 全量拷贝 [P0]

**位置**: `src/terminal/TerminalModel.cpp` 第45-49行

```cpp
QVector<TerminalLine> TerminalModel::lines() const
{
    QMutexLocker locker(&m_mutex);
    return m_lines;  // 全量拷贝
}
```

**问题分析**:
- `m_lines` 默认最大50000行，每行 `TerminalLine` 包含 `QByteArray` + `QDateTime` + 枚举，单行约40-80字节
- 50000行时，`lines()` 返回值拷贝约 2-4 MB 数据
- 该方法在 `MainWindow::onExportData()` (第684行) 中被调用，导出时触发一次全量拷贝
- `QVector<TerminalLine>` 的隐式共享(Copy-on-Write)在 `const` 返回时会退化: 因为方法内加了 `QMutexLocker`，mutex 锁的作用域与返回值拷贝在同一语句中，`m_lines` 的引用计数在锁释放前已完成，Qt的COW优化可部分生效，但仍有额外开销

**影响范围**: 数据导出时短暂卡顿。在高频数据流(115200+ baud)下，如果 TerminalWidget 的渲染也调用 `lines()`，会导致UI线程阻塞。

**优化建议**:

方案A: 使用 `lines(int start, int count)` 分段接口 (已有实现，第51-58行)
```cpp
// 导出时分批读取，避免全量拷贝
QVector<TerminalLine> batch;
int offset = 0;
const int batchSize = 1000;
while ((batch = m_terminalModel->lines(offset, batchSize)).size() > 0) {
    exporter.writeBatch(batch);
    offset += batch.size();
}
```

方案B: 提供 `const QVector<TerminalLine>& linesRef() const` 只读引用接口(需保证调用线程安全)

**优先级**: P0 -- 数据导出是常用功能，50000行时用户可感知卡顿。

---

### 2.2 TerminalModel::removeFirst() 的 O(n) 性能问题 [P0]

**位置**: `src/terminal/TerminalModel.cpp` 第20-23行, 第37-39行

```cpp
while (m_lines.size() > m_maxLines) {
    m_lines.removeFirst();  // O(n) - 每次删除首元素需要移动所有后续元素
}
```

**问题分析**:
- `QVector::removeFirst()` 的时间复杂度为 O(n)，因为它需要将 index 1 到 size-1 的所有元素向前移动一位
- 当 `m_lines` 达到 50000 行上限时，每次 `appendReceived()` 或 `appendSent()` 都可能触发一次 `removeFirst()`
- 在高频串口数据流中(例如每秒100+帧的传感器数据)，`appendReceived()` 被高频调用
- 每次调用 `removeFirst()` 需要移动 ~50000 个 `TerminalLine` 对象，每个对象包含 QByteArray (间接指针)和 QDateTime，实际是 ~50000 次内存移动
- 综合: 在50000行满时，每秒100次 append = 100 * 50000 = 5,000,000 次元素移动/秒
- 注意: `while` 循环理论上只在 size > maxLines 时执行，但由于每次只 append 一行，循环体通常只执行一次。问题在于那一次 `removeFirst()` 本身就是 O(n)

**量化影响**:
- QVector 的 `removeFirst()` 底层是 `memmove`，50000 个 TerminalLine 约 4MB，单次移动约 0.1-0.5ms
- 100Hz 数据流下，每秒额外消耗 10-50ms 在内存移动上
- 在 1MHz+ 高波特率场景下影响更显著

**优化方案**:

#### 方案A: 使用已有的 RingBuffer<TerminalLine> 替换 QVector (推荐) [P0]

项目中已有 `src/utils/RingBuffer.h`，完全可用于此场景:

```cpp
// TerminalModel.h 改造
private:
    RingBuffer<TerminalLine> m_lines{50000};  // 替换 QVector<TerminalLine>
```

优势:
- `push()` 操作 O(1)，无需 `removeFirst()`
- 自动覆盖最旧行，无需 while 循环
- 已有线程安全的 RingBuffer 实现，可直接使用
- `lines()` 和 `lines(start, count)` 需要适配 RingBuffer 的环形索引访问

劣势:
- RingBuffer 当前没有 `toArray()` 或 `mid(start, count)` 批量读取接口，需要扩展
- `lineCount()` 可直接用 `m_lines.count()`

#### 方案B: 环形索引 + QVector (最小改动)

不替换数据结构，而是用 head/tail 索引模拟环形:

```cpp
// TerminalModel.h
int m_head = 0;   // 环形首元素索引
int m_count = 0;  // 当前有效行数

// appendReceived 中
int slot = (m_head + m_count) % m_maxLines;
if (m_count < m_maxLines) {
    m_buffer[slot] = line;
    m_count++;
} else {
    m_buffer[m_head] = line;  // 覆盖最旧
    m_head = (m_head + 1) % m_maxLines;
}
```

**推荐**: 方案A，复用已有的 RingBuffer，保持项目组件一致性。需要为 RingBuffer 添加 `toList()` 和 `mid(start, count)` 辅助方法。

---

### 2.3 appendReceived/appendSent 代码重复 [P2]

**位置**: `src/terminal/TerminalModel.cpp` 第8-25行 与 第27-43行

两段代码除 `DataDirection::Rx`/`Tx` 和 `m_rxBytes`/`m_txBytes` 外完全相同。可提取为 `appendLine(data, direction)` 私有方法。

---

### 2.4 setMaxLines() 无锁保护 [P2]

**位置**: `src/terminal/TerminalModel.cpp` 第87-90行

```cpp
void TerminalModel::setMaxLines(int max)
{
    m_maxLines = max;  // 无 QMutexLocker，可能在 appendReceived 的 mutex 持有期间并发读写
}
```

`m_maxLines` 在 `appendReceived()` 和 `appendSent()` 中被读取(持有锁)，但在 `setMaxLines()` 中被写入(未持有锁)。存在数据竞争风险。

---

### 2.5 性能瓶颈汇总

| 编号 | 问题 | 影响 | 优先级 | 位置 |
|------|------|------|--------|------|
| P-001 | `lines()` 全量拷贝 | 50000行导出卡顿 | P0 | TerminalModel.cpp:45 |
| P-002 | `removeFirst()` O(n) | 高频数据流下内存移动开销 | P0 | TerminalModel.cpp:20,37 |
| P-003 | appendReceived/appendSent 重复 | 维护成本 | P2 | TerminalModel.cpp:8-43 |
| P-004 | `setMaxLines()` 无锁保护 | 数据竞争 | P2 | TerminalModel.cpp:87-90 |

---

## 三、重复代码检测

### 3.1 MainWindow 导航树面板切换 if-else 链 [P1]

**位置**: `src/core/MainWindow.cpp` 第419-481行

```cpp
// 功能性节点
if (text == tr("数据导出")) { onExportData(); return; }
if (text == tr("TCP客户端")) { onConnectNetwork(ConnectionType::TcpClient); return; }
if (text == tr("TCP服务端")) { onConnectNetwork(ConnectionType::TcpServer); return; }
if (text == tr("UDP")) { onConnectNetwork(ConnectionType::Udp); return; }

// 面板选择
QWidget* target = nullptr;
if (text == tr("配置"))        target = m_serialConfig;
else if (text == tr("终端"))   target = m_terminal;
else if (text == tr("统计"))   target = m_dataStats;
else if (text == tr("协议"))   target = m_protocolView;
else if (text == tr("帧编辑器")) target = m_frameEditor;
else if (text == tr("波形图"))  target = m_chartWidget;
else if (text == tr("OTA升级")) target = m_otaWidget;

// 隐藏所有，只显示目标
for (auto* w : allPanels) {
    if (w) w->setVisible(w == target);
}
```

**问题分析**:
1. 7个面板的 if-else 链每次新增面板都要修改此代码块
2. 功能性节点(数据导出、TCP、UDP)和面板切换节点混在同一个lambda中
3. `static const QVector<QPair<QString, QWidget*>> panels` (第423-431行) 已声明但未使用(所有值均为 nullptr)，是无效代码
4. 动画创建代码(第462-480行)每次面板切换都会 `new QGraphicsOpacityEffect`，存在内存管理隐患

**改进方案**: 数据驱动的面板映射表

```cpp
// MainWindow.h 中定义面板ID枚举
enum class PanelId {
    Config, Terminal, Stats, Protocol,
    FrameEditor, Chart, Ota
};

// 私有成员
QMap<PanelId, QWidget*> m_panels;
QMap<QString, PanelId> m_navToPanel;
QMap<QString, std::function<void()>> m_navActions;
```

```cpp
// connectSignals() 中初始化映射
m_panels = {
    {PanelId::Config,      m_serialConfig},
    {PanelId::Terminal,    m_terminal},
    {PanelId::Stats,       m_dataStats},
    {PanelId::Protocol,    m_protocolView},
    {PanelId::FrameEditor, m_frameEditor},
    {PanelId::Chart,       m_chartWidget},
    {PanelId::Ota,         m_otaWidget},
};

m_navToPanel = {
    {tr("配置"),     PanelId::Config},
    {tr("终端"),     PanelId::Terminal},
    {tr("统计"),     PanelId::Stats},
    {tr("协议"),     PanelId::Protocol},
    {tr("帧编辑器"), PanelId::FrameEditor},
    {tr("波形图"),   PanelId::Chart},
    {tr("OTA升级"),  PanelId::Ota},
};

m_navActions = {
    {tr("数据导出"), [this]() { onExportData(); }},
    {tr("TCP客户端"), [this]() { onConnectNetwork(ConnectionType::TcpClient); }},
    {tr("TCP服务端"), [this]() { onConnectNetwork(ConnectionType::TcpServer); }},
    {tr("UDP"),       [this]() { onConnectNetwork(ConnectionType::Udp); }},
};

// 导航树点击处理
connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
    QString text = index.data().toString();

    // 优先匹配功能性动作
    if (m_navActions.contains(text)) {
        m_navActions[text]();
        return;
    }

    // 面板切换
    if (m_navToPanel.contains(text)) {
        switchPanel(m_navToPanel[text]);
    }
});
```

```cpp
// 统一的面板切换方法
void MainWindow::switchPanel(PanelId id)
{
    QWidget* target = m_panels.value(id, nullptr);
    if (!target) return;

    for (auto* w : m_panels) {
        if (w) w->setVisible(w == target);
    }

    // 统一的淡入动画
    fadePanelIn(target);
}
```

**收益**:
- 新增面板只需在映射表中增加一行，无需修改 if-else 链
- 消除无效的 `static const QVector<QPair<QString, QWidget*>> panels` 声明
- 动画逻辑统一封装，避免每次 new QGraphicsOpacityEffect 的内存碎片
- 功能性动作与面板切换逻辑分离，职责更清晰

---

### 3.2 发送数据逻辑三处重复 [P1]

**位置**:
- `MainWindow::onSendData()` 第600-634行
- `MainWindow::onQuickCommand()` 第636-645行
- 定时发送器回调 第385-391行

三处代码均执行: `write(data)` + `appendSent(data)` + `logData(data)` + `updateStatusBar()`

**改进方案**: 提取私有方法

```cpp
// MainWindow.h
void sendAndRecord(const QByteArray& data);

// MainWindow.cpp
void MainWindow::sendAndRecord(const QByteArray& data)
{
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) return;
    qint64 written = m_currentConn->write(data);
    if (written > 0) {
        m_terminalModel->appendSent(data);
        m_dataLogger->logData(data, DataLogger::Direction::Sent);
        updateStatusBar();
    }
}
```

---

### 3.3 连接信号绑定代码重复 [P2]

**位置**: `MainWindow::onConnectSerial()` 第569-576行 与 `MainWindow::onConnectNetwork()` 第748-755行

两处代码完全相同:
```cpp
connect(m_currentConn, &IConnection::dataReceived, this, &MainWindow::onDataReceived);
connect(m_currentConn, &IConnection::stateChanged, this, &MainWindow::onConnectionStateChanged);
connect(m_currentConn, &IConnection::errorOccurred, this, [](const QString& msg) { ... });
```

**改进方案**: 提取 `bindConnectionSignals(IConnection* conn)` 私有方法。

---

### 3.4 面板切换动画每次 new QGraphicsOpacityEffect [P2]

**位置**: `src/core/MainWindow.cpp` 第462-479行

每次面板切换都 `new QGraphicsOpacityEffect(target)` 和 `new QPropertyAnimation`，虽然设置了 `DeleteWhenStopped`，但频繁的堆分配会影响性能。

**改进方案**: 使用一个缓存的动画对象，或在 `fadePanelIn()` 方法中复用 QPropertyAnimation 实例。

---

### 3.5 重复代码汇总

| 编号 | 重复位置 | 类型 | 行数 | 优先级 |
|------|---------|------|------|--------|
| D-001 | 导航树面板切换 if-else 链 | 结构重复 | ~60行 | P1 |
| D-002 | 发送数据逻辑 (3处) | 逻辑重复 | ~30行 x3 | P1 |
| D-003 | 连接信号绑定 (2处) | 代码重复 | ~8行 x2 | P2 |
| D-004 | 面板切换动画堆分配 | 模式重复 | ~18行 | P2 |
| D-005 | TerminalModel append 方法 (2处) | 结构重复 | ~18行 x2 | P2 |

---

## 四、改进建议总表

### 4.1 P0 -- 必须修复 (影响运行时性能)

| 编号 | 建议 | 涉及文件 | 预计工作量 |
|------|------|---------|-----------|
| P0-01 | TerminalModel 底层数据结构从 QVector 改为 RingBuffer，消除 removeFirst() 的 O(n) 开销 | `TerminalModel.h/cpp` + `RingBuffer.h`(扩展) | 约150行 |
| P0-02 | DataExporter::exportToFile 改为使用 `lines(start, count)` 分段读取接口，避免全量拷贝 | `MainWindow.cpp` onExportData() + `DataExporter.h/cpp` | 约50行 |
| P0-03 | setMaxLines() 添加 QMutexLocker 保护 | `TerminalModel.cpp` | 3行 |

### 4.2 P1 -- 应该修复 (影响架构质量和可维护性)

| 编号 | 建议 | 涉及文件 | 预计工作量 |
|------|------|---------|-----------|
| P1-01 | 导航树面板切换重构为数据驱动映射表，消除 if-else 链 | `MainWindow.h/cpp` | 约100行 |
| P1-02 | 提取 sendAndRecord() 统一发送数据逻辑 | `MainWindow.h/cpp` | 约40行 |
| P1-03 | FrameParser 的创建和配置分发从 MainWindow 下沉到业务层 | `MainWindow.cpp` + 新增 `ProtocolEngine` 或移入 `ConnectionManager` | 约120行 |
| P1-04 | OtaWidget 中的连接绑定改为由 ConnectionManager 统一管理 | `MainWindow.cpp` + `OtaWidget.cpp` | 约30行 |

### 4.3 P2 -- 可以延后 (锦上添花)

| 编号 | 建议 | 涉及文件 | 预计工作量 |
|------|------|---------|-----------|
| P2-01 | 提取 TerminalModel::appendLine() 消除 appendReceived/appendSent 重复 | `TerminalModel.cpp` | 约20行 |
| P2-02 | 提取 MainWindow::bindConnectionSignals() 消除连接信号绑定重复 | `MainWindow.cpp` | 约15行 |
| P2-03 | 提取 MainWindow::switchPanel() + fadePanelIn() 统一面板切换和动画 | `MainWindow.h/cpp` | 约60行 |
| P2-04 | onConnectionStateChanged 中的面板可见性控制改为调用统一的 switchPanel() | `MainWindow.cpp` | 约20行 |
| P2-05 | 删除无效的 `static const QVector<QPair<QString, QWidget*>> panels` 声明 | `MainWindow.cpp` 第423-431行 | 删除8行 |

---

## 五、架构健康度评分

| 维度 | 评分 (1-10) | 说明 |
|------|------------|------|
| 分层合规 | 8/10 | 依赖方向正确，无反向依赖。表现层承担了部分业务编排职责 |
| 性能 | 6/10 | TerminalModel 在大数据量下存在两个 O(n) 瓶颈 |
| 代码重复 | 6/10 | 5处重复模式，导航树切换和发送逻辑最为显著 |
| 组件复用 | 9/10 | RingBuffer/CRC/HexConverter 等公共组件复用良好 |
| 可扩展性 | 7/10 | if-else 面板切换和硬编码的发送逻辑影响新功能添加 |
| **综合** | **7.2/10** | 架构基础扎实，性能优化和去重是下一阶段重点 |

---

## 六、下次审查关注点

1. RingBuffer 替换 TerminalModel 底层数据结构后的性能回归测试
2. 面板路由映射表重构后的可维护性验证
3. 多连接 Tab 切换功能的架构准备度(当前 MainWindow 只支持单个连接)
4. FrameParser 下沉到业务层后的依赖关系变化
