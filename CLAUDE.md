# EmbedDebug - 项目开发约束文档

> 本文档是所有开发行为的最高约束。每次开发前必须完整阅读。
> 修改本文档需要用户审查通过后方可生效。

---

## 一、项目概况

| 项 | 值 |
|----|-----|
| 应用名称 | EmbedDebug |
| 项目路径 | `E:\Embedded\Tool\Serial_tool\User_Serial` |
| 当前版本 | 0.1.0 |
| 评分起点 | 1分 / 目标 1000分 |
| Git分支 | `feat/embed-debug` |
| Git远程 | `https://github.com/ParacosmYy/GS_Tool.git` |

---

## 二、构建环境

| 项目 | 路径/版本 |
|------|-----------|
| Qt 6.8.3 | `E:/Tool/DevEnv/Qt/6.8.3/mingw_64` |
| GCC 14.2.0 | `E:/Tool/DevEnv/x86_64-14.2.0-release-win32-seh-msvcrt-rt_v12-rev2/mingw64/bin` |
| CMake 4.0.1 | `E:/Tool/DevEnv/cmake-4.0.1-windows-x86_64/cmake-4.0.1-windows-x86_64/bin` |
| Ninja 1.13.2 | PATH中 |
| GDB 16.2 | PATH中 |
| J-Link V932 | `E:/Embedded/Tool/SEGGER_IOT/JLink_V932` |

### 构建命令

```bash
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64
cmake --build build
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/windeployqt.exe build/EmbedDebug.exe
```

### VS Code IntelliSense

`.vscode/c_cpp_properties.json` 配置:
- includePath: `${workspaceFolder}/src`, `E:/Tool/DevEnv/Qt/6.8.3/mingw_64/include/**`, GCC标准库头文件
- defines: `UNICODE`, `_UNICODE`, `QT_CORE_LIB`, `QT_GUI_LIB`, `QT_WIDGETS_LIB`, `QT_SERIALPORT_LIB`, `QT_CHARTS_LIB`, `QT_NETWORK_LIB`
- compilerPath: `E:/Tool/DevEnv/.../mingw64/bin/g++.exe`
- intelliSenseMode: `gcc-x64`, cppStandard: `c++17`

---

## 三、开发工作流（强制执行）

### 3.1 迭代流程

```
┌──────────────────────────────────────────────────────────────┐
│  Step 1: PRD需求文档     [产品经理]                            │
│    ↓ 输出: docs/prd/PRD_xxx.md                               │
├──────────────────────────────────────────────────────────────┤
│  Step 2: 需求评审         [系统架构师 + 产品经理]              │
│    ↓ 输出: 架构影响分析、公共组件复用检查                      │
│    ↓ 确认: 无重复造轮子、设计模式正确                         │
├──────────────────────────────────────────────────────────────┤
│  Step 3: 架构设计         [系统架构师]                         │
│    ↓ 输出: 类图、接口定义、设计模式选型                       │
├──────────────────────────────────────────────────────────────┤
│  Step 4: 编码实现         [核心开发 + UI开发 + 协议开发]       │
│    ↓ 可并行: 核心/协议无依赖可同时开发                        │
│    ↓ 约束: 每个commit ≥ 200行代码变更                        │
├──────────────────────────────────────────────────────────────┤
│  Step 5: 集成联调         [集成开发]                           │
│    ↓ 跨模块串联，端到端验证                                   │
├──────────────────────────────────────────────────────────────┤
│  Step 6: 编译验证         [QA工程师]                           │
│    ↓ 约束: 零编译错误才能commit                               │
├──────────────────────────────────────────────────────────────┤
│  Step 7: 代码审查         [代码审查员]                         │
│    ↓ 约束: 设计模式合规、无重复代码、分层正确                  │
├──────────────────────────────────────────────────────────────┤
│  Step 8: Git Commit + Push                                    │
│    ↓ 约束: commit message 中文，说明改了什么和为什么          │
│    ↓ 评分: 本次commit +1分                                    │
└──────────────────────────────────────────────────────────────┘
```

### 3.2 Agent角色分配（8人团队，研发占比62.5%）

| # | 角色 | Agent类型 | 职责 | 类别 |
|---|------|-----------|------|------|
| 1 | **产品经理** | `requirements-analyst` | 编写PRD需求文档，澄清需求边界，验收标准定义 | 管理 |
| 2 | **系统架构师** | `system-architect` | 类图设计、接口定义、设计模式选型、模块依赖关系 | 研发 |
| 3 | **核心开发** | `general-purpose` | 基础设施层开发：IConnection、ConnectionFactory、SettingsManager、工具类 | 研发 |
| 4 | **UI开发** | `frontend-architect` | 表现层开发：TerminalWidget、ConfigPanel、QuickCommandBar、主题、布局 | 研发 |
| 5 | **协议开发** | `general-purpose` | 业务层开发：OTA协议(X/Y/ZMODEM)、帧解析器、Intel HEX解析 | 研发 |
| 6 | **集成开发** | `general-purpose` | 跨层集成：RTT/J-Link桥接、TCP/UDP连接、数据流串联、端到端联调 | 研发 |
| 7 | **代码审查员** | `self-review` | 代码质量审查：设计模式合规、重复代码检测、分层违规检查 | 质量 |
| 8 | **QA工程师** | `quality-engineer` | 编译验证、单元测试、边界条件测试、性能测试 | 质量 |

**并行策略**:
- Step 4 编码阶段，角色3~6可同时启动，各自负责不同模块
- 角色3(核心)和角色5(协议)无依赖，可完全并行
- 角色4(UI)依赖角色3(核心)的接口定义，需等接口稳定后启动
- 角色6(集成)需等其他模块完成后串联
- 角色7(审查)和角色8(QA)在编码完成后串行执行

### 3.3 Commit 规则（铁律）

1. **每次commit必须 ≥ 200行代码变更**（不含空行和注释）
2. **每次commit = +1分**
3. **零编译错误才能commit** — 编译不过必须先修
4. **commit message格式**:

```
<模块名>: <简述改了什么>

<详细说明为什么这样改，解决了什么问题>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```

5. **禁止的行为**:
   - 禁止提交编译不过的代码
   - 禁止重复造轮子（公共组件只写一次）
   - 禁止不经PRD直接写代码
   - 禁止不经架构审查直接加新类

---

## 四、架构原则

### 4.1 设计模式要求

所有模块必须遵循以下设计模式:

| 模式 | 应用场景 | 项目中的使用 |
|------|---------|-------------|
| **策略模式 (Strategy)** | OTA协议切换 (XMODEM/YMODEM/ZMODEM) | `IProtocol` 接口 + 具体协议实现 |
| **观察者模式 (Observer)** | 数据流分发 | Qt 信号/槽机制 |
| **工厂模式 (Factory)** | 创建不同类型的连接 | `ConnectionFactory` |
| **状态模式 (State)** | 连接状态管理 | `ConnectionState` 枚举驱动行为 |
| **单例模式 (Singleton)** | 全局管理器 | `SettingsManager`, `ThemeManager` |
| **模板方法 (Template Method)** | OTA传输流程 | `BaseTransfer::execute()` 骨架 |
| **适配器模式 (Adapter)** | J-Link SDK适配 | `JLinkBridge` 适配 `IConnection` |
| **命令模式 (Command)** | 快捷指令、发送历史 | `QuickCommand` 数据结构 |

### 4.2 分层架构

```
┌─────────────────────────────────┐
│        表现层 (Presentation)     │  QWidget / QPainter
│  MainWindow, ConfigPanel, View  │  只做UI展示，不含业务逻辑
├─────────────────────────────────┤
│        业务层 (Business)        │  QObject
│  ConnectionMgr, OtaManager,    │  业务逻辑编排，协调各模块
│  ProtocolEngine, ChartManager  │
├─────────────────────────────────┤
│        数据层 (Data)            │  QObject / 纯C++
│  TerminalModel, FrameParser,   │  数据模型、解析、计算
│  CRC, RingBuffer, DataLogger   │
├─────────────────────────────────┤
│        基础设施层 (Infra)       │  纯C++ / Qt底层封装
│  IConnection, SerialConnection │  硬件/OS抽象、IO操作
│  JLinkBridge, SettingsManager │
└─────────────────────────────────┘
```

### 4.3 依赖规则（单向，不可反向）

```
表现层 → 业务层 → 数据层 → 基础设施层
```

- 表现层可以依赖业务层和数据层
- 业务层可以依赖数据层和基础设施层
- **基础设施层不能依赖任何上层** — 它是最底层的独立模块
- **数据层不能依赖表现层** — 模型不知道谁在显示它

### 4.4 公共组件清单（只写一次，全局复用）

| 组件 | 文件 | 用途 |
|------|------|------|
| `CRC` | `utils/CRC.h` | CRC8/CRC16-CCITT/CRC16-Modbus/CRC32/checksum |
| `HexConverter` | `utils/HexConverter.h` | HEX编码/解码/校验 |
| `RingBuffer<T>` | `utils/RingBuffer.h` | 线程安全环形缓冲区模板 |
| `SettingsManager` | `utils/SettingsManager.h/cpp` | 单例，配置持久化 |
| `ThemeManager` | `core/ThemeManager.h/cpp` | 单例，主题切换 |
| `DataLogger` | `utils/DataLogger.h/cpp` | 日志记录/回放 |
| `IConnection` | `connection/IConnection.h` | 连接抽象接口 |
| `TerminalModel` | `terminal/TerminalModel.h/cpp` | 终端数据模型 |
| `TerminalWidget` | `terminal/TerminalWidget.h/cpp` | 自绘制终端控件 |
| `Constants` | `core/Constants.h` | 全局枚举和常量 |

**规则**: 上表中的组件已经过验证，任何新功能需要CRC/HEX/缓冲区/配置等能力时，直接复用，不得重写。

### 4.5 新增类的检查清单

在创建任何新类之前，必须确认:
- [ ] 是否有现有的公共组件可以复用？
- [ ] 这个类属于哪一层（表现/业务/数据/基础设施）？
- [ ] 它的依赖是否满足单向规则（不反向依赖）？
- [ ] 是否需要新的设计模式？如果需要，在PRD中说明
- [ ] 接口是否足够抽象，方便未来扩展？

---

## 五、编码规范

### 5.1 语言和风格

- C++17 标准
- 详细中文注释（开发者是C++/Qt新手）
- 头文件引用使用相对src目录的路径: `#include "core/Constants.h"`
- Qt信号/槽用新式connect语法（函数指针），**禁止** SIGNAL/SLOT 宏
- 每个类一对 .h/.cpp 文件，放在对应子目录中

### 5.2 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `SerialConnection` |
| 方法 | camelCase | `setBaudRate()` |
| 成员变量 | m_ 前缀 + camelCase | `m_portName` |
| 常量 | k 前缀 + PascalCase | `kMaxBufferSize` |
| 枚举值 | PascalCase | `ConnectionState::Connected` |
| 文件名 | PascalCase.h/cpp | `SerialConnection.h` |
| 头文件卫士 | 全大写 | `#ifndef SERIAL_CONNECTION_H` |
| 命名空间 | camelCase | `namespace hexConvert` |
| 宏 | UPPER_SNAKE_CASE | `#define EMBEDDEBUG_VERSION` |

### 5.3 头文件规则

```cpp
#ifndef NAMESPACE_CLASS_NAME_H     // 头文件卫士
#define NAMESPACE_CLASS_NAME_H

#include <Qt先>                     // Qt头文件在前
#include <STL次>                     // STL头文件次之
#include "项目头文件最后"            // 项目头文件最后

class ClassName : public QObject {  // 继承用public
    Q_OBJECT

public:                             // 访问修饰符缩进1格
    explicit ClassName(QObject* parent = nullptr);
    ~ClassName() override;

    // 禁止拷贝和赋值（QObject派生类）
    ClassName(const ClassName&) = delete;
    ClassName& operator=(const ClassName&) = delete;

signals:
    void dataReady(const QByteArray& data);

private slots:
    void onInternalEvent();

private:
    // 成员变量声明区（底部）
    QString m_memberVar;
};
```

### 5.4 内存管理

- QObject父子树管理生命周期，优先用 `new Xxx(parent)`
- 非QObject对象用 `std::unique_ptr` / `std::shared_ptr`
- **禁止裸 `new` 不配对 `delete`** — 必须有明确的拥有者
- 大缓冲区用 `QByteArray` 或 `std::vector`，不要手动 `malloc`

---

## 六、PRD文档规范

### 6.1 文件位置

```
docs/prd/PRD_<编号>_<简述>.md
```

### 6.2 PRD模板

```markdown
# PRD-<编号>: <功能名称>

## 背景
<为什么需要这个功能，解决什么问题>

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ...     | P0/P1/P2 | core/serial |

## 接口设计
<新增的类、方法、信号>

## 依赖的公共组件
<列出复用的现有组件>

## 设计模式
<使用了什么设计模式，为什么>

## 影响范围
<修改了哪些文件，对现有功能的影响>

## 验收标准
<怎样算完成>
```

---

## 七、Git规范

- 分支: `feat/embed-debug`
- 远程: `https://github.com/ParacosmYy/GS_Tool.git`
- commit message 用中文
- 不提交 build/ 目录
- `.vscode/settings.json` 和 `c_cpp_properties.json` 需要提交
- **每次commit前**:
  1. `cmake --build build` 确认零错误
  2. `git diff --stat` 确认 ≥ 200行变更
  3. commit message 遵循第三章格式

---

## 八、评分追踪

| # | Commit内容 | 分数 |
|---|-----------|------|
| 0 | 项目初始状态 | 1 |
| 1 | (待填充) | 2 |
| 2 | (待填充) | 3 |
| ... | 目标: 1000分 | 1000 |

---

## 九、项目目录结构

```
User_Serial/
├── CMakeLists.txt
├── CLAUDE.md                    # 本约束文档
├── README.md                    # 企业级项目文档
├── EmbedDebug.bat               # 双击启动脚本
├── docs/
│   └── prd/                     # PRD需求文档目录
│       └── PRD_xxx.md
├── src/
│   ├── main.cpp
│   ├── core/                    # 应用核心层
│   │   ├── MainWindow.h/cpp
│   │   ├── ConnectionManager.h/cpp
│   │   ├── ConnectionFactory.h/cpp   # 工厂模式
│   │   ├── ThemeManager.h/cpp
│   │   └── Constants.h
│   ├── connection/              # 基础设施层: 连接抽象
│   │   ├── IConnection.h             # 策略接口
│   │   ├── SerialConnection.h/cpp
│   │   ├── TcpConnection.h/cpp
│   │   ├── UdpConnection.h/cpp
│   │   └── RttConnection.h/cpp
│   ├── terminal/                # 表现层: 终端显示
│   │   ├── TerminalWidget.h/cpp      # 自绘制控件
│   │   ├── TerminalModel.h/cpp       # 数据模型
│   │   └── TerminalSearchBar.h/cpp
│   ├── serial/                  # 表现层: 串口功能UI
│   │   ├── SerialConfigPanel.h/cpp
│   │   ├── QuickCommandBar.h/cpp
│   │   ├── TimedSender.h/cpp
│   │   ├── SendHistory.h/cpp
│   │   └── DataStatistics.h/cpp
│   ├── protocol/                # 业务层: 协议解析
│   │   ├── IProtocol.h               # 策略接口
│   │   ├── FrameDefinition.h/cpp
│   │   ├── FrameVisualEditor.h/cpp
│   │   ├── FrameParser.h/cpp         # 状态机
│   │   ├── ProtocolView.h/cpp
│   │   └── IntelHexParser.h/cpp
│   ├── chart/                   # 表现层: 波形显示
│   │   ├── ChartWidget.h/cpp
│   │   ├── ChannelConfig.h/cpp
│   │   └── ChartModel.h/cpp
│   ├── ota/                     # 业务层: OTA升级
│   │   ├── OtaManager.h/cpp
│   │   ├── OtaWidget.h/cpp
│   │   ├── OtaHistoryModel.h/cpp
│   │   └── protocols/
│   │       ├── IProtocol.h           # 策略接口(策略模式)
│   │       ├── BaseTransfer.h/cpp    # 模板方法模式
│   │       ├── XModemTransfer.h/cpp
│   │       ├── YModemTransfer.h/cpp
│   │       └── ZModemTransfer.h/cpp
│   ├── rtt/                     # 基础设施层: RTT
│   │   ├── JLinkBridge.h/cpp         # 适配器模式
│   │   ├── RttChannelModel.h/cpp
│   │   ├── RttViewer.h/cpp
│   │   └── RttConfigPanel.h/cpp
│   └── utils/                   # 基础设施层: 公共工具
│       ├── RingBuffer.h               # 模板
│       ├── DataLogger.h/cpp
│       ├── DataExporter.h/cpp
│       ├── SettingsManager.h/cpp      # 单例
│       ├── HexConverter.h             # 纯函数
│       └── CRC.h                      # 纯函数
├── resources/
│   ├── icons/
│   ├── themes/
│   │   ├── dark_terminal.qss
│   │   ├── modern_dark.qss
│   │   └── light.qss
│   ├── translations/
│   └── app.qrc
└── tests/
    └── ...
```
