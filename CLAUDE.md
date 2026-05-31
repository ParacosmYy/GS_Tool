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

### 3.2 Agent角色分配（9人团队，研发占比55.6%，设计占比11.1%）

| # | 角色 | Agent类型 | 职责 | 类别 |
|---|------|-----------|------|------|
| 1 | **产品经理** | `requirements-analyst` | 编写PRD需求文档，澄清需求边界，验收标准定义 | 管理 |
| 2 | **系统架构师** | `system-architect` | 类图设计、接口定义、设计模式选型、模块依赖关系 | 研发 |
| 3 | **核心开发** | `general-purpose` | 基础设施层开发：IConnection、ConnectionFactory、SettingsManager、工具类 | 研发 |
| 4 | **UI开发** | `frontend-architect` | 表现层开发：TerminalWidget、ConfigPanel、QuickCommandBar、主题、布局 | 研发 |
| 5 | **协议开发** | `general-purpose` | 业务层开发：OTA协议(X/Y/ZMODEM)、帧解析器、Intel HEX解析 | 研发 |
| 6 | **集成开发** | `general-purpose` | 跨层集成：RTT/J-Link桥接、TCP/UDP连接、数据流串联、端到端联调 | 研发 |
| 7 | **UI/UX产品体验师** | `frontend-architect` | 观感评审：对照现代化UI标准审查界面布局、配色、间距、动效、一致性，输出改进建议 | 设计 |
| 8 | **代码审查员** | `self-review` | 代码质量审查：设计模式合规、重复代码检测、分层违规检查 | 质量 |
| 9 | **QA工程师** | `quality-engineer` | 编译验证、单元测试、边界条件测试、性能测试 | 质量 |

**并行策略**:
- Step 4 编码阶段，角色3~6可同时启动，各自负责不同模块
- 角色3(核心)和角色5(协议)无依赖，可完全并行
- 角色4(UI)依赖角色3(核心)的接口定义，需等接口稳定后启动
- 角色6(集成)需等其他模块完成后串联
- 角色7(UI/UX体验师)在每个迭代完成后进行观感评审，输出改进项
- 角色8(审查)和角色9(QA)在编码完成后串行执行

### 3.3 架构管理审查（每5次commit执行一次）

每完成5次commit（commit #5, #10, #15, ...），必须执行一次全面的架构管理审查:
- **审查人**: 系统架构师 (system-architect)
- **审查范围**:
  1. 分层架构合规性（表现层/业务层/数据层/基础设施层，无反向依赖）
  2. 公共组件复用检查（无重复造轮子）
  3. 设计模式合规性（8大模式是否正确应用）
  4. 模块间耦合度评估（接口是否足够抽象）
  5. 头文件引用规范（相对src路径、Qt→STL→项目顺序）
  6. 新增类是否登记到公共组件清单
  7. 目录结构是否按照规划组织
- **输出**: 架构审查报告，列出问题项和改进建议
- **原则**: 像嵌入式开发一样，分层清晰、复用性高、地基打稳

### 3.4 Commit 规则（铁律）

1. **每次commit必须 ≥ 300行代码变更**（不含空行和注释）
2. **每次commit = +1分，不足300行不允许commit**
3. **零编译错误才能commit** — 编译不过必须先修
4. **每次commit后必须验证 EmbedDebug.bat 能正常启动应用** — bat启动失败不允许commit
5. **commit message格式**:

```
<模块名>: <简述改了什么>

<详细说明为什么这样改，解决了什么问题>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```

6. **禁止的行为**:
   - 禁止提交编译不过的代码
   - 禁止重复造轮子（公共组件只写一次）
   - 禁止不经PRD直接写代码
   - 禁止不经架构审查直接加新类
   - 禁止不足300行变更就提交commit
   - 禁止提交后bat无法启动应用

### 3.5 多Agent并行工作流（每次迭代必须执行）

每次迭代必须同时启动以下 8 个 Agent 进行并行工作，形成完整的 workflow 闭环:

```
┌─────────────────────────────────────────────────────────────────┐
│                    每次迭代启动 8 Agent                          │
├──────────┬──────────────────┬──────────────────────────────────────┤
│ # │ 角色       │ Agent类型            │ 职责                     │
├──────────┼──────────────────┼──────────────────────────────────────┤
│ 1 │ 产品经理   │ requirements-analyst │ 编写/更新PRD，定义需求边界 │
│ 2 │ 系统架构师 │ system-architect     │ 设计类图、接口、模式选型   │
│ 3 │ 核心开发   │ general-purpose      │ 基础设施层+核心代码实现    │
│ 4 │ UI开发     │ frontend-architect   │ 表现层UI开发+QSS主题       │
│ 5 │ 协议开发   │ general-purpose      │ 业务层协议/解析器开发      │
│ 6 │ UI/UX审查  │ frontend-architect   │ 观感评审，硬编码颜色检测   │
│ 7 │ 代码审查   │ self-review          │ 代码质量+模式合规+分层检查 │
│ 8 │ QA工程师   │ quality-engineer     │ 编译验证+测试计划+边界测试 │
└──────────┴──────────────────┴──────────────────────────────────────┘
```

**执行顺序**:
1. **第一波（并行启动 Agent 1+2）**: 产品经理写PRD + 架构师设计，两个Agent同时工作
2. **第二波（并行启动 Agent 3+4+5）**: 三个开发Agent根据PRD和架构同时编码
3. **第三波（并行启动 Agent 6+7+8）**: 开发完成后，三个审查Agent并行审查
4. **主线程汇总**: 收集8个Agent的结果 → 修复问题 → 编译验证 → commit

**强制要求**:
- 每次迭代必须使用 `Agent` 工具启动8个子Agent，不得省略
- 所有Agent使用 `run_in_background: true` 并行执行
- 主线程等待所有Agent完成后汇总结果
- 汇总后修复Agent发现的问题，确保编译通过
- 最终验证 `EmbedDebug.bat` 能正常启动应用

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

## 六、现代化UI执行标准（UI/UX产品体验师强制执行）

> 本节由 UI/UX 产品体验师角色维护，所有UI相关开发必须遵守以下标准。
> 所有文字说明使用中文，开发者必须能无障碍理解每一条规则。

### 6.0 设计灵感与标杆参考

本项目的UI风格对标以下业界顶级产品的设计语言:

| 标杆产品 | 借鉴要点 | 在EmbedDebug中的应用 |
|---------|---------|---------------------|
| **Linear** (linear.app) | 极简暗色主题、微妙的边框分隔、精准的留白、流畅的面板切换动画 | 导航树选中态、面板切换过渡、整体暗色调 |
| **Raycast** (raycast.com) | 毛玻璃效果、键盘优先交互、优雅的弹出面板、图标一致性 | 快捷指令栏、搜索栏、快捷键系统 |
| **Arc Browser** (arc.net) | 侧边栏彩色标签、丝滑的Tab切换、空间感层次 | 导航树连接类型标识、Tab切换动画 |
| **Vercel仪表板** (vercel.com) | 数据展示清晰、状态指示器优雅、进度条流畅 | OTA进度条、数据统计面板、连接状态指示 |
| **Warp终端** (warp.dev) | 现代终端体验、命令块分组、智能补全 | 终端显示区、发送历史、搜索过滤 |

**核心设计理念**: "克制而精致" — 不追求花哨效果，每个动效都有明确的功能目的。
参考 Linear 的 "少即是多" 哲学：用留白和层次而非装饰来表达结构。

### 6.1 设计原则

| 原则 | 中文说明 | 执行要求 |
|------|---------|---------|
| **一致性** | 相同功能使用完全相同的视觉表现，不能出现两种风格的按钮或输入框 | 所有按钮、输入框、列表必须使用统一样式 |
| **层次分明** | 通过颜色深浅、大小、间距来表达信息优先级 | 主操作区 > 辅助区 > 背景装饰，亮度递减 |
| **留白呼吸** | 控件间保持充足间距，让人眼睛有休息的空间 | 最小间距 8px，分组间距 16px，面板内边距 12px |
| **即时反馈** | 用户每一次操作都必须有看得见的回应，不能让用户猜测系统是否响应了 | 按钮有 hover/pressed 状态、加载有指示、操作有结果通知 |
| **信息密度** | 嵌入式工具需要在有限空间内展示大量数据，但不能让人眼花缭乱 | 关键数据实时可见，次要信息按需展开折叠 |
| **动效克制** | 每一个动画都是为了帮助用户理解"发生了什么"，不是为了炫技 | 动画必须服务于功能（位置变化、状态变化、引导注意力） |

### 6.2 配色体系

所有主题必须定义以下语义色板，控件使用语义色而非硬编码颜色:

```
--bg-primary      : 主背景（最深，整个窗口底色）
--bg-secondary    : 次背景（面板/卡片/分组框的背景）
--bg-tertiary     : 三级背景（输入框/悬浮提示/下拉列表背景）
--bg-hover        : 鼠标悬浮背景（导航项、列表行）
--text-primary    : 主文字（高对比度，标题和重要数据）
--text-secondary  : 次文字（描述文字、标签、表头）
--text-muted      : 弱文字（占位符、禁用态文字、提示信息）
--accent          : 强调色（主操作按钮、选中态、链接、焦点边框）
--accent-hover    : 强调色悬停态（比accent亮10-15%）
--accent-pressed  : 强调色按下态（比accent暗10-15%）
--border          : 边框色（分隔线、输入框边框）
--border-focus    : 焦点边框色（输入框获得焦点时的边框）
--success         : 成功色（连接成功、传输完成、校验通过）
--warning         : 警告色（正在连接、传输中、需注意）
--error           : 错误色（断开连接、传输失败、校验失败）
--shadow          : 阴影色（面板浮起时的阴影）
--scrollbar       : 滚动条默认色
--scrollbar-hover : 滚动条悬停色
```

**禁止**: 在 QSS 中硬编码 `#RRGGBB` 色值用于语义功能。
**正确**: 使用对应语义色（如错误信息用 `color: var(--error)`）。

### 6.3 布局与间距

| 元素 | 尺寸 | 中文说明 |
|------|------|---------|
| 工具栏高度 | 36-40px | 紧凑不臃肿，不占用终端显示空间 |
| 导航树最小宽度 | 180px | 保证中文字体可读 |
| 导航树最大宽度 | 280px | 不占用过多终端空间 |
| 面板内边距 | 8-12px | 控件与面板边缘之间的留白 |
| 控件间距 | 6-8px | 同行/同列相邻控件之间的间距 |
| 分组间距 | 12-16px | 不同功能分组之间的间距（GroupBox之间） |
| 按钮最小高度 | 28px | 手指触控友好 |
| 输入框高度 | 28-32px | 与旁边的按钮高度对齐 |
| 发送区域高度 | 36-40px | 包含输入框和发送按钮的底部区域 |
| 状态栏高度 | 24-28px | 底部信息条，紧凑展示 |
| 搜索栏高度 | 32-36px | 嵌入终端顶部的搜索条 |
| 圆角统一值 | 6px | 所有按钮、输入框、面板使用统一圆角 |

### 6.4 排版规范

| 规则 | 值 | 中文说明 |
|------|-----|---------|
| 终端字体 | Consolas / JetBrains Mono / Source Code Pro | 等宽字体，必须支持中文回退显示 |
| 界面字体 | "Microsoft YaHei UI" / "Segoe UI" | Windows系统默认UI字体，中文显示清晰 |
| 终端字号 | 13-14px | 默认13px，用户可在设置中调整 |
| 界面字号 | 12-13px | 标签12px，分组标题13px |
| 行高 | 字号 × 1.5 | 终端行间距，比UI行高稍大便于阅读 |

### 6.5 过渡动画规范（必须实现）

> 参考 Linear 的面板切换和 Raycast 的弹出效果。
> 所有动画使用 QPropertyAnimation 实现，缓动曲线统一使用 QEasingCurve。

#### 必须实现的动画清单

| 动画名称 | 触发场景 | 动画类型 | 持续时间 | 缓动曲线 | 实现方式 |
|---------|---------|---------|---------|---------|---------|
| **按钮悬浮渐变** | 鼠标进入/离开按钮 | 背景色渐变 | 200ms | OutCubic | QPropertyAnimation on "widgetStyleSheet" 或 QSS transition |
| **按钮按下回弹** | 鼠标按下/释放按钮 | 背景色渐变 + 轻微缩放 | 100ms | Linear | QSS pressed 状态 + 可选 scale transform |
| **面板滑入展开** | 点击导航树切换面板 | 从右侧滑入 + 淡入 | 250ms | OutCubic | QPropertyAnimation on geometry + windowOpacity |
| **面板滑出收起** | 当前面板被替换 | 向左滑出 + 淡出 | 200ms | InCubic | 同上，反向 |
| **搜索栏展开** | Ctrl+F 打开搜索 | 从顶部向下滑出 | 200ms | OutCubic | QPropertyAnimation on maximumHeight: 0→36 |
| **搜索栏收起** | Esc 关闭搜索 | 向上收回 | 150ms | InCubic | QPropertyAnimation on maximumHeight: 36→0 |
| **连接状态脉冲** | 正在连接中 | 状态点呼吸灯效果 | 1500ms循环 | InOutSine | QPropertyAnimation on 点的opacity: 0.3→1.0→0.3 |
| **进度条流动** | OTA传输进行中 | 进度条填充色渐变流动 | 2000ms循环 | Linear | QPropertyAnimation on 渐变偏移量 |
| **进度条完成** | 传输100% | 填充色从accent变为success | 400ms | OutCubic | QPropertyAnimation on 背景色 |
| **导航树选中滑动** | 点击不同导航项 | 左侧指示线从旧位置滑到新位置 | 250ms | OutCubic | QPropertyAnimation on 指示线Y位置 |
| **通知吐司弹出** | 操作完成/错误提示 | 从底部向上弹出 + 淡入 | 300ms | OutBack | QPropertyAnimation on pos + windowOpacity |
| **通知吐司消失** | 自动消失 | 向上飘出 + 淡出 | 250ms | InCubic | 同上，反向 |
| **主题切换过渡** | 切换暗色/亮色主题 | 整体淡入淡出 | 300ms | InOutCubic | QGraphicsOpacityEffect + QPropertyAnimation |
| **Tab标签切换** | 点击不同连接Tab | 下划线滑动到新Tab | 200ms | OutCubic | QPropertyAnimation on 下划线X位置 |

#### 动画实现铁律

1. **使用 QPropertyAnimation** — 所有动画必须用 Qt 的动画框架，禁止用 QTimer 手动插值
2. **缓动曲线统一** — 展开/滑入用 `QEasingCurve::OutCubic`，收起/滑出用 `QEasingCurve::InCubic`
3. **时长限制** — 最短100ms（按钮按下），最长400ms（进度完成），超出禁止
4. **禁止的动画类型**:
   - 禁止弹跳效果(Bounce/Back除非用于通知弹出)
   - 禁止3D旋转/翻转
   - 禁止超过500ms的动画
   - 禁止彩虹色渐变或闪烁
5. **性能要求** — 动画帧率不低于30fps，不能卡顿掉帧

#### Qt动画代码模板

```cpp
// 标准面板滑入动画
QPropertyAnimation* anim = new QPropertyAnimation(widget, "maximumHeight");
anim->setStartValue(0);
anim->setEndValue(targetHeight);
anim->setDuration(250);
anim->setEasingCurve(QEasingCurve::OutCubic);
anim->start(QAbstractAnimation::DeleteWhenStopped);

// 标准淡入动画
QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(widget);
widget->setGraphicsEffect(effect);
QPropertyAnimation* fade = new QPropertyAnimation(effect, "opacity");
fade->setStartValue(0.0);
fade->setEndValue(1.0);
fade->setDuration(200);
fade->setEasingCurve(QEasingCurve::OutCubic);
fade->start(QAbstractAnimation::DeleteWhenStopped);
```

### 6.6 组件设计规范

#### 按钮体系（参考Linear的按钮层次）

| 按钮类型 | 外观 | 应用场景 |
|---------|------|---------|
| **主要按钮** | 实心填充 accent 色，白色文字，hover时亮度+10% | 连接、开始传输、发送 |
| **次要按钮** | 透明背景，accent 色描边，accent 色文字，hover时填充半透明accent | 清除、导出、配置 |
| **危险按钮** | 实心填充 error 色，白色文字 | 断开连接、取消传输 |
| **幽灵按钮** | 无边框无背景，text-secondary 色文字，hover时显示bg-hover背景 | 折叠展开、更多选项 |
| **禁用态** | 所有按钮禁用时 opacity=40%，无hover效果 | 条件不满足时 |

#### 输入框（参考Vercel的表单设计）

- 正常态: bg-tertiary 背景 + 1px border 色边框 + 6px圆角
- 悬浮态: border 色边框微微变亮（通过QSS transition）
- 焦点态: 2px border-focus 色边框 + 轻微外发光（box-shadow效果）
- 错误态: 2px error 色边框 + 浅红背景
- 禁用态: opacity=50%，不可点击

#### 下拉选择框（ComboBox）

- 与输入框等高，相同的边框和圆角风格
- 下拉箭头使用 accent 色的小三角图标
- 下拉弹出列表: bg-secondary 背景 + 6px圆角 + 轻微阴影
- 选中项: bg-hover 背景

#### 树形导航（参考Arc的侧边栏设计）

- 选中项: bg-hover 背景 + 左侧 3px accent 色竖线（从上一位平滑滑动过来）
- 悬浮态: bg-hover 半透明背景，无竖线
- 展开/折叠图标: 使用小三角形（▸/▾），text-secondary 色
- 连接类型图标: 串口(蓝色圆点)、TCP(绿色圆点)、UDP(黄色圆点)、RTT(紫色圆点)
- 分组标题: 大写字母 + 1px底部边框，类似 Linear 的项目分组

#### 状态栏（参考Vercel Dashboard底部信息条）

- 固定在窗口底部，高度 24-28px
- 左侧: 连接状态指示器（带呼吸动画的彩色圆点）
- 中间: 当前活动连接的名称
- 右侧: RX/TX 字节数、实时速率
- 背景: bg-primary + 顶部 1px border 分隔线

#### 搜索栏（参考Raycast的搜索弹出效果）

- 嵌入终端顶部，不独占一行，有展开/收起动画
- 背景: bg-tertiary + 轻微底部阴影
- 关闭按钮在右侧，ESC键可关闭
- 模式切换(正则/HEX/大小写)使用切换按钮组

#### 通知吐司（Toast，参考Linear的通知设计）

- 在窗口右下角弹出，不遮挡主要操作区
- 成功: success色左边框 + 成功图标 + 消息文字
- 错误: error色左边框 + 错误图标 + 消息文字
- 3秒后自动消失，有向上飘出的消失动画
- 可点击关闭按钮提前关闭

### 6.7 观感评审检查清单

UI/UX 产品体验师每次迭代后必须检查:

- [ ] 所有颜色使用语义色板，无硬编码色值（主题定义文件除外）
- [ ] 间距符合 6.3 标准，视觉上没有拥挤或紧凑的区域
- [ ] 字体使用正确（终端用等宽字体，界面用系统字体）
- [ ] 按钮/输入框/下拉框的样式在全局保持一致
- [ ] 每个按钮都有 hover、pressed、disabled 三种状态
- [ ] 每个输入框都有 normal、focus、error、disabled 四种状态
- [ ] 连接状态指示有明确颜色（绿色=已连接，黄色=连接中，红色=断开/错误）
- [ ] 连接中状态有呼吸动画，不能是静态黄色
- [ ] 导航树选中态清晰，有左侧指示线
- [ ] 面板切换有滑入/滑出动画
- [ ] 搜索栏有展开/收起动画
- [ ] 主题切换有淡入淡出过渡
- [ ] OTA进度条有流动效果和完成变色动画
- [ ] 窗口 resize 时布局不错乱
- [ ] 暗色/亮色主题切换后所有控件可读
- [ ] 所有动画帧率不低于30fps，不卡顿

### 6.8 UI执行强制规则（铁律，违反不允许commit）

1. **禁止在C++代码中硬编码颜色值到setStyleSheet()** — 所有颜色必须从主题QSS文件中获取
   - 错误示例: `m_btn->setStyleSheet("QPushButton { background-color: #89b4fa; }")`
   - 正确做法: 在QSS主题文件中定义 `QPushButton#startBtn { background-color: var(--accent); }`
   - 唯一例外: TerminalWidget自绘引擎中使用QPainter时，颜色从ThemeManager获取
2. **所有QWidget必须设置objectName** — QSS选择器依赖objectName
3. **新增面板/控件必须在所有主题QSS中添加对应样式** — 不允许只改一个主题
4. **按钮必须有hover、pressed、disabled三种状态** — 纯色按钮不能只有默认状态
5. **所有用户可见文字必须使用tr()包裹** — 支持中英双语
6. **所有面板切换必须有过渡动画** — 不能突然出现/消失，必须有滑入/滑出或淡入/淡出
7. **所有状态变化必须有视觉反馈** — 连接/断开、传输开始/结束、错误发生都必须有颜色变化或动画

### 6.9 主题QSS文件管理规范

每个主题QSS文件必须定义完整的语义色板变量:

```css
/* 必须定义的颜色变量 */
--bg-primary:      #1e1e2e;   /* 主背景 */
--bg-secondary:    #313244;   /* 面板/卡片背景 */
--bg-tertiary:     #45475a;   /* 输入框背景 */
--bg-hover:        #45475a;   /* 鼠标悬浮背景 */
--text-primary:    #cdd6f4;   /* 主文字 */
--text-secondary:  #a6adc8;   /* 次文字 */
--text-muted:      #6c7086;   /* 弱文字 */
--accent:          #89b4fa;   /* 强调色 */
--accent-hover:    #b4d0fb;   /* 强调色悬停 */
--accent-pressed:  #74a8f7;   /* 强调色按下 */
--border:          #313244;   /* 边框 */
--border-focus:    #89b4fa;   /* 焦点边框 */
--success:         #a6e3a1;   /* 成功 */
--warning:         #f9e2af;   /* 警告 */
--error:           #f38ba8;   /* 错误 */
--shadow:          rgba(0,0,0,0.3); /* 阴影 */
--scrollbar:       #45475a;   /* 滚动条 */
--scrollbar-hover: #585b70;   /* 滚动条悬停 */
```

每个新增控件必须在三个主题文件中同步添加样式:
- `resources/themes/dark_terminal.qss`
- `resources/themes/modern_dark.qss`
- `resources/themes/light.qss`

### 6.10 动画实现优先级

| 优先级 | 动画 | 理由 |
|-------|------|------|
| P0 必须实现 | 面板切换滑入/滑出 | 最频繁的交互，没有动画体验极差 |
| P0 必须实现 | 搜索栏展开/收起 | Linear级别的体验基准 |
| P0 必须实现 | 连接状态呼吸动画 | 让用户知道"正在连接中"而非卡死 |
| P0 必须实现 | 主题切换淡入淡出 | 切换主题时不能闪瞎眼 |
| P1 应该实现 | 进度条流动效果 | OTA体验的关键视觉反馈 |
| P1 应该实现 | 导航树选中线滑动 | Arc侧边栏级别的精致感 |
| P1 应该实现 | 通知吐司弹出/消失 | 操作反馈的优雅方式 |
| P2 可以延后 | Tab切换下划线滑动 | 锦上添花的细节 |

---

## 七、PRD文档规范

### 7.1 文件位置

```
docs/prd/PRD_<编号>_<简述>.md
```

### 7.2 PRD模板

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

## 八、Git规范

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

## 九、评分追踪

| # | Commit内容 | 分数 |
|---|-----------|------|
| 0 | 项目初始状态 | 1 |
| 1 | 工厂模式+数据导出+发送历史+数据统计+架构重构 | 2 |
| 2 | 搜索栏+多主题切换+会话持久化+UI/UX产品体验师角色+现代化UI标准 | 3 |
| 3 | 协议帧格式定义+状态机解析器+解析结果表格展示 | 4 |
| 4 | 帧可视化编辑器+实时波形图+IntelHEX解析器 | 5 |
| 5 | 架构审查修复: 悬挂指针+重复代码+反向依赖解耦 | 6 |
| 6 | TCP Client/Server + UDP网络连接实现 | 7 |
| 7 | XMODEM协议传输(Checksum/CRC/1K)+OTA管理器+OTA操作面板 | 8 |
| 8 | YMODEM协议传输(Block 0文件信息+批量传输)+OTA集成 | 9 |
| 9 | ZMODEM协议传输(CRC32+连续发送+HEX/BIN帧)+OTA集成 | 10 |
| 10 | 架构审查修复: SerialConfigPanel分层违规+HEX验证复用+TcpConnection类型bug+面板切换重构 | 11 |
| 11 | DataLogger数据日志记录和回放(EDL二进制格式+变速回放) | 12 |
| 12 | OtaHistoryModel OTA历史记录模型(持久化+表格展示+自动记录) | 13 |
| 13 | README中文版重写+bat启动验证 | 14 |
| 14 | CLAUDE.md约束文档更新:UI强制规则+QSS规范+多Agent工作流+300行规则 | 15 |
| 15 | BaseTransfer模板方法基类提取+XModem/YModem/ZModem重构+架构审查+PRD文档 | 16 |
| 16 | QSS主题迁移+UI全面中文化+OTA性能优化O(n^2)→O(n)+ChannelConfig/ChartModel实现 | 18 |
| 17 | 过渡动画+ChartModel集成+QSS按钮状态修复+硬编码颜色清除+中英语言选择 | 19 |
| ... | 目标: 1000分 | 1000 |

---

## 十、项目目录结构

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
