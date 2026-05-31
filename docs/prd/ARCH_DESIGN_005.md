# 架构设计文档 ARCH_DESIGN_005

> 设计人: 系统架构师 (system-architect)
> 设计日期: 2026-05-31
> 关联审查: ARCH_REVIEW_004 (V-001, P-001, D-002, P0-02)
> 涉及模块: core/MainWindow, terminal/TerminalModel, utils/DataExporter
> 预计变更: ~200行新增/修改, ~60行删除

---

## 一、设计背景

ARCH_REVIEW_004 中标记了以下待改进项:

| 审查编号 | 问题描述 | 优先级 |
|---------|---------|--------|
| V-001 | MainWindow 中发送数据的三处重复代码 (表现层承担业务编排) | P1 |
| D-002 | onSendData / onQuickCommand / TimedSender回调 逻辑重复 (约90行) | P1 |
| P-001 | DataExporter::exportToFile 使用 lines() 全量拷贝, 50000行时卡顿 | P0 |
| P0-02 | 导出时应改为 lines(start, count) 分段接口 | P0 |

本次设计覆盖四个重构点:

1. 提取 `sendAndRecord()` 统一发送数据链路
2. 评估 `m_rightPanel` (QStackedWidget) 的合理性并建议降级为 QWidget
3. DataExporter 改为分段导出接口
4. 补充 DataExporter 支持流式写入的接口设计

---

## 二、重构点 1: sendAndRecord() 发送数据统一方法

### 2.1 现状分析: 三处重复代码

**位置 A -- onSendData() (MainWindow.cpp 第686-719行)**

```
完整链路: 连接检查 -> 文本解析 -> HEX转换 -> write -> appendSent -> logData -> addHistory -> clearInput -> updateStatusBar
```

这是最完整的一条链路, 包含输入解析和UI状态清理, 仅在手动发送按钮触发。

**位置 B -- onQuickCommand() (MainWindow.cpp 第722-731行)**

```
精简链路: 连接检查 -> write -> appendSent -> logData -> updateStatusBar
```

快捷指令回调, 数据已经预处理为 QByteArray, 无需解析和UI清理。

**位置 C -- TimedSender回调 (MainWindow.cpp 第510-516行)**

```
精简链路: 连接检查 -> write -> appendSent -> updateStatusBar
```

定时发送回调, 最简版本, 注意此处缺少 `logData` 调用 -- 这可能是一个遗漏的bug。

### 2.2 差异矩阵

| 步骤 | onSendData | onQuickCommand | TimedSender回调 |
|------|-----------|---------------|----------------|
| 连接状态检查 | 有 | 有 | 有 |
| IConnection::write(data) | 有 (检查written) | 有 (不检查返回值) | 有 (不检查返回值) |
| TerminalModel::appendSent | 有 | 有 | 有 |
| DataLogger::logData(Sent) | 有 | 有 | **缺失** |
| SendHistory::addEntry | 有 | 无 | 无 |
| m_sendInput->clear() | 有 | 无 | 无 |
| updateStatusBar() | 有 | 有 | 有 |

关键发现:

- **TimedSender回调缺少 logData**: 定时发送的数据不会被记录到 .edl 日志文件。这违反了用户对"所有收发数据都被录制"的预期。当录制开关打开时, 定时发送的数据在回放中会丢失。
- **onQuickCommand 和 TimedSender 不检查 write 返回值**: 如果 write 返回 0 或 -1, 仍然会执行 appendSent 和 logData, 造成终端显示和日志中出现未实际发送的数据。
- **三处 write 调用的错误处理策略不一致**: onSendData 检查 written > 0, 其余两处不检查。

### 2.3 设计方案: sendAndRecord()

```cpp
// MainWindow.h -- private 区域新增
private:
    // 统一发送数据方法: write -> appendSent -> logData -> updateStatusBar
    // 返回实际写入的字节数, 0 表示未发送(连接断开或写入失败)
    qint64 sendAndRecord(const QByteArray& data);
```

```cpp
// MainWindow.cpp -- 新增方法实现
qint64 MainWindow::sendAndRecord(const QByteArray& data)
{
    // 前置条件: 连接有效且处于已连接状态
    if (!m_currentConn || m_currentConn->state() != ConnectionState::Connected) {
        return 0;
    }

    if (data.isEmpty()) {
        return 0;
    }

    qint64 written = m_currentConn->write(data);
    if (written <= 0) {
        return 0;
    }

    // 统一后处理: 写入成功后执行所有记录操作
    m_terminalModel->appendSent(data);
    m_dataLogger->logData(data, DataLogger::Direction::Sent);
    updateStatusBar();

    return written;
}
```

### 2.4 调用点重构

**onSendData() 重构后:**

```cpp
void MainWindow::onSendData()
{
    // UI层职责: 解析输入、转换编码
    QString text = m_sendInput->text();
    if (text.isEmpty()) return;

    bool isHex = (m_sendModeCombo->currentIndex() == 1);
    QByteArray data;
    if (isHex) {
        data = HexConverter::fromHexString(text);
        if (data.isEmpty()) {
            m_sendInput->setProperty("hasError", true);
            m_sendInput->style()->unpolish(m_sendInput);
            m_sendInput->style()->polish(m_sendInput);
            return;
        }
    } else {
        data = text.toUtf8();
    }

    // 统一发送
    qint64 written = sendAndRecord(data);

    // UI层职责: 更新发送状态
    if (written > 0) {
        m_sendHistory->addEntry(text, isHex);
        m_sendInput->clear();
    }
    // 无论成功失败, 清除错误状态
    m_sendInput->setProperty("hasError", written <= 0);
    m_sendInput->style()->unpolish(m_sendInput);
    m_sendInput->style()->polish(m_sendInput);
}
```

**onQuickCommand() 重构后:**

```cpp
void MainWindow::onQuickCommand(const QByteArray& data)
{
    sendAndRecord(data);
}
```

**TimedSender回调重构后:**

```cpp
connect(m_timedSender, &TimedSender::sendData, this, [this](const QByteArray& data) {
    sendAndRecord(data);
});
```

### 2.5 重构收益

| 维度 | 改善 |
|------|------|
| Bug修复 | TimedSender回调现在会正确记录 logData |
| 一致性 | 三处发送均检查 write 返回值, 错误处理策略统一 |
| 代码量 | 删除约30行重复代码, 新增15行统一方法 |
| 可维护性 | 新增发送场景(如脚本引擎、MQTT网关)只需调用 sendAndRecord() |
| 分层合规 | sendAndRecord() 封装了跨 write/appendSent/logData/statusBar 的业务编排 |

### 2.6 职责边界说明

sendAndRecord() 的职责边界:

- **属于它**: write + appendSent + logData + updateStatusBar (业务编排)
- **不属于它**: 输入解析 (HexConverter)、历史记录管理 (SendHistory)、UI状态清理 (clearInput)
- **原因**: onSendData 需要额外的输入解析和UI操作, 这些是表现层独有的职责, 不应下沉到发送方法中

---

## 三、重构点 2: m_rightPanel 从 QStackedWidget 改为 QWidget

### 3.1 现状分析

MainWindow.cpp 第137行:

```cpp
m_rightPanel = new QStackedWidget;
```

m_rightPanel 被声明为 QStackedWidget, 但在整个代码库中的实际使用方式是:

| 调用点 | 代码 | 作用 |
|-------|------|------|
| MainWindow.cpp:138 | `rightLayout->addWidget(m_rightPanel, 1)` | 作为布局子组件 |
| MainWindow.cpp:232 | `m_rightPanel->addWidget(serialPanel)` | 仅添加了一个子页面 |
| MainWindow.cpp:233 | `m_rightPanel->setCurrentIndex(0)` | 设置当前页为索引0 |

关键发现:

- **m_rightPanel 只包含一个页面 (serialPanel)**。QStackedWidget 的核心能力是管理多个互斥页面并通过索引切换, 但这里只添加了一个页面。
- **面板切换不通过 m_rightPanel 的 setCurrentIndex()**。实际的切换逻辑在 switchToPanel() 方法中, 通过直接控制各个 QWidget 的 setVisible(true/false) 实现。面板是 serialPanel 的子组件, 不是 m_rightPanel 的直接子页面。
- **QStackedWidget 的边框/边距成为多余开销**。QStackedWidget 继承自 QFrame, 默认有 NoFrame, 但其内部布局管理增加了不必要的复杂度。

面板切换的实际架构:

```
m_rightPanel (QStackedWidget)        <-- 仅1个子页面, 切换能力未使用
  +-- serialPanel (QWidget)
        +-- m_serialConfig           <-- setVisible控制
        +-- m_dataStats              <-- setVisible控制
        +-- m_protocolView           <-- setVisible控制
        +-- m_frameEditor            <-- setVisible控制
        +-- m_chartWidget            <-- setVisible控制
        +-- m_otaWidget              <-- setVisible控制
        +-- terminalContainer        <-- 默认可见
        +-- m_quickCmdBar            <-- 始终可见
        +-- sendFrame                <-- 始终可见
```

### 3.2 设计方案

**方案: 将 QStackedWidget 替换为 QWidget**

MainWindow.h 第7行删除 QStackedWidget 头文件引用, 第146行修改类型:

```cpp
// 修改前
#include <QStackedWidget>
// ...
QStackedWidget* m_rightPanel;

// 修改后
// 无需额外 include, QWidget 已通过其他头文件间接引入
// ...
QWidget* m_rightPanel;
```

MainWindow.cpp 第137-138行修改:

```cpp
// 修改前
m_rightPanel = new QStackedWidget;
rightLayout->addWidget(m_rightPanel, 1);

// 修改后
m_rightPanel = new QWidget;
m_rightPanel->setLayout(new QVBoxLayout(m_rightPanel));
m_rightPanel->layout()->setContentsMargins(0, 0, 0, 0);
m_rightPanel->layout()->setSpacing(0);
rightLayout->addWidget(m_rightPanel, 1);
```

MainWindow.cpp 第232-233行修改:

```cpp
// 修改前
m_rightPanel->addWidget(serialPanel);
m_rightPanel->setCurrentIndex(0);

// 修改后
m_rightPanel->layout()->addWidget(serialPanel);
```

### 3.3 影响范围

| 文件 | 修改内容 |
|------|---------|
| MainWindow.h 第7行 | 删除 `#include <QStackedWidget>` |
| MainWindow.h 第146行 | `QStackedWidget*` 改为 `QWidget*` |
| MainWindow.cpp 第137行 | `new QStackedWidget` 改为 `new QWidget` |
| MainWindow.cpp 第138行 | 为 QWidget 设置 QVBoxLayout |
| MainWindow.cpp 第232行 | `addWidget` 改为 `layout()->addWidget` |
| MainWindow.cpp 第233行 | 删除 `setCurrentIndex(0)` 调用 |

### 3.4 决策理由

| 因素 | 分析 |
|------|------|
| QStackedWidget 的价值 | 管理多个互斥页面并按索引切换。当前只用1个页面, 无切换 |
| 实际切换机制 | switchToPanel() 通过 setVisible() 控制 serialPanel 的子组件 |
| 额外开销 | QStackedWidget 继承链: QWidget -> QFrame -> QStackedWidget, 多一层不必要的 QFrame |
| 代码可读性 | 使用 QWidget 更准确地表达"这是一个容器"的语义, 不会误导开发者以为有多页面切换 |
| 未来扩展 | 如果将来需要真正的多页面 Tab 切换, 应使用 QTabWidget 或自定义 TabBar, 而非 QStackedWidget 嵌套在 setVisible 体系中 |

---

## 四、重构点 3: DataExporter 分段导出

### 4.1 现状分析

MainWindow::onExportData() 第770行:

```cpp
m_dataExporter->exportToFile(filePath, format, m_terminalModel->lines());
```

`m_terminalModel->lines()` 触发一次全量深拷贝。当前 TerminalModel 已使用环形缓冲区, `lines()` 实现如下:

```cpp
QVector<TerminalLine> TerminalModel::lines() const
{
    QMutexLocker locker(&m_mutex);
    QVector<TerminalLine> result;
    result.reserve(m_count);
    for (int i = 0; i < m_count; ++i) {
        result.append(m_buffer[physicalIndex(i)]);
    }
    return result;
}
```

50000行时, 需要拷贝 50000 个 TerminalLine 对象, 每个包含 QByteArray + QDateTime + 枚举, 约 2-4 MB。拷贝期间持有互斥锁, 阻塞 appendReceived/appendSent 的写入。

同时, 已有的分段接口 `lines(int start, int count)` 同样存在:

```cpp
QVector<TerminalLine> TerminalModel::lines(int start, int count) const
{
    QMutexLocker locker(&m_mutex);
    // ... 分段拷贝, 每次最多 count 行
}
```

### 4.2 设计方案: 流式分段导出

在 DataExporter 中新增一个接受回调的分段导出接口:

```cpp
// DataExporter.h -- 新增公共方法
public:
    // 流式导出: 通过分段读取避免全量内存拷贝
    // lineProvider: 由调用方提供的行读取回调, 返回指定范围的行
    //   参数: start=起始逻辑索引, count=请求行数
    //   返回: 实际读取到的行列表 (可能少于count, 表示已到末尾)
    using LineProvider = std::function<QVector<TerminalLine>(int start, int count)>;

    bool exportStreamed(const QString& filePath, Format format,
                        const LineProvider& provider, int totalLines,
                        const QDateTime& from = QDateTime(),
                        const QDateTime& to = QDateTime());
```

```cpp
// DataExporter.cpp -- exportStreamed 实现
bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   const LineProvider& provider,
                                   int totalLines,
                                   const QDateTime& from,
                                   const QDateTime& to)
{
    if (filePath.isEmpty() || totalLines <= 0 || !provider) {
        return false;
    }

    // BIN 格式需要按时间过滤, 但分段读取时无法在写入端过滤
    // 所以 BIN 格式回退到全量读取 (时间过滤场景下)
    bool needsTimeFilter = from.isValid() || to.isValid();

    if (format == Bin && !needsTimeFilter) {
        // BIN 无时间过滤: 可以纯流式写入
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) return false;

        int offset = 0;
        const int batchSize = 1000;
        while (offset < totalLines) {
            QVector<TerminalLine> batch = provider(offset, batchSize);
            if (batch.isEmpty()) break;
            for (const auto& line : batch) {
                file.write(line.data);
            }
            offset += batch.size();
        }
        file.close();
        return true;
    }

    if (format == Bin && needsTimeFilter) {
        // BIN + 时间过滤: 必须先收集所有数据再过滤
        // 回退到 lines() 全量接口
        QVector<TerminalLine> allLines;
        allLines.reserve(totalLines);
        int offset = 0;
        const int batchSize = 1000;
        while (offset < totalLines) {
            QVector<TerminalLine> batch = provider(offset, batchSize);
            if (batch.isEmpty()) break;
            allLines.append(batch);
            offset += batch.size();
        }
        return exportBin(filePath, filterByTime(allLines, from, to));
    }

    // TXT / CSV 格式: 逐批写入文件
    // 如果需要时间过滤, 先收集全部数据再过滤
    if (needsTimeFilter) {
        QVector<TerminalLine> allLines;
        allLines.reserve(totalLines);
        int offset = 0;
        const int batchSize = 1000;
        while (offset < totalLines) {
            QVector<TerminalLine> batch = provider(offset, batchSize);
            if (batch.isEmpty()) break;
            allLines.append(batch);
            offset += batch.size();
        }
        // 复用现有的 exportTxt/exportCsv 方法
        QVector<TerminalLine> filtered = filterByTime(allLines, from, to);
        if (filtered.isEmpty()) return false;
        switch (format) {
        case Txt: return exportTxt(filePath, filtered);
        case Csv: return exportCsv(filePath, filtered);
        default:  return false;
        }
    }

    // TXT / CSV 无时间过滤: 纯流式逐批写入
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    if (format == Csv) {
        out << "timestamp,direction,data_hex,data_ascii\n";
    }

    int offset = 0;
    const int batchSize = 1000;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, batchSize);
        if (batch.isEmpty()) break;

        for (const TerminalLine& line : batch) {
            QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
            QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
            QString hex = HexConverter::toHexString(line.data);
            QString ascii = toAsciiString(line.data);

            if (format == Txt) {
                out << QString("[%1] [%2] %3 | %4\n").arg(timeStr, dirStr, hex, ascii);
            } else {
                out << timeStr << ',' << dirStr << ',' << hex << ','
                    << '"' << ascii << '"' << '\n';
            }
        }

        offset += batch.size();
    }

    file.close();
    return true;
}
```

### 4.3 调用点重构

MainWindow::onExportData() 重构后:

```cpp
void MainWindow::onExportData()
{
    int totalLines = m_terminalModel->lineCount();
    if (totalLines == 0) {
        QMessageBox::information(this, tr("Export"), tr("No data to export"));
        return;
    }

    QString filter = tr("Text files (*.txt);;CSV files (*.csv);;Binary files (*.bin)");
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Data"),
                                                     QString(), filter);
    if (filePath.isEmpty()) return;

    DataExporter::Format format = DataExporter::Txt;
    if (filePath.endsWith(".csv", Qt::CaseInsensitive))
        format = DataExporter::Csv;
    else if (filePath.endsWith(".bin", Qt::CaseInsensitive))
        format = DataExporter::Bin;

    // 使用 Lambda 提供分段读取回调
    auto provider = [this](int start, int count) -> QVector<TerminalLine> {
        return m_terminalModel->lines(start, count);
    };

    if (m_dataExporter->exportStreamed(filePath, format, provider, totalLines)) {
        statusBar()->showMessage(tr("Exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file"));
    }
}
```

### 4.4 性能对比

| 场景 | 修改前 | 修改后 |
|------|-------|-------|
| 50000行导出TXT | 一次拷贝 2-4 MB, 锁持有时间长 | 分50批, 每批1000行约40-80KB, 锁持有时间短 |
| 导出期间新数据写入 | 被阻塞直到拷贝完成 | 每批间隔可插入, 阻塞时间降低约50倍 |
| 峰值内存占用 | 2-4 MB (全量拷贝) | 40-80 KB (单批拷贝) |
| 时间过滤场景 | 全量拷贝后过滤 | 仍需全量收集, 但分批读取减少锁竞争 |

### 4.5 分段大小选择依据

batchSize = 1000 的理由:

- 1000行 TerminalLine 约 40-80 KB, 与 L1/L2 缓存友好
- 50000行总量分50批, 每批锁持有时间约 0.01-0.05ms (环形缓冲区索引计算 + 拷贝)
- 50次文件写入, 每次写入少量数据, OS文件缓冲区可合并
- 如果 batchSize 过大(如10000), 单次锁持有时间增加, 对高频数据流不友好
- 如果 batchSize 过小(如100), 函数调用和文件 I/O 开销占比增大

### 4.6 对 DataExporter 旧接口的处理

保留 `exportToFile()` 方法不删除, 保持向后兼容:

```cpp
// 旧接口保留, 内部改为调用 exportStreamed
bool exportToFile(const QString& filePath, Format format,
                  const QVector<TerminalLine>& lines,
                  const QDateTime& from = QDateTime(),
                  const QDateTime& to = QDateTime());
```

旧接口适用于调用方已有全量数据在内存中的场景(如测试用例、回放导出), 不强制迁移到流式接口。

---

## 五、依赖关系与分层合规性检查

### 5.1 sendAndRecord() 的分层分析

```
sendAndRecord() 内部调用链:

MainWindow (表现层)
  -> IConnection::write()         (基础设施层)  合规: 表现层可依赖基础设施层
  -> TerminalModel::appendSent()  (数据层)      合规: 表现层可依赖数据层
  -> DataLogger::logData()        (基础设施层)  合规: 表现层可依赖基础设施层
  -> updateStatusBar()            (表现层内部)   合规: 表现层内部调用
```

依赖方向: 表现层 -> 数据层/基础设施层, 符合单向依赖规则。

严格来说, sendAndRecord() 是一个"发送数据并记录"的业务编排, 最理想的归属是业务层 (如 ConnectionManager 新增 sendAndRecord 方法)。但考虑到:

1. 当前 ConnectionManager 仅负责连接生命周期管理, 不涉及终端数据记录
2. appendSent 和 logData 分属不同模块, 引入业务层中间类会增加复杂度
3. 当前重构目标是消除重复代码, 不是重新划分架构层

因此, 将 sendAndRecord() 放在 MainWindow 作为私有方法是当前阶段的最优解。未来如果引入多Tab连接, 可以将其提取到 ConnectionManager 或新的 SessionController 中。

### 5.2 分段导出的分层分析

```
onExportData() 调用链:

MainWindow (表现层)
  -> TerminalModel::lineCount()    (数据层)      合规
  -> TerminalModel::lines(s, c)    (数据层)      合规
  -> DataExporter::exportStreamed() (基础设施层)  合规
```

无新增跨层依赖。

---

## 六、实现计划

### 6.1 实施顺序

| 步骤 | 内容 | 涉及文件 | 预计行数 |
|------|------|---------|---------|
| 1 | 新增 sendAndRecord() 方法 | MainWindow.h (声明), MainWindow.cpp (实现) | +15行 |
| 2 | 重构 onSendData() 调用 sendAndRecord | MainWindow.cpp | -10行, +8行 |
| 3 | 重构 onQuickCommand() 调用 sendAndRecord | MainWindow.cpp | -5行, +1行 |
| 4 | 重构 TimedSender回调调用 sendAndRecord | MainWindow.cpp | -4行, +1行 |
| 5 | m_rightPanel 类型从 QStackedWidget 改为 QWidget | MainWindow.h, MainWindow.cpp | ~6行修改 |
| 6 | DataExporter 新增 exportStreamed 方法 | DataExporter.h, DataExporter.cpp | +80行 |
| 7 | 重构 onExportData() 调用 exportStreamed | MainWindow.cpp | -5行, +12行 |
| 8 | 删除 MainWindow.h 中的 `#include <QStackedWidget>` | MainWindow.h | -1行 |

### 6.2 风险评估

| 风险 | 概率 | 影响 | 缓解措施 |
|------|------|------|---------|
| sendAndRecord 改变了 TimedSender 的行为 (新增 logData) | 高 | 低 | 这是bug修复, 符合预期。但需确认定时发送时录制状态 |
| QStackedWidget -> QWidget 导致布局微小差异 | 低 | 低 | QStackedWidget 默认 NoFrame, QWidget 无边框, 视觉无变化 |
| 分段导出时数据总量在导出过程中发生变化 | 中 | 低 | 使用快照的 totalLines 仅用于循环终止条件, 实际以 provider 返回空为准 |

### 6.3 测试要点

| 测试场景 | 预期结果 |
|---------|---------|
| 手动发送文本 -> 检查终端显示/日志/状态栏 | 三处均正确更新 |
| 手动发送HEX -> 检查终端显示/日志/状态栏 | HEX转换正确, 后续流程一致 |
| 快捷指令发送 -> 检查终端显示/日志/状态栏 | logData 被正确调用 |
| 定时发送 + 录制开启 -> 停止录制后回放 | 定时发送数据出现在回放中 (修复之前的缺失) |
| 发送时连接断开 -> sendAndRecord 返回0 | 终端不显示未发送的数据, 状态栏不更新 |
| 导出50000行TXT -> 检查文件完整性 | 文件行数 = 终端行数, 无截断 |
| 导出50000行CSV -> 检查表头和数据 | 表头正确, 数据完整 |
| 导出50000行BIN -> 比较与旧接口输出 | 二进制内容一致 |
| 导出时新数据持续写入 -> 终端不卡顿 | 分段读取不阻塞UI超过50ms |
| 面板切换 (7个面板来回切换) -> 布局无异常 | 与修改前行为一致 |

---

## 七、遗留项与未来方向

### 7.1 sendAndRecord 的进一步演进

当项目支持多连接 Tab 切换时, sendAndRecord 应从 MainWindow 移出:

```
当前:  MainWindow::sendAndRecord()
未来:  SessionController::sendAndRecord()   // 业务层
       SessionController 持有 IConnection + TerminalModel + DataLogger
       MainWindow 只调用 SessionController 的接口
```

### 7.2 DataExporter 的进一步演进

当前分段导出在时间过滤场景下仍需全量收集。可以改进为:

- TerminalModel 提供 `linesByTimeRange(from, to)` 接口, 利用时间戳有序性进行二分查找定位起止行, 避免全量扫描
- 或在 appendLine 时维护一个稀疏的时间索引 (每100行记录一个时间戳锚点), 支持快速定位

### 7.3 面板容器的进一步演进

如果将来需要真正的多面板 Tab 切换 (如多串口同时显示各自的终端), 建议的架构:

```
QWidget* m_rightPanel             // 顶层容器
  +-- QTabBar* m_sessionTabs      // 连接Tab栏
  +-- QStackedWidget* m_tabPages  // Tab内容页
        +-- SessionPage* page1    // 每个连接一个页面
        +-- SessionPage* page2
```

此时 QStackedWidget 的多页面切换能力才会被充分利用。
