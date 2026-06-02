# PRD-018: 数据导出流式化 + 协议视图导出增强 + 终端渲染优化 + 导航树连接类型图标

## 背景

第五次架构审查周期（commit #20 之后的增量检查）结合 UI/UX 产品体验师观感评审，识别出四个需要在本迭代中解决的跨层问题：

1. **导出性能瓶颈**: PRD-017 R4 提出了 `exportBegin/exportBatch/exportFinish` 的三段式流式导出方案，但实际实现仍使用 `m_terminalModel->lines()` 全量拷贝。需要完成流式化改造，同时引入更抽象的 `LineProvider` 回调模式，使 DataExporter 解耦具体数据源。
2. **协议导出格式单一**: ProtocolView 仅支持 CSV 导出，无法满足结构化数据交换（JSON）和快速单行复制场景。
3. **终端渲染冗余访存**: TerminalWidget::paintEvent 在绘制可见行时通过 `lineAt()` 访问 TerminalModel 获取 direction 字段，但 m_cachedLines 已持有格式化后的文本。缓存中缺少 direction 信息导致每帧额外的 model 访问。
4. **导航树缺少连接类型标识**: CLAUDEMD 6.6 节明确要求"串口(蓝色圆点)、TCP(绿色圆点)、UDP(黄色圆点)、RTT(紫色圆点)"的视觉区分，当前实现中导航树节点均为纯文本，无法快速辨别连接类型。

**审查基准**: PRD-017 交付后，当前评分 21/1000，commit #21。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | DataExporter 流式导出: 引入 LineProvider 回调模式，按批次从数据源拉取数据，避免 50000 行全量深拷贝 | P0 | utils/DataExporter, core/MainWindow |
| R2 | ProtocolView 导出格式增强: 新增 JSON 导出选项 + "复制行" 右键上下文菜单 | P1 | protocol/ProtocolView |
| R3 | TerminalWidget 方向缓存: 在 m_cachedLines 中同步缓存 direction 字段，消除渲染路径上的 model 访问 | P1 | terminal/TerminalWidget |
| R4 | 导航树连接类型图标: 在导航树各连接类型节点前显示语义色圆点 | P1 | core/MainWindow, resources/themes |

## 需求详细说明

---

### R1: DataExporter 流式导出 (LineProvider 回调模式)

#### 问题分析

当前 `MainWindow::onExportData()` (MainWindow.cpp 第776行) 调用:

```cpp
m_dataExporter->exportToFile(filePath, format, m_terminalModel->lines())
```

`m_terminalModel->lines()` 在 TerminalModel.cpp 第36-46行的实现为:

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

当终端缓冲区达到 50000 行上限时的量化影响:

| 指标 | 当前值 | 目标值 |
|------|--------|--------|
| mutex 单次持有时间 | 约 3-5 ms（50000 行深拷贝） | < 0.1 ms（1000 行/批） |
| 内存峰值增量 | 约 3-6 MB（完整 QVector 副本） | 约 60 KB（单批 1000 行） |
| 导出过程中 UI 响应 | 主线程阻塞，界面卡顿 | 分段 yield，UI 流畅 |

TerminalModel 已提供 `lines(int start, int count)` 分段接口（第48-62行），PRD-017 R4 提出了三段式方案但未引入数据源抽象。本需求用 `LineProvider` 回调模式替代硬编码的 TerminalModel 耦合，使 DataExporter 可复用于未来 ProtocolView 导出、DataLogger 回放导出等场景。

#### 方案设计

**Step 1: 定义 LineProvider 回调类型**

```cpp
// DataExporter.h 新增
#include <functional>

// 数据行提供者回调: 给定起始偏移和请求数量，返回实际获取的行
// 返回空 QVector 表示数据耗尽或出错
// 该抽象使 DataExporter 解耦具体数据源 (TerminalModel / ProtocolView / DataLogger)
using LineProvider = std::function<QVector<TerminalLine>(int offset, int count)>;
```

**Step 2: 新增流式导出主接口**

```cpp
// DataExporter.h 新增
public:
    // 流式导出: 通过 LineProvider 按需拉取数据，避免全量拷贝
    // totalLines: 数据源的总行数 (由调用方通过 model->lineCount() 获取)
    // provider:   数据拉取回调，DataExporter 内部按 batchSize 分段调用
    // batchSize:  每次从 provider 拉取的行数，默认 1000
    bool exportStreamed(const QString& filePath, Format format,
                        int totalLines,
                        const LineProvider& provider,
                        int batchSize = 1000);
```

**Step 3: exportStreamed 实现**

```cpp
bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   int totalLines,
                                   const LineProvider& provider,
                                   int batchSize)
{
    if (filePath.isEmpty() || totalLines <= 0 || !provider) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // CSV 格式写表头
    if (format == Csv) {
        stream << "timestamp,direction,data_hex,data_ascii\n";
    }

    bool success = true;
    for (int offset = 0; offset < totalLines && success; offset += batchSize) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) {
            success = false;
            break;
        }

        for (const TerminalLine& line : batch) {
            switch (format) {
            case Txt: {
                QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
                QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
                QString hex = HexConverter::toHexString(line.data);
                QString ascii = toAsciiString(line.data);
                stream << QString("[%1] [%2] %3 | %4\n").arg(timeStr, dirStr, hex, ascii);
                break;
            }
            case Csv: {
                QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
                QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
                QString hex = HexConverter::toHexString(line.data);
                QString ascii = toAsciiString(line.data);
                stream << timeStr << ',' << dirStr << ',' << hex << ','
                       << '"' << ascii << '"' << '\n';
                break;
            }
            case Bin:
                // Bin 格式直接写原始字节，无文本流需求
                file.write(line.data);
                break;
            }
        }
    }

    file.close();
    return success;
}
```

**Step 4: MainWindow::onExportData 改用 exportStreamed**

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

    // 通过 Lambda 捕获 TerminalModel 的分段接口，构建 LineProvider
    LineProvider provider = [this](int offset, int count) -> QVector<TerminalLine> {
        return m_terminalModel->lines(offset, count);
    };

    if (m_dataExporter->exportStreamed(filePath, format, totalLines, provider)) {
        statusBar()->showMessage(tr("Exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file"));
    }
}
```

**Step 5: 保留旧接口向后兼容**

`exportToFile(filePath, format, lines)` 保持原样，内部仍直接处理 QVector。该接口用于不需要流式优化的场景（如测试、外部调用者已持有完整数据集的情况）。

#### 批次大小选择依据

| 批次大小 | 单批内存占用 | mutex 持有时间(估) | I/O 调用次数(50000行) |
|---------|-------------|-------------------|---------------------|
| 500 行 | 约 30 KB | < 0.05 ms | 100 次 |
| **1000 行 (默认)** | **约 60 KB** | **< 0.1 ms** | **50 次** |
| 5000 行 | 约 300 KB | < 0.5 ms | 10 次 |

1000 行/批在内存占用和 I/O 效率之间取得平衡，mutex 持有时间远低于帧间隔(16ms)。

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/utils/DataExporter.h` | 新增 LineProvider 类型定义；新增 exportStreamed 方法声明 |
| `src/utils/DataExporter.cpp` | 实现 exportStreamed 流式导出方法 |
| `src/core/MainWindow.cpp` | onExportData 改用 exportStreamed + Lambda LineProvider |

不影响 TerminalModel、ProtocolView 或其他模块的接口。exportToFile 保持不变。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R1-AC1 | onExportData 不再调用 `m_terminalModel->lines()` 全量接口 | 代码审查: grep `lines()` 在 onExportData 中仅出现于 Lambda 内的 `lines(offset, count)` | 0 次全量调用 |
| R1-AC2 | 50000 行数据导出时 mutex 单次持有时间 < 1 ms | 在 TerminalModel::lines(offset, count) 入口/出口添加 QElapsedTimer 测量 | < 1 ms |
| R1-AC3 | 导出结果与全量导出完全一致 | 测试: 生成 10000 行混合 Rx/Tx 数据，分别用 exportToFile 和 exportStreamed 导出，diff 比较输出文件 | 无差异 |
| R1-AC4 | TXT/CSV/BIN 三种格式均正常导出 | 手动测试: 分别导出 .txt/.csv/.bin 文件，验证内容格式正确 | 全部正确 |
| R1-AC5 | 空数据导出时正确提示 "No data to export" | 手动测试: 清空终端后点击导出按钮 | 提示出现 |
| R1-AC6 | 旧接口 exportToFile 仍可正常调用 | 单元测试: 直接调用 exportToFile 验证输出正确 | 输出正确 |
| R1-AC7 | LineProvider 可用于非 TerminalModel 数据源 | 设计审查: LineProvider 类型定义不依赖 TerminalModel 头文件 | 编译通过且无 TerminalModel include |

---

### R2: ProtocolView 导出格式增强

#### 问题分析

当前 ProtocolView::setupUI (ProtocolView.cpp 第67-103行) 中的导出逻辑:

1. **仅 CSV 格式**: 文件对话框过滤器为 `tr("CSV files (*.csv)")`，无其他格式选项
2. **CSV 写入内联**: 导出逻辑以 Lambda 形式直接写在 m_exportBtn 的 clicked 信号连接中，约 30 行代码无法复用
3. **无行级复制**: 表格支持行选中（SelectRows 模式），但没有右键菜单支持快速复制单行数据

用户场景分析:

| 场景 | 当前体验 | 期望体验 |
|------|---------|---------|
| 将协议帧数据导入 Python/MATLAB 分析 | CSV 可用但需要手动解析列 | JSON 结构化数据可直接 json.loads() |
| 分享单帧异常数据到聊天/文档 | 需要导出全量 CSV 再手动查找 | 右键复制当前行，直接粘贴 |
| 与其他工具交换数据 | CSV 格式通用但缺乏帧结构信息 | JSON 保留字段名-值映射，可读性强 |

#### 方案设计

**Step 1: 导出逻辑提取为独立方法**

```cpp
// ProtocolView.h 新增
private:
    bool exportToCsv(const QString& filePath);
    bool exportToJson(const QString& filePath);

// ProtocolView.cpp
bool ProtocolView::exportToCsv(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);
    stream << "#,Time";
    for (const auto& name : m_fieldNames) {
        stream << "," << name;
    }
    stream << "\n";

    for (int i = 0; i < m_frames.size(); ++i) {
        const auto& frame = m_frames[i];
        stream << (i + 1) << ","
               << frame.value("_frameTime").toString();
        for (const auto& name : m_fieldNames) {
            stream << "," << frame.value(name).toString();
        }
        stream << "\n";
    }
    file.close();
    return true;
}

bool ProtocolView::exportToJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // 使用 QJsonDocument 构建 JSON
    QJsonArray framesArray;
    for (int i = 0; i < m_frames.size(); ++i) {
        const auto& frame = m_frames[i];
        QJsonObject frameObj;
        frameObj["index"] = i + 1;
        frameObj["time"] = frame.value("_frameTime").toString();

        // 用户字段（排除内部 _ 前缀字段）
        QJsonObject fieldsObj;
        for (auto it = frame.constBegin(); it != frame.constEnd(); ++it) {
            if (!it.key().startsWith('_')) {
                fieldsObj[it.key()] = QJsonValue::fromVariant(it.value());
            }
        }
        frameObj["fields"] = fieldsObj;
        framesArray.append(frameObj);
    }

    QJsonDocument doc(framesArray);
    stream << doc.toJson(QJsonDocument::Indented);
    file.close();
    return true;
}
```

**Step 2: 更新导出按钮逻辑，支持 CSV/JSON 格式选择**

```cpp
// ProtocolView.cpp setupUI() 中替换原 m_exportBtn clicked 连接
connect(m_exportBtn, &QPushButton::clicked, this, [this]() {
    if (m_frames.isEmpty()) {
        QMessageBox::information(this, tr("Export"), tr("No data to export"));
        return;
    }

    QString filter = tr("CSV files (*.csv);;JSON files (*.json)");
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Protocol Data"),
                                                     QString(), filter);
    if (filePath.isEmpty()) return;

    bool success = false;
    if (filePath.endsWith(".json", Qt::CaseInsensitive)) {
        success = exportToJson(filePath);
    } else {
        success = exportToCsv(filePath);
    }

    if (success) {
        m_statusLabel->setText(tr("Exported %1 frames").arg(m_frames.size()));
    } else {
        QMessageBox::warning(this, tr("Export"), tr("Cannot write to file"));
    }
});
```

**Step 3: 添加右键 "复制行" 上下文菜单**

```cpp
// ProtocolView.h 新增
private slots:
    void onCopyRow();

private:
    void setupContextMenu();

// 新增成员
    QMenu* m_contextMenu = nullptr;
    QAction* m_copyRowAction = nullptr;

// ProtocolView.cpp
void ProtocolView::setupContextMenu()
{
    m_contextMenu = new QMenu(this);
    m_contextMenu->setObjectName("protocolContextMenu");

    m_copyRowAction = new QAction(tr("Copy Row"), this);
    m_copyRowAction->setObjectName("protocolCopyRowAction");
    m_contextMenu->addAction(m_copyRowAction);

    connect(m_copyRowAction, &QAction::triggered, this, &ProtocolView::onCopyRow);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_table, &QTableView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QModelIndex index = m_table->indexAt(pos);
        if (index.isValid()) {
            m_contextMenu->popup(m_table->viewport()->mapToGlobal(pos));
        }
    });
}

void ProtocolView::onCopyRow()
{
    QModelIndexList selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    int row = selected.first().row();
    if (row < 0 || row >= m_frames.size()) return;

    const QVariantMap& frame = m_frames[row];
    QStringList parts;
    parts << QString("#%1").arg(row + 1);
    parts << frame.value("_frameTime").toString();

    for (const auto& name : m_fieldNames) {
        parts << QString("%1: %2").arg(name, frame.value(name).toString());
    }

    QApplication::clipboard()->setText(parts.join("  |  "));
}
```

**Step 4: 在 setupUI 末尾调用 setupContextMenu()**

```cpp
void ProtocolView::setupUI()
{
    // ... 现有代码不变 ...

    // 右键菜单
    setupContextMenu();
}
```

**Step 5: ProtocolView.h 需要新增 include**

```cpp
#include <QMenu>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QClipboard>
```

#### JSON 输出格式示例

```json
[
  {
    "index": 1,
    "time": "14:32:05.123",
    "fields": {
      "Header": "0xAA55",
      "Length": "12",
      "Cmd": "0x01",
      "CRC": "0xA1B2"
    }
  },
  {
    "index": 2,
    "time": "14:32:05.456",
    "fields": {
      "Header": "0xAA55",
      "Length": "8",
      "Cmd": "0x03",
      "CRC": "0xC3D4"
    }
  }
]
```

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/protocol/ProtocolView.h` | 新增 exportToCsv/exportToJson 方法声明；新增 setupContextMenu 方法；新增 m_contextMenu/m_copyRowAction 成员；新增 QMenu/QJson 相关 include |
| `src/protocol/ProtocolView.cpp` | 提取 CSV 导出为 exportToCsv；新增 exportToJson JSON 导出实现；新增 setupContextMenu 和 onCopyRow 右键菜单；更新导出按钮连接逻辑 |

不影响其他模块。ProtocolView 的公共接口 (addFrame, clear, allFrames) 保持不变。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R2-AC1 | 导出对话框同时提供 CSV 和 JSON 格式选项 | 手动测试: 点击导出按钮，验证文件类型过滤器包含 "CSV files" 和 "JSON files" | 两种格式均列出 |
| R2-AC2 | CSV 导出结果与原有格式完全一致 | 测试: 导出相同数据集的 CSV，与旧版本输出 diff 比较 | 无差异 |
| R2-AC3 | JSON 导出结果为合法 JSON，结构包含 index/time/fields 三层 | 使用 json.loads() 或 jsonformatter.com 验证 | 解析成功且结构正确 |
| R2-AC4 | JSON 中不包含 `_` 前缀的内部字段 (`_rawPayload`, `_rawFrame`, `_frameTime` 不在 fields 内) | 代码审查: 检查 exportToJson 中 `!it.key().startsWith('_')` 过滤逻辑 | 仅 _frameTime 映射到 "time"，其他内部字段不出现 |
| R2-AC5 | 右键点击有效行弹出上下文菜单，包含 "Copy Row" 选项 | 手动测试: 选中一行后右键，验证菜单出现 | 菜单正确弹出 |
| R2-AC6 | 点击 "Copy Row" 后剪贴板包含格式化的行数据 | 手动测试: 复制后粘贴到文本编辑器验证 | 格式为 "#N  time  field1: val  field2: val" |
| R2-AC7 | 右键点击空白区域不弹出菜单 | 手动测试: 在表格空白区域右键 | 无菜单弹出 |
| R2-AC8 | 上下文菜单样式遵循主题语义色 | 观感评审: 检查 m_contextMenu 的 objectName 是否在 QSS 中定义样式 | QSS 中有对应样式 |

---

### R3: TerminalWidget 方向缓存

#### 问题分析

当前 `TerminalWidget::paintEvent` (TerminalWidget.cpp 第110-191行) 的渲染循环:

```cpp
for (int i = startLine; i < endLine; ++i) {
    const TerminalLine& line = m_model->lineAt(i);  // 每行都访问 model

    // 选择背景色
    if (i >= m_selectionStartLine && ...) { ... }

    // 根据方向设置文字颜色 -- 这里需要 direction
    if (line.direction == DataDirection::Tx) {
        painter.setPen(m_txColor);
    } else {
        painter.setPen(m_rxColor);
    }

    // 绘制时间戳后再次恢复方向颜色
    if (m_showTimestamp) {
        // ...
        if (line.direction == DataDirection::Tx) {   // 第二次读取 direction
            painter.setPen(m_txColor);
        } else {
            painter.setPen(m_rxColor);
        }
    }

    painter.drawText(xOffset + 4, y + m_lineHeight - 4, m_cachedLines[i]);
}
```

**问题量化**:

| 指标 | 当前值 | 目标值 |
|------|--------|--------|
| 每帧 lineAt() 调用次数 | 可见行数 x 1（获取 direction） | 0（从缓存读取） |
| lineAt() 内部开销 | QMutexLocker + 数组索引（非阻塞但仍有锁竞争风险） | 无（纯内存读取） |
| direction 判断次数 | 可见行数 x 2（数据颜色 + 时间戳后恢复颜色） | 可见行数 x 2（判断次数不变，但数据来源从 model 改为缓存） |

可见行数典型值: 40-60 行（800px 窗口 / 18px 行高）。高流量数据场景下，paintEvent 可能以 30-60fps 频率触发，此时 mutex 锁竞争会影响数据接收线程的 appendReceived 性能。

**注意**: 代码中 `lineAt()` 的调用不仅用于 direction，还在第155行获取整个 `TerminalLine&` 引用用于后续的 `line.timestamp` 和 `line.direction` 读取。将 direction 和 timestamp 同时缓存可以完全消除 paintEvent 中的 model 访问。

#### 方案设计

**Step 1: 扩展缓存结构体**

```cpp
// TerminalWidget.h 新增缓存行结构
struct CachedLine {
    QString formattedText;       // 格式化后的显示文本（原 m_cachedLines 内容）
    DataDirection direction;     // 数据方向，避免渲染时回查 model
    QString timestamp;           // 预格式化的时间戳字符串，避免每帧重新 toString()
};

// 成员变量变更
mutable QVector<CachedLine> m_cachedLines;   // 替换原 QVector<QString>
mutable int m_cachedLineCount = 0;
```

**Step 2: 更新缓存填充逻辑**

```cpp
// TerminalWidget.cpp 修改 formatLine 方法和缓存更新逻辑

// formatLine 返回 CachedLine 而非 QString
TerminalWidget::CachedLine TerminalWidget::formatLine(const TerminalLine& line) const
{
    CachedLine cached;
    cached.direction = line.direction;
    cached.timestamp = line.timestamp.toString("HH:mm:ss.zzz");

    switch (m_displayMode) {
    case DisplayMode::Hex:
        cached.formattedText = HexConverter::toHexString(line.data);
        break;
    case DisplayMode::Mixed:
        cached.formattedText = QString::fromUtf8(line.data) + "  |  "
                               + HexConverter::toHexString(line.data);
        break;
    case DisplayMode::Text:
    default:
        cached.formattedText = QString::fromUtf8(line.data);
        break;
    }

    return cached;
}
```

**Step 3: 更新 paintEvent 渲染循环**

```cpp
void TerminalWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), m_bgColor);
    painter.setFont(m_font);

    if (!m_model) {
        painter.setPen(m_timestampColor);
        painter.drawText(rect(), Qt::AlignCenter, tr("未连接 - 等待数据..."));
        return;
    }

    int totalLines = m_model->lineCount();

    if (m_cachedLineCount != totalLines) {
        m_cachedLines.resize(totalLines);
        for (int i = m_cachedLineCount; i < totalLines; ++i) {
            m_cachedLines[i] = formatLine(m_model->lineAt(i));  // 仅增量时访问 model
        }
        m_cachedLineCount = totalLines;
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) {
            m_scrollOffset = m_maxScrollOffset;
        }
    }

    int startLine = m_scrollOffset;
    int endLine = qMin(startLine + m_visibleLines + 1, totalLines);

    int y = 0;
    for (int i = startLine; i < endLine; ++i) {
        const CachedLine& cached = m_cachedLines[i];  // 从缓存读取，不访问 model

        if (i >= m_selectionStartLine && i <= m_selectionEndLine
            && m_selectionStartLine >= 0) {
            painter.fillRect(0, y, width(), m_lineHeight, m_selectionBg);
        }

        // 方向颜色: 直接从缓存读取
        QColor dataColor = (cached.direction == DataDirection::Tx) ? m_txColor : m_rxColor;
        painter.setPen(dataColor);

        int xOffset = 0;
        if (m_showTimestamp) {
            painter.setPen(m_timestampColor);
            painter.drawText(4, y + m_lineHeight - 4, cached.timestamp);
            QFontMetrics fm(m_font);
            xOffset = fm.horizontalAdvance(cached.timestamp) + 12;
            painter.setPen(dataColor);  // 恢复方向颜色，无需再次读取 direction
        }

        painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.formattedText);
        y += m_lineHeight;
    }
}
```

**Step 4: 更新相关方法签名**

```cpp
// TerminalWidget.h 中 formatLine 返回类型变更
private:
    CachedLine formatLine(const TerminalLine& line) const;
```

**Step 5: 其他引用 m_cachedLines 的位置适配**

- `setModel()`: `m_cachedLines.clear()` 不变（QVector<CachedLine> 的 clear 语义与 QVector<QString> 一致）
- `clear()`: 同上
- `onDataCleared()`: 同上
- `setDisplayMode()`: `m_cachedLineCount = 0` 触发全量重新格式化，逻辑不变

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/terminal/TerminalWidget.h` | 新增 CachedLine 结构体；m_cachedLines 类型改为 QVector<CachedLine>；formatLine 返回类型改为 CachedLine |
| `src/terminal/TerminalWidget.cpp` | paintEvent 渲染循环改为从 CachedLine 读取 direction/timestamp；formatLine 实现返回 CachedLine |

不影响 TerminalModel 接口。不影响其他模块。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R3-AC1 | paintEvent 渲染循环中不调用 `m_model->lineAt()` | 代码审查: paintEvent 内部无 `lineAt` 或 `m_model->` 调用 | 0 次调用 |
| R3-AC2 | 终端渲染效果与修改前完全一致（Rx 白色文字，Tx 绿色文字，时间戳灰色） | 手动测试: 发送和接收数据，检查颜色和格式正确 | 视觉无差异 |
| R3-AC3 | 显示模式切换（文本/HEX/混合）功能正常 | 手动测试: 切换三种模式，验证显示内容正确 | 三种模式正确 |
| R3-AC4 | 时间戳显示/隐藏切换功能正常 | 手动测试: 开关时间戳，验证时间戳正确显示/隐藏 | 切换正常 |
| R3-AC5 | 滚动浏览历史数据时渲染正确 | 手动测试: 发送大量数据后滚轮上滚，验证各行颜色和数据正确 | 渲染正确 |
| R3-AC6 | CachedLine 内存增量可接受 | 估算: 50000 行 x (QString ~24B + DataDirection 4B + QString ~20B) 约 2.4 MB | < 5 MB |

---

### R4: 导航树连接类型视觉指示器

#### 问题分析

CLAUDEMD 6.6 节"树形导航"设计规范明确要求:

> 连接类型图标: 串口(蓝色圆点)、TCP(绿色圆点)、UDP(黄色圆点)、RTT(紫色圆点)

当前 `MainWindow::setupUI()` (MainWindow.cpp 第60-130行) 中导航树节点均为纯文本 QStandardItem:

```cpp
auto* serialItem = new QStandardItem(tr("串口"));
// ...
auto* networkItem = new QStandardItem(tr("网络"));
auto* tcpClientItem = new QStandardItem(tr("TCP客户端"));
auto* udpItem = new QStandardItem(tr("UDP"));
```

分组节点（串口、网络、工具）和叶子节点均无视觉区分，用户无法快速识别连接类型。

**需要标记颜色的节点**:

| 节点 | 连接类型 | 圆点颜色 | 语义色映射 |
|------|---------|---------|-----------|
| 串口 (分组) | Serial | 蓝色 | `#89b4fa`（accent） |
| TCP客户端 (叶子) | TcpClient | 绿色 | `#a6e3a1`（success） |
| TCP服务端 (叶子) | TcpServer | 绿色 | `#a6e3a1`（success） |
| UDP (叶子) | Udp | 黄色 | `#f9e2af`（warning） |
| RTT (预留) | Rtt | 紫色 | `#cba6f7`（ lavender） |

#### 方案设计

**Step 1: 创建彩色圆点 QPixmap 工具方法**

```cpp
// MainWindow.h 新增
private:
    // 创建指定颜色的圆形图标 (12x12, 内含 8x8 圆点)
    static QPixmap createDotIcon(const QColor& color);
```

```cpp
// MainWindow.cpp
QPixmap MainWindow::createDotIcon(const QColor& color)
{
    QPixmap pixmap(12, 12);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(2, 2, 8, 8);  // 8px 直径圆点，上下左右留 2px 边距

    return pixmap;
}
```

**Step 2: 在导航树构建时设置图标**

```cpp
void MainWindow::setupUI()
{
    // ... 现有代码 ...

    // 串口分组 - 蓝色圆点
    serialItem->setIcon(createDotIcon(QColor("#89b4fa")));

    // ... 串口子节点（配置、终端、统计等）不需要图标 ...

    // 网络分组 - 不设图标（网络本身不是连接类型）
    // 网络子节点按连接类型标记
    tcpClientItem->setIcon(createDotIcon(QColor("#a6e3a1")));   // 绿色
    tcpServerItem->setIcon(createDotIcon(QColor("#a6e3a1")));   // 绿色
    udpItem->setIcon(createDotIcon(QColor("#f9e2af")));          // 黄色

    // ... 其余代码不变 ...
}
```

**Step 3: 确保导航树 iconSize 合适**

```cpp
m_navTree = new QTreeView;
m_navTree->setObjectName("navTree");
m_navTree->setHeaderHidden(true);
m_navTree->setIconSize(QSize(12, 12));   // 新增: 匹配圆点图标尺寸
m_navTree->setMinimumWidth(180);
m_navTree->setMaximumWidth(280);
m_navTree->setIndentation(16);
```

**Step 4: 主题兼容考虑**

硬编码颜色（如 `#89b4fa`）仅用于装饰性圆点图标，不属于语义色板的"功能色"使用场景。原因:

1. 圆点图标是通过 `createDotIcon()` 在 C++ 代码中用 QPainter 绘制的，无法通过 QSS 控制
2. 这些颜色与 QSS 语义色板中的 `--accent`、`--success`、`--warning` 保持一致
3. 主题切换时，如果需要圆点颜色跟随主题变化，后续可通过 ThemeManager::semanticColor() 接口动态获取

当前阶段使用与暗色主题一致的固定色值。当 PRD-014 的 ThemeManager 语义色查询接口完善后，可改为动态获取。

**RTT 预留**: RTT 节点尚未添加到导航树（依赖 JLinkBridge 集成），本需求不创建 RTT 节点项，但在 Constants.h 或后续代码中预留紫色 `#cba6f7` 的约定。

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/MainWindow.h` | 新增 createDotIcon 静态方法声明 |
| `src/core/MainWindow.cpp` | 新增 createDotIcon 实现；setupUI 中为串口/TCP/UDP 节点设置图标；设置 navTree iconSize |

不涉及 TerminalModel、DataExporter 或其他模块。

#### 验收标准

| 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|---------|---------|---------|
| R4-AC1 | 串口分组节点左侧显示蓝色圆点 | 手动测试: 启动应用，观察导航树 | 蓝色圆点可见 |
| R4-AC2 | TCP客户端和 TCP服务端节点显示绿色圆点 | 手动测试: 展开网络分组 | 绿色圆点可见 |
| R4-AC3 | UDP 节点显示黄色圆点 | 手动测试: 展开网络分组 | 黄色圆点可见 |
| R4-AC4 | 配置/终端/统计/协议/帧编辑器/波形图/OTA/数据导出 节点无圆点图标 | 手动测试: 检查各叶子节点 | 无图标 |
| R4-AC5 | 圆点大小统一，不遮挡文字，不造成行高异常 | 观感评审: 对比有/无图标的行高度一致 | 视觉统一 |
| R4-AC6 | 圆点颜色在暗色/亮色主题下均清晰可辨 | 手动测试: 切换三种主题，观察圆点 | 三种主题下均可辨别 |
| R4-AC7 | 导航树点击选中、展开折叠行为不受影响 | 手动测试: 点击各节点切换面板，展开/折叠分组 | 功能正常 |

---

## 接口设计

### 新增接口

| 接口 | 所属类 | 文件 | 用途 |
|------|--------|------|------|
| `LineProvider` (using) | DataExporter | `utils/DataExporter.h` | 数据行拉取回调类型定义 |
| `exportStreamed()` | DataExporter | `utils/DataExporter.h/cpp` | 流式导出主入口 |
| `exportToCsv()` | ProtocolView | `protocol/ProtocolView.h/cpp` | CSV 导出（从 Lambda 提取） |
| `exportToJson()` | ProtocolView | `protocol/ProtocolView.h/cpp` | JSON 导出新接口 |
| `setupContextMenu()` | ProtocolView | `protocol/ProtocolView.h/cpp` | 右键菜单初始化 |
| `onCopyRow()` [slot] | ProtocolView | `protocol/ProtocolView.h/cpp` | 复制行数据到剪贴板 |
| `CachedLine` (struct) | TerminalWidget | `terminal/TerminalWidget.h` | 缓存行数据结构（文本+方向+时间戳） |
| `createDotIcon()` [static] | MainWindow | `core/MainWindow.h/cpp` | 创建彩色圆点 QPixmap |

### 变更接口

| 接口 | 变更类型 | 影响分析 |
|------|---------|---------|
| `DataExporter::exportToFile()` | 不变 | 保留旧接口向后兼容 |
| `TerminalWidget::formatLine()` | 返回类型从 QString 改为 CachedLine | 仅 Widget 内部使用，无外部调用者 |
| `TerminalWidget::m_cachedLines` | 元素类型从 QString 改为 CachedLine | 仅 Widget 内部使用 |

### 新增信号

本 PRD 不新增信号。

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 | 涉及需求 |
|------|------|---------|---------|
| HexConverter | `utils/HexConverter.h` | exportStreamed 内部复用 toHexString() | R1 |
| TerminalModel | `terminal/TerminalModel.h/cpp` | 使用已有的 lines(offset, count) 分段接口 | R1 |
| Constants | `core/Constants.h` | DataDirection 枚举定义（CachedLine 中使用） | R3 |
| QApplication::clipboard() | Qt 内置 | 复制行数据到系统剪贴板 | R2 |
| QJsonDocument/QJsonArray/QJsonObject | Qt 内置 | JSON 序列化 | R2 |
| ThemeManager | `core/ThemeManager.h/cpp` | 后续可扩展: createDotIcon 从 ThemeManager 获取语义色 | R4 (预留) |

---

## 设计模式

| 模式 | 应用场景 | 涉及需求 | 说明 |
|------|---------|---------|------|
| **回调/策略模式 (Callback/Strategy)** | LineProvider 回调抽象数据源 | R1 | 通过 std::function 将数据获取逻辑从导出逻辑中解耦，DataExporter 不依赖具体 Model 类型 |
| **提取方法 (Extract Method)** | ProtocolView 导出逻辑从 Lambda 提取为独立方法 | R2 | 消除 Lambda 中的大段内联代码，提高可测试性和可读性 |
| **数据局部性优化 (Data Locality)** | CachedLine 结构体将渲染所需的文本/方向/时间戳连续存储 | R3 | 缓存命中时无需跨越对象边界访问 model，减少 cache miss |
| **工厂方法 (Factory Method)** | createDotIcon 静态方法统一创建不同颜色的圆点图标 | R4 | 集中图标创建逻辑，避免分散的 QPixmap 构造代码 |

---

## 影响范围

### 文件变更矩阵

| 文件 | R1 | R2 | R3 | R4 |
|------|----|----|----|-----|
| `src/utils/DataExporter.h` | 新增 LineProvider 类型、exportStreamed 声明 | - | - | - |
| `src/utils/DataExporter.cpp` | 实现 exportStreamed | - | - | - |
| `src/protocol/ProtocolView.h` | - | 新增 exportToCsv/exportToJson/setupContextMenu/onCopyRow 声明；新增 QMenu/QJson include；新增 m_contextMenu/m_copyRowAction 成员 | - | - |
| `src/protocol/ProtocolView.cpp` | - | 提取 CSV 导出方法；实现 JSON 导出；实现右键菜单；更新导出按钮逻辑 | - | - |
| `src/terminal/TerminalWidget.h` | - | - | 新增 CachedLine 结构体；m_cachedLines 类型变更；formatLine 返回类型变更 | - |
| `src/terminal/TerminalWidget.cpp` | - | - | paintEvent 渲染循环重构；formatLine 实现变更 | - |
| `src/core/MainWindow.h` | - | - | - | 新增 createDotIcon 静态方法声明 |
| `src/core/MainWindow.cpp` | onExportData 改用 exportStreamed | - | - | 新增 createDotIcon 实现；setupUI 中设置节点图标和 iconSize |

### 预计变更量

| 需求 | 新增行数(估) | 修改行数(估) | 删除行数(估) |
|------|------------|------------|------------|
| R1 | 约 80 行 | 约 20 行 | 约 5 行 |
| R2 | 约 90 行 | 约 20 行 | 约 25 行 |
| R3 | 约 15 行 | 约 30 行 | 约 10 行 |
| R4 | 约 15 行 | 约 8 行 | 0 行 |
| **合计** | **约 200 行** | **约 78 行** | **约 40 行** |

### 跨模块影响评估

- **R1 vs R2**: 两者都涉及导出功能，但作用于不同数据源（终端数据 vs 协议帧数据）。R1 的 LineProvider 模式为 R2 未来接入统一导出框架预留了扩展点，但本迭代不强制 R2 使用 LineProvider。
- **R3 独立**: TerminalWidget 缓存变更不影响其他模块，TerminalModel 接口不变。
- **R4 独立**: 导航树图标变更仅影响 MainWindow 的 setupUI 方法，不影响面板切换或导航逻辑。

---

## 验收标准总表

| 需求 | 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|------|---------|---------|---------|
| R1 | R1-AC1 | 不调用 lines() 全量接口 | 代码审查 | 0 次全量调用 |
| R1 | R1-AC2 | mutex 单次持有时间 < 1 ms | QElapsedTimer 测量 | < 1 ms |
| R1 | R1-AC3 | 流式/全量导出结果一致 | diff 比较输出文件 | 无差异 |
| R1 | R1-AC4 | TXT/CSV/BIN 三种格式正常 | 手动测试 | 全部正确 |
| R1 | R1-AC5 | 空数据提示正确 | 手动测试 | 提示出现 |
| R1 | R1-AC6 | 旧接口 exportToFile 仍可用 | 单元测试 | 输出正确 |
| R1 | R1-AC7 | LineProvider 不依赖 TerminalModel | 编译检查 | 无 include 依赖 |
| R2 | R2-AC1 | 导出对话框含 CSV 和 JSON 选项 | 手动测试 | 两种格式列出 |
| R2 | R2-AC2 | CSV 导出格式与原来一致 | diff 比较 | 无差异 |
| R2 | R2-AC3 | JSON 导出合法且结构正确 | json.loads() 验证 | 解析成功 |
| R2 | R2-AC4 | JSON 不含内部字段 | 代码审查 + 输出验证 | 仅 time 映射 |
| R2 | R2-AC5 | 右键菜单包含 "Copy Row" | 手动测试 | 菜单出现 |
| R2 | R2-AC6 | 复制行内容格式正确 | 粘贴验证 | 格式正确 |
| R2 | R2-AC7 | 空白区域右键无菜单 | 手动测试 | 无弹出 |
| R2 | R2-AC8 | 上下文菜单遵循主题样式 | QSS 检查 | objectName 已定义 |
| R3 | R3-AC1 | paintEvent 不调用 lineAt() | 代码审查 | 0 次调用 |
| R3 | R3-AC2 | 渲染颜色效果不变 | 手动测试 | 视觉无差异 |
| R3 | R3-AC3 | 显示模式切换正常 | 手动测试 | 三种模式正确 |
| R3 | R3-AC4 | 时间戳开关正常 | 手动测试 | 切换正确 |
| R3 | R3-AC5 | 滚动浏览渲染正确 | 手动测试 | 渲染正确 |
| R3 | R3-AC6 | CachedLine 内存增量 < 5 MB | 估算 | < 5 MB |
| R4 | R4-AC1 | 串口节点蓝色圆点 | 手动测试 | 可见 |
| R4 | R4-AC2 | TCP 节点绿色圆点 | 手动测试 | 可见 |
| R4 | R4-AC3 | UDP 节点黄色圆点 | 手动测试 | 可见 |
| R4 | R4-AC4 | 非连接节点无图标 | 手动测试 | 无图标 |
| R4 | R4-AC5 | 圆点大小统一不遮挡 | 观感评审 | 视觉统一 |
| R4 | R4-AC6 | 三种主题下圆点可辨 | 手动测试 | 均可辨别 |
| R4 | R4-AC7 | 导航树交互功能不受影响 | 手动测试 | 功能正常 |

---

## 实施优先级

| 顺序 | 需求 | 优先级 | 理由 |
|------|------|--------|------|
| 1 | **R1 (P0)** | 数据导出流式化 | 影响用户体验的最严重性能瓶颈：50000 行数据导出导致 UI 卡顿和内存峰值 |
| 2 | **R3 (P1)** | 终端方向缓存 | 渲染性能优化，且实现简单、风险低、变更范围小 |
| 3 | **R2 (P1)** | 协议视图导出增强 | 功能增强，涉及新增 JSON 序列化和右键菜单，变更量较大 |
| 4 | **R4 (P1)** | 导航树图标 | 纯 UI 观感改进，不影响核心功能，变更量最小 |

**并行策略**:
- R1 和 R3 无依赖关系，可由核心开发（R1）和 UI 开发（R3）同时进行
- R2 由协议开发独立完成
- R4 由 UI 开发在 R3 完成后追加（共用同一文件 TerminalWidget 不冲突，R4 改的是 MainWindow）

---

## 验证度量指标

### 性能度量

| 度量项 | 度量方法 | 当前基线 | 目标值 |
|--------|---------|---------|--------|
| 导出 50000 行耗时 | QElapsedTimer 测量 onExportData 总时间 | 约 100-200 ms | < 150 ms（流式后文件 I/O 瓶颈） |
| 导出期间 mutex 单次持有时间 | 在 lines(offset, count) 内测量 | 约 3-5 ms | < 0.1 ms |
| 导出期间内存峰值增量 | 任务管理器观察或自定义分配器统计 | 约 3-6 MB | < 100 KB |
| paintEvent 中 model 访问次数 | 在 lineAt() 入口计数 | 每帧 40-60 次 | 每帧 0 次（仅增量格式化时访问） |
| paintEvent 执行时间 | QElapsedTimer 测量 | 约 0.3-0.5 ms | < 0.3 ms（消除 mutex 开销） |

### 功能度量

| 度量项 | 度量方法 | 目标值 |
|--------|---------|--------|
| JSON 导出解析成功率 | 使用 QJsonDocument::fromJson() 验证输出 | 100% |
| 复制行数据完整性 | 粘贴后与表格显示内容比对 | 字段值完全匹配 |
| 导航树节点图标覆盖率 | 检查所有连接类型节点是否设置了对应颜色圆点 | 100%（串口/TCP/UDP） |
