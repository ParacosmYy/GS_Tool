# ADR 0025：企业级模块化架构与质量门禁

状态：已接受（2026-08-10）  
范围：SerialForge Python/PySide6 桌面端的后续演进、目录边界、依赖方向和交付门禁。

## 背景

SerialForge 已经从单一串口窗口扩展为 UART、TCP/UDP、BLE、RTT、协议解析、派生数据、回放和
命令管理工作站；后续还要承载 OTA、升级安全和调试输出扩展。继续把功能直接追加到 `MainWindow`
会形成隐式共享状态、跨模块控件访问和难以回滚的“大类”。用户要求采用企业级、可拓展、低耦合的
工程范式，并把每个源码文件控制在 1000 行以内。

“参考字节跳动”在本 ADR 中只解释为借鉴公开可见的工程共性：明确 owner、稳定契约、分层依赖、
组合根、可观测质量门禁、渐进式迁移和可回滚交付；不把任何未公开的公司内部规范或流程冒充为
本项目事实。

## 决策

### 1. 以变化边界划分 bounded context

每个上下文有唯一 owner，跨上下文只能通过 domain/application DTO、port 或 presentation contract
交换数据：

| 上下文 | owner | 允许职责 | 明确禁止 |
|---|---|---|---|
| Connection/Session | `application/sessions.py`、`session_router.py` | 会话生命周期、取消、连接能力 | Qt 控件、密钥、协议字段渲染 |
| Transport | `domain/ports.py`、`infrastructure/*transport.py` | UART/网络/BLE/RTT I/O adapter | 业务状态判断、直接通知 Qt |
| Protocol | `domain/protocols.py`、`application/protocols.py` | framing、checksum、source-aware pipeline | socket、文件、UI |
| Component/Dataset | `domain/components.py`、`datasets.py`、对应 application worker | 声明式派生、有限 transform、统计 | 用户脚本、动态导入、传输控制 |
| Replay | `domain/replay.py`、`application/replay.py` | 严格 JSONL RX ingress、时间轴、历史来源 | 连接设备、TX、伪造实时来源 |
| Commands | `domain/commands.py`、`application/commands.py` | 有界 immutable batch、可取消 enqueue | 脚本、循环、ACK 假设、广播 |
| Presentation | `presentation/controllers/*`、`viewmodels.py` | UI 组合、状态投影、主题和可访问性 | pyserial、socket、设备句柄、密钥 |
| OTA/Debug extensions | `ota/*`、`debug/*` | contract-only 的升级/日志扩展槽位 | 未验收硬件操作、vendor SDK 反向渗透 |

### 2. 依赖只向内，组合根负责装配

依赖方向固定为：

```text
presentation controllers ──> presentation contracts / application DTO
application services     ──> domain ports / domain models
infrastructure adapters  ──> domain ports
composition root         ──> concrete implementations + ViewModel + MainWindow
```

`src/serialforge/composition.py` 是运行时依赖组合根；`presentation/controllers/bootstrap.py` 只
负责桌面窗口初始化、Qt shell 组装、信号接线和首屏投影。`presentation/main_window.py` 只保留
`QMainWindow` 生命周期入口、公开委托接口和组合调用，不再承载业务流程实现。

当前 controller 迁移采用显式 `window` 边界以保持行为可追溯；这属于兼容阶段，不得继续扩大。
新功能必须优先增加最小 typed contract，下一阶段按 feature state/action DTO 逐步去除完整
`MainWindow` facade 依赖。禁止通过 `MainWindowContext`、无限制 `dict`、mixin 或全局单例绕过该规则。

ARCH-6 采用 owner-module contract 渐进迁移，而不是一次性引入 typed facade object 或 mixin。首个
`ARCH-6a` 切片已将 UART/TCP/UDP/RTT/BLE 的纯配置 builder 和 BLE UUID 解析移出 MainWindow 转发：
`connection_runtime.py` 直接调用本模块函数，`ble.py` 直接调用 BLE discovery builder；保留 Qt callback、控件
所有权和生命周期入口。后续再按 workspace/terminal/commands/protocol family 建立更窄的 feature DTO。

`ARCH-6b` 继续采用同一 owner-module contract：删除仅由 terminal runtime 自身调用的
`MainWindow._current_entry` 与 `MainWindow._render`，由 `terminal_runtime.py` 直接调用 `current_entry()` 与
`render()`。该切片不触及仍承担跨区 signal、Qt parent、Tab 或 lifecycle 接线的 terminal wrapper；不能把后续
需要完整窗口上下文的 runtime 访问一次性伪装成 typed DTO。

`ARCH-6c` 将 composition/workspace 的纯转发层继续收敛。`bootstrap.py` 直接调用 header、error、terminal、send、shortcut 和
Tab 组装 owner；`workspace.py` 直接组合现有 connection/protocol/send page builder，并直接连接自身的 Tab change/transition
函数；`connection_builder.py` 直接使用 `composition.py` 的 bounded `enum_combo()`/`timeout_spin()`。`lifecycle.py` 直接调用
`workspace_runtime.py` 的 `workspace_motion_enabled()`、`stop_workspace_transition()` 和 `on_workspace_tab_changed()`。因此 `MainWindow` 不再暴露这批
页面构建、滚动页、焦点安装或工作区动效 facade，Qt parent、widget ownership、signal 顺序、主题/动效策略和业务回调保持不变。
该切片不扩展到仍承载 ViewModel signal/cross-feature 状态的业务 wrapper；后续按稳定 feature contract 继续迁移。

`ARCH-6d` 继续以最小动作边界迁移连接按钮：`contracts.py` 暴露 `ConnectionActionCallback`，`workspace.py` 将
`connection_runtime.toggle_connection` 作为显式 callback 注入 `connection_builder.py`；builder 只负责把 callback 接到
Qt button signal，`toggle_connection()` 在 owner action 边界吸收 Qt 的 checked/payload 参数。这样删除了
`MainWindow._toggle_connection` 纯转发，同时保留现有 session gate、错误入口、按钮文案、parent/lifetime 和关闭时序；
没有把连接配置 DTO 或 transport 实现引入 composition/workspace。

`ARCH-6e` 继续删除两个无状态的展示格式 facade：`endpoint_label()` 和 `history_label()` 从
`controllers/lifecycle.py` 迁移到 `presentation/formatters.py`，该模块只依赖 domain DTO，不持有窗口、Qt
对象或 controller；`connection_runtime.py` 与 `terminal_runtime.py` 直接导入 formatter。这样避免让独立
controller 通过 `MainWindow` 间接获取纯函数，也避免 `lifecycle.py` 与 connection runtime 形成反向循环依赖。
该切片删除 `MainWindow._endpoint_label` / `_history_label`，不改变端点身份、历史预览、signal 或生命周期语义。

`ARCH-6f` 将动态 QSS 属性刷新从 `MainWindow._set_dynamic_property` 收窄为
`presentation/property_refresh.py:refresh_dynamic_property()`。该原语只依赖 Qt widget，负责“值未变化则跳过，
值变化则 setProperty + unpolish/polish + update”；bootstrap 直接注入给 `StatusSurfaceController`，connection、
commands、terminal 和 lifecycle controller 显式导入它。删除 facade 不改变状态投影、主题 selector 命中、Qt signal 或
生命周期，仅移除窗口作为纯 UI 原语的隐式中转。

`ARCH-6g` 按 owner 归还两个单调用点：`status_surface_source()` 进入 `controllers/status_surfaces.py`，bootstrap
通过显式 source callback 注入；`tcp_server_readiness()` 进入 `controllers/connection.py`，连接 gate 直接调用，保留
IPv4、回环/LAN 确认、allowlist 数量和原有中文错误文案。删除 `MainWindow._status_surface_source` /
`_tcp_server_readiness` 以及 lifecycle 中错误归属的转发，不改变连接按钮、LAN 安全条件、状态 surface source 或生命周期。

`ARCH-6h` 将 `current_send_mode()` 与 `selected_command_batch()` 从 commands controller 的兼容入口移到
`presentation/command_selection.py`。该模块只读取 bounded Qt combo 的 current data，保留 `CommandMode` fallback 和
`Qt.ItemDataRole.UserRole` 类型检查；commands、connection、terminal runtime 显式依赖它，删除
`MainWindow._current_send_mode` / `_selected_command_batch`，不产生 controller-to-controller 依赖或改变发送/批量 gate。

`ARCH-6i` 将发送与批量按钮的 Qt signal payload 吸收到 commands owner 的 `*_action()` adapter；terminal builder
使用 `partial(owner_action, window)` 接入发送、批量按钮，composition 的 Ctrl+Enter 也直接接入同一发送 action。
删除 `MainWindow._send_current`、`_new/_edit/_delete/_run_command_batch` 五个 facade；原有 dialog parent、ViewModel
batch command、发送 gate、快捷键和生命周期保持不变，checked/payload 不进入 domain 或 application。

`ARCH-6j` 继续删除 commands signal/结果转发 facade：commands controller 内部直接调用
`on_command_batch_selection_changed()` / `render_command_batch_results()`；terminal 的 batch combo 直接注入
selection handler，bootstrap 直接注入 ViewModel batch signals 并用同一 owner 函数完成初始投影。删除
`MainWindow._on_command_batches_changed`、`_on_command_batch_selection_changed`、`_on_command_batch_changed`、
`_render_command_batch_results`，保持 signal 顺序、首屏 snapshot、batch 状态轨道和 connection controls 更新语义。

`ARCH-6k` 将 workspace 的运行态从 builder 中分离：`workspace_runtime.py` 单独负责 Tab 可见性、派生 surface 挂起、共享 motion
policy 和一次性过渡；`workspace.py` 只创建控件和接线，`lifecycle.py` 直接调用 runtime helper。该拆分修复了 motion callback
直连后 `workspace ↔ lifecycle` 的循环导入，避免用动态 import 或 MainWindow facade 遮盖依赖；主题切换、共享 frame、低动效、暂停
以及隐藏/最小化/关闭清理保持原语义。

`ARCH-6l` 将 BLE 写入模式的 bounded selector 与 capability projection 收敛到 `presentation/ble_selection.py`；`ble.py` 保留
会修改 combo 的 normalize action，`connection.py` 与 `commands.py` 直接读取 selector，删除 MainWindow 的 3 个 BLE helper facade。
该模块只依赖 domain DTO 与 Qt combo data，不持有 ViewModel、transport 或设备句柄。

`ARCH-6m` 将 `parser_pipeline_supported()`、`derived_source_supported()` 和 `derived_source_unavailable_text()` 收敛到
`presentation/protocol_scope.py`。该模块只读取 transport/history presentation facts；protocol editor 的 enable/reset/status
副作用仍由 `protocol_config.py` 拥有，connection/lifecycle/derived/workspace/terminal 直接消费查询。删除 MainWindow 的 3 个
protocol scope facade，不改变 UART/TCP Client 派生 gate 或 UDP/raw-only 语义。

### 3. Presentation controller 按稳定变化理由拆分

当前目录按以下边界维护：

```text
presentation/controllers/
├─ bootstrap.py          窗口状态注入、shell 组装、ViewModel signal wiring
├─ composition.py        页面包装、协议面板接线、快捷键和焦点顺序
├─ workspace.py           顶栏、Tab、滚动页和显式 callback 接线
├─ workspace_runtime.py   Tab 可见性、派生 surface 挂起和一次性切换过渡
├─ connection_builder.py 连接配置控件构建
├─ connection_presets.py 连接 preset DTO 与内置 catalog 应用
├─ connection_runtime.py 连接/传输配置快照与连接动作
├─ connection.py         连接状态投影和控件 gate
├─ protocol.py            Protocol/Component/Dataset/Curve 视图构建
├─ protocol_config.py    framing、preset、timing 和协议编辑提交
├─ derived_data.py       Component/Dataset/Curve 状态投影与渲染调度
├─ terminal.py            终端、错误栏和发送控件构建
├─ ../terminal_surface.py 终端空态自绘装饰与上下文空态 presentation widget
├─ ../command_batch_surface.py 批量步骤轨道的 presentation projection renderer
├─ ../replay_activity_surface.py 回放活动轨迹的 presentation projection renderer
├─ ../analysis_status_surface.py 协议/Component/Dataset/Curve 通用状态轨道 renderer
├─ ../theme_palette_surface.py 主题语义色点阵装饰 renderer
├─ ../connection_status_surface.py 连接带四节点状态路径 renderer
├─ ../connection_preset_codec.py schema v1 preset catalog 白名单 codec
├─ ../connection_preset_store.py preset catalog port 与唯一 QSettings adapter
├─ terminal_runtime.py   终端、记录、历史和预览事件投影
├─ commands.py            批量命令 UI 动作与结果表
├─ replay.py              回放动作和状态投影
├─ ble.py                 BLE 扫描、通知和特征控件动作
├─ lifecycle.py           主题、动效、错误、状态和 Qt 生命周期
└─ status_surfaces.py     状态表面注册与动态属性刷新
```

controller 不直接互相读取 widget；确需跨区协作时，通过 `presentation/contracts.py` 的最小回调、
ViewModel signal 或 application command 完成。Qt 对象的 parent/lifetime、关闭 fence、隐藏/最小化
暂停策略必须显式记录。

终端的空白区域由 `presentation/terminal_surface.py` 提供 presentation-only 空态层：它只接收
`terminal_runtime.py` 从已有 preview buffer、SessionState、历史来源和 paused 事实投影出的状态，不持有
业务状态、不触碰 transport/application、不创建独立时钟；有界 RX preview 一旦存在，空态层必须隐藏。
实时观测与发送工具栏可以按窗口宽度使用两行 `QGridLayout`，但必须保持现有控件对象名、signal、发送 gate、
键盘顺序和共享 MotionController，不把响应式布局判断扩散到 domain/application。

协议/遥测页面进一步按稳定变化理由划分四个 presentation surface：协议配置、组件遥测、Dataset/曲线和历史回放。
这些 `QFrame[role="surface"]` 只提供静态层级与主题背景，不复制状态、不控制业务可见性、不持有 callback 或 timer；
内部工具条使用明确的 `QGridLayout`、bounded dynamic label/combo 和 stretch，必须保留
`ProtocolPanelWidgets`、callback、widget reference 与显式 Tab 顺序。这样可以修复窄宽度几何重叠，同时不把
响应式策略泄漏到 domain/application。

协议流水线摘要继续保留为 `QLabel#pipelineSummary` 的稳定文本/可访问契约；其装饰轨道由
`presentation/pipeline_surface.py:PipelineSurfaceLabel` 提供。该子类只消费主题 token 和 bootstrap 已接入的共享
`MotionController` frame，不能读取或复制 `source/state`、不能持有 controller、创建 timer 或改变文字/tooltip/
AccessibleDescription。lifecycle 的 reduced-motion、暂停、隐藏、最小化和关闭分支必须显式停止轨道，保留静态回退。

顶栏 `StatusIndicator` 是同一 presentation 动效边界内的纯几何 renderer：它只消费 lifecycle 已投影的
`SessionState` 和共享 frame，使用当前 `ThemeSpec` 绘制 discovered/opening/open/closing/closed/error 的静态/动态
标记，不增加业务分支、timer、字段或后端依赖。尺寸固定为 22×22，Qt arc 角度转换和绘制 inset 必须保持边界安全。

批量命令和历史回放的动态反馈也必须保持 projection-only。`CommandBatchSurfaceLabel` 接收
`commands.py` 从既有 `CommandBatchSnapshot` 派生的有界步骤投影，在最多 32 个 slot 内表达 pending/current/accepted/failed，
不把业务进度复制进 UI；`ReplayActivityLabel` 接收 `replay.py` 的状态与已发出记录计数，只绘制活动轨迹，不显示不存在的
总记录数或百分比。两个标签不访问 ViewModel/domain/transport，不创建 QTimer，保留 `QLabel#commandBatchStatus`、
`QLabel#replayStatus` 的文字和无障碍描述，并由 `lifecycle.py` 的统一 motion fan-out 负责 frame/stop、隐藏、最小化、暂停、
低动效和关闭回退。

四个协议派生状态条复用 `presentation/analysis_status_surface.py:AnalysisStatusLabel`。该通用 renderer 只读取
`StatusSurfaceController` 已经投影到 QLabel 上的 `source/state` 动态属性，保留各 owner controller 设置的文字、tooltip
和 AccessibleDescription；它不引入 projection DTO、domain snapshot 或独立时钟，只在安全底部区域绘制状态信号轨道。
`protocol.py` 负责四个 label 的构造，`lifecycle.py` 负责共享 frame/stop fan-out；未知状态静态回退，replay 的专用
`records_emitted` projection 继续保持独立。QSS 仍以 `QLabel#...` 稳定 selector 命中 QLabel 子类，必须用运行时样式冒烟和
三主题、低动效、窄宽度证据覆盖该假设。

主题选择器旁的 `ThemePaletteSwatch` 是独立的 presentation-only 装饰 renderer：固定 62×22 逻辑像素，只从父链
`ThemeSpec` 读取 accent/accent_blue/accent_purple/accent_pink/surface_input/border 语义 token，不复制主题 DTO、
不添加主题 signal/QTimer、不进入 Tab 顺序或无障碍树。`apply_theme()` 已负责根属性和子控件 update，lifecycle 负责共享
frame/stop 以及隐藏、最小化、暂停和关闭回退；若 980px 头部布局无法容纳，应调整布局边界，不得强行压缩 combo 文案。

`UI-1.33` 的 `ConnectionStatusRail` 是连接带的 presentation-only 装饰 renderer，固定在独立底行，绘制端点→传输→会话→
数据四节点路径。`lifecycle.update_state_badge()` 通过 `set_state()` 注入已有 `SessionState`；renderer 不读取 ViewModel/domain、
不复制 RX 计数、不暗示真实数据进度、不创建 timer 或持有 `MotionController`，只消费共享 frame 的 `set_frame()/stop()`。
它必须从 `ThemeSpec` 取色，使用逻辑坐标，并在 980/1180、三主题、HIDPI、隐藏/最小化/关闭和静态停止回退下保持现有连接
按钮、信号、无障碍文案和 Tab 顺序不变。

`UI-1.34` 在既有 `TerminalEmptyState` 的父层增加低对比度观测背景：网格、四角 bracket、状态色 scan line 和五节点 pulse
只能表达“空态画布已准备”，不能表达 RX 计数、吞吐、时间或连接进度。父层必须先完成 QFrame 基础绘制，再在裁剪后的
`contentsRect()` 中绘制，子卡片、glyph、文字和 accessibility 继续由子控件负责；`set_frame()/stop()` 只同步已有共享
MotionController 帧，禁止新增 timer、控件、状态源和 lifecycle 接线。三主题、980/1180、HIDPI 与生命周期静态回退是该组件的
固定验收项。

`UI-1.35` 的主题切换过渡位于独立 `presentation/theme_transition.py`，不属于 ThemeSpec 或业务状态。`apply_theme()` 先完成
主题切换，helper 只对 bootstrap 持有的 `appRoot` 施加一次性、短时的 `QGraphicsOpacityEffect/QPropertyAnimation` 淡入；
`lifecycle.py` 负责调用和在 reduced-motion、暂停、隐藏、最小化、关闭时停止，Qt effect/animation 必须清理。该 helper 不创建
常驻 QTimer、不改变焦点/无障碍树、不读取 ViewModel/domain、不覆盖现有 graphics effect；快速连续切换、自然完成和失效 Qt
wrapper 都必须回到静态主题。

`UI-1.36` 将真实数据态终端的装饰收敛到同一 `presentation/terminal_surface.py:TerminalViewport`。该类保留既有
`QPlainTextEdit` 作为文本、选择、滚动和无障碍事实源，先执行原生 `paintEvent()`，再只在 viewport 已有上下 padding 内绘制
低对比度上下标尺、四角 bracket 和共享 frame 驱动的短信号线；因此装饰不会伪造 RX 数据，也不会覆盖真实文字。它不引入
业务状态、计时器、控制器或新的生命周期分支，`bootstrap.py` 仅替换为同契约子类，`lifecycle.py` 复用已有 motion surface
注册并统一停止。窄 viewport 使用静态标尺回退；三主题、980/1180、1.5x HIDPI、空态/数据态、横向滚动和隐藏/暂停/关闭
静态回退构成该切片的固定验收边界。

`UI-1.37` 将“窗口可见但没有连接/接收活动时完全静止”的缺口收敛到现有 `MotionController`，而不是给信号场或状态
renderer 各自增加 timer。`MotionController.ambient_active` 是 presentation-only 的生命周期旗标：show 时开启，
`suspended` 负责隐藏/最小化停表，`set_motion_enabled(False)` 和 `set_paused(True)` 仍可立即停表，close 时先关闭 ambient
再释放 controller。共享 96ms 时钟只发出既有 `(phase, animated)` frame，业务状态、RX 计数和 transport 事实不进入该旗标；
静态主题绘制仍是 reduced-motion 和窗口不可用时的回退。验收必须同时记录空闲帧基线/结果、隐藏/恢复/低动效/暂停/关闭状态、
三主题无白色像素审计和完整窗口截图；若后续 profile 证明该低频共享时钟不符合预算，应优先降低共享频率或缩减 renderer，
不得复制成多个控件时钟。

`UI-1.38` 为三页工作区补充 `WorkspaceRouteSurface`，但不把装饰 beacon 塞进 TabBar 的 corner slot。实际验证发现，
当前 Qt style 在 980px 下可将固定 132×28 corner widget 布置到 `x=953`（父宽 952）并裁切；因此 `workspace.py` 返回
`workspaceShell`，由其纵向布局承载真实 `QTabWidget` 和右对齐的 `workspaceRouteStrip`。真实 Tab widget、Tab 文案、currentChanged
信号、键盘顺序和 180ms page transition 保持不变；route 只消费 bounded index 与共享 frame，NoFocus/鼠标透明/空 accessibility，
不读 ViewModel/domain、不创建 timer。验收固定覆盖 980/1180 geometry、三主题 near-white=0、Tab index 0/1/2、横向滚动=0、
隐藏/恢复/关闭静态回退和 route 不在 focus chain。

连接快速配置采用 presentation-owned 的 versioned DTO：只携带安全的显示/连接选项，应用动作只填充
现有表单，绝不自动连接或保存密钥。当前 catalog 是 schema v1 immutable tuple，key 唯一且数量限制为
`1..16`，typed values 在构造时完成范围校验；七项内置入口明确包含既有 BLE GATT 扫描兼容项。builder
通过 `ConnectionPresetApplyCallback` 把 DTO action 交给 MainWindow 的 typed wrapper，再由
`presentation/controllers/connection_presets.py` 应用；`connection_runtime.py` 继续只消费控件规范化值并
构造 domain config，刻意不反向依赖 presentation DTO。TCP Server 传输/监听主机/监听端口变化统一失效旧
LAN 确认与 allowlist。持久化用户 catalog 必须另行定义 port/adapter，不能把 QSettings 直接引入 connection
builder，也不能把完整连接字典塞入 MainWindow。

`ARCH-7b` 定义了 `ConnectionPresetCatalogStore` port、schema v1 的显式 JSON codec 和唯一 Qt
`QSettingsConnectionPresetCatalogStore` adapter。组合根读取并把 frozen `ConnectionPresetCatalog` 注入 MainWindow，
`connection_builder.py` 只接收 catalog；坏 payload、未知字段/enum、超长内容和非严格整数均回退七项 builtin，保存时
begin/set/end 任一失败都不执行 sync。该切片只建立安全 persistence boundary，不包含用户自定义 preset editor、builtin/custom
merge 语义或 save action；这些必须另行评审，避免把未实现的配置管理能力伪装成已完成。

`ARCH-7c` 在不扩张连接运行时边界的前提下补齐用户自定义 preset 管理。`connection_preset_editor.py` 是
presentation-only `QDialog`，只收集名称/备注并为新项生成内部 key；当前表单的安全字段快照、DTO 构造、catalog 合并和
显式 save/delete action 由 `controllers/connection_presets.py` 负责。`connection_preset_surface.py` 只负责接收
`QComboBox` 并刷新 builtin/custom 展示，`connection_builder.py` 只负责 bounded combo 组装和按钮组合，二者不形成
controller-to-controller 依赖；`composition.py` 创建唯一 store 实例，bootstrap/MainWindow 只注入它；没有把 QSettings 或 store 读写
下沉到 builder，也没有新增 MainWindow facade。builtin key 具有保留语义，store 只持久化 custom 项，读取时归一化为
immutable builtin + custom；schema v1 的旧完整 catalog 仍可读取并按同一规则归一化。活动会话和历史回放锁定保存/删除，
保存返回 `bool`，失败时本会话可以继续使用但必须显示“本地保存失败”，删除失败保持原 catalog/UI。该切片只保存规范化
连接选项，不保存密钥、设备句柄、BLE 身份、端口选择或自动连接指令。

偏好持久化同样走显式端口：`presentation/preferences.py` 的 frozen
`PresentationPreferences` 只保存版本、主题 key 和动效开关；`PreferenceStore` 是 bootstrap/lifecycle
使用的唯一边界，`QSettingsPreferenceStore` 是唯一 Qt adapter。坏 schema、未知主题和非规范 bool
必须 fail-open 到安全默认，controller 不得直接读写 QSettings。

### 4. 主题是语义 token，不是散落的颜色字面量

`ThemeSpec` 是主题的唯一语义来源，QSS 和 QPainter 自绘控件都从当前 widget 的主题上下文取色。
`apply_theme()` 写入主题属性并刷新子控件；对话框从 parent 继承主题 key。新增颜色必须先进入
`theme_tokens.py`/`ThemeSpec`，不得在自绘控件里硬编码白色或某一主题的背景色。主题验收至少覆盖：

`theme_stylesheet_runtime.py` 是纯 `ThemeSpec -> QSS` 渲染边界；`theme_variant_shell.py` 和
`theme_variant_controls.py` 分别拥有 shell 状态与原生控件 override。主题 token 至少区分
success/info/warning/error/history/neutral/disabled、hover/pressed/focus、selection 和
on-accent，不允许用启发式全局十六进制替换把同一颜色的多个语义混在一起。默认主题继续直接
复用稳定基线，其他主题追加完整的语义 override，避免低层控件、popup、Tab overflow、scrollbar
或 viewport 回退到系统白色 palette。

- 背景、surface、input、滚动 viewport、corner、popup、tab overflow 无系统白色回落；
- 文本、边框、状态、历史/实时来源、错误和 focus token 在所有主题中有值；
- 自绘状态灯、信号场、曲线和弹窗与当前主题同步；
- reduced-motion、暂停、隐藏、最小化和关闭时动效可停止并保留静态信息。
- 工作区 `QTabBar` 的原生 `drawBase` 由 workspace owner 显式关闭，避免 Qt style 注入未主题化的白色基线；该策略不得扩散到全局 palette 或其他 Tab 容器。
- 连接/协议的布局组装必须控制长选项的 size hint，并让动态状态标签向兄弟控件让出空间；只读结果表隐藏无业务意义的纵向行号 header，白色回退通过局部组装修复而非全局 palette。

### 5. OTA 与调试站扩展保持 contract-only 直到目标证据齐备

`ota/transports/{xmodem,ymodem,tftp}` 是独立协议适配边界；`ota/security` 只允许带认证的
AES-GCM/CCM profile 和 key reference；`debug/{rtt,jlink}` 只预留 attach-only 原始打印契约。
这些目录当前不进入组合根，不读取密钥、不加载 vendor SDK/DLL、不执行 flash/memory/halt/reset，
也不把 contract 文件误标成“已支持”。真实实现必须有目标 bootloader/MCU/工具链、公开一手资料、
安全审查、可回滚策略和授权硬件证据。

### 6. 质量门禁可自动执行

- `uv run ruff check src scripts`：导入、未使用依赖和静态规则；
- `python -m compileall -q src scripts`：语法/字节码门；
- `scripts/check_source_limits.py src`：每个 Python 源文件最多 1000 行；
- `scripts/check_theme_tokens.py`：三套主题 semantic token、关键 pseudo-state selector、无白底回落
  和稳定 QSS legacy literal inventory；
- `scripts/check.ps1`：统一本地静态门；
- `scripts/package.ps1`：唯一 PyInstaller 入口，产物、manifest、归档清单、依赖和许可证库存隔离；
- 主题选择器静态审计、模块 import smoke 和架构文档同步；
- GUI/offscreen、EXE 启动、真实设备、OTA、J-Link 和签名发布分别作为授权后的运行时门，不能用
  静态通过替代。

本轮遵守项目规则，不创建或运行 unit test、mock、fixture、harness 或 test-only 资产；这不是对
运行时质量的认证声明。

### 7. 交接文件不污染仓库根目录

最新交接入口固定为 `docs/handoffs/current.md`，历史交接使用日期和范围命名追加保存。根目录不再
放置 `handoff.md`；README、AGENTS 和 WORKFLOW 必须引用新入口。

## 后果

- 新功能拥有明确的 owner 和可替换端口，UI 不必知道后端设备细节；
- staged controller migration 期间仍存在显式 `window` facade，短期增加了委托代码，但便于保持
  业务行为、生命周期和每文件行数可审计；后续必须逐步收窄为 typed feature contract；
- 主题自绘控件多了一层上下文解析，但可以避免“QSS 已变、曲线/状态灯仍旧色”的视觉分裂；
- OTA/J-Link 仍不是已交付的刷写/探针功能，能力边界清晰但需要目标资料和授权后续实现；
- 质量门禁会在大文件、导入层级和交付 provenance 违规时尽早失败，减少后期整包返工。

## 验证状态

本 ADR 记录的是架构决策，不声称 GUI、硬件、OTA 安全或正式发布已经验收。当前静态门、导入烟测、
源码行数门和打包证据以 `docs/handoffs/current.md` 为准；未授权运行时项目必须明确列为未运行。
