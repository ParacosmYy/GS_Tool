# PRD-036: P1 Bug修复、代码质量合规与功能增强

## 背景

commit #35 完成了全模块 Bug 审查冲刺、QSS 一致性收尾、ChartWidget 主题集成、OTA 增强和 SessionManager 集成。当前评分 37 分。

经对 commit #35 后代码库的全面审查，发现以下三类问题需要在本迭代中解决:

**问题一: P1 Bug 仍未修复（4项）**

| Bug | 当前状态 | 风险 |
|-----|---------|------|
| OTA setConnection() 活跃传输期间调用 | OtaManager::setConnection() 无状态检查，直接覆盖 m_conn 和三个协议实例的连接。如果在传输进行中调用，底层协议正在通过旧连接读写数据，连接被替换后数据流断裂，可能导致数据损坏或传输挂起 | P1 -- 传输中切换连接导致数据损坏 |
| TimedSender 线程安全 | m_queue 和 m_queueIndex 无任何线程保护。虽然 QTimer 回调在主线程执行，但 setData()/setDataQueue() 可从任意线程调用（如 RecordingController 回放线程），与 onTimeout() 的 m_queue 读写存在数据竞争 | P1 -- 多线程环境下定时发送数据竞争 |
| DataExporter 写入错误检查不充分 | 10 个导出方法中，QTextStream 的 write 操作无错误检查。QTextStream 本身不报告磁盘满/IO 错误，需要检查 QFile::error() 或 QIODevice::errorString()。特别是 exportStreamedBin 中 file.write() 的返回值虽然会循环，但中途写入失败时仍继续写入后续批次，产生损坏文件 | P1 -- 磁盘满或IO错误时导出文件损坏 |
| TerminalModel::lineAt() 使用 Q_ASSERT | 行 77 的 `Q_ASSERT(index >= 0 && index < m_count)` 在 Release 构建（QT_NO_DEBUG 宏定义）中被完全移除。传入越界索引时，Release 模式下直接访问越界内存，可能导致崩溃或未定义行为 | P1 -- Release构建越界访问崩溃 |

**问题二: 文件体积违规（2项）**

| 文件 | 当前行数 | 上限 | 超出 |
|------|---------|------|------|
| MainWindow.h | 202 行 | 200 行 | +2 行 |
| TerminalWidget.h | 221 行 | 200 行 | +21 行 |
| DataExporter.cpp | 535 行 | 500 行 | +35 行 |

经逐行审查:
- MainWindow.h 超出 2 行，可通过精简注释或合并相邻成员声明修复
- TerminalWidget.h 超出 21 行，根因是右键菜单成员（m_contextMenu/m_copyAction/m_pasteAction/m_clearAction/m_selectAllAction/m_searchAction 共 6 个成员，行 208-214）导致膨胀。需要将右键菜单逻辑提取到独立的 TerminalContextMenu 类中
- DataExporter.cpp 超出 35 行，根因是流式导出的 5 个方法（exportStreamedPlain/HexDump/Csv/Timestamped/Bin）存在大量重复的"打开文件 + 分批循环 + 格式化"模板代码。可通过提取公共流式循环模板方法消除重复

**问题三: 功能增强（3项）**

| 特性 | 用户价值 | 复杂度 |
|------|---------|-------|
| 连接状态指示器视觉增强 | 当前状态栏只有文字标签（m_connStatusLbl），缺少 CLAUDE.md 6.6 节要求的彩色圆点+呼吸动画状态指示。NavigationController 已实现呼吸动画（startBreathingAnimation/stopBreathingAnimation），但未应用到工具栏按钮或状态指示器上 | 低 |
| 终端右键菜单 | TerminalWidget 已有右键菜单实现（m_contextMenu，含复制/粘贴/清屏/全选/搜索 5 个菜单项），当前功能基本完整。增强项: 搜索菜单项应传递搜索文本为当前选中文本、粘贴在 HEX 模式下应解析 HEX 输入、菜单快捷键提示（Ctrl+C/V/F） | 低 |
| 数据统计面板实时速率图表 | 当前 DataStatistics 仅用 QLabel 显示速率数值，无历史趋势可视化。新增小型内嵌折线图显示最近 60 秒的 RX/TX 速率趋势，帮助用户直观判断数据流稳定性和突发情况 | 中 |

---

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | OTA setConnection() 传输状态守卫 -- 活跃传输期间拒绝连接切换 | P1 | ota/OtaManager.h/cpp |
| R2 | TimedSender 线程安全 -- 为 m_queue/m_queueIndex 添加互斥保护 | P1 | serial/TimedSender.h/cpp |
| R3 | DataExporter 写入错误检查 -- 10 个导出方法增加 IO 错误检测和提前退出 | P1 | utils/DataExporter.cpp |
| R4 | TerminalModel::lineAt() Q_ASSERT 替换为 Release 安全的边界检查 | P1 | terminal/TerminalModel.cpp |
| R5 | MainWindow.h 缩减至 200 行以内 | P1 | core/MainWindow.h |
| R6 | TerminalWidget.h 右键菜单成员提取到 TerminalContextMenu | P1 | terminal/TerminalWidget.h/cpp, 新增 terminal/TerminalContextMenu.h/cpp |
| R7 | DataExporter.cpp 提取流式模板方法降至 500 行以内 | P1 | utils/DataExporter.h/cpp |
| R8 | 连接状态指示器视觉增强 -- 工具栏添加彩色圆点+连接/断开动画 | P2 | core/ConnectionController.h/cpp, core/NavigationController.h/cpp |
| R9 | 终端右键菜单增强 -- HEX模式粘贴解析、搜索传递选中文本、快捷键提示 | P2 | terminal/TerminalWidget.cpp, terminal/TerminalContextMenu.h/cpp |
| R10 | 数据统计面板实时速率图表 -- 最近60秒RX/TX速率趋势折线 | P2 | serial/DataStatistics.h/cpp, 新增 serial/RateMiniChart.h/cpp |

---

## 需求详细说明

---

### R1: OTA setConnection() 传输状态守卫 (P1)

#### 问题分析

`OtaManager::setConnection()` (OtaManager.cpp 行 114-120) 当前实现:

```cpp
void OtaManager::setConnection(IConnection* conn)
{
    m_conn = conn;
    m_xmodem->setConnection(conn);
    m_ymodem->setConnection(conn);
    m_zmodem->setConnection(conn);
}
```

无任何状态检查。当 OTA 传输正在进行时（m_otaState == Transferring），如果外部调用 setConnection():
1. 底层协议实例的连接被替换为新的 conn
2. 旧连接上的待读取数据（如 ACK/NAK 响应）丢失
3. 新连接可能处于不同状态，协议状态机收到意外数据
4. 最终导致传输挂起或数据损坏

调用链: MainWindow 在 handleConnectionState(ConnectionState::Connected) 中调用 m_otaManager->setConnection(conn)。如果用户在 OTA 传输中切换到另一个串口连接，就会触发此问题。

#### 修改方案

在 setConnection() 中添加传输状态守卫:

```cpp
void OtaManager::setConnection(IConnection* conn)
{
    // 传输进行中拒绝连接切换，避免数据损坏
    if (isTransferring()) {
        qWarning() << "OtaManager: Cannot change connection during active transfer";
        return;
    }
    m_conn = conn;
    m_xmodem->setConnection(conn);
    m_ymodem->setConnection(conn);
    m_zmodem->setConnection(conn);
}
```

#### 验收标准

- [ ] setConnection() 在 isTransferring() == true 时不修改 m_conn 和协议实例连接
- [ ] 正常流程（非传输状态下设置连接）不受影响
- [ ] 传输中被拒绝时输出 qWarning 日志

---

### R2: TimedSender 线程安全 (P1)

#### 问题分析

`TimedSender` 类（TimedSender.h/cpp）的成员变量 `m_queue`（QList<QByteArray>）和 `m_queueIndex`（int）无任何线程保护。

- `onTimeout()` 由 QTimer 在主线程事件循环中触发，读取 m_queue 和 m_queueIndex
- `setData()` 和 `setDataQueue()` 可从任意线程调用（例如 RecordingController 的回放线程或从其他信号槽跨线程调用）
- `start()` 修改 m_queueIndex

当前调用场景分析:
- SendController 在主线程调用 setData/setDataQueue/start/stop -- 安全
- 但如果 TimedSender 被用于其他场景（如录制回放触发定时发送），则存在数据竞争

#### 修改方案

为 TimedSender 添加 QMutex 保护共享状态:

```cpp
// TimedSender.h 新增:
#include <QMutex>
// ...
private:
    mutable QMutex m_mutex;  ///< 保护 m_queue 和 m_queueIndex 的线程安全锁
```

```cpp
// setData/setDataQueue/start/stop/onTimeout 中添加 QMutexLocker
void TimedSender::setData(const QByteArray& data)
{
    QMutexLocker locker(&m_mutex);
    m_queue = {data};
    m_queueIndex = 0;
}

void TimedSender::onTimeout()
{
    QMutexLocker locker(&m_mutex);
    if (m_queue.isEmpty()) {
        locker.unlock();
        stop();
        return;
    }
    QByteArray dataToSend = m_queue[m_queueIndex];
    m_queueIndex = (m_queueIndex + 1) % m_queue.size();
    locker.unlock();
    emit sendData(dataToSend);
}
```

注意: emit 信号在锁释放后执行，避免持锁发信号导致潜在死锁。

#### 验收标准

- [ ] setData/setDataQueue/start/stop/onTimeout 中所有对 m_queue 和 m_queueIndex 的访问受 QMutex 保护
- [ ] emit sendData() 在 QMutexLocker 释放后执行
- [ ] 功能行为不变 -- 单线程场景下行为与修复前完全一致

---

### R3: DataExporter 写入错误检查 (P1)

#### 问题分析

DataExporter.cpp 中 10 个导出方法（5 个全量 + 5 个流式）存在以下问题:

1. **QTextStream 写入无错误检查**: exportPlain/exportHexDump/exportCsv/exportTimestamped 和对应的流式方法使用 `out << ...` 写入数据，QTextStream 的 operator<< 不报告错误。如果磁盘满或 IO 错误，写入静默失败，方法最终返回 true，用户以为导出成功。

2. **QFile::write() 返回值未检查**: exportBin 和 exportStreamedBin 中 `file.write(line.data)` 的返回值被忽略。如果写入字节数小于预期，文件内容不完整。

3. **流式导出中途失败不中断**: exportStreamedBin 的 while 循环中，如果某批次的 file.write() 失败，循环继续处理后续批次，产生的文件是损坏的。

#### 修改方案

**方案一: 全量导出方法在写入循环后检查文件状态**

```cpp
bool DataExporter::exportPlain(const QString& path,
                                const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    for (const TerminalLine& line : lines) {
        // ... 格式化和写入 ...
    }

    out.flush();  // 确保缓冲区数据写入磁盘
    if (file.error() != QFile::NoError) {
        file.close();
        QFile::remove(path);  // 删除损坏的导出文件
        return false;
    }
    file.close();
    return true;
}
```

**方案二: 流式导出在每批写入后检查**

```cpp
bool DataExporter::exportStreamedBin(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;

    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;

        for (const TerminalLine& line : batch) {
            qint64 written = file.write(line.data);
            if (written != line.data.size()) {
                file.close();
                QFile::remove(path);  // 删除不完整文件
                return false;
            }
        }
        offset += batch.size();
    }

    file.close();
    return true;
}
```

**统一错误处理**: 所有导出方法在失败时删除不完整的输出文件，返回 false。

#### 验收标准

- [ ] 5 个全量导出方法在写入完成后检查 QFile::error()，出错时删除文件并返回 false
- [ ] 5 个流式导出方法在每批写入后检查错误，出错时立即中断循环、删除文件并返回 false
- [ ] exportBin/exportStreamedBin 检查 file.write() 返回值是否等于预期字节数
- [ ] 正常导出流程无性能影响（仅增加一次 error() 检查）

---

### R4: TerminalModel::lineAt() Q_ASSERT 替换 (P1)

#### 问题分析

`TerminalModel::lineAt()` (TerminalModel.cpp 行 74-79):

```cpp
TerminalLine TerminalModel::lineAt(int index) const
{
    QMutexLocker locker(&m_mutex);
    Q_ASSERT(index >= 0 && index < m_count);
    return m_buffer[physicalIndex(index)];
}
```

`Q_ASSERT` 在 Release 构建中被 `#ifdef QT_NO_DEBUG` 移除。传入越界索引时:
- Debug 构建: Q_ASSERT 触发断言失败，程序中止（开发阶段可发现）
- Release 构建: Q_ASSERT 被移除，`physicalIndex(index)` 可能返回越界的物理索引，访问 `m_buffer` 越界元素，导致崩溃或未定义行为

调用方 TerminalWidget 在 paintEvent 和搜索刷新中高频调用 lineAt()，虽然 index 来自 m_cachedLineCount 和 m_model->lineCount() 的比较，理论上不应越界，但作为防御性编程原则，公共 API 必须处理边界情况。

#### 修改方案

将 Q_ASSERT 替换为 Release 安全的边界检查:

```cpp
TerminalLine TerminalModel::lineAt(int index) const
{
    QMutexLocker locker(&m_mutex);
    // 边界检查: Debug 模式保留断言快速定位问题，Release 模式返回空对象防止崩溃
    if (index < 0 || index >= m_count) {
        Q_ASSERT_X(false, "TerminalModel::lineAt", "index out of range");
        return TerminalLine{};  // 返回默认空行，避免越界访问
    }
    return m_buffer[physicalIndex(index)];
}
```

使用 `Q_ASSERT_X` 在 Debug 模式下仍然提供断言信息（含函数名和错误描述），Release 模式下 `if` 检查确保安全返回。

#### 验收标准

- [ ] Release 模式下传入越界索引不崩溃，返回空 TerminalLine
- [ ] Debug 模式下越界索引触发 Q_ASSERT_X 断言（保留开发期快速定位能力）
- [ ] 所有现有 lineAt() 调用方（TerminalWidget paintEvent/搜索、DataExporter 导出）不受影响

---

### R5: MainWindow.h 缩减至 200 行以内 (P1)

#### 问题分析

MainWindow.h 当前 202 行，超出 CLAUDE.md 4.6 节规定的 .h 文件 200 行上限。超出仅 2 行。

逐行审查发现可压缩的位置:

1. 行 200（`#endif // MAINWINDOW_H`）之前有一个空行（行 199-200 之间），但这个空行与 `};` 分隔是合理的格式
2. 行 105-106 之间有 `// ==================== 根本组件 ====================` 注释行
3. 多个注释分组标题各占一行

#### 修改方案

精简 MainWindow.h 中的分组注释，合并相邻的短注释:

```cpp
// 修改前（2 行）:
    // ==================== 背景组件 ====================
    // ==================== 会话管理 ====================

// 修改后（合并到 1 行）:
    // ---- 背景组件 & 会话管理 ----
```

或删除某个最不重要的分隔注释行。只需压缩 3 行注释即可将总行数降至 199 行。

#### 验收标准

- [ ] MainWindow.h 行数 <= 200
- [ ] 类声明结构不变，所有成员和方法声明完整保留
- [ ] 注释仍然清晰可读

---

### R6: TerminalWidget.h 右键菜单成员提取到 TerminalContextMenu (P1)

#### 问题分析

TerminalWidget.h 当前 221 行，超出 200 行上限 21 行。根因是行 208-214 新增了右键菜单相关的 7 个成员声明:

```cpp
    // ---- 右键菜单 ----
    QMenu* m_contextMenu;            ///< 终端右键菜单
    QAction* m_copyAction;           ///< 复制菜单项
    QAction* m_pasteAction;          ///< 粘贴菜单项
    QAction* m_clearAction;          ///< 清屏菜单项
    QAction* m_selectAllAction;      ///< 全选菜单项
    QAction* m_searchAction;         ///< 搜索菜单项
```

加上对应的注释和分组标题共约 10 行。同时 TerminalWidget.cpp 中的右键菜单构建逻辑（行 507-567 约 60 行）也可以一并提取。

#### 修改方案

新增 `TerminalContextMenu` 类，封装终端右键菜单的创建、状态更新和动作执行:

```cpp
// terminal/TerminalContextMenu.h
/**
 * @brief 终端右键菜单管理器 - 封装右键菜单的创建和动作处理
 *
 * 协作: TerminalWidget(持有) / QClipboard(粘贴)
 */
class TerminalContextMenu : public QObject {
    Q_OBJECT
public:
    explicit TerminalContextMenu(QWidget* parent);

    /** @brief 在指定全局坐标弹出菜单 */
    void popup(const QPoint& globalPos, bool hasSelection);

signals:
    void copyRequested();
    void pasteRequested();
    void clearRequested();
    void selectAllRequested();
    void searchRequested();

private:
    QMenu* m_menu;
    QAction* m_copyAction;
    QAction* m_pasteAction;
    QAction* m_clearAction;
    QAction* m_selectAllAction;
    QAction* m_searchAction;
};
```

TerminalWidget 中替换为:
```cpp
    // 修改前: 7 个成员 + 注释 ≈ 10 行
    // 修改后: 1 个成员
    TerminalContextMenu* m_contextMenu;  ///< 右键菜单管理器
```

减少约 6 行头文件声明 + TerminalWidget.cpp 减少约 40 行构建/处理代码。

#### 验收标准

- [ ] TerminalWidget.h 行数 <= 200
- [ ] TerminalContextMenu.h/cpp 遵循 .h<=200, .cpp<=500 限制
- [ ] 右键菜单功能不变 -- 复制/粘贴/清屏/全选/搜索 5 个菜单项全部保留
- [ ] contextMenuEvent 逻辑委托给 TerminalContextMenu::popup()

---

### R7: DataExporter.cpp 提取流式模板方法降至 500 行以内 (P1)

#### 问题分析

DataExporter.cpp 当前 535 行，超出 500 行上限 35 行。

5 个流式导出方法存在明显的模板重复模式:

```cpp
bool DataExporter::exportStreamedXxx(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            // <-- 只有这里的格式化逻辑不同 -->
        }
        offset += batch.size();
    }
    file.close();
    return true;
}
```

"打开文件 -> 循环拉取 -> 格式化写入 -> 关闭文件" 的骨架在 5 个方法中完全相同，仅格式化部分不同。

#### 修改方案

提取公共流式骨架方法:

```cpp
/**
 * @brief 流式导出骨架方法 -- 消除 5 个流式方法的"打开-循环-关闭"重复代码
 * @param formatLine 单行格式化回调，返回 true 继续，false 中断
 */
bool DataExporter::exportStreamedImpl(
    const QString& path,
    LineProvider provider,
    int totalLines,
    int batchSize,
    bool textMode,
    const std::function<bool(QTextStream&, const TerminalLine&)>& formatLine);
```

5 个流式方法各自缩减为 10-15 行的 formatLine 回调 + 一行 exportStreamedImpl 调用。

预估减少约 120 行重复代码，DataExporter.cpp 降至约 420 行。

同时 DataExporter.h 中新增的 exportStreamedImpl 声明约增加 5 行，但 117 行 + 5 行 = 122 行仍在 200 行以内。

#### 验收标准

- [ ] DataExporter.cpp 行数 <= 500
- [ ] DataExporter.h 行数 <= 200
- [ ] 5 种格式的流式导出功能不变（Plain/HexDump/CSV/Timestamped/Bin 输出内容完全一致）
- [ ] HexDump 特殊逻辑（残余缓冲区/全局地址偏移）通过扩展参数或独立处理保留

---

### R8: 连接状态指示器视觉增强 (P2)

#### 问题分析

当前连接状态指示:
- **状态栏**: QLabel 文字（m_connStatusLbl）显示"未连接"/"已连接: COMx"/"连接中..."
- **呼吸动画**: NavigationController 已实现 startBreathingAnimation/stopBreathingAnimation（对传入的 QLabel 施加 QGraphicsOpacityEffect + QPropertyAnimation，1500ms 循环，opacity 0.3~1.0）
- **缺少**: CLAUDE.md 6.6 状态栏规范要求的"左侧: 连接状态指示器（带呼吸动画的彩色圆点）"

当前呼吸动画应用在 QLabel 整体上（文字+背景一起呼吸），而非独立的彩色圆点。

#### 修改方案

**新增 ConnectionStatusIndicator 控件**（嵌入状态栏左侧）:

```
[圆点 10x10px] [文字 "已连接: COM3"]
 ^-- 彩色       ^-- m_connStatusLbl
     绿色=已连接
     黄色=连接中（呼吸动画）
     红色=断开/错误
     灰色=未连接
```

实现方式:
1. 在 setupStatusBar() 中创建 QLabel 作为圆点指示器，固定大小 10x10px
2. 使用 QWidget::setStyleSheet 设置背景色圆角为圆形（border-radius: 5px）
3. 连接状态变化时切换圆点颜色
4. 连接中状态调用 NavigationController::startBreathingAnimation(m_statusDot)
5. 连接成功/断开时调用 stopBreathingAnimation(m_statusDot)

预估工作量: 30 行代码（在 MainWindow::setupStatusBar 中创建圆点 + handleConnectionState 中切换颜色/动画）。

#### 验收标准

- [ ] 状态栏左侧显示 10x10px 彩色圆点
- [ ] 未连接: 灰色 (#6c7086)
- [ ] 连接中: 黄色 (#f9e2af) + 呼吸脉冲动画 (opacity 0.3~1.0, 1500ms)
- [ ] 已连接: 绿色 (#a6e3a1)
- [ ] 连接错误: 红色 (#f38ba8)
- [ ] 颜色使用 ThemeManager 语义色板，不硬编码

---

### R9: 终端右键菜单增强 (P2)

#### 问题分析

TerminalWidget 当前右键菜单（行 507-567）已实现 5 个基础菜单项: 复制/粘贴/清屏/全选/搜索。

增强点:

1. **HEX 模式下粘贴解析**: 当 DisplayMode == Hex 时，粘贴操作应自动识别并转换 HEX 文本（如 "AA 55 01 02"）为二进制数据发送，而非直接粘贴 HEX 文本到输入框
2. **搜索传递选中文本**: 当用户右键时已有选中文本，搜索菜单项应自动以选中文本为搜索关键词激活搜索栏
3. **快捷键提示**: 菜单项文字后显示快捷键（Ctrl+C / Ctrl+V / Ctrl+F），提升可发现性

#### 修改方案

在 TerminalContextMenu::popup() 中根据上下文动态调整:

```cpp
void TerminalContextMenu::popup(const QPoint& globalPos, bool hasSelection,
                                 const QString& selectedText)
{
    m_copyAction->setEnabled(hasSelection);
    m_copyAction->setText(tr("复制\tCtrl+C"));
    m_pasteAction->setText(tr("粘贴\tCtrl+V"));
    m_searchAction->setText(selectedText.isEmpty()
        ? tr("搜索\tCtrl+F")
        : tr("搜索 \"%1\"\tCtrl+F").arg(selectedText.left(20)));
    m_menu->popup(globalPos);
}
```

搜索菜单触发时: 如果有 selectedText，将其作为初始搜索关键词传递给 TerminalSearchBar。

HEX 粘贴增强: 在 pasteRequested 信号处理中检查 DisplayMode，Hex 模式下使用 HexConverter::fromHexString() 解析剪贴板文本。

#### 验收标准

- [ ] 右键菜单显示快捷键提示（Ctrl+C/V/F）
- [ ] 选中文字后右键搜索，搜索栏自动以选中文本为关键词
- [ ] 选中文字过长时截断显示（最多 20 字符）
- [ ] HEX 模式下粘贴自动解析 HEX 文本为二进制数据
- [ ] 非法 HEX 文本粘贴时显示提示信息

---

### R10: 数据统计面板实时速率图表 (P2)

#### 问题分析

当前 DataStatistics 面板（DataStatistics.h/cpp）使用 QLabel 显示:
- RX 累计字节数 / 速率
- TX 累计字节数 / 速率
- 连接持续时间

缺少历史趋势可视化。用户无法直观判断:
- 数据流是否稳定（匀速 vs 突发）
- 何时出现了数据突发或停滞
- RX 和 TX 的速率对比

#### 修改方案

新增 `RateMiniChart` 控件，嵌入 DataStatistics 面板中:

```
┌─────────────────────────────┐
│  RX:  12.5 KB    Rate: 1.2 KB/s  │
│  ┌─────────────────────────────┐ │
│  │  ╱\    ╱\                    │ │  ← RX 速率折线（绿色）
│  │ /  \  /  \   ╱\              │ │
│  │/    \/    \_/  \________     │ │
│  │  0    10   20   30   60s     │ │
│  └─────────────────────────────┘ │
│  TX:  3.2 KB    Rate: 0.3 KB/s  │
│  ┌─────────────────────────────┐ │
│  │  ────____──────____──        │ │  ← TX 速率折线（蓝色）
│  └─────────────────────────────┘ │
│  Time: 00:05:23                  │
└─────────────────────────────┘
```

实现细节:
- 使用 QWidget + QPainter 自绘（与 TerminalWidget 同风格，不依赖 QChart 减轻依赖）
- 固定显示最近 60 秒数据，1 秒 1 个采样点（60 个数据点）
- 数据存储: `QVector<double> m_rxRateHistory, m_txRateHistory`（固定 60 容量）
- DataStatistics::onRefreshTimer() 中将当前速率追加到历史数组
- RX 折线颜色: `ThemeManager::SemanticColor::Success` (#a6e3a1)
- TX 折线颜色: `ThemeManager::SemanticColor::Accent` (#89b4fa)
- 图表高度: 60px（紧凑，不占用过多面板空间）
- 背景色使用 `ThemeManager::SemanticColor::BgSecondary`
- 网格线使用 `ThemeManager::SemanticColor::Border`

#### 验收标准

- [ ] DataStatistics 面板中显示 RX 和 TX 各一个速率趋势折线图
- [ ] 折线图显示最近 60 秒数据，每秒更新一次
- [ ] RX 使用绿色折线，TX 使用蓝色折线，颜色从 ThemeManager 获取
- [ ] 图表高度不超过 60px，宽度随面板拉伸
- [ ] 窗口 resize 时图表正确重绘
- [ ] DataStatistics.h 行数 <= 200, DataStatistics.cpp 行数 <= 500

---

## 依赖的公共组件

| 组件 | 文件 | 用途 |
|------|------|------|
| ThemeManager | core/ThemeManager.h/cpp | R8/R9/R10 获取语义色板颜色 |
| NavigationController | core/NavigationController.h/cpp | R8 复用呼吸动画方法 |
| HexConverter | utils/HexConverter.h | R9 HEX 文本解析 |
| TerminalSelectionManager | terminal/TerminalSelectionManager.h | R9 获取选中文本 |
| TerminalSearchManager | terminal/TerminalSearchManager.h | R9 传递搜索关键词 |

---

## 设计模式

| 需求 | 设计模式 | 说明 |
|------|---------|------|
| R6 | 组合模式 (Composition) | TerminalWidget 持有 TerminalContextMenu 实例，委托右键菜单逻辑 |
| R7 | 模板方法模式 (Template Method) | exportStreamedImpl 定义流式导出骨架，formatLine 回调提供差异化逻辑 |
| R8 | 观察者模式 (Observer) | 连接状态变化信号驱动指示器颜色切换 |
| R10 | 观察者模式 (Observer) | 定时器触发速率采样和图表重绘 |

---

## 影响范围

| 文件 | 变更类型 | 影响描述 |
|------|---------|---------|
| ota/OtaManager.h | 修改 | setConnection() 添加传输状态守卫注释 |
| ota/OtaManager.cpp | 修改 | setConnection() 添加 isTransferring() 检查 |
| serial/TimedSender.h | 修改 | 新增 QMutex 成员 |
| serial/TimedSender.cpp | 修改 | 所有共享状态访问添加 QMutexLocker |
| utils/DataExporter.h | 修改 | 新增 exportStreamedImpl 私有方法声明 |
| utils/DataExporter.cpp | 修改 | 10 个导出方法添加错误检查，5 个流式方法重构为调用模板方法 |
| terminal/TerminalModel.cpp | 修改 | lineAt() Q_ASSERT 替换为 Release 安全检查 |
| core/MainWindow.h | 修改 | 精简注释压缩至 200 行以内 |
| terminal/TerminalWidget.h | 修改 | 右键菜单成员替换为 TerminalContextMenu* |
| terminal/TerminalWidget.cpp | 修改 | 右键菜单逻辑委托给 TerminalContextMenu |
| terminal/TerminalContextMenu.h | **新增** | 右键菜单管理器类声明 |
| terminal/TerminalContextMenu.cpp | **新增** | 右键菜单管理器实现 |
| core/MainWindow.cpp | 修改 | setupStatusBar() 创建状态圆点指示器 |
| core/NavigationController.cpp | 修改 | handleConnectionState 中切换圆点颜色和动画 |
| serial/DataStatistics.h | 修改 | 新增 RateMiniChart 成员 |
| serial/DataStatistics.cpp | 修改 | 新增速率历史数组和图表布局 |
| serial/RateMiniChart.h | **新增** | 速率趋势迷你图类声明 |
| serial/RateMiniChart.cpp | **新增** | 速率趋势迷你图实现 |

---

## 验收标准

### 功能验收

- [ ] R1: OTA 传输期间调用 setConnection() 被拒绝，日志输出警告
- [ ] R2: TimedSender 多线程并发调用 setData + onTimeout 无数据竞争（ThreadSanitizer 验证）
- [ ] R3: 磁盘满时导出返回 false，不产生损坏文件
- [ ] R4: Release 模式下 TerminalModel::lineAt(-1) 返回空 TerminalLine 而不崩溃
- [ ] R8: 连接状态指示器正确显示四种状态颜色和呼吸动画
- [ ] R9: 右键菜单快捷键提示可见，选中文字后搜索自动填充
- [ ] R10: 速率趋势图实时更新，60 秒历史数据可观察

### 代码质量验收

- [ ] MainWindow.h <= 200 行
- [ ] TerminalWidget.h <= 200 行
- [ ] DataExporter.cpp <= 500 行
- [ ] 所有 .h 文件 <= 200 行, 所有 .cpp 文件 <= 500 行
- [ ] 所有新增类遵循 .h/.cpp 配对，放在对应子目录
- [ ] 所有新增类有完整的 Doxygen 注释
- [ ] 零编译错误
