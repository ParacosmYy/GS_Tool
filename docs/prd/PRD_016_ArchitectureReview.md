# PRD-016: 架构审查 (Commit #20 里程碑)

## 背景

根据 CLAUDE.md 3.3 节规定，每完成 5 次 commit 必须执行一次全面的架构管理审查。当前 commit #17 完成后分数为 19，下一个 commit #18 将使分数达到 20，触发第四次架构审查（前三次分别在 commit #5、#10、#15 执行）。

本次审查不仅覆盖常规的分层合规性、设计模式、耦合度检查，还集中解决代码审查中发现的 5 个已知技术债务，并规划 3 项架构级优化功能。这是在项目进入 RTT/J-Link 等复杂功能开发之前的关键质量关卡。

**审查基准**: commit #17，评分 19/1000，已完成 17 次迭代。

## 一、审查范围

### 1.1 分层架构合规性全面检查

按 CLAUDE.md 4.2 节定义的四层架构，逐层审查所有模块的依赖方向是否符合单向规则:

```
表现层 (Presentation)  -->  业务层 (Business)  -->  数据层 (Data)  -->  基础设施层 (Infra)
```

#### 各层模块清单与归属

| 层级 | 模块 | 文件 | 审查重点 |
|------|------|------|---------|
| **表现层** | MainWindow | `core/MainWindow.h/cpp` | 不含业务逻辑，仅编排 UI 和信号转发 |
| **表现层** | SerialConfigPanel | `serial/SerialConfigPanel.h/cpp` | 仅展示配置表单，不直接操作连接 |
| **表现层** | TerminalWidget | `terminal/TerminalWidget.h/cpp` | 自绘制控件，依赖 TerminalModel 但不修改数据 |
| **表现层** | TerminalSearchBar | `terminal/TerminalSearchBar.h/cpp` | 纯 UI 控件 |
| **表现层** | QuickCommandBar | `serial/QuickCommandBar.h/cpp` | 纯 UI 控件 |
| **表现层** | DataStatistics | `serial/DataStatistics.h/cpp` | 纯展示，从 TerminalModel 读取数据 |
| **表现层** | ProtocolView | `protocol/ProtocolView.h/cpp` | 纯展示，接收帧解析结果 |
| **表现层** | FrameVisualEditor | `protocol/FrameVisualEditor.h/cpp` | UI 表单，输出 FrameDefinition |
| **表现层** | ChartWidget | `chart/ChartWidget.h/cpp` | 纯展示，依赖 ChartModel |
| **表现层** | OtaWidget | `ota/OtaWidget.h/cpp` | OTA 操作面板，委托 OtaManager 处理业务 |
| **业务层** | ConnectionManager | `core/ConnectionManager.h/cpp` | 连接生命周期编排 |
| **业务层** | OtaManager | `ota/OtaManager.h/cpp` | OTA 传输流程编排 |
| **业务层** | FrameParser | `protocol/FrameParser.h/cpp` | 帧解析状态机 |
| **数据层** | TerminalModel | `terminal/TerminalModel.h/cpp` | 终端数据缓冲区管理 |
| **数据层** | ChartModel | `chart/ChartModel.h/cpp` | 波形图数据模型 |
| **数据层** | ChannelConfig | `chart/ChannelConfig.h/cpp` | 通道配置数据结构 |
| **数据层** | OtaHistoryModel | `ota/OtaHistoryModel.h/cpp` | OTA 历史记录持久化 |
| **数据层** | IntelHexParser | `protocol/IntelHexParser.h/cpp` | HEX 文件解析算法 |
| **数据层** | FrameDefinition | `protocol/FrameDefinition.h` | 帧定义数据结构 |
| **数据层** | SendHistory | `serial/SendHistory.h/cpp` | 发送历史数据管理 |
| **基础设施层** | IConnection | `connection/IConnection.h` | 连接抽象接口 |
| **基础设施层** | SerialConnection | `connection/SerialConnection.h/cpp` | 串口硬件抽象 |
| **基础设施层** | TcpConnection | `connection/TcpConnection.h/cpp` | TCP 网络抽象 |
| **基础设施层** | UdpConnection | `connection/UdpConnection.h/cpp` | UDP 网络抽象 |
| **基础设施层** | ConnectionFactory | `core/ConnectionFactory.h/cpp` | 连接对象创建工厂 |
| **基础设施层** | SettingsManager | `utils/SettingsManager.h/cpp` | 配置持久化单例 |
| **基础设施层** | ThemeManager | `core/ThemeManager.h/cpp` | 主题管理单例 |
| **基础设施层** | RingBuffer | `utils/RingBuffer.h` | 通用环形缓冲区模板 |
| **基础设施层** | CRC | `utils/CRC.h` | 校验算法 |
| **基础设施层** | HexConverter | `utils/HexConverter.h` | HEX 编解码 |
| **基础设施层** | DataLogger | `utils/DataLogger.h/cpp` | 日志录制/回放 |
| **基础设施层** | DataExporter | `utils/DataExporter.h/cpp` | 数据导出 |
| **基础设施层** | BaseTransfer | `ota/protocols/BaseTransfer.h/cpp` | OTA 传输模板方法基类 |
| **基础设施层** | XModemTransfer | `ota/protocols/XModemTransfer.h/cpp` | XMODEM 协议实现 |
| **基础设施层** | YModemTransfer | `ota/protocols/YModemTransfer.h/cpp` | YMODEM 协议实现 |
| **基础设施层** | ZModemTransfer | `ota/protocols/ZModemTransfer.h/cpp` | ZMODEM 协议实现 |

#### 合规性检查项

| 检查编号 | 检查内容 | 判定标准 |
|---------|---------|---------|
| AR-01 | 基础设施层模块不应 `#include` 任何上层头文件 | `connection/`, `utils/`, `ota/protocols/` 下的文件不得引用 `core/MainWindow`, `serial/*`, `terminal/*`, `protocol/FrameVisualEditor`, `protocol/ProtocolView`, `ota/OtaWidget`, `chart/*` |
| AR-02 | 数据层模块不应 `#include` 表现层头文件 | `terminal/TerminalModel`, `protocol/FrameParser`, `serial/SendHistory` 等不得引用 `core/MainWindow`, `serial/SerialConfigPanel`, `ota/OtaWidget` |
| AR-03 | 表现层可以直接依赖数据层读取数据 | 允许: MainWindow 引用 TerminalModel、FrameParser |
| AR-04 | 表现层通过业务层编排业务操作 | 允许: MainWindow 引用 ConnectionManager、OtaManager |
| AR-05 | 无循环依赖 | A 引用 B，B 不得引用 A（通过前向声明或接口解耦） |
| AR-06 | 公共组件无重复实现 | CRC、HexConverter、RingBuffer、SettingsManager 全局唯一 |
| AR-07 | 设计模式合规 | 对照 CLAUDE.md 4.1 的 8 大模式逐一验证 |

### 1.2 设计模式合规性审查

| 模式 | 预期应用点 | 审查要点 |
|------|-----------|---------|
| **策略模式** | OTA 协议切换 | `IProtocol` 接口 + XModem/YModem/ZModem 具体实现，OtaManager 持有策略指针 |
| **观察者模式** | 数据流分发 | IConnection::dataReceived 信号 -> MainWindow -> TerminalModel/FrameParser/DataLogger |
| **工厂模式** | 连接创建 | ConnectionFactory::create() 返回 IConnection*，MainWindow 不知道具体类名 |
| **状态模式** | 连接状态管理 | ConnectionState 枚举驱动 UI 行为（MainWindow::onConnectionStateChanged 的 switch 分支） |
| **单例模式** | 全局管理器 | SettingsManager::instance(), ThemeManager::instance() 线程安全 |
| **模板方法** | OTA 传输流程 | BaseTransfer::execute() 定义骨架，子类 override 钩子方法 |
| **适配器模式** | J-Link SDK | RttConnection 适配 IConnection 接口（待实现） |
| **命令模式** | 快捷指令 | QuickCommand 数据结构 + QuickCommandBar 发送触发 |

### 1.3 代码规范审查

| 检查编号 | 检查内容 |
|---------|---------|
| CR-01 | 头文件引用顺序: Qt 头文件 -> STL 头文件 -> 项目头文件 |
| CR-02 | 引用路径使用相对 src 目录: `#include "core/Constants.h"` |
| CR-03 | 命名规范: 类名 PascalCase, 方法 camelCase, 成员 m_ 前缀, 常量 k 前缀 |
| CR-04 | Qt 信号/槽使用函数指针 connect 语法，禁止 SIGNAL/SLOT 宏 |
| CR-05 | 所有用户可见文字使用 tr() 包裹 |
| CR-06 | QSS 中无硬编码色值用于语义功能（主题文件除外） |
| CR-07 | 所有 QWidget 设置 objectName |

## 二、已知问题清单与修复规格

### 问题 P1: TerminalModel::lines() 返回值拷贝导致性能瓶颈

#### 问题描述

`TerminalModel::lines()` 方法（`TerminalModel.cpp` 第 45-49 行）返回 `QVector<TerminalLine>` 的完整拷贝:

```cpp
QVector<TerminalLine> TerminalModel::lines() const
{
    QMutexLocker locker(&m_mutex);
    return m_lines;  // 全量拷贝: O(n) 内存分配 + O(n) 数据复制
}
```

此外，`appendReceived()` 和 `appendSent()` 中使用 `m_lines.removeFirst()` 进行淘汰（第 20-22 行）:

```cpp
while (m_lines.size() > m_maxLines) {
    m_lines.removeFirst();  // O(n): 每次删除首元素需移动后续所有元素
}
```

#### 影响分析

| 场景 | 影响 |
|------|------|
| 高速串口数据接收 (115200 bps) | 每秒约 11KB 数据，假设每行 20 字节，约 550 行/秒。`removeFirst` 每次触发 50000 个元素的内存移动 |
| 数据导出 | `onExportData()` 调用 `m_terminalModel->lines()` 全量拷贝 50000 行数据，耗时约 5-10ms |
| 终端渲染 | TerminalWidget 每次重绘可能调用 `lines()` 获取数据，高频调用下内存拷贝开销显著 |
| 内存峰值 | `lines()` 返回拷贝期间，内存占用翻倍（原始 + 拷贝各 50000 行） |

#### 性能量化基准

| 指标 | 当前实现 | 目标值 |
|------|---------|--------|
| `removeFirst()` 单次耗时 (50000 行) | ~0.5ms (O(n)) | <0.01ms (O(1)) |
| `lines()` 全量拷贝耗时 (50000 行) | ~3-5ms | 不再需要全量拷贝 |
| `appendReceived()` 在达到上限后的单次耗时 | ~0.5ms (removeFirst) | <0.01ms (环形覆盖) |
| 内存峰值（满缓冲区时） | 2x 缓冲区大小 | 1x 缓冲区大小 |

#### 修复规格

见第三节 "新功能 PRD - R1: TerminalModel 环形缓冲区重构"。

---

### 问题 P2: FrameVisualEditor.cpp 中 tr() 字符串语言不一致

#### 问题描述

`FrameVisualEditor.cpp` 中 `tr()` 包裹的字符串存在严重的中英混用问题。通过代码审查（`FrameVisualEditor.cpp` 全文），发现以下不一致:

| 行号 | 当前代码 | 问题 |
|------|---------|------|
| 22 | `tr("帧头/帧尾配置")` | 中文 -- 正确 |
| 28 | `tr("Header:")` | 英文 -- 应为 `tr("帧头:")` 或保持英文但统一体系 |
| 33 | `tr("Footer:")` | 英文 -- 应为 `tr("帧尾:")` |
| 38 | `tr("Length Field")` | 英文 GroupBox 标题 |
| 44 | `tr("None")` | 英文特殊值文本 |
| 45 | `tr("Offset:")` | 英文 |
| 49 | `tr("Size:")` | 英文 |
| 51 | `tr("Big Endian")` | 英文 -- 技术术语 |
| 57 | `tr("Actual payload = length field value - adjust")` | 英文 tooltip |
| 58 | `tr("Adjust:")` | 英文 |
| 63 | `tr("Checksum")` | 英文 GroupBox 标题 |
| 68 | `tr("Type:")` | 英文 |
| 73 | `tr("Auto")` | 英文特殊值文本 |
| 74 | `tr("Offset:")` | 英文 |
| 79 | `tr("Start:")` | 英文 |
| 84 | `tr("Data Fields")` | 英文 GroupBox 标题 |
| 89 | `tr("Name"), tr("Type"), tr("Offset"), tr("Size"), tr("Unit")` | 英文表头 |
| 98 | `tr("Add Field")` | 英文按钮 |
| 99 | `tr("Remove Field")` | 英文按钮 |
| 108 | `tr("Apply Definition")` | 英文按钮 |

**问题本质**: 帧编辑器面板中只有"帧头/帧尾配置"相关的 3 个 tr() 使用了中文，其余全部使用英文。这不是翻译文件缺失的问题，而是源代码中 tr() 包裹的源字符串本身就混用了两种语言。Qt 的 tr() 机制以源字符串作为翻译 key，当源字符串混用中英文时，无论加载哪种语言的 .ts 翻译文件，都无法实现一致的用户体验。

#### 修复原则

选择一种统一的源字符串语言作为 tr() key:

- **方案 A (推荐)**: 源字符串统一使用中文。当 `QTranslator` 未加载时显示中文，加载英文翻译文件时显示英文。这与 OtaWidget 中大部分 tr() 使用中文的惯例一致。
- **方案 B**: 源字符串统一使用英文。需要额外为中文创建 .ts 翻译文件。与 OtaWidget 的现有惯例冲突。

#### 修复范围

| 分类 | 涉及字符串 | 统一为 |
|------|-----------|--------|
| GroupBox 标题 | `"帧头/帧尾配置"`, `"Length Field"`, `"Checksum"`, `"Data Fields"` | 中文: `"帧头/帧尾配置"`, `"长度字段"`, `"校验配置"`, `"数据字段"` |
| 表单标签 | `"Header:"`, `"Footer:"`, `"Offset:"`, `"Size:"`, `"Type:"`, `"Start:"`, `"Adjust:"` | 中文: `"帧头:"`, `"帧尾:"`, `"偏移:"`, `"大小:"`, `"类型:"`, `"起始:"`, `"调整:"` |
| 复选框 | `"Big Endian"` | 保留技术术语: `"大端序 (Big Endian)"` |
| 特殊值文本 | `"None"`, `"Auto"` | 中文: `"无"`, `"自动"` |
| 按钮 | `"Add Field"`, `"Remove Field"`, `"Apply Definition"` | 中文: `"添加字段"`, `"删除字段"`, `"应用定义"` |
| 表头 | `"Name"`, `"Type"`, `"Offset"`, `"Size"`, `"Unit"` | 中文: `"名称"`, `"类型"`, `"偏移"`, `"大小"`, `"单位"` |
| 提示文字 | `"帧头HEX字节，如 AA 55"`, `"Actual payload = ..."` | 统一中文: `"帧头HEX字节，如 AA 55"`, `"实际载荷 = 长度字段值 - 调整值"` |

---

### 问题 P3: OtaWidget.cpp 中 tr() 字符串混用中英文

#### 问题描述

`OtaWidget.cpp` 中 tr() 字符串同样存在中英混用问题。对比分析:

| 行号 | 当前代码 | 语言 |
|------|---------|------|
| 38 | `tr("固件文件")` | 中文 |
| 42 | `tr("选择固件文件 (.bin / .hex) ...")` | 中文 |
| 44 | `tr("浏览")` | 中文 |
| 54 | `tr("传输设置")` | 中文 |
| 59 | `tr("XMODEM-CRC (Recommended)")` | 混用: 协议名英文 + 括号内英文 |
| 64 | `tr("协议:")` | 中文 |
| 67 | `tr("开始传输")` | 中文 |
| 71 | `tr("取消")` | 中文 |
| 88 | `tr("传输进度")` | 中文 |
| 102 | `tr("就绪")` | 中文 |
| 115 | `tr("传输日志")` | 中文 |
| 127 | `tr("OTA历史记录")` | 中文 |
| 148 | `tr("清除历史")` | 中文 |
| 163 | `tr("Firmware files (*.bin *.hex);;...")` | **英文** -- 文件过滤器 |
| 164 | `tr("Select Firmware File")` | **英文** -- 文件对话框标题 |
| 168 | `tr("Selected file: %1")` | **英文** -- 日志消息 |
| 176 | `tr("Error: No firmware file selected")` | **英文** -- 错误消息 |
| 187 | `tr("Starting transfer: %1, Protocol: %2")` | **英文** -- 日志消息 |
| 195 | `tr("Failed to start transfer")` | **英文** -- 错误消息 |
| 202 | `tr("Transfer cancelled by user")` | **英文** -- 日志消息 |
| 209 | `tr("Transferring: %1%")` | **英文** -- 状态消息 |
| 232 | `tr("ETA: %1")` | **英文** -- 状态消息 |
| 241 | `tr("Transfer Complete")` | **英文** -- 状态消息 |
| 245 | `tr("Transfer completed in %1s")` | **英文** -- 日志消息 |
| 261 | `tr("Error: %1")` | **英文** -- 错误消息 |
| 262 | `tr("Error: %1")` | **英文** -- 日志消息 |

**规律总结**: OtaWidget 的 UI 标签（GroupBox 标题、按钮文字、placeholder）统一使用了中文 tr()，但所有运行时消息（日志输出、状态更新、错误提示、文件对话框）全部使用了英文 tr()。这种模式使得中文用户在操作过程中看到中文按钮但英文状态消息，体验割裂。

#### 修复范围

| 分类 | 当前英文 tr() | 统一为中文 |
|------|-------------|-----------|
| 文件过滤器 | `"Firmware files (*.bin *.hex);;Binary files (*.bin);;Intel HEX (*.hex);;All files (*.*)"` | `"固件文件 (*.bin *.hex);;二进制文件 (*.bin);;Intel HEX (*.hex);;所有文件 (*.*)"` |
| 对话框标题 | `"Select Firmware File"` | `"选择固件文件"` |
| 日志消息 | `"Selected file: %1"` | `"已选择文件: %1"` |
| 错误消息 | `"Error: No firmware file selected"` | `"错误: 未选择固件文件"` |
| 日志消息 | `"Starting transfer: %1, Protocol: %2"` | `"开始传输: %1, 协议: %2"` |
| 错误消息 | `"Failed to start transfer"` | `"启动传输失败"` |
| 日志消息 | `"Transfer cancelled by user"` | `"传输已由用户取消"` |
| 状态消息 | `"Transferring: %1%"` | `"传输中: %1%"` |
| 状态消息 | `"ETA: %1"` | `"预计剩余: %1"` |
| 状态消息 | `"Transfer Complete"` | `"传输完成"` |
| 日志消息 | `"Transfer completed in %1s"` | `"传输完成，耗时 %1s"` |
| 错误消息 | `"Error: %1"` | `"错误: %1"` |
| 协议选项 | `"XMODEM-CRC (Recommended)"` | `"XMODEM-CRC (推荐)"` |

---

### 问题 P4: ConnectionFactory 缺少对 RTT 类型的处理

#### 问题描述

`ConnectionFactory::create()` 方法（`ConnectionFactory.cpp` 第 7-22 行）中，`ConnectionType::Rtt` 分支直接返回 `nullptr`:

```cpp
case ConnectionType::Rtt:
    return nullptr;
```

同时，项目中 `src/rtt/` 目录为空，不存在 `RttConnection.h/cpp` 文件。`Constants.h` 中已定义 `ConnectionType::Rtt` 枚举值但无对应实现。

#### 影响分析

1. 用户点击导航树中的 RTT 相关节点时，`MainWindow::onConnectNetwork()` 会通过 `ConnectionManager::createConnection()` 调用 `ConnectionFactory::create(ConnectionType::Rtt)`，返回 `nullptr`，触发 "不支持" 提示
2. CLAUDE.md 4.1 中明确将"适配器模式"应用于"J-Link SDK 适配"，`JLinkBridge` 应适配 `IConnection` 接口。但该适配器尚未实现
3. 项目目录结构中已规划 `src/rtt/JLinkBridge.h/cpp`、`RttChannelModel.h/cpp`、`RttViewer.h/cpp`、`RttConfigPanel.h/cpp`，但均为空

#### 修复规格

| 优先级 | 内容 | 说明 |
|--------|------|------|
| P2 | 在 ConnectionFactory 中添加 RTT 的 stub 实现（返回 nullptr 并输出 warning 日志） | 短期: 明确告知该类型尚未实现，不静默失败 |
| P1 | 实现 RttConnection 适配器 | 中期: 适配 J-Link SDK 的 RTT API 到 IConnection 接口 |
| P1 | 实现 JLinkBridge | 中期: 封装 J-Link SDK 的设备发现、连接、断开操作 |

**本 PRD 范围内仅处理 P2 级别的 stub 改进**，P1 级别的完整 RTT 实现由后续 PRD 规划。

**ConnectionFactory stub 改进规格**:

```cpp
case ConnectionType::Rtt:
    qWarning() << "ConnectionFactory: RTT connection not yet implemented";
    return nullptr;
```

---

### 问题 P5: DataLogger 回放时没有时间戳对齐

#### 问题描述

`DataLogger::startPlayback()` 方法（`DataLogger.cpp` 第 116-179 行）中，回放的时间戳对齐逻辑存在缺陷:

**问题 A: 变速回放时基准时间偏移**

在 `resumePlayback()` 方法（第 206-211 行）中:

```cpp
void DataLogger::resumePlayback()
{
    if (!m_playing || !m_playbackPaused) return;
    m_playbackPaused = false;
    m_playbackElapsed.restart();  // 重启计时器
    m_playbackTimer->start();
}
```

`m_playbackElapsed.restart()` 将经过时间重置为 0，但 `m_playbackBaseTime` 未更新。在 `onPlaybackTick()` 中:

```cpp
qint64 elapsed = static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
qint64 currentTime = m_playbackBaseTime + elapsed;
```

暂停恢复后，`elapsed` 从 0 重新开始，`m_playbackBaseTime` 仍为旧值，导致时间跳跃。但更关键的是:

**问题 B: 暂停恢复后 `m_playbackBaseTime` 未累加已回放的时间偏移**

暂停时，`m_playbackBaseTime` 应该累加从上次 resume/restart 到暂停时的 elapsed 时间。否则恢复后回放位置会退回到暂停前的起点。

**问题 C: 变速回放的时间精度**

当 `m_playbackSpeed` 为较大值（如 10x、100x）时，单次 tick 的时间步进可能跨越多条记录。当前的 `while (m_nextRecordTime <= currentTime)` 循环会一次性发送所有到期记录，但这些记录之间的原始时间间隔信息丢失了。对于需要逐条观察的场景（如调试协议交互时序），这会导致不可预期的行为。

#### 修复规格

| 修复项 | 说明 |
|--------|------|
| F5-1 | `pausePlayback()` 中记录当前 `m_playbackBaseTime + m_playbackElapsed.elapsed() * m_playbackSpeed` 到 `m_playbackBaseTime` |
| F5-2 | `resumePlayback()` 中 `m_playbackElapsed.restart()` 后，由于 `m_playbackBaseTime` 已在暂停时更新，时间轴连续性得到保证 |
| F5-3 | 在 `onPlaybackTick()` 的 while 循环中，添加最大单次发送记录数限制（如 100 条），超限时分批发送避免 UI 阻塞 |

**修复后的时序逻辑**:

```
录制时间轴:  0ms --- 50ms --- 100ms --- 150ms --- 200ms --- 250ms
回放时间轴:  0ms ========= PAUSE ========== RESUME ==========
                                        ↑                    ↑
                                  baseTime=100ms        elapsed=0 (restart)
                                                       currentTime = 100 + 0*speed = 100ms
                                                       下一条记录 150ms > 100ms, 等待
```

---

## 三、新功能 PRD

### R1: TerminalModel 环形缓冲区重构 (P0 -- 性能关键)

#### 背景

当前 `TerminalModel` 使用 `QVector<TerminalLine>` 管理数据，`removeFirst()` 操作为 O(n)。项目已存在 `RingBuffer<T>` 模板类（`utils/RingBuffer.h`），但 `TerminalModel` 未使用它。

#### 需求规格

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| R1-1 | 将 `TerminalModel` 的内部存储从 `QVector<TerminalLine>` 迁移到环形缓冲区 | P0 |
| R1-2 | 淘汰旧数据时使用环形覆盖（O(1)），替代 `removeFirst()`（O(n)） | P0 |
| R1-3 | 提供 `lines(int start, int count)` 的零拷贝版本（通过迭代器或回调方式访问） | P1 |
| R1-4 | 保持 `lines()` 接口的向后兼容（仍返回 QVector，但内部从环形缓冲区构建） | P0 |
| R1-5 | `lineCount()`, `rxBytes()`, `txBytes()`, `clear()` 接口行为不变 | P0 |
| R1-6 | 线程安全保证不变（QMutex 保护所有读写操作） | P0 |

#### 接口设计

```cpp
// TerminalModel.h 修改后的核心接口
class TerminalModel : public QObject {
    Q_OBJECT

public:
    explicit TerminalModel(QObject* parent = nullptr);

    // 添加数据（行为不变）
    void appendReceived(const QByteArray& data);
    void appendSent(const QByteArray& data);

    // 获取所有行（向后兼容，内部从环形缓冲区构建 QVector）
    QVector<TerminalLine> lines() const;

    // 获取指定范围的行（向后兼容）
    QVector<TerminalLine> lines(int start, int count) const;

    // 新增: 按索引读取单行（零拷贝，避免构建整个 QVector）
    bool lineAt(int index, TerminalLine& line) const;

    // 获取总行数
    int lineCount() const;

    // 获取接收/发送字节统计
    quint64 rxBytes() const;
    quint64 txBytes() const;

    // 清空所有数据
    void clear();

    // 设置最大行数限制（同时调整环形缓冲区容量）
    void setMaxLines(int max);

signals:
    void dataAppended(int firstNewLine, int count);
    void dataCleared();

private:
    RingBuffer<TerminalLine> m_buffer;  // 替代 QVector<TerminalLine> m_lines
    mutable QMutex m_mutex;
    int m_maxLines = 50000;
    quint64 m_rxBytes = 0;
    quint64 m_txBytes = 0;
};
```

#### 实现要点

1. **RingBuffer 容量管理**: `setMaxLines()` 调用 `m_buffer.setCapacity(max)`。注意 `setCapacity` 会清空现有数据，需要在设置前发出警告或选择性地保留最新数据。
2. **淘汰策略**: 环形缓冲区的 `push()` 方法在满时自动覆盖最旧数据（O(1)），天然替代 `removeFirst()` + `append()` 的组合。
3. **lines() 兼容实现**: 遍历 RingBuffer 从索引 0 到 count()-1，逐个 `at()` 读取并构建 QVector。这是降级兼容接口，高频调用者应迁移到 `lineAt()` 或 `lines(start, count)`。
4. **dataAppended 信号**: `push()` 返回新元素的环形索引，用于计算 `firstNewLine` 参数。
5. **RingBuffer<T> 模板实例化**: `RingBuffer<TerminalLine>` 需要TerminalLine 支持拷贝赋值（当前已支持）。

#### 依赖的公共组件

| 组件 | 文件 | 复用方式 |
|------|------|---------|
| `RingBuffer<T>` | `utils/RingBuffer.h` | 直接使用，作为 `TerminalModel` 的内部存储 |

#### 设计模式

| 模式 | 应用场景 |
|------|---------|
| **策略模式**（内部实现变更） | 存储策略从 QVector 切换到 RingBuffer，外部接口不变 |

#### 影响范围

| 文件 | 修改内容 |
|------|---------|
| `src/terminal/TerminalModel.h` | 将 `QVector<TerminalLine> m_lines` 替换为 `RingBuffer<TerminalLine> m_buffer`；新增 `lineAt()` 方法 |
| `src/terminal/TerminalModel.cpp` | 重写 `appendReceived()`, `appendSent()`, `lines()`, `clear()`, `setMaxLines()` |
| `src/terminal/TerminalWidget.cpp` | 如有高频调用 `lines()` 的渲染逻辑，建议迁移到 `lineAt()` |
| `src/utils/DataExporter.cpp` | 调用 `lines()` 导出数据，接口不变无需修改 |

#### 验收标准

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| AC-R1-1 | `appendReceived()` 在缓冲区满时的单次耗时 < 0.01ms | 使用 `QElapsedTimer` 测量 10000 次连续 append 的平均耗时 |
| AC-R1-2 | `lines()` 返回结果与重构前完全一致（相同输入产生相同输出） | 单元测试: 构造 1000 条数据，对比新旧实现的 `lines()` 返回值 |
| AC-R1-3 | `lineAt()` 单次访问耗时 < 0.001ms | 性能测试 |
| AC-R1-4 | 内存占用不随 append 操作线性增长（环形覆盖） | 监控: append 100000 条数据后，内存占用稳定在 m_maxLines 条的水平 |
| AC-R1-5 | `clear()` 后 `lineCount()` == 0，`rxBytes()` == 0 | 功能测试 |
| AC-R1-6 | 编译零错误零警告，EmbedDebug.bat 正常启动 | 构建验证 |

---

### R2: 导航树面板切换重构 (P1 -- 架构改善)

#### 背景

`MainWindow::connectSignals()` 中的导航树点击处理（第 419-481 行）使用 `if-else` 链匹配面板名称:

```cpp
if (text == tr("配置"))        target = m_serialConfig;
else if (text == tr("终端"))   target = m_terminal;
else if (text == tr("统计"))   target = m_dataStats;
else if (text == tr("协议"))   target = m_protocolView;
else if (text == tr("帧编辑器")) target = m_frameEditor;
else if (text == tr("波形图")) target = m_chartWidget;
else if (text == tr("OTA升级")) target = m_otaWidget;
```

当前代码已做了部分重构（使用 `allPanels[]` 数组收集面板），但目标面板的匹配仍依赖字符串比较的 `if-else` 链。每次新增导航面板都需要修改这段代码，违反了开放封闭原则。

#### 需求规格

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| R2-1 | 建立导航节点名称到面板 widget 的注册映射表，消除 if-else 链 | P1 |
| R2-2 | 新增导航面板时只需在注册表中添加一行，无需修改切换逻辑 | P1 |
| R2-3 | 功能性节点（数据导出、网络连接）走独立的命令分发，不走面板切换 | P1 |
| R2-4 | 保持面板切换动画行为不变 | P1 |

#### 接口设计

```cpp
// MainWindow.h 新增:
struct NavPanelEntry {
    QString name;       // 导航节点显示名 (tr() 的源字符串)
    QWidget* panel;     // 对应的面板 widget
};

// MainWindow.h 新增成员:
QMap<QString, QWidget*> m_navPanelMap;          // 面板名称 -> widget 映射
QList<QWidget*> m_allNavPanels;                  // 所有可切换面板的有序列表
QMap<QString, std::function<void()>> m_navActionMap;  // 功能性节点的命令映射
```

```cpp
// MainWindow.cpp setupUI() 末尾，注册所有面板:
m_navPanelMap = {
    {tr("配置"),    m_serialConfig},
    {tr("终端"),    m_terminal},
    {tr("统计"),    m_dataStats},
    {tr("协议"),    m_protocolView},
    {tr("帧编辑器"), m_frameEditor},
    {tr("波形图"),   m_chartWidget},
    {tr("OTA升级"),  m_otaWidget},
};
m_allNavPanels = {
    m_serialConfig, m_terminal, m_dataStats,
    m_protocolView, m_frameEditor, m_chartWidget, m_otaWidget
};

// 功能性节点命令
m_navActionMap = {
    {tr("数据导出"),  [this]() { onExportData(); }},
    {tr("TCP客户端"), [this]() { onConnectNetwork(ConnectionType::TcpClient); }},
    {tr("TCP服务端"), [this]() { onConnectNetwork(ConnectionType::TcpServer); }},
    {tr("UDP"),       [this]() { onConnectNetwork(ConnectionType::Udp); }},
};
```

```cpp
// MainWindow.cpp connectSignals() 中的导航树点击处理:
connect(m_navTree, &QTreeView::clicked, this, [this](const QModelIndex& index) {
    QString text = index.data().toString();

    // 1. 检查是否为功能性节点
    if (m_navActionMap.contains(text)) {
        m_navActionMap[text]();
        return;
    }

    // 2. 查找面板映射
    auto it = m_navPanelMap.find(text);
    if (it == m_navPanelMap.end()) return;
    QWidget* target = it.value();

    // 3. 切换面板: 隐藏所有，只显示目标
    for (auto* w : m_allNavPanels) {
        if (w) w->setVisible(w == target);
    }

    // 4. 面板淡入动画（逻辑不变）
    // ... 保持现有动画代码 ...
});
```

#### 影响范围

| 文件 | 修改内容 |
|------|---------|
| `src/core/MainWindow.h` | 新增 `m_navPanelMap`, `m_allNavPanels`, `m_navActionMap` 成员 |
| `src/core/MainWindow.cpp` | `setupUI()` 末尾注册映射表；`connectSignals()` 中替换 if-else 链 |

#### 验收标准

| 编号 | 验收条件 |
|------|---------|
| AC-R2-1 | 点击导航树所有面板节点，切换行为与重构前完全一致 |
| AC-R2-2 | 点击"数据导出"仍弹出导出对话框 |
| AC-R2-3 | 点击"TCP客户端"、"TCP服务端"、"UDP"仍触发对应的网络连接流程 |
| AC-R2-4 | 面板切换动画（淡入效果）正常工作 |
| AC-R2-5 | `connectSignals()` 中的导航树 lambda 中不再有任何 `if-else` 链用于面板匹配 |
| AC-R2-6 | 编译零错误零警告 |

---

### R3: 帧编辑器 UI 字符串统一到 tr() 体系 (P1 -- 国际化基础)

#### 背景

问题 P2 已详细列出 FrameVisualEditor.cpp 中 tr() 源字符串混用中英文的情况。本功能将 FrameVisualEditor 和 OtaWidget 中所有 tr() 源字符串统一为中文，为后续的 .ts 翻译文件体系打好基础。

#### 需求规格

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| R3-1 | FrameVisualEditor.cpp 中所有 tr() 源字符串统一为中文 | P1 |
| R3-2 | OtaWidget.cpp 中所有 tr() 源字符串统一为中文 | P1 |
| R3-3 | 修改后的字符串在无翻译文件加载时显示中文（默认语言） | P1 |
| R3-4 | 修改后的字符串在加载英文翻译文件时显示英文（需创建 .ts 文件） | P2 |
| R3-5 | ComboBox 中硬编码的英文项（如 "UInt8", "None", "CRC16-CCITT"）保留为技术术语不翻译 | P1 |

#### 修改清单

**FrameVisualEditor.cpp 修改项**:

| 行号 | 修改前 | 修改后 |
|------|--------|--------|
| 22 | `tr("帧头/帧尾配置")` | 不变 |
| 27 | `tr("帧头HEX字节，如 AA 55")` | 不变 |
| 28 | `tr("Header:")` | `tr("帧头:")` |
| 32 | `tr("帧尾HEX字节（可选）")` | 不变 |
| 33 | `tr("Footer:")` | `tr("帧尾:")` |
| 38 | `tr("Length Field")` | `tr("长度字段")` |
| 44 | `tr("None")` | `tr("无")` |
| 45 | `tr("Offset:")` | `tr("偏移:")` |
| 48 | `m_lengthSizeCombo->addItems({"1 byte", "2 bytes"})` | `m_lengthSizeCombo->addItems({tr("1 字节"), tr("2 字节")})` |
| 49 | `tr("Size:")` | `tr("大小:")` |
| 51 | `tr("Big Endian")` | `tr("大端序")` |
| 57 | `tr("Actual payload = length field value - adjust")` | `tr("实际载荷 = 长度字段值 - 调整值")` |
| 58 | `tr("Adjust:")` | `tr("调整:")` |
| 63 | `tr("Checksum")` | `tr("校验配置")` |
| 67 | `m_checksumTypeCombo->addItems({"None", "Sum8", ...})` | `m_checksumTypeCombo->addItems({tr("无"), "Sum8", "CRC8", "CRC16-CCITT", "CRC16-Modbus", "CRC32"})` -- 校验算法名为技术术语保留英文 |
| 68 | `tr("Type:")` | `tr("类型:")` |
| 73 | `tr("Auto")` | `tr("自动")` |
| 74 | `tr("Offset:")` | `tr("偏移:")` |
| 79 | `tr("Start:")` | `tr("起始:")` |
| 84 | `tr("Data Fields")` | `tr("数据字段")` |
| 88-90 | `tr("Name"), tr("Type"), ...` | `tr("名称"), tr("类型"), tr("偏移"), tr("大小"), tr("单位")` |
| 98 | `tr("Add Field")` | `tr("添加字段")` |
| 99 | `tr("Remove Field")` | `tr("删除字段")` |
| 108 | `tr("Apply Definition")` | `tr("应用定义")` |
| 166 | `QString("field_%1")` | `tr("字段_%1")` |
| 171 | `"0"` | 不变（数值） |
| 172 | `"1"` | 不变（数值） |
| 247 | typeCombo 英文项列表 | 同第 67 行处理 |

**OtaWidget.cpp 修改项**:

| 行号 | 修改前 | 修改后 |
|------|--------|--------|
| 59 | `tr("XMODEM-CRC (Recommended)")` | `tr("XMODEM-CRC (推荐)")` |
| 163 | `tr("Firmware files (*.bin *.hex);;...")` | `tr("固件文件 (*.bin *.hex);;二进制文件 (*.bin);;Intel HEX (*.hex);;所有文件 (*.*)")` |
| 164 | `tr("Select Firmware File")` | `tr("选择固件文件")` |
| 168 | `tr("Selected file: %1")` | `tr("已选择文件: %1")` |
| 176 | `tr("Error: No firmware file selected")` | `tr("错误: 未选择固件文件")` |
| 187 | `tr("Starting transfer: %1, Protocol: %2")` | `tr("开始传输: %1, 协议: %2")` |
| 195 | `tr("Failed to start transfer")` | `tr("启动传输失败")` |
| 202 | `tr("Transfer cancelled by user")` | `tr("传输已由用户取消")` |
| 209 | `tr("Transferring: %1%")` | `tr("传输中: %1%")` |
| 232 | `tr("ETA: %1")` | `tr("预计剩余: %1")` |
| 241 | `tr("Transfer Complete")` | `tr("传输完成")` |
| 245 | `tr("Transfer completed in %1s")` | `tr("传输完成，耗时 %1 秒")` |
| 261 | `tr("Error: %1")` | `tr("错误: %1")` |

#### 影响范围

| 文件 | 修改行数（估算） |
|------|----------------|
| `src/protocol/FrameVisualEditor.cpp` | ~25 处 tr() 字符串修改 |
| `src/ota/OtaWidget.cpp` | ~13 处 tr() 字符串修改 |

#### 验收标准

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| AC-R3-1 | FrameVisualEditor 所有 tr() 源字符串为中文 | grep `tr(".*[a-zA-Z]")` 不应匹配到非技术术语的英文 tr() 调用 |
| AC-R3-2 | OtaWidget 所有 tr() 源字符串为中文 | 同上 |
| AC-R3-3 | 应用启动后帧编辑器面板显示全中文界面 | 手动验证 |
| AC-R3-4 | OTA 面板操作过程中的状态消息、日志输出均为中文 | 启动传输测试 |
| AC-R3-5 | 校验算法名（Sum8, CRC8, CRC16-CCITT 等）、数据类型名（UInt8, Int16LE 等）保留英文 | 这些是技术术语，不应翻译 |
| AC-R3-6 | 编译零错误零警告 | 构建验证 |

---

## 四、综合验收标准

### 4.1 架构审查验收

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| AC-A1 | 所有模块分层归属正确，无反向依赖 | 人工审查所有 `#include` 指令，确认依赖方向符合四层架构规则 |
| AC-A2 | 8 大设计模式在代码中正确应用 | 逐一验证各模式的应用点，检查接口设计、多态使用、生命周期管理 |
| AC-A3 | 公共组件清单中的组件无重复实现 | grep 搜索 CRC、HexConverter、RingBuffer 等关键词，确认无重复定义 |
| AC-A4 | 所有新增类已登记到公共组件清单（如适用） | 检查 CLAUDE.md 4.4 节 |
| AC-A5 | 头文件引用规范全部符合 | 检查所有 .h/.cpp 文件的 #include 顺序和路径格式 |
| AC-A6 | 所有 QWidget 已设置 objectName | grep 检查 `new QWidget`, `new QPushButton` 等后是否紧跟 `setObjectName` |

### 4.2 已知问题修复验收

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| AC-P1 | TerminalModel 环形缓冲区重构后性能指标达标 | 见 R1 验收标准 |
| AC-P2 | FrameVisualEditor tr() 源字符串全中文 | 见 R3 验收标准 |
| AC-P3 | OtaWidget tr() 源字符串全中文 | 见 R3 验收标准 |
| AC-P4 | ConnectionFactory RTT 分支输出 warning 日志而非静默返回 nullptr | 调用 `ConnectionFactory::create(ConnectionType::Rtt)` 后检查 qDebug 输出 |
| AC-P5 | DataLogger 暂停恢复后回放位置连续，无跳跃 | 录制含 50 条间隔不等的数据，回放中暂停-恢复-变速，验证数据按原始时序输出 |

### 4.3 新功能验收

| 编号 | 验收条件 | 度量方法 |
|------|---------|---------|
| AC-R1 | TerminalModel 环形缓冲区重构功能正确性和性能达标 | 见 R1 验收标准 |
| AC-R2 | 导航树面板切换映射表重构后所有面板切换正常 | 见 R2 验收标准 |
| AC-R3 | 帧编辑器和 OTA 面板 UI 字符串统一为中文 | 见 R3 验收标准 |

### 4.4 全局质量验收

| 编号 | 验收条件 |
|------|---------|
| AC-G1 | 零编译错误，零编译警告 |
| AC-G2 | EmbedDebug.bat 启动验证通过 |
| AC-G3 | 所有现有功能回归测试通过（串口连接、发送/接收、OTA 传输、数据导出、主题切换） |
| AC-G4 | 代码变更行数 >= 300 行（满足 commit 规则） |

## 五、实施优先级与分工

| 阶段 | 内容 | 负责角色 | 依赖 |
|------|------|---------|------|
| 阶段 1 | 架构审查检查 (AR-01 ~ AR-07, CR-01 ~ CR-07) | 代码审查员 | 无 |
| 阶段 2 | R1: TerminalModel 环形缓冲区重构 | 核心开发 | 阶段 1 完成 |
| 阶段 3 | R2: 导航树面板切换重构 | UI 开发 | 阶段 1 完成 |
| 阶段 4 | R3: 帧编辑器 + OTA UI 字符串统一 | UI 开发 | 阶段 1 完成 |
| 阶段 5 | P4: ConnectionFactory RTT stub 改进 | 核心开发 | 阶段 1 完成 |
| 阶段 6 | P5: DataLogger 回放时间戳对齐修复 | 核心开发 | 阶段 1 完成 |
| 阶段 7 | 编译验证 + 回归测试 | QA 工程师 | 阶段 2-6 全部完成 |
| 阶段 8 | 代码审查 + commit | 代码审查员 | 阶段 7 通过 |

**说明**: 阶段 2-6 之间无依赖关系，可并行实施。阶段 1 的审查结果可能发现额外的架构问题，需要在阶段 2-6 中一并修复。

## 六、变更文件汇总

| 文件 | 变更类型 | 涉及需求 |
|------|---------|---------|
| `src/terminal/TerminalModel.h` | 修改 | R1: 内部存储迁移到 RingBuffer |
| `src/terminal/TerminalModel.cpp` | 重写 | R1: append/lines/clear/setMaxLines 实现 |
| `src/core/MainWindow.h` | 修改 | R2: 新增导航映射表成员 |
| `src/core/MainWindow.cpp` | 修改 | R2: 导航切换逻辑重构 |
| `src/protocol/FrameVisualEditor.cpp` | 修改 | R3/P2: tr() 字符串统一 |
| `src/ota/OtaWidget.cpp` | 修改 | R3/P3: tr() 字符串统一 |
| `src/core/ConnectionFactory.cpp` | 修改 | P4: RTT stub 添加 warning |
| `src/utils/DataLogger.cpp` | 修改 | P5: 回放时间戳对齐修复 |
