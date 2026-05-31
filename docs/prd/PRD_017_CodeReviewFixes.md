# PRD-017: 代码审查修复 (ARCH_REVIEW_004 缺陷修补)

## 背景

第四次架构审查 (ARCH_REVIEW_004) 在 commit #20 里程碑执行，审查报告编号 D-002、P-001、P0-02 等条目发现了四项需要在下个迭代中修复的代码质量问题。这些问题不影响当前功能运行，但会导致内存泄漏、性能退化、架构语义混乱和可维护性风险。

本 PRD 针对这四个独立问题逐一制定修复方案、验收标准和度量方法。

**审查基准**: ARCH_REVIEW_004，当前评分 20/1000，commit #20。

## 需求列表

| ID | 需求描述 | 审查编号 | 优先级 | 涉及模块 |
|----|---------|---------|--------|---------|
| R1 | 发送逻辑统一: 提取 sendAndRecord(QByteArray, bool isHex) 方法，消除三处重复 | D-002, V-001 | P1 | core/MainWindow |
| R2 | Completer 内存泄漏: 复用单一 QStringListModel，每次 historyChanged 只更新数据而不 new 新 model | 新发现 | P1 | core/MainWindow |
| R3 | QStackedWidget 误用: m_rightPanel 声明为 QStackedWidget 但只添加一个 page，改为普通 QWidget | 架构语义 | P2 | core/MainWindow |
| R4 | TerminalModel 导出优化: onExportData 利用 lines(start, count) 分段接口替代 lines() 全量拷贝 | P0-02, P-001 | P0 | core/MainWindow, utils/DataExporter |

## 需求详细说明

---

### R1: 发送逻辑统一

#### 问题分析

当前 MainWindow 中有三处执行"发送数据并记录"的代码:

1. **onSendData()** (第686-720行): 手动输入发送，包含 HEX 解析、错误提示、历史记录、输入框清空等 UI 专属逻辑
2. **onQuickCommand()** (第722-731行): 快捷指令发送，只有 write + appendSent + logData + updateStatusBar
3. **TimedSender 回调** (第510-516行): 定时发送器回调，与 onQuickCommand 逻辑完全一致

三处共同的核心操作:
- `m_currentConn->write(data)` -- 通过连接写入
- `m_terminalModel->appendSent(data)` -- 记录到终端模型
- `m_dataLogger->logData(data, DataLogger::Direction::Sent)` -- 记录到日志
- `updateStatusBar()` -- 更新状态栏字节数

三处共同的连接状态前置检查:
- `m_currentConn && m_currentConn->state() == ConnectionState::Connected`

#### 方案设计

提取 `sendAndRecord(const QByteArray& data)` 私有方法，封装核心发送逻辑。onSendData 中保留 HEX 解析、错误提示、历史记录等 UI 专属逻辑，最终调用 sendAndRecord 完成实际发送。

```cpp
// MainWindow.h 新增声明
private:
    void sendAndRecord(const QByteArray& data);

// MainWindow.cpp 实现
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

**调用点改造**:

- `onSendData()`: HEX 解析后调用 `sendAndRecord(data)`，成功后额外执行 `m_sendHistory->addEntry()` 和 `m_sendInput->clear()`
- `onQuickCommand()`: 直接调用 `sendAndRecord(data)`
- TimedSender 回调: 直接调用 `sendAndRecord(data)`

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/MainWindow.h` | 新增 sendAndRecord 私有方法声明 |
| `src/core/MainWindow.cpp` | 新增 sendAndRecord 实现；重构 onSendData、onQuickCommand、TimedSender 回调 |

不涉及任何接口变更，不影响其他模块。

#### 验收标准

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| R1-AC1 | 发送逻辑在三处调用点不存在重复的 write+appendSent+logData+updateStatusBar 序列 | 代码审查: grep 搜索 `appendSent` 出现次数，期望仅在 sendAndRecord 内出现 1 次 |
| R1-AC2 | 手动输入发送功能行为不变(文本/HEX模式、错误提示、历史记录、输入框清空) | 手动测试: 文本模式发送 "hello"、HEX模式发送 "AA 55"、HEX错误输入 "ZZ" 验证红框提示 |
| R1-AC3 | 快捷指令发送功能行为不变 | 手动测试: 点击 AT/Reset/Status 按钮验证终端显示发送数据 |
| R1-AC4 | 定时发送功能行为不变 | 手动测试: 配置定时发送间隔 1000ms，启动后验证终端每秒显示发送数据 |

---

### R2: Completer 内存泄漏

#### 问题分析

当前代码 (MainWindow.cpp 第519-521行):

```cpp
connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
    m_sendCompleter->setModel(new QStringListModel(m_sendHistory->recentTexts(), this));
});
```

每次 `historyChanged` 信号触发时，都会 `new QStringListModel(...)` 创建新的 model 并传给 completer。`QCompleter::setModel()` 的文档说明: completer 不会接管旧 model 的所有权，也不会自动删除旧 model。由于新 model 的 parent 设为 `this`(MainWindow)，旧 model 只有在 MainWindow 析构时才会被删除。

**量化影响**:
- 用户每发送一条命令触发一次 historyChanged
- 每次泄漏一个 QStringListModel 对象(约 100-500 字节 + 内部 QStringList 数据)
- 长时间运行(例如调试会话 8 小时，发送 500 条命令)累积 500 个泄漏的 QStringListModel
- 由于 parent 机制，MainWindow 关闭时全部释放，不是真正的硬泄漏，但运行时内存持续增长

#### 方案设计

在 MainWindow 中持有一个 `QStringListModel* m_completerModel` 成员，初始化时创建一次。historyChanged 时只调用 `setStringList()` 更新数据。

```cpp
// MainWindow.h 变更
private:
    QStringListModel* m_completerModel;  // 新增: completer 复用的单一 model

// MainWindow.cpp setupUI() 中 (替代第217-219行)
m_completerModel = new QStringListModel(m_sendHistory->recentTexts(), this);
m_sendCompleter = new QCompleter(m_completerModel, this);
m_sendCompleter->setCaseSensitivity(Qt::CaseInsensitive);
m_sendCompleter->setCompletionMode(QCompleter::PopupCompletion);
m_sendInput->setCompleter(m_sendCompleter);

// MainWindow.cpp connectSignals() 中 (替代第519-521行)
connect(m_sendHistory, &SendHistory::historyChanged, this, [this]() {
    m_completerModel->setStringList(m_sendHistory->recentTexts());
});
```

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/MainWindow.h` | 新增 m_completerModel 成员变量 |
| `src/core/MainWindow.cpp` | setupUI 中初始化 m_completerModel；connectSignals 中改为 setStringList |

不涉及 SendHistory 或其他模块的接口变更。

#### 验收标准

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| R2-AC1 | historyChanged 触发后不创建新的 QStringListModel 对象 | 代码审查: grep 搜索 `new QStringListModel` 在 MainWindow.cpp 中仅出现 1 次(初始化) |
| R2-AC2 | 发送历史自动补全功能正常工作 | 手动测试: 发送 "AT" 后，在输入框输入 "A"，弹出补全列表包含 "AT" |
| R2-AC3 | 连续发送 50 条不同命令后，内存中 QStringListModel 实例数恒为 1 | 通过在 sendAndRecord 前后打印 m_completerModel 指针地址验证不变 |
| R2-AC4 | 补全列表随历史更新正确变化 | 手动测试: 发送 "test1"、"test2" 后，输入 "t" 弹出补全列表包含两条 |

---

### R3: QStackedWidget 误用

#### 问题分析

当前 MainWindow.h 第146行声明:
```cpp
QStackedWidget* m_rightPanel;
```

但 MainWindow.cpp 第137-233行的实际用法:
```cpp
m_rightPanel = new QStackedWidget;
// ...
auto* serialPanel = new QWidget;
// 所有面板(serialConfig, dataStats, protocolView, ...)都添加到 serialPanel 中
// 通过 show/hide 切换面板
// 只有 serialPanel 被 addWidget 到 m_rightPanel
m_rightPanel->addWidget(serialPanel);
m_rightPanel->setCurrentIndex(0);
```

整个应用生命周期中 `m_rightPanel` 只包含一个 page (serialPanel)，所有面板切换都通过 `switchToPanel()` 方法中的 `setVisible(true/false)` 实现。QStackedWidget 的核心功能(多 page 索引切换、setCurrentIndex)从未被使用。

**问题分类**: 架构语义错误 -- 用了 QStackedWidget 的类型但没有使用其语义。对 Qt 框架阅读者造成误导，让人误以为面板切换通过 StackedWidget 的 page 机制实现。

#### 方案设计

**方案: 将 m_rightPanel 从 QStackedWidget 改为 QWidget**

由于面板切换已经通过 `switchToPanel()` 方法中的 show/hide + 淡入淡出动画实现，QStackedWidget 完全多余。改为普通 QWidget 不影响任何功能。

```cpp
// MainWindow.h 变更
#include <QStackedWidget>  // 删除此行
// ...
QWidget* m_rightPanel;     // 类型从 QStackedWidget* 改为 QWidget*

// MainWindow.cpp setupUI() 中
m_rightPanel = new QWidget;  // 从 new QStackedWidget 改为 new QWidget
auto* rightLayout = new QVBoxLayout(m_rightPanel);
rightLayout->setContentsMargins(0, 0, 0, 0);
rightLayout->setSpacing(0);

// serialPanel 直接添加到 rightLayout，不再经过 addWidget
auto* serialPanel = new QWidget;
// ... serialPanel 内部构造不变 ...
rightLayout->addWidget(serialPanel);  // 直接添加，非 addWidget 到 StackedWidget

// 删除以下两行:
// m_rightPanel->addWidget(serialPanel);
// m_rightPanel->setCurrentIndex(0);
```

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/core/MainWindow.h` | 移除 `#include <QStackedWidget>`；m_rightPanel 类型改为 QWidget* |
| `src/core/MainWindow.cpp` | setupUI 中 new QWidget 替代 new QStackedWidget；移除 addWidget/setCurrentIndex 调用 |

不影响面板切换逻辑(由 switchToPanel 方法独立处理)。

#### 验收标准

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| R3-AC1 | 代码中不存在 QStackedWidget 相关的类型和头文件引用 | grep 搜索 `QStackedWidget` 在 src/ 目录返回 0 结果 |
| R3-AC2 | 编译零错误零警告 | cmake --build build 成功 |
| R3-AC3 | 所有面板切换功能正常(配置、终端、统计、协议、帧编辑器、波形图、OTA升级) | 手动测试: 逐一点击导航树 7 个面板节点，验证面板正确显示/隐藏 |
| R3-AC4 | 面板切换动画正常(淡入淡出过渡) | 手动测试: 从终端切换到配置面板，观察 250ms 淡入动画 |
| R3-AC5 | 窗口 resize 时布局不错乱 | 手动测试: 拖拽窗口边缘改变大小，导航树和面板正确缩放 |

---

### R4: TerminalModel 导出优化

#### 问题分析

当前 `MainWindow::onExportData()` (第751-775行) 调用 `m_terminalModel->lines()` 获取全量数据:

```cpp
if (m_dataExporter->exportToFile(filePath, format, m_terminalModel->lines())) {
```

`lines()` 方法 (TerminalModel.cpp 第36-46行) 在持有 mutex 期间构建完整的 QVector 深拷贝。当终端缓冲区达到 50000 行上限时:
- 锁持有时间: 遍历 50000 个元素并深拷贝，约 2-4 MB 数据
- 内存峰值: 原始数据 2-4 MB + 拷贝数据 2-4 MB = 4-8 MB
- UI 阻塞: 在主线程执行，导出期间界面短暂卡死

TerminalModel 已经提供了 `lines(int start, int count)` 分段接口 (第48-62行)，但 DataExporter 的接口只接受完整的 QVector 参数，需要重构为流式接口。

#### 方案设计

**方案: DataExporter 增加流式导出接口，MainWindow 按批次传递数据**

Step 1: DataExporter 新增流式导出方法

```cpp
// DataExporter.h 新增
public:
    // 流式导出: 多次调用 writeBatch 写入，最后调用 finishClose 关闭文件
    bool exportBegin(const QString& filePath, Format format);
    bool exportBatch(const QVector<TerminalLine>& batch);
    bool exportFinish();

private:
    QFile m_exportFile;                        // 流式导出文件句柄
    QTextStream m_exportStream;                // 文本流(仅 TXT/CSV 使用)
    Format m_exportFormat = Txt;               // 当前导出格式
    bool m_exporting = false;                  // 导出进行中标志
    bool m_firstBatch = true;                  // 首批标志(CSV 写表头)
```

Step 2: MainWindow::onExportData 使用分段接口

```cpp
void MainWindow::onExportData()
{
    if (m_terminalModel->lineCount() == 0) {
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

    // 分段导出: 每批 1000 行，避免全量拷贝
    const int batchSize = 1000;
    int totalLines = m_terminalModel->lineCount();

    if (!m_dataExporter->exportBegin(filePath, format)) {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file"));
        return;
    }

    bool success = true;
    for (int offset = 0; offset < totalLines; offset += batchSize) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = m_terminalModel->lines(offset, count);
        if (batch.isEmpty() || !m_dataExporter->exportBatch(batch)) {
            success = false;
            break;
        }
    }

    if (!m_dataExporter->exportFinish()) {
        success = false;
    }

    if (success) {
        statusBar()->showMessage(tr("Exported to %1").arg(filePath), 3000);
    } else {
        QMessageBox::warning(this, tr("Export Failed"), tr("Cannot write to file"));
    }
}
```

Step 3: 保留旧接口兼容性

`exportToFile(filePath, format, lines)` 保持不变，供第三方(如未来脚本引擎)使用。内部实现改为调用 exportBegin/exportBatch/exportFinish。

#### 批次大小选择依据

- 每批 1000 行 x 约 60 字节/行 = 约 60 KB 内存
- mutex 持有时间: 拷贝 1000 行约 0.05ms，远低于全量拷贝 50000 行的 2-5ms
- 文件 I/O: 每批一次 write 调用，操作系统缓冲区吸收，无性能损失

#### 影响范围

| 文件 | 变更内容 |
|------|---------|
| `src/utils/DataExporter.h` | 新增 exportBegin/exportBatch/exportFinish 方法；新增文件句柄和状态成员 |
| `src/utils/DataExporter.cpp` | 实现流式导出；原 exportToFile 改为委托调用流式接口 |
| `src/core/MainWindow.cpp` | onExportData 改为分段调用 |

#### 验收标准

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| R4-AC1 | 导出功能不再调用 `lines()` 全量接口 | 代码审查: grep 搜索 `lines()` 在 onExportData 中不存在，仅有 `lines(offset, count)` |
| R4-AC2 | 50000 行数据导出时 mutex 单次持有时间不超过 1ms | 添加 DWT 计时代码测量 lines(offset, 1000) 调用耗时，打印到 qDebug |
| R4-AC3 | 导出结果与全量导出完全一致 | 测试脚本: 生成 10000 行数据，分别用新旧接口导出，diff 比较输出文件 |
| R4-AC4 | TXT/CSV/BIN 三种格式均正常导出 | 手动测试: 分别导出 .txt/.csv/.bin 文件，验证内容正确 |
| R4-AC5 | 空数据导出时正确提示"No data to export" | 手动测试: 清空终端后点击导出按钮 |
| R4-AC6 | 旧接口 exportToFile 仍可正常调用 | 单元测试: 调用 exportToFile 验证输出正确 |

---

## 依赖的公共组件

| 组件 | 文件 | 复用方式 |
|------|------|---------|
| SendHistory | `serial/SendHistory.h/cpp` | R2 中 recentText() 接口不变，仅改变调用方式 |
| TerminalModel | `terminal/TerminalModel.h/cpp` | R4 中使用已有的 lines(start, count) 分段接口 |
| DataExporter | `utils/DataExporter.h/cpp` | R4 扩展接口，保持向后兼容 |
| HexConverter | `utils/HexConverter.h` | R1 中 onSendData 内部继续使用，不涉及变更 |

## 设计模式

本 PRD 不引入新的设计模式。R1 使用 **提取方法 (Extract Method)** 重构手法消除重复代码。R4 使用 **流式接口 (Stream/Chunked Processing)** 模式降低内存峰值。

## 影响范围汇总

| 文件 | R1 | R2 | R3 | R4 |
|------|----|----|----|-----|
| `src/core/MainWindow.h` | 新增 sendAndRecord 声明；移除 QStackedWidget include | 新增 m_completerModel 成员 | m_rightPanel 类型改为 QWidget* | - |
| `src/core/MainWindow.cpp` | 重构三处发送逻辑 | setupUI 和 connectSignals 中 completer 初始化/更新改为复用 | setupUI 中移除 StackedWidget 语义 | onExportData 改为分段导出 |
| `src/utils/DataExporter.h` | - | - | - | 新增流式导出接口和成员 |
| `src/utils/DataExporter.cpp` | - | - | - | 实现流式导出方法 |

预计总变更量: 约 200 行新增/修改，约 80 行删除。

## 验收标准总表

| 需求 | 编号 | 验收条件 | 度量方法 | 通过标准 |
|------|------|---------|---------|---------|
| R1 | R1-AC1 | 发送逻辑无重复 | grep appendSent 在 MainWindow.cpp 中仅出现 1 次 | 1 次 |
| R1 | R1-AC2 | 手动发送功能不变 | 手动测试文本/HEX/错误三种场景 | 全部通过 |
| R1 | R1-AC3 | 快捷指令功能不变 | 手动测试 AT/Reset/Status | 全部通过 |
| R1 | R1-AC4 | 定时发送功能不变 | 手动测试 1000ms 间隔定时发送 | 数据正确发送 |
| R2 | R2-AC1 | 不创建新 QStringListModel | grep `new QStringListModel` 仅 1 次(初始化) | 1 次 |
| R2 | R2-AC2 | 自动补全正常 | 输入 "A" 弹出包含 "AT" 的补全列表 | 列表正确 |
| R2 | R2-AC3 | Model 实例数恒为 1 | 打印指针地址验证 | 地址不变 |
| R2 | R2-AC4 | 补全列表随历史更新 | 发送新命令后补全列表包含该命令 | 包含 |
| R3 | R3-AC1 | 不存在 QStackedWidget 引用 | grep `QStackedWidget` 在 src/ 返回 0 | 0 结果 |
| R3 | R3-AC2 | 编译零错误 | cmake --build build | 成功 |
| R3 | R3-AC3 | 7 个面板切换正常 | 逐一点击导航树节点 | 全部切换 |
| R3 | R3-AC4 | 切换动画正常 | 观察淡入淡出 | 有动画过渡 |
| R3 | R3-AC5 | resize 布局正确 | 拖拽窗口边缘 | 无错乱 |
| R4 | R4-AC1 | 不调用 lines() 全量接口 | 代码审查 onExportData | 无 lines() 调用 |
| R4 | R4-AC2 | mutex 持有时间 < 1ms | DWT 计时打印 | < 1ms |
| R4 | R4-AC3 | 导出内容一致 | 新旧接口输出 diff | 无差异 |
| R4 | R4-AC4 | 三种格式正常 | 分别导出 txt/csv/bin | 内容正确 |
| R4 | R4-AC5 | 空数据提示正确 | 清空终端后导出 | 提示出现 |
| R4 | R4-AC6 | 旧接口仍可用 | 调用 exportToFile | 输出正确 |

## 实施优先级

1. **R4 (P0)**: 先修复导出性能问题，因为它影响用户在大量数据场景下的体验
2. **R1 (P1)**: 消除发送逻辑重复，这是架构审查标记的首要技术债务
3. **R2 (P1)**: 修复 Completer 内存泄漏，虽然是软泄漏但随使用时间累积
4. **R3 (P2)**: 最后处理 QStackedWidget 误用，纯粹是代码语义清理
