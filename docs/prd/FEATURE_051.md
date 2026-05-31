# FEATURE-051: 新特性评估提案 (Commit #51, 17x3 周期)

> **提案人**: 新特性设计者 (角色 #10)
> **评估周期**: Commit #51 (17 x 3)
> **目标交付起始**: Commit #54 (18 x 3)
> **状态**: 待评审

---

## 0. 上下文分析

### 0.1 当前项目状态

项目已完成 55 次 commit，积累了以下核心能力:

| 能力域 | 已实现模块 | 成熟度 |
|--------|-----------|--------|
| 连接层 | IConnection + Serial/TCP/UDP + DTR/RTS/信号线监控 | 稳定 |
| 数据协议 | FrameParser + JustFloat/FireWater + ProtocolBridgeManager | 稳定 |
| 终端显示 | TerminalWidget (自绘) + 搜索/HEX/十进制/方向前缀 | 稳定 |
| 波形图 | ChartWidget + ChartModel (滑动窗口/降采样) + 10通道 + 双调色板 | 基础完成 |
| OTA | XMODEM/YMODEM/ZMODEM + BaseTransfer模板方法 | 稳定 |
| 数据管理 | DataLogger (EDL v2) + 录制/回放 + 书签 + 范围导出 | 稳定 |
| UI系统 | 三主题 + QSS语义色板 + 面板切换动画 + Toast + 呼吸灯 | 稳定 |

### 0.2 已消耗的候选池

| 候选项 | 状态 |
|--------|------|
| JustFloat/FireWater 协议 (P0) | **已实现** (FEATURE-001, commit #22-#24) |
| 串口驱动检测 (P2) | **部分实现** (SerialDriverDetector 已存在) |
| 数据录制增强 (P1) | **部分实现** (书签/seek/范围导出已在 #43/#44 落地) |

### 0.3 上一轮评估回顾 (Commit #42, 14x3)

FEATURE-042 提出了三个候选:
- **042A**: Chart Cursor Measurement & Zoom/Pan (中选, 部分能力随 #43/#44 渗透)
- **042B**: Modbus RTU Frame Template (暂缓)
- **042C**: Data Recording Enhancement (书签部分已实现)

---

## 1. 提案 A: 高级波形引擎 -- FFT频谱 / 直方图 / 多Y轴 / 游标测量

### 1.1 功能描述

将 ChartWidget 从 "被动显示线条" 升级为 "交互式信号分析仪器"。参考 VOFA+ 和 Serial Studio，增加以下能力:

| 子功能 | 用户价值 |
|--------|---------|
| **FFT 频谱分析** | 对选定通道做实时 FFT，显示频域波形。嵌入式开发者调试 PWM/ADC 采样频率、传感器振动频谱时必不可少 |
| **直方统计** | 统计选定通道的值分布直方图。快速判断信号是否偏移、噪声幅度、量程是否合理 |
| **多 Y 轴** | 当温度通道 (20-80) 和电压通道 (0-3.3) 同时显示时，共用一个 Y 轴导致低压信号被压成一条线。多 Y 轴让每条曲线有独立的刻度 |
| **游标测量** | 双游标垂直线，拖拽读取精确值和 delta。用户故事: "我的 PWM 输出在某个时刻出现了毛刺，我需要读出毛刺的精确幅值和时间偏移" |
| **缩放平移** | 滚轮缩放 Y 轴 (Ctrl+滚轮缩放 XY)，中键拖拽平移，"重置视图"按钮回到自动滚动模式 |

### 1.2 为什么是现在

三个理由让这个时机最合适:

**理由一: 基础波形管道已经稳定。** ChartModel (滑动窗口+降采样)、ChannelConfig (数据源映射+线性变换)、IProtocolBridge (JustFloat/FireWater/FrameParser 统一数据入口) 三层管道已经跑通并经过多轮 Bug 审计。FFT/直方图只需要从 ChartModel 的缓冲区读取数据做数学变换，不碰连接层和协议层。

**理由二: FEATURE-042A 的游标/缩放提案已经设计过。** 虽然没有作为独立 feature 通过，但 #43/#44 的 Bug 修复冲刺已经自然包含了部分交互增强 (DataBookmark seek、进度条 objectName、QSS 一致性)。游标测量和缩放平移的架构设计在 FEATURE-042A 中已经想清楚了，不需要从零开始。

**理由三: 波形图是 EmbedDebug 对标 VOFA+ 的核心差异化战场。** CLAUDE.md §3.4.4 明确写了 "目标：超越VOFA+的波形体验"。当前 ChartWidget 只有基础线条渲染，没有 FFT、没有直方图、没有游标。VOFA+ 有。如果我们不尽快补上这个差距，用户没有理由从 VOFA+ 迁移过来。

### 1.3 架构影响

#### 新增文件

| 文件 | 层次 | 职责 |
|------|------|------|
| `src/chart/ChartAnalysisEngine.h/.cpp` | 数据层 | 纯数学引擎: FFT (Cooley-Tukey radix-2)、直方统计 (分箱+计数)、Y 轴范围计算。无 Qt 依赖的纯 C++ 类 |
| `src/chart/ChartCursorOverlay.h/.cpp` | 表现层 | 透明 QWidget 叠在 QChartView 之上，QPainter 绘制双游标线、值标签、delta 读出 |
| `src/chart/ChartInteractionController.h/.cpp` | 业务层 | 管理游标位置、缩放/平移状态，协调 ChartModel 查询和 ChartCursorOverlay 渲染 |
| `src/chart/MultiAxisManager.h/.cpp` | 业务层 | 管理多个 QValueAxis 实例与通道的映射关系，处理 Y 轴自动/手动范围切换 |

#### 修改文件

| 文件 | 改动范围 |
|------|---------|
| `src/chart/ChartWidget.h/.cpp` | 添加分析模式切换 (时域/频域/直方)、游标开关按钮、多 Y 轴初始化逻辑。~150 行改动 |
| `src/chart/ChartModel.h/.cpp` | 添加 `valueAt(channel, xIndex)`、`dataRange(channel, xStart, xEnd)` 查询接口。~50 行改动 |
| `src/core/Constants.h` | 添加 `enum class ChartAnalysisMode { TimeDomain, FFT, Histogram }` |
| `resources/themes/*.qss` (3个文件) | 添加游标颜色 token、分析面板样式。每个主题 ~20 行 |

#### 复用的现有组件

| 组件 | 复用方式 |
|------|---------|
| `ChartModel` | FFT/直方从 `channelData()` 和 `allChannelData()` 获取原始采样数据 |
| `ChannelConfig` | 通道的 scale/offset 线性变换在 FFT 之前应用 |
| `ChartColors` | 多 Y 轴的颜色从现有调色板分配 |
| `ThemeManager` | 游标和分析面板的视觉样式走语义色板 |
| `SettingsManager` | 持久化分析模式、游标位置、多 Y 轴配置 |

#### 设计模式

| 模式 | 应用 |
|------|------|
| 策略模式 | `ChartAnalysisEngine` 提供统一的 `analyze(data, mode)` 接口，FFT 和 Histogram 是两种策略 |
| 观察者模式 | `ChartInteractionController` 监听 ChartModel 的 `dataUpdated` 信号，驱动游标值刷新 |
| 单一职责 | 游标渲染 (`ChartCursorOverlay`) 和交互逻辑 (`ChartInteractionController`) 严格分离 |

### 1.4 工作量估算

| 阶段 | 文件 | 代码行数 | 迭代 |
|------|------|---------|------|
| 核心数学 | `ChartAnalysisEngine.h/.cpp` | ~350 行 | #54 |
| 多 Y 轴 + 游标 | `MultiAxisManager` + `ChartCursorOverlay` + `ChartInteractionController` | ~500 行 | #55 |
| ChartWidget 集成 + UI | `ChartWidget` 改造 + `Constants` + QSS | ~400 行 | #56 |
| FFT/直方图面板渲染 | FFT 频谱渲染 + 直方图渲染 + 模式切换动画 | ~350 行 | #57 |
| **合计** | 12 个文件 (4 新增 + 8 修改) | **~1600 行** | **4 个迭代** |

### 1.5 用户价值评估

| 维度 | 评分 (1-5) | 说明 |
|------|-----------|------|
| 痛点频率 | 5 | 每个 EmbedDebug 用户都会使用波形图。没有 FFT/游标的波形图是 "半成品" |
| 竞品对标 | 5 | VOFA+ 有 FFT+直方图，Serial Studio 有仪表盘。没有这些功能就无法竞争 |
| 差异化潜力 | 4 | 如果做得比 VOFA+ 更好 (多 Y 轴+游标+FFT 一体化)，这是真正的卖点 |
| 依赖就绪度 | 5 | ChartModel + IProtocolBridge 管道完全就绪，不需要改动连接层或协议层 |

---

## 2. 提案 B: Modbus RTU/ASCII 帧解析模板

### 2.1 功能描述

在 FrameVisualEditor 中添加 "加载模板" 按钮，预置 Modbus RTU 和 Modbus ASCII 的帧定义。用户选择模板后自动填充帧头、长度字段、校验算法、字段布局，无需手动配置即可解析 Modbus 报文。

| 子功能 | 用户价值 |
|--------|---------|
| **模板注册表** | `FrameTemplateLibrary` 单例，加载 `resources/templates/*.json` 中的预置模板 |
| **Modbus RTU 模板** | 从站地址 + 功能码 + 数据 + CRC16-Modbus。覆盖读保持寄存器(0x03)、读线圈(0x01)、写单个寄存器(0x06) |
| **Modbus ASCII 模板** | `:` 前缀 + LRC 校验 + CR/LF 后缀 |
| **模板下拉菜单** | 在 FrameVisualEditor 工具栏添加模板选择按钮，选择后自动填充整个帧定义 |
| **用户自定义模板保存** | 用户配置好的帧定义可以 "另存为模板" 到本地 JSON 文件 |

### 2.2 为什么是现在

**理由一: 帧解析基础设施完全就绪。** FrameParser (状态机)、FrameVisualEditor (可视化编辑)、FrameDefinition (JSON 序列化)、CRC (CRC16-Modbus 已在 `utils/CRC.h` 中) 全部就绪。模板功能只需要一个 "加载预设 + 填充 UI" 的薄层。

**理由二: FEATURE-042B 已设计过但被暂缓。** 上一轮评估中这个提案已经被写好了详细设计，当时因为优先让 Bug 修复冲刺通过而被暂缓。设计工作不需要重做，直接复用。

**理由三: 工业用户群的敲门砖。** Modbus 是工业 MCU 开发中最常见的协议。EmbedDebug 目前的用户群偏向消费电子/物联网，提供 Modbus 模板是进入工业调试市场的最低成本入口。

### 2.3 架构影响

#### 新增文件

| 文件 | 层次 | 职责 |
|------|------|------|
| `src/protocol/FrameTemplateLibrary.h/.cpp` | 数据层 | 模板注册表: 加载 JSON 文件、查询模板列表、返回 FrameDefinition |
| `resources/templates/modbus_rtu.json` | 资源 | Modbus RTU 响应帧定义 (Read Holding Registers / Read Coils / Write Single Register) |
| `resources/templates/modbus_ascii.json` | 资源 | Modbus ASCII 帧定义 |

#### 修改文件

| 文件 | 改动范围 |
|------|---------|
| `src/protocol/FrameVisualEditor.h/.cpp` | 添加 "加载模板" 按钮和下拉菜单、`loadTemplate()` 方法。~80 行 |
| `src/protocol/FrameDefinition.h/.cpp` | 确保 JSON 序列化支持模板元数据 (name/category/description)。~30 行 |
| `resources/app.qrc` | 添加模板 JSON 文件条目。~5 行 |

#### 复用的现有组件

| 组件 | 复用方式 |
|------|---------|
| `FrameDefinition` | 模板就是预填充的 FrameDefinition，序列化/反序列化完全复用 |
| `FrameVisualEditor` | `loadTemplate()` 调用现有的 `setDefinition()` 刷新 UI |
| `CRC` (utils/CRC.h) | CRC16-Modbus 已实现，Modbus RTU 模板直接引用 |
| `SettingsManager` | 持久化用户最近使用的模板和自定义模板路径 |

### 2.4 工量估算

| 阶段 | 文件 | 代码行数 | 迭代 |
|------|------|---------|------|
| 模板引擎 + JSON | `FrameTemplateLibrary` + JSON 模板文件 | ~200 行 | 1 个迭代 |
| UI 集成 | `FrameVisualEditor` 改造 + 保存模板 + QSS | ~200 行 | 1 个迭代 |
| **合计** | 5 个文件 (1 新增 + 2 资源 + 2 修改) | **~400 行** | **2 个迭代** |

### 2.5 用户价值评估

| 维度 | 评分 (1-5) | 说明 |
|------|-----------|------|
| 痛点频率 | 3 | 不是每个 EmbedDebug 用户都用 Modbus，但工业用户的需求很集中 |
| 竞品对标 | 3 | Docklight 有，但 Docklight 是付费软件。免费工具中 Modbus 模板不常见 |
| 差异化潜力 | 3 | 模板本身不是差异化，但模板 + 可视化编辑器 + 实时解析的组合有吸引力 |
| 依赖就绪度 | 5 | FrameParser/CRC/FrameVisualEditor 全部就绪，零风险 |

---

## 3. 提案 C: 虚拟串口回环测试

### 3.1 功能描述

在 EmbedDebug 内部创建一对虚拟串口 (TX↔RX 互联)，用户无需任何硬件即可测试收发功能、验证协议解析、调试快捷指令。参考 com0com 的核心功能，但作为 EmbedDebug 的内置特性而非独立驱动。

| 子功能 | 用户价值 |
|--------|---------|
| **虚拟串口对创建** | 使用 Windows `com0com` 或 `com2tcp` 后端，一键创建虚拟 COM 端口对 |
| **回环模式** | 一个端口发送的数据自动出现在另一个端口的接收缓冲区 |
| **延迟注入** | 可配置固定延迟 (1-1000ms)，模拟真实串口的传输延迟 |
| **错误注入** | 按概率注入帧错误/校验错误，测试错误恢复逻辑 |
| **无硬件开发验证** | 开发新功能 (帧解析、OTA) 时无需插上真实设备 |

### 3.2 为什么是现在

**理由一: 开发效率工具。** 当前 EmbedDebug 的开发团队 (也就是 AI Agent 们) 每次验证新功能都需要 "假设有真实硬件"。内置虚拟串口让开发和测试形成闭环，不用硬件就能验证数据流。

**理由二: 复杂度最低。** 在所有剩余候选中，这个特性的代码量最少、架构影响最小、风险最低。在高级波形引擎这种大特性的间隙插入一个小特性，可以给团队节奏上的缓冲。

**理由三: 降低新用户上手门槛。** 用户下载 EmbedDebug 后如果没有串口设备，只能看到一个空界面。虚拟回环让新用户立即看到数据流动，形成第一印象。

### 3.3 架构影响

#### 新增文件

| 文件 | 层次 | 职责 |
|------|------|------|
| `src/connection/VirtualPortPair.h/.cpp` | 基础设施层 | 虚拟串口对管理: 创建/销毁、数据转发、延迟注入 |
| `src/serial/VirtualPortConfigPanel.h/.cpp` | 表现层 | 虚拟串口配置面板: 创建端口对按钮、延迟/错误注入参数 |

#### 修改文件

| 文件 | 改动范围 |
|------|---------|
| `src/connection/ConnectionFactory.h/.cpp` | 添加 VirtualConnection 类型。~30 行 |
| `src/core/ConnectionController.h/.cpp` | 添加虚拟端口创建/连接逻辑。~50 行 |
| `src/core/Constants.h` | 添加 `ConnectionType::Virtual`。~3 行 |
| `resources/themes/*.qss` | 虚拟端口面板样式。每个主题 ~15 行 |

#### 依赖决策

有两种实现路径:

| 方案 | 依赖 | 优点 | 缺点 |
|------|------|------|------|
| **A: com0com 后端** | 需要安装 com0com 驱动 | 完全模拟真实串口，所有串口功能可测试 | 外部依赖，安装复杂 |
| **B: 内存回环** | 无外部依赖，QBuffer 实现 | 零依赖，开箱即用 | 不是真实串口，无法测试驱动相关功能 |

**推荐方案 B**: 纯内存回环。创建一对 `IConnection` 实现，内部用 `QBuffer` + `QByteArray` 互联。延迟通过 `QTimer::singleShot` 模拟。这样零外部依赖，且完全复用现有 `IConnection` 抽象。

### 3.4 工作量估算

| 阶段 | 文件 | 代码行数 | 迭代 |
|------|------|---------|------|
| 虚拟连接 + 回环逻辑 | `VirtualPortPair` + `VirtualConnection` | ~300 行 | 1 个迭代 |
| 配置面板 + UI 集成 | `VirtualPortConfigPanel` + `ConnectionFactory` 改造 + QSS | ~250 行 | 1 个迭代 |
| **合计** | 6 个文件 (2 新增 + 4 修改) | **~550 行** | **2 个迭代** |

### 3.5 用户价值评估

| 维度 | 评分 (1-5) | 说明 |
|------|-----------|------|
| 痛点频率 | 3 | 日常调试不常用，但在开发/测试/演示场景中价值很高 |
| 竞品对标 | 2 | com0com 免费可用，EmbedDebug 的差异化是 "内置" 而非 "能力" |
| 差异化潜力 | 2 | 功能本身不构成竞争壁垒，但提升 "开箱即用" 体验 |
| 依赖就绪度 | 5 | IConnection 抽象 + ConnectionFactory 工厂模式完美支持新连接类型 |

---

## 4. 综合对比与推荐

### 4.1 三提案对比矩阵

| 维度 | A: 高级波形引擎 | B: Modbus 模板 | C: 虚拟串口回环 |
|------|----------------|---------------|----------------|
| **优先级** | P0 | P2 (暂缓提升) | P2 |
| **用户价值** | 极高 | 中高 | 中 |
| **代码量** | ~1600 行 | ~400 行 | ~550 行 |
| **迭代数** | 4 | 2 | 2 |
| **架构风险** | 中 (改动 ChartWidget 核心) | 极低 | 低 |
| **复用潜力** | 高 (分析引擎可独立复用) | 中 (模板机制通用) | 低 |
| **竞品紧迫性** | 紧迫 (VOFA+ 有 FFT) | 不紧迫 | 不紧迫 |
| **依赖就绪度** | 完全就绪 | 完全就绪 | 完全就绪 |

### 4.2 推荐方案

**推荐提案 A (高级波形引擎) 作为 #54 起始的下一特性。**

理由:

1. **竞争窗口正在关闭。** VOFA+ 在 2024 年已经提供了 FFT + 直方图 + 游标测量。EmbedDebug 的波形图目前只有基础线条渲染，这是对标 VOFA+ 的核心差距。每推迟一个评估周期 (6 个 commit)，差距就拉大一次。

2. **基础设施不再增长。** ChartModel、IProtocolBridge、ChannelConfig 三层管道已经稳定了 30+ 个 commit。FFT 和直方图从这些管道读取数据做数学变换，不需要也不应该等待任何新的基础设施。等待不会让实现更简单。

3. **1600 行分 4 个迭代是可控的。** 每个迭代 300-400 行，符合 CLAUDE.md 的 300 行最小 commit 规则。ChartAnalysisEngine (纯数学) 和 ChartCursorOverlay (纯渲染) 可以并行开发，互不依赖。

4. **B 和 C 可以在波形引擎的间隙插入。** Modbus 模板 (400 行/2 迭代) 和虚拟回环 (550 行/2 迭代) 都是小体量特性。如果波形引擎某个迭代出现架构问题需要额外迭代修复，可以在 #55 或 #56 之间穿插一个 Modbus 模板迭代来保持交付节奏。

### 4.3 建议的实施序列

```
#54: ChartAnalysisEngine -- FFT + 直方统计算法 (纯数学，无 UI)
#55: ChartCursorOverlay + ChartInteractionController -- 游标测量 + 缩放平移
#56: MultiAxisManager + ChartWidget 集成 -- 多 Y 轴 + 分析模式切换
#57: FFT 频谱渲染 + 直方图渲染 + 模式切换动画 -- 波形引擎完整交付
```

如果 #54-#55 期间发现 P0 Bug，在 #56 之前插入 Bug 修复冲刺。Modbus 模板可在 #58 开始。

---

## 5. 留给评审员的问题

1. **FFT 实现策略**: 是用纯 C++ 手写 Cooley-Tukey radix-2 FFT (~150 行)，还是引入 KissFFT 等轻量库？手写的好处是零外部依赖，缺点是性能和精度可能不如专业库。项目约束 (CLAUDE.md) 没有禁止外部库，但倾向于 Qt + STL。

2. **Qt Charts vs 自绘**: FFT 频谱图用 QBarSeries (柱状图) 还是 QPainter 自绘？QBarSeries 复用 Qt Charts 基础设施但样式受限。QPainter 自绘灵活但代码量大。

3. **Modbus 模板是否与波形引擎并行开发**: 如果团队带宽允许，Modbus 模板 (2 迭代) 可以在波形引擎 #55-#56 期间由协议开发 (角色 #5) 并行完成，不影响波形引擎主线。
