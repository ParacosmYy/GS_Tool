# SerialForge 产品与工程约束

## ARCH-128 / UI-1.201 协议页与命令页宽度契约

- 协议页局部 `_ResponsiveProtocolRow` 只能在 `controllers/protocol.py` 内拥有相应 label/control row；不得扩大为 shared form helper，
  不得接管业务状态、DTO、signals、transport、timer 或 scroll owner。
- `protocol_config_context`、`component_table`、`component_preview`、`dataset_preview` 的实例级横向 contract 为
  `minimumWidth=0` + `QSizePolicy.Ignored`；叶子内部滚动保持 native。`DatasetCurveWidget` 的 260px 最小宽度只有在可见状态实际造成
  外层溢出时才允许改为实例级收缩，不得预防性修改类本身。
- 命令 action row 与 empty header 必须分别由 `command_workspace_builder.py`、`command_batch_empty_state.py` 的本地 owner 管理；
  按钮 identity、signals、focus/Tab、accessibility 和 empty-state API 保持。
- 禁止用改变 `ScrollBarAlwaysOff` 或共享 `ResponsiveScrollArea` 的方式掩盖布局问题；验收必须检查外层 `hmax==0` 且 content 不超过 viewport。
- 必须验证三主题 `star_trail/moonlit_ocean/sakura_night`、`546/547/560/600/640/768/900/1120/1240px`、四页外层矩阵，以及协议 table/
  preview/Dataset preview/curve 可见状态；相关 Python 文件不得超过 1000 行。

## ARCH-127 / UI-1.200 UART 参数列级契约

- `ResponsiveUartForm` 只能接收既有 labeled field wrapper；不得创建 raw control、复制 UART DTO、持有业务状态、signals、timer 或动画时钟。
- 字段 hard width 使用 `max(minimumWidth, minimumSizeHint.width)`，preferred width 在此基础上再比较 `sizeHint.width`；regular 必须保持四个
  语义列，compact 两列，narrow 单列。water-fill 规则必须确定且可复核，不得由窗口魔法像素决定。
- `minimumSizeHint()` 只能暴露当前可进入的 hard contract，`sizeHint()` 才表达 preferred；FontChange、StyleChange、LayoutRequest 和
  resize 必须幂等重排，mode 未变化时不得重建或重复连接控件。
- 必须验证三主题与 `546/547/560/600/640/768/900/1120/1240px` 的字段 minimum、无 overlap、focus/identity、生命周期和页面 hmax=0；
  保持唯一共享 `MotionController.TARGET_HZ=120`，不得因布局新增 timer 或 per-widget clock。相关 Python 文件不得超过 1000 行。

## ARCH-126 / UI-1.199 Header 响应式控制带约束

- `ResponsiveHeaderControls` 是 Header 三个现有 control cluster 的唯一几何 owner；API 只能接收 `status_cluster`、`motion_controls`、`theme_controls`
  和 parent，不得接收 window、bindings、ViewModel、业务状态、callbacks、偏好、MotionController 或 timer。
- 三个既有 QFrame 由组合根创建并完成业务接线后，必须一次性注入同一个 owner，并在注入完成后保持同一 QObject parent；模式切换允许同一 owner
  `QGridLayout` 内批量 `removeWidget/addWidget`，但禁止业务侧 `setParent()`、跨 layout 转移、重建控件、重复注册或断开/重连 signals。
  HeaderChromeBindings、window refs、accessible name/description、focus/Tab 顺序必须保持。
- 模式必须按实际 sizing contract 推导：`W >= Rmin` 为 REGULAR，`Cmin <= W < Rmin` 为 COMPACT，其余为 NARROW；不能保留旧的三卡横排布局与新 owner
  竞争，也不能把 `sizeHint()` 变成窗口级硬 floor。REGULAR 第一列保留 status preferred width，不能等比 stretch 导致宽屏重新省略文本。
- COMPACT/NARROW 中 status cluster 必须独占整行；546px 窗口内容约 514px 时，`connectionContext`、`sourceBadge`、`stateValue` 的 visible text 不得包含
  `…` 或空值。可读性必须通过真实字体度量和运行时 geometry 断言，而非只看 `hmax=0`。
- owner 的首次 show/polish、FontChange、StyleChange、LayoutRequest 和 resize 必须经过统一幂等 reflow；mode 未变化时不得 remove/re-add。`minimumSizeHint()`
  的窄态宽度只取 hard-min contract，高度按当前模式各行高度总和与 vertical spacing/margins 推导。
- 必须验证三主题与 `546/547/560/640/768/900/1120/1180/1240px`，以及 `900→546→900` 往返、焦点保持、页面 horizontal maximum=0、hide/show/close、
  status 完整文本、三件 QFrame parent identity、唯一 MotionController/120Hz scheduler。offscreen 证据不等同真实 Windows/HIDPI/显示器 120fps/EXE/hardware 验收。
- 本轮相关 Python 文件必须不超过 1000 行；本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A；不得声称认证合规。

## ARCH-125 / UI-1.198 窄窗口响应式 contract

- `bootstrap.py` 不得恢复窗口级 `setMinimumSize(980, 720)` 或其他固定宽度 floor；初始 `1240×820` 只是启动偏好，最低宽度必须来自真实内容
  sizing contract。不得以 `minimumSizeHint()` 把宽屏偏好重新伪装成不可压缩的窗口 floor。
- `_ResponsiveConnectionBand` 是传输/预设 shell 的唯一布局 owner，必须只重排既有控件；三态为 `REGULAR`、`COMPACT`、`NARROW_COMPACT`，阈值由
  控件 `minimumSizeHint()/sizeHint()`、layout spacing 和 margins 推导。连接按钮、预设选择、持久化动作、context 和 status rail 在窄态必须有
  明确的分行位置，不得通过重建控件绕过 identity、focus、Tab、accessibility 或 bindings contract。
- `ResponsiveUartForm` 是十个既有 UART labeled field wrapper 的唯一布局 owner。它不得创建 raw control、复制 UART DTO、持有连接状态、signals、
  timer 或业务动作；`REGULAR` 为宽屏多列、`COMPACT` 为成对字段、`NARROW` 为单列。`minimumSizeHint()` 只能暴露窄态字段 contract。
- FontChange、StyleChange、LayoutRequest 和 resize 必须触发幂等 sizing invalidation；模式未变化时不得 remove/re-add widget。布局必须使用
  `SetNoConstraint`，并在 owner 内清除/设置当前 mode 的 stretch，禁止保留会被立即覆盖的构造阶段 stretch。
- 必须验证三主题 `star_trail`、`moonlit_ocean`、`sakura_night` 的 `520/546/560/640/768/900/980/1180` 宽度，连接页与 UART 参数区不得出现
  水平 overflow、sibling overlap 或白色 fallback；还要覆盖 FontChange、StyleChange、LayoutRequest、hide/show/close 和对象生命周期。现有唯一
  `MotionController` 的 `TARGET_HZ=120` scheduler 不得因本轮布局改动而增加 timer 或 per-widget clock。
- 本轮相关 Python 文件必须不超过 1000 行；本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A；不得据此声称 MISRA、
  ISO 26262 或任何认证合规。

## ARCH-124 / UI-1.197 命令上下文带约束

- `CommandContextBand` 是选择上下文的唯一布局 owner；builder 只能创建并注入既有 `QLabel`、`QComboBox` 和 `ActionRailButton`，不得在 owner
  内复制业务状态、ViewModel、DTO、signals、执行动作、transport、timer 或动画。
- 宽屏使用两列，窄屏使用单列；切换阈值必须由现有 cell 的 `minimumSizeHint()/sizeHint()` 和 layout spacing/margins 推导，不得写窗口级魔法像素。
  使用 `QLayout.SizeConstraint.SetNoConstraint`，wrapper 最小宽度为 0；历史 ComboBox 保留现有 220px 最小宽度。
- `minimumSizeHint()` 必须保持单列可进入的 compact 最小宽度，并按当前列数返回单行/双行高度；`sizeHint()` 只表达当前模式偏好，不能把宽屏偏好
  变成不可压缩的最低宽度。初始化、resize、FontChange、StyleChange 和 LayoutRequest 必须幂等，cell 不得重叠或重复挂载。
- 上下文带不得增加 `role=surface` 或新 QSS/theme selector，避免嵌套卡片和白色 fallback；五个批量操作按钮必须留在独立 action row。本轮不实现操作行
  折叠、不改变焦点/Tab 顺序、不改变 scroll owner。
- 必须验证 320/330/520/640/720/980/1180 宽度的 1/2 列切换、当前高度 contract、三主题、style relayout、workspace hide/show/close、对象 identity、
  accessibility 与唯一 MotionController 的 120Hz scheduler；offscreen 证据不得外推为真实显示器 120fps、Windows HIDPI、EXE startup 或硬件验收。
- 相关 Python 文件必须不超过 1000 行。本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A；不得据此声称 MISRA、ISO 26262
  或认证合规。

## ARCH-123 / UI-1.196 命令批处理空态与页面节奏约束

- `CommandBatchEmptyState` 必须保持 `Expanding/Preferred`：空态按自身内容自然收缩，不得重新占用命令页的 flexible stretch slot；
  `command_workspace_builder.py` 只能保留现有批处理 signals、bindings、ViewModel 和执行链路。
- 三步引导必须是 presentation-only 的静态 `CommandBatchStepCard`，不增加焦点控件、业务状态、DTO、执行动作、timer、线程或独立动效时钟；
  `CommandBatchStepRail` 只能重排已有三张卡片。
- `CommandBatchStepRail` 的初始化和 resize 必须经过同一个幂等 `_relayout()`；逻辑宽度达到 720px 使用 3 列，否则使用 1 列；首次显示和反复
  resize 均不得出现同格占位、卡片重叠或残留布局项。不得依赖一次偶然的 resize 才完成首帧布局。
- `ShortPageVerticalRhythm.CENTER` 是 `ResponsiveScrollArea` 的默认兼容行为：短页居中、overflow 顶对齐；命令工作台通过
  `scroll_page(..., vertical_rhythm=ShortPageVerticalRhythm.TOP)` 显式 opt-in 顶部节奏。禁止按 `object_name == "commandPage"` 特判，
  禁止改变既有 QScrollArea/viewport/滚动条 owner，禁止在各 builder 复制对齐公式。
- 主题新增 selector 只能引用既有 semantic token（`history_surface`、`surface`、`accent`、`text`、`text_muted` 等），三主题必须无白色
  fallback；不得在组件中写硬编码白色或主题色。
- 本轮必须验证 520/640/720/980/1180 宽度首次显示、反复 resize、无障碍描述、三主题、hide/show/close，以及唯一 MotionController 的
  120Hz scheduler；offscreen 证据不得外推为真实显示器 120fps、Windows HIDPI、EXE startup 或硬件验收。相关 Python 文件不超过 1000 行。
- 当前 ARCH-123 约束 supersede ARCH-115 中“命令页空态继续拥有自身 `Expanding` slot”的历史描述；ARCH-115 其余共享短页默认行为仍有效。
- 本轮无 embedded C/C++、固件或硬件改动，public-vendor-source applicability=N/A；不得据此声称 MISRA、ISO 26262 或认证合规。

## ARCH-122 / UI-1.195 高刷新与概览指标约束

- `MotionController` 必须保持窗口内唯一共享动效时钟与唯一 controller-owned `QTimer`；本轮 scheduler slot 为 4ms，`TARGET_HZ=120` 仍是
  调度目标，不得表述为显示器实际 120fps。
- `MotionController` 内部的 `_tick` elapsed、`_last_tick_at`、activity deadline、`activity_active()` 和 `_can_animate()` 必须使用同一高精度
  `perf_counter` 时间域；禁止把 `perf_counter` 时间戳与 `monotonic` deadline 比较。不得波及业务层时间源。
- frame budget 必须保留 phase elapsed 推进、50ms phase delta cap 和每回调最多一帧；出帧后只能保留小数余量，禁止循环补发过期预算、增加
  repaint burst、第二个 timer、线程或 per-widget 时钟。暂停、低动效、隐藏、最小化、关闭和 show/rearm 语义必须保持。
- `_ResponsiveMetricGrid` 只能重排已有扩展概览 metric tuple，列策略为 6/3/2/1，阈值由可用宽度、`_METRIC_MIN_WIDTH` 和 spacing 推导；不得改 DTO、
  metric 顺序、业务状态、动画、滚动 owner、theme objectName 或 OTA/AES/RTT/J-Link contract。value 必须可收缩/换行，完整原文必须在
  accessibility description 中保留；相关文件不超过 1000 行。
- 验收必须包含：10 秒 offscreen scheduler 约 118–122Hz；50ms 卡顿只 emit 一帧且 budget `[0,1)`；activity deadline；reduced-motion/pause/
  hide/minimize/close/rearm；640/800/980/1180 窄宽度和最长合法动作文本无横向撑宽；三主题无白色 fallback。offscreen 证据不得外推为真实 GUI/HIDPI/
  显示器/EXE 性能或硬件验收。embedded C/C++ applicability=N/A。

## ARCH-121 / UI-1.194 总览态 workspace 高度预算约束

- 总览态 workspace shell 的纵向 floor 只能由 `workspace.py::_ResponsiveWorkspaceShell` 与
  `_overview_workspace_minimum_height()` 管理；预算必须来自 root layout 当前 sibling 的
  `minimumHeight()/minimumSizeHint()`、contents margins 和 spacing，不得把 190/220 等固定像素值当作所有窗口的实际高度。
- `minimumHeight` 是安全 floor，不是实际 height 上限：宽屏剩余空间允许 workspace 自然扩张；只有 floor 封顶 220px。980×720
  的安全预算约为 168px，连接配置 viewport 可以继续由既有 `QScrollArea` 纵向滚动，不能为了扩大 viewport 挤压 observation、
  terminal 或 send。
- focus transition 的 snapshot 字段和恢复语义不得改变；snapshot restore 完成后允许调用 shell 的无状态同步入口重算 overview
  floor。focus 模式中不得触发 overview 同步。
- resize/首显 settle 只允许最多两轮合并的一次性 queued callback，必须有 pending/round guard、对象有效性检查和幂等同步；
  禁止常驻 timer、无界递归、geometry animation、第二个 MotionController 或业务时钟。
- 三主题×正常/低动效×980/1180/1240、快速宽屏→980 resize、focus↔overview、hide/show/close 必须验证：root sibling
  无 overlap、无越界、下方三 surface 可见性正确、queued 状态最终清零；实际 shell height 与 floor 必须分开断言。
- 不得改变 `WorkspaceShellBindings`、Tab 顺序、signals/callbacks、accessibility、scroll owner、业务/transport、UART 参数、
  OTA/AES/RTT/J-Link 边界；相关 Python 文件不超过 1000 行。embedded C/C++ applicability 为 N/A。

## ARCH-120 / UI-1.193 工具栏 sizing 约束

- 实时观测栏的几何策略只能由 `_ResponsiveObservationBand` 管理；发送栏的 sizing policy 只能由
  `_configure_send_layout_sizing()`/`_set_horizontal_policy()` 管理，不得在 lifecycle、ViewModel、业务
  controller、QSS 或 domain 中复制布局规则。
- 动作按钮、显示/发送模式、CRLF、快捷命令和保存快捷必须使用 Qt `Fixed` policy 保持 intrinsic width；
  不得调用 `setFixedWidth(sizeHint())`，避免 DPI、字体、翻译变化造成截断。输入、摘要和状态 surface 的
  stretch/bounded contract 必须保持；不得重建 QWidget 或改变 `TerminalControlBindings`、signals/callbacks、
  focus/accessibility、发送/接收语义。
- 不得新增 timer、业务状态、nested scroll、动画、MotionController、transport 或 OTA/AES/RTT/J-Link coupling；
  三主题×980/1000/1080/1180/1240/1440 总览态 resize、低动效、hide/show/close 必须无越界、无重叠、无按钮压缩；
  相关 Python 文件不超过 1000 行。embedded C/C++ applicability 为 N/A。

## ARCH-119 / UI-1.192 连接控制带响应式约束

- 连接控制带的几何重排只能由 `connection_builder.py::_ResponsiveConnectionBand` 负责；不得在 connection
  runtime、ViewModel、domain、QSS 或业务 callback 中复制断点或布局公式。
- 断点必须由控件自身 `minimumSizeHint()/sizeHint()/minimumWidth()`、已有 layout spacing 和 contents margin
  推导，不得写死窗口像素阈值；compact layout 只重排既有 QWidget，不得重建控件或改变
  `ConnectionShellBindings`、signal/callback、selection、accessibility、focus 和 object identity。
- 不得新增 timer、MotionController、动画、业务状态、nested scroll、transport/OTA/AES/RTT/J-Link coupling；
  UART 参数区本轮保持不变。常规布局与紧凑布局必须都无越界、无重叠，摘要和链路 rail 在 compact 模式填满
  内容列；相关 Python 文件不超过 1000 行。
- 三主题×980/1000/1080/1180/1240/1440、正反向 resize、重复切换、低动效、hide/show/close 必须验证；
  offscreen 几何证据不等同真实 Windows GUI/HIDPI 或显示器 120fps 验收。embedded C/C++ applicability 为 N/A。

## ARCH-118 / UI-1.191 shell transition 互斥约束

- 任意窗口时刻最多一个一次性 shell transition（theme/workspace/focus/transport）处于 active；启动新过渡前
  必须调用 `prepare_shell_transition(window, ShellTransitionKind.X)`，不得创建第二个 timer、MotionController
  或新的 geometry animation。
- `transition_coordinator.py` 只能编排既有 stop owner；不得持有动画细节、graphics effect、布局、业务状态、
  滚动、transport、OTA/AES/RTT/J-Link 数据；延迟导入必须保持无循环依赖。
- 无效目标、隐藏/最小化、低动效/暂停、resize、hide、close 等早退和生命周期边界必须调用幂等全量清理；
  旧 finish callback 不得复活已停止的 effect。卡片、workspace 几何、accessibility、selection/detail 和
  唯一 120Hz MotionController 必须保持；相关 Python 文件不超过 1000 行。
- 快速 theme→tab→focus→transport 及逆向序列、四类早退、隐藏/恢复/最小化/关闭、重复 stop 必须验证；
  118Hz offscreen scheduler 样本只能作为调度证据，不能宣称真实显示器 120fps。embedded C/C++ applicability 为 N/A。

## ARCH-117 / UI-1.190 扩展能力分区约束

- `_build_capability_section()` 只能包装 panel 已创建的 title、hint 与 `ResponsiveCapabilityGrid`；不得
  重建 capability card、复制 selection/detail/accessibility/Tab 状态，或接入 OTA/AES/RTT/J-Link contract。
- 分区必须 `NoFocus`、`Expanding/Preferred`，上下 margin 10px、内部 spacing 7px；水平 margin 保持 0，
  以保留 1180px OTA 三卡的 1100px 最小网格宽度。不得新增 QSS、nested scroll、timer、动画或独立 MotionController。
- 既有原生 QScrollArea、`HMAX=0`、卡片对象 identity、焦点/选中详情、隐藏/恢复/关闭生命周期必须保持；
  三主题×980/1180/1240 与 `1→2→3→2→1→3` resize 必须无重叠、无布局残留；相关 Python 文件不超过
  1000 行。embedded C/C++ applicability 为 N/A。

## ARCH-116 / UI-1.189 扩展能力卡网格约束

- `ResponsiveCapabilityGrid` 只能接收并重排已有 card QWidget；不得重建卡片、复制 capability catalog、
  改写 selection/detail/accessibility/tab order 或接入 OTA/AES/RTT/J-Link 业务 contract。
- 列数必须使用扣除 contents margin 与 horizontal spacing 的可用宽度计算，范围为 `1..min(3, len(cards))`；
  `card_min_width` 必须为正；宽度不足一张卡时保底 1 列。重排只在列数变化时发生，保留 10px spacing 与等权 stretch。
- 容器必须 `NoFocus`，现有 card 的 focus/checked/accessibility 身份保持；扩展页仍通过唯一原生 QScrollArea
  滚动，水平 `HMAX=0`，不得新增 nested scroll、timer 或独立 MotionController。
- 三主题×980/1180/1240、1↔2↔3 列反向 resize、卡片无重叠、焦点/选中详情、隐藏/恢复、最小化、关闭和
  相关文件不超过 1000 行必须通过；Qt 允许等权 stretch 的 1px 像素取整差。embedded C/C++ applicability 为 N/A。

## ARCH-115 / UI-1.188 短页垂直节奏约束

- 所有设置页必须继续通过 `controllers/composition.py:scroll_page()` 进入共享滚动边界；垂直节奏由
  `ResponsiveScrollArea` 单一 presentation owner 管理，不得在 connection/protocol/command/extension
  builder 中复制居中公式。
- 内容高度不超过 viewport 时只切换垂直 `AlignVCenter`；内容超出时必须为 `AlignTop` 并保留现有
  QScrollArea 原生滚动。水平 alignment 必须保持 Fill，禁止用 `AlignHCenter` 造成窄列。
- 不得重置页面 layout 的 stretch、size policy、子控件 geometry 或 command empty state 的 `Expanding`
  owner；不得新增 nested scroll、timer、独立 MotionController、业务状态或信号契约。
- resize、show、scroll rangeChanged 的刷新必须幂等；三主题×980/1180/1240×四 workspace、focus/
  overview、隐藏/恢复、最小化、关闭、HMAX=0 和长页滚动范围必须保持。相关 Python 文件不超过 1000 行；
  embedded C/C++ applicability 为 N/A。

## ARCH-114 / UI-1.187 根布局伸缩与恢复约束

- focus 模式的根布局 stretch 必须由 `workspace_focus_transition.py` 单一 owner 管理：shell 获取可用
  高度，terminal slot 退让为隐藏 slack；不得在 builder、ViewModel、QSS 或业务模块复制 stretch 公式。
- 进入 focus 前必须以 `None` 哨兵保存 root/shell/tabs/route/terminal 的可恢复状态；退出时先恢复布局
  policy、min/max 和 stretch，再恢复 terminal 子控件 hidden、父 surface、鼠标透明，最后 activate layout
  并清空快照。重复切换和失败短路不得留下半恢复状态。
- shell 内 tabs 是唯一可伸缩内容 owner，route strip 保持固定 31px；不得用逐帧 `maximumHeight`、
  `setGeometry()` 或新建 root spacer 解决白色空行/组件拥挤。
- focus/overview 的几何结算必须立即完成，视觉过渡只能复用唯一 `MotionController` 的 opacity track；
  `TARGET_HZ=120` 仍是共享调度目标，不声称真实显示器 120fps。
- 三主题×980/1180/1240×四 workspace、隐藏/恢复、最小化、关闭、低动效/暂停、横向 `HMAX=0` 和
  唯一 controller 必须通过；相关 Python 文件不超过 1000 行。embedded C/C++ applicability 为 N/A。

## ARCH-113 / UI-1.186 专注工作区自然高度约束

- `workspace_focus_transition.py` 是 focus 静态布局、终端 slack、surface 可见性和一次性 opacity
  过渡的唯一 owner；不得把自然高度公式复制到 builder、ViewModel、业务状态、QSS 或 transport/
  OTA/AES/RTT/J-Link 模块。
- focus 模式只能复用已有透明 `terminalSurface` 作为 layout slack；不得新增根级 spacer、改变
  composition root 顺序、调用 `setGeometry()`/`move()` 强行摆放，或恢复逐帧 `maximumHeight` 动画。
- 结算必须以当前 `QScrollArea` 内容 `minimumSizeHint()` 加 tabs/shell chrome 为依据；目标高度放不下
  时保持 native vertical scrolling，不能用固定高度截断表单；短页可在可用空间内自然收敛。
- 进入/退出必须使用 `None` 快照哨兵，保存并恢复 terminal 子控件 `isHidden()`、父 surface hidden
  状态、鼠标透明、vertical policy、minimum/maximum height；事件过滤器必须在释放前移除，重复
  `True/False`、before-show focus、resize 和 route 切换必须幂等且无状态泄漏。
- tab route 在 focus 未改变时只能调用 `refresh_workspace_focus_layout()`；resize 复用同一 owner，
  不得启动第二个 animation group、timer 或动效时钟。三主题×980/1180/1240×四 workspace 的
  focus/overview、横向 `HMAX=0`、terminal 可见性和唯一 MotionController 必须保持；相关 Python 文件
  不超过 1000 行。embedded C/C++ applicability 为 N/A。

## ARCH-112 / UI-1.185 紧凑顶栏密度约束

- 自适应边界必须使用实际 `header.width()`；低于 `1120px` 的紧凑态只隐藏
  `header._state_caption`，不得隐藏状态灯、状态值、状态组 accessible name/description 或主题/动效
  控件；达到阈值后必须恢复标题。
- `controllers/workspace.py:_sync_header_density()` 是该视觉策略的唯一 owner；不得把紧凑判断复制到
  ViewModel、业务状态、QSS、transport、OTA/AES/RTT/J-Link 或新的响应式状态源。
- resize、首屏、状态更新、pause/reduced-motion、隐藏、恢复和关闭路径必须保持现有生命周期；不得为
  这一视觉 token 新增 timer、线程、动画时钟、设备 I/O 或 scroll owner。
- 三主题与 `980×720`、`1040×720`、`1120×720`、`1240×820` 的真实 Qt offscreen 组合验证中，
  紧凑态标题隐藏、宽屏标题恢复、无横向溢出、状态可访问性保持；embedded C/C++ applicability 为 N/A。

## ARCH-111a / UI-1.184 共享动效预算约束

- `MotionController` 是全窗口唯一动效时钟，继续使用 8ms `PreciseTimer` 与 `TARGET_HZ=120`；
  预算必须由已有实测 elapsed 累积 `elapsed × TARGET_HZ`，不得回退为固定整数槽位预算。
- 每次 `_tick()` 最多发出一个 animated frame，不得用循环补发积压帧；phase 的 elapsed cap、
  `frame_changed` contract、可见 surface fan-out 和单一 owner 必须保持。
- pause、reduced-motion、隐藏、最小化、rearm、关闭必须继续停止 timer 并清零时间锚点/预算；
  不得新增组件 timer、线程、repaint loop、业务状态、scroll owner、设备 I/O 或 OTA/AES/RTT/
  J-Link coupling。
- offscreen scheduler 样本只能作为调度证据，不得宣称真实显示器 120Hz、HIDPI 或正式发行性能；
  本轮 Python/PySide6 presentation-only，embedded C/C++ applicability 为 N/A。

## ARCH-110 / UI-1.183 命令空态居中约束

- `CommandBatchEmptyState` 必须继续是命令页唯一 vertical stretch owner；本轮仅允许在组件内部
  水平两侧使用 stretch，把 glyph、copy、CTA 作为自然宽度内容组居中。
- 不得把宽度/居中策略放入 command controller、ViewModel、命令执行状态或外层 scroll owner；不得
  引入固定最大宽度、第二个响应式状态源或额外 spacer 与 vertical stretch 竞争。
- CTA signal、StrongFocus、accessible name/description、glyph/action 共享 frame 接口和低动效/
  暂停/隐藏/关闭生命周期必须保持；不得新增 timer、thread、paint loop、业务状态或 OTA/debug coupling。
- 三主题、980×720、1240×820 下内容 union 中心偏差应不超过 `2px`，横向/纵向 scrollbar maximum
  为 `0`，相关 Python 文件低于 1000 行；embedded C/C++ applicability 为 N/A。

## ARCH-109 / UI-1.182 首帧可读淡入约束

- focus/overview 切换必须先结算最终布局，再对下方实时观测、终端和发送 surface 做 opacity-only
  reveal；不得恢复 `maximumHeight`/geometry 动画，不得用透明空白替代内容布局。
- overview reveal 起点固定由 `workspace_focus_transition.py` 的命名常量拥有，目标为首帧可读且仍有
  轻微过渡；不得复制到 QSS、ViewModel、业务状态或新的控件状态机。
- 继续复用唯一 `MotionController`、`MotionDrivenAnimationGroup`、reduced-motion/暂停/隐藏/关闭
  生命周期；不得新增 timer、singleShot、processEvents、paint loop、scroll owner、线程或设备 I/O。
- 三主题、980×720、1240×820、总览/专注、四 workspace 必须保持无 sibling overlap、无横向滚动，
  下方 surface 在总览过渡中可见，专注完成后高度为 0；embedded C/C++ applicability 为 N/A。

## ARCH-108 / UI-1.181 扩展焦点滚动约束

- 能力卡获得 Tab focus 或点击后必须保持在现有 `QScrollArea.viewport()` 可视区内；详情显示引起的
  content 高度变化必须同步结算，不得用 `QTimer.singleShot`、`processEvents`、常驻 timer 或新
  scroll owner 规避时序问题。
- `embedded_extension_panel.py` 只能复用现有 scroll owner：先结算 `QScrollArea.widget()`，再调用
  原生 `ensureWidgetVisible`，只在真实 top/bottom 越过安全边界时调整 vertical scrollbar；不得
  改变横向滚动策略、Tab 顺序、业务状态、OTA/AES/RTT/J-Link contract 或设备 I/O。
- 三主题、980×720、1240×820、7 张能力卡逐一聚焦必须无裁切、无 sibling overlap、`hmax=0`，详情
  accessible description 保持非空；相关 Python 文件低于 1000 行；embedded C/C++ applicability 为 N/A。

## ARCH-107 / UI-1.180 扩展详情按需展开约束

- 扩展工具站首屏不得重复渲染第一张能力卡的详情；`ExtensionCapabilityDetail` 初始隐藏，首次
  Tab 聚焦或点击能力卡后显示对应只读详情，不得创建空白占位或新的滚动 owner。
- `embedded_extension_panel.py` 只拥有页面组合与局部选中投影；能力 catalog、OTA 三协议、AES 安全
  槽位和 RTT/J-Link attach-only/contract-only 状态不得复制到 UI 状态机或连接业务。
- 不得新增后端、密钥读取、vendor 工具、timer、thread、设备 I/O 或独立动画时钟；卡片必须保持
  键盘可操作、accessible description 完整、三主题下无文字裁切和 sibling overlap。所有相关 Python
  文件低于 1000 行；embedded C/C++ applicability 为 N/A。

## ARCH-106 / UI-1.179 工作区模式上下文约束

- 模式文案必须复用 `WorkspaceContextLabel` 与既有 `workspaceShell[mode]` owner；不得新增 ViewModel
  字段、第二个 focus 状态源、timer、scroll owner、业务依赖或独立 route strip 控件。
- focus 显示“专注 · 当前页”，overview 显示“总览 · 当前页”；accessible description/tooltip 必须
  说明实时观测、终端和发送区是暂时收起且仍在后台工作，或当前为全部可见总览。
- 不得改变 route strip 固定 31px、Tab/焦点顺序、滚动提示 geometry、共享 MotionController 或
  transport/session/OTA/debug contract；所有相关 Python 文件低于 1000 行。
- 三主题、980×720、1240×820、四 workspace 和 focus/overview 必须无文案裁切、sibling overlap 与
  横向滚动；本轮 embedded C/C++ public-source applicability 为 N/A。

## ARCH-105 / UI-1.178 滚动位置提示约束

- `WorkspaceScrollHint` 必须只读取当前 `QScrollArea` 原生 vertical scrollbar；不得新增滚动状态
  源、section 计数、timer、scroll owner、业务字段或 ViewModel 依赖。
- `top/middle/bottom/complete` 文案必须同时表达位置与动作，accessible name/description 必须
  与真实 scrollbar 状态同步；提示不可交互，不得改变键盘 Tab 顺序或焦点路径。
- 稳定 controls stylesheet 负责 selector/字重，variant stylesheet 继续只覆盖 `ThemeSpec` token；
  不得引入近白背景带、改变 route strip 固定高度或造成横向滚动。所有相关 Python 文件低于 1000 行。
- 三主题、980×720、1240×820、四 workspace 和 focus/overview 必须保持 top/middle/bottom/complete
  状态正确、提示无裁切、horizontal scroll 最大值为 0。embedded C/C++ public-source applicability
  为 N/A。

## ARCH-104 / UI-1.177 命令空态画布约束

- `command_workspace_builder.py` 是命令页布局 owner；没有批量命令时，`CommandBatchEmptyState`
  必须是页面唯一 vertical stretch owner，不得再用 root trailing stretch 与它竞争空间。
- `command_batch_empty_state.py` 只负责 presentation 排版：允许 expanding vertical policy、glyph
  垂直居中和 copy 区前后 stretch；不得读取 ViewModel、创建命令状态、执行批量命令或接管 scroll。
- `controllers/commands.py` 继续拥有 batch/snapshot/visible 投影；有批量命令时空态必须隐藏，
  结果表和命令执行语义不得改变。所有相关 Python 文件低于 1000 行。
- 三主题、980×720、1240×820 和四 workspace 必须保持无 sibling geometry overlap；不得引入白色
  背景带、横向滚动、第二 timer、线程、paint loop 或 OTA/debug coupling。
- 共享动画继续复用唯一 `MotionController`、`TARGET_HZ=120`、8ms `PreciseTimer` target；1ms/
  elapsed-budget 试验已证明无收益并撤回。离屏约 117Hz 只可作为 scheduler evidence，不得宣称真实
  显示器 120fps。embedded C/C++ public-source applicability 为 N/A。

## ARCH-103 / UI-1.176 路线条模式主题约束

- 路线条模式样式必须只读取既有 `workspaceShell[mode="focus|overview"]` 动态属性；不得新增
  `focus` 状态源、业务字段、controller timer 或 transport/session 依赖。
- `theme_stylesheet_controls.py` 负责稳定 selector 结构，`theme_variant_controls.py` 只能替换
  `ThemeSpec` 语义 token。focus 可使用 info/history 渐变与 accent-blue 上沿，overview 保持 neutral
  上沿；不得使用近白字面量、修改 route strip 固定高度、改变 Tab/焦点顺序或覆盖 scroll hint。
- 三主题与 980/1240 尺寸必须保持 route strip、focus button、scroll hint 可见且无横向滚动；该视觉
  层不创建新的 paint loop、timer、线程或 animation owner。所有相关 Python 文件低于 1000 行。
- 本轮离屏共享时钟 0.5 秒采样均值 `8.41ms`、有效采样约 `118.8Hz`，只代表 scheduler target 证据，
  不得宣称真实显示器 120fps；embedded C/C++ public-source applicability 为 N/A。

## ARCH-102 / UI-1.175 顶栏密度与过渡布局约束

- `presentation/controllers/workspace.py` 是 app header density 的唯一 owner；紧凑态可以收敛 header
  自身 margins、cluster padding、cluster spacing、brand spacing 和语义 QSS 圆角，但不得隐藏主题
  选择器、动效开关、连接状态或焦点可操作控件。纯装饰 `SignalFieldWidget` 与色板 swatch 才能按
  既有 compact policy 隐藏。
- `workspace_focus_transition.py` 在 focus/overview 切换时必须先一次性应用最终布局并激活根布局，
  再创建 opacity-only transition。不得将 `QTabWidget.maximumHeight`、页面 geometry 或 sibling
  allocation 作为动画 track；这是防止中间帧裁切和组件挤压的硬约束。
- 进入、退出、快速反转、reduced-motion、pause、resize、hide、minimize、close 都必须回到静态布局，
  清理 graphics effect 和 shared-clock group。视觉过渡仍只能复用窗口级 `MotionController`，不得
  新增第二 timer、线程、paint loop、scroll owner、业务状态或设备 I/O。
- 本轮实测：980×720 中间帧 25/55/95/140ms sibling 无重叠，focus viewport `514px`；三主题×980/1240
  `horizontal_max=[0,0,0,0]`；1 秒离屏时钟 `frames=119, mean=8.27ms, effective≈120.9Hz`，最大
  `17.08ms` 仅记录为 offscreen scheduler jitter，不得宣称真实显示器 120fps。所有相关 Python 文件
  低于 1000 行；embedded C/C++ public-source applicability 为 N/A。

## ARCH-101 / UI-1.174 全部一次性 UI 过渡共享时钟约束

- page、theme root+sweep、transport panel、dialog、focus 的一次性过渡必须各自保持 finite owner，
  但全部复用同一个窗口级 `MotionController.frame_changed`；不得创建全局 TransitionManager、第二个
  `QTimer`、页面级常驻 clock 或为 dialog 单独创建 timer。
- `QPropertyAnimation` 只能在 `MotionDrivenAnimationGroup` 内提供 property/easing/interpolation
  contract；业务 owner 不得调用内部 track 的 `start()`。一个 group 的多个 track（如 theme root opacity
  与 sweep geometry）只能由 group 启动，防止双时钟竞争写属性。
- 每个 owner 必须拥有自己的 stop/finish/clear 路径：graphics effect、overlay、group 引用和
  `frame_changed` connection 在自然完成、快速触发、pause、reduced-motion、resize、hide、minimize、
  close 时都要释放。driver 对中途失效 target 必须 stop 并触发 owner cleanup，不得继续写 Qt 对象。
- transport panel 只有在目标 panel 可见时才启动；dialog 通过现有 parent chain 使用主窗口 shared clock，
  找不到 clock 时静态回退。不得把 SessionState、transport I/O、OTA/debug 状态带入 shared driver。
- 本轮连续活动窗口离屏采样为 page `8.13–8.95ms`、theme `9.40–9.89ms`、transport `8.21–9.05ms`、
  dialog `8.21–8.72ms`、focus `8.67ms`；这只是当前 scheduler 证据，不等同真实显示器 FPS、HIDPI
  或发行性能承诺。所有相关 Python 文件继续低于 1000 行；Python/PySide6 presentation-only，
  embedded C/C++ public-source applicability 为 N/A，不作 MISRA、ISO 26262、ASIL 或认证声明。

## ARCH-100 / UI-1.173 专注过渡的 120Hz 共享时钟约束

- 专注设置↔总览的高度与 reveal opacity 过渡必须由
  `presentation/workspace_focus_transition.py` 维护布局/生命周期，由
  `presentation/motion_transition.py:MotionDrivenAnimationGroup` 维护有限属性 track；不得把
  `SessionState`、transport、OTA/debug 状态或设备 I/O 放入动效 owner。
- `MotionDrivenAnimationGroup` 必须复用唯一 `MotionController.frame_changed`（8ms/120Hz target）；
  不得为单个属性、页面、主题、弹窗或 transport panel 新增 `QTimer`、线程或常驻 repaint loop。
  `QPropertyAnimation` 可作为 easing/interpolation contract，但不得启动 Qt 自有动画 driver 造成
  cadence 分裂；driver 应以 monotonic elapsed time 应对帧合并。
- 进入、退出、快速反转、显式 stop、reduced-motion、pause、resize、hide、minimize 和 close 必须
  恢复静态布局并清理临时 graphics effect；共享 driver 不能在窗口不可见或动效关闭后继续写属性。
- 本轮证据只覆盖 focus transition；page/theme/dialog 的其他一次性 fade 未自动视为 120Hz，后续迁移
  必须独立建切片、架构复核、测量和交接。offscreen 的 9.04ms 活动样本只表示当前 scheduler 证据，
  不等同真实显示器 FPS 或发行性能承诺。
- 本轮所有相关 Python 文件必须保持低于 1000 行；不得以合并大文件规避门禁。Python/PySide6
  presentation-only，embedded C/C++ public-source applicability 为 N/A，不作固件标准、MISRA、
  ISO 26262、ASIL 或认证声明。架构师/独立 reviewer 调用超时关闭，未伪造外部 PASS；父代理完成
  五轴 review 与行为保持简化评估。

## ARCH-97 / UI-1.170 扩展能力卡内容安全高度约束

- `embedded_extension_panel.py:_build_capability_card()` 是能力卡内容与无障碍文案的 owner；卡片
  已声明 `122px` 内容安全下限。`theme_stylesheet_extension.py` 必须在专属
  `QPushButton#extensionCapabilityCard` selector 中重复表达该视觉 contract，不能让后置通用
  `QPushButton` 规则把卡片压缩到约 `83px`。
- 该最小高度只保护标题、状态、summary 和 boundary 的展示空间，不改变卡片的点击/焦点/Enter/空格
  选择契约、只读详情投影、OTA/debug contract-only/attach-only 状态或任何 application DTO。
- 三主题必须复用同一专属 selector 语义；980×720 与 1240×820 继续由既有外层 `QScrollArea` 承载
  纵向增长，不能新增 nested scroll、splitter、timer、线程、业务状态或第二个 motion owner。
- 本轮是 Python/PySide6 presentation stylesheet 修复，embedded C/C++ public-vendor-source
  applicability 为 N/A；架构师 `019ff342-7abf-7e22-bc8e-40816e7aeef3` 和独立 reviewer
  `019ff346-ded8-70b2-9843-fb9845b50952` 均超时关闭，未伪造外部 PASS。父代理完成五轴 review、
  简化评估及三主题/双尺寸离屏验证。

## ARCH-96 / UI-1.169 有限整数选项与紧凑布局约束

- `presentation/bounded_value_combo.py` 的 `_BoundedNumericCombo` 继续是 presentation-only
  numeric selector owner；`BoundedFloatCombo` 管理秒数，`BoundedIntCombo` 管理整数显示、范围和
  旧值兼容。协议最大帧、UDP 最大报文、TCP Server 客户端数、批量命令延时各自拥有独立 option
  catalog，不得把 domain/application 规则搬进 selector。
- 有限业务值必须使用不可编辑 `QComboBox` 直接选择：帧长使用常见字节档位，UDP 使用常见报文档位，
  client 数使用 1/2/4/8/16 等档位，命令延时使用 0/10/20/50/100/200/500/1000/1500/2000 ms。
  端口必须继续保留 `QSpinBox`/可输入端点能力，因为端口不是有限业务选项。
- `value()`、`setValue()`、`setRange()` 保持原数字控件的 presentation contract；程序化旧值若不在
  catalog 中只插入一个不可编辑临时选项，不能静默改值。协议固定上限切换时必须抑制 clear/rebuild
  的中间 signal，用户真实选择仍可触发 dirty projection；不得产生重复业务通知或第二状态源。
- numeric selector 默认设置紧凑的 96–148px 宽度上限，防止 `QComboBox` 在 980px 页面中把网络、
  协议、命令或扩展页撑出横向滚动；布局、主题、tab order、disabled gate、accessibility 和
  120Hz MotionController 继续由既有 owner 负责。
- 本轮不新增 timer、线程、事件总线、scroll owner、端口自动连接、设备 I/O 或 OTA/AES/RTT/J-Link
  coupling；所有相关 Python 文件低于 1000 行。option catalog 后续扩展必须以真实有限值域为前提，
  不得为了统一而强行改造 host、port、allowlist 等自由输入。
- 离屏证据：`ARCH96_RANGE_SIGNAL_PASS`；协议固定 MAVLink 上限 `280` 且 disabled；命令 `1234 ms`
  与网络 `65507 B/16 clients` 程序化值读取通过；切换后四个工作区 visible scroll 的
  `horizontal_max=[0, 0, 0, 0]`，UDP 页数值控件宽度 `148px`。
- 本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability 为
  N/A；不作固件、MISRA、ISO 26262、ASIL 或认证声明。架构师
  `019ff32a-7b39-7c50-a28b-ab39345c386c` 与独立 reviewer `019ff332-291b-7273-a6fd-11faa93e7aa5`
  均超时关闭，未伪造外部 PASS；父代理完成五轴 review、简化评估，并修正了 range rebuild signal
  与 compact width 风险。

## ARCH-95 / UI-1.168 超时选项化约束

- `presentation/bounded_value_combo.py` 是 presentation-only 的有限浮点选项控件 owner；
  `BoundedFloatCombo` 只持有显示选项和秒数映射，不读取 ViewModel、domain DTO、transport 或
  持久化状态。`controllers/composition.py:timeout_combo()` 是 UART、网络、BLE 八个超时控件的
  唯一构造入口，builder 只负责放置和补充语义文案。
- 超时控件必须是不可编辑 `QComboBox`，使用统一选项表直接选择 `1 ms`、`200 ms`、`1 s`、
  `30 s` 等人类可读值；运行时值始终是秒。`0` 只在允许的 UART 字节间超时中显示为“未设置”，
  继续映射为现有 `None` 语义。UART、TCP/UDP/RTT、BLE 的默认值和上下界保持不变。
- `value()`/`setValue()` 是兼容 presentation binding 的窄接口；`setValue()` 对超界值按既有
  `QDoubleSpinBox` 语义裁剪，对非选项的程序化旧值只插入一个不可编辑的临时选项，不开放文本
  输入，也不把自定义值写入 domain 或 settings。transport runtime 继续只读取 `.value()`，
  不得复制第二份超时状态。
- 选项 label、AccessibleTextRole、AccessibleName、tooltip 和字段标签必须说明单位与“未设置”
  语义；布局只能依赖既有 themed `QComboBox` 样式和表单 row，不得引入第二个 scroll owner、
  常驻 timer、逐控件动画、事件总线、worker、业务状态或 OTA/AES/RTT/J-Link coupling。
- 本轮所有相关 Python 文件保持低于 1000 行；新增的 option catalog 必须可被后续有限端口、
  帧长等控件复用，但本轮不扩大范围、不把所有数字控件强行泛化。
- 离屏证据：八个控件均 `editable=False`，option count 为
  `[10, 11, 12, 16, 16, 16, 16, 16]`，默认值保持
  `[2.0, 1.0, 0.0, 3.0, 0.2, 3.0, 5.0, 30.0]`；超界裁剪和程序化 `0.123` 秒读取通过，
  980×720 组合根无新增横向挤压。Ruff、compileall、source-limit、theme audit 通过。
- 本轮只修改 Python/PySide6 presentation，embedded C/C++ public-vendor-source applicability 为
  N/A；不作固件、MISRA、ISO 26262、ASIL 或认证声明。架构师
  `019ff31a-b844-7ca1-b77d-3d75984a90bc` 与独立 reviewer `019ff320-947e-7850-bba9-ea631f12335a`
  在等待窗口内超时并关闭，未伪造外部 PASS；父代理完成五轴 review、行为保持简化评估和非破坏性
  Qt 验证。

## ARCH-94 / UI-1.167 工作区 reveal 防拥挤约束

- `presentation/workspace_focus_transition.py` 继续是专注设置/总览几何过渡的唯一 owner；退出
  专注设置时，`liveObservationBand`、`terminalSurface`、`sendControlBand` 的高度动画仍必须
  复用一次性 `QParallelAnimationGroup` 和现有 220ms easing，只额外使用 bounded 的临时 opacity
  effect 让内容在空间分配期间渐显，避免中间帧把内部控件硬挤在一起。
- opacity effect 只允许挂在本轮 reveal 的三块下方 presentation surface 上；完成、快速反转、
  reduced-motion、暂停、隐藏、最小化、resize 和 close 路径必须恢复 opacity、解绑 effect，并且
  不得触碰其他 owner 的 graphics effect。不得留下悬挂 effect、常驻 timer、第二个
  `MotionController`、事件总线、业务状态、滚动 owner 或 OTA/AES/RTT/J-Link 依赖。
- `_workspace_focus_mode` 仍只是当前 window 的 presentation flag。focus 只隐藏/显示实时观测、
  terminal、send 三块；session worker、接收/记录/解析/连接 gate、Tab index、focus/accessibility
  和主题 token 保持不变。进入 focus 仍先隐藏下方 surface，退出总览仍以实测高度作为动画目标。
- 本轮不把 opacity transition 当作 120Hz 业务或显示器 FPS；唯一共享 `MotionController` 继续
  保持 `TARGET_HZ=120`、8ms `PreciseTimer` scheduler target、elapsed-time phase 和既有静态回退。
  980×720/1240×820 中间帧与 settled 状态均不得出现 overlap、横向滚动或系统白色回退。
- 本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability 为
  N/A；不作固件、MISRA、ISO 26262、ASIL 或认证声明。架构师/独立 reviewer 调用超时关闭，未伪造
  外部 PASS；父代理完成五轴 review、行为保持简化评估和非破坏性离屏验证。

## ARCH-93 / UI-1.166 推荐连接配置与 stale context 约束

- `controllers/connection_presets.py:hydrate_recommended_connection_preset()` 是首屏推荐配置
  hydration 的唯一 owner；`bootstrap.py` 只在组合根完成既有 transport projection 后调用一次。推荐
  profile 使用 `connection_presets.py` 的稳定 key `DEFAULT_CONNECTION_PRESET_KEY`，不得在 bootstrap
  里复制 UART 数值或直接写控件字段。
- 推荐 profile 只通过既有 `QComboBox.currentIndexChanged` → `_dispatch_connection_preset()` →
  `apply_connection_preset()` 路径填充安全表单值；绝不自动连接、探测设备、保存密钥或改变
  ViewModel session 状态。UART 物理端口仍需用户选择/输入，连接按钮仍需用户显式点击。
- `connection_runtime.on_transport_changed()` 必须清理与当前 `TransportKind` 不匹配的 preset
  selection/context，避免用户手动切换到 TCP、UDP、BLE 或 RTT 后仍看到 UART 旧摘要。清理只影响
  presentation combo/context，不清空用户输入、不启动连接、不复制第二个业务状态源。
- 用户显式选择其他内置/自定义 profile 或把 combo 置回 placeholder 的行为保持；save/delete
  自定义 profile 继续由既有 controller/store 负责，推荐 hydration 不持久化“已选择”状态。
- 本轮所有相关 Python 文件须保持低于 1000 行；推荐逻辑不得引入新 timer、线程、全局状态、
  builder 反向依赖或自动连接副作用。accessibility description、tooltip、低动效和现有连接 gate
  必须与当前 selection 同步。
- 离屏证据：启动默认 `uart-115200-8n1`；7 个 builtin profile 全部切换/应用通过；手动改 TCP
  清理旧 selection；4 个 workspace 的 visible scroll `horizontal_max=0`；980px 连接带 `138px`、
  UART panel `217px`。本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source
  applicability 为 N/A，不作固件、MISRA、ISO 26262、ASIL 或认证声明。
- ARCH-93 架构师与独立 reviewer 调用在服务窗口内超时并关闭，未伪造外部 PASS；父代理完成
  fresh correctness/architecture/security/performance/readability review 与行为保持简化评估。

## ARCH-92 / UI-1.165 响应式连接带与 120Hz 动效约束

- `controllers/composition.py:scroll_page()` 是共享滚动页的布局入口；内容布局必须显式使用
  `Qt.AlignmentFlag.AlignTop`，让短页面从顶部开始，剩余空间属于 workspace viewport，不得被
  连接控件吸收成空白段。
- `controllers/connection_builder.py` 是连接页组合 owner。`connectionControlBand` 与 UART
  panel 采用水平 `Expanding`、垂直 `Fixed`，依赖自然高度和既有内部布局；不得用 stretch 把
  配置控件拉满 viewport，也不得为修复一页布局而新增第二个 scroll owner。
- 主题背景、边框、surface 和文本继续只能来自 `ThemeSpec`/语义 token；连接带、UART panel、
  scroll viewport 和所有状态 surface 不得回退为硬编码白色背景。布局切片不得改变 UART、TCP、
  UDP、BLE、RTT 的业务绑定、默认值、连接 gate、focus 或 accessibility 契约。
- `MotionController` 仍是唯一 presentation clock：`TARGET_HZ=120`、`PreciseTimer`、8ms
  scheduler slot、0.96 nominal frame budget、elapsed-time phase 和 stop/rearm/close 语义必须
  保持。120Hz 是 Qt scheduler 的目标/离屏采样口径，不是显示器真实 FPS 保证；禁止增加逐控件
  常驻 timer、线程或 paint loop。
- `lifecycle_motion.py` 的 `motion_active()` 是可选的表面能力谓词：只有 `animated=True` 且表面
  明确无动态 glyph 时才跳过逐帧 fan-out；`animated=False` 必须仍投影到所有表面，确保低动效、
  暂停、隐藏、最小化、关闭和状态切换可确定地收敛。各 surface 自己拥有动态判定，不得把业务状态
  复制到共享 motion owner。
- 本轮布局/动效改动涉及的 Python 文件均须不超过 1000 行；不得为了减少 fan-out 分支引入
  universal mixin、registry、第二状态源或循环 import。
- 本轮离屏证据：980/1240 宽度连接页可见 scroll 的 `horizontal_max=0`，连接带运行时高度
  `138px`，动效样本平均 `120.83Hz`；隐藏时 timer 停止、再次显示后可重启。字体目录警告属于
  当前 PySide6 环境资源提示，不作为本轮布局/动效修复的成功条件。
- 本轮为 Python/PySide6 presentation-only；embedded C/C++ public-vendor-source applicability
  为 N/A，不作固件、MISRA、ISO 26262、ASIL 或认证合规声明。架构师调用与独立 review 调用在
  服务窗口内超时并关闭，未伪造外部 PASS；父代理完成 fresh review 与简化评估。

## ARCH-91 / UI-1.164 ViewModel worker owner 约束

- `presentation/viewmodel_jobs.py` 是 `_DiscoverySignals`、`_DiscoveryJob`、`_BleScanSignals` 和
  `_BleScanJob` 的唯一实现 owner；它只负责阻塞 discovery 调用、queued result signals 和异常到
  `ErrorInfo` 的映射，不得读取 Session 状态、创建线程池、启动 timer 或写入 UI 控件。
- `SessionViewModel` 继续拥有全局 `QThreadPool`、两个取消 `Event`、discovery/BLE busy 状态、
  closing gate、状态文本和 completed/failed 回调。`refresh_ports()`、`scan_ble()` 的 signal 连接、
  入队顺序、取消检查与 stop 生命周期必须保持，不能在 worker 模块中复制第二个状态源。
- ViewModel 的 30 个 Qt signal、构造函数和对外方法保持兼容；新模块只向内依赖 domain 的 port、
  model、error 以及 presentation Qt adapter，不反向依赖 ViewModel，避免循环 import。
- `viewmodels.py=918`、`viewmodel_jobs.py=94`，均低于 1000 行；后续新增 worker 逻辑应继续按职责
  拆到 bounded module，不通过 universal mixin/context 降低可读性。
- 本轮验证：Ruff、compileall、`scripts/check.ps1`、30-signal/2-job import contract、source-limit、
  theme token audit、onefile provenance 和三 artifact hash equality 均通过；未启动 GUI/EXE，真实
  窗口、显示器 FPS、硬件/HIL、连接/OTA/debug 与签名验收待授权。架构师与独立 review 调用在服务
  窗口内超时并关闭，未伪造外部 PASS；父代理完成依赖、行为保持、简化与生命周期 fresh-pass。

## ARCH-90 / UI-1.163 共享动效 owner 约束

- `controllers/lifecycle_motion.py` 是共享 presentation motion catalog、可见性快照、frame fan-out
  与 live-RX activity projection 的唯一 owner；它不得创建第二个 timer、读取传输 payload、持有
  ViewModel 业务状态或改变应用生命周期。
- `controllers/lifecycle.py` 继续是 shell 状态/主题/窗口事件协调器，并通过兼容导入保留
  `on_motion_frame`、`invalidate_motion_surface_snapshot`、`set_data_activity_motion` 和内部 stop
  helper 的现有调用路径。`workspace.py`、`terminal_runtime.py`、`workspace_runtime.py` 不得因本轮
  owner 拆分而改写 facade、回调或 lazy import 边界。
- 动效不变量必须保持：`MotionController.TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target、
  elapsed-time phase、reduced-motion/暂停/隐藏/关闭 stop/rearm、activity-only surface gate 和
  可见 snapshot 缓存。120Hz 仍是 scheduler target，不得写成显示器真实 FPS 保证。
- motion catalog 的 surface 顺序、phase offset、`MOTION_MODE` 过滤、隐藏 activity surface stop、
  `_motion_activity_active`/`_motion_data_activity_active` 状态投影和 status footer/live band 更新
  必须保持；拆分不得改变数据接收、暂停显示、连接、记录、回放或错误行为。
- 新文件与协调器均须低于 1000 行；本轮结果为 `lifecycle.py=662`、`lifecycle_motion.py=300`。
  不得为了减少行数引入 universal context/mixin、第二个状态源或循环 import。
- 本轮验证：Ruff、compileall、`scripts/check.ps1`、motion re-export/cadence contract、source-limit、
  theme token audit 和 onefile provenance 均通过；未启动 GUI/EXE，真实窗口、三主题截图、显示器
  FPS、硬件/HIL 与签名验收待授权。架构师与独立 review 调用在服务窗口内超时并关闭，未伪造外部
  PASS；父代理完成依赖、行为保持、简化与资源边界 fresh-pass。

## ARCH-89 / UI-1.162 协议/遥测页横向密度约束

- `controllers/protocol.py:build_protocol_panel()` 继续是协议配置、组件遥测、Dataset/曲线和原始
  回放四个 presentation surface 的组合 owner；本轮不新增 builder，不扩大 `ProtocolPanelWidgets`
  的迁移边界。`composition.py`、`derived_data.py`、`protocol_config.py` 和 `replay.py` 继续只消费
  既有 binding/callback，不读取布局内部对象。
- `protocol_detail_layout` 必须使用语义纵向容器：分隔符 Hex、长度字节、字节序通过既有
  `build_labeled_field()` 组成一行 peer fields，说明文案独立放在后续行；禁止恢复为一次铺开的
  7 列 `QGridLayout`，避免窄窗口下标签和控件互相挤压。
- Dataset 的配置状态与“加载/导出”动作必须分行；回放速度与“选择并回放/暂停/停止”动作必须分行。
  协议页 surface 之间与 surface 内部保持可读的垂直节奏，不能通过压缩间距换取横向内容。
- `ProtocolPanelWidgets` 的 37 个字段、所有 `ProtocolPanelCallbacks`、`itemData`、动态状态投影、
  Tab 顺序、accessibility 和历史回放只读语义必须保持。布局调整不得改变协议应用、重置、组件
  Codec/Dataset 文件动作或回放连接 gate。
- 本轮只创建 presentation layout，不新增 ViewModel、transport、线程、timer、backend 或动画时钟；
  动效继续由唯一 `MotionController`/既有 surface leaf 消费，`TARGET_HZ=120` 与 8ms scheduler target
  的“目标频率”口径不变。
- 本轮验证：Ruff、compileall、`scripts/check.ps1`、37-field import contract、source-limit、
  theme token audit 和 onefile provenance 均通过；未启动 GUI/EXE，真实 980×720/1240×820 几何、
  三主题截图、键盘焦点、显示器 FPS、硬件/HIL 与签名验收待授权。架构师及独立审查调用在服务窗口
  内超时并关闭，未伪造外部 PASS；父代理完成 binding、布局、依赖和简化 fresh-pass。

## ARCH-88 / UI-1.161 命令管理页选择与动作分行约束

- `controllers/command_workspace_builder.py:build_command_workspace()` 是命令管理页的唯一组合
  owner；`terminal.py` 只负责错误通知，不能重新承载历史/批量命令控件构造。`workspace.py` 只把
  返回的 layout 放入既有 `commandPage` 外层 scroll。
- 发送历史保持一行；批量命令必须拆为“选择行 + 命令操作行”，下拉框不与新建/编辑/删除/执行/
  停止按钮争抢同一横向带。操作行允许 trailing stretch，纵向高度由既有外层滚动承载，不得创建
  nested scroll、splitter 或第二个 layout owner。
- `CommandBatchControlBindings` 的 9 个字段、window refs、`currentIndexChanged`、五类 action
  callback、`empty_state.new_requested`、status/results projection、focus/accessibility 和
  `install_tab_order()` 顺序必须保持；布局重排不得改变命令选择不自动执行、停止动作、空态 CTA
  和连接 gate 语义。
- 命令 builder 只创建 presentation widgets，不读取 batch 业务事实、不启动执行、不创建 timer、
  线程、backend 或新 motion clock；运行时继续由 `commands.py`、ViewModel、`composition.py`、
  `lifecycle.py` 消费 typed binding。
- 本轮验证覆盖静态、编译、Ruff、import contract、source-limit 和 theme token audit；未启动 GUI/EXE，
  真实 980×720/1240×820 几何、三主题截图、键盘焦点实测、显示器 FPS、硬件/HIL 待授权。

## ARCH-87 / UI-1.160 终端观测与发送区语义行约束

- `controllers/terminal_toolbar_builder.py:build_terminal_toolbar()` 是实时观测 band 的唯一构建
  owner；`controllers/send_bar_builder.py:build_send_bar()` 是实时发送 band 的唯一构建 owner。
  `terminal.py` 只保留错误通知、发送历史和批量命令 surface；`bootstrap.py` 直接接入两个 builder。
- 两个 live band 使用“输入/控制行 + 状态/动作行”的语义化 `QHBoxLayout`，禁止把标题、输入、
  状态和动作重新压回固定多列 QGridLayout。发送输入必须有伸缩优先级；状态、快捷命令、CRLF 和
  保存动作放在第二行，避免窄窗口下按钮互相挤压。既有外层 terminal surface scroll/viewport owner
  不改变。
- `TerminalControlBindings` 的字段、window staged references、`itemData`、signals/callbacks、
  dynamic `source/state` properties、focus/accessibility 文案和 action enable gate 必须保持；
  新 builder 不读取业务事实、不创建 ViewModel、transport、线程或第二个 animation clock。
- `SendContextSurface`、`SendStateSurface`、`DataActivitySurface` 仍只消费既有 projection；布局拆分
  不得改变暂停显示不停止接收、回车发送、快捷命令只填入不自动发送、批量命令显式执行等语义。
- 本轮验证只覆盖静态、编译、Ruff、import contract、source-limit 和 theme token audit；未启动
  GUI/EXE，980×720/1240×820 真实几何、三主题截图、显示器 FPS、硬件/HIL 仍待授权。

## ARCH-86 / UI-1.159 动效疏朗、120Hz 时钟与连接面板 owner 约束

- `MotionController` 仍是 presentation 层唯一共享时钟，保持 `TARGET_HZ=120`、
  `PreciseTimer`、8ms scheduler target、elapsed-time phase、reduced-motion/暂停/隐藏/关闭
  生命周期。`SignalFieldWidget` 只能消费 `frame_changed`，不得创建局部 timer、第二套 phase
  或把 scheduler target 宣称为显示器真实 120Hz。
- 头部 signal rail 只保留疏朗的三点均匀轨道、低对比度导引线和少量 sparkle；禁止在 148×34
  装饰面内叠加密集网格、过多移动点、长 trail 或业务状态。它必须继续是 NoFocus、鼠标透明、
  无业务事实、可在低动效下静态绘制的 presentation leaf。
- `controllers/network_builder.py:build_network_panel()` 是 TCP Client/Server、UDP、RTT
  endpoint surface 的唯一构建 owner；`connection_builder.py` 只组合 connection shell、UART、
  network/ BLE 返回的 typed bindings，并负责把网络控件接到既有 endpoint callback。不得在 runtime
  controller 里创建控件或复制 `_network_*` 状态。
- `NetworkControlBindings` 字段、window staged-compatibility 引用、默认值、范围、`itemData`、
  server peer callback、allowlist/LAN gate、Tab/accessibility 文案必须保持；网络面板的 12px
  横向 / 10px 纵向间距用于缓解拥挤，不得通过删除字段或新增 nested scroll 规避布局问题。
- `connection_primitives.py` 只提供无状态 label/combo/hint helper，不读取 window、ViewModel、
  domain 或 transport；OTA/AES、RTT/J-Link contract-only/attach-only 边界不因本轮视觉改动提前
  接入。每个源文件继续不超过 1000 行。
- 本轮已授权验证只包括静态、编译、Ruff、import contract 与既有检查；未启动 GUI/EXE、未执行
  真实连接、硬件、显示器刷新率或 HIL 验收。后续若得到明确 UI 运行验证授权，必须覆盖
  980×720/1240×820、三主题、signal rail 无重叠、network horizontal maximum=0、exact-white=0、
  reduced-motion、hide/show、close 与 120Hz scheduler 采样。

## ARCH-85 / UI-1.158 派生数据空态表面与垂直密度约束

- `controllers/protocol.py` 只声明组件帧与 Dataset 预览的初始 presentation 状态；
  `controllers/derived_data.py` 是运行时可见性投影的唯一 owner。不得让协议 builder、ViewModel
  或业务 pipeline 各自维护第二份显示状态。
- 组件无帧时只显示 `ComponentEmptyStateSurface`；`component_preview` 仅在已有协议帧时展开。
  Dataset 无样本时由既有 `dataset_status`、配置动作和曲线空态提供引导；`dataset_preview` 仅在
  有样本时展开。隐藏预览仍保留原 objectName、AccessibleName、placeholder、最大块数和内容
  contract，数据到达后必须恢复可见。
- 空态切换不得改变 Component/Dataset/Curve 的 typed 数据、过滤器、CSV action、replay、
  callback、Tab/focus/accessibility 或任何 transport 行为；不得创建 nested scroll、局部 timer、
  第二个 animation clock 或新的状态源。每个源文件继续不超过 1000 行。
- 必须验证真实组合根在 980×720 与 1240×820、三主题下无横向滚动、无组件/预览重复空白表面、
  数据态可恢复、exact-white=0、reduced-motion、hide/show 与 close 清理。`TARGET_HZ=120`、
  `PreciseTimer`、8ms scheduler target 和“不是显示器精确 120fps”的口径保持不变。

## ARCH-84 / UI-1.157 BLE 配置密度与 owner 约束

- `presentation/controllers/ble_builder.py:build_ble_panel()` 是 BLE GATT 参数 surface 的唯一组合 owner；
  `connection_builder.py` 只负责连接壳、网络/UART/RTT 组合和把返回的 `BleControlBindings` 接入外层
  `QVBoxLayout`，不得重新复制 BLE 控件构造。
- BLE surface 必须按“扫描与筛选 → 设备发现与连接 → GATT 特征与收发”语义分组，字段使用
  `form_fields.build_labeled_field()` / `build_field_row()` 形成可读的垂直标签和均匀伸缩行；禁止用
  7 列固定网格把扫描、过滤、设备、特征和收发动作压缩到同一横向带，也不得创建 nested scroll、
  splitter、第二套响应式 owner 或局部 timer。
- `BleControlBindings` 字段、窗口 runtime 兼容引用、`BleGattWriteMode` 的 `itemData()`、扫描/设备/
  特征/通知/读取/写入 callback、默认值、Tab/focus/accessibility 和“手动扫描、显式连接、不自动重连”
  边界必须保持。布局重排不得触发真实扫描、连接、配对、通知订阅或写入。
- 980×720 与 1240×820、三主题必须验证 BLE active panel 的 horizontal maximum=0、字段矩形无重叠、
  关键控件可聚焦、exact-white=0、reduced-motion 与 close 清理；允许纵向内容由既有 settings scroll
  承载。该切片不修改 `MotionController` 的 120Hz scheduler、lifecycle fan-out 或任何 transport backend。

## ARCH-83 / UI-1.156 扩展能力卡可用性与 owner 约束

- `embedded_extension_panel.py` 是扩展工具站的唯一 presentation 组装 owner；它只消费
  application-owned immutable `ExtensionCapability` DTO，并在本地维护 7 张卡片的单选展示状态。
  `ExtensionPanelWidgets` 只暴露外层 `layout` 与 `station_overview`，详情控件保持 panel 内部私有。
- 每张能力卡必须是一个完整的 `QPushButton` 语义控件且只有一个 Tab stop；整卡点击、Enter、空格和
  获得键盘焦点都必须落入同一个选择路径，始终保持恰好一张卡选中。选择只更新高亮和详情，不得
  连接设备、启动 OTA/debug、读取密钥、创建 backend、改变 application/domain 状态或增加 timer。
- `extension_capability_detail.py:ExtensionCapabilityDetail` 是无状态叶子展示组件，只投影当前 DTO 的
  `title/group/reference/key/state/summary/boundary` 和固定的“不执行”边界；不得拥有卡片列表、回调、
  transport handle、密钥、vendor SDK、线程或持久化。详情复用扩展页既有外层 `QScrollArea`，禁止嵌套
  scroll、splitter、sticky panel 或第二套滚动 owner。
- `contract_only` 继续使用 info/blue 语义，`attach_only` 继续使用 history/purple 语义；base stylesheet
  与 `theme_variant_shell.py` 必须对称覆盖 card/detail selector，禁止白色 literal 或原生默认表面回退。
  `theme_stylesheet.py` 只负责按 `BASE_STYLESHEET + EXTENSION_STYLESHEET + CONTROLS_STYLESHEET`
  组合，extension 专属 QSS 不回流到 base 模块。
- 必须验证 980×720、1240×820、三主题、7 张卡片的 click/focus/Tab/Shift-Tab/Enter/Space 可操作性、
  详情 raw identity、恰好一项 selected、可见扩展滚动页 `horizontal maximum=0`、exact-white=0、
  reduced-motion 与 close 清理，以及 DTO catalog 未被选择动作修改。该条款 supersede UI-1.120 中
  “静态 QFrame、NoFocus、不可操作”的历史 affordance 限制，但保留其只读 DTO、无 backend、无动作执行
  和 OTA/debug contract-only/attach-only 边界。

## ARCH-82 / UI-1.155 动效快照与 120Hz scheduler 约束

- `MotionController` 必须是 presentation 层唯一共享时钟；保持 `TARGET_HZ=120`、`PreciseTimer`、
  8ms scheduler target。不得把 scheduler target、offscreen 回调或 compositor 事件间隔宣称为真实显示器
  120Hz；动画 phase 必须按 elapsed time 推进。
- `lifecycle.py` 必须维护完整 surface catalog 与可见 snapshot。逐帧 fan-out 只能消费缓存 snapshot，
  不得在每帧对 56 个 surface 做 `isVisibleTo()` 扫描；Show/Hide/ParentChange、resize、route、主题和
  native window-state 边界必须使 snapshot 失效。完整 catalog index 必须保留，用于稳定 phase offset。
- hide/minimize/close 必须停 clock、停 surface、清理 activity；show/restore 必须经过 closing、visible、
  minimized fence，并只在 `rearm_required` 时 rearm controller-owned timer。queued callback 可以是有限的
  两跳 lifecycle defer，但不得使用 sleep、processEvents、无界重试、第二个常驻 timer 或第二个 frame fan-out。
- pause 和 reduced-motion 是既有 policy gate，不得设置 host-visibility `rearm_required`，也不得由恢复回调
  强制绕过；`frame_changed` signal、phase、activity、controller identity 和业务状态保持不变。
- 本切片只能存在于 `presentation/`；不得接入 domain/application/infrastructure、设备 I/O、OTA/AES、
  RTT/J-Link 或新线程。每个源文件继续不超过 1000 行，所有新增状态必须有 lifecycle owner 和 close fence。

## ARCH-81 / UI-1.154 UART 字段布局约束

- UART 展示布局只能由 `controllers/connection_builder.py` 组合；三行字段必须复用
  `form_fields.py` 的 `build_labeled_field()` 与 `build_field_row()`，不在 domain/application 或
  network/BLE builder 中复制 Qt 布局逻辑。
- 允许建立 `port_controls` 与 `line_controls` 两个局部 presentation 容器；它们只能组合已有
  端口/刷新按钮与独占/DTR/RTS 控件，不得创建第二份控件、状态源、timer 或连接动作。
- 禁止改变 `UartControlBindings` 字段名、combo `itemData`/枚举值、默认值、timeout range、信号
  callback、连接 gate、Tab 顺序、AccessibleName/Description 或手动输入 COMx 契约。摘要必须继续
  由已有 combo projection 刷新。
- 验证至少覆盖 980×720 与 1240×820、三套主题、10 个字段的无交叠几何、可见 settings scroll
  page 的 horizontal maximum=0、目标控件 Tab 顺序、默认值/typed value/摘要回写和 close 清理。
  不得把 `TARGET_HZ=120` scheduler target 改写为精确显示器 120fps。
- 本切片不得接入 domain、application、infrastructure、硬件、设备 I/O、OTA/AES、RTT/J-Link 或
  新的全局动效时钟；源文件仍必须保持不超过 1000 行。

## ARCH-78 / UI-1.151 动效层级约束

- `MotionController` 必须保持 presentation 层唯一时钟；`activity_active()` 只能是只读的时钟
  投影，不得让 widget 创建 timer、读取业务状态或自行调用 `request_activity()`；120Hz 仍表示
  约 8ms scheduler target，不得宣称显示器精确 120fps。
- `ActionRailButton`、`BusyActionButton`、`TerminalEmptyState`、`CommandBatchEmptyState` 和
  `ComponentEmptyStateSurface` 只能通过 `MOTION_MODE="activity"` 声明短时 activity 动效；
  `AnalysisStatusLabel` 等持续语义状态不能被误标为 activity-only。该 marker 不得改变点击、
  enabled、checked/payload、objectName、焦点、Tab 或 accessibility 契约。
- `controllers/lifecycle.py` 是 activity fan-out、下降沿 stop、隐藏 surface 一次性 stop 和
  全局 cleanup 的唯一 owner。activity-only surface 在 activity=false 时不能每个 ambient frame
  重复 `set_frame(..., False)`；隐藏、最小化、暂停、低动效和关闭必须停止并静态收敛，重新可见
  后才恢复。不得为此新增 widget timer、全局 registry、业务状态、线程、设备 I/O 或 OTA/AES/
  RTT/J-Link 依赖。

## ARCH-79 / UI-1.152 命令空态密度约束

- `build_send_panel()` 必须通过 trailing stretch 吸收命令页空余高度；`CommandBatchEmptyState`
  只能按自身 size hint/minimum height 展示，禁止通过扩大空态、固定窗口坐标、额外 resize handler
  或 timer 填充空白。空态仍需保留现有 `new_requested`、CTA、accessible name、batch snapshot
  可见性和 shared motion contract。
- ARCH-79 的调度记录必须区分 `TARGET_HZ=120`、`PreciseTimer`/8ms scheduler 配置与 Windows
  offscreen 实测回调间隔；不得把约 15.6ms 的平台观测包装成精确 120fps，也不得未经真实可见
  GUI 基准就改为 deadline/0ms/adaptive timer。实际 phase 继续按 elapsed time 推进。

## ARCH-80 / UI-1.153 顶部 header 密度约束

- header 响应式策略只能由 `presentation/controllers/workspace.py` 的 `_AdaptiveHeader` 与
  `_sync_header_density()` 持有；不得把 header 宽度判断扩散到 `lifecycle.py`、application、domain
  或各 feature builder。
- 当 header 的 Qt logical width `< 1120` 时，只允许隐藏 `SignalFieldWidget` 与
  `ThemePaletteSwatch` 两个纯装饰件，并由 `ResponsiveHeaderControls.set_density(compact=True)` 将其 owner grid spacing 从 8px 调整为 6px；宽度恢复时
  必须原位恢复。`statusCluster`、`motionControls`、`themeControls`、主题 combo、低动效/暂停
  checkbox、连接状态、焦点/Tab、AccessibleName/Description 均不得被隐藏、重排、禁用或改语义。
- 本切片不得新增 `QTimer`、MotionController、业务状态、偏好字段、事件总线、设备 I/O、网络、
  OTA/AES 或 RTT/J-Link 依赖。装饰件已有 NoFocus/鼠标透明/空 accessibility 语义，不能借此隐藏
  任何用户可操作控件。
- 验证必须覆盖 header logical width 边界、三主题、980×720 与宽屏恢复、可见 settings scroll
  page 横向 maximum=0、低动效/暂停、hide/show/close；120Hz 仍只表示 scheduler target，不能
  将 offscreen timer 观测写成显示器精确帧率。

## 产品约束

- Windows-first，首版只追求稳定的本地调试，不默认联网、遥测或云同步；
- UART 是第一个完整 vertical slice；TCP/UDP、BLE GATT、RTT 是后续独立适配器；
- 终端必须同时支持文本和 Hex，保留原始字节；
- 日志、发送历史、快捷命令、筛选、暂停滚动和错误状态属于基础体验；
- 设备 profile 只描述连接和解析，不执行任意 Python、命令或 DLL；
- schema v1 固定字段 profile 必须向后兼容；schema v2 JSON/TLV 只能使用有限声明式字段、
  JSON Pointer 和平面 TLV，禁止 wildcard/filter/表达式/脚本/动态导入；
- codec 仅生成 RX 派生预览，raw JSONL、终端、framing/checksum、发送和会话生命周期不受其失败影响；
- Dataset 只消费 typed component value；transform chain 最多 8 项，输入队列最多 128 项/256 KiB，保留窗口最多 1024 条；
- Dataset 配置只允许 scale、offset、clamp、enum 四类内置声明，不执行表达式、脚本、动态导入或自动 TX 编码；
- 历史回放只消费严格 recorder JSONL 的 RX 原始记录；不连接设备、不发送 TX、不重新录制，必须标记 `historical` 来源；
- replay parser segment 以连续有效 RX 记录的原始 `RawRecord.session_id` 为边界；segment 变化必须在
  protocol worker FIFO 顺序中结束旧 partial frame 并清理 parser 状态，不得由 replay 线程直接跨层 reset；
- 回放行、文件、记录、协议 ingress 和派生窗口均有界；暂停只冻结回放时钟，不把背压静默转化为数据丢失；
- 曲线只消费 Dataset typed value 的一个选中 series；最多 512 个有限数值点，错误/非数值不转换成零，历史与实时不混画；
- 批量命令只允许 immutable 的固定步骤序列：最多 16 个宏、32 步/宏、512 B/步、16 KiB/宏、180 s 总等待；不允许脚本、循环、条件、变量、ACK 等待、自动重试、广播或 RTT 批量发送；
- 批量命令执行只报告本地发送队列的“已提交”，不伪装成设备已收到/处理；停止不能撤回已入队数据，session 断开后不得切换目标；
- 协议预设只提供有界、不可变的 `ProtocolConfig` 模板；选择预设不自动改变 parser，必须显式应用；未知的 Modbus RTU、MAVLink、NMEA 语义不能由通用 preset 名称冒充完成；
- OTA 扩展先保持 contract-only：传输协议槽位固定为 XMODEM、YMODEM、TFTP 三个独立 adapter 目录；在目标 bootloader、包格式、超时、重试和恢复策略完成一手资料确认前，不得把槽位显示为“已支持 OTA”；
- OTA 传输必须与镜像读取、目标身份、升级编排、校验和 UI 分离；传输 worker 只报告有界进度和错误，不把“本地发送完成”描述成设备已经刷写或启动成功；
- NMEA 0183 当前只允许 Line RX checksum profile：要求 `$...*HH` 的非空 ASCII sentence，格式错误与 checksum 错误可见；不解析标准字段、不自动 TX、不宣称认证或全版本兼容；
- Modbus RTU 当前允许独立的 `MODBUS_RTU_TIMED` stream boundary 加已分帧 ADU RX codec：只有带
  `HOST_READ_GAP`/`DEVICE_TIMESTAMP` 质量的 gap 才能应用 t1.5/t3.5；普通 read chunk 不能证明
  物理静默间隔；地址 0–247、功能码基本格式、256 B 上限和 CRC16 低字节在前可见；不做完整
  事务/功能码语义、不自动 TX；
- MAVLink 当前允许独立的 `MAVLINK_STREAM` 有界结构提取/resync 加 v1/v2 RX validator/codec：
  CRC_EXTRA 必须来自 profile 的显式 message-id 映射；结构完整但缺失映射和未认证 v2 signature
  必须是 `UNVERIFIED`，不做完整 dialect 字段解析、签名认证或 TX；
- protocol/component/Dataset 派生事件必须携带本阶段和上游 generation；配置/reset 后低于 fence 的事件不得进入下游 worker、统计、样本或 UI；generation barrier 不得改变 raw recorder 与 transport 事实路径；
- 错误栏由 `SessionViewModel` 单一持有 `ErrorInfo | None`；MainWindow 的本地表单错误必须通过
  `show_local_error()` 进入同一状态，禁止直接写 QLabel/status bar 形成第二套错误源；
- Windows 打包只允许通过 scripts/package.ps1；core/BLE 与 onedir/onefile 必须按版本、变体、模式
  隔离输出，禁止共享 staging 或用旧产物冒充当前包；每个模式必须带 PROVENANCE.json、SHA256、
  PyInstaller archive listing、依赖树、NOTICE 和第三方清单；
- 完整 IDE 调试、烧录、动态插件市场和脚本引擎不属于首版。

## 工程约束

- Python 3.12，Windows x64；
- PySide6 只存在于 presentation；
- 设备库只存在于 infrastructure；
- application 依赖 domain ports，不依赖具体 Qt 或设备库；
- 一个模块一个职责，公开边界使用类型化接口和 DTO；
- `presentation/main_window.py` 只能作为 composition shell：负责窗口初始化、主题、controller 组装、跨区接线和统一生命周期；不得继续承载连接、终端、协议、回放、批量命令等具体工作区流程；
- composition/workspace 的 owner controller 必须由 bootstrap 或 owner module 直接调用；`MainWindow` 不得为纯页面构建、滚动页包装、焦点/快捷键安装、
  workspace Tab 变化或 workspace 淡入保留 pass-through facade。workspace 内部可以组合既有 presentation builder，但不得借助动态属性、
  `__getattr__`、mixin 或无限制 callback map 隐藏依赖；生命周期调用必须直接使用 `workspace_runtime.py` 的显式函数。
- `presentation/main_window.py` 与 `src/serialforge/presentation/controllers/*.py` 每个 Python 源文件最多 1000 行；由 `scripts/check_source_limits.py src` 和 `scripts/check.ps1` 强制执行。超过上限必须按稳定变化边界拆分，不能通过压缩格式或隐藏大段字符串规避；
- 纯连接配置 builder 必须由 `controllers/connection_runtime.py` 这个 owner module 直接拥有和调用；BLE controller 通过显式模块函数调用，不能把 UART/TCP/UDP/RTT/BLE config builder 重新挂回 `MainWindow` facade。逐族迁移不得使用 `__getattr__`、动态注册、monkey-patch 或 mixin 隐藏依赖；Qt 生命周期和真正的 QWidget parent 仍由 `MainWindow` 持有。
- 连接面板的动作由 `ConnectionActionCallback` 等显式 typed callback 注入；builder 只组装 QWidget，不能通过 `MainWindow` 保留
  纯动作转发。Qt signal 的 checked/payload 参数必须在 owner action 边界安全吸收，不能改变业务语义或把信号细节泄漏到 domain。
- 纯展示格式化（例如端点选择器、历史预览标签）必须放在 `presentation/formatters.py` 等无状态模块；formatter 只能依赖
  domain DTO，不得读取 MainWindow、Qt widget、ViewModel 或 controller，调用方必须显式导入，禁止为纯函数保留静态 facade。
- `presentation/property_refresh.py` 只提供有界的动态属性刷新原语：属性值未变化时不得重复 polish，变化时必须按
  `setProperty` → `unpolish/polish` → `update` 顺序刷新；它不得读取 ViewModel/domain 状态、创建 timer 或反向依赖 controller。
- 状态 surface 的 source 投影必须由 `controllers/status_surfaces.py` owner 提供并通过 bootstrap 显式注入；TCP Server
  的 IPv4/回环/LAN-confirm/allowlist readiness 必须由 `controllers/connection.py` owner 直接计算，禁止通过 MainWindow
  保留单调用点 facade，且不得放宽原有网络授权 gate。
- `presentation/command_selection.py` 只负责从 bounded combo 读取 typed command mode/batch；必须保留 mode fallback、
  `Qt.ItemDataRole.UserRole` 和 `None` 语义，不得把发送/批量 gate、ViewModel 状态或 transport 逻辑塞入 selector，也不得通过
  MainWindow 暴露纯读取 facade。
- `presentation/ble_selection.py` 只负责读取 bounded BLE write-mode combo 和 characteristic capability；必须保留
  `Qt.ItemDataRole.UserRole`、enum fallback、`None`/空 capability 语义，不得读取 ViewModel、设备句柄或 transport。会修改
  combo 的 normalize action 留在 `controllers/ble.py`；connection/commands 直接导入 selector，MainWindow 不得保留纯 BLE
  selection facade。
- `presentation/protocol_scope.py` 只负责 parser/derived-source applicability 查询；必须保留 history、UART/TCP Client 与
  UDP/raw-only gate 语义，不得修改 editor、worker、ViewModel 或控件。`protocol_config.py` 保留 enable/reset/status 副作用，
  其他 owner controller 直接导入 query，MainWindow 不得保留纯 protocol scope facade。
- ARCH-6n 的连接控件刷新必须由组合根在 `controllers/bootstrap.py` 绑定一个显式命名回调
  `window._refresh_connection_controls`，其实现只能指向 `controllers/connection.py:update_connection_controls`；owner controller 不得再调用
  `MainWindow._update_connection_controls` 纯转发。回调绑定不得复制 enable/hint/busy 业务规则、引入无限制 callback map、动态注册、mixin 或第二个状态源，
  首屏 hydration、session state、历史回放 gate、BLE/TCP Server readiness、关闭时序和 accessibility 必须保持不变。
- ARCH-6o 的 transport/protocol framing callback 必须由组合根显式绑定为
  `window._on_transport_changed` 与 `window._on_protocol_framing_changed`；`MainWindow` 不得重新声明同名纯转发方法或导入对应 owner controller。
  builder、preset、replay、composition、protocol signal 的现有调用可以继续消费 callback，但必须保持 Qt payload 吸收、transport panel 可见性、协议草稿/应用语义、首屏 hydration、
  lifecycle fence 和 accessibility，不得借此把 connection/protocol policy 复制到 bootstrap。
- ARCH-6p 要求剩余 callback wiring 按 feature owner 分组在 `controllers/bootstrap.py` 的显式 `_bind_*_callbacks()` 中完成：
  protocol、derived、replay、BLE、terminal、lifecycle 每个绑定函数只能做 `partial(owner_function, window)` 接线，不能复制业务判断、构造第二套 DTO、
  引入无限制 callback registry、`__getattr__`、mixin 或动态 monkey-patch。所有绑定必须早于对应 Qt timer/signal 使用；`MainWindow` 只允许保留构造和四个 Qt 生命周期覆写，
  原 callback 名称、payload、首屏 projection、focus/Tab/accessibility、关闭 fence 和 owner policy 必须保持。
- 发送、批量编辑/删除/执行等 Qt action 必须由 `controllers/commands.py` 的 owner adapter 吸收 signal checked/payload；
  `terminal.py`/`composition.py` 通过显式 `partial` 接线，MainWindow 不得为单一 action 保留兼容 facade，原 action 的
  dialog parent、ViewModel command 和生命周期语义必须保持不变。
- commands 的 batch signal/结果投影必须由 `commands.py` canonical owner 直接调用；bootstrap 只做显式 signal 注入和首屏
  owner 调用，terminal 只连接 combo signal。禁止为同一 owner function 在 MainWindow 保留 signal/结果转发 facade，且必须
  保留首屏 snapshot、状态表面投影与 connection controls refresh 顺序。
- workspace 的 Tab 可见性、派生 surface 挂起、motion policy 和一次性过渡必须由 `workspace_runtime.py` 单独拥有；`workspace.py`
  只负责控件组装与显式 signal 接线，`lifecycle.py` 只消费 runtime helper。两者不得互相导入，不得通过 MainWindow 或延迟动态
  facade 隐藏循环依赖；主题/动效 callback 的 checked/index/frame payload 必须在 owner 边界吸收并保留静态回退。
- `presentation/motion_policy.py:decorative_motion_enabled()` 是一次性/环境装饰动效共享的偏好判定，不承载连接、协议或
  数据状态；`workspace_runtime.py`、transport transition 和其他 presentation owner 必须复用它，不得各自复制 reduced-motion/
  motion-paused 判断。它不创建 timer，不改变业务 worker 或终端滚动。
- `presentation/transport_panel_transition.py` 只能消费 `connection_runtime.py` 已完成显隐 projection 的一个 QWidget，
  以最多一个短时 `QGraphicsOpacityEffect/QPropertyAnimation` 对当前可见 UART/TCP/UDP/BLE/RTT 配置 panel 做 160ms fade。
  它不得读取 ViewModel、修改 transport config、改变 panel focus/tab/accessibility、自动连接或把 fade 当成状态源；已有
  `graphicsEffect()` 必须保留，自己的 effect/animation 必须在快速切换、主题切换、低动效、暂停、隐藏、最小化和关闭时清理。
  不得创建常驻 QTimer、加入 MotionController fan-out 或制造横向滚动；静态回退和三主题 near-white 门禁是固定验收项。
- `presentation/dialog_transition.py` 是自定义对话框入口过渡的唯一 owner，只允许由
  `command_batch_editor.py` 与 `connection_preset_editor.py` 的 `showEvent`/`hideEvent` 调用。标准参数固定为 150ms、
  opacity `0.90 -> 1.0`、`OutCubic`；必须复用 `motion_policy.py` 的父窗口低动效/暂停/环境回退，不能读取 ViewModel、draft、
  transport、协议或安全状态，也不能把动画当成业务状态源。已有 `graphicsEffect()` 不得覆盖；自然完成、重复 show、hide、
  close、隐藏/最小化和关闭路径必须解绑 effect、停止并销毁 animation，避免保留悬空 QObject 或半透明 dialog。该过渡不得改变
  dialog 的按钮 action、校验、焦点、Tab/accessibility、popup、尺寸或原生 `QMessageBox`/`QFileDialog` 行为；禁止新增常驻
  `QTimer`、第二套 MotionController、图片/GIF/字体资源和跨层依赖。验收至少覆盖三套主题、正常动效、reduced-motion、显式暂停及
  hide 后 `graphicsEffect() is None` 的一次性生命周期向量。
- `ConnectionPresetEditorDialog` 的 `presetMetadataFields` 只能作为 bounded presentation surface：必须复用既有
  `QWidget[role="surface"]` selector，名称/备注字段标签必须使用 `QLabel[role="muted"]`，不得新增主题 token、独立 QSS 文件、
  业务状态或动画时钟。surface 不得改变 `ConnectionPresetMetadata` 的 key/label/description、空名称和长度校验、保存/取消 action、
  活动会话 gate、AccessibleName/Description、初始 label focus 或 Tab 顺序；controller/store 继续拥有 DTO、持久化与成功/失败反馈。
  三套主题必须有非白色 surface/label fallback，隐藏 dialog 后仍由 `dialog_transition.py` 负责清理 effect。
- UI-1.76 的快速配置选择反馈只能由 `connection_builder.py:_dispatch_connection_preset()` 在既有 combo selection projection
  完成后请求 `window._motion_controller.request_activity(420)`；`ConnectionPresetContextSurface` 不得创建 timer、解释连接状态、
  自动连接或改写 combo value/tooltip/AccessibleDescription。该请求可在选择内置、自定义或清空选项后触发，但不能在
  `refresh_connection_preset_combo()` 的 blocked-signal hydration 中伪造用户活动。动画只复用既有 frame/stop fan-out，必须在
  reduced-motion、显式暂停、隐藏、最小化和关闭时静态；selection、apply callback、焦点、Tab/popup/accessibility 和三主题 near-white
  契约必须保持。
- UI-1.77 的连接方式 section 标题必须与对应参数 panel 使用同一份 transport projection：UART 只显示 UART 标题/panel，BLE GATT 只显示
  BLE 标题/panel，TCP Client、TCP Server、UDP 和 J-Link RTT 只显示网络标题/panel；不得留下隐藏内容的孤立标题或额外空行。
  `connection_builder.py` 只能保存三个 presentation widget 引用，`connection_runtime.py:on_transport_changed()` 负责同步
  `setVisible`；不得把标题可见性放进 domain/ViewModel/DTO，不得新增 timer 或改变既有 `transport_panel_transition.py` 的 active-panel owner。
  三个标题必须继续使用 `role="section"`，首次 hydration、六种链路、三套主题、切换后 effect 清理和关闭路径都必须保持 parity；焦点、Tab、
  AccessibleName/Description、combo/panel 业务状态和 reduced-motion 静态回退不得改变。
- UI-1.78 的工作区导航 activity 只能由 `workspace.py` 将 `QTabWidget.currentChanged` 接入
  `workspace_runtime.py:request_workspace_activity()`；该 owner 只允许复用 `MotionController.request_activity(320)`，不得创建新的
  `QTimer`、导航状态、event bus 或改变 `on_workspace_tab_changed()` 的 renderer suspension / `animate_workspace_transition()` 的 page fade。
  请求必须在窗口已可见且未隐藏、未最小化、未关闭时才生效；首次 hydration、show/hide、最小化/恢复、reduced-motion、暂停和关闭不得伪造
  用户 activity。三主题、三个 workspace、键盘/鼠标切换必须保持 Tab 文案、index、focus、AccessibleName/Description 和读屏顺序不变；
  route beacon 仍是 NoFocus/鼠标透明的装饰层。
- UI-1.79 的错误 activity 只能由 `lifecycle.on_error_changed()` 在确认非空 `ErrorInfo` 后复用
  `MotionController.request_activity(520)`；必须先检查窗口可见、未关闭，并允许 `_motion_controller` 尚未存在时安全返回。它不得创建
  `QTimer`、改变错误 DTO/严重性/文案/清除按钮/焦点、把 pulse 当成错误数量或进度，也不得在 `info is None` 的清除路径请求动画。
  三套主题下 errorBar/`ErrorSignalSurface` 的 active/clear 语义必须保持；初始化、隐藏、最小化、暂停、reduced-motion 和关闭保持静态，
  读屏只读取既有 error message 与 AccessibleDescription。
- UI-1.80 的派生 activity 只能由 `controllers/derived_data.py:_request_derived_activity()` 复用
  `MotionController.request_activity(360)`；调用方只允许在已验证的非空 `tuple[ComponentFrameRow, ...]` 或
  `tuple[DatasetSample, ...]` 快照进入 `on_component_rows_changed()`/`on_dataset_samples_changed()` 时请求。必须检查窗口可见、未最小化、未关闭且
  Protocol workspace 当前可见；不得创建 `QTimer`、复制 stats/rows/samples、修改 DTO/表格/预览/曲线内容或把 pulse 当成吞吐/进度。空快照、bootstrap
  hydration、后台页、暂停、reduced-motion、隐藏和关闭必须保持静态；三套主题、实时/历史来源、clear 和已有 renderer throttle 语义不变。
- UI-1.81 的回放 activity 只能由 `controllers/replay.py:_request_replay_activity()` 复用唯一 `MotionController`：PLAYING 使用既有 520ms，
  EOF/STOPPED/ERROR 使用一次 480ms；必须检查窗口可见、未最小化、未关闭，并允许 controller 尚未存在时安全返回。PAUSED/EMPTY、bootstrap hydration、
  隐藏、最小化、暂停和 reduced-motion 不得伪造 activity。`ReplayActivityLabel` 的 history/error marker 只能读取已有
  `ReplayActivityProjection`，不得创建 `QTimer`、记录进度/百分比、改变 replay DTO、按钮状态、source badge、文案、focus 或 accessibility；三套主题
  必须保持 history/error 语义色和静态回退，关闭时由既有 lifecycle stop 清理。
- UI-1.82 的批量命令 activity 只能由 `controllers/commands.py:_request_command_batch_activity()` 复用唯一 `MotionController`：RUNNING 使用既有
  520ms，COMPLETED/STOPPED/FAILED 使用一次 480ms；必须检查窗口可见、未最小化、未关闭，并允许 controller 尚未存在时安全返回。IDLE、bootstrap hydration、
  隐藏、最小化、暂停和 reduced-motion 不得伪造 activity。`CommandBatchSurfaceLabel` 的 completed/stopped/failed marker 只能读取已有 bounded
  `CommandBatchSurfaceProjection`，不得创建 `QTimer`、百分比/时间进度、改变 batch snapshot、结果表、按钮、发送队列、焦点或 accessibility；三套主题
  必须保持 success/warning/error 语义色和静态回退，关闭时由既有 lifecycle stop 清理。
- UI-1.83 的 `AnalysisStatusLabel` 只能读取既有 `state/source` presentation properties 与共享 `set_frame()` 输入：`error`、`blocked`、`history` 只允许绘制
  静态叉、双横栏、回退箭头 marker，marker 不得成为唯一语义来源；文字、AccessibleDescription、objectName、QSS、焦点和 controller projection 必须保持不变。
  marker 必须使用 `theme_spec_for_widget()` 的语义 token，不得硬编码白色/背景色，不得新增 DTO、状态源、事件总线、百分比、局部 `QTimer` 或业务时钟；未知状态、
  empty/idle、低动效、暂停、隐藏、最小化和关闭必须保持静态中性回退，三套主题必须可渲染。
- UI-1.84 的三个设置页必须统一经过 `controllers/composition.py:scroll_page()`：content 使用横向 `QSizePolicy.Expanding`，`QScrollArea` 必须
  `widgetResizable=True` 且 `HorizontalScrollBarPolicy=ScrollBarAlwaysOff`，纵向策略保持 `ScrollBarAsNeeded`。不得在 feature controller 中单独创建横向滚动条、
  用亮色槽表达布局边界或为了隐藏横带修改业务内容；页面 objectName、键盘 Tab 顺序、AccessibleDescription、长文案可见性和三主题 palette 必须保持不变。
- UI-1.85 的 stable QSS 不得直接写入近白 `#fff…` 文字/选中色：默认文字必须复用 `{TEXT}`，选中/高亮文字必须复用 `{SELECTION_TEXT}`，对应 token
  必须由 `ThemeSpec`/variant renderer 提供。该约束只收敛颜色来源，不得改变 selector、焦点环、selection background、Tab 文案、表格内容、AccessibleName/Description
  或业务状态；三套主题的 stylesheet 必须无白色 background fallback，并且实际 checkbox/combo/table/tab widget 可渲染。
- UI-1.86 的 stable control QSS 必须复用已有语义 token：`QTableWidget` 使用 surface/border/selection/interaction roles，表头和 Tooltip 使用
  history/surface/text roles，状态栏使用 background/surface/history border，双向滚动条与 corner 使用 surface-input/neutral-border；不得在
  `theme_stylesheet_controls.py` 新增散落 hex literal、白色 fallback、第二套主题 palette、局部 timer 或业务依赖。三套主题的 variant override
  仍由 `theme_variant_controls.py` 持有，stable template 不得复制 variant policy；必须通过静态 token audit 与 Qt offscreen surface vector。
- UI-1.87 将语义 token 约束扩展到 stable controls 的状态面、输入/禁用态、按钮、checkbox、workspace Tab、terminal 和空态：颜色必须来自已有
  `ThemeSpec` 角色，不能新增 hex literal、palette registry、selector、局部动画时钟或 controller/设备依赖。必须保留原有 property state、焦点、选择、禁用、键盘和
  accessibility 契约；三主题的实际视觉覆盖仍归 `theme_variant_controls.py`，通过静态 token audit 与真实 Qt offscreen 控件组合 vector。
- UI-1.88 要求 `theme_stylesheet_base.py` 的 stable shell 也不得包含 hex literal：app root、section/error/status、connection/station/send band、pipeline/status/badge
  的颜色必须复用现有 `ThemeSpec` 语义角色；不得改变状态 selector、property projection、动效时序、布局、焦点、键盘和 accessibility。base/controls template 不得
  复制 variant policy 或新增 palette registry；三主题必须通过 shell state offscreen vector，稳定模板 literal audit 必须为零。
- UI-1.89 要求 `controllers/lifecycle.py:on_motion_frame()` 作为共享 `MotionController.frame_changed` 的末端 lifecycle fence：`_closing`、不可见、最小化或
  `workspace_motion_enabled()` 为 false 时不得向装饰 surface 分发排队帧，必须调用统一 stop 并清除 data-activity presentation property；不得修改
  SessionViewModel、transport、recorder、parser、send queue 或新增第二个动效策略源。正常可见且允许动效时必须保持既有 fan-out；验证至少覆盖四种阻断态和一种正常态的无窗口 Qt 向量。
- UI-1.90 的扩展工具站只能消费 `application.extension_capabilities` 的不可变 DTO：XMODEM/YMODEM/TFTP 和 AES-GCM/CCM 必须显示为契约预留，RTT/J-Link 必须显示为 attach-only；presentation 不得导入
  OTA/debug adapter、socket、pyserial、SEGGER SDK/DLL、加密库或原始密钥，不得提供升级、解密、签名激活、memory/halt/run/reset/flash 或 vendor 命令动作。第四个 Tab 只增加只读导航，前三个业务页的 index、焦点、状态和 `protocol_tab_index` 必须保持。
- UI-1.91 的 capability card 必须保持标题与状态 badge 的语义分层：`QLabel#extensionCapabilityTitle` 只表达正文标题，不能继承通用 `role="status"` 胶囊外壳；`QLabel#extensionCapabilityState` 只表达 contract maturity，`contract_only` 使用 info surface/border，`attach_only` 使用 history surface/border 与 purple accent。selector 必须复用已有主题 token，覆盖默认模板和三套主题回退；不得新增 hex literal、palette、DTO 字段、业务状态源、常驻 timer、交互动作或 OTA/debug 依赖。卡片数量、可访问名称/描述和只读边界必须保持。
- UI-1.92 的 workspace Tab transition 必须是一次性、presentation-only opacity effect：`stop_workspace_transition()` 与自然完成路径都必须恢复 opacity、解绑 page 的 `QGraphicsOpacityEffect` 并安全处理已回收 Qt 对象；快速切换不得留下上一页 effect。低动效、暂停、隐藏、最小化和关闭必须回到静态页面，不得改变 Tab index、route、focus/accessibility、业务状态、共享 MotionController 时钟或新增 timer。
- UI-1.93 的 capability card 标题不得设置 `role="status"`，避免标题被辅助技术解释为状态；`QLabel#extensionCapabilityState` 必须保留 `role="status"` 和 `state="contract_only|attach_only"`，并继续使用既有三主题 QSS selector。不得改变 DTO、卡片数量/布局、可访问文案、只读边界或引入新的业务状态、动作、timer、palette、依赖或 OTA/debug adapter。
- UI-1.94 的 `UartTimingSummarySurface` 必须是只读 presentation projection，只消费 UART 波特率/数据位/校验/停止位/流控 combo 的当前值；必须随手动选择和既有 ConnectionPreset combo signal 更新，显示摘要和 accessible description，但不得接受手输波特率、写入 ViewModel/TransportConfig、改变连接 gate、协议 timing 计算、Tab 顺序或新增 timer/业务状态。摘要必须位于 UART panel 内，复用 semantic token，三主题不能回落系统白色 palette；无效暂态必须静态显示“参数待选择”。
- UI-1.95 的 `build_embedded_station_overview()` 只能消费既有 immutable capability catalog，并派生能力槽位数、已激活后端数、当前动作和接入前置条件；在当前 `contract_only`/`attach_only` catalog 下必须明确显示“只读规划层”和 `0` 个已激活后端。不得新增 active 状态源、后端探测、动作按钮、设备连接、密钥/SDK/socket/pyserial/crypto 依赖、timer 或 vendor 命令；overview 必须位于扩展页内，复用 `QFrame[role="stationBand"]` 与 semantic token，三主题不能出现系统白色回退，并保留 accessible name/description。
- ARCH-7d / UI-1.96 要求 `application/extension_station.py:ExtensionStationSummary` 作为扩展站摘要唯一 owner：DTO 必须 frozen、bounded、`read_only=True`，并由 `extension_station_summary()` 从 capability catalog 派生。`presentation/embedded_station_overview.py` 只能消费该 DTO 和 semantic theme，不得再次解释 `contract_only`/`attach_only`、探测后端、创建动作/timer 或导入 OTA/debug adapter；未来新增能力状态只能先扩展 application contract 与验证，再由 UI 被动渲染。
- UI-1.135 的 `ExtensionStationGroupSummary` 必须保持 frozen、bounded、key 唯一，并由 `extension_station_summary()` 唯一派生；有分组摘要时各组 capability/active 计数之和必须分别等于总数。`embedded_station_overview.py` 只能消费分组 DTO，使用六列 metrics grid 首屏展示 OTA 传输、OTA 安全和调试输出计数，超过六项时才自动换行；不得复制 capability state policy、创建动作/timer、探测后端或引入 OTA/debug adapter；当前真实后端仍必须显示为 0/未激活，三主题和 980/1180 页面不得横向溢出。
- UI-1.136 的 `WorkspaceScrollHint` 必须是 presentation-only projection，只观察当前 `settingsScroll` 的 vertical scrollbar range/value；`workspace.py` 负责一次性组装，`workspace_runtime.on_workspace_tab_changed()` 负责 Tab 绑定和旧 scrollbar signal 解绑。提示只能显示 bounded 的 top/middle/bottom/complete 文案，必须提供动态 AccessibleDescription/tooltip、NoFocus 和鼠标透明；不得创建 timer、第二套滚动策略、导航/业务状态、DTO、设备 I/O 或改变 `scroll_page()` 的横向关闭策略。颜色必须由 base/variant semantic token 对称提供，980/1180、三主题和无横向滚动门禁保持。
- ARCH-6q 的 `WorkspaceShellBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有 workspace shell、Tab、TabBar、route、scroll hint 和 focus button 的 Qt 引用；`workspace.py` 是唯一组装 owner，其他 owner 必须通过 `workspace_bindings_for()` 消费，不得重新引入 `_workspace_tabs`、`_workspace_route`、`_workspace_focus_button`、`_workspace_scroll_hint`、`_workspace_tab_bar` 或 `_workspace_shell` 动态 facade。bundle 不得携带 ViewModel/application/domain 状态、业务 callback、timer、transport 或导航策略；启动早期缺失必须安全返回，既有 Tab/焦点/动效/滚动语义和 980/1180 无横向溢出保持。
- UI-1.137 的 `WorkspaceContextLabel` 必须是 route strip 内的只读、NoFocus、鼠标透明 presentation projection，只消费 `QTabWidget` 的 bounded current index，并将 index 0/1/2/3 映射为链路配置/解析与遥测/命令管理/能力预览。`workspace.py` 是唯一组装 owner，`workspace_runtime.py` 只能在既有 Tab change 边界同步；不得读取 ViewModel、Session、transport、OTA/debug DTO，不得创建 timer、导航动作、第二套状态源或设备 I/O。state、文本、AccessibleDescription、tooltip 必须一次性同步；无效 index 安全回退到首项。base/variant QSS 必须对称复用 ThemeSpec token，宽度保持 158～218px，route strip 固定 31px；三主题、980/1180、四 Tab 不得横向溢出或出现白色回退。
- ARCH-6r 的 `UartControlBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有 UART 表单 Qt widget 引用；`connection_builder.py` 是唯一组装 owner，其他 controller 必须通过 `uart_bindings_for()` 消费，不得重新引入 `_port_combo`、`_baud_combo`、`_data_bits`、`_parity`、`_stop_bits`、`_flow_control`、UART timeout 或 UART checkbox 的跨 controller 动态 facade。bundle 不得携带 ViewModel/application/domain 状态、业务 callback、timer、transport、设备句柄或密钥；初始化早期缺失时 projection 安全返回，需要配置的路径必须显式报错。builder 内可暂留迁移期兼容字段，但不得复制 option catalog、改变 `currentData()`、UART preset、Modbus timing、连接 gate、section 显隐、Tab/focus/accessibility、三主题无白色回退或 980/1180 无横向溢出。
- ARCH-6s 的 `NetworkControlBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有 TCP Client/Server、UDP、RTT 共用网络端点 panel 的 Qt widget 引用；`connection_builder.py` 是唯一组装 owner，其他 controller 必须通过 `network_bindings_for()` 消费，不得重新引入 `_network_host`、`_network_port`、`_network_local_host`、`_network_local_port`、网络 timeout、`_udp_limit`、`_rtt_channel`、TCP Server allowlist/LAN/max-clients/peer selector 的跨 controller 动态 facade。bundle 不得携带 `server_target_explicit`、默认值标志、peer snapshot、ViewModel/application/domain 状态、callback、timer、transport 或网络探测策略；初始化早期缺失时 projection 安全返回，需要配置的路径必须显式报错。TCP/UDP/RTT/Server 的 `itemData`、preset、allowlist readiness、LAN 确认失效、peer 发送目标、默认端点、连接 gate、Tab/focus/accessibility、三主题无白色回退和 980/1180 无横向溢出必须保持。
- ARCH-6t 的 `BleControlBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有 BLE panel/title/hint、扫描/连接 timeout、name/service filter、device/cache/pair、characteristic/properties、read/notify/write-mode 的 Qt widget 引用；`connection_builder.py` 是唯一组装与 signal wiring owner，其他 controller 必须通过 `ble_bindings_for()` 消费，不得重新引入 `_ble_scan_timeout`、`_ble_name_filter`、`_ble_service_filter`、`_ble_device_combo`、`_ble_connect_timeout`、`_ble_pair`、`_ble_cached_services`、`_ble_characteristic_combo`、`_ble_read_button`、`_ble_notify_check` 或 `_ble_write_mode` 的跨 controller 动态 facade。bundle 不得携带 `_ble_notification_pending`、`_ble_notification_ref`、`_ble_notification_timer`、device/characteristic snapshot、ViewModel/application/domain 状态、callback、backend handle、密钥或扫描/写入策略；初始化早期缺失时 projection 安全返回，需要配置的路径必须显式报错。BLE 扫描/过滤、设备与特征选择、通知订阅/回滚、写模式 normalize、`itemData`、连接 gate、Tab/focus/accessibility、三主题无白色回退和 980/1180 无横向溢出必须保持。
- ARCH-6u 的 `TerminalControlBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有实时观测 band、terminal surface/viewport/empty state、display/pause、recording、data activity、send mode/input/CRLF/button/state/context、quick menu/button 和 history combo/clear button 的 Qt 引用；`bootstrap.py` 是组合根唯一组装 owner，其他 controller 必须通过 `terminal_bindings_for()` 消费，不得重新引入 `_terminal`、`_terminal_empty_state`、`_terminal_surface`、`_display_mode`、`_pause_check`、`_record_button`、`_data_activity_label`、`_send_mode`、`_send_input`、`_newline_check`、`_send_button`、`_send_state_label`、`_send_context_surface`、`_quick_menu`、`_history_combo` 或 `_clear_history_button` 的跨 controller 动态 facade。bundle 不得携带 `_preview_buffer`、`_preview_render_pending`、`_preview_render_timer`、history/quick snapshot、recording/session state、MotionController、ViewModel/application/domain 状态、callback、业务策略或传输句柄；初始化早期 projection 安全返回，发送路径必须显式报告缺失控件。实时预览、暂停显示、原始记录、文本/Hex、CRLF、Ctrl+Enter、快捷命令、历史加载、发送 gate、Tab/focus/accessibility、三主题无白色回退和 980/1180 无横向溢出必须保持。
- ARCH-6w 的 `CommandBatchControlBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有命令管理页的 batch combo、new/edit/delete/run/stop 按钮、status rail、results table 和 empty-state Qt 引用；`bootstrap.py` 在 `workspace.py` 完成命令页 builder 后唯一组装，其他 controller 必须通过 `command_batch_bindings_for()` 消费，不得重新引入 `_command_batch_combo`、`_new_batch_button`、`_edit_batch_button`、`_delete_batch_button`、`_run_batch_button`、`_stop_batch_button`、`_command_batch_status`、`_command_batch_results` 或 `_command_batch_empty` 的跨 controller 动态 facade。bundle 不得携带 `_command_batches`、`_command_batch_snapshot`、ViewModel/application/domain 状态、batch catalog/snapshot、callback、timer、transport 或执行策略；初始化早期 projection 安全返回。`terminal.py` 可在构建阶段保留兼容装配字段，但命令、连接、选择、composition 和 lifecycle 新代码必须走 typed accessor；batch 选择、编辑、删除、执行/停止、空态、结果表、Tab/focus/accessibility、共享 MotionController、三主题无白色回退和 980/1180 无横向溢出必须保持。
- ARCH-6x / UI-1.140 的 `ConnectionShellBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有连接 control band、transport mode surface、transport/preset combo、preset context、保存/删除按钮、connection status rail 和 connect button 的 Qt 引用；`connection_builder.py` 在 refresh preset combo 前唯一组装，其他 controller 必须通过 `connection_shell_bindings_for()` 消费，不得重新引入 `_transport_combo`、`_connection_preset`、`_connection_hint`、`_transport_mode_surface`、`_connection_control_band`、`_connection_status_rail`、`_save_connection_preset_button`、`_delete_connection_preset_button` 或 `_connect_button` 的跨 controller 动态 facade。bundle 不得携带 preset catalog/store、session/ViewModel 状态、callbacks、timer、transport handle、peer/device snapshot 或连接策略；初始化早期 projection 安全返回，需要执行连接/保存/删除的路径必须显式报错。构建器可保留装配阶段字段，但 connection/runtime/preset/commands/composition/derived/protocol/replay/scope/lifecycle 新代码必须走 typed accessor；transport `itemData`、preset signal、hint/accessibility、连接 gate、TCP/BLE/RTT/UART 语义、Tab/focus、共享 120Hz 动效、三主题无白色回退和 980/1180 无横向溢出必须保持。
- ARCH-6y / UI-1.141 的 `HeaderChromeBindings` 必须是 frozen/slots 的 presentation wiring bundle，只持有 status cluster/context/source/state label/indicator、motion controls/checks、theme controls/combo/swatch、brand mark 和 signal field 的 Qt 引用；`workspace.py` 在首次 `on_theme_changed()` 前唯一组装，lifecycle 与 composition 必须通过 `header_chrome_bindings_for()` 消费，不得重新引入 `_context_label`、`_source_badge`、`_state_indicator`、`_state_label`、`_status_cluster`、`_motion_check`、`_motion_pause_check`、`_theme_combo`、`_theme_palette_swatch`、`_brand_mark` 或 `_signal_field` 的跨 controller 动态 facade。bundle 不得携带 `PresentationPreferences`、MotionController、theme policy、app root、error/footer、ViewModel/application/domain 状态、callbacks、timer 或业务动作；初始化早期主题回调必须安全返回。`app_root`、error bar、status footer 继续由 bootstrap/terminal owner 管理；header 状态文本、主题切换、低动效/暂停、shared 120Hz fan-out、关闭静态回退、Tab/focus/accessibility、三主题无白色回退和 980/1180 无横向溢出必须保持。
- ARCH-6z / UI-1.142 的自适应工作区必须由 `workspace_runtime.on_workspace_tab_changed()` 作为唯一路由同步 owner：Tab index 0（“链路 / 连接”）是默认总览，index 1/2/3（协议 / 遥测、命令管理、扩展 / 工具站）必须复用既有 `set_workspace_focus_mode()` 自动进入配置 focus；不得在 runtime 直接复制 `setVisible()`、`maximumHeight`、QPropertyAnimation 或新建 splitter/layout policy。focus transition 必须继续使用既有 220ms 动画、reduced-motion/隐藏/最小化/关闭静态回退和手动 focus button；用户手动切换只覆盖当前路由，下一次 Tab 路由变化可重新同步。focus 只隐藏 presentation 的实时观测、terminal、send 三块，下方 session/接收/记录/发送 worker、连接 gate、Tab index、焦点顺序、accessible 文案、主题 token 和业务 callbacks 不得改变；不得新增 timer、事件总线、ViewModel/domain 状态或 OTA/debug 依赖。980×720 与 1240×820、三主题、四页 horizontal maximum=0、无重叠和 shared MotionController 120Hz/8-9ms 不变量必须保持。
- ARCH-70 / UI-1.143 的 `open_connection_setup()` 必须是 presentation-only 的显式 onboarding 入口，由 `bootstrap.py` 唯一把 `TerminalEmptyState.connection_requested` 接入；它只能选择 bounded index 0、复用 `set_workspace_focus_mode(True)` 并将焦点回到真实 `QTabWidget`，不得创建连接、读取/修改 ViewModel/session 状态、绕过连接 gate、写入 transport config、复制 terminal empty-state 文案或新增业务 callback。默认启动 index 0 仍为总览；CTA 触发后 connection focus 必须收起实时观测/terminal/send 三块但保持后台 worker、主题、reduced-motion、Tab/accessibility、shared 120Hz lifecycle 不变。重复点击、closing、缺失 workspace bundle 和无效 tab 必须安全返回；不得新增 timer、事件总线、splitter、外部依赖或 OTA/debug coupling。
- ARCH-71 / UI-1.144 的 workspace 导航必须保持单一主过渡：当 `on_workspace_tab_changed()` 已通过既有 `set_workspace_focus_mode()` 启动有效 focus geometry transition 时，`animate_workspace_transition()` 必须清理旧 page opacity transition 并跳过新的 180ms page fade；同一 focus mode 内的 Tab 切换仍可保留 page fade。进入 focus 时下方 live/terminal/send surface 必须先 `setVisible(False)` 并清零 maximum-height，不得把内部控件压缩到过渡帧；返回总览可沿用既有 0→bounded target 的 reveal。不得复制 `QPropertyAnimation` policy、增加 timer/event bus/业务状态或改变 `MotionController` 120Hz/8-9ms cadence；所有路径必须保持 reduced-motion、隐藏/最小化/关闭静态回退、effect/group 生命周期清理、Tab/accessibility、主题和 980/1240 响应式边界。
- ARCH-72 / UI-1.145 的 header 长状态文案必须由 `presentation/bounded_text_label.py:BoundedTextLabel` 统一承载：可见文本只能按当前 label 宽度使用右侧省略，完整值必须保留在 `full_text`、tooltip 和 accessible description 中；resize、字体或 style 变化后必须重新计算，不能把一次性省略结果当作业务状态。该组件只能是 presentation-only QLabel owner，不得读取 ViewModel/domain、复制连接/来源判断、创建 timer/animation/event bus 或改变 `HeaderChromeBindings` 的 widget wiring；`workspace.py` 负责三个 header label 的组装，lifecycle 继续沿用既有 `setText()` 更新路径。size hint 必须有界且 minimum width 可收缩，status cluster 在 980/1240 下不得撑开、与 motion/theme controls 重叠或把长文案改为隐藏；三主题、reduced-motion、focus、accessibility、120Hz/8-9ms、无白色回退和既有连接状态语义必须保持。
- ARCH-73 / UI-1.146 的主题切换必须保持 `theme_transition.py` 单一 owner 和既有 root fade/cleanup contract；`ThemeTransitionSurface` 的 sweep 只能在 bounded 的顶部 shell 带内运行（当前最大高度 220px），不得覆盖 terminal、send 或下方数据 surface 的完整工作区。不得新增 timer、MotionController、业务状态、事件总线、主题状态或外部依赖；`stop_theme_transition()`、reduced-motion、暂停、隐藏、最小化、关闭、快速切换、三主题、980/1240、focus/page transition、accessibility 和 120Hz/8-9ms 不变量必须保持。该 sweep 只是装饰反馈，不能阻挡鼠标/键盘，也不能改变 palette 生效时机或任何连接/协议/记录语义。
- ARCH-74 / UI-1.147 必须让主题过渡显式接收 `target=appRoot` 与 `sweep_host=header chrome`；host 必须是 target 的可见子树，越界时安全返回，禁止把 terminal/send/workspace data surface 作为 sweep host。root opacity fade 与 host-local `ThemeTransitionSurface` 仍由同一个 theme-transition owner 统一清理；窗口 resize 必须先停止一次性 sweep 再交给 Qt 重排，不能用过期 geometry 继续绘制。不得新增 timer、MotionController、业务状态、事件总线、主题状态或外部依赖；三主题、980×720/1240×820、快速双切、reduced-motion、暂停、隐藏、最小化、关闭、root opacity 恢复、queued cleanup、accessibility 和 120Hz/8-9ms 不变量必须保持。
- ARCH-75 / UI-1.148 的 focus 几何过渡必须由 `workspace_focus_transition.py` 唯一拥有：进入/退出必须基于当前实际 widget 高度和 root layout 重算后的目标高度做 `maximumHeight` 动画，不得把 `_MAX_HEIGHT`/其它哨兵高度作为用户可见的动画区间，避免表单在中间帧突跳或挤压。进入 focus 仍先隐藏 live/terminal/send，退出仍恢复既有总览；`_apply_static_layout()` 必须清理临时 min/max/visible 约束，`lifecycle.resizeEvent()` 必须在 Qt 基类重排前停止 focus transition。不得新增 timer、MotionController、splitter、事件总线、业务状态、滚动页策略或 OTA/debug coupling；980×720/1240×820、三主题、四 Tab、快速反转、resize 中断、reduced-motion、暂停、隐藏、最小化、关闭、accessibility、无横向溢出和 120Hz/8-9ms 不变量必须保持。
- ARCH-76 / UI-1.149 的 connection onboarding 必须保持 presentation-only 且一次性：仅在完整 terminal bindings 完成后、初始 `SessionState.CLOSED` 时启动既有 focus；`workspace_runtime.py` 是 `_connection_onboarding` 的 owner，bootstrap 只负责初始化/触发，terminal runtime 只在首次 `OPEN` 通知 leave。leave 只有当前处于 connection Tab 才返回总览；协议/命令/扩展 Tab 的 focus、历史回放、ERROR/失败重试、连接 gate、ViewModel/domain/session 状态不得被改变。手动“返回总览”必须清除标志，show/hide/minimize/resize/close/reduced-motion 必须复用既有静态收敛，不得新增 shell min-height、splitter、timer、MotionController、业务状态、事件总线、滚动页策略或 OTA/debug coupling；980×720/1240×820、三主题、四 Tab、无重叠、无横向溢出、accessibility 与 120Hz/8-9ms 不变量必须保持。
- ARCH-77 / UI-1.150 supersedes the earlier 8/9ms wording for the shared motion scheduler: `presentation/widgets.py:MotionController` remains the only presentation clock and uses one `PreciseTimer` with an 8ms periodic scheduler slot as an approximately-120Hz target. The phase must continue to use monotonic elapsed time; this is a scheduler target, not a claim of exact compositor/display FPS. No renderer may add a timer, thread, event bus, business state, device I/O, or OTA/debug dependency. Reduced-motion, explicit pause, hidden/minimized, close, queued timeout, and resume must stop or statically settle all surfaces without a catch-up burst.
- ARCH-77 / UI-1.150 的协议页布局必须保持 presentation-only：`controllers/protocol.py` 可以把协议、组件、Dataset、Curve、Replay 控件整理为语义 field/action rows，`presentation/form_fields.py` 只能提供无状态的标签/控件组合 helper。必须保持 `ProtocolPanelWidgets` 字段、所有 typed `itemData()`、callback signal 顺序、keyboard/accessibility 文案、外层 `QScrollArea`、vertical scroll hint、四个 surface owner 与 application/domain contract；不得新增嵌套滚动容器、固定内容宽度/高度、业务状态、协议策略、设备 I/O 或 OTA/debug coupling。980×720/1240×820、三主题下 horizontal maximum 必须为 0，动态 enable/disable、焦点顺序和无重叠必须保持。
- ARCH-6v / UI-1.138 的 `MotionController` 必须保持单一 presentation 时钟，目标 cadence 为 120Hz，使用 PreciseTimer 与 8/9ms 交替 slot，并以 elapsed time 推进 phase；不得为任一装饰控件新增 QTimer、线程、事件总线或业务状态。`lifecycle.on_motion_frame()` 只能向 visible-to-window 的既有 surface fan-out，确定性相位错峰只能改变装饰绘制，不得改变状态、文案、Tab、accessibility、连接、发送、记录或回放语义；reduced-motion、暂停、隐藏、最小化和关闭必须停止/静态回退。`terminal.py` 的实时观测/发送/历史布局必须有 bounded 控件宽度、明确行高和间距，默认窗口 1240×820、最小窗口高度 720；980/1180 组合根不得产生控件重叠、横向滚动或系统白色回退。
- UI-1.97 的 UART 波特率必须保持 25 个常用预设且不可编辑；数据位、校验、停止位、流控必须使用有界下拉并显示中文用户文案，同时提供 tooltip 与 accessible description。label 只能改变 presentation 文本，`currentData()` 的 int/enum/string contract、`UartTransportConfig`、快速配置、连接 gate、Tab 顺序、主题 token 和三主题非白色回退不得改变；不得重新引入手输波特率或第二套 UART option catalog。
- UI-1.98 的网络/BLE/RTT/TCP Server 控件必须继续使用既有 bounded range、typed `itemData` 和 runtime controller；RTT 通道与 BLE 写入模式必须保持不可编辑并使用中文 label，TCP Server allowlist/LAN/最大客户端/发送目标、BLE 扫描/过滤/设备/缓存/配对/特征/通知和网络 host/port/timeout/UDP 上限必须提供 tooltip 或 accessible description。零值超时只能把 presentation 特殊文案显示为“未设置”，不得改变 zero-as-unbounded 语义；不得新增网络探测、vendor 工具启动、连接动作、状态源、timer、密钥/SDK/socket/crypto 依赖或第二套 option catalog。
- UI-1.99 的协议 preset、帧格式、校验、最大帧、分隔符、长度字节、字节序、曲线序列和回放速度必须保持 bounded、不可手输并提供中文 label/tooltip/accessibility；只允许改变 presentation 文本，`ProtocolPreset`/`FramingKind`/`ChecksumKind`/Dataset field/CurveSnapshot 与所有 `itemData` contract、解析 gate、signal 顺序、状态源、snapshot 生命周期和既有动效不得改变。Dataset 配置动态刷新必须继续显示“数值序列/样本”等中文 fallback；不得新增协议策略、timer、线程、设备 I/O、外部依赖或把 domain/application policy 复制进 UI。
- UI-1.100 的 `PipelineSurfaceLabel` 必须只消费既有 `state/source` 动态属性、`ThemeSpec` 和共享 MotionController frame；六种状态 `idle/active/transition/draft/blocked/history` 必须有界映射到节点进度与 semantic token。只有 active/transition/draft 可以绘制 pulse；idle/history/blocked、reduced-motion、隐藏/最小化和关闭必须静态回退，blocked 只能显示 presentation 阻塞标记，不得伪造业务错误或进度。非法属性必须 fail-open 到 idle/live；不得新增 timer、线程、事件总线、状态源、设备 I/O、外部资源或白色 literal，也不得改变 pipeline 文本、accessible description、QSS properties、Tab 顺序、application/domain contract 或 OTA/debug 边界。
- UI-1.101 的 `SignalFieldWidget` 只能在既有 148×34 presentation surface 内绘制资源无关的星芒、彗尾和轨道光点；颜色必须来自 `ThemeSpec`，动态位置/透明度只能消费共享 MotionController frame。必须保持 NoFocus、鼠标透明、空 accessibility、`set_frame()/stop()` API 和既有 fan-out；reduced-motion、暂停、隐藏、最小化和关闭必须静态回退。不得新增控件级 QTimer、业务状态、图片/字体/网络资源、跨层 callback、白色 literal 或 OTA/debug 依赖，也不得改变 header 布局、Tab 顺序和用户可读状态文本。
- UI-1.102 的 `QFrame#extensionStationOverview`、`QFrame#extensionCapabilityCard` 和 `QLabel#extensionStationOverviewValue` 必须由 base stylesheet 与 theme variant shell 对称覆盖；`contract_only` 只能使用 info/blue 语义，`attach_only` 只能使用 history/purple 语义。只允许投影既有 `state`/DTO，禁止复制 capability policy、增加 token、状态源、动作、timer、资源、vendor 依赖或白色 literal；7 张卡片、接入概览的只读/accessibility 文案和 OTA/debug contract 边界必须保持。
- UI-1.103 的 `QFrame#workspaceShell` 与 `QFrame#workspaceRouteStrip` 必须由 base stylesheet 和 `theme_variant_controls.py` 对称覆盖，且只使用已有 `ThemeSpec` semantic token；不得写入白色 literal、第二套 palette 或原生默认透明回退。`workspace.py` 仍是组装 owner，真实 `QTabWidget`、route beacon 的固定尺寸/NoFocus/鼠标透明/空 accessibility、Tab index、焦点顺序、currentChanged 和共享 MotionController 生命周期必须保持；不得把 route 装饰条变成导航状态源、业务动作、常驻 timer 或 OTA/debug 依赖，980/1180 布局不得出现横向滚动。
- UI-1.104 的 `QStatusBar::item` 与 `QStatusBar QLabel` 必须由 base/variant controls theme 对称覆盖为透明背景、零边框和既有 muted token；不得改变 native status text、`StatusFooterSurface` 的 116×18 几何、state/fault/activity/frame/stop API、accessibility 或 lifecycle fan-out。不得新增 footer 状态源、timer、palette、白色 literal、业务动作或 OTA/debug 依赖，底部 chrome 在三主题与 980/1180 布局中不得出现系统白色分隔线。
- UI-1.105 的 `QComboBox#transportCombo` 必须由 `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 对称覆盖，复用既有 `info_surface/info_border/accent_blue/focus/disabled_*` token；六种 `TransportKind`、itemData、不可编辑、accessible name、signal wiring、TransportModeSurface glyph、连接 gate、Tab 顺序和 runtime 行为必须保持。不得把 selector 样式、palette、第二套 transport 状态源、timer、设备 I/O、OTA/debug 动作或白色 literal 带入 builder/controller；enabled/hover/focus/disabled 与 980/1180 布局不得回落系统默认白色外观。
- UI-1.106 的 `QComboBox#themePicker` 必须由 `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 对称覆盖，复用既有 `history_surface/history_border/accent_purple/focus/disabled_*` token；三个主题 key/icon、itemData、不可编辑、accessible name、`currentIndexChanged`、`on_theme_changed()`、一次性 transition、palette swatch、Tab 顺序和业务状态必须保持。不得把主题选择器变成第二套主题状态源、常驻 timer、设备/OTA/debug 动作或白色 literal；popup、enabled/hover/focus/disabled 与 980/1180 布局不得回落系统默认白色外观。
- UI-1.107 的 `ConnectionStatusRail` 彗尾、外环和中心光点必须只消费既有 `_phase`、`_animated`、`_state`、节点位置、`ThemeSpec` 与共享 lifecycle frame；颜色必须使用状态语义 token，不得新增控件级 timer、业务状态、公开 API、资源、线程、I/O、palette 或 transport/OTA/debug 依赖。`opening/open/closing` 才允许沿用既有共享帧动态绘制，`discovered/closed/error`、停止、低动效、隐藏、最小化和关闭必须静态稳定；必须保持 NoFocus、鼠标透明、空 accessibility、六状态文案和连接行为。
- UI-1.108 的用户可见 `QComboBox` 必须显式声明 editable 语义、accessible name、tooltip 和 accessible description；transport、终端显示/发送、发送历史、批量命令及批量编辑器选择器必须 `setEditable(False)`，UART 端口是唯一允许 `setEditable(True)` 的 combo。不得改变 itemData、currentIndexChanged/editTextChanged、连接 gate、批量执行、焦点顺序、主题 QSS 或引入第二套 option catalog；提示文案不得承诺自动连接、发送或执行，不能把 Qt 文案下沉到 application/domain。
- UI-1.109 的清除错误、清空终端预览、清除发送历史、快捷命令菜单/保存、保存/删除自定义连接和 UART 读写超时控件必须在原 owner 内提供准确 tooltip 与 accessible description；文案必须说明既有副作用、不会影响的连接/发送/原始记录范围及秒单位/连接后生效语义。不得改变 clicked/runtime callback、超时 bounded range、默认值、DTO、状态源、signal、timer、线程、设备 I/O、依赖或 OTA/debug 边界；不得引入全局 affordance registry、重复动作状态或 application/domain Qt 文案。
- `workspace.py` 必须把真实 `QTabWidget` 与装饰性 `WorkspaceRouteSurface` 放入同一个 presentation-owned shell；route 只能通过
  `workspace_runtime.on_workspace_tab_changed()` 接收 bounded Tab index，并通过 lifecycle 的共享 frame/stop fan-out 动画。route
  必须 `NoFocus`、鼠标透明、空 accessibility，不得改变 Tab 顺序或替代可读 Tab 文案。固定宽度装饰不能依赖
  `QTabWidget.setCornerWidget()`，必须在 layout-owned strip 内右对齐，且 980/1180 下不得引入横向滚动。
- `presentation/workspace_tab_icons.py` 只能提供四个固定语义 glyph（链路、协议、命令、扩展）的主题化 `QIcon`；必须同时配置
  Normal/Selected/Disabled 三态并使用 `ThemeSpec` 语义色，不能用 emoji、文字或外部图片资源替代。原生 Tab 的文案、index、
  `currentChanged`、焦点、键盘导航和 accessibility 契约必须保持；`tabs=None` 或少于三页时必须安全返回/有界刷新。`workspace.py`
  只负责组装，`lifecycle.py` 只负责主题切换刷新；glyph owner 不得读取业务状态、创建常驻 timer、改变 Tab 顺序或引入第二套
  导航模型，且必须保留静态图标回退。
- `presentation/brand_mark_surface.py` 只能提供固定 38×38 的 Header 装饰徽记；必须使用现有 `ThemeSpec` 语义色和
  `QPainter` 几何，不得加入角色/IP、GIF、字体、SVG/PNG、网络或第三方资源。它必须 `NoFocus`、鼠标透明、空
  accessibility，不得读取连接/协议/传输状态或拥有自己的 timer；`workspace.py` 只负责放置，`lifecycle.py` 只能通过既有
  `_motion_surfaces()` 传递共享 frame/stop。低动效、暂停、隐藏、最小化和关闭必须静态回退，且不得改变 wordmark、状态胶囊、
  主题选择器或 Header 的 Tab 顺序。
- `build_app_header()` 必须保持“品牌层 + 控制层”的两层 presentation layout：品牌层承载 brand mark/wordmark/subtitle/
  signal field，控制层承载 status cluster/motion controls/theme controls。980px 最小窗口和 1180px 宽屏均不得依赖横向滚动、
  裁切或隐藏可操作的连接/动效/主题控件；只允许改变布局几何，不得改变现有 window facade、signal、tab order、业务状态或
  accessibility 语义。响应式布局不得新增独立 timer 或跨 controller 的隐式状态。
- `TerminalEmptyState` 的背景/glyph 仍是 presentation-only renderer，但允许一个明确的、可访问的
  `connection_requested` 无参数 signal 和 `primaryButton` CTA。CTA 只能表达打开链路连接 Tab 的意图，不能自动连接、
  读取 transport/session 配置或修改 ViewModel；它只在 idle/waiting/transition 可见，paused/history 必须隐藏。
  `bootstrap.py` 只能用显式 signal wiring 接入 `workspace_runtime.select_workspace_tab()`；workspace runtime 负责关闭门、
  Tab 索引边界、真实 Tab 切换和焦点恢复。不得让 terminal surface 持有 `QTabWidget` 或引入第二套导航 owner。CTA 底部
  signal rail 只能由 `TerminalEmptyState.set_frame/stop` 转发共享 `MotionController` frame，使用 `ThemeSpec` accent，
  不得创建 QTimer、读取业务状态或在 hidden/paused/history 状态绘制可见动效。
- `presentation/widgets.py:MotionController` 是装饰性 shell 的唯一共享低频时钟；`ambient_active` 只能由生命周期 owner 在可见窗口
  开启，隐藏/最小化必须依靠 `suspended` 停表，关闭必须清理。它不得携带业务状态或替代 reduced-motion/显式暂停策略；新增 renderer
  只能消费共享 frame 并保留静态回退，禁止新增控件级常驻 `QTimer`。当前 96ms 间隔约为 10.4Hz，必须用离屏帧数和同条件审计证明
  动效确实存在且隐藏/低动效/暂停/关闭可停止。
- 已存在 canonical owner function 的 terminal runtime helper 必须优先由 `controllers/terminal_runtime.py` 同模块直接调用；
  `MainWindow` 不为 `_current_entry`、`_render` 这类仅供 owner module 自身使用的函数保留转发 facade。该规则不扩展到
  仍承担 signal/cross-workspace/lifecycle 接线的 wrapper，后续切片必须逐项评审并保持可回滚。
- presentation 工作区按稳定变化边界放入 `presentation/controllers/`：`bootstrap`、`composition`、`workspace`、`connection_*`、`connection`、`terminal_*`、`protocol*`、`derived_data`、`replay`、`commands`、`ble`、`lifecycle`、`status_surfaces` 分别高内聚；跨 controller 只能通过 `presentation/contracts.py` 的最小类型化回调/DTO 和 ViewModel signals；
- `presentation/preferences.py` 只暴露 frozen `PresentationPreferences` 与 `PreferenceStore` port；`QSettingsPreferenceStore` 是唯一 Qt 持久化实现。schema、主题 key 和布尔值必须显式校验，坏值逐字段回退安全默认；lifecycle 不得直接访问 QSettings，保存失败不能破坏当前 UI 状态；
- 协议、连接和 BLE 的长选项必须通过 presentation 层 bounded-combo 策略限制 size hint；动态说明/Profile/Dataset 文本必须允许换行并保留 tooltip/AccessibleDescription，不得把最长文案传播为 scroll content 的硬性宽度；
- 980px 宿主宽度是当前 offscreen 响应式门槛：UART、TCP Client/Server、UDP、BLE GATT、RTT 页面不得出现横向滚动；网络/RTT 长提示必须使用可收缩的 `QSizePolicy.Ignored` 和非零可读下限；
- 终端空态必须是 presentation-only 的上下文投影：只能从已有 preview buffer、SessionState、历史来源和 preview paused
  事实派生，不得新增业务字段、传输判断、网络请求或第二套状态源；有 RX 数据时必须让位给真实终端内容。
- 实时观测带与发送带采用两行可收缩网格时，必须保留既有 widget 名称、signal 连接、发送 gate、键盘逻辑顺序和
  `MotionController` 生命周期；长状态文案必须有可读最小宽度、AccessibleDescription/tooltip，不能只依赖 tooltip，
  也不能用新的常驻 timer 或外部视觉资源填充空态。
- 协议/遥测页必须按稳定责任边界组织为协议配置、组件遥测、Dataset/曲线、历史回放四个 presentation surface；
  surface 只能是静态 Qt 容器，不得复制 `state/source`、控制业务 enable/visible、接入 lifecycle/timer 或跨层 callback。
  组件、Dataset、曲线和回放工具条必须使用明确的 Grid 行列/stretch，动态标签和 combo 要有 bounded 尺寸，980px 下不得
  发生控件重叠或横向滚动；`ProtocolPanelWidgets`、callback、widget ref 和键盘顺序属于稳定契约。
- `presentation/pipeline_surface.py` 的 `PipelineSurfaceLabel` 只能是现有 `pipelineSummary` QLabel 的装饰性子类：
  必须保留文字、AccessibleDescription、`objectName`、`source/state` 动态属性和 QSS 命中；只消费共享
  `MotionController` 帧绘制主题化轨道，不得读取/复制业务状态，不得创建 QTimer、持有 controller 或引入外部资源。
  reduced-motion、暂停、隐藏、最小化、关闭必须通过现有 lifecycle 入口停止并保留静态文字。
- 顶栏 `StatusIndicator` 只能把已有 `SessionState` presentation 投影和共享 `MotionController` 帧转换为 22×22 的
  几何状态标记；`discovered/opening/open/closing/closed/error` 必须各有静态可辨识回退，动画只读 `_phase/_animated`，
  不得新增 timer、业务状态、生命周期判断或外部资源；Qt `drawArc` 的弧度必须显式换算为 1/16 度单位并限制在控件边界内。
- `presentation/command_batch_surface.py:CommandBatchSurfaceLabel` 只能消费 `commands.py` 从已有
  `CommandBatchSnapshot` 投影出的 `CommandBatchSurfaceProjection`；它可以绘制最多 32 个步骤的待执行、当前、已提交和
  失败轨道，但不得读取 ViewModel/domain、复制业务进度、创建 timer 或改变 `QLabel#commandBatchStatus` 的文字/无障碍描述。
- `presentation/replay_activity_surface.py:ReplayActivityLabel` 只能消费 `replay.py` 投影出的 `ReplayActivityProjection`；
  回放没有权威总记录数时只能绘制活动轨迹，不能伪造百分比或总进度。两个自绘标签都必须只暴露 projection/frame/stop
  边界，由 lifecycle 统一接入共享 `MotionController`，并在低动效、暂停、隐藏、最小化和关闭时静态回退。
- `presentation/analysis_status_surface.py:AnalysisStatusLabel` 只能复用既有 `QLabel` 的文字、无障碍描述和
  `source/state` 动态属性；它只从 presentation property 读取状态，绘制底部装饰信号轨道，并暴露
  `set_frame()`/`stop()` 给共享 `MotionController`。不得保存 domain/application snapshot、复制业务状态、创建
  `QTimer`、改变 objectName/QSS 属性或遮挡换行文字；四个 Protocol/Component/Dataset/Curve 状态条由
  `lifecycle.py` 统一负责 frame/stop，未知状态必须静态中性回退，replay 专用轨迹不与其泛化耦合。
- `presentation/theme_palette_surface.py:ThemePaletteSwatch` 只能读取父链 `ThemeSpec` 语义 token 绘制 62×22
  装饰色点阵；它必须鼠标透明、不可聚焦、空 accessibility，不进入键盘 Tab 顺序，不创建 `QTimer` 或主题信号。
  主题刷新复用 `apply_theme()` 对子控件的 update，动效复用共享 `MotionController`，由 `lifecycle.py` 统一 frame/stop；
  固定尺寸接入后必须通过 980/1180 头部几何、三主题、HIDPI 逻辑坐标和隐藏/最小化/关闭静态回退检查。
- `presentation/theme_picker_icons.py:refresh_theme_picker_icons()` 只能把 `ThemeSpec` 的 surface/accent/disabled token
  渲染成原生 `QComboBox` item icon；不得读 ViewModel/SessionState、创建 timer、写主题偏好、替换 label/`UserRole`/tooltip、
  改变 currentIndexChanged、焦点或 Tab 顺序。图标必须 resource-free、三主题非空、无白色背景回退；workspace 只在 combo 条目
  组装后调用一次，不能为同一图标新增 lifecycle 状态或第二套主题事实源。
- `presentation/connection_status_surface.py:ConnectionStatusRail` 只能绘制连接带底行的端点→传输→会话→数据四节点
  装饰路径；状态必须由 `lifecycle.update_state_badge()` 显式传入既有 `SessionState`，不得读取 ViewModel/domain、复制 RX
  计数或暗示真实数据进度。它只能暴露 `set_state()`、`set_frame()`、`stop()`，复用共享 `MotionController`，不创建 `QTimer`、
  不持有控制器、不进入 Tab/无障碍顺序；必须使用 `ThemeSpec` 语义 token、逻辑坐标和独立底行，并通过六状态、三主题、
  980/1180、HIDPI、隐藏/最小化/关闭与静态停止回退检查。
- `presentation/terminal_surface.py:TerminalEmptyState` 的观测背景只能是父层 presentation 装饰：`paintEvent()` 必须先让
  QFrame 完成基础绘制，再在 `contentsRect()` 裁剪区域内用语义 token 绘制低对比度网格、角标和共享 frame scan line，子卡片、
  glyph、标题、提示和 AccessibleDescription 必须保持原有契约。不得将 RX 字节、时间、吞吐、连接进度或业务统计画进背景；
  CTA 是单独的 navigation intent 控件，不能成为背景 painter 的业务入口；除该 CTA 外不得新增控件、QTimer、状态源或
  lifecycle 接线，`set_frame()/stop()` 必须在 reduced-motion、暂停、隐藏、最小化、关闭时静态回退。
- `presentation/action_surface.py:ActionRailButton` 只能是原生 `QPushButton` 的共享装饰子类：保留原生点击、焦点、
  `primaryButton` QSS 与 Accessible 契约，只消费 `(phase, animated)` 和父链 `ThemeSpec.accent` 绘制底部 signal rail；
  不得创建 QTimer、保存业务状态、读取 ViewModel 或把视觉活动解释为传输进度。`TerminalActionButton` 兼容导出只能指向该
  shared renderer，不得复制第二份 painter 实现。
- `presentation/action_surface.py:BusyActionButton` 只能装饰已有异步 presentation action（当前为 connection opening/closing、UART discovery、BLE scan、
  原始 recording、批量停止和历史回放播放）：保留 click、焦点、disabled、QSS、AccessibleName/Description 和 controller 接线；`set_busy()` 的值必须
  来自对应既有 session/busy/recording/batch/replay projection，不能读取 ViewModel 或启动 timer。它只消费共享 `MotionController` 的
  `(phase, animated)`，busy 时绘制 activity rail，低动效/暂停/隐藏/关闭时必须保留静态 rail；不得暗示真实传输/写盘进度、替换
  按钮文字语义或新增扫描/刷新/记录业务状态。
- `presentation/status_footer_surface.py:StatusFooterSurface` 只能作为 native `QStatusBar` 的永久装饰子控件：状态栏原生
  message 保持唯一权威文案和无障碍表达；surface 只接受 lifecycle 投影的 `SessionState`、fault、RX activity 与共享
  `MotionController` frame，固定 116×18、鼠标透明、不可聚焦且不进入 Tab 顺序。它不得读取 ViewModel、复制文字、创建
  `QTimer`、承载业务状态或伪造吞吐/进度；reduced-motion、暂停、隐藏、最小化、关闭必须保留静态轨道，状态/错误清除必须
  由 lifecycle 显式投影。
- `presentation/send_input_surface.py:SendInputSurface` 只能继承原生 `QLineEdit` 做后置装饰：`QLineEdit.paintEvent()` 必须
  先完成文本、选择、光标、placeholder、焦点、剪贴板和键盘契约，surface 只接受 `blocked/waiting/ready/busy/history` 的
  既有 `send_band_state` 投影与共享 `MotionController` `(phase, animated)`。不得读取 ViewModel、复制状态文案、修改发送 gate、
  创建 `QTimer` 或把轨道解释为发送进度；焦点/ready/busy 只改变装饰色与 pulse，reduced-motion、暂停、隐藏、最小化、关闭必须
  静态回退。
- `presentation/error_surface.py:ErrorSignalSurface` 只能作为 errorBar 的 presentation-only 28×28 故障 beacon；错误文字、
  AccessibleName/Description、清除按钮和 error container 可见性仍由既有 `ErrorInfo`/lifecycle owner 负责。surface 只接受
  `set_active()`、共享 `MotionController` 的 `set_frame()`/`stop()`，必须鼠标透明、不可聚焦、空 accessibility；不得读取 ViewModel、
  复制错误文案、创建 `QTimer`、改变 clear 语义或把 pulse 作为错误数量/严重性数值。active/clear、reduced-motion、暂停、隐藏、最小化、
  关闭必须都有静态回退。
- `presentation/data_activity_surface.py:DataActivitySurface` 只能继承原生 `QLabel` 做后置装饰：必须先保留既有接收文字、
  `objectName`、QSS、AccessibleName/Description 和布局契约，再在底部 padding 绘制低对比度实时/历史 activity rail。surface 只接受
  `DataActivityProjection` 的 bounded source/最近字节/窗口字节与 lifecycle 的 `set_activity()`/共享 `set_frame()`/`stop()`；不得读取
  ViewModel、复制 RX buffer、绘制吞吐/百分比/设备进度、创建 `QTimer` 或改变暂停/接收语义。reduced-motion、暂停、隐藏、最小化和关闭
  必须静态回退，文案仍由 `terminal_runtime.py` 唯一拥有。
- `presentation/send_state_surface.py:SendStateSurface` 只能继承原生 `QLabel` 做后置装饰：必须先保留 sendState 的文字、
  `objectName`、QSS、AccessibleName/Description 和布局契约，再读取既有 `state` property（`blocked/waiting/ready/busy/history`）绘制
  四节点 rail。不得读取 ViewModel、复制发送 gate/文案、创建 `QTimer` 或改变发送/批量/历史语义；仅 `busy` 在共享 frame 可用时 pulse，
- `presentation/send_context_surface.py:SendContextSurface` 只能作为发送带内原生 `QLabel` 的 presentation-only 上下文摘要：
  `controllers/send_context.py` 只读取既有 `QComboBox` mode、原生 `QLineEdit` 文本和 CRLF checkbox，生成 immutable
  `SendContextProjection`；`project_send_form()` 必须复用 domain `MAX_COMMAND_PAYLOAD_BYTES`，空输入、坏 Hex 和超限 payload
  必须以明确文本/AccessibleDescription 表达，不能等到点击发送才首次暴露可修正的格式/容量问题。surface 不调用 send action、
  不改变 `connection.py` 的 connected/batch/history/BLE/TCP gate，不保存 payload、不读取 ViewModel、不创建 `QTimer`，只消费 projection
  与共享 `MotionController` frame/stop 绘制 byte rail。它必须鼠标透明、不可聚焦、不进入 Tab 顺序；`terminal.py` 只装配并把 mode/CRLF
  变化接入既有输入刷新回调，`connection.py` 继续是发送 gate owner，`lifecycle.py` 统一 reduced-motion、暂停、隐藏、最小化、关闭静态回退。
  默认 stylesheet 与每个 theme override 必须覆盖 `QLabel#sendContext` 的 empty/ready/invalid 三态，禁止系统 palette 形成白色回退。
- `terminal.py` 中的 `QLabel#sendShortcutHint` 只能作为发送控制带的 presentation-only 快捷键提示：固定显示既有
  `Ctrl+Enter 发送` 文案，AccessibleName/Description 与 tooltip 必须说明同一动作；它必须 `NoFocus`、不接受鼠标动作、不进入
  Tab 顺序，不注册快捷键、不调用 send action、不读取 ViewModel、不维护发送状态或 timer。发送快捷键的注册与 action owner
  保持不变；提示尺寸以 176–184 px 为边界，必须在 760/952/1152 内容宽度向量中保持文字区不裁切。默认 stylesheet 与每个
  theme override 必须覆盖 `QLabel#sendShortcutHint` 的文字、背景、边框和圆角语义色，禁止系统 palette 留下白色 fallback。
- UI-1.73 的 `controllers/terminal.py:_field_label()` 只负责实时观测工具栏“显示”字段的局部装配，必须使用
  `QLabel[role="muted"]`；`实时观测` section 必须继续使用 `role="section"`，暂停/记录/接收状态必须保留 `role="status"`。
  helper 不得读取 ViewModel/DTO、接收 display mode signal、改变 `_rerender_preview`、暂停/记录/数据活动、快捷键、焦点、Tab 顺序或
  `MotionController` 生命周期，不得创建 timer 或让系统 palette 形成白色 label fallback。
- `controllers/connection_builder.py:_field_label()` 是连接表单普通字段标签的唯一局部装配 helper：UART/网络/BLE 参数
  标签必须使用 `QLabel[role="muted"]`，以便三套主题统一控制层级和对比度；不得在字段标签里读取 ViewModel、写入配置、接收
  signal、创建 timer 或改变 Tab 顺序。section 标题必须继续使用 `role="section"`，hint/状态/快捷提示不得被批量降级为字段标签，
  不得用重复的全局 label 工厂隐藏连接页依赖，也不得让系统 palette 形成白色背景 fallback。
- `controllers/protocol.py:_field_label()` 是协议/遥测配置表单普通字段标签的局部装配 helper：预设、Framing、校验、最大帧、
  Delimiter、长度字节、字节序和过滤等字段必须使用 `QLabel[role="muted"]`；协议 section 标题必须继续使用
  `role="section"`，status/hint/dynamic label 必须保留各自语义，不能批量降级。该 helper 不得读取 ViewModel/DTO、写入协议配置、
  接收 callback、创建 timer、改变焦点/Tab 顺序/布局列或引入第二套主题状态源，默认 stylesheet 和 theme override 不得留下白色
  palette fallback。
- `presentation/protocol_config_context_surface.py:ProtocolConfigContextSurface` 只能作为协议配置面板内的
  presentation-only 摘要：`controllers/protocol_context.py` 读取既有 framing/checksum/max-frame 控件与
  `protocolStatus.property("state")`，生成 immutable `ProtocolConfigContextProjection`，不得复制
  `derived_source_supported()`、`_protocol_editor_dirty` 或 parser/apply/reset 业务判断。它只能展示草稿/已应用、
  framing、checksum、最大帧长和下一步说明，不应用配置、不清理派生状态、不读取 ViewModel、不保存 payload/配置、
  不创建 `QTimer`；`protocol_config.py` 仍是 editor/status/gate owner，`protocol.py` 只装配，
  `lifecycle.py` 必须统一共享 MotionController 的 `set_frame()`/`stop()`。surface 必须鼠标透明、不可聚焦、宽度
  有界（190–520 px），补齐 AccessibleName/Description/tooltip；默认 stylesheet 与每个 theme override 必须覆盖
  waiting/active/draft/blocked/history/idle，禁止系统 palette 或白色回退。
- `presentation/component_empty_state.py:ComponentEmptyStateSurface` 只能作为 Component 表的
  presentation-only 空态卡片：必须保留 `derived_data.py` 的 `setText()`/`text()` 完整文案契约，卡片只额外
  提供无参数 `load_requested` intent，显式接入既有 `ProtocolPanelCallbacks.on_load_component_codec`；不得读取
  ViewModel、创建 Profile/Codec 状态、判断 `derived_source_supported()`、解析 rows 或执行加载动作。空态 CTA
  的 enabled/visible 只能由 `protocol_config.py:set_derived_controls_enabled()` 传入既有 source gate；
  `derived_data.py` 只能依据已经计算的 `visible` rows 互斥隐藏空表头/显示空态、或恢复原生只读表格，不能改变
  过滤、选择、导出和 codec 语义。卡片及其 CTA/glyph 必须接入 `lifecycle.py` 共享 MotionController frame/stop，
  无本地 `QTimer`，鼠标透明区域不得拦截 CTA 焦点；默认 stylesheet 与每个 theme override 必须覆盖
  `componentEmptyState`、eyebrow、title、hint，禁止系统 palette 白色回退。
- `presentation/dataset_curve.py:DatasetCurveWidget` 的空态增强只能消费既有 immutable `CurveSnapshot`、
  `ThemeSpec` 和 lifecycle 共享 `MotionController` frame/stop：`field_name=None`、`sample_count=0`、无可绘制点
  可以显示 glyph、等待说明和 signal rail，但不得创建新的业务状态、百分比/进度、series 自动选择或 CTA action。
  必须保持 `set_snapshot()` 的有界 debounce `QTimer`、`flush()`、`set_suspended()`、`shutdown()`、焦点环、
  AccessibleDescription 和真实 points 的坐标/颜色/来源语义；`set_frame()`/`stop()` 只能改变空态装饰，不能
  改变 snapshot 或数据曲线。组件不得新增常驻动效 timer，隐藏、最小化、关闭和 reduced-motion 必须静态回退。
- `presentation/observation_viewport.py:ObservationViewport` 必须保持 `QPlainTextEdit` 的原生文本、滚动、只读、
  placeholder、AccessibleName/Description 和 controller 更新契约；它只能读取自身内容、`ThemeSpec` 与 lifecycle
  共享 `MotionController` `(phase, animated)`，绘制上下标尺、信号轨和有限 scope 节点，不持有 raw frame、解析结果、
  Dataset sample、业务状态或本地 `QTimer`。`TerminalViewport` 必须继续保留既有类型/API 并固定使用 terminal scope；
  Component/Dataset 只能通过 component/dataset scope 复用 renderer，scope 不得改变数据/滚动事实。三类视口统一由
  `lifecycle.py` fan-out，reduced-motion、暂停、隐藏、最小化和关闭必须静态回退，不得产生横向滚动或系统白色 palette。
- `presentation/connection_preset_context_surface.py:ConnectionPresetContextSurface` 只能消费
  `connection_preset_surface.py` 投影的既有 `ConnectionPreset | None`，显示内置/自定义来源、传输类型、bounded 描述和原有
  `connection_hint` 下一步文案；它不得应用 preset、触发连接、持久化、读取 ViewModel、保存密钥/设备身份或创建业务状态源。
  原有 `connection_hint` QLabel 的 text、AccessibleName/Description、tooltip 和 controller wiring 必须保持；surface 仅暴露
  `set_preset()`、共享 `set_frame()`/`stop()`，使用 `ThemeSpec` 语义 token、无白色 palette 回退，低动效/暂停/隐藏/最小化/关闭
  必须静态回退，生命周期由 `lifecycle.py` 统一接入 MotionController。
- `presentation/transport_mode_surface.py:TransportModeSurface` 只能镜像既有 `_transport_combo.currentData()` 的有界
  `uart`、`tcp_stream`、`tcp_server`、`udp_datagram`、`ble_gatt`、`rtt` 值，绘制模式 glyph；相邻 `QComboBox` 是唯一的选择、
  键盘和无障碍文案 owner。glyph 不得连接、修改 transport config、读取 ViewModel/SessionState、创建模式状态、自动切换面板
  或创建本地 `QTimer`；`set_mode()`、共享 `set_frame()`/`stop()` 只能更新装饰。六模式必须有明确的安全静态回退，980/1180
  下不得制造横向滚动或白色 palette，生命周期由 `lifecycle.py` 统一 fan-out。
- `presentation/command_batch_empty_state.py:CommandBatchEmptyState` 只负责无 batch 时的用户引导：公开无参数
  `new_requested` intent 与 `set_frame()/stop()`，不直接调用 ViewModel/domain/transport。`commands.py` 是唯一的 projection
  owner：无 batch 时必须互斥隐藏 `CommandBatchSurfaceLabel` 和空结果表、显示完整 CTA 卡片；选中 batch 后恢复 status/table，
  不改变 snapshot、执行 gate、结果表选择或 connection controls。CTA 必须服从 `connection.py` 的 batch-edit lock，980/1180 下
  不得因空表头或重复状态带把按钮裁切到 viewport 外。
- `command_batch_editor.py` 的 bounded steps table 必须只保留横向“步骤”列作为序号来源；冗余 `QTableWidget.verticalHeader()`
  必须隐藏，不能让系统 palette 形成白色列。默认 controls stylesheet 与每个 theme override 必须显式覆盖
  `QSpinBox/QDoubleSpinBox` 的 `::up-button`、`::down-button`、`::up-arrow`、`::down-arrow` 背景、hover、disabled 和箭头颜色；
  保留原生 stepper、键盘、焦点和无障碍行为，不得通过关闭按钮或外部图片规避主题问题，也不得影响 `QComboBox`。
- UI-1.72 的 `command_batch_editor.py:_field_label()` 是对话框普通字段/辅助说明的局部装配 helper：名称、快捷命令、固定顺序说明、
  当前步骤和延时必须使用 `QLabel[role="muted"]`，以便三套主题保持字段与编辑控件的层级对比。`_table_empty` 必须继续是
  `role="subtle"`，错误 label 必须继续是 `role="error"`；不得批量改写空态、错误、按钮提示或动态状态。helper 不得读取
  ViewModel/DTO、改变 `_DraftStep`/bounded payload 校验、接收 signal、创建 timer、改变 popup、焦点或 Tab 顺序，也不得让系统
  palette 形成白色背景 fallback。
- `presentation/terminal_surface.py:TerminalViewport` 只能继承现有 `QPlainTextEdit` 的呈现边界：必须先完成原生文本、选择和滚动绘制，
  再在已有 viewport 上下 padding 内绘制低对比度标尺、角标和共享 frame 信号；不得覆盖真实 RX 文本、添加 overlay 文本、读取/复制
  RX 计数或时间，不得改变 `objectName`、只读、无障碍、滚动条和键盘契约。它只暴露 `set_frame()/stop()`，不创建 QTimer、业务状态源、
  controller 或新的 lifecycle 分支；窄 viewport 仍须保留静态回退，三主题、980/1180、1.5x HIDPI、空态/数据态、横向滚动和隐藏/暂停/关闭
  静态停止是固定验收项。
- `presentation/theme_transition.py` 只能提供一次性主题切换过渡：`apply_theme()` 仍是主题事实入口，helper 只对
  bootstrap 持有的 `appRoot` 使用短时 `QGraphicsOpacityEffect/QPropertyAnimation`，不得读取业务状态、创建常驻 QTimer、
  改变焦点/无障碍树或引入第二套主题 token。lifecycle 必须在低动效、暂停、隐藏、最小化和关闭时停止并清理 effect；
  快速连续切换、自然完成和 Qt 对象已回收的边界必须保持安全静态状态。
- `presentation/theme_transition_surface.py:ThemeTransitionSurface` 只能作为上述一次性过渡的鼠标透明、不可聚焦、空 accessibility
  overlay：只使用当前 `ThemeSpec` 的 semantic accent 绘制短暂几何 sweep，由 `theme_transition.py` 的同一生命周期管理
  `geometry` animation。不得创建 `QTimer`、读取 ViewModel/SessionState、进入 Tab 顺序、改变焦点/无障碍树或把 sweep 当成业务进度；
  start/stop/finish、快速主题切换、低动效/暂停、隐藏/最小化/关闭必须清理 overlay 并回到静态主题，三主题与 near-white 门禁必须通过。
- 连接快速配置只允许使用 presentation 内的版本化 `ConnectionPreset` DTO、不可变七项 builtin 和有界 custom catalog；
  它只能填入安全的连接选项，不得自动连接、保存原始密钥、设备句柄或 BLE 身份。选择器必须是 bounded combo，
  应用后保留明确的“已填入、确认端点后连接”提示；catalog 必须是 schema v1 的 immutable tuple，preset key 唯一、
  总数量处于 `1..16`，每类 typed values 在 DTO 构造时完成范围校验。builtin key 永远不可覆盖，custom 项只能追加/替换
  自身，当前内置七项明确包含既有 BLE GATT 扫描兼容入口。
  `ConnectionPresetApplyCallback` 进入 MainWindow，再由 presentation controller 填表；`connection_runtime.py`
  只消费控件中的规范化值构造 domain config，不直接依赖 presentation DTO。TCP Server 的传输切换、监听主机
  或监听端口变化必须失效旧 LAN 确认与 allowlist，避免隐藏授权状态复用。
- `connection_preset_codec.py` 只能序列化 schema v1 的显式白名单字段；JSON schema、DTO 数值和 enum 必须严格校验，
  拒绝 bool/float 冒充整数、未知字段、未知 transport、超长 payload 和坏 catalog，并由
  `connection_preset_store.py` 回退 immutable builtin catalog。`QSettingsConnectionPresetCatalogStore` 是唯一持久化
  adapter，begin/set/end 任一失败都必须 fail-open 且跳过 sync；builder 只接收组合根注入的 catalog，不直接访问 QSettings。
  `ConnectionPresetEditorDialog` 只编辑名称/备注，当前表单快照由 `controllers/connection_presets.py` 构造 typed DTO；
  editor 不接触密钥、设备身份或自动连接。store 只持久化 custom 项并在读取时与 immutable builtin 合并；旧 schema v1
  完整 catalog 会被规范化为 builtin + custom。显式 save/delete 只能在非活动会话/非回放期间执行，save 返回 `bool`，失败时
  当前会话可保留新项但必须明确提示，delete 失败必须保持原 UI/catalog。
- UI-1.43 连接快速配置选中态必须由 `connection_preset_surface.py` 集中维护：`customSelected` 只表达 presentation
  视觉状态，tooltip 与 `AccessibleDescription` 必须同步覆盖未选择、内置、自定义三态；不能把“内置”固定文案留在
  自定义路径，也不能只依赖颜色识别。用户选择仍只填表、不自动连接；本切片不推断手动修改后的 dirty/匹配状态。
- `QComboBox#connectionPresetCombo[customSelected="true"]` 必须在默认 stylesheet、三个 theme override 和
  `check_theme_tokens.py` 必需 selector 集合中保持一致；normal/hover/focus/disabled 均需有主题回退，disabled 不能
  残留自定义底色。ComboBox 保留原生 popup、键盘、焦点和无障碍行为，不新增控件级动画或自绘 popup。
- UI-1.44 协议确认弹窗必须由 `protocol_config.py` 保留确认范围、`dialog_surface.py` 统一配置
  `surfaceRole="confirmation"`、标准 Ok/Cancel 返回语义、按钮文案和 AccessibleName/AccessibleDescription；
  helper 不得读取 ViewModel、协议 worker、transport 或密钥，也不得创建 timer。确认按钮使用既有 `dangerButton` 语义，
  默认按钮必须继续是取消，避免键盘 Enter 在未明确确认时清空派生状态。
- `QMessageBox[surfaceRole="confirmation"]` 必须在默认 controls stylesheet 与主题 override 中使用 warning/history/surface
  语义 token 完成外壳、信息文本、按钮尺寸和禁用回退；`check_theme_tokens.py` 必须锁定该 selector，禁止回落到系统白色
  palette。对话框保持 Qt 原生焦点、键盘、标准按钮和无障碍树，不增加弹窗专用动画或自绘 popup。
- 协议确认弹窗只说明既有派生缓存清理边界；“仅有未完成帧是否需要确认”和“重置文案是否与 editor 行为完全一致”属于后续协议
  状态切片，不能在 UI-1.44 通过 presentation helper 偷渡改变确认触发条件或协议业务语义。
- UI-1.45 的 ComboBox popup 白带修复必须由 `presentation/popup_surface.py` 只配置 Qt 原生
  `QComboBox.view().window()` 顶层 `QFrame` 的 `surfaceRole="comboPopup"` 外壳；不得替换 `QListView`、关闭原生 popup、
  改写键盘/焦点/选中行为，或引入事件过滤器、QComboBox 子类、全局 QApplication stylesheet 和常驻 timer。
- popup frame 的直接 stylesheet 必须由 `theme_stylesheet_runtime.py` 按 `ThemeSpec` 生成小范围规则；默认 controls stylesheet、
  theme override 和 `check_theme_tokens.py` 必须共同覆盖 `QFrame[surfaceRole="comboPopup"]`。样式只处理 frame 背景、边框和圆角，
  item/viewport/scrollbar 继续消费既有原生控件主题规则，禁止复制业务状态或业务文案。
- `theme.py:apply_theme()` 必须在主题切换时刷新现有 ComboBox popup；主窗口组合完成和带 combo 的编辑器构造完成时必须补一次初始化刷新，
  以覆盖 popup top-level window 不继承父窗口 QSS 的 Qt 行为。刷新必须幂等、无业务副作用，并在 980/1180、三主题、popup 展开/关闭下保持近白像素为 0。
- UI-1.46 的文件选择器必须统一通过 `presentation/file_dialog_surface.py` 的 `get_open_file_name()` / `get_save_file_name()`；
  控制器不得直接调用 `QFileDialog.getOpenFileName()` 或 `getSaveFileName()`，避免 Qt 非原生子控件回落系统白色 palette。
- 文件选择器桥只映射 Qt 的 `ExistingFile`/`AnyFile`、`AcceptOpen`/`AcceptSave` 和 selected filter 返回值；必须保留文件系统模型、路径导航、
  键盘/焦点、取消、确认、筛选和保存覆盖确认语义，不得引入自绘文件浏览器、路径缓存、文件 I/O、ViewModel 或 transport 依赖。
- 为使主题 QSS 在 Windows 可见，file-dialog surface 明确使用 `DontUseNativeDialog`；这是 Qt 文件对话框外壳选择，不代表已验证 Windows 原生
  Shell 的视觉或无障碍行为。其 `QFileDialog`/tree/list/header/sidebar/button/scrollbar 规则必须由 `ThemeSpec` 渲染，不能依赖全局 QApplication stylesheet。
- 文件选择器内部 ComboBox 仍使用 `popup_surface.py` 的原生 popup frame 桥；禁止在 file-dialog helper 中创建 timer、事件过滤器、全局样式或复制业务状态。
- `check_theme_tokens.py` 必须锁定 QFileDialog tree/list/header/sidebar/button selector；三主题离屏截图须无目录/列表大面积白底，缺少字体时的浅色文字字形不得被误报为背景白块。
- 只读表格若不需要 Qt 行号，必须在组装层隐藏 `verticalHeader()`；业务序号/步骤列仍由表格内容提供，不得用全局 palette 掩盖原生 header 的白色回退；
- 执行状态只允许由已有 snapshot/ViewModel 事实投影到 presentation `state` 属性；批量命令使用 `empty/ready/running/completed/stopped/failed`，不新增 timer、业务字段或 QSS 内部业务判断；
- `controllers/bootstrap.py` 只负责依赖注入、Qt shell 组装、signal wiring 和首屏投影；`composition.py` 只负责页面/焦点/快捷键接线；controller 不得把 bootstrap 变成新的 God object；
- 禁止用共享 `MainWindowContext`、无限制 `dict` 或 mixin 隐藏跨模块控件依赖；controller 不得直接读取另一个 controller 的 widget，Qt parent/lifetime 必须明确，关闭/隐藏/最小化/低动效策略必须有统一入口；
- 主窗口拆分必须按增量切片执行：先抽取构建和接口，再迁移状态/事件处理，最后删除兼容别名；每片保持可编译、静态检查通过并更新 ADR/handoff，禁止一次性重写导致业务语义和生命周期不可追溯；
- `ota/`、`ota/transports/`、`ota/security/`、`debug/` 与 `presentation/` 是独立变化边界；UI 只消费 application DTO，不导入 OTA adapter、J-Link/SEGGER SDK、socket、pyserial 或加密密钥；
- OTA 安全框架只允许接入经过审查的 authenticated-encryption backend（当前预留 AES-256-GCM、AES-128-CCM）；禁止 ECB、裸 CBC、硬编码密钥、密钥进入 DTO/日志、静默降级、跳过签名/摘要/anti-rollback，激活前必须 fail-closed 验证；
- OTA security profile 只传递 `key_id`/签名 key 引用和策略，不传递原始 key；nonce 唯一性、密钥生命周期、版本单调性、目标身份绑定、掉电恢复和安全回滚必须由后续 target-specific adapter/bootloader 契约明确承担；
- RTT/J-Link 输出框架只允许 attach-only 原始日志与显式 Down 写入；禁止 memory read/write、halt、run、reset、flash、任意 J-Link 命令和自动启动 vendor 工具；RTT/J-Link 记录必须有通道、方向、时间和有界 payload；
- 所有后台工作可取消、可报告状态，UI 线程不阻塞；
- 批量调度在 application worker 中使用可取消等待；presentation 只编辑 immutable DTO 和消费 progress snapshot，不直接访问 transport worker；
- 协议预设目录属于 domain，不能导入 Qt、transport、设备库或动态协议插件；presentation 只负责选择、编辑和显式提交 `ProtocolConfig`；
- NMEA checksum 复用 domain protocol decoder 的 Line 生命周期；原始 recorder/terminal 是事实来源，派生 payload 不得替代完整 wire bytes；
- Modbus RTU ADU 校验位于独立 `domain.modbus`，component codec 只消费一个 `DecodedFrame.payload`；未来 timing framer 必须通过显式边界契约接入，不能把时序逻辑塞进字段 codec；
- MAVLink packet 校验位于独立 `domain.mavlink`，component codec 只消费一个 `DecodedFrame.payload`；
  CRC_EXTRA 映射是 profile 数据，不由 stream boundary 猜测；dialect generator 和 signing verifier
  仍必须通过独立边界接入；
- `GapObservation` 属于 parser ingress DTO，数值必须和 timing quality 一起传递；host read gap
  不得在 UI、codec 或文档中被描述为物理 wire timestamp。generation fence 属于 application
  pipeline 与 immutable event DTO，不塞入 transport、codec 或 Qt 控件；Replay session segment、
  实时 stream boundary 和本地 UI error 状态各自定义，不得互相隐式解决；
- `ProtocolIngressUnit.segment_id` 只属于 parser lifecycle；不把 replay 原始 session 复制为新的 transport/source identity，
  也不把 segment 分支散落到 component、Dataset、recorder 或 UI；
- 错误状态只在 presentation 层聚合；异步 session/replay source gate 仍在 ViewModel 消费分支执行，
  不把错误生命周期逻辑塞入 transport、recorder 或 domain codec；
- 主题必须使用 `ThemeSpec` 语义 token。QSS 与 QPainter 自绘控件均通过 `theme_key_for_widget()` /
  `theme_spec_for_widget()` 获取当前主题；禁止在自绘控件里硬编码背景色或默认白色高光导致主题切换分裂。
  `apply_theme()` 负责写入根主题属性、刷新子控件；新对话框从 parent 继承主题；
- `theme_stylesheet_runtime.py` 只能做纯 `ThemeSpec -> QSS` 渲染；Shell 和 native-control override
  必须分别位于 `theme_variant_shell.py` / `theme_variant_controls.py`。新增颜色先登记为
  success/info/warning/error/history/neutral/disabled、interactive、focus、selection 或 on-accent
  语义 token；禁止用未分语义的全局十六进制替换掩盖同色多义。每个主题必须覆盖 popup、viewport、
  corner、Tab overflow、禁用态和状态来源，不能留下系统白色 palette 回落；由
  `scripts/check_theme_tokens.py` 维护 token/selector 覆盖门和 legacy literal inventory；
- 企业级架构 owner、依赖方向和 staged contract 迁移以 ADR 0025 为准；“参考字节跳动”只表示公开
  工程共性，不得声称拥有其内部规范、代码或认证；
- 所有缓存、帧长、命令长度、日志文件和连接数都有上限；
- 新增依赖必须更新 `docs/DEPENDENCIES.md` 和锁定文件；
- 当前不创建或运行测试专用代码和资产，验证使用已有工具、静态检查、启动检查和授权硬件。
- provenance helper 只能读取项目唯一版本源；manifest 必须记录锁文件 hash、source revision、PE
  metadata、签名状态、硬件状态和许可证清单状态，内容扫描不得允许 J-Link/SEGGER/probe-rs vendor binary；
- core 变体不得收集 Bleak/WinRT；ble 变体必须显式启用 optional extra；RTT attach-only 代码不能
  为了打包切片引入驱动、SDK、DLL 或外部厂商工具。

## 安全与授权约束

- UI-1.118 的 scrollbar 只允许在 stable/theme controls stylesheet 中复用既有
  `BORDER_STRONG`/`accent_purple`、`ACCENT`/`accent`、`ACCENT_PINK`/`accent_pink` 和 surface
  token；必须显式覆盖 default、hover、pressed、disabled 和 corner，禁止白色或系统 palette 回退。
  不得通过 QScrollBar 子类、事件过滤器、timer、MotionController、业务状态、DTO、线程、I/O 或
  callback 改写原生滚动范围、箭头、焦点、键盘和 QPlainTextEdit/terminal 语义。
- UI-1.118 的验证必须覆盖 settingsScroll 与 terminal 等实际 QAbstractScrollArea，在 980/1180
  两个窗口尺寸和三个主题检查 thumb 可见、pressed selector、垂直滚动 range、横向滚动策略和无
  白色背景带；视觉反馈只表达“可滚动/正在拖动”，不能表达连接、OTA、协议或设备状态。
- UI-1.119 的动效性能实验必须以用户端到端证据为准：不得仅因为减少 `set_frame()` 调用就保留
  全局 motion registry、逐帧 `isVisible()` 查询或 Tab 可见集合缓存。共享 `MotionController`、
  `lifecycle.py` 的全量 stop、reduced-motion、暂停、隐藏、最小化和关闭 contract 必须保持简单且
  可回滚；实验若没有同一窗口尺寸/主题/事件处理条件下的稳定收益，必须撤回源码并记录样本、
  环境、结论和未运行的真实硬件/发行验收，不得宣称性能提升。
- UI-1.120 的扩展能力卡约束已由 ARCH-83 / UI-1.156 supersede：历史实现曾要求静态只读 `QFrame`、
  NoFocus 且不得有 clicked/pressed signal；当前实现改为整卡 `QPushButton` 语义、单一 Tab stop 与
  click/focus/Enter/Space 选择，但仍禁止动态 backend 动作、后端句柄、密钥、OTA/J-Link 调用和新的
  业务状态源。`contract_only`/`attach_only` 的 info/blue 与 history/purple 语义、三主题 base/variant
  对称、两列等权 stretch、980/1180 无横向溢出、workspace 淡入和 reduced-motion 静态回退继续保持。
- UI-1.121 的批量命令状态 affordance 必须集中在 `CommandBatchSurfaceLabel`：左侧状态点只能由
  已有 `CommandBatchSurfaceProjection.state` 映射 `neutral/success/warning/error` 语义色，running
  呼吸环只能消费 lifecycle 传入的共享 `(phase, animated)`。不得新增状态源、百分比、设备 ACK/吞吐
  文案、clicked/pressed signal、本地 `QTimer`、MotionController、线程、I/O、OTA/AES/RTT/J-Link
  依赖或 QSS 内部业务判断；原有 snapshot、步骤轨道、结果表、批量 gate、AccessibleDescription、
  NoFocus、鼠标透明和 `stop()` 静态回退必须保持。默认 stylesheet 与每个 theme override 必须覆盖
  非白色状态 surface，左侧 padding 必须避免 marker 与中文文案重叠，980/1180 不得制造横向溢出。
- UI-1.134 的 UART `port/baud/data_bits/parity/stop_bits/flow_control` selector 必须继续由
  `connection_builder.py:_configure_bounded_combo()` 负责 bounded presentation width；980px
  内宽不能因长 option size hint 产生 horizontal scroll，且 1180px 必须保持原有可读空间。
  port 允许 editable COMx，其他 selector 必须保持不可编辑；itemData、选项数量、默认值、
  currentIndexChanged、tooltip/accessibility、连接 gate、焦点顺序和 transport DTO 不得改变。
  允许闭合 combo 的 Qt elide，但 popup/accessibility 必须保留完整选择语义；不得通过全局字体、
  删除选项、resize 状态机、第二套 layout、timer、业务状态或 OTA/debug 依赖解决几何问题。
- UI-1.133 的 `EmbeddedStationRouteGlyph` 必须是 `embedded_station_overview.py` 内的资源无关
  presentation renderer：固定 96×64、NoFocus、鼠标透明、空 accessibility，只消费父链
  `ThemeSpec` 和共享 `MotionController` `(phase, animated)`；不得创建 QTimer、读取
  `ExtensionStationSummary` 的业务字段、解释 contract-only/attach-only、增加后端/动作/密钥/
  OTA/debug 依赖或绘制设备进度。`EmbeddedStationOverview` 负责 `set_frame()/stop()` 静态
  回退；`ExtensionPanelWidgets` 是 layout 与 overview 的唯一 typed composition bundle，
  `workspace.py` 不得通过 `findChild()` 注册，`lifecycle.py` 只能复用既有 `_motion_surfaces()`。
  三主题、980/1180 下必须无横向滚动、无系统白色回退；隐藏、最小化、暂停、低动效和关闭必须
  保持静态路线与只读指标。
- UI-1.132 的 `CommandBatchEmptyState` 卡片必须继续使用 `Expanding/Preferred` 外壳，但其
  `_eyebrow`、`_title`、`_hint` copy QLabel 必须在自身 presentation owner 内使用纵向 `Fixed`；
  不得让文案控件吸收专注模式剩余高度，也不得通过 QSS、resize handler、timer 或命令 runtime
  修复。hint 必须保留 word-wrap，长文案不得裁切；glyph、CTA、`new_requested` signal、共享
  MotionController、焦点/accessibility、主题 token、命令目录和 DTO 不变。三主题、980/1180
  必须无横向滚动和系统白色回退，不新增状态源、线程、I/O、OTA/AES/RTT/J-Link 或设备依赖。
- UI-1.131 的网络/RTT responsive hint 必须复用 `controllers/connection_builder.py` 的
  `_configure_responsive_hint()`：保留 `wordWrap`、120/520px 横向边界、tooltip 和 accessible
  description，纵向使用 `Fixed` size policy，不能让 TCP Client/RTT 说明 QLabel 吸收父级剩余高度。
  长文案必须按当前可用宽度换行且不被裁切；不得通过 QSS、resize handler、timer、传输 runtime
  或硬编码 panel 高度修复。TCP Server allowlist、UDP、BLE、UART、连接 gate、焦点/Tab、主题
  token 和 DTO 不变；三主题、980/1180、六种 transport 必须无横向滚动和系统白色回退，不新增
  状态源、动画、线程、I/O、OTA/AES/RTT/J-Link 或设备依赖。
- UI-1.130 的 `UartTimingSummarySurface` 必须在自身 presentation owner 内使用横向可伸缩、
  纵向 `Fixed` 的 size policy；不得让 UART 参数摘要 rail 吸收父级剩余高度，不得通过 QSS、
  resize handler、timer 或连接 runtime 设定硬编码高度。摘要继续只消费既有 UART selector projection，
  保持文案、tooltip、accessible description、主题 role、signal、连接 gate 和 DTO 不变；三主题、
  980/1180 的稳定专注布局必须无横向滚动和系统白色回退，不新增状态源、动画、线程、I/O、
  OTA/AES/RTT/J-Link 或设备依赖。
- UI-1.129 的 UART 端口空态提示必须由 `controllers/connection_builder.py` 在同一
  `port_placeholder` 语义边界中同时设置 `QComboBox` 与可编辑内部 `QLineEdit`；不得只设置
  combo placeholder 后假设 Qt 会自动传播，也不得把 placeholder 当作 endpoint、写入
  `TransportConfig` 或触发自动连接。端口 combo 仍是唯一允许用户手动输入的 selector，
  `currentText()`、刷新、连接 gate、焦点/Tab/accessibility 和枚举列表保持不变；三主题、980/1180、
  手输/清空必须无横向滚动和系统白色回退，不新增 timer、状态源、QSS 业务分支、OTA/AES/RTT/J-Link
  或设备 I/O。
- UI-1.128 的连接页 UART/网络/BLE section 标题必须由 `controllers/connection_builder.py` 的
  `_section_label()` 统一构造，保留 `role="section"`，使用横向 `Preferred`、纵向 `Fixed` 的
  size policy；不得让标题吸收父级剩余高度，不得通过 QSS、resize handler、timer 或 transport
  runtime 修复。UART/网络/BLE active title 与 panel 必须在三主题、980/1180 和六种连接方式下
  无重叠、无横向滚动、无白色回退；连接状态、焦点、无障碍文案和 panel 可见性不变。
- UI-1.127 的专注设置模式只能由 `presentation/workspace_focus_transition.py` 管理：模式默认关闭、
  只存于当前 window presentation，不得写入偏好或成为 session/transport 状态。按钮只能临时隐藏
  `liveObservationBand`、`terminalSurface`、`sendControlBand`，不得停止接收、记录、解析、连接或
  pipeline；高度过渡必须是一次性 `QParallelAnimationGroup`，不得新增常驻 QTimer、第二套
  MotionController、QGraphicsEffect 悬挂对象、业务事件或滚动策略。低动效、暂停、隐藏、最小化、
  关闭时必须停止 group、清理动画并静态落到当前模式；按钮必须保持主题化、键盘焦点、accessible
  name/description、Tab 顺序和动态文案。三主题、980/1180、专注/总览状态不得有白色回退、横向
  溢出或 route/tab overlap；不引入 OTA/AES/RTT/J-Link 或设备 I/O。
- UI-1.126 的 `build_workspace_tabs()` 只能在 presentation 组装层调整 `QTabWidget` 的空间约束：
  不得把工作区硬最小高度写死为高于父级可用高度；在 980×680 下 tab viewport 必须让位给固定
  31px `workspaceRouteStrip`，由页面已有的 `QScrollArea#settingsScroll` 承担纵向内容。不得新增
  resize/QTimer、responsive 业务状态、第二套 scroll policy、controller/ViewModel facade、设备 I/O、
  OTA/AES/RTT/J-Link 依赖；Tab index、焦点/Tab 顺序、terminal/live/send surface、shared motion
  lifecycle、三主题 token 和 accessibility 必须保持。980/1180 下不得发生 tab/route overlap、
  横向滚动或系统白色回退。
- UI-1.125 的 `ConnectionStatusRail` 只能消费 lifecycle 已传入的既有 session state 与共享
  `(phase, animated)`；`open` 必须使用 success 语义在末节点绘制完成勾，`error` 必须使用 error
  语义在首节点绘制叉号，opening/closing/open 的移动彗尾继续复用共享 frame。不得新增状态源、
  连接判断、按钮动作、QTimer、线程、I/O、OTA/AES/RTT/J-Link 依赖或 controller facade；closed/
  discovered 的原有静态路径、`stop()`、reduced-motion、暂停、隐藏/最小化/关闭、NoFocus 和鼠标
  透明必须保持。marker 只能使用既有 ThemeSpec token，三主题 exact-white=0，980/1180 不得
  产生横向溢出。
- UI-1.124 的连接快速配置上下文只能消费既有 `ConnectionPreset | None` 投影：空值必须映射为
  `source="none"`/`state="empty"`，内置 key 映射为 `source="builtin"`，自定义 key 映射为
  `source="custom"`/`state="selected"`；不得在 surface 内调用 apply/connect、读取 ViewModel、
  重新解析 DTO、创建新的连接状态源或改变“不自动连接”文案/accessible description。属性更新必须
  复用 `refresh_dynamic_property()`；base/variant QSS 与自绘 surface 必须对称使用已有
  `neutral/info/history` semantic token，三主题不得出现白色回退。不得新增 timer、线程、I/O、
  OTA/AES/RTT/J-Link 依赖；共享 frame/stop、NoFocus、鼠标透明、焦点/Tab 和 980/1180 响应式边界
  必须保持。
- UI-1.123 的 Component 空态只能消费 `set_derived_controls_enabled()` 已计算的 source gate：
  enabled 必须投影为 `waiting/COMPONENT / WAITING`，disabled 必须投影为 `blocked/COMPONENT / BLOCKED`，
  且 blocked 必须隐藏/禁用现有 Profile/Codec CTA、保留完整 `setText()/text()` 文案和无参数
  `load_requested` intent。不得在 `ComponentEmptyStateSurface` 读取 ViewModel、判断 transport/history、
  解析 rows、改变 codec/过滤/表格语义、创建 timer/线程/I/O、引入 OTA/AES/RTT/J-Link 依赖或新增
  controller facade；state 更新必须复用 `refresh_dynamic_property()`，base/variant QSS 必须对称使用
  neutral/warning semantic token，三主题无白色回退，980/1180 不得横向溢出，shared frame/stop 和
  NoFocus/鼠标透明/accessibility contract 必须保持。
- UI-1.122 的协议/Component/Dataset/Curve/Replay 状态 marker 必须集中在共享
  `AnalysisStatusLabel`：只能读取既有 `state/source` property 和 shared `(phase, animated)`，
  `history` 必须继续使用既有 purple source 语义，不能复制 pipeline/statistics/generation 业务
  判断。不得新增状态源、进度/吞吐/ACK 文案、点击动作、本地 `QTimer`、第二套 MotionController、
  线程、I/O、OTA/AES/RTT/J-Link 依赖或 QSS 内部业务判断；原生 QLabel 文本、AccessibleName/
  Description、底部五节点轨道、NoFocus、鼠标透明、`stop()` 静态回退、三主题非白色 surface 和
  980/1180 无横向溢出必须保持。左侧 padding 必须以真实组合根 geometry 证明不裁切中文状态文案。
- UI-1.117 的字体适配必须集中在 `presentation/font_runtime.py`，由 `presentation/main.py` 在
  `QApplication` 创建后调用；只允许发现已有 Qt/system font、选择 family，或在 Qt font database
  没有 CJK family 时注册标准本机字体文件。禁止复制/打包未经授权的字体资产、写入用户字体目录、
  下载字体、修改业务状态或引入全局字体 registry。
- UI-1.117 的失败回退必须是无阻断的：字体路径不存在、注册失败或没有可用 family 时保留 Qt
  默认字体并继续启动；不得因字体美化失败影响连接、接收、发送、记录、协议、OTA 或 debug 行为。
  `theme_stylesheet_base.py` 只维护有序字体 fallback；font runtime 不得创建 timer、线程、设备
  I/O、网络、持久化或跨层 callback。字体适配验证必须覆盖至少一个 CJK family、三主题、四工作区、
  980/1180 布局和无横向溢出。
- UI-1.116 的普通主窗口动作必须复用 `ActionRailButton`，并由原 feature owner 创建、由 `lifecycle._motion_surfaces()` 显式收集；`BusyActionButton` 继续专用于已有异步 busy 投影。不得通过基类替换改变 signal payload、objectName、enabled gate、焦点/Tab 或原生 `QPushButton` 行为。
- UI-1.116 的 action rail 只能消费 ThemeSpec 与 shared frame；不得新增 timer、MotionController、registry、业务状态、DTO、线程、I/O 或跨层 callback。动作的可用性、确认、发送、连接、回放和批量执行仍归原 controller/application owner，`stop()` 必须覆盖隐藏、最小化、暂停、低动效和关闭静态回退。
- UI-1.115 的工作区 Tab 动态装饰必须封装在 `presentation/workspace_tab_surface.py:AnimatedWorkspaceTabBar`；只允许在 native TabBar paint 之后绘制当前选中 Tab 的 signal underline/halo，不得替换 QTabWidget 导航、焦点、键盘、currentChanged、icon 或 scroll-button 语义。
- UI-1.115 必须由 `workspace.py` 显式 setTabBar、由 `lifecycle._motion_surfaces()` 复用 shared frame/stop fan-out；不得新增 timer、MotionController、业务状态、registry、Qt signal 转发、线程、I/O 或跨层依赖。`stop()` 和 reduced-motion/paused/hidden/minimized/close 静态回退必须可验证。
- UI-1.114 的四个协议/组件/Dataset 派生动作必须复用 `presentation/action_surface.py:ActionRailButton` 与 `lifecycle._motion_surfaces()`；动作创建、业务回调和 source gate 仍归 protocol/protocol_config owner，signal rail 只能消费已有 ThemeSpec 与 frame payload。
- UI-1.114 不得新增常驻 timer、第二套 MotionController、动作 registry、状态源、DTO、线程、I/O 或业务判断；`set_frame()`/`stop()` 必须保留隐藏、最小化、低动效、暂停和关闭时的静态回退，禁用态 tooltip/accessibility 仍由既有 gate 投影。
- UI-1.113 的 Profile/Codec 加载、组件 CSV 导出、Dataset 配置加载和 Dataset CSV 导出必须保留既有 enabled gate，并在原 owner 内提供动作专属 tooltip/accessibility；禁用态必须说明来源原因，不得将“当前不可用”误写成设备、连接或 OTA 错误。
- UI-1.113 不得改变四个回调、焦点/Tab 顺序、派生 DTO、解析状态、原始终端/记录、发送、线程、timer、依赖、设备 I/O 或 OTA/debug contract-only/attach-only 边界；不得引入全局 affordance registry 或新的状态源。

- UI-1.110～UI-1.112 的动作、字段、对话框与 QAction 必须在原 presentation owner 内提供准确 tooltip 和 accessible description；动态状态必须与既有 projection 同步，不能创建全局 affordance registry、第二套状态源或跨层 Qt 文案服务。
- 发送输入、发送格式、CRLF payload 与发送按钮的说明必须区分“输入/格式化/发送”职责；UART 字节间超时必须保留秒单位和 0 的不设定语义；保存、取消、确认、填入不得暗示自动连接、自动发送、设备已接收、OTA 已执行或 J-Link 已 attach。
- 对话框和 QAction 的 affordance 改动不得改变 DTO、校验、信号、回调、焦点/Tab 顺序、关闭生命周期、设备 I/O、线程、timer、依赖或 OTA/debug contract-only/attach-only 边界；必须通过三主题 offscreen 组合根和 near-white 门禁。

- TCP Server 默认只监听回环地址；LAN 监听必须显式确认并支持 peer allowlist；
- 自动重连要求稳定设备身份，VID/PID 或名称不能单独作为安全身份；
- J-Link RTT 当前只允许 attach-only Telnet 原始字节读取和显式 Down 写入，禁止通用内存读写、halt、run、reset 和 flash；该能力排在 UART、网络、BLE 和协议层之后；
- OTA 默认关闭且不进入当前 core/BLE 发行变体；没有目标设备型号、bootloader 版本、包格式、签名链、密钥托管、恢复/回滚证据前，不允许连接、传输、激活或声称“安全升级”；
- OTA 的 XMODEM/YMODEM/TFTP 仅代表待实现的协议适配边界，不等于协议认证、可靠重试、设备兼容性或安全性；每个 adapter 必须单独记录规范版本、目标范围、边界、失败恢复和硬件验收证据；
- AES 不是完整升级安全方案；必须同时记录镜像 hash、签名验证、anti-rollback、目标绑定、nonce/密钥管理、失败恢复和激活结果，不能只展示“已加密”状态；
- SEGGER SDK/DLL 不默认打包，分发前必须有书面授权结论；
- 任何设备密钥、网络凭据和敏感日志默认不写入普通日志；
- PySide6/Qt、PyInstaller 和各传输库的许可证必须随发行物记录；当前第三方文件只能标记为 inventory，
  在完整许可证文本和法律复核完成前，manifest 必须保持 release_eligible=false。
