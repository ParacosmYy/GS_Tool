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
| README 企业级宣传标准 | [docs/constraints/02-workflow.md#41-readme-企业级宣传标准](docs/constraints/02-workflow.md#41-readme-企业级宣传标准) | 涉及 README / 对外说明 / 产品宣传 |
| 架构原则 + 设计模式 | [docs/constraints/03-architecture.md](docs/constraints/03-architecture.md) | 涉及架构/新增类 |
| 编码规范 | [docs/constraints/04-coding-standard.md](docs/constraints/04-coding-standard.md) | 每次编码 |
| UI执行标准 | [docs/constraints/05-ui-standard.md](docs/constraints/05-ui-standard.md) | 涉及UI改动 |
| Git + Commit规则 | [docs/constraints/06-git-commit.md](docs/constraints/06-git-commit.md) | 每次提交 |
| 目录结构 | [docs/constraints/07-directory-structure.md](docs/constraints/07-directory-structure.md) | 涉及文件创建/移动 |
| 图标标准 | [docs/constraints/08-icon-standard.md](docs/constraints/08-icon-standard.md) | 涉及图标/SVG使用 |
| Serial Station架构 | [docs/serial_station_architecture.md](docs/serial_station_architecture.md) | 涉及串口上位机重构/新增协议 |

---

## 铁律（所有开发都必须遵守，违反不允许commit）

### 工作流铁律
1. **禁止不经PRD直接写代码** — 每个功能必须有PRD
2. **禁止不经架构审查直接加新类** — 新类必须通过检查清单
3. **每次 commit 必须为一次完整代码增量，代码变更量≥500行** — 不足500行不允许代码 commit（不计文档/空白/注释）。
4. **零编译错误才能commit** — 编译不过必须先修
5. **`EmbedDebug.bat` 双击能启动是最低验收线** — 每次 commit 后必须验证；任何影响构建、启动、入口、资源、依赖、路径的改动，收口前也必须验证或说明无法验证的具体原因
5.5. **禁止恢复遗留 native 主线** — 不再新增 native 源码、原生构建清单、原生构建目录或 native 测试入口。
5.6. **禁止提交构建产物** — 严禁提交 `build/`、`dist/`、PyInstaller workpath、缓存、二进制产物或虚拟环境。
5.7. **启动入口永远走 Python/PyQt** — `EmbedDebug.bat` 只能进入 `uv run start-embeddebug`，不得恢复 native exe fallback。
5.8. **README 必须按企业级宣传入口维护** — README 是项目对外第一入口，后续涉及产品能力、UI、Serial Station、协议、构建或启动方式变化时，必须按 [02-workflow §4.1](docs/constraints/02-workflow.md#41-readme-企业级宣传标准) 检查是否同步更新，禁止只堆命令、空泛口号或无法由代码/测试/文档证明的宣传点
5.9. **禁止把工程存在当作用户完成** — 功能状态必须同时标记工程状态、用户可用状态和设备验证状态；没有入口、没有用户路径或没有真实/替身验证的能力，不得写成“已完成”
5.10. **每轮迭代完成必须提交** — 一个 PRD/Specs/批次子任务收口后必须 commit；不能把多轮迭代长期堆在工作区。若因构建失败、用户已有改动或环境缺失暂不能 commit，必须在收口说明中写清阻塞原因和下一可提交点

### 架构铁律
6. **分层单向依赖**: 表现层→业务层→数据层→基础设施层，**禁止反向**
7. **禁止在MainWindow中写业务逻辑** — 委托给Controller/Manager
8. **PyQt 主窗口模块 ≤ 300行** — 超过必须拆分
9. **公共组件只写一次** — CRC/HexConverter/RingBuffer/SettingsManager等已验证组件不得重写
10. **模块间依赖必须遵循 [03-architecture.md](docs/constraints/03-architecture.md) 的依赖方向规则** — 禁止反向依赖、禁止同层横向依赖、禁止跨层跳级
10.5. **Serial Station必须遵循 [docs/serial_station_architecture.md](docs/serial_station_architecture.md)** — UI、core、protocols、services、workers 边界必须隔离；core 不依赖具体协议，协议不依赖 UI
10.6. **并行开发必须先切边界** — 多 Agent 并行只允许在文件、接口、验收命令和合流顺序都写清后启动；共享入口、`pyproject.toml`、`uv.lock`、启动脚本默认单 Agent 串行

### 编码铁律
11. **Python 3.10+ 标准** — import 顺序: 标准库→第三方→项目内部
12. **PyQt 信号/槽集中装配** — UI 只发意图，controller 编排状态，不跨层随意连接
13. **必要中文注释/docstring** — 公开类和复杂公开方法必须说明职责
14. **QObject 生命周期明确** — QWidget/QObject 使用 parent 体系，后台任务不直接更新 UI

### UI铁律
15. **禁止在 PyQt 逻辑中散落硬编码颜色** — 颜色和状态样式集中管理
16. **所有QWidget必须设置objectName** — QSS依赖
17. **按钮必须有hover/pressed/disabled三种状态**
18. **面板切换必须有过渡动画** — 禁止突然出现/消失
19. **所有用户可见文字必须走翻译入口或集中常量**

### 文件体积铁律
20. **.py ≤ 300行** — 超过说明职责过多
21. **测试文件聚焦单一行为域，≤ 250 行** — 超过应拆分为 unit/integration/ui_smoke。同域小文件必须合并（2026-06-22 精简：168→112 文件）
22. **单个方法 ≤ 80行** — 超过说明逻辑过于复杂

### 测试文件组织规则（2026-06-22 新增）
23. **测试文件按域分组** — 同域测试合并到单文件（如 `test_controller_state_core.py` 含 connection/callback/workbench state）。禁止同域散落 >3 个文件。
24. **文件命名 `test_<域>_<子域>.py`** — 如 `test_dashboard_layout.py`、`test_animations_factories.py`、`test_theme_core.py`。避免 `test_<单个控件>.py` 孤儿文件。
25. **commit 前必须跑全量 unit + smoke** — 不能只跑 smoke（Oracle 审计 2026-06-22 发现：仅 smoke 通过但 unit 有 3 个回归未发现）。

---

## 交付状态口径（不可省略）

> 本节是后续项目审计、README 宣传、PRD 验收和并行 Agent 合流的统一口径。
> 任何功能都不能只写“已实现”。必须同时说明工程状态、用户可用状态和设备验证状态。

### 三轴状态

| 轴 | 说明 | 允许状态 |
|----|------|----------|
| 工程状态 | Python 代码、测试、启动、打包、架构边界是否落地 | `E0未开始` / `E1设计中` / `E2骨架` / `E3已接入主线` / `E4自动化测试通过` / `E5可维护收口` |
| 用户可用状态 | 用户是否能从主界面或明确入口完成真实操作路径 | `U0不可见` / `U1可见不可用` / `U2局部可用` / `U3主流程可用` / `U4体验完整` |
| 设备验证状态 | 是否经过替身、虚拟设备或真实硬件验证 | `D0未验证` / `D1纯单测` / `D2替身/模拟验证` / `D3虚拟设备验证` / `D4真实设备验证` |

### 完成定义

- **工程完成**：至少达到 `E4`，且 Python 测试、启动 smoke 和必要打包验证可运行。
- **用户完成**：至少达到 `U3`，用户能按 README 或界面入口完成核心路径。
- **设备完成**：至少达到 `D3`；涉及 UART、RTT、CAN、BLE、USB、SPI/I2C 等外设时，未到 `D4` 必须明确写“真实设备未验证”。
- **对外宣传完成**：必须同时满足 `E4 + U3 + D2`；如果文案暗示真实硬件可用，必须满足 `D4`。
- **大厂级可维护完成**：除上述状态外，还必须满足模块边界清晰、公共能力复用、测试覆盖关键路径、失败路径可诊断、README/PRD/约束文档一致。

### 禁止表述

- 禁止只写“已实现”“完成”“骨架完成”而没有三轴状态。
- 禁止把 stub、TODO、空方法、只写 UI 壳层描述为用户可用。
- 禁止把没有入口的模块描述为用户已可使用。
- 禁止把没有硬件、虚拟设备或替身测试的外设能力描述为设备已验证。

---

## 快速参考

| 项 | 值 |
|----|-----|
| 应用名称 | EmbedDebug |
| 项目路径 | `E:\Embedded\Tool\Serial_tool\User_Serial` |
| 当前版本 | 0.1.0 |
| 评分 | 774（见 [docs/tracking/SCORE_TRACKING.md](docs/tracking/SCORE_TRACKING.md)） |
| Git分支 | `feat/embed-debug` |
| Git远程 | `https://github.com/ParacosmYy/GS_Tool.git` |
| 测试文件数 | 170（Batch 49-103 新增 58 个测试文件） |
| 测试通过 | 2034 passed, 2 skipped, 0 failed |

### Python/PyQt 命令
```powershell
uv run start-embeddebug
uv run test-embeddebug-py
uv run package-embeddebug --version local --clean
uv run verify-package-embeddebug --package-dir dist\EmbedDebugPy-local-windows-x64
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

> 以下历史审计仅保留迁移背景；当前实现以 `python/embeddebug/`、`tests/python/` 和 PRD-135/136 为准。
> 旧状态标记仅保留历史阅读，不再作为完成依据。后续必须使用“工程状态 / 用户状态 / 设备验证”三轴口径。
> 更新: 2026-06-03 二次审计 — UI-01~UX-03已全部实现，审计表已更正

---

### 0、状态审计强制修正规则

1. 本节历史表格中的 ✅ 只能理解为“当时认为已实现”，不能直接作为当前完成证明。
2. 后续新增或修正条目必须新增三列：`工程状态`、`用户状态`、`设备验证`。
3. `说明` 必须写证据路径，例如源码路径、测试路径、README 入口、验证命令或硬件验证记录。
4. 对 UART、RTT、CAN、BLE、USB、SPI/I2C、MQTT 等外设/连接能力，未经过真实设备或明确替身验证时，不得写“完整可用”。
5. 如果一个功能只有骨架、stub、TODO、空方法或无主界面入口，应优先列入“下一轮迭代缺口”，而不是放在“已完成”。

### 0.1 当前重点能力重判（双状态口径）

| 能力 | 工程状态 | 用户状态 | 设备验证 | 当前口径 |
|------|----------|----------|----------|----------|
| Serial Station UART 基础工站 | E4 自动化测试通过 | U2 局部可用 | D1 纯单测 | 当前证据位于 `python/embeddebug/serial_station/` 与 `tests/python/`，仍需真实/虚拟串口收发验证和用户路径验收 |
| Serial Station 协议选择/ASCII/Modbus/Custom MD | E4 自动化测试通过 | U2 局部可用 | D1 纯单测 | 协议构建和解析有 pytest 覆盖，真实设备协议链路未验证前不得宣传为硬件调试完整闭环 |
| SEGGER RTT | E2/E3 骨架或构建接入 | U1 可见不可用 | D0 未验证 | J-Link SDK stub 或未真实 DLL 调用时，只能标为“待真实集成” |
| BLE/CAN/MQTT/SPI/I2C/USB 扩展 | E2-E4 视模块而定 | U1-U2 视入口而定 | D0-D1 默认 | 有代码框架不等于用户完成，必须逐项补三轴状态和验收证据 |

---

### 一、UI 基础设施 (优先级: 最高)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| UI-01 | **BasePanel 容器组件** | ✅ | PRD-061 | 已实现: 标题栏/折叠/动画/阴影/空状态/骨架屏(509行) |
| UI-02 | **IconManager SVG图标系统** | ✅ | PRD-062 | 已实现: SVG着色管线+缓存(138行)。lucide 图标库 146 个 SVG（2026-06-22 扩展完成） |
| UI-03 | **EmptyStateWidget/LoadingSpinner/SkeletonWidget** | ✅ | PRD-063 | 已实现: 三个组件全部完成(EmptyState 196行/Spinner 135行/Skeleton 115行) |
| UI-04 | **CommandPalette 命令面板** | ✅ | PRD-064 | 已实现: 模糊搜索+半透明遮罩+事件过滤(298行) |
| UI-05 | **IconNavBar 三栏布局** | ✅ | PRD-065 | 已实现: 分类按钮+滑动指示器(133行) |
| UI-06 | **QSS主题生成器** | ✅ | PRD-066 | 已实现: qss_builder 程序化生成 + palette/tokens 分离 + 12 个 qss_sections_* 模块化段 |

### 二、交互体验增强 (优先级: 高)

| # | 待办项 | 状态 | PRD | 说明 |
|---|--------|------|-----|------|
| UX-01 | **SendCompleter 智能补全** | ✅ | PRD-067 | 已实现为SmartAutoComplete(234行): 前缀匹配+频率排序+弹出列表 |
| UX-02 | **ScriptRecorder/Player 脚本录制回放** | ✅ | PRD-068 | 已实现: 记录/发送/回放+JSON保存(193行) |
| UX-03 | **DataDiffWidget 数据对比** | ✅ | PRD-069 | 已实现: HTML diff渲染+颜色标记(318行) |
| UX-04 | **键盘快捷键体系完善** | ❌ | 05-ui-standard §十五 | Ctrl+F/Ctrl+P/Ctrl+Shift+R/Ctrl+Enter 等全局快捷键尚未全面实现 |
| UX-05 | **响应式布局** | ❌ | 05-ui-standard §十七 | 窗口<900px自动折叠导航树、断点动画过渡。窗口resize时布局适配 |
| UX-06 | **弹窗/对话框体系统一** | ✅ | 05-ui-standard §十六 | 已实现: QMessageBox 全部替换为自定义弹窗（2026-06-22 审计 0 处残留） |

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
1. ~~UI-06 QSS主题生成器~~ ✅ 已完成（qss_builder 程序化生成）
2. ~~UX-04 键盘快捷键体系~~ ✅ 已完成（shortcuts/ 包：ShortcutDef + ShortcutCategory + DEFAULT_SHORTCUTS 14 快捷键 + manager.py）
3. ~~UX-05 响应式布局~~ ✅ 已完成（responsive_layout.py + main_window._install_responsive_layout 断点系统）
4. ~~UX-06 弹窗体系统一~~ ✅ 已完成（QMessageBox 0 残留）

#### 第二优先级: 骨架功能完善
5. ~~MQTT TopicModel~~ ✅ 已完成（改用 client_stub + codec 模式，未引入空壳 QAbstractItemModel）
6. ~~BLE GattModel~~ ✅ 已完成（改用 gatt + transport_stub 模式，未引入空壳 model）
7. **USB libusb集成** → 11个TODO stub实现
8. ~~Dashboard布局持久化~~ ✅ 已完成（canvas.save_layout/load_layout + to_layout_dict JSON 序列化）
9. **ProtocolEngine CRC校验** → 协议完整性验证
10. ~~DBC解析器~~ ✅ 已完成（can/dbc.py，145 行，11 函数/类）

#### 第三优先级: 图标+代码质量
11. ~~Lucide SVG扩展~~ ✅ 已完成（14→146 个 SVG）
12. ~~Doxygen补全~~ ✅ 已完成（340/348 模块 docstring，98% 覆盖）
13. ~~tr()合规审计~~ ✅ 已完成（test_tr_compliance 守护，0 违规）
14. ~~objectName审计~~ ✅ 已完成（tools/ 4 文件修复 + qss_sections_tools 契约）

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

---

## 会话成果存档 (2026-06-22)

### Batch 40-46：UI/动画质量提升 + 测试精简

> 21 commits（`016dfab62` → `c5992d0ee`），评分 665 → 677，测试 1421 → 1601 passed。

#### 环境修复
- `%APPDATA%\uv\uv.toml` 配置清华 PyPI 镜像；`uv.lock` 254 个 URL 重写为 `pypi.tuna.tsinghua.edu.cn`（`uv sync` 从卡死 → 32 秒）

#### 新增模块（22 个）
- **Batch 40**: 5 动画（bounce_path/glow/rotate/typewriter/elastic_snap）+ 5 控件（chip/segmented/rich_tooltip/info_banner/progress_ring）
- **Batch 41**: Drawer / Badge / SkeletonAnimation / PageSlideAnimation / CrcCalculatorPanel
- **Batch 42**: TimestampConverterPanel / HexViewerPanel / ByteFrequencyAnalyzer
- **Batch 43**: ColorTweenAnimation / StaggerCoordinator / ThemeSerializer / 2 守护测试

#### UI 审计 P0 修复
- **35+ 按钮补齐 :pressed + :disabled + :focus**（原仅 ~4 个按钮有三态）
- **35 按钮补齐 lucide 图标**（原仅 13 个）
- **Toast Unicode 字形 → SVG**（info/check-circle/alert-triangle/x-circle）
- **INFO 语义色独立**（#3b82f6 蓝，不再复用 ACCENT 青）
- **QComboBox CSS 三角箭头 + QPushButton :focus 键盘焦点**
- **QTabBar :hover + :selected:hover**
- **light theme 补 6 个 CARD_* token**（CARD_SHEEN_*/INNER_TOP_EDGE/GROUND_SHADOW/HOVER_RING）
- **ConfigurableButton ripple 默认开启 + :disabled**
- **Splitter handle 2→4px**

#### 动画审计 P0 修复
- **AppShell 离场动画隐形 bug 修复**（`setCurrentIndex` 延迟到 `QTimer.singleShot(160ms)` 后执行，离场 fade_out 可见）
- **连接成功 pop 动画**（ScaleAnimation.pop on disconnect 按钮）
- **Dashboard tab 切换淡入**（FadeTransition.fade_in）
- **6 个 easing/duration token 新增**（EASE_OUT_QUART/QUINT/MATERIAL_EMPHASIZED + DURATION_CONTAINER + KEYFRAME_PREVIEW + STAGGER_STEP_MS）

#### 测试精简
- **168 → 112 文件**（-56，按域分组合并，4 个并行 agent）
- 合并规则：同域小文件合并到 `test_<域>_<子域>.py`，每个 ≤250 行

#### 研究交付（3 份审计）
- VOFA+ / MobaXterm UI 模式调研（librarian）：appTheme token 系统 + RGB 值 + 8 层 50+ 项审计清单
- UI gap 审计（explore）：30 按钮缺状态 / 27 缺图标 / Toast Unicode / light theme 缺 token / 无 INFO 色
- 动画 gap 审计（explore）：AppShell leave 隐形 / dead code 清单 / cross_fade sequential bug / stagger_fade GC 风险
- 完整路线图：[docs/superpowers/specs/2026-06-22-batch43-ui-animation-polish-iter.md](docs/superpowers/specs/2026-06-22-batch43-ui-animation-polish-iter.md)

### Batch 49-102：永续迭代（加载态/死代码/CI/lint/测试覆盖）

> 54 commits，评分 677 → 774（+97 from 677 / +71 from 703），测试 1601 → 2034 passed（+433 / +356）。

#### Batch 49 加载态 + 空态（6 子任务，22 ui_smoke）
- EmptyStateWidget.hide_with_fade 对称方法
- Log/Waveform/Dashboard 空态 + 加载态全覆盖
- Connection ProgressRing 内嵌 + 4 条连接路径同步

#### Batch 50-62 lint 严格化（六系列）
- F（Pyflakes）+ UP（pyupgrade）+ B（bugbear）+ SIM（simplify）+ RUF（ruff-specific）+ C4（comprehensions）
- 清理 97 未使用 import + 5 未使用变量 + 81 处语法现代化 + 16 bugbear 修复
- GitHub Actions CI（Windows runner，push/PR 跑 test + STRICT 模式 + lint + smoke）

#### Batch 51-53 死代码治理（widget/token 清零）
- 7 个死 widget 全部 wire（Drawer/Badge/Chip/InfoBanner/Segmented/ToggleSwitch/Divider）
- 8 个预留动画 token 消费（DURATION_CONTAINER/FLOUT/DRAWER/PROGRESS + EASE_OUT_QUART/QUINT/IN_QUART/OUT_QUAD）
- widget 白名单 10→2（设计内），token 白名单 9→1（待场景）

#### Batch 66-96 测试覆盖（+356 测试）
- MQTT client/codec/message + BLE codec/gatt/transport + CAN frame/dbc
- SPI/I2C codec/config/bridge + RTT protocol + OTA config/protocols/engine
- GPS model/parser + SVD model/parser + EyeDiagram model + HexFormatter
- PerfMetric/Snapshot + RecordingTimeline/Player/Format + OperationResult
- Plugins discovery + Project audit + Controller helpers + Transport connections

#### Bug 修复（3 个）
- HexFormatter format_int big_endian 参数被忽略（struct.unpack 总用 little-endian）
- smoke _connect_button AttributeError（connection_toolbar getattr 防御）
- devtools 自测目录名硬编码（GS_Tool → 接受 User_Serial）

#### 文档全面更新
- CLAUDE.md ROADMAP 审计：第一优先级 4 项全完成（UX-04/05/UI-06/DBC）
- README 工程状态 E4→E5 + CI badge + 质量门禁 lint
- SCORE_TRACKING 回填 54 个 batch 条目

#### 后续路线图
- **Batch 47**: 死代码激活（RichTooltip install / Skeleton windowOpacity→QGraphicsOpacityEffect / RotateAnimation spinner）
- **Batch 48**: 排版 token（FONT_ROLE_* / LETTER_SPACING_* / LINE_HEIGHT_*）+ 间距 token 统一
- **Batch 49** ✅: 加载态（Connect/Refresh ProgressRing）+ 空态（log/waveform/dashboard）— 6 子任务，22 ui_smoke 测试，1695 passed
- **Batch 50** ✅: 死代码守护测试（test_no_dead_widgets + test_animation_wiring）+ CI 严格化（GitHub Actions + ruff + pytest config）— 6 子任务，8 守护测试，1686 passed，lint clean
- **Batch 51**: (待规划)

---

### Batch 47 成果（2026-06-22，score 677→691，18 commits）

#### 死代码激活（7 个模块 → 全部接入生产线）
- SkeletonAnimation：windowOpacity → QGraphicsOpacityEffect（修复内嵌控件不可见 bug）
- BouncePathAnimation → canvas.add_widget_at（dashboard 控件放置弹入）
- TypewriterAnimation → status_messages.set_status_text（状态栏打字机效果）
- GlowAnimation → connection_control_state（连接成功脉冲发光）
- ElasticSnapAnimation → _dashboard_widget_menu._edit_properties（属性编辑弹性归位）
- RichTooltip/install_tooltip → connection_toolbar（4 个工具栏按钮富文本 tooltip）
- InfoBanner/Chip 关闭 × → lucide x SVG（从手绘线条迁移）

#### UI 审计 P0 修复
- Dashboard canvas 空态拖拽提示（paintEvent 居中文本）
- Log view placeholder 改为 actionable 引导
- **全连接路径加载态**：fake/serial/tcp/udp connect + refresh ports + BLE scan 均有 _set_loading 反馈

#### Token 迁移（全应用字体/间距硬编码清零）
- font-size:NNpx → FONT_* tokens（value_display/slider/waveform_overlays/dashboard palette）
- setPointSize(N) → FONT_POINT_* tokens（empty_state/value_display/led/gauge + 4 tools）
- setContentsMargins/setSpacing 硬编码 → SPACING_INT_* tokens（layout_cards/top_bar/sections/layout_main/toast）
- tokens.py 新增：FONT_POINT_LARGE/HEADING/BODY/TINY + SPACING_INT_XS~2XL
