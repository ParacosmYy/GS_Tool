# EmbedDebug - 项目开发约束文档

> 本文档是所有开发行为的最高约束入口。
> 详细约束已拆分为模块化文档，分布在 `docs/constraints/` 目录下。
> 修改本文档或约束模块需要用户审查通过后方可生效。

---

## 约束模块索引

| 模块 | 文件 | 何时加载 |
|------|------|---------|
| 项目概况 + 构建环境 | [docs/constraints/01-project-overview.md](docs/constraints/01-project-overview.md) | 每次开发 |
| 开发工作流 + Agent | [docs/constraints/02-workflow.md](docs/constraints/02-workflow.md) | 每次迭代 |
| 架构原则 + 设计模式 | [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md) | 涉及架构/新增类 |
| 编码规范 | [docs/constraints/04-coding-standard.md](docs/constraints/04-coding-standard.md) | 每次编码 |
| UI执行标准 | [docs/constraints/05-ui-standard.md](docs/constraints/05-ui-standard.md) | 涉及UI改动 |
| Git + Commit规则 | [docs/constraints/06-git-commit.md](docs/constraints/06-git-commit.md) | 每次提交 |
| 目录结构 | [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md) | 涉及文件创建/移动 |
| 图标标准 | [docs/constraints/08-icon-standard.md](docs/constraints/08-icon-standard.md) | 涉及图标/SVG使用 |

---

## 铁律（所有开发都必须遵守，违反不允许commit）

### 工作流铁律
1. **禁止不经PRD直接写代码** — 每个功能必须有PRD
2. **禁止不经架构审查直接加新类** — 新类必须通过检查清单
3. **每次commit ≥ 300行代码变更** — 不足300行不允许commit
4. **零编译错误才能commit** — 编译不过必须先修
5. **每次commit后必须验证 EmbedDebug.bat 能正常启动**
5.5. **禁止提交构建系统(CMakeLists.txt)中不存在的源文件** — 所有 .h/.cpp 必须先在 CMakeLists.txt 注册才能提交。禁止"查无产生"死代码刷分

### 架构铁律
6. **分层单向依赖**: 表现层→业务层→数据层→基础设施层，**禁止反向**
7. **禁止在MainWindow中写业务逻辑** — 委托给Controller/Manager
8. **MainWindow.cpp ≤ 500行** — 超过必须拆分
9. **公共组件只写一次** — CRC/HexConverter/RingBuffer/SettingsManager等已验证组件不得重写
10. **模块间依赖必须遵循 [03-architecture.md](docs/constraints/03-architecture.md) 的依赖方向规则** — 禁止反向依赖、禁止同层横向依赖、禁止跨层跳级

### 编码铁律
11. **C++17标准** — 头文件引用: Qt→STL→项目，使用相对src路径
12. **Qt信号/槽用新式connect语法** — 禁止SIGNAL/SLOT宏
13. **详细中文注释** — Doxygen格式，每个公开方法/成员变量必须有注释
14. **禁止裸new不配对delete** — QObject父子树或智能指针

### UI铁律
15. **禁止C++中硬编码颜色到setStyleSheet()** — 颜色从QSS主题获取
16. **所有QWidget必须设置objectName** — QSS依赖
17. **按钮必须有hover/pressed/disabled三种状态**
18. **面板切换必须有过渡动画** — 禁止突然出现/消失
19. **所有用户可见文字必须用tr()包裹**

### 文件体积铁律
20. **.cpp ≤ 500行** — 超过说明职责过多
21. **.h ≤ 200行** — 超过说明成员/方法过多
22. **单个方法 ≤ 80行** — 超过说明逻辑过于复杂

---

## 快速参考

| 项 | 值 |
|----|-----|
| 应用名称 | EmbedDebug |
| 项目路径 | `E:\Embedded\Tool\Serial_tool\User_Serial` |
| 当前版本 | 0.1.0 |
| 评分 | 见 [docs/tracking/SCORE_TRACKING.md](docs/tracking/SCORE_TRACKING.md) |
| Git分支 | `feat/embed-debug` |
| Git远程 | `https://github.com/ParacosmYy/GS_Tool.git` |

### 构建命令
```bash
cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=E:/Tool/DevEnv/Qt/6.8.3/mingw_64
cmake --build build
E:/Tool/DevEnv/Qt/6.8.3/mingw_64/bin/windeployqt.exe build/EmbedDebug.exe
```

### Commit Message格式
```
<模块名>: <简述改了什么>

<详细说明为什么这样改>

评分: <当前总分> + 1 = <新分数>
变更: <文件数> files, <+新增行数> insertions, <-删除行数> deletions
```

---

## 项目现状审计 (2026-06-03)

> 以下基于对 src/ 源码结构、PRD_043~PRD_069、ROADMAP 文档、约束文档体系的全面审计。
> 状态标记: ✅ 已完成 | 🔧 进行中 | 📋 PRD已写未实现 | ❌ 未开始
> 更新: 2026-06-03 二次审计 — UI-01~UX-03已全部实现，审计表已更正

---

### 一、UI 基础设施 (优先级: 最高)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| UI-01 | **BasePanel 容器组件** | ✅ | PRD-061 | 已实现: 标题栏/折叠/动画/阴影/空状态/骨架屏(509行) |
| UI-02 | **IconManager SVG图标系统** | ✅ | PRD-062 | 已实现: SVG着色管线+缓存(138行)。图标数量不足(仅14个SVG，需50+) |
| UI-03 | **EmptyStateWidget/LoadingSpinner/SkeletonWidget** | ✅ | PRD-063 | 已实现: 三个组件全部完成(EmptyState 196行/Spinner 135行/Skeleton 115行) |
| UI-04 | **CommandPalette 命令面板** | ✅ | PRD-064 | 已实现: 模糊搜索+半透明遮罩+事件过滤(298行) |
| UI-05 | **IconNavBar 三栏布局** | ✅ | PRD-065 | 已实现: 分类按钮+滑动指示器(133行) |
| UI-06 | **QSS主题生成器** | ❌ | PRD-066 | 当前3套QSS共4740行手动维护，需自动化生成 |

### 二、交互体验增强 (优先级: 高)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| UX-01 | **SendCompleter 智能补全** | ✅ | PRD-067 | 已实现为SmartAutoComplete(234行): 前缀匹配+频率排序+弹出列表 |
| UX-02 | **ScriptRecorder/Player 脚本录制回放** | ✅ | PRD-068 | 已实现: 记录/发送/回放+JSON保存(193行) |
| UX-03 | **DataDiffWidget 数据对比** | ✅ | PRD-069 | 已实现: HTML diff渲染+颜色标记(318行) |
| UX-04 | **键盘快捷键体系完善** | ❌ | 05-ui-standard §十五 | Ctrl+F/Ctrl+P/Ctrl+Shift+R/Ctrl+Enter 等全局快捷键尚未全面实现 |
| UX-05 | **响应式布局** | ❌ | 05-ui-standard §十七 | 窗口<900px自动折叠导航树、断点动画过渡。窗口resize时布局适配 |
| UX-06 | **弹窗/对话框体系统一** | ❌ | 05-ui-standard §十六 | 禁止QMessageBox，统一自定义弹窗(确认/警告/错误)。当前仍有QMessageBox使用 |

### 三、核心功能缺失 (优先级: 高)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| FN-01 | **高级波形引擎 Phase 1~4** | ✅ | FEATURE_WaveformEngine | 已实现: FFT(842行)+多Y轴(391行)+游标(535行)+直方/散点(903行)+缩放(478行) |
| FN-02 | **数据录制回放增强(F1)** | ✅ | ROADMAP_FutureFeatures | 已实现: 统一时间轴+变速回放+标注(1465行recording模块) |
| FN-03 | **自定义协议脚本引擎(F2)** | ✅ | ROADMAP_FutureFeatures | 已实现: ProtocolSchema+ProtocolEngine+模板库(745行schema+682行engine) |
| FN-04 | **SEGGER RTT 深度集成(F6)** | 🔧 | ROADMAP_FutureFeatures | 骨架已建(718行rtt模块)，J-Link SDK动态加载为stub，需真实DLL集成 |

### 四、数据与导出 (优先级: 中)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| DA-01 | **多通道数据导出增强(F3)** | ✅ | ROADMAP_FutureFeatures | 已实现: CSV/XLSX/PNG导出(1237行export模块) |
| DA-02 | **终端分屏+标签页(F4)** | ✅ | ROADMAP_FutureFeatures | 已实现: 水平/垂直分屏+多Tab+双视图(814行layout模块) |

### 五、高级功能 (优先级: 中低)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| AD-01 | **仪表盘模式(F5)** | ✅ | ROADMAP_FutureFeatures | 已实现: Gauge/进度条/LED/数值显示(1130行dashboard模块)，布局保存待完善 |
| AD-02 | **数据流触发器(F7)** | ✅ | ROADMAP_FutureFeatures | 已实现: 触发器引擎+规则管理(958行automation模块)，PlaySound动作为stub |
| AD-03 | **工程会话管理(F8)** | ✅ | ROADMAP_FutureFeatures | 已实现: .edproj文件+工程切换(1066行settings模块) |
| AD-04 | **串口高级调试(F9)** | ✅ | — | 已实现: 信号线监控+错误计数+流量曲线(1108行data模块+379行signals模块) |
| AD-05 | **性能监控(F10)** | ✅ | ROADMAP_FutureFeatures | 已实现: FPS+管道延迟+吞吐量(327行perf模块) |

### 六、远期扩展 (优先级: 低，ROADMAP_MassiveExpansion)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| EX-01 | BLE/BLE调试(F12) | ✅骨架 | MassiveExpansion | 已实现框架(1301行ble模块)，GATT Model有11个空方法体 |
| EX-02 | CAN/CAN-FD总线(F13) | ✅骨架 | MassiveExpansion | 已实现框架(811行can模块)，DBC解析为TODO |
| EX-03 | MQTT客户端(F14) | ✅骨架 | MassiveExpansion | 已实现框架(1102行mqtt模块)，TopicModel有12个空方法体 |
| EX-04 | SPI/I2C桥接(F16) | ✅骨架 | MassiveExpansion | 已实现框架(1349行spi_i2c模块) |
| EX-05 | **插件系统(F11)** | ✅骨架 | ROADMAP_FutureFeatures | 已实现框架(727行plugin模块) |

---

### 七、待完成项目 (更新后)

#### 第一优先级: 遗留核心缺失
1. **UI-06 QSS主题生成器** → 自动化3套主题维护
2. **UX-04 键盘快捷键体系** → ShortcutManager统一管理
3. **UX-05 响应式布局** → 断点系统+导航树自适应
4. **UX-06 弹窗体系统一** → 替换QMessageBox

#### 第二优先级: 骨架功能完善
5. **MQTT TopicModel** → 12个空QAbstractItemModel方法实现
6. **BLE GattModel** → 11个空方法实现
7. **USB libusb集成** → 11个TODO stub实现
8. **Dashboard布局持久化** → JSON序列化
9. **ProtocolEngine CRC校验** → 协议完整性验证
10. **DBC解析器** → CAN数据库文件解析

#### 第三优先级: 图标+代码质量
11. **Lucide SVG扩展** → 从14个扩展到50+个
12. **Doxygen补全** → 新增模块注释
13. **tr()合规审计** → 新增文件国际化
14. **objectName审计** → 新增控件QSS依赖

#### 第四优先级: RTT真实集成
15. **J-Link SDK真实调用** → 需要J-Link DLL

---

### 八、体验感差距 (对比竞品)

| 竞品 | EmbedDebug 差距 | 改进方向 |
|------|----------------|---------|
| **VOFA+** | 波形无游标/缩放/FFT | FN-01 波形引擎 |
| **Serial Studio** | 无仪表盘(Gauge/LED) | AD-01 仪表盘模式 |
| **MobaXterm** | 无终端分屏/Tab | DA-02 终端增强 |
| **Docklight** | 无触发器/自动化 | AD-02 数据流触发器 |
| **J-Link RTT Viewer** | RTT未实现 | FN-04 RTT集成 |
| **VS Code** | 无命令面板(Ctrl+P) | UI-04 CommandPalette |
| **Linear/Arc** | 图标/空状态/动画不一致 | UI-01~UI-03 基础组件 |
