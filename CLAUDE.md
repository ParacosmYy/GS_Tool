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

## 六、现代化UI执行标准（UI/UX产品体验师强制执行）

> 本节由 UI/UX 产品体验师角色维护，所有UI相关开发必须遵守以下标准。
> 每个 Step 结束后，UI/UX 产品体验师对照本标准进行观感评审。

### 6.1 设计原则

| 原则 | 说明 | 执行要求 |
|------|------|---------|
| **一致性** | 相同功能使用相同的视觉表现 | 所有按钮、输入框、列表必须使用统一样式 |
| **层次分明** | 通过颜色深浅和间距表达信息优先级 | 主操作区 > 辅助区 > 背景装饰，亮度递减 |
| **留白呼吸** | 控件间保持充足间距，不拥挤 | 最小间距 8px，分组间距 16px，面板内边距 12px |
| **即时反馈** | 用户操作必须产生可见的视觉响应 | 按钮 hover/pressed 状态、加载指示、操作结果通知 |
| **信息密度** | 嵌入式工具需要高信息密度但不杂乱 | 关键数据实时可见，次要信息按需展开 |

### 6.2 配色体系

所有主题必须定义以下语义色板，控件使用语义色而非硬编码颜色:

```
--bg-primary      : 主背景（最深）
--bg-secondary    : 次背景（面板/卡片）
--bg-tertiary     : 三级背景（输入框/悬浮提示）
--text-primary    : 主文字（高对比度）
--text-secondary  : 次文字（描述/标签）
--text-muted      : 弱文字（占位符/禁用）
--accent          : 强调色（主操作按钮/选中态/链接）
--accent-hover    : 强调色悬停
--border          : 边框色
--border-focus    : 焦点边框色
--success         : 成功/连接成功
--warning         : 警告/正在连接
--error           : 错误/断开连接
--scrollbar       : 滚动条
--scrollbar-hover : 滚动条悬停
```

**禁止**: 在 QSS 中硬编码 `#RRGGBB` 色值用于语义功能（如 `color: red` 表示错误）。
**正确**: 使用对应语义色（如 `color: #f38ba8` 即 error 色）。

### 6.3 布局与间距

| 元素 | 尺寸 | 说明 |
|------|------|------|
| 工具栏高度 | 36-40px | 紧凑，不占用终端空间 |
| 导航树最小宽度 | 180px | 保证文字可读 |
| 导航树最大宽度 | 280px | 不占用过多终端空间 |
| 面板内边距 | 8-12px | 控件与面板边缘的间距 |
| 控件间距 | 6-8px | 同行/同列相邻控件间距 |
| 分组间距 | 12-16px | 不同功能组之间的间距 |
| 按钮最小高度 | 28px | 触控友好 |
| 输入框高度 | 28-32px | 与按钮对齐 |
| 发送区域高度 | 36-40px | 包含输入框和按钮 |
| 状态栏高度 | 24-28px | 紧凑信息展示 |
| 搜索栏高度 | 32-36px | 终端顶部嵌入 |

### 6.4 排版

| 规则 | 值 | 说明 |
|------|-----|------|
| 终端字体 | Consolas / JetBrains Mono / Source Code Pro | 等宽，支持中文回退 |
| UI字体 | "Microsoft YaHei UI" / "Segoe UI" | 系统默认UI字体 |
| 终端字号 | 13-14px | 默认13px，可配置 |
| UI字号 | 12-13px | 标签12px，标题13px |
| 行高 | 字号 × 1.4 | 终端行间距 |

### 6.5 交互与动效

| 交互 | 动效 | 持续时间 |
|------|------|---------|
| 按钮 hover | 背景色变浅 | 150ms |
| 按钮 pressed | 背景色加深 | 100ms |
| 面板展开/收起 | 高度动画 | 200ms ease-out |
| 搜索栏出现/消失 | 滑入/滑出 | 150ms |
| Tab切换 | 淡入淡出 | 100ms |
| 连接状态变化 | 状态栏颜色过渡 | 300ms |
| 窗口resize | 即时重绘 | 0ms |

**禁止**: 过长的动画（>300ms）、弹跳效果、3D变换等分散注意力的动效。

### 6.6 组件设计规范

#### 按钮
- 主操作按钮(Connect/Send): 填充 accent 色，白色文字
- 次要按钮(Clear/Export): 描边样式，透明背景
- 危险按钮(Disconnect): 填充 error 色
- 禁用态: 50% 透明度，无 hover 效果

#### 输入框
- 正常态: 浅色背景 + 细边框
- 焦点态: border-focus 色边框
- 错误态: error 色边框 + 浅红背景
- 圆角: 4px

#### 下拉框(ComboBox)
- 与输入框等高
- 下拉箭头使用 accent 色
- 下拉列表背景 = bg-secondary

#### 树形导航
- 选中项: bg-tertiary + 左侧 2px accent 色竖线
- hover: bg-tertiary 半透明
- 展开/折叠图标: text-secondary 色
- 不可编辑

#### 状态栏
- 固定在底部，高度 24-28px
- 左侧: 连接状态（带颜色指示点）
- 右侧: RX/TX 字节数
- 背景: bg-primary + 顶部 1px border

#### 搜索栏
- 嵌入终端顶部，不独占一行
- 背景: bg-tertiary
- 关闭按钮在右侧
- 模式切换(正则/HEX)使用 checkbox

### 6.7 观感评审检查清单

UI/UX 产品体验师每次迭代后检查:

- [ ] 所有颜色使用语义色板，无硬编码色值（除主题定义文件外）
- [ ] 间距符合 6.3 标准，无紧凑/拥挤区域
- [ ] 字体使用正确（终端用等宽，UI用系统字体）
- [ ] 按钮/输入框/下拉框样式一致
- [ ] hover/pressed/focus 状态均有视觉反馈
- [ ] 连接状态有明确的颜色指示（绿=连接，黄=连接中，红=断开/错误）
- [ ] 导航树选中态清晰
- [ ] 状态栏信息实时更新
- [ ] 窗口 resize 时布局不错乱
- [ ] 暗色/亮色主题切换后所有控件可读

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
