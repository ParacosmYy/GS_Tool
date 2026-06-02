# EmbedDebug 后续特性路线图 (PRD-061)

> 状态: 规划中 | 优先级排序 | 基于当前架构能力 + 竞品对标分析
> 前置: 架构重构已完成(40+子模块), 波形引擎Phase 1~3已交付

---

## 目录

1. [特性总览与优先级矩阵](#一特性总览与优先级矩阵)
2. [F1: 数据录制回放增强](#f1-数据录制回放增强)
3. [F2: 自定义协议脚本引擎](#f2-自定义协议脚本引擎)
4. [F3: 多通道数据导出增强](#f3-多通道数据导出增强)
5. [F4: 终端增强 — 分屏 + 标签页](#f4-终端增强--分屏--标签页)
6. [F5: 仪表盘模式 (Gauge Dashboard)](#f5-仪表盘模式-gauge-dashboard)
7. [F6: J-Link RTT 深度集成](#f6-j-link-rtt-深度集成)
8. [F7: 数据流触发器与自动化](#f7-数据流触发器与自动化)
9. [F8: 工程会话管理](#f8-工程会话管理)
9. [F9: 串口高级调试](#f9-串口高级调试)
10. [F10: 性能监控与分析](#f10-性能监控与分析)
11. [F11: 插件/扩展系统](#f11-插件扩展系统)
12. [依赖关系图](#依赖关系图)
13. [实施建议与里程碑](#实施建议与里程碑)

---

## 一、特性总览与优先级矩阵

| ID | 特性名称 | 优先级 | 预估代码量 | 依赖 | 竞品参考 |
|----|---------|--------|-----------|------|---------|
| F1 | 数据录制回放增强 | **P0** | ~800行 | Waveform Phase 4 | VOFA+ 录制回放 |
| F2 | 自定义协议脚本引擎 | **P0** | ~1200行 | 无 | Serial Studio JSON协议 |
| F3 | 多通道数据导出增强 | **P1** | ~500行 | F1 | Docklight CSV/Excel |
| F4 | 终端分屏 + 标签页 | **P1** | ~600行 | 无 | MobaXterm/Tera Term |
| F5 | 仪表盘模式 | **P1** | ~1000行 | Waveform Phase 4 | Serial Studio 仪表盘 |
| F6 | J-Link RTT 深度集成 | **P1** | ~800行 | 无 | J-Link RTT Viewer |
| F7 | 数据流触发器与自动化 | **P2** | ~700行 | 无 | Docklight 触发器 |
| F8 | 工程会话管理 | **P2** | ~500行 | F8 | VS Code Workspace |
| F9 | 串口高级调试 | **P2** | ~600行 | 无 | Docklight/Serial Port Monitor |
| F10 | 性能监控与分析 | **P2** | ~400行 | F1 | Serial Studio 统计 |
| F11 | 插件/扩展系统 | **P3** | ~1500行 | F2 | VS Code Extension |

### 优先级定义

- **P0**: 核心差异化能力，直接提升调试效率，近期交付
- **P1**: 完善工具链，填补功能空白，中近期交付
- **P2**: 高级功能，提升专业度和效率，中期交付
- **P3**: 远期架构升级，依赖前面特性的经验积累

---

## F1: 数据录制回放增强

> 优先级: P0 | 模块: `core/recording/`, `utils/log/` | 预估: ~800行

### 动机
当前 `RecordingController` 只有基础录制(开始/停止/保存)和简单回放。对比 VOFA+ 的录制回放体验，缺失：
- **多流时间对齐**: 波形通道数据与终端文本数据不在同一时间轴
- **变速回放**: 无法加速/减速/逐帧回放
- **回放中的导航**: 无法跳转到特定时间点或数据事件
- **录制标注**: 无法在录制过程中标记关键时刻

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F1-R1 | 统一时间轴: 所有数据流(终端、波形、协议)共用毫秒时间戳基准 | P0 |
| F1-R2 | 录制文件格式升级: 二进制格式，包含元数据(采样率/通道/协议)，向后兼容旧CSV | P0 |
| F1-R3 | 变速回放: 0.25x/0.5x/1x/2x/4x/8x 速度，支持暂停后逐帧步进 | P0 |
| F1-R4 | 回放时间轴导航: 进度条拖拽 + 时间输入跳转 + 事件标记跳转 | P1 |
| F1-R5 | 录制标注: 快捷键添加文本标记，标记列表侧栏，点击跳转 | P1 |
| F1-R6 | 录制文件预览: 加载文件后显示时长/通道数/数据量摘要，再决定回放 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `RecordingTimeline` | `core/recording/RecordingTimeline.h/cpp` | 数据层 | 时间轴管理，统一时间戳，事件索引 |
| `PlaybackController` | `core/recording/PlaybackController.h/cpp` | 业务层 | 回放控制(播放/暂停/变速/跳转) |
| `RecordingFileFormat` | `utils/log/RecordingFileFormat.h/cpp` | 数据层 | 二进制录制文件的读写 |
| `RecordingMarker` | `core/recording/RecordingMarker.h/cpp` | 数据层 | 标记数据结构和管理 |
| `PlaybackWidget` | `core/recording/PlaybackWidget.h/cpp` | 表现层 | 回放控制UI(进度条/速度/标记) |

### 设计要点
- `RecordingTimeline` 维护一个全局时钟，所有数据写入时附带这个时钟的时间戳
- `PlaybackController` 是 `RecordingController` 的兄弟，共享 `RecordingTimeline`
- 录制文件: 头部魔数 + 版本 + 元数据JSON + 分chunk的数据块，每个chunk有时间戳偏移量
- 向后兼容: 检测文件头，若无魔数则按旧CSV格式解析

### 验收标准
1. 录制包含波形+终端的混合数据，回放时两者时间同步
2. 变速回放平滑无数据丢失
3. 标记可添加、可查看列表、可跳转
4. 旧格式CSV文件仍可加载
5. 所有新增文件 ≤ 500行/文件, ≤ 80行/方法

---

## F2: 自定义协议脚本引擎

> 优先级: P0 | 模块: `protocol/` | 预估: ~1200行

### 动机
当前 `protocol/parser/FrameParser` 只支持固定帧格式(头+字段+尾)。嵌入式调试中协议千变万化:
- Modbus RTU 的 CRC + 功能码 + 寄存器地址
- 自定义 TLV (Type-Length-Value) 协议
- COBS 编码协议
- SLIP 包装协议

参考 Serial Studio 的 JSON 协议定义，需要让用户用简单的方式描述自己的协议，而不是修改C++代码。

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F2-R1 | 协议定义JSON格式: 支持"帧头匹配/长度字段/校验和/CRC"的组合规则 | P0 |
| F2-R2 | 内置协议模板: Modbus RTU, JustFloat, FireWater, COBS, SLIP, 自定义TLV | P0 |
| F2-R3 | 协议定义可视化编辑器: 在现有FrameVisualEditor基础上扩展 | P1 |
| F2-R4 | 协议字段映射到通道: 协议解析后的字段可映射到波形通道/终端显示 | P0 |
| F2-R5 | 实时协议验证: 连接时自动验证收到的数据是否匹配协议定义 | P1 |
| F2-R6 | 协议模板导入/导出: JSON文件，可分享给其他开发者 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `ProtocolSchema` | `protocol/schema/ProtocolSchema.h/cpp` | 数据层 | 协议定义的数据模型(JSON加载/校验) |
| `ProtocolEngine` | `protocol/engine/ProtocolEngine.h/cpp` | 业务层 | 根据ProtocolSchema解析字节流 |
| `ProtocolTemplateLibrary` | `protocol/schema/ProtocolTemplateLibrary.h/cpp` | 数据层 | 内置协议模板的管理和加载 |
| `ProtocolSchemaEditor` | `protocol/editor/ProtocolSchemaEditor.h/cpp` | 表现层 | JSON协议定义的表单式编辑器 |
| `ProtocolFieldMapper` | `protocol/engine/ProtocolFieldMapper.h/cpp` | 数据层 | 协议字段→波形通道的映射规则 |

### 协议定义JSON示例
```json
{
  "name": "Modbus RTU",
  "framing": {
    "type": "length_field",
    "header": [null],
    "length_field": { "offset": 2, "size": 1, "adjustment": 2 },
    "checksum": { "type": "crc16_modbus", "offset": -2, "size": 2 }
  },
  "fields": [
    { "name": "slave_address", "offset": 0, "size": 1, "type": "uint8" },
    { "name": "function_code", "offset": 1, "size": 1, "type": "uint8" },
    { "name": "data", "offset": 3, "size": "dynamic", "type": "bytes" }
  ],
  "channel_mapping": [
    { "field": "data", "decode": "float32_le", "channels": ["ch1","ch2"] }
  ]
}
```

### 设计要点
- `ProtocolSchema` 是纯数据模型，从JSON加载后不可变
- `ProtocolEngine` 替代 `FrameParser`，但保留 `IProtocolBridge` 接口兼容
- 现有 `JustFloatBridge` / `FireWaterBridge` 重构为 `ProtocolSchema` 的模板实例
- `ProtocolFieldMapper` 输出到 `ChartModel`，复用现有波形基础设施

### 验收标准
1. 用户可从JSON文件加载协议定义并实时解析数据
2. 内置模板(Modbus/COBS/SLIP)开箱即用
3. 协议字段可映射到波形通道并实时显示
4. 现有 JustFloat/FireWater 协议仍然正常工作
5. 所有新增文件 ≤ 500行/文件

---

## F3: 多通道数据导出增强

> 优先级: P1 | 模块: `utils/export/`, `chart/model/` | 预估: ~500行

### 动机
当前 `DataExporter` 只导出终端数据。波形通道数据无法导出。嵌入式开发者经常需要:
- 将波形数据导出为 CSV 供 Python/MATLAB 分析
- 导出为 Excel (带通道名/单位/时间戳列)
- 导出截图 (PNG/SVG) 用于报告

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F3-R1 | 波形CSV导出: 时间列 + 每个通道一列，含表头(通道名+单位) | P0 |
| F3-R2 | 波形Excel导出: .xlsx格式，带格式化(列宽/颜色/冻结首行) | P1 |
| F3-R3 | 波形截图导出: PNG/SVG，含图例和通道信息水印 | P1 |
| F3-R4 | 批量导出: 选择时间范围 + 选择通道 | P1 |
| F3-R5 | 剪贴板复制: 选中区域的数值直接Ctrl+C复制为TSV | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `ChartExporter` | `utils/export/ChartExporter.h/cpp` | 数据层 | 波形数据导出(CSV/XLSX/PNG) |
| `ExportDialog` | `utils/export/ExportDialog.h/cpp` | 表现层 | 导出选项对话框(格式/范围/通道选择) |

### 设计要点
- CSV导出用 Qt 原生，Excel导出考虑 QtXlsxWriter 或轻量级xlsx库
- 截图导出: `ChartWidget::grab()` → `QPixmap::save()`
- SVG导出: `QSvgGenerator` 渲染 QChart
- `ExportDialog` 是通用对话框，终端导出和波形导出共用

### 验收标准
1. CSV导出后用 Excel/Python 可直接打开，列名清晰
2. 截图导出包含图例，分辨率≥2x屏幕DPI
3. 时间范围选择精确到毫秒

---

## F4: 终端增强 — 分屏 + 标签页

> 优先级: P1 | 模块: `terminal/`, `core/panels/` | 预估: ~600行

### 动机
嵌入式调试中经常需要同时观察多个数据源:
- 上位机命令和设备响应分开显示
- 不同串口的数据对比
- HEX视图和文本视图并排
- 参考MobaXterm的标签页终端体验

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F4-R1 | 水平/垂直分屏: 两个终端视图并排或上下，共享或独立数据源 | P0 |
| F4-R2 | 标签页: 多个终端Tab，每个Tab可连接不同数据源 | P0 |
| F4-R3 | HEX并排视图: 同一数据同时显示文本和HEX | P1 |
| F4-R4 | 分屏/标签页状态保存到会话 | P2 |
| F4-R5 | 拖拽重排标签页 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `TerminalSplitter` | `terminal/layout/TerminalSplitter.h/cpp` | 表现层 | 分屏容器(QSplitter)，管理多个TerminalWidget |
| `TerminalTabManager` | `terminal/layout/TerminalTabManager.h/cpp` | 表现层 | 标签页管理(QTabWidget)，Tab增删切换 |
| `TerminalDualView` | `terminal/layout/TerminalDualView.h/cpp` | 表现层 | 文本+HEX并排视图，共享滚动同步 |

### 设计要点
- `TerminalSplitter` 包装 `QSplitter`，每个Section是一个独立的 `TerminalWidget`
- 数据源共享: 多个TerminalWidget可以连接同一个 `TerminalModel` (只读镜像)
- 独立数据源: 各TerminalWidget连接不同 `TerminalModel` (不同串口)
- `TerminalDualView` 使用 `QSplitter` 水平分割，左侧文本右侧HEX，滚动位置同步

### 验收标准
1. 分屏可水平/垂直切换，拖拽分隔线调整比例
2. 每个分屏区域可独立设置数据源
3. HEX并排视图同步滚动
4. 标签页可新增/关闭，状态保存到会话

---

## F5: 仪表盘模式 (Gauge Dashboard)

> 优先级: P1 | 模块: 新增 `dashboard/` | 预估: ~1000行

### 动机
参考 Serial Studio 的仪表盘，嵌入式开发者经常需要直观监控关键数值:
- 电机转速、温度、电池电压 → 仪表盘/进度条
- 传感器姿态角 → 姿态球
- 系统状态 → LED指示灯

当前只有折线图，缺少"实时数值监控"的展示方式。

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F5-R1 | 仪表盘布局: 自由拖拽排列组件的网格布局 | P0 |
| F5-R2 | 组件类型: 数值显示、仪表盘(Gauge)、进度条、LED指示灯、迷你折线图 | P0 |
| F5-R3 | 组件绑定通道: 每个组件绑定一个或多个波形通道 | P0 |
| F5-R4 | 阈值告警: 设置上下限，超限时组件变色+通知 | P1 |
| F5-R5 | 仪表盘模板: 预设常用布局(电机监控/传感器/电池)，可自定义保存 | P1 |
| F5-R6 | 仪表盘全屏模式: F11全屏显示，适合远距离监控 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `DashboardWidget` | `dashboard/DashboardWidget.h/cpp` | 表现层 | 仪表盘主容器，网格布局管理 |
| `GaugeWidget` | `dashboard/GaugeWidget.h/cpp` | 表现层 | 圆形仪表盘组件(QPainter自绘) |
| `ProgressBarWidget` | `dashboard/ProgressBarWidget.h/cpp` | 表现层 | 数值进度条组件 |
| `LedIndicatorWidget` | `dashboard/LedIndicatorWidget.h/cpp` | 表现层 | LED指示灯组件 |
| `NumericDisplayWidget` | `dashboard/NumericDisplayWidget.h/cpp` | 表现层 | 大字号数值显示组件 |
| `DashboardModel` | `dashboard/DashboardModel.h/cpp` | 数据层 | 仪表盘布局和组件配置的数据模型 |

### 设计要点
- 所有组件继承自 `BaseDashboardComponent` (QWidget)，定义统一接口: `setValue(double)`, `setRange(min,max)`, `bindChannel(name)`
- `DashboardWidget` 使用 `QGridLayout`，支持拖拽调整位置
- 组件全部QPainter自绘，不依赖外部SVG资源
- 配置保存为JSON，包含组件类型/位置/绑定通道/阈值
- 主题适配: 所有颜色从 `ThemeManager::SemanticColor` 获取

### 验收标准
1. 至少4种组件类型可添加到仪表盘
2. 拖拽排列流畅
3. 绑定通道后数值实时更新
4. 主题切换时所有组件正确变色
5. 配置可保存和加载

---

## F6: J-Link RTT 深度集成

> 优先级: P1 | 模块: `connection/`, 新增 `rtt/` | 预估: ~800行

### 动机
当前 `IConnection` 接口已预留 J-Link RTT 支持，但 `src/rtt/` 是空目录(已清理)。RTT是嵌入式调试的核心能力:
- 零干扰调试输出(不占UART/不影响时序)
- 多通道RTT (Channel 0=调试输出, Channel 1=数据流)
- 上行(目标→PC)和下行(PC→目标)双向通信

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F6-R1 | J-Link SDK动态加载: 运行时LoadLibrary，SDK不存在时优雅降级 | P0 |
| F6-R2 | RTT连接实现: 实现 `IConnection` 接口，支持通道选择 | P0 |
| F6-R3 | RTT配置面板: 设备选择(STM32F4等)/接口(SWD/JTAG)/速度/RTT通道 | P0 |
| F6-R4 | RTT数据流映射: Channel 0→终端, Channel 1+→波形通道 | P1 |
| F6-R5 | RTT状态监控: 连接状态/缓冲区使用率/吞吐量显示 | P1 |
| F6-R6 | 多目标支持: 下拉列表选择已连接的调试探针 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `JLinkRttConnection` | `rtt/JLinkRttConnection.h/cpp` | 基础设施层 | RTT连接，实现IConnection接口 |
| `JLinkSdkLoader` | `rtt/JLinkSdkLoader.h/cpp` | 基础设施层 | J-Link SDK动态加载和函数指针管理 |
| `RttConfigPanel` | `rtt/RttConfigPanel.h/cpp` | 表现层 | RTT参数配置UI |
| `RttChannelManager` | `rtt/RttChannelManager.h/cpp` | 业务层 | RTT多通道管理和数据路由 |

### 设计要点
- `JLinkSdkLoader` 使用 `QLibrary` 动态加载 `JLink_x64.dll`，导出函数指针
- 加载失败时: RttConfigPanel显示"请安装J-Link Software"，不影响其他连接类型
- `JLinkRttConnection` 实现 `IConnection` 的 open/close/send/receive
- `RttChannelManager` 负责将 RTT Channel N 的数据路由到 `TerminalModel`(文本) 或 `ChartModel`(波形)

### 验收标准
1. J-Link未安装时应用正常启动，RTT选项显示提示
2. J-Link连接后可通过RTT接收数据并在终端显示
3. RTT通道可映射到波形通道
4. 断开J-Link后应用不崩溃

---

## F7: 数据流触发器与自动化

> 优先级: P2 | 模块: 新增 `automation/` | 预估: ~700行

### 动机
参考 Docklight 的触发器功能，嵌入式调试经常需要:
- "当收到特定字符串时自动发送响应" (模拟设备)
- "当波形通道超过阈值时自动记录数据" (异常捕获)
- "当连续N次未收到心跳时触发告警" (看门狗监控)
- "每隔100ms发送一次查询命令" (轮询模式，TimedSender已部分支持)

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F7-R1 | 触发器定义: 条件(匹配模式/阈值/超时) + 动作(发送/记录/通知/脚本) | P0 |
| F7-R2 | 匹配模式: 精确字符串/正则/HEX字节序列/数值范围 | P0 |
| F7-R3 | 动作类型: 发送数据/开始录制/停止录制/Toast通知/播放声音 | P0 |
| F7-R4 | 触发器列表UI: 添加/编辑/删除/启用/禁用触发器 | P1 |
| F7-R5 | 触发器统计: 触发次数/最后触发时间 | P2 |
| F7-R6 | 条件组合: AND/OR逻辑组合多个条件 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `TriggerRule` | `automation/TriggerRule.h/cpp` | 数据层 | 触发规则数据模型(条件+动作) |
| `TriggerEngine` | `automation/TriggerEngine.h/cpp` | 业务层 | 规则评估引擎，监听数据流 |
| `TriggerAction` | `automation/TriggerAction.h/cpp` | 数据层 | 动作执行器(发送/记录/通知) |
| `TriggerManager` | `automation/TriggerManager.h/cpp` | 业务层 | 触发器生命周期管理 |
| `TriggerListPanel` | `automation/TriggerListPanel.h/cpp` | 表现层 | 触发器列表和编辑UI |

### 设计要点
- `TriggerEngine` 监听 `TerminalModel` 的 `dataReceived` 信号和 `ChartModel` 的 `dataAdded` 信号
- 匹配在数据层完成，避免阻塞UI线程
- 规则保存为JSON，与会话一起持久化
- 动作执行通过信号/槽异步触发

### 验收标准
1. 可创建"收到OK时发送AT+RESET"的触发器并正确触发
2. 可创建"通道值>阈值时Toast通知"的触发器
3. 触发器可启用/禁用
4. 规则可保存到会话

---

## F8: 工程会话管理

> 优先级: P2 | 模块: `core/settings/` | 预估: ~500行

### 动机
当前 `SessionManager` 保存基础窗口状态。嵌入式开发者经常在多个项目间切换:
- 项目A: STM32F4 + UART + JustFloat协议 + 4通道波形
- 项目B: ESP32 + TCP + 自定义协议 + OTA升级
- 项目C: RK3568 + J-Link RTT + Modbus

需要"工程"概念，一键切换所有配置。

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F8-R1 | 工程概念: 包含连接配置/协议/波形通道/快捷指令/终端设置/触发器的完整快照 | P0 |
| F8-R2 | 工程切换: 下拉菜单选择工程，一键切换所有配置 | P0 |
| F8-R3 | 工程文件: 保存为 `.edproj` JSON文件，可分享 | P1 |
| F8-R4 | 最近工程列表: 启动时显示最近打开的工程 | P1 |
| F8-R5 | 工程模板: 内置STM32/ESP32/RK3588等常见平台模板 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `Project` | `core/settings/Project.h/cpp` | 数据层 | 工程数据模型(所有配置的聚合) |
| `ProjectManager` | `core/settings/ProjectManager.h/cpp` | 业务层 | 工程生命周期管理(加载/保存/切换) |
| `ProjectWelcomeDialog` | `core/settings/ProjectWelcomeDialog.h/cpp` | 表现层 | 启动欢迎页(最近工程/新建/模板) |

### 设计要点
- `.edproj` 是JSON文件，包含: 连接配置/协议定义/波形通道/快捷指令/触发器/窗口布局
- `ProjectManager` 协调各子Manager导入/导出配置
- 启动时若有默认工程则直接加载，否则显示欢迎页
- 模板是预设的 `.edproj` 文件

### 验收标准
1. 可保存当前所有配置为工程
2. 切换工程后所有配置正确恢复
3. 工程文件可复制到其他电脑使用

---

## F9: 串口高级调试

> 优先级: P2 | 模块: `serial/` | 预估: ~600行

### 动机
嵌入式串口调试不仅限于收发数据，还需要:
- 监控串口信号线状态 (RTS/CTS/DTR/DSR/DCD/RI)
- 精确的发送时序控制 (字节间延迟/帧间延迟)
- 线路噪音和信号质量评估
- 串口流量监控 (发送/接收速率曲线)

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F9-R1 | 信号线状态显示: RTS/CTS/DTR/DSR/DCD/RI 6条线的实时状态指示灯 | P0 |
| F9-R2 | 信号线手动控制: RTS/DTR 开关按钮 | P0 |
| F9-R3 | 发送时序控制: 字节间延迟(ms)/帧间延迟(ms)设置 | P1 |
| F9-R4 | 流量监控: 实时收发速率曲线图 (bytes/s) | P1 |
| F9-R5 | 串口错误统计面板: Framing/Parity/Overrun错误计数+曲线 | P2 |
| F9-R6 | 自定义波特率: 任意输入波特率值(不限于预设列表) | P1 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `SignalLineMonitor` | `serial/signals/SignalLineMonitor.h/cpp` | 数据层 | 串口信号线状态轮询 |
| `SignalLineWidget` | `serial/signals/SignalLineWidget.h/cpp` | 表现层 | 信号线状态指示灯和控制 |
| `TrafficMonitor` | `serial/data/TrafficMonitor.h/cpp` | 数据层 | 流量统计和速率计算 |
| `TrafficMonitorWidget` | `serial/data/TrafficMonitorWidget.h/cpp` | 表现层 | 收发速率曲线显示 |

### 设计要点
- `SignalLineMonitor` 使用 `QSerialPort::pinoutSignals()` 轮询
- `TrafficMonitor` 作为 `IConnection` 的中间层，统计所有经过的数据量
- 速率计算: 每秒采样一次累计字节数，维护最近60秒的滑动窗口

### 验收标准
1. 6条信号线状态实时更新(≤100ms延迟)
2. RTS/DTR开关可控
3. 流量曲线平滑显示
4. 自定义波特率可输入任意值

---

## F10: 性能监控与分析

> 优先级: P2 | 模块: `utils/` | 预估: ~400行

### 动机
EmbedDebug 自身在处理高频数据时也需要性能监控:
- 终端渲染帧率
- 波形刷新延迟
- 数据解析吞吐量
- 内存使用趋势

帮助用户判断性能瓶颈是在应用侧还是设备侧。

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F10-R1 | 性能统计面板: CPU/内存/FPS/延迟显示 | P1 |
| F10-R2 | 数据管道延迟: 从接收到渲染的端到端延迟测量 | P1 |
| F10-R3 | 吞吐量统计: 数据接收/解析/渲染各阶段的速率 | P0 |
| F10-R4 | 性能日志: 可选的性能数据记录到文件 | P2 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `PerformanceMonitor` | `utils/perf/PerformanceMonitor.h/cpp` | 基础设施层 | 性能指标采集和分析 |
| `PerformanceOverlay` | `utils/perf/PerformanceOverlay.h/cpp` | 表现层 | 性能数据悬浮窗(Debug模式) |

### 设计要点
- 使用 `QElapsedTimer` 测量各阶段耗时
- `PerformanceOverlay` 类似游戏FPS计数器，显示在窗口角落
- 可通过设置开关启用，默认关闭

### 验收标准
1. FPS计数器数值准确
2. 数据管道延迟可测量到毫秒精度
3. 性能面板可开关

---

## F11: 插件/扩展系统

> 优先级: P3 | 模块: 新增 `plugin/` | 预估: ~1500行

### 动机
无法预见所有用户需求。插件系统让高级用户(或第三方)扩展EmbedDebug:
- 自定义数据解析插件
- 自定义可视化组件
- 自定义协议支持
- 自动化测试脚本

### 需求列表

| ID | 需求描述 | 优先级 |
|----|---------|--------|
| F11-R1 | 插件接口定义: `IEmbedDebugPlugin` (初始化/数据接入/UI扩展) | P0 |
| F11-R2 | 插件加载器: 扫描plugins/目录，动态加载DLL | P0 |
| F11-R3 | 插件API: 提供终端数据读写/波形通道添加/面板注册的接口 | P0 |
| F11-R4 | Python脚本插件: 通过嵌入Python解释器支持脚本插件 | P2 |
| F11-R5 | 插件市场: 未来可考虑在线插件分发(远期) | P3 |

### 新增类

| 类 | 文件 | 层 | 职责 |
|---|------|-----|------|
| `IEmbedDebugPlugin` | `plugin/IEmbedDebugPlugin.h` | 接口层 | 插件抽象接口 |
| `PluginManager` | `plugin/PluginManager.h/cpp` | 业务层 | 插件加载/卸载/生命周期 |
| `PluginApi` | `plugin/PluginApi.h/cpp` | 业务层 | 暴露给插件的API接口 |
| `PluginConfigPanel` | `plugin/PluginConfigPanel.h/cpp` | 表现层 | 插件管理UI |

### 设计要点
- 插件是独立的DLL/SO，导出 `embedDebugPluginInit(PluginApi*)` 函数
- `PluginApi` 是有限的接口，只暴露必要的操作(注册面板/添加通道/发送数据)
- 安全: 插件在主线程运行，不提供文件系统直接访问
- Python脚本插件通过 `Python.h` 嵌入，提供更安全的沙箱

### 验收标准
1. 示例插件可编译、加载、注册UI面板
2. 插件可读取终端数据和发送数据
3. 损坏的插件不影响主应用启动

---

## 依赖关系图

```
                    ┌─────────────┐
                    │ F11 插件系统 │ (P3 远期)
                    └──────┬──────┘
                           │ 需要稳定的扩展API
                    ┌──────┴──────┐
                    │  F2 协议引擎  │ (P0)
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
       ┌──────┴──────┐    │     ┌──────┴──────┐
       │ F1 录制回放  │    │     │ F7 触发器    │ (P2)
       │   (P0)      │    │     └─────────────┘
       └──────┬──────┘    │
              │           │
       ┌──────┴──────┐    │
       │ F3 数据导出  │    │    ┌──────────────┐
       │   (P1)      │    │    │ F8 工程会话    │ (P2)
       └─────────────┘    │    │ 收集所有配置    │
                          │    └──────┬───────┘
              ┌───────────┤           │
              │           │     ┌─────┴───────┐
       ┌──────┴──────┐   │     │ F5 仪表盘    │ (P1)
       │ F4 终端增强  │   │     └─────────────┘
       │   (P1)      │   │
       └─────────────┘   │
                    ┌────┴──────┐
                    │ F6 RTT    │ (P1)
                    │ J-Link    │
                    └───────────┘

       ┌──────────────┐        ┌──────────────┐
       │ F9 串口高级   │ (P2)  │ F10 性能监控  │ (P2)
       └──────────────┘        └──────────────┘
```

### 依赖说明
- **F1 → F3**: 导出增强依赖录制文件格式(可导出录制的数据)
- **F2 → F11**: 协议引擎为插件系统提供扩展API的基础
- **F8**: 独立但收集所有其他特性的配置(最后实施)
- **F5**: 依赖波形引擎Phase 4完成(需要ChartModel的数据源)
- **F4, F6, F7, F9, F10**: 相互独立，可并行开发

---

## 实施建议与里程碑

### Milestone 1: 核心差异化 (当前 → +2周)
**目标**: 让EmbedDebug在核心调试场景上明显优于VOFA+

| 顺序 | 特性 | 原因 |
|------|------|------|
| 1 | Waveform Phase 4 (直方图/散点) | 波形引擎收尾 |
| 2 | **F1 数据录制回放** | 最高ROI — 调试中最常用的功能 |
| 3 | **F2 自定义协议引擎** | 核心差异化 — 支持任意协议 |

### Milestone 2: 功能完善 (+2周 → +4周)
**目标**: 填补功能空白，接近专业工具水平

| 顺序 | 特性 | 原因 |
|------|------|------|
| 4 | F3 数据导出增强 | 配合F1，数据可导出分析 |
| 5 | F4 终端分屏+标签页 | 多任务调试体验 |
| 6 | F5 仪表盘模式 | 直观的实时监控 |
| 7 | F6 J-Link RTT集成 | 嵌入式专业调试能力 |

### Milestone 3: 专业级工具 (+4周 → +6周)
**目标**: 达到专业级嵌入式调试工具

| 顺序 | 特性 | 原因 |
|------|------|------|
| 8 | F7 触发器与自动化 | 自动化测试场景 |
| 9 | F8 工程会话管理 | 多项目切换效率 |
| 10 | F9 串口高级调试 | 专业串口分析 |
| 11 | F10 性能监控 | 自身优化+用户诊断 |

### Milestone 4: 生态建设 (+6周 → 远期)
**目标**: 建立可扩展的生态

| 顺序 | 特性 | 原因 |
|------|------|------|
| 12 | F11 插件系统 | 需要前面所有特性的API经验 |

### 代码量估算

| Milestone | 新增代码(行) | 新增文件数 |
|-----------|-------------|-----------|
| MS1 | ~2,000 | ~15 |
| MS2 | ~2,900 | ~20 |
| MS3 | ~2,200 | ~18 |
| MS4 | ~1,500 | ~8 |
| **总计** | **~8,600** | **~61** |

---

## 附录: PRD输出计划

每个特性在正式开发前需要输出以下文档:

| 文档 | 内容 | 时机 |
|------|------|------|
| `FEATURE_XXX.md` | 特性总览 + 动机 + 架构设计 + 分阶段计划 | 开发前 |
| `docs/prd/PRD_XXX.md` | 具体需求列表 + 接口设计 + 验收标准 | 每次迭代 |
| 约束文档更新 | 如有新公共组件/设计模式，更新03/07 | 新增类时 |

### 推荐输出顺序

1. **现在**: `FEATURE_DataRecordingReplay.md` (F1详细PRD)
2. **现在**: `FEATURE_ProtocolEngine.md` (F2详细PRD)
3. Phase 4 完成后: `FEATURE_DataExport.md` (F3详细PRD)
4. F1完成后: `FEATURE_TerminalEnhance.md` (F4详细PRD)
5. 以此类推...

---

> **注意**: 本路线图是活文档，随着开发推进和用户反馈持续更新。优先级可根据实际需求调整。
> 特性之间除明确标注的依赖外，均独立可并行开发。
