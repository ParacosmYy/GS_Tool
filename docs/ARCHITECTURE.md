# SerialForge Python 架构

## ARCH-128 / UI-1.201 协议页与命令页局部响应式宽度 owner

协议页与命令页的窄宽度问题由局部 presentation owner 收敛，不再把共享 scroll owner 当作布局补丁。`controllers/protocol.py` 内的
`_ResponsiveProtocolRow` 只接收已经创建的 label/control widget，在同一 `QGridLayout` 内按 regular/compact/narrow 重排行列；
`protocol_config_context`、component table/preview、Dataset preview 只在实例级释放外层 intrinsic width，内部数据视图仍保留自己的
原生滚动语义。命令页分别由 `command_workspace_builder.py` 的 `_ResponsiveCommandActionRow` 与
`command_batch_empty_state.py` 的 `_ResponsiveCommandEmptyHeader` 负责 action/空态 header 几何。

这些 owner 不创建业务控件，不持有 ViewModel、DTO、signals、transport、timer 或 MotionController，也不修改 `ResponsiveScrollArea`、
`composition.py` 或全局滚动条策略。真实组合根三主题×9宽度×4页共 108 行，以及协议可见叶子 27 行均验证外层 `hmax=0`、内容不越过
viewport。ARCH-128 的详约见 [`ADR-0179`](adr/0179-responsive-page-width-contracts.md)。

## ARCH-127 / UI-1.200 UART 参数列级 sizing contract

`ResponsiveUartForm` 是 UART labeled field wrapper 的唯一几何 owner。它以字段 `minimumWidth/minimumSizeHint/sizeHint` 推导 hard 与
preferred contract，在 regular 模式显式使用四个语义列，在 compact 模式使用两列，在 narrow 模式使用单列；water-fill 分配是确定性的，
不会依赖 Qt 的隐式猜测。mode 变化与 FontChange/StyleChange/LayoutRequest 均在同一 owner 内清理并重设列宽/stretch，不重建控件或信号。

窗口初始宽度只是偏好，字段 hard contract 才是可进入下限。ARCH-127 只改 presentation geometry，保留 builder 的控件创建、bindings、
signals、focus/Tab、UART runtime wiring 与唯一共享 120Hz MotionController。详约见 [`ADR-0178`](adr/0178-uart-column-sizing-contract.md)。

## ARCH-126 / UI-1.199 Header 三态响应式控制带

本轮修复 Header 在窄窗口下状态摘要被压缩为 `UA…/实…/…` 的可读性问题。新增
`presentation/responsive_header_controls.py::ResponsiveHeaderControls` 作为 Header 控制区唯一几何 owner；它只接收
`build_app_header()` 已创建的 `status_cluster`、`motion_controls`、`theme_controls` 三个 QFrame，不持有业务状态、偏好、signals、
callbacks、MotionController、timer 或主题策略。

owner 使用同一个 `QGridLayout(SetNoConstraint)` 在 `REGULAR`、`COMPACT`、`NARROW` 三态间重排：宽屏三组同排，紧凑态 status 独占一行、
动效与主题同排，窄态三组分三行。模式阈值由 QFrame 的 sizing contract、spacing 和 margins 推导；`minimumSizeHint()` 的宽度只使用窄态
hard-min contract，不把 preferred width 变成窗口 floor。三组 QFrame 在注入时一次性成为 owner 的子对象，模式切换只在同一 owner grid 内
批量 `removeWidget/addWidget`，不跨 layout、不重复 setParent、不重建、不重连信号；同态只 invalidate/activate。

`_AdaptiveHeader::_sync_header_density()` 仍是 Header policy owner，但职责限定为 density property、装饰 signal field/theme swatch 显隐、
内部间距和 state caption 显隐，完成这些变化后调用 owner sizing invalidation，不再直接管理三个控制卡片的行列。HeaderChromeBindings、window refs、
生命周期控制器和 Tab/focus/accessibility wiring 不变。状态卡在紧凑/窄态独占完整行；当前 Microsoft YaHei UI offscreen font metrics 下，546px
窗口内容宽约 514px 时三条状态文本完整需求约 332px，验证无省略号。

架构师 `019ff613-3aea-7f92-b6db-2567ca74cee7` 对 owner、reparent、sizing、首帧和最终 stretch/height contract APPROVE。
最终独立代码审查 `019ff62a-986e-71c0-98f0-13007776a724` 与修复复核
`019ff62a-3bae-7cd2-a4c2-c82732a8110a` 均为 `APPROVE WITH ADVISORIES`（Critical/Required=0）；简化审查
`019ff62a-98bb-7851-a19d-56c59142fded` 为 `APPROVE WITH ADVISORIES`（无必须简化项）。审查保留两条非阻断 advisory：
组合根的三个既有 QFrame 由 builder 创建后一次性注入 owner，注入完成后统一 parent；以及只含一个 owner 子项的
`controls_row.setSpacing(8)` 为无行为影响的冗余语句。本轮为 Python/PySide6 presentation-only，embedded C/C++
public-vendor-source applicability=N/A。

## ARCH-125 / UI-1.198 窄窗口响应式窗口、连接带与 UART 参数表单

本轮把“窗口能缩小，但内容仍挤成横向长条”的问题收敛为两个独立的 presentation geometry owner。`bootstrap.py` 不再设置
`980×720` 的硬窗口最小尺寸，只保留初始 `1240×820` 偏好；窗口最低宽度由真实内容的 sizing contract 推导，避免窗口级魔法像素。

`connection_builder.py::_ResponsiveConnectionBand` 只持有既有传输/预设/连接控件的几何布局，按控件的
`minimumSizeHint()/sizeHint()`、spacing 和 margins 选择 `REGULAR`、`COMPACT`、`NARROW_COMPACT` 三态。宽度不足时，连接按钮、
预设选择、保存/删除动作、上下文提示和状态 rail 分行；对象 identity、signals、bindings、itemData、Tab 顺序、scroll owner 和连接业务状态
仍由原 builder/controller 管理。`FontChange`、`StyleChange`、`LayoutRequest` 与 resize 均经过幂等 sizing invalidation。

`presentation/responsive_uart_form.py::ResponsiveUartForm` 是 UART 字段的独立 geometry owner。它只接收 builder 已创建的十个 labeled field
wrapper，不创建 raw control、不持有 UART DTO 或业务状态；按既有 sizing contract 在 `REGULAR`、`COMPACT`、`NARROW` 三态中排列，窄态为
单字段纵向列表。这样端口、波特率、数据位、校验、停止位、流控、超时和线路控制不会在窄窗口互相挤压；`connection_builder.py` 仍负责控件创建、
信号连接和 `UartControlBindings` 暴露。

架构师 `019ff5c9-8eff-7cd3-9c2f-ab0a40ec9ef0` 对硬窗口移除、连接带三态、sizing invalidation、UART owner API 和最终微简化均 APPROVE。
独立审查 `019ff5df-ef65-7f00-9b30-03c62fad4be9` 在初始反馈要求补齐全页窄态与 sizing invalidation，修正后最终为
`APPROVE WITH ADVISORIES`（Critical/Required=0）；简化审查 `019ff5df-efac-7453-96b5-f9d61b07c0ff` 的初始建议已处理，最终为
`APPROVE WITH ADVISORIES`（无必须简化项）。本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability=N/A。

## ARCH-124 / UI-1.197 命令上下文带与独立操作行

`presentation/command_context_band.py::CommandContextBand` 是命令页选择上下文的唯一 presentation geometry owner。它接收
`command_workspace_builder.py` 已创建的历史标签、历史 ComboBox、清除按钮、批量标签和批量 ComboBox，仅把两个既有选择 cell
按 Qt sizing contract 放入一列或两列；不创建业务控件，不读取 ViewModel，不持有批量定义、执行状态、transport、timer 或动画状态。

`command_workspace_builder.py` 是组合根：负责创建控件、连接现有 signals，并把五个批量操作按钮放入独立 action row；`CommandContextBand`
不拥有按钮行为、执行链路或新的公开业务接口。owner 使用 `QGridLayout.SizeConstraint.SetNoConstraint`，由 cell 的
`minimumSizeHint()/sizeHint()` 推导宽屏阈值；窄屏只切换现有 cell 的行列位置，保持对象 identity、focus/accessibility 和 Tab 顺序。
compact mode 的最小宽度取单列最大 cell，当前列数对应的最小高度随布局模式变化；不通过窗口魔法像素、不增加 QSS selector、不套嵌套
`role=surface` 卡片。命令操作折叠/收纳不在本切片范围。

架构裁决：`019ff5a5-cb5d-75b3-9fa6-c41bc2a66802` APPROVE owner/boundary；Terra follow-up
`019ff5a8-f256-7002-b8fc-86de2dab2dd0` APPROVE Qt sizing contract 与非嵌套视觉边界；`019ff5b5-7c29-7fa3-9c27-14b251e0e951`
APPROVE 最小高度/宽度稳定契约；`019ff5c0-2320-7ec1-99d1-2b7d90444be1` APPROVE docstring-only 修正。独立审查
`019ff5bc-af7c-7f91-bc2d-891292e96db0` 无 Critical/Required，结论 `APPROVE WITH ADVISORIES`；简化审查
`019ff5bc-b25e-7743-9a2b-f0ae71f185f8` 无必须简化项。本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source
applicability=N/A。

## ARCH-123 / UI-1.196 命令批处理空态与页面垂直节奏

`presentation/command_batch_empty_state.py::CommandBatchEmptyState` 是命令页空态的 presentation owner。它只持有空态文案、装饰 glyph、
主操作信号和静态三步引导，不读取 ViewModel、批处理 catalog、执行状态或 transport。`CommandBatchStepRail` 只重排已经创建的三张
`CommandBatchStepCard`；初始化和 `resizeEvent` 都经过同一个幂等 `_relayout()`，按逻辑宽度在 3 列和 1 列之间切换，不创建焦点控件、timer、
动画循环或业务状态。`command_workspace_builder.py` 只消费现有 `new_requested`，空态使用自然高度，不再占用页面的 flexible stretch slot。

`presentation/responsive_scroll_area.py::ShortPageVerticalRhythm` 是共享短页面的显式节奏契约：默认 `CENTER` 保留连接/协议/扩展页的
短内容居中与 overflow 顶对齐；`workspace.py` 仅为命令工作台传入 `TOP`，使批处理操作从 viewport 顶部开始，避免工作台空态被居中到
大块无层次空白。`composition.py::scroll_page()` 负责传递该 keyword-only 策略；不按页面名称硬编码，不改变 `QScrollArea`、viewport、
滚动条或业务链路。

主题边界仅新增 `commandBatchStep*` 的语义样式，默认样式和三主题 override 均复用既有 semantic token；不写裸色，不改变
`MotionController`、`set_frame()`、`stop()`、焦点顺序、accessibility contract、OTA/AES/RTT/J-Link contract-only/attach-only 边界。
本轮为 Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability=N/A。

架构裁决：主裁决 `019ff58a-9d3d-75b0-9638-95529059138a` APPROVE；页面节奏 follow-up 与首帧幂等 follow-up 均 APPROVE，后者为
`019ff59a-996b-76e0-adbf-48a8633392a1`。独立审查 `019ff595-c839-7311-8c47-1a4006951912` 与最终审查
`019ff5a0-c897-7791-8c69-aeb9ffb23b47` 均无 Critical/Required，结论为 `APPROVE WITH ADVISORIES`；简化审查
`019ff595-cb11-7370-b84c-6d088909da57` 与最终简化审查 `019ff5a0-cb7f-7fe3-aee0-f0c5a25920bc` 均无必须简化项。

## ARCH-122 / UI-1.195 高刷新共享时钟与响应式概览指标

`presentation/widgets.py:MotionController` 仍是全窗口唯一 presentation scheduler。为适配 Windows 的高精度调度，`PreciseTimer` 使用
4ms slot；其 `_tick` elapsed、`_last_tick_at`、activity deadline 和 `_can_animate()` 判断统一使用 `time.perf_counter()`，不与业务层
时间源混用。frame budget 仍按 `elapsed × TARGET_HZ` 累积，每个 callback 最多 emit 一帧，出帧后保留小数余量并丢弃积压整数预算，避免
卡顿恢复后的 repaint burst。该方案不新增 timer、线程或 MotionController，保留 phase cap、reduced-motion、pause、hide/minimize/close/rearm
生命周期；4ms 是 scheduler 机会，不是显示器 FPS 保证。

`embedded_station_overview.py::_ResponsiveMetricGrid` 是扩展概览指标的唯一 presentation 排布 owner。它只重排已有 summary metric tuple，
按扣除 layout margin/spacing 后的可用宽度在 6/3/2/1 列间切换；指标 value 使用可忽略宽度、最小宽度 0 和换行，完整原文保留在
`accessibleDescription`。不改变 `ExtensionStationSummary`、业务状态、动画转发、theme objectName、滚动 owner 或 OTA/AES/RTT/J-Link
contract-only/attach-only 边界。Python/PySide6 presentation-only，embedded C/C++ public-vendor-source applicability 为 N/A。

架构审批：`019ff579-3e62-7551-9cf5-4907a3638a9a` APPROVE perf_counter 统一时间域；独立 reviewer `019ff57c-bd7a-79b3-838c-6471f5a8d87c`
APPROVE，无 Critical/Required findings；简化 reviewer `019ff57c-bdd6-7f30-965b-cb05faefd0b1` 认为无需扩大简化（仅指出未读取标记可删，
本轮不改）。

## ARCH-121 / UI-1.194 overview workspace height budget owner

`controllers/workspace.py::_ResponsiveWorkspaceShell` 是总览态 workspace shell 的唯一纵向高度 owner；
`_overview_workspace_minimum_height()` 读取 root layout 当前 sibling 的最小高度预算，计算不会挤压实时观测、终端和发送
surface 的安全 `minimumHeight`。980×720 下 floor 约 168px，宽屏 floor 封顶 220px；shell 的实际高度可以在剩余空间充足时
自然扩张，不能把实际高度误读成 floor。

该 owner 不拥有 page 内容、业务状态、ViewModel、transport 或 scroll。连接/协议/命令/扩展页继续由既有
`QScrollArea` 承担纵向 overflow；`workspace_focus_transition.py` 只在 focus snapshot restore 完成后调用一个兼容能力
同步入口，确保总览 floor 不残留 focus 临时高度。resize 首帧使用最多两轮、可合并的一次性 queued settle，并以
`shiboken6.isValid` 防止对象销毁后的回调；它不是常驻 timer、动画时钟或新的 MotionController。

不改变 `WorkspaceShellBindings`、Tab、signals/callbacks、focus/accessibility、transport 业务、滚动 owner、共享
`MotionController` 或 OTA/AES/RTT/J-Link contract-only/attach-only 边界。Python/PySide6 presentation-only，embedded
C/C++ public-vendor-source applicability 为 N/A。架构裁决：`019ff53b-61e5-7a10-8ad9-a1f153e5903a`、
`019ff549-d471-71b3-90aa-d8ba7f08933e`、`019ff54e-0953-7c21-a93f-3f77d09ce5e4`、
`019ff550-f612-7fe1-b11e-bee17163d8e9` 均为 APPROVE。

## ARCH-120 / UI-1.193 terminal/send toolbar sizing owners

`controllers/terminal_toolbar_builder.py::_ResponsiveObservationBand` 是实时观测栏的 presentation geometry
owner；`controllers/send_bar_builder.py::_configure_send_layout_sizing()` 是发送栏 sizing policy owner。两者
只消费既有 QWidget 的 Qt sizing contract，并把按钮/选择器等动作控件保持 intrinsic，允许状态/输入/摘要 surface
吸收剩余宽度；不持有业务状态、ViewModel、transport、scroll owner、timer、动画或 MotionController。

两处 owner 都在已有 builder 组装完成后调整同一批 QWidget 的 size policy/spacing，不创建替代控件，不复制
`TerminalControlBindings`，不改变 signal/callback、focus/accessibility、发送/接收状态或 OTA/AES/RTT/J-Link
边界。Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-119 / UI-1.192 connection shell responsive owner

`controllers/connection_builder.py::_ResponsiveConnectionBand` 是连接页 transport/preset shell 的唯一
响应式 presentation layout owner。它只登记 panel 已创建的 QWidget，并在 `resizeEvent` 中依据现有控件
`minimumSizeHint()/sizeHint()/minimumWidth()` 推导是否进入 compact layout；它不创建控件、不持有业务状态、
不改变 `ConnectionShellBindings`、signal/callback、MotionController 或滚动 owner。

常规布局保留既有三行结构；紧凑布局把 transport/连接动作与 preset/保存删除分成两行，再让 preset context
和 status rail 各自跨满内容列。重排只 remove/re-add 同一批 QWidget，object identity、focus/accessibility 和
主题 objectName/state 保持不变。UART 参数区、transport 业务、OTA/AES/RTT/J-Link contract 边界不在本轮范围。
Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-118 / UI-1.191 shell transition mutual-exclusion owner

`presentation/transition_coordinator.py` 是窗口级一次性 shell transition 的唯一互斥编排 owner。它只定义
`ShellTransitionKind`、在新 transition 启动前停止其他 owner 的 `prepare_shell_transition()`，以及生命周期/早退
使用的幂等 `stop_shell_transitions()`；四个既有 transition owner 仍负责自身动画、effect、完成回调和 identity
检查。协调器通过延迟导入避免循环依赖，不创建 timer、MotionController、布局、业务状态或滚动 owner。

`theme_transition.py`、`workspace_runtime.py`、`workspace_focus_transition.py`、`transport_panel_transition.py`
在有效启动前登记互斥边界；无效、隐藏、最小化、低动效和窗口 lifecycle 早退统一全量清理。`lifecycle.py` 只调用
协调器做窗口级清理。共享 `MotionController` 仍是唯一时钟，OTA/AES/RTT/J-Link 和 transport 业务边界不变。
Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-117 / UI-1.190 扩展能力分区 rhythm owner

`presentation/embedded_extension_panel.py::_build_capability_section()` 是扩展工具站每个能力组的
presentation 分区 owner。它只接收 panel 已创建的 title、hint 与既有 `ResponsiveCapabilityGrid` 卡片，
用一个 `QFrame#extensionCapabilitySection` 统一承载视觉层级和垂直节奏；容器 `NoFocus`、不持有业务
状态、不创建滚动 owner、timer、动画或 MotionController。卡片仍由 panel 创建并由 panel 持有选择/详情/
accessibility/Tab 顺序，原生 QScrollArea 仍由 `scroll_page()` 持有。

分区水平不增加内缩（`0, 10, 0, 10`），避免与网格的 360px 卡片最小宽度叠加后使 1180px OTA 三卡
错误退回两列；上下边距 10px、内部 spacing 7px 用于形成可辨识的三段能力层级。Python/PySide6
presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-116 / UI-1.189 扩展能力卡响应式网格 owner

`presentation/responsive_capability_grid.py` 是扩展工具站卡片摆放的唯一 presentation layout owner。
它接收 `embedded_extension_panel.py` 已创建的同一批 card QWidget，在 resize 时按可用宽度（扣除布局
边距与列间距）选择 `1..min(3, len(cards))` 列，仅在列数变化时 remove/re-add 原对象；不重建 card，
不拥有 selection/detail/accessibility/tab order，也不创建 timer、scroll owner、MotionController 或业务状态。

`embedded_extension_panel.py` 继续拥有 capability catalog、selection_controls、detail projection 和
`ensure_card_visible()`；现有 QScrollArea 继续是唯一滚动 owner。980px 内容宽度保持两列，1180/1240px
OTA 三卡使用三列，双卡组保持两列；Qt 像素取整导致同一行出现最多 1px 宽度差属于等权 stretch 的正常结果。
Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-115 / UI-1.188 短页面垂直节奏 owner

`presentation/responsive_scroll_area.py` 是共享设置页滚动容器的唯一垂直节奏 owner。它只持有现有
`QVBoxLayout` 引用，并在 viewport resize、show 和 vertical scrollbar range 变化时，根据 layout
`sizeHint()/minimumSize()` 与 viewport 高度切换垂直 `AlignVCenter` 或 `AlignTop`；水平 alignment、
layout stretch、size policy 和页面 API 均保持不变。内容超出 viewport 时仍由同一个 `QScrollArea` 原生
滚动，未创建 nested scroll、timer、MotionController 或业务状态。

`controllers/composition.py:scroll_page()` 只负责包装这个 presentation helper；连接、协议、命令和扩展
页面 builder 不感知策略。命令页的 `CommandBatchEmptyState` 继续拥有自身 `Expanding` slot，避免重复
分配伸缩权。Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-114 / UI-1.187 根布局伸缩权与主题工作区 owner

`workspace_focus_transition.py` 继续是 focus/overview 的唯一 presentation owner。本轮把根布局中
workspace shell 与已有 terminal slot 的 stretch 所有权显式化：focus 使用 `shell=1, terminal=0`，
overview 通过 `_FocusLayoutSnapshot` 恢复进入前的 root stretch、shell/tabs policy、min/max、route
stretch、terminal parent/child hidden、鼠标透明和终端 size policy。shell 内部由 tabs 拥有可伸缩高度，
route strip 保持固定 31px；页面内容仍由现有 `QScrollArea` 负责垂直滚动。

布局先静态结算，动画只复用现有唯一 `MotionController` 做 opacity；不新增 timer、geometry animation、
root spacer、业务状态、设备 I/O 或 OTA/AES/RTT/J-Link coupling。新增的 bool 返回契约只表达临时布局
应用成功与否，调用者继续按失败短路。Python/PySide6 presentation-only，embedded C/C++ public-source
applicability 为 N/A。

## ARCH-113 / UI-1.186 专注工作区自然高度 owner

`workspace_focus_transition.py` 继续是 focus/overview 模式、下方 surface 可见性和一次性视觉过渡的
唯一 presentation owner。本轮在 focus 模式隐藏实时观测与发送区，并复用已有透明 `terminalSurface`
作为根布局的可伸缩 slack；临时保存并恢复 terminal 子控件的 `isHidden()` 语义、父 surface 可见性、
鼠标透明属性、垂直 `QSizePolicy` 及 min/max height。当前 `QScrollArea` 内容的
`minimumSizeHint()` 决定短页面的自然 tabs/shell 高度，目标高度超过可用窗口空间时继续交给原生滚动，
不制造新的白色占位块。

`workspace_runtime.py` 仅在 workspace route 变化且 focus 状态未变化时调用显式 refresh；
`lifecycle.py` 在原生 resize 后复用同一 owner 重新结算。没有改变组合根 child 顺序、root stretch、
业务状态、连接/记录 worker、`MotionController`、timer、QSS、scroll owner 或 OTA/AES/RTT/J-Link
边界。Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-112 / UI-1.185 紧凑顶栏状态标题 owner

`controllers/workspace.py:_sync_header_density()` 继续是自适应顶栏的唯一 presentation owner。本轮将
视觉上的“连接状态”标题保存在 `_AdaptiveHeader._state_caption`，并以实际 `header.width()` 为依据：
低于 `1120px` 时只隐藏这个与状态灯/状态值重复的视觉 token，保留状态灯、当前状态文本、状态组的
accessible name/description 以及动态状态投影；达到阈值时恢复标题。该策略不创建新的状态源、timer、
QSS 规则、业务判断、滚动 owner、设备 I/O 或 OTA/AES/RTT/J-Link 依赖。Python/PySide6
presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-111a / UI-1.184 共享动效时钟 elapsed budget owner

`MotionController` 继续是全窗口唯一的 presentation scheduler：它保留 8ms `PreciseTimer`、
`TARGET_HZ=120`、共享 `frame_changed` fan-out 以及 pause/reduced-motion/hidden/minimized/close
生命周期。为消除整数 8ms 槽位带来的长期预算偏差，`widgets.py` 现在用已有的实测 monotonic
elapsed 累积 `elapsed × TARGET_HZ`；每个 tick 最多发出一帧，不循环追赶积压预算，因此不会在
事件循环恢复时制造 repaint burst。该切片不新增 timer、线程、组件状态、业务判断、scroll owner
或 OTA/AES/RTT/J-Link 依赖。Python/PySide6 presentation-only，embedded C/C++ public-source
applicability 为 N/A。

## ARCH-110 / UI-1.183 命令空态水平空间 owner

`CommandBatchEmptyState` 继续拥有命令空态的 presentation 排版；它保留命令页唯一 vertical
stretch slot。本轮只在组件内部的 horizontal root layout 两侧增加 stretch，并让 glyph+copy
使用自然宽度，从而把空态内容组置于画布中心。命令 controller、ViewModel、执行语义、scroll
owner、CTA signal、accessibility 和唯一 `MotionController` 依赖不变。Python/PySide6
presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-109 / UI-1.182 专注/总览首帧 reveal owner

`workspace_focus_transition.py` 继续是 focus/overview 模式、下方 surface 可见性和一次性视觉
过渡的唯一 presentation owner。布局仍先一次性结算最终 sibling geometry，再由既有
`MotionDrivenAnimationGroup` 驱动 opacity；本轮仅把 overview reveal 的起点提到命名常量 `0.82`，
避免 surface 在 220ms 过渡内完全透明造成空白画布。没有新增 timer、动画时钟、布局 track、业务
状态、scroll owner、transport/OTA/debug 依赖。唯一 `MotionController`、120Hz target、生命周期
stop/cleanup 和 reduced-motion policy 保持。Python/PySide6 presentation-only，embedded C/C++
public-source applicability 为 N/A。

## ARCH-108 / UI-1.181 扩展焦点可见性 owner

扩展页组合 owner 继续负责能力卡选择与详情可见性；当详情改变现有 scroll content 高度时，它同步
调用 content `adjustSize()`/layout activation，再复用外层 `QScrollArea.ensureWidgetVisible`，最后
只在实际 viewport geometry 越界时修正既有 vertical scrollbar。能力卡不读取滚动状态、不创建 timer，
应用层 catalog 与 extension contract 边界不变。该修复只发生在 focus/click presentation 路径，
不进入 `MotionController` 的共享帧循环。Python/PySide6 presentation-only，embedded C/C++
public-source applicability 为 N/A。

## ARCH-107 / UI-1.180 扩展详情展示 owner

`embedded_extension_panel.py` 继续是扩展页的组合 owner。它只控制
`ExtensionCapabilityDetail` 的 presentation 可见性：初始隐藏，能力卡第一次 focus/click 后显示；
详情组件仍只投影传入的不可变 `ExtensionCapability`，不读取 ViewModel、不调用 OTA/安全/调试后端。
能力目录和 contract-only/attach-only 状态仍属于 application catalog；本轮没有改变 XMODEM/YMODEM/
TFTP、AES-128-CCM/AES-256-GCM、RTT/J-Link 的边界。没有新增 timer、thread、设备 I/O 或共享动效
owner，卡片选中状态仅是扩展页局部 presentation state。Python/PySide6 presentation-only，embedded
C/C++ public-source applicability 为 N/A。

## ARCH-106 / UI-1.179 工作区模式上下文 owner

`WorkspaceContextLabel` 继续是 route strip 中页面上下文文案的唯一 presentation leaf；它复用已有
Tab index 投影页面名称，并由 `workspace_focus_transition.py` 的既有 `_set_shell_mode` 同步接收
focus/overview 模式。模式信息只写入同一 label 的动态 `mode` 属性、文案、tooltip 和 accessible
description，不进入 ViewModel，不复制 focus 状态，也不创建新的 widget、timer、scroll owner 或
动画时钟。`workspace.py` 的固定 31px route strip、`WorkspaceScrollHint` 和 `WorkspaceRouteSurface`
边界保持不变。Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-105 / UI-1.178 工作区滚动位置语义 owner

`WorkspaceScrollHint` 继续是 route strip 的唯一滚动语义 owner。它只绑定当前
`QScrollArea.verticalScrollBar()` 的 `valueChanged` 和 `rangeChanged`，将已有的
`top/middle/bottom/complete` 映射为位置与动作文案；不读取 ViewModel，不统计 section，不创建
第二个 scroll owner，也不驱动动画。

提示的 accessible name 与动态 description 在同一 presentation leaf 内维护，稳定 stylesheet
只调整字重，四态颜色仍由现有 semantic token selector 提供。`workspace.py` 仍负责 route strip
组合与固定高度，`workspace_runtime.py` 仍负责绑定当前 Tab；依赖方向没有反转。Python/PySide6
presentation-only，embedded C/C++ public-source applicability 为 N/A。

## ARCH-104 / UI-1.177 命令空态工作区画布 owner

命令页的布局继续由 `presentation/controllers/command_workspace_builder.py` 组合，空态的视觉
排版继续由 `presentation/command_batch_empty_state.py` 自包含。没有批量命令时，空态占据页面唯一
可伸缩 vertical slot；`CommandBatchEmptyState` 使用 expanding policy，glyph 保持垂直居中，
copy 区前后 stretch 负责内容焦点。这样布局不会让 builder 和 root trailing stretch 同时分配空白。

`controllers/commands.py` 仍是 batch/snapshot/visible 投影的唯一状态 owner；空态组件不读取
ViewModel、不执行命令、不改变结果表、不拥有 scroll。批量命令出现后 controller 继续隐藏空态，
自然高度和外层滚动策略保持不变。没有新增 timer、线程、paint loop、业务状态或 OTA/debug 依赖。

本轮曾验证 1ms scheduler + elapsed budget，但真实 `app.exec()` 没有改善回调 cadence，且在平台
0ms/16ms 交错下会丢弃积压预算，因此撤回；最终实现保持唯一 `MotionController`、8ms
`PreciseTimer`、`TARGET_HZ=120` target 和既有生命周期门禁。三主题×980/1240、四 workspace
真实 Qt offscreen 组合无 sibling overlap；Python/PySide6 presentation-only，embedded C/C++
public-source applicability 为 N/A。

## ARCH-103 / UI-1.176 路线条模式层级 owner

路线条的模式视觉继续由既有 `workspaceShell` owner 暴露的 `mode` 动态属性驱动。稳定的
`theme_stylesheet_controls.py` 定义 focus/overview 的语义结构，`theme_variant_controls.py` 只覆盖
当前 `ThemeSpec` 的颜色；没有把 focus 状态复制进业务 ViewModel，也没有在 route surface 中新增
状态源。focus 使用 info→history 渐变和蓝色上沿，overview 使用 neutral 上沿，固定 31px route strip
高度与 `WorkspaceFocusButton`、`WorkspaceScrollHint` 的键盘/accessibility contract 保持不变。

该切片只改变 workspace shell 的 presentation stylesheet，不触碰 transport、session、protocol、
OTA/debug contract 或共享时钟；三主题 focus/overview 实跑、980/1240 四 workspace 横向 scroll
与无障碍文案通过。Python/PySide6 presentation-only，embedded C/C++ public-source applicability
为 N/A，不作固件标准或认证声明。

## ARCH-102 / UI-1.175 顶栏密度与布局先结算的过渡

紧凑顶栏的响应式策略继续由 `presentation/controllers/workspace.py` 单独拥有：header 根据自身宽度
同步上下留白、控制簇内边距、间距和 density QSS；主题选择器、焦点和无障碍语义不移交给连接业务
builder。这样 980px 窗口减少的是装饰壳的空白，而不是字段、状态或可操作控件。

专注模式的 `workspace_focus_transition.py` 负责模式状态、下方 surface 可见性和临时 effect；切换时
先调用静态布局 policy 并激活根布局，再由 `MotionDrivenAnimationGroup` 只写 opacity。`QTabWidget`
的 `maximumHeight` 不再作为动画 track，因此配置页不会在共享帧之间被连续压缩；overview 的实时、
终端和发送 surface 也在最终 sibling 位置上分别淡入。时钟仍只有窗口级 `MotionController`，没有
新增 timer、线程、业务状态、scroll owner 或 transport/OTA/debug 依赖。

中间帧证据：980×720 的 overview/focus 25/55/95/140ms sibling 几何均无重叠，focus 连接页 viewport
保持 514px；三主题×980/1240 横向 scroll 均为 0。Python/PySide6 presentation-only，embedded
C/C++ public-source applicability 为 N/A，不作固件标准或认证声明。

## ARCH-101 / UI-1.174 一致的共享帧时钟与独立过渡 owner

本轮将 page fade（`controllers/workspace_runtime.py`）、theme root fade+sweep
（`theme_transition.py`）、transport panel fade（`transport_panel_transition.py`）和 dialog fade
（`dialog_transition.py`）迁移到 `MotionDrivenAnimationGroup`。统一的是时钟，不是 owner：每个功能
仍持有自己的有限 transition、目标 effect、完成回调和清理入口；全窗口只有既有一个
`MotionController`，因此不产生第二 `QTimer` 或全局 TransitionManager 的高耦合状态仓。

`QPropertyAnimation` 只在 driver 内维护 property 类型、起止值和 easing/interpolation，不调用
`start()`；只有 `MotionDrivenAnimationGroup.start()` 连接 `frame_changed` 并请求共享 activity。主题
sweep 和 root opacity 属于同一 group，避免两个时钟同时写 sweep 几何。driver 在 active frame 中
检查 target 的 `shiboken6.isValid()`，失效时 stop 并完成 owner cleanup；生命周期仍由各 owner 与
`lifecycle.py` 在 pause、reduced-motion、resize、hide、minimize、close 处收口。

dialog 通过现有 parent chain 取得主窗口 `_motion_controller`；没有 controller 时回到静态效果清理，
不会创建 dialog 自己的 timer。该方案保持 owner 高内聚、依赖向内，并不把 transport/session/OTA/
debug 状态带进动效层。

本轮 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A。ARCH-101
架构师 `019ff382-2d72-7d32-a067-09b13c9a32d5` 建议保留“每功能一个 finite owner、全窗口一个共享
时钟”；独立 reviewer `019ff389-959a-77b0-8f26-1e444ee50ac6` 超时关闭，未形成外部结论；父代理
完成 correctness、architecture、security、performance、readability 五轴 review 与简化评估。

## ARCH-100 / UI-1.173 共享帧驱动的专注过渡 owner

本轮把专注设置↔总览的有限过渡从 Qt 默认 `QPropertyAnimation` 时钟迁移到
`presentation/motion_transition.py:MotionDrivenAnimationGroup`。`QPropertyAnimation` 仍只提供
属性类型、起止值和 easing/interpolation contract；它不调用 `start()`，由 group 在唯一
`MotionController.frame_changed` 信号上按 monotonic elapsed time 调用 `setCurrentTime()`，再将
当前值写入目标属性。这样高度与 reveal opacity track 共用既有 8ms/120Hz target scheduler，避免
默认动画 timer 在 offscreen/Windows 环境下约 35Hz 的低 cadence。

`workspace_focus_transition.py` 继续拥有 focus 的布局目标、临时 opacity effect、反转和清理；
`MotionDrivenAnimationGroup` 不读取 SessionState、transport、OTA/debug DTO，不拥有 timer、线程、
scroll owner、设备 I/O 或业务状态。`lifecycle.py` 仍在 reduced-motion、pause、resize、hide、
minimize、close 边界停止并静态收口。其他 page/theme/dialog 一次性 fade 仍由各自 owner 管理，后续
迁移必须另开增量并复用该 driver，不能为每类动画再创建时钟。

本轮是 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为 N/A；
Luna 架构师 `019ff36f-09e1-7a03-a5ac-2f4aa8393791` 与独立 reviewer
`019ff376-5ce1-7ae2-a4b6-aa1faf66cf7c` 调用超时关闭，未形成外部结论；父代理完成
correctness、architecture、security、performance、readability 五轴 review 与行为保持简化评估。

## ARCH-99 / UI-1.172 紧凑窗口连接页 focus owner

本轮将“连接配置页是否需要完整首屏”的判断归口到 `presentation/controllers/workspace_runtime.py`。
当窗口高度处于 `760px` 及以下的紧凑区间时，连接页自动复用既有 `workspace_focus_transition`，
把实时观测、终端和发送区让位；`resizeEvent` 和 Tab 路由只调用同步入口，不创建新的时钟或滚动
容器。用户在连接页点击“返回总览”或“专注设置”时写入本会话 presentation-only override，自动
策略不会抢回显式选择。宽屏仍由既有 onboarding/overview 逻辑控制，非连接工作区仍进入 focus。

`workspace.py` 只负责把用户点击转成窄接口，`lifecycle.py` 只负责 resize 生命周期，连接/协议/会话
业务状态没有进入该策略；`open_connection_setup` 在 Tab 信号前设置 CTA intent，避免宽屏下先淡出再
立刻展开的双动画。该切片不新增 timer、thread、事件总线、业务状态、设备 I/O、OTA/debug coupling
或第二 scroll owner。本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source
applicability 为 N/A；Luna/Terra 架构调用与独立 reviewer 均在等待窗口内超时关闭，父代理完成五轴
review 与行为保持简化评估。

## ARCH-98 / UI-1.171 共享动作按钮高度契约

本轮将 `ActionRailButton` 的内容安全最小高度归口到 `presentation/action_surface.py`，避免空态
和终端 surface 在各自 builder 中重复猜测主题字体的尺寸。`command_batch_empty_state.py`、
`component_empty_state.py` 与 `terminal_surface.py` 只组合动作和既有 callback，不再写局部
`setFixedHeight(30)`；因此不会出现主题 QSS 计算出的 `minimumHeight=36` 与固定 `maximumHeight=30`
互相矛盾。动作按钮仍是 native Qt 控件，装饰 rail 仍只消费既有 MotionController frame。

该修复不新增 timer、线程、业务状态、滚动 owner、事件总线或 backend 依赖，不改变 button 文案、
signal、焦点、accessibility 或空态投影。三主题和四 workspace 的横向 scroll 继续由既有外层 owner
负责；本轮为 Python/PySide6 presentation-only，embedded C/C++ public-source applicability 为
N/A。架构师与独立 reviewer 调用在等待窗口内超时关闭，父代理完成五轴 review 与行为保持简化评估。

## ARCH-97 / UI-1.170 扩展能力卡的视觉 contract owner

本轮修复的是 QSS 优先级，不是业务能力实现。`embedded_extension_panel.py` 继续拥有能力卡的
内容、选择信号和 accessibility；`theme_stylesheet_extension.py` 只声明该 objectName 的颜色、
边界和内容安全高度。专属 `QPushButton#extensionCapabilityCard` selector 通过明确的
`min-height: 122px` 保住 builder 已声明的布局 contract，抵抗后置通用按钮规则的默认高度覆盖。

卡片变高后仍由既有 extension `QScrollArea` 承担纵向内容，workspace、MotionController、
application capability DTO、OTA/AES/RTT/J-Link owner 和连接生命周期均不参与该修复。三主题只
通过既有 `ThemeSpec` override 继承同一高度语义；本轮不新增 timer、线程、事件总线、状态源或
backend。embedded C/C++ public-source applicability 为 N/A；架构师与独立 reviewer 调用超时，
父代理完成五轴复核与行为保持简化评估。

## ARCH-96 / UI-1.169 有限整数选项的 owner 边界

本轮没有把“用户不应手填常用数字”实现为业务配置中心，而是在既有
`presentation/bounded_value_combo.py` 内增加 `_BoundedNumericCombo` 的共享选择行为，并由
`BoundedIntCombo` 暴露窄的 `value()`/`setValue()`/`setRange()` contract。帧长、UDP 报文、Server
client 数和批量延时的 option catalog 仍是各自 presentation builder 的输入；端口和 host 等自由
输入不经过该控件。

协议动态固定上限只调用 selector 的 `setRange()`，该 owner 在 clear/rebuild/select 期间屏蔽
中间 `currentIndexChanged`，避免 dirty callback 被重建过程重复触发；用户实际改变选项仍沿用
已有 combo signal。程序化导入的非 catalog 值会临时插入当前 selector，保持旧 DTO 的精确值，不
引入 settings 或 ViewModel 的第二状态源。`connection_runtime.py`、protocol config 和 command
editor 仍是各自业务值的解释 owner。

这是一项局部 presentation 增量，不新增通用表单框架、线程、timer、事件总线或跨层 facade。紧凑
宽度限制属于控件的视觉 contract，防止数值下拉把 980px 页面撑出横向 scroll；布局、主题和 tab
order 继续由原有 workspace/connection/dialog owner 负责。本轮为 Python/PySide6 presentation-only，
embedded C/C++ public-source applicability 为 N/A；架构师与独立 reviewer 调用超时关闭，父代理
完成五轴复核和简化评估。

## ARCH-95 / UI-1.168 超时选项控件的 owner 边界

本轮把“用户不应手填超时小数”收敛为一个 presentation leaf：
`presentation/bounded_value_combo.py:BoundedFloatCombo` 维护有限的 label→seconds 映射，
`controllers/composition.py:timeout_combo()` 负责统一按最小/最大值筛选选项并设定默认值，
`connection_builder.py`、`network_builder.py` 和 `ble_builder.py` 只负责组合字段。新控件不依赖
domain/application，因而不会把 UI 配置状态反向带入 transport 或 session。

`connection_bindings.py` 将八个字段的具体类型改为 `BoundedFloatCombo`，但保持窄的
`.value()`/`.setValue()` contract；`connection_runtime.py` 的 DTO 构造路径没有变，仍由 runtime
在连接前读取秒数。超界程序化赋值按原 spin box 行为裁剪，非选项旧值只作为不可编辑临时项保留
精确值，避免迁移时静默改变配置；用户没有文本编辑入口，也没有 settings/domain 第二状态源。

这是一项局部 presentation 迁移，不新增通用数值框架、timer、signal bus、线程或跨层 facade。后续
若端口、帧长等数字控件也需要选项化，应按各自业务范围新增 catalog 并复用边界原则，不把不相关
的配置强行塞进 timeout module。本轮为 Python/PySide6 presentation-only，embedded C/C++
public-source applicability 为 N/A；架构师与独立 reviewer 调用超时关闭，父代理完成五轴复核
和简化评估。

## ARCH-94 / UI-1.167 工作区 reveal 的 owner 边界

本轮只调整 `presentation/workspace_focus_transition.py` 的 presentation 生命周期。专注设置切回
总览时，既有 `QParallelAnimationGroup` 继续负责 Tab 与三块下方 surface 的实测高度过渡；新加的
临时 `QGraphicsOpacityEffect` 只作为 reveal 的视觉遮罩，让 `liveObservationBand`、
`terminalSurface`、`sendControlBand` 在获得足够空间后渐显。它不读取 ViewModel、不解释 SessionState，
也不把 opacity 当作业务状态或进度。

`_clear_focus_transition_effects()` 是 effect 的唯一清理入口：自然完成、停止、快速反转、低动效、
隐藏、最小化、resize 和关闭最终都会回到 `_apply_static_layout()`，恢复 opacity 并解绑由本 owner
创建的 effect；已有外部 graphics effect 不会被覆盖。工作区仍只通过既有 `WorkspaceShellBindings`
和 `TerminalControlBindings` 取得 presentation widget，依赖方向没有反转，也没有新增 timer、
MotionController、线程、scroll owner、业务状态或 OTA/debug coupling。

ARCH-94 不改变 `MotionController.TARGET_HZ=120`、8ms PreciseTimer scheduler target、phase、Tab、
focus/accessibility、session/transport 或主题 contract。该切片是 Python/PySide6 presentation-only；
embedded C/C++ public-source applicability 为 N/A。架构师与独立 reviewer 调用均在服务窗口内超时
关闭，父代理完成 fresh correctness/architecture/security/performance/readability review 与简化评估。

## ARCH-93 / UI-1.166 推荐连接 preset 的 hydration 边界

本轮把“首次打开少填几项”的体验放在既有 preset controller 内，而不是把默认 UART 数值塞进
bootstrap 或 ViewModel。`connection_presets.py` 只声明稳定推荐 key；
`controllers/connection_presets.py` 负责从 bounded catalog 找到该 entry 并把 combo 设为已存在的
selection；实际字段填充仍沿用 `_dispatch_connection_preset()` 与 `apply_connection_preset()` 的
既有路径。`bootstrap.py` 只在窗口组合完成后调用一次 hydration，因此 composition root 不拥有
业务值，也没有新增 facade 或状态源。

手动 transport 变化由 `connection_runtime.py` 负责检测 selection 的 `ConnectionPreset.transport`；
不匹配时将 combo/context 回到 placeholder，避免 stale summary。选择 profile 时 transport combo
信号暂时屏蔽、应用值后重新投影既有 panel；因此推荐、内置、自定义和 clear 都共用同一条安全路径，
不自动连接、不访问硬件、不保存密钥。依赖仍是 `bootstrap → preset controller → typed bindings`，
`connection_runtime` 只协调 transport/presentation 边界。

本轮只增加一次性 UI hydration 和 stale-context guard，没有增加 timer、线程、backend、scroll owner
或跨层循环；Python/PySide6 presentation-only，embedded C/C++ public vendor source applicability 为
N/A。架构师/独立 reviewer 调用超时关闭，父代理 fresh review 未发现 Required finding；这不等同于
外部审查 PASS。

## ARCH-92 / UI-1.165 响应式布局与 motion fan-out 边界

本轮把连接页的几何责任收敛到既有 presentation owner：`composition.py` 只定义 scroll page 的
顶部对齐与共享 viewport contract，`connection_builder.py` 只定义连接带、UART panel 和既有
typed bindings 的组合几何。二者不读取 ViewModel 业务事实，不创建新的滚动容器、线程或计时器；
剩余空间由 workspace viewport 持有，短配置页不会再被垂直 stretch 拉出白色空段。

共享动效仍由 `MotionController` 单独拥有时钟。`lifecycle_motion.py` 只负责可见 surface 快照、
activity gate 和 frame fan-out；`motion_active()` 作为 surface 层可选能力，让连接 rail、状态
指示器、preset context 和 footer 在静态状态下退出逐帧重绘，但不把状态模型搬进协调器。没有
predicate 的既有 surface 继续走兼容 fallback；`animated=False` 仍广播给全部 surface，保持
停止/低动效/隐藏恢复的确定性。

本轮只减少无效 repaint、修复布局 ownership，不新增 clock 或跨层 facade。依赖方向继续为
`composition/connection_builder → presentation leaves`，运行时状态仍由 lifecycle/ViewModel
投影；Python/PySide6 presentation-only，embedded C/C++ public vendor source applicability 为
N/A。所有相关文件保持 1000 行以内；父代理 fresh review 未发现 Required finding，独立审查调用
超时关闭，不能视为外部 review PASS。

## ARCH-91 / UI-1.164 ViewModel worker owner boundary

本轮把 `viewmodels.py` 中四个 Qt worker carrier/job 拆到 `presentation/viewmodel_jobs.py`，形成
明确的 bounded worker owner。新模块只依赖 domain discovery port、BLE discovery model/error 和
presentation Qt adapter；它不依赖 `SessionViewModel`，不持有 session 状态，不创建线程池或 timer，
因此依赖方向继续向内且没有新增 import cycle。

`SessionViewModel` 保持 facade 和生命周期 owner：全局 `QThreadPool`、discovery/BLE cancellation
`Event`、busy/closing 状态、用户状态文本、完成/失败回调仍留在原模块。`refresh_ports()` 与
`scan_ble()` 仅实例化 job、连接既有 signals 并提交到同一线程池；30 个 Qt signal、构造与对外方法
保持不变。worker 的 `SerialForgeError`/unknown exception 映射和取消后的 emission gate 原样保留。

拆分后 `viewmodels.py=918`、`viewmodel_jobs.py=94`，两者各自低于 1000 行；没有引入第二个状态
源、线程池、事件总线、mixin 或兼容性 facade。架构师与独立 review 调用均在服务窗口内超时并关闭，
父代理完成 fresh review，未发现 Required finding；这不等同于外部审查 PASS。该切片为
Python/PySide6 presentation/application boundary，embedded C/C++ public vendor source applicability
为 N/A，不作固件、MISRA、ISO 26262 或认证合规声明。

## ARCH-90 / UI-1.163 shared motion owner boundary

本轮把接近 1000 行的 `controllers/lifecycle.py` 中共享动效协调责任迁移到
`controllers/lifecycle_motion.py`。新 owner 只依赖 presentation bindings、`workspace_runtime` 的
motion policy 和 presentation property bridge，负责 catalog、visibility snapshot、frame fan-out、
activity gate 与 live-RX activity projection；它不创建 `QTimer`，也不拥有 domain/application 状态。

`lifecycle.py` 仍保留主题、状态 badge、connection context、错误通知及 Qt show/hide/close/change
事件，并直接导入 motion symbols。`workspace.py` 仍从 lifecycle 连接 shared `frame_changed`，
`terminal_runtime.py` 仍从 lifecycle 更新 data activity，`workspace_runtime.py` 的反向调用仍是
function-local lazy import，因此没有新增 import cycle，也没有改变现有 window facade。

拆分后 `lifecycle.py=662` 行、`lifecycle_motion.py=300` 行，两个文件均保持单一职责；
`MotionController.TARGET_HZ=120`、8ms `PreciseTimer` target、phase offset、visible snapshot、
stop/rearm/close 语义均保持。Python/PySide6 presentation-only；embedded C/C++ public vendor
source applicability 为 N/A。架构师与独立 review 调用在服务窗口内超时并关闭，父代理完成
fresh review，未发现 Required finding；这不等同于外部 review PASS。

## ARCH-89 / UI-1.162 protocol workspace density boundary

本轮继续由 `controllers/protocol.py:build_protocol_panel()` 作为协议/遥测组合 owner，未把
`ProtocolPanelWidgets` 拆成新的跨模块契约。这样 `composition.py` 仍只负责 callback 注入和 staged
window refs，`protocol_config.py`、`derived_data.py`、`replay.py` 仍只通过既有 refs 更新业务投影，
没有引入反向依赖或第二个状态源。

布局内部把原本 7 列 `QGridLayout` 改为 `QVBoxLayout` 下的三字段语义行；Dataset 和 Replay 各自把
状态/速度与动作分开，surface root/inner spacing 统一提高。这个变化只影响几何，不改变 37 个
`ProtocolPanelWidgets` 字段、callbacks、signals、tab order 或 MotionController 生命周期。新增
布局函数没有必要形成独立 builder：当前边界仍是单个协议工作区，拆出文件只会增加 binding 迁移面；
若后续协议页继续加入新的协议族或独立状态源，再以 typed sub-binding 拆分。

本轮仍是 Python/PySide6 presentation-only；embedded C/C++ public vendor source applicability 为
N/A。架构师调用和独立审查调用均在服务窗口内超时并关闭，父代理完成 fresh review，未发现
Required finding；这不等同于外部审查 PASS。

## ARCH-88 / UI-1.161 command workspace builder 边界

本轮把命令管理页从 `terminal.py` 移到 `command_workspace_builder.py`。新 owner 只创建发送历史、
批量选择、命令动作、状态、结果表和空态 CTA；定义、快照、执行、停止、enablement 和 motion activity
仍由既有 ViewModel/command controller/lifecycle owner 负责。`workspace.py` 只负责把 layout 放进既有
`commandPage` scroll，`bootstrap.py` 继续在同一组合根组装 `CommandBatchControlBindings`。

批量区采用选择行与动作行的用户任务分层，动作行的 trailing stretch 释放窄窗口空间；所有 action 仍是
原来的 typed widgets 和 callbacks。依赖方向为 `workspace → command_workspace_builder → commands /
presentation leaves`，runtime 不反向导入 builder。没有新增状态源、timer、线程、backend 或 scroll
owner；本轮仍是 Python/PySide6 presentation-only，embedded C/C++ public vendor source applicability
为 N/A。

## ARCH-87 / UI-1.160 terminal live-band builder 边界

本轮将 `terminal.py` 的两个高密度 live band 拆成两个 presentation owner：
`terminal_toolbar_builder.py` 只创建实时观测控制及其状态 rail，`send_bar_builder.py` 只创建发送
输入、发送动作、发送状态、内容摘要和快捷动作。两者仅接收现有 window presentation boundary，
绑定既有 callbacks，并把 staged references 交给 `bootstrap.py` 组装 `TerminalControlBindings`。

`terminal.py` 继续持有错误通知、发送历史和批量命令，因为这些 surface 共享 command workspace 的
历史/批处理组合上下文；它不再持有 live observation/send band 的控件构造。两行 layout 用输入优先
的 stretch 和独立的 status/action row 表达用户任务，不改变 application/domain 状态，也不增加
scroll owner、timer、线程或 backend。

依赖方向保持为 `bootstrap → builder → presentation leaf/typed binding`，runtime controller 仍只
通过 `terminal_bindings_for(window)` 消费 bundle。Python/PySide6 presentation-only；embedded C/C++
public vendor source applicability 为 N/A。

## ARCH-86 / UI-1.159 动效 leaf 与网络 endpoint builder 边界

本轮把两类容易膨胀的 presentation 责任收敛到稳定 owner：`widgets.py` 的
`MotionController` 继续持有唯一共享时钟，`SignalFieldWidget` 只是无状态绘制叶子；网络构建则
从 `connection_builder.py` 移到 `controllers/network_builder.py`。后者创建 TCP/UDP/RTT 控件，
设置默认值和 accessibility，并返回原有 `NetworkControlBindings`；它不读取 ViewModel、不持有
transport lifecycle、不启动 socket/Telnet/J-Link，也不改变 endpoint callback。

`connection_primitives.py` 是无状态的 presentation helper 边界，负责统一 bounded combo、hint、
field label、section label 的视觉约束。`connection_builder.py` 仅保留组合根、UART/外壳 wiring、
网络 endpoint signal fan-out 和 BLE builder 接入；runtime controller 继续通过
`network_bindings_for(window)` 消费 typed bundle。这样新网络协议可以在独立 builder/contract 中扩展，
不会把 UART、BLE、窗口 lifecycle 和动效时钟再次耦合到一个大函数。

头部 signal rail 由 5 个移动点收敛为 3 个等间距轨道点，减少装饰层级但不改变共享 frame signal、
phase、pause/reduced-motion 或 close fence。当前实现没有新增业务状态、线程、timer、滚动 owner
或硬件路径；Python/PySide6 presentation-only，嵌入式 C/C++ public vendor source applicability
为 N/A。

## ARCH-85 / UI-1.158 派生空态可见性 owner

本轮把“空态 guidance”与“有数据观察窗口”明确成互斥的 presentation projection。`protocol.py`
只在组合阶段创建 `component_preview` / `dataset_preview` 并将其初始状态设为 hidden；
`derived_data.py` 根据已有 ViewModel projection 的 `protocol_frames_latest` 与 Dataset samples
决定是否展开。组件空态仍由 `rerender_component_table()` 管理，Dataset 状态仍由既有统计投影管理，
因此没有把业务事实复制到 Qt widget。

无帧时，组件区保留一个可操作的 `ComponentEmptyStateSurface`；有帧时，预览恢复可见并继续承载
最近 120 条有界原始摘要。Dataset 无样本时只保留配置 action、统计状态和曲线自身空态；样本到达后
恢复有界观察窗口。所有既有字段、signals、binding、accessibility 和关闭路径保持不变，隐藏只是
减少重复占位与不必要的空白表面，不改变数据管线或 transport owner。

该切片仍限定于 presentation controller/builder；不接入 domain、application、infrastructure、
硬件、OTA/AES、RTT/J-Link 或新线程。共享 `MotionController` 继续是唯一动效时钟，预览不创建
timer；ARCH-82 的 120Hz scheduler target 只作为约 8ms 调度目标，不被包装为显示器刷新率保证。

## ARCH-84 / UI-1.157 BLE 配置 surface 分层

BLE GATT 参数组合从 `connection_builder.py` 的多传输大函数中移到
`presentation/controllers/ble_builder.py`。新 builder 只接收已有的 window presentation boundary，
创建 BLE controls、连接现有 callbacks，并返回原样的 `BleControlBindings`；它不读取 ViewModel、
application/domain 状态，不创建设备 session，也不拥有扫描/连接策略。

布局 owner 采用三个用户任务分组：扫描与筛选、设备发现与连接、GATT 特征与收发。每组内部通过
`form_fields` 的无状态 helper 组合垂直字段，长内容交给既有外层 `settingsScroll`，避免 7 列网格在
980px 窗口中把字段挤成不可读的横向带。`connection_builder.py` 仍是 connection shell 的组合入口，
只把 `BleControlBindings.title/panel` 加入 root；`ble.py`、`connection.py`、`commands.py` 继续消费
typed bundle/accessor，既有 window attribute 是 staged migration 的兼容 wiring，不是新的状态源。

该切片只改变 presentation geometry 与文件边界：`BleGattWriteMode` itemData、控件默认值、signal
callback、BLE 手动扫描/显式连接/不自动重连语义、focus/accessibility、shared MotionController 和
close fence 保持不变。新 builder 不引入 timer、线程、网络、密钥、vendor SDK 或硬件动作。

## ARCH-83 / UI-1.156 扩展能力卡选择与详情投影

扩展页的组合责任仍由 `presentation/embedded_extension_panel.py` 单独持有。它读取 application 层的
immutable `ExtensionCapability` tuple，按既有 OTA 传输、OTA 安全和 RTT/J-Link 三个 group 组装
overview、一个详情叶子和 7 张全卡片 selector；不会向 extension DTO 增加 UI 状态，也不会把 backend
动作提前接入未来工具站。

`ExtensionCapabilityDetail` 位于 `presentation/extension_capability_detail.py`，只负责把一个 DTO 投影为
标题、中文 group/reference、raw `key/group/reference/state`、summary、boundary 和固定的只读 guard。
它没有列表 owner、timer、信号总线、transport handle 或设备依赖。卡片的 selected 状态与详情更新仍在
panel 的局部闭包中完成，`ExtensionPanelWidgets` 只把外层 layout/overview 交给 workspace composition，
避免跨 controller 暴露内部控件。

卡片使用完整 `QPushButton` 语义：整卡是唯一焦点节点，鼠标 click、键盘 Enter/Space 和 focus projection
都调用同一份 bounded selection；选择仅改变 presentation surface。详情插在 overview 与分组卡之间，
复用 `scroll_page()` 外层滚动容器，不创建 nested scroll/splitter/sticky layout。卡片和详情的静态 QSS
由 `theme_stylesheet_extension.py` 持有，三主题 variant override 由 `theme_variant_shell.py` 对称覆盖；
`theme_stylesheet.py` 仅按 base → extension → controls 组合 stylesheet。

这项决策 supersede UI-1.120 的“静态 NoFocus QFrame”交互限制，但保留只读、无 backend、无设备 I/O、
无 OTA/debug 执行路径的架构边界。详情是用户可理解的说明投影，不是 OTA/AES/RTT/J-Link 实现入口。

## ARCH-82 / UI-1.155 动效 surface 快照与恢复调度

`controllers/lifecycle.py` 是动效 surface 可见性和生命周期的唯一 owner。它第一次需要时建立
不可变 `_motion_surface_catalog`，并在 `Show`、`Hide`、`ParentChange`、resize、Tab route、主题/窗口
状态边界将 `_motion_surface_snapshot` 标脏；真正的 `isVisibleTo()` 查询只发生在快照重建，不发生在每个
animation frame。`on_motion_frame()` 使用完整 catalog index 作为相位偏移，避免隐藏/显示切换后相邻
surface 改变相位；activity-only surface 在从可见快照移除时只 stop 一次。

`MotionController` 仍是 presentation 层唯一共享时钟：`TARGET_HZ=120`、`PreciseTimer`、8ms 是
scheduler target。窗口第一次显示、hide/show 和 minimize/restore 都由 lifecycle 先 suspend，再通过
同一个可合并的 queued callback 跨过 native show path；callback 先检查 closing/visible/minimized，只有
`rearm_required` 时才停止并断开旧 timer、重建同配置 timer，然后按既有 ambient/paused/reduced-motion
policy 恢复。`frame_changed` fan-out、phase、activity 状态和 controller 实例保持稳定；不创建第二个常驻
时钟、不把 `processEvents()` 或 sleep 放进 UI 逻辑。offscreen/Windows 恢复阶段的调度粒度单独记录，
不作为真实显示器刷新率承诺。

该切片只修改 presentation controller/widget 的生命周期边界，未改变 domain/application/infrastructure、
UART/网络/BLE/RTT、OTA/AES contract-only/attach-only 语义或硬件路径。

## ARCH-81 / UI-1.154 UART 参数字段密度

`controllers/connection_builder.py` 是 UART 参数面板的唯一 presentation 组合 owner；本轮只将
原本八列 `QGridLayout` 改为三行语义字段布局，并复用 `form_fields.py` 的
`build_labeled_field()` / `build_field_row()`。第一行承载端口与波特率，第二行承载数据位、校验、
停止位和流控，第三行承载三个超时字段与线路控制。端口刷新按钮和三个线路 checkbox 先在各自的
局部 QWidget 内组合，再作为一个字段交给通用字段 helper，避免把控制集合的 ownership 扩散到
网络/BLE builder。

`UartControlBindings` 仍由同一 builder 唯一组装；`itemData`、默认值、枚举值、timeout range、
signal callback、连接 gate、Tab 顺序和 accessibility 文案不变。`UartTimingSummarySurface` 仍
只消费现有 combo projection，domain/application/infrastructure、MotionController、全局 timer、
设备 I/O 和 OTA/AES/RTT/J-Link contract-only/attach-only 边界均未改变。该切片的目标是降低
980px 窗口的横向压缩，不把业务状态或响应式策略下沉到 domain。

## ARCH-78 / UI-1.151 动效层级边界

`MotionController` 仍是唯一 presentation clock；它只额外暴露只读的
`activity_active()` 投影，生命周期 controller 负责把共享 frame 分成两级：持续环境装饰与
短时业务活动反馈。`ActionRailButton`、`BusyActionButton` 以及 terminal/batch/component
三个空态 composite 通过 `MOTION_MODE="activity"` 声明 activity-only；其他状态 surface
继续接收环境帧。按钮和 composite 不读取时钟、不请求 activity，也不拥有 timer。

`controllers/lifecycle.py` 是唯一 fan-out/cleanup owner：activity 结束时按 true→false 下降沿
停止 activity surface，隐藏页在活动期间只做一次 stop，重新可见后才恢复 frame；全局 hide、
minimize、pause、reduced-motion 和 close 仍统一调用全量 stop。这个边界减少默认路由上持续
动画的 action surface 数量，不改变 callback、session、protocol、itemData、焦点或 accessibility。

## ARCH-80 / UI-1.153 顶部 header 响应式密度

`controllers/workspace.py` 的 `_AdaptiveHeader` 是 header 几何策略的唯一 owner；它在自身
`resizeEvent()` 中调用 `_sync_header_density()`，不把 presentation 几何判断下沉到 lifecycle
controller。header 的 Qt logical width 小于 1120 时，只隐藏没有焦点、鼠标透明且没有无障碍名称的
`SignalFieldWidget` 与 `ThemePaletteSwatch`，并把控制簇间距从 8px 收到 6px；`statusCluster`、
`motionControls`、`themeControls`、主题下拉、低动效/暂停动效和连接状态不移动、不折叠、不禁用。
宽度恢复后装饰件按当前 ThemeSpec 原位显示。

该策略不创建 timer、不写偏好、不复制业务状态、不改变 `HeaderChromeBindings`、Tab 顺序、
accessibility、`MotionController` 或 `lifecycle._motion_surfaces()`；装饰件不可见时由既有可见性
过滤自然跳过 repaint。这样把窄窗口的空间让给可操作控件，同时保留宽屏的二次元星芒与色盘表面。

## ARCH-79 / UI-1.152 命令空态垂直密度

`controllers/terminal.py:build_send_panel()` 继续是命令页纵向组合 owner；在
`CommandBatchEmptyState` 之后增加 trailing stretch，把 QScrollArea 的剩余高度交给 spacer，
让空态按自身 `sizeHint/minimumHeight` 呈现。`CommandBatchEmptyState` 的 state、signal、CTA、
accessible name、motion marker 和 batch bindings 不变；没有把窗口高度、业务状态或滚动策略
下沉到 component，也没有新增 timer 或固定坐标。

## 目标分层

```text
┌──────────────────────────────────────────────────────────┐
│ presentation                                           │
│ PySide6 Window / View / ViewModel / Qt signal-slot       │
└──────────────────────┬───────────────────────────────────┘
                       │ commands + immutable events
┌──────────────────────▼───────────────────────────────────┐
│ application                                             │
│ stream/datagram/GATT session managers / CommandService    │
│ RecorderCoordinator / ProfileService / ParserPipeline    │
│ ReconnectPolicy                                          │
└──────────────────────┬───────────────────────────────────┘
                       │ domain ports
┌──────────────────────▼───────────────────────────────────┐
│ domain                                                  │
│ Endpoint / TransportConfig / Ingress / Egress / Frame    │
│ SessionState / Capabilities / Error / Decoder contracts  │
└──────────────────────┬───────────────────────────────────┘
                       │ implementations
┌──────────────────────▼───────────────────────────────────┐
│ infrastructure                                         │
│ pyserial | bleak | socket/asyncio | RTT Telnet bridge   │
│ file persistence | logging | device discovery            │
└──────────────────────────────────────────────────────────┘
```

依赖方向向内：presentation 可以调用 application，application 只依赖 domain 中的端口，infrastructure 实现端口但不反向依赖界面。新增 CAN、USB HID 或 MQTT 时，只增加适配器和配置，不重写终端、记录器和解析器。

## 包结构

当前已落地的 UART 切片保持最小边界：

```text
src/serialforge/
├─ domain/
│  ├─ models.py              不依赖 Qt/设备库的值对象和 DTO
│  ├─ protocols.py           framing/checksum、source key 和 frame DTO
│  ├─ protocol_presets.py    有界内置协议预设目录和配置匹配
│  ├─ modbus.py               已分帧 Modbus RTU ADU 校验与 CRC16
│  ├─ mavlink.py              已分帧 MAVLink v1/v2 校验与 CRC_EXTRA 边界
│  ├─ components.py          profile schema、固定字段 codec 和 component row DTO
│  ├─ codecs.py              schema v2 JSON/TLV/Modbus RTU/MAVLink 配置、解析器和显式 codec router
│  ├─ transforms.py          有限 scale/offset/clamp/enum 变换链
│  ├─ datasets.py            dataset schema、typed sample 和 bounded dataset DTO
│  ├─ replay.py              严格 JSONL 回放契约、来源标记和预算快照
│  ├─ commands.py            immutable 批量命令、步骤、request 和 progress snapshot
│  ├─ events.py              不可变会话/原始字节事件
│  ├─ ports.py               Transport、Session、Event、Protocol、Component、Dataset、Replay、Batch 端口
│  └─ errors.py              领域错误和可展示错误
├─ application/
│  ├─ sessions.py            单活动会话、worker 和有界出站队列
│  ├─ tcp_server_sessions.py  TCP Server listener/multi-client worker
│  ├─ gatt_sessions.py        单设备 BLE GATT asyncio worker
│  ├─ protocols.py            bounded parser pipeline 与 source worker
│  ├─ components.py           profile store、component worker 和 event bridge
│  ├─ datasets.py             dataset config store、transform worker 和 event bridge
│  ├─ replay.py               有界历史 RX reader、时间轴、暂停/停止和 protocol ingress
│  ├─ commands.py             有界批量目录、typed write dispatcher 和可取消 enqueue worker
│  ├─ session_router.py       stream/server/GATT 会话路由
│  ├─ events.py              有界事件总线和背压计数
│  ├─ services.py            原始记录、发送历史、快捷命令和身份缓存
│  ├─ paths.py               LOCALAPPDATA 日志/记录边界
│  └─ extension_capabilities.py OTA/debug contract-only 能力 DTO 与静态 catalog
├─ infrastructure/
│  ├─ serial_transport.py    pyserial 适配器和端口发现
│  ├─ tcp_transport.py       标准库 TCP Client 适配器
│  ├─ socket_stream_transport.py TCP/RTT 可复用有界 socket stream
│  ├─ rtt_transport.py        attach-only J-Link RTT Telnet 适配器
│  ├─ tcp_server_transport.py 标准库 IPv4 TCP Server 适配器
│  ├─ udp_transport.py       标准库 UDP 单播适配器
│  ├─ ble_gatt_discovery.py   可选 Bleak 扫描适配器
│  ├─ ble_gatt_transport.py   可选 Bleak GATT 适配器
│  ├─ network_transport.py   TCP/UDP factory
│  └─ transport_factory.py   UART/网络/RTT typed routing
├─ presentation/
│  ├─ main_window.py         <=1000 行的窗口 shell、业务回调入口和生命周期
│  ├─ formatters.py          共享的纯展示标签格式化（不持有窗口或 Qt 状态）
│  ├─ property_refresh.py    语义动态属性的 QSS re-polish/update 原语（str/bool 等 Qt-compatible 值）
│  ├─ command_selection.py   typed command mode/batch selector 读取
│  ├─ command_bindings.py    frozen command-batch widget bundle 与安全 accessor
│  ├─ ble_selection.py       typed BLE write-mode selector/capability helper
│  ├─ protocol_scope.py       protocol parser/derived-source applicability queries
│  ├─ contracts.py            controller 间最小类型化回调/DTO 边界
│  ├─ controllers/
│  │  ├─ bootstrap.py         状态注入、Qt shell 组装、ViewModel signal wiring
│  │  ├─ composition.py       页面包装、协议面板接线、快捷键和焦点顺序（owner 直接调用）
│  │  ├─ workspace.py         顶栏、workspace shell、Tab、滚动页和显式 callback 接线
│  │  ├─ workspace_runtime.py Tab 可见性、派生 surface 挂起和一次性切换过渡
│  │  ├─ connection_builder.py 连接配置控件构建
│  │  │                         UART widget bundle 的唯一构造 owner
│  │  ├─ connection_runtime.py 连接快照与连接动作
│  │  │                         连接动作通过 typed callback 注入 builder
│  │  ├─ connection.py        连接状态投影、控件 gate 和 TCP Server 就绪校验
│  │  ├─ terminal.py          终端、错误栏和发送控件构建
│  │  ├─ send_context.py       发送表单上下文 projection owner
│  │  ├─ terminal_runtime.py  终端、记录、历史和预览事件投影
│  │  ├─ protocol.py          Protocol/Component/Dataset/Curve 视图构建
│  │  ├─ protocol_config.py   framing、preset、timing 和协议提交
│  │  ├─ protocol_context.py  协议编辑控件的可见摘要 projection owner
│  │  ├─ derived_data.py      派生数据状态投影和渲染调度
│  │  ├─ replay.py            历史回放动作与来源投影
│  │  ├─ commands.py          发送/批量动作、Qt payload adapter 与结果表
│  │  ├─ ble.py               BLE 扫描、通知和特征动作
│  │  ├─ lifecycle.py         主题、动效、错误和 Qt 生命周期
│  │  └─ status_surfaces.py   状态表面 source、注册与动态属性刷新
│  ├─ component preview       有界解析帧预览入口
│  ├─ curve.py                immutable Dataset 曲线投影
│  ├─ dataset_curve.py        Qt Widgets-only QPainter 曲线/空态 signal rail 渲染
│  ├─ command_batch_editor.py 有限步骤/文本/Hex/延时编辑器
│  ├─ dialog_transition.py    自定义对话框一次性入口淡入与 effect 生命周期
│  ├─ theme.py                public theme API、主题上下文和应用入口
│  ├─ theme_tokens.py         ThemeSpec 与语义 token catalog
│  ├─ theme_stylesheet_runtime.py 纯 ThemeSpec → QSS 渲染边界
│  ├─ theme_variant_shell.py  shell/状态/来源主题 override
│  ├─ theme_variant_controls.py 原生控件/popup/viewport 主题 override
│  ├─ theme_transition.py     一次性主题切换过渡与 Qt effect 生命周期
│  ├─ theme_transition_surface.py 主题切换一次性语义色 sweep overlay
│  ├─ motion_policy.py        共享 presentation 动效偏好判定（无业务状态）
│  ├─ transport_panel_transition.py 连接方式配置面板一次性 fade 与 effect 生命周期
│  ├─ workspace_tab_icons.py 主题化原生 workspace Tab glyph renderer
│  ├─ brand_mark_surface.py 资源无关的 Header 几何品牌徽记 renderer
│  ├─ action_surface.py       共享原生 CTA signal rail renderer（只消费 shell frame）
│  ├─ data_activity_surface.py 原生 QLabel 接收活动 rail（只消费有界 presentation projection/frame）
│  ├─ send_state_surface.py    原生 QLabel 发送状态 rail（只消费既有 state property/frame）
│  ├─ send_context_surface.py   原生 QLabel 发送上下文摘要/字节 rail（只消费 bounded form projection/frame）
│  ├─ protocol_config_context_surface.py  协议配置/应用状态摘要与共享 frame rail
│  ├─ component_empty_state.py  Component 空态卡片、Profile/Codec CTA 与共享 frame rail
│  ├─ command_batch_empty_state.py  批量命令空态 glyph/CTA presentation surface
│  ├─ terminal_surface.py     TerminalEmptyState 与真实数据 TerminalViewport renderer
│  ├─ workspace_route_surface.py 四节点工作区路线 beacon（只消费 Tab index/frame）
│  ├─ workspace_scroll_hint.py route strip 滚动可发现性 projection（只消费 active scrollbar）
│  ├─ workspace_context_surface.py 当前 Tab 的只读工作区上下文提示
│  ├─ bounded_text_label.py     header 长状态的有界省略与完整文案可恢复表面
│  ├─ workspace_bindings.py    frozen workspace shell widget bundle
│  ├─ connection_bindings.py    frozen UART/网络/BLE widget bundle 与安全 accessor
│  ├─ terminal_bindings.py      frozen terminal/send/history widget bundle 与安全 accessor
│  ├─ embedded_extension_panel.py 扩展工具站只读 capability card（只消费 application DTO）
│  ├─ connection_status_surface.py 连接带四节点状态路径 renderer
│  ├─ theme_palette_surface.py 主题语义色点阵 renderer
│  ├─ theme_picker_icons.py    原生主题选择器色盘 icon renderer
│  ├─ connection_preset_codec.py schema v1 preset catalog 白名单 codec
│  ├─ connection_preset_store.py preset catalog port 与 QSettings adapter
│  ├─ connection_preset_surface.py builtin/custom combo presentation surface、选中态与辅助文本上下文
│  ├─ connection_preset_context_surface.py preset identity/next-step summary 与共享 frame 装饰
│  ├─ connection_preset_editor.py custom preset 名称/备注 editor（不接触密钥/设备身份）
│  ├─ preferences.py          版本化 PresentationPreferences 与 PreferenceStore adapter
│  ├─ viewmodels.py          Qt 状态/命令适配
│  ├─ widgets.py             120Hz 目标 MotionController 与 presentation-only 几何 renderer
│  ├─ qt.py                  PySide6 可选依赖边界
│  └─ main.py                入口
├─ ota/
│  ├─ contracts.py           有界镜像描述、传输请求、进度和 OTA port
│  ├─ security/
│  │  └─ contracts.py        AES-GCM/CCM 候选、安全策略和验证结果 port
│  └─ transports/
│     ├─ xmodem/              XMODEM adapter 槽位（contract-only）
│     ├─ ymodem/              YMODEM adapter 槽位（contract-only）
│     └─ tftp/                TFTP adapter 槽位（contract-only）
├─ debug/
│  ├─ contracts.py            有界 RTT/J-Link 原始日志 port
│  ├─ rtt/                    RTT adapter 槽位（attach-only）
│  └─ jlink/                  J-Link adapter 槽位（不含 vendor SDK/DLL）
└─ composition.py            唯一的依赖组合根
```

`presentation/dialog_surface.py` 是协议确认弹窗的 presentation-only owner：它只配置主题角色、标准按钮文案和
AccessibleName/AccessibleDescription，不决定是否弹窗、不读取 ViewModel，也不持有协议或传输状态。`protocol_config.py`
仍是确认范围与 `exec()` 返回语义的 owner，QSS 只消费 `surfaceRole="confirmation"`。

`presentation/popup_surface.py` 只负责给 Qt 原生 `QComboBox.view().window()` 的顶层 popup `QFrame` 注入小范围主题样式；
它不替换 `QListView`、不接管键盘/焦点、不创建事件过滤器或 timer。`theme.py` 负责主题切换刷新，bootstrap 在主窗口组合完成后
补齐首次 popup，命令编辑器在自身 combo 构造完成后补齐对话框 popup；popup frame 的 palette 仍由 theme stylesheet runtime 提供。

`presentation/file_dialog_surface.py` 是 Qt `QFileDialog` 的 presentation-only 主题桥：统一把现有 open/save 文件选择调用
映射为 Qt 的 `ExistingFile`/`AnyFile` 与 `AcceptOpen`/`AcceptSave` 模式，配置 `DontUseNativeDialog` 以保证 QSS 在 Windows
上可见，并复用 `popup_surface` 处理文件对话框内部 ComboBox 的顶层 popup。它不保存路径、不读取 ViewModel、不写文件，
也不重做文件系统模型；`theme_stylesheet_runtime.py` 只渲染 QFileDialog 的 bounded surface/list/tree/header/button 规则。

Presentation controller 拆分遵循 ADR 0024 和 ADR 0025。`MainWindow` 不应成为所有工作区状态的共享容器，也不使用
mixin 通过隐式 `self` 传递依赖；controller 只依赖 ViewModel、自己的 Qt parent 和明确的类型化回调。
迁移期间的显式 window 委托是临时措施，最终应按 feature state/action contract 继续收窄；当前
`bootstrap.py` 只做装配，不承载 transport、协议或安全业务逻辑。

BLE 写入模式的 bounded selector/capability projection 位于 `presentation/ble_selection.py`；它只读取 Qt combo data
或 domain characteristic DTO。`ble.py` 保留会修改控件的 normalize action，connection/commands 直接消费 selector，
不得再通过 MainWindow 保留纯 BLE helper facade。

Protocol parser/derived-source applicability queries 位于 `presentation/protocol_scope.py`；它只读取当前 transport/history
presentation facts，不改变 editor、worker 或 ViewModel 状态。所有 gate owner 直接消费该模块，MainWindow 不得保留纯 scope facade。

`presentation/widgets.py:MotionController` 是装饰性 UI 的唯一共享时钟，目标 cadence 约为 120Hz，使用
PreciseTimer 的 8ms 周期调度 slot，并按 elapsed time 推进 phase；8ms 是四舍五入的调度目标，不承诺
显示器精确 120fps。`ambient_active` 只表达可见窗口的环境信号场
是否需要出帧，不承载业务状态；`lifecycle.py` 在 show 时开启，只向 visible-to-window 的 surface fan-out，
隐藏/最小化通过 `suspended` 停止，关闭时清理。低动效、显式暂停和关闭仍优先级更高，所有 renderer 必须保留静态回退，
禁止以新的控件级常驻 timer 替代该边界。

`presentation/theme_transition.py` 是主题切换一次性过渡的唯一 owner：它同时管理 `appRoot` 的短时 opacity fade 与
`theme_transition_surface.py:ThemeTransitionSurface` 的几何 sweep，并在自然完成、快速连续切换、低动效、暂停、隐藏、最小化
和关闭路径原子清理 effect、overlay 与 animation。sweep 只读取当前 `ThemeSpec` 语义 token，不进入 accessibility/focus 树，
不读取业务状态、不使用 `MotionController`、不创建常驻 timer。

`presentation/transport_panel_transition.py` 是连接方式切换时配置面板一次性 fade 的唯一 owner：
`connection_runtime.py` 仍拥有 UART/TCP/UDP/BLE/RTT 的业务显隐和控件状态，只在既有 projection 完成后传入当前可见 panel。
transition helper 只创建短时 `QGraphicsOpacityEffect/QPropertyAnimation`，复用 `motion_policy.py` 的低动效/暂停偏好，
不读取 ViewModel、不改变焦点/无障碍树、不创建常驻 timer；`lifecycle.py` 在主题切换、低动效、暂停、隐藏、最小化和关闭时
停止并清理 effect。已有 graphics effect 不会被覆盖，快速切换只保留最后一个 transition。

`presentation/dialog_transition.py` 是自定义 `CommandBatchEditorDialog` 与 `ConnectionPresetEditorDialog` 的一次性入口淡入
唯一 owner：它只消费父窗口的 `decorative_motion_enabled()` 结果，在 show boundary 创建 150ms、0.90 → 1.0、OutCubic 的
`QGraphicsOpacityEffect/QPropertyAnimation`，并在自然完成或 hide boundary 恢复 opacity、解绑 effect 和销毁 animation。已有
graphics effect 不会被覆盖；低动效、显式暂停或环境 reduced-motion 时保持静态显示。两个 dialog owner 只负责调用 show/hide hook，
不把动画状态写入 ViewModel、不改变 draft/validation/焦点/Tab/accessibility，不接触 native `QMessageBox`/`QFileDialog`，也不创建
常驻 timer 或业务状态源。

`ConnectionPresetEditorDialog:presetMetadataFields` 是自定义连接配置元数据的 presentation-only surface：它复用既有
`QWidget[role="surface"]` 主题规则包住名称/备注两个 bounded `QLineEdit`，字段标签统一使用 `role="muted"`。surface 只改变
视觉分组和次级对比度，不保存数据、不改变 `ConnectionPresetMetadata`、空名称/长度校验、按钮 action、初始焦点或 Tab 顺序；
连接 controller 仍是 DTO 生成、保存/删除和活动会话 gate 的 owner。

快速配置选择的确认反馈仍由 `connection_builder.py:_dispatch_connection_preset()` 负责在既有选择/应用 projection 完成后调用
`MotionController.request_activity(420)`；`ConnectionPresetContextSurface` 只消费共享 `frame_changed` 并绘制已有 bounded rail。
该请求不表达连接成功、不自动连接、不修改 combo value/accessible text、不创建第二个 timer；reduced-motion、暂停、隐藏、最小化和
关闭由 MotionController/lifecycle 的既有静态回退统一处理。

连接页的 UART/网络/BLE section 标题引用由 `connection_builder.py` 与对应参数 panel 一起装配到 window facade，
`connection_runtime.py:on_transport_changed()` 是标题与 panel 可见性 projection 的唯一 owner。UART、TCP Client、TCP Server、UDP、
BLE GATT、J-Link RTT 都只投影到既有三组 presentation widgets：UART 和 BLE 使用专属标题/panel，网络类共享网络标题/panel；
标题统一保留 `role="section"`，不会新增业务状态、DTO 字段、timer 或第二套 transition。既有 `transport_panel_transition.py` 仍只接收
当前 active panel，标题与 panel 在同一次切换中保持 parity，避免隐藏内容留下孤立标题行。

`workspace.py` 返回一个 presentation-owned `workspaceShell`：真实 `QTabWidget` 仍挂在 `window._workspace_tabs`，
其下方的 `workspaceRouteStrip` 只承载 `WorkspaceRouteSurface`。路线 beacon 读取 `currentChanged` 投影出的 index，
不进入焦点/无障碍顺序，不代替 Tab 文案；它使用 shell 的共享 frame，并在 980/1180 下通过布局右对齐。固定宽度装饰不应
依赖 `QTabWidget.setCornerWidget()`，因为当前 Qt style 可能把 corner geometry 放到父边界外而被裁切。

`workspace_runtime.py:request_workspace_activity()` 是可见用户导航的 presentation-only 确认入口：它只在窗口未关闭、未隐藏、未最小化时向
唯一 `MotionController` 请求 320ms activity window。`workspace.py` 将它作为 `QTabWidget.currentChanged` 的独立 signal consumer 接入，
不改变 `on_workspace_tab_changed()` 的 derived renderer suspension，也不改变 `animate_workspace_transition()` 的 180ms page fade。
首次 hydration、隐藏/最小化/关闭、暂停或 reduced-motion 由既有生命周期和 MotionController fence 保持静态；route beacon、状态 rail 等共享
frame consumer 只获得一次短暂视觉确认，不保存导航状态、不创建 timer、不进入焦点/Tab/accessibility 树。

`presentation/workspace_tab_icons.py` 是原生 workspace `QTabWidget` 的主题化 glyph owner：它只根据 `ThemeSpec` 生成四个有界
`QIcon`（链路、协议、命令、扩展）的 Normal/Selected/Disabled pixmap，并在 workspace 组装完成和主题切换后刷新。它不替换原生
Tab 文案、index、signal、焦点、键盘或 accessibility 语义，不读取 ViewModel、不保存业务状态、不创建 timer，也不引入图片
资源依赖；`workspace.py` 负责一次性组装，`lifecycle.py` 只负责主题刷新。

`controllers/protocol.py:_field_label()` 是协议/遥测配置表单的局部装配 helper：它只创建普通字段 `QLabel` 并设置
`role="muted"`，交由共享主题 stylesheet 统一控制次级对比度。协议 section 标题继续由 `role="section"` 表达，状态、hint
和动态说明继续由各自 owner 保留原有属性；该 helper 不读取 DTO/ViewModel、不接收 signal、不改变 callback、焦点、Tab 顺序、
布局列或协议状态。协议页 builder 只负责把既有字段放入 helper，业务配置与 parser 仍归 `protocol_config.py`/domain owner。

`presentation/brand_mark_surface.py` 是 Header 的 presentation-only 品牌锚点：它用固定 38×38 的 `QPainter` 几何绘制星轨/S
徽记，不加载角色、GIF、字体、SVG/PNG 或第三方资源。`workspace.py` 只把它放在现有 `SERIALFORGE` wordmark 前，
`lifecycle.py` 通过既有 `_motion_surfaces()` 将共享 `MotionController` frame/stop fan-out 给它；它不进入焦点/无障碍树，
不读取连接状态、不拥有 timer、不改变 Header 的文字、状态胶囊或主题选择器。绘制颜色始终来自父窗口的 `ThemeSpec`，因此
主题切换会通过现有 `apply_theme()` 子树刷新自然重绘，低动效/暂停/隐藏/最小化/关闭保留静态徽记。

`workspace.py:build_app_header()` 使用两层 layout-owned shell：`brand_row` 只放品牌徽记、wordmark、副标题和信号场，
`controls_row` 只放连接状态、动效偏好和主题选择器。两层布局保留现有 widget facade 名称、signal、tab order 和 lifecycle
owner，不把响应式判断散落到业务 controller；因此在 980px 最小窗口下控制器仍完整可用，1180px 宽屏下品牌层与控制层保持
清晰间距。该调整只改变几何组合，不改变连接、协议、记录、主题或动效事实。

终端空态的“打开链路连接”按钮属于 `TerminalEmptyState` 的 presentation intent，不是 transport action。
`bootstrap.py` 负责显式 signal wiring，`workspace_runtime.select_workspace_tab()` 是唯一的 Tab/焦点导航 owner；
该函数只检查关闭与索引、切换真实 `QTabWidget` 并恢复焦点，不自动连接、不改变 ViewModel/session/transport。
CTA 复用主题 `primaryButton`，只在 idle/waiting/transition 显示；paused/history 保持只读空态。按钮底部的
`TerminalActionButton` signal rail 只消费共享 `MotionController` frame 和 `ThemeSpec.accent`，不新增 timer、业务状态或
导航副作用；`TerminalEmptyState` 负责转发 `set_frame/stop`，保持单一生命周期入口。

`presentation/action_surface.py:BusyActionButton` 继续保留原生 `QPushButton` 的点击、焦点、QSS 和无障碍契约，
只接受既有异步 presentation projection（连接 opening/closing、UART discovery、BLE scan、原始 recording、批量停止和历史回放播放）的 busy 值，并从同一 `MotionController`
消费 frame 绘制底部 activity rail。它不持有 ViewModel、不启动 timer；低动效、暂停、隐藏、最小化和关闭时退回静态 busy rail，
按钮文字和真实业务语义仍由各自 controller owner 保持。

`presentation/status_footer_surface.py:StatusFooterSurface` 作为 native `QStatusBar` 的永久装饰子控件，保留状态栏原生
message 作为唯一可访问、可读的权威文案；它只接受 lifecycle 已投影的 `SessionState`、fault、RX activity 和共享 frame，
绘制 116×18 的 session/activity signal rail。它不复制状态文字、不进入键盘焦点顺序、不读取 ViewModel、不创建 timer；
低动效、暂停、隐藏、最小化和关闭时保留静态轨道，bootstrap 只负责挂载，lifecycle 负责状态投影。

`presentation/send_input_surface.py:SendInputSurface` 继续继承原生 `QLineEdit`，由 `connection.py` 投影已有
`send_band_state`（blocked/waiting/ready/busy/history），只在原生编辑结果之后绘制底部状态轨和焦点反馈。它不覆盖文本、选择、
光标、剪贴板、回车或 placeholder 语义；只消费 state 与共享 `MotionController` frame，不读取 ViewModel、不创建 timer。
`terminal.py` 只负责装配，`lifecycle.py` 负责统一 frame/stop fan-out；低动效、暂停、隐藏、最小化和关闭时保留静态轨道。

`presentation/error_surface.py:ErrorSignalSurface` 只作为 errorBar 的 28×28 鼠标透明故障 beacon；错误文字、清除按钮和可见性仍由
`lifecycle.on_error_changed()` 的既有 `ErrorInfo` 事实拥有。它只接收 active/frame/stop，不复制错误文案、不读取 ViewModel、不创建
timer；错误清除时显式静态复位，低动效、暂停、隐藏、最小化和关闭时保留静态环/叉标。

`lifecycle.on_error_changed()` 在非空 `ErrorInfo` 首次投影完成后，只对已可见窗口向唯一 `MotionController` 请求 520ms activity window，
让既有 error beacon、Header signal field 和 status rail 获得一次即时确认。该请求不改变错误模型、严重性、文字、清除按钮或焦点，也不在
初始化/隐藏窗口伪造 activity；清除错误不请求新 pulse，reduced-motion、暂停和关闭由既有 MotionController/lifecycle fence 统一静态回退。

`controllers/derived_data.py:_request_derived_activity()` 是派生输出的唯一 presentation activity 入口：只有非空且类型正确的
`ComponentFrameRow`/`DatasetSample` 快照进入已可见、未最小化、当前 Protocol workspace 的窗口时，才向唯一 `MotionController` 请求一次
360ms activity window。它不读取或复制业务计数，不改变 component/dataset/curve DTO、表格/预览/曲线内容或 renderer throttle；空快照、初始化 hydration、
后台 Protocol 页、暂停、reduced-motion、隐藏和关闭均由入口 guard 或 shared clock 静态回退。Component/Dataset worker 仍由 application owner
负责，`derived_data.py` 只负责已有快照的 UI projection。

`controllers/replay.py:_request_replay_activity()` 是历史回放状态确认的唯一入口：PLAYING 继续请求 520ms，EOF/STOPPED/ERROR 在可见、未最小化、
未关闭窗口请求一次 480ms shared activity；PAUSED/EMPTY 和隐藏 hydration 不请求。`ReplayActivityLabel` 只消费已有 `ReplayActivityProjection`，
在 `history/error` 状态画静态终态 marker，不把记录数量转成百分比或进度，也不创建局部 timer；回放状态、按钮 enable、source badge、历史数据和无障碍文案
仍由 replay controller/既有 DTO 拥有，lifecycle 继续统一 frame/stop/close fence。

`controllers/commands.py:_request_command_batch_activity()` 是批量命令结果确认的唯一入口：RUNNING 使用既有 520ms，COMPLETED/STOPPED/FAILED
在可见、未最小化、未关闭窗口请求一次 480ms shared activity。`CommandBatchSurfaceLabel` 继续只消费 bounded
`CommandBatchSurfaceProjection`，在三个终态绘制勾/横线/叉 marker；不把 accepted steps 解释成时间进度，不改变 batch snapshot、结果表、按钮、发送队列、
焦点或 accessibility，也不创建局部 timer。commands controller 保持动作/快照 owner，lifecycle 继续统一 shared frame/stop/close fence。

`presentation/analysis_status_surface.py:AnalysisStatusLabel` 继续只消费既有 `QLabel` 的 `state/source` presentation properties 与共享 frame；UI-1.83 为
`error`、`blocked`、`history` 在底部 rail 增加静态几何 marker，分别使用叉、双横栏和回退箭头辅助确认状态。marker 位置只由既有 active-node 映射决定，颜色继续来自
`ThemeSpec` 语义 token；不新增 DTO、状态源、事件总线或局部 timer，不改变文字、AccessibleDescription、objectName、QSS、焦点和 controller projection。
未知状态仍回退到 idle 的中性静态轨道，lifecycle 继续统一 frame/stop/close fence。

`controllers/composition.py:scroll_page()` 是三个设置页共享的滚动边界：UI-1.84 让 scroll content 使用横向 `Expanding` size policy，并明确关闭
`QScrollArea` 横向滚动条；纵向滚动保持 `ScrollBarAsNeeded`，`widgetResizable`、页面 objectName、layout、焦点顺序和业务内容不变。这样工作区不会把横向滚动槽误显示成
白色/亮色分隔线，也不会把滚动策略复制到各 feature controller；980/1180/1440 宽度和三套主题均通过 presentation vector 验证。

UI-1.85 将 stable QSS 模板中八处近白文字/选中色统一接入已有 `{TEXT}` 与 `{SELECTION_TEXT}` 语义 token：`theme_stylesheet_base.py` 和
`theme_stylesheet_controls.py` 只负责默认模板结构，`ThemeSpec` 与对应 variant override 继续负责主题值；不新增 token、不改变 selector、焦点、选择、Tab、表格或
accessibility 契约。这样主题切换不会因为 stable 模板残留 `#fff…` 而重新引入不一致的近白高光。

UI-1.86 延续同一边界，将 `theme_stylesheet_controls.py` 中表格、表头、状态栏、双向滚动条、corner 和 Tooltip 的稳定颜色改为已有
`BACKGROUND`、`SURFACE`、`SURFACE_INPUT`、`BORDER`、`HISTORY_*`、`NEUTRAL_BORDER` 与 interaction/selection token。稳定模板仍只描述
selector 结构和默认角色，`theme_variant_controls.py` 继续拥有三套主题的具体控件覆盖；不新增 token registry、第二份 palette 或 controller 依赖。
表头与 Tooltip 的渐变只引用语义角色，因此保留层级感而不会重新引入散落近白/紫黑字面量。

UI-1.87 将同一策略扩展到 controls stable template 的状态标签、surface/errorBar、输入/禁用态、SpinBox/ComboBox popup、Menu、按钮、checkbox、workspace Tab、terminal
和终端/批量空态。颜色只从已有 `ThemeSpec` 角色渲染，selector 与 `theme_variant_controls.py` 的覆盖 owner 不变；这样稳定回退、主题切换和后续新增调试站页面共享同一
语义层级，不需要复制一份控件 QSS 或把主题策略带入 controller。

UI-1.88 完成 base shell 的同一收敛：app root、section/error/status、连接 control band、station live/history、send/pipeline、protocol/component/dataset/replay status
以及 preset/context badge 不再携带稳定 hex 色板。稳定模板现在只表达结构、状态 selector、几何和 semantic token，shell variant 继续表达三套主题值；base/controls 的稳定 literal audit
为零，后续 OTA/debug presentation 可以直接复用同一状态角色而无需复制局部色值。

UI-1.89 将共享动效帧的最后一段生命周期责任固定在 `controllers/lifecycle.py:on_motion_frame()`：排队帧在关闭、不可见、最小化或共享动效策略关闭时不再进入任何装饰 consumer，并同步清除 data-activity presentation property。`MotionController` 继续只拥有低频时钟，`motion_policy.py` 继续拥有动效策略，业务 ViewModel/transport/recorder/parser 不感知该门禁；这保持了 presentation owner 的高内聚和向内依赖。

UI-1.90 将未来嵌入式能力的“展示”与“执行”分开：`application/extension_capabilities.py` 提供带 group/reference/state 的不可变 DTO，`presentation/embedded_extension_panel.py` 只将 XMODEM/YMODEM/TFTP、AES-GCM/CCM 和 RTT/J-Link 的当前 contract-only/attach-only 状态渲染成只读卡片。`workspace.py` 增加第四个扩展 Tab，`workspace_route_surface.py` 和 `workspace_tab_icons.py` 只扩展 bounded navigation visuals；前三个业务页、`protocol_tab_index`、ViewModel 和传输生命周期不变。

UI-1.91 继续保持 application DTO 与 presentation 的单向边界，只修正 capability card 的语义层级：`extensionCapabilityTitle` 是正文标题，不再继承通用 status pill；`extensionCapabilityState` 独立表达能力成熟度，`contract_only` 使用 info surface/border，`attach_only` 使用 history surface/border 与 purple accent。规则位于 `theme_stylesheet_base.py` 的稳定模板，复用现有 `ThemeSpec` token，三套主题由同一 QSS 语义回退覆盖；不新增 palette、状态源、动画时钟、业务动作或 OTA/debug 依赖。

UI-1.92 将 workspace Tab 的一次性淡入 effect 释放收敛到 `controllers/workspace_runtime.py`：stop 和自然完成都经过同一个 `_release_workspace_effect()`，先恢复 opacity，再从 page 解绑 effect，并使用 Qt validity 检查避免对已回收对象操作。快速切换先结束上一动画；低动效、隐藏、最小化和关闭仍由现有 `stop_workspace_transition()` 静态回退。该 owner 只处理 presentation 生命周期，不读取 ViewModel、不改变 route/index、业务状态或共享 MotionController。

UI-1.93 将 capability card 的内容语义进一步收敛：`extensionCapabilityTitle` 只作为标题标签，不再携带 `role="status"`；`extensionCapabilityState` 继续由 status role 和 capability state 属性表达 contract maturity。这样状态语义只存在于 badge，QSS 仍由 `theme_stylesheet_base.py` 按 objectName/state 负责视觉层级；application DTO、panel builder、OTA/debug owner 和只读边界均不变。

UI-1.94 增加 `presentation/uart_timing_surface.py` 作为 UART 表单的纯展示 owner：它只接收 connection builder 从现有 combo 当前值投影出的 typed enum，并渲染紧凑摘要与 accessible description。快速配置通过既有 combo signal 自动刷新；摘要不写入 ViewModel、domain config、transport 或协议 timing 状态，`connection_builder.py` 只负责组合与接线，主题样式继续由稳定 semantic QSS owner 管理。

UI-1.95 增加 `presentation/embedded_station_overview.py` 作为扩展工具站的只读信息 owner：它从已有
`application.extension_capabilities` immutable catalog 派生能力槽位数、已激活后端数和当前动作，并渲染只读规划层与目标接入前置条件。它不定义 active 状态、不打开后端、不创建动作或 timer；`embedded_extension_panel.py` 只负责把 overview 与能力卡片组合，application/OTA/debug 边界保持不变。

ARCH-7d / UI-1.96 将摘要派生进一步收回 `application/extension_station.py`：`ExtensionStationSummary` 是 frozen、bounded、
presentation-neutral DTO，`extension_station_summary()` 是 catalog 到摘要的唯一派生入口。`embedded_station_overview.py` 只消费并
渲染该 DTO，不再了解 `contract_only`/`attach_only` 的状态集合；这样未来增加 active/blocked 状态时不会把 application policy 复制到 UI，
也不会引入第二套状态源、动作或后端探测。

UI-1.135 在同一 application owner 内增加 `ExtensionStationGroupSummary`，由
`extension_station_summary()` 把能力 catalog 投影为 OTA 传输、OTA 安全和调试输出三组有界计数，
并校验分组总数与总能力数一致。`embedded_station_overview.py` 只消费 immutable 分组摘要，使用六列
metrics grid 保持首屏显示分组数量，未来超过六项时自动换行；它不重新解释 capability state、不创建后端动作或 timer，
因此扩展站的未来增长仍沿着 application DTO → presentation renderer 的单向依赖演进。

UI-1.136 将配置页的滚动可发现性收敛到 `presentation/workspace_scroll_hint.py`：
`WorkspaceScrollHint` 只观察当前 `settingsScroll` 的原生 vertical scrollbar，把 range/value
投影为顶部、中部、底部和无溢出四态，`workspace.py` 负责组装，
`workspace_runtime.py` 负责 Tab 切换时的绑定与解绑。它不创建第二套滚动策略、业务状态、导航模型或 timer，
提示位于既有 route strip，base/variant QSS 只复用已有 semantic token。

ARCH-6q 将 workspace shell 的纯 Qt wiring 收敛到 `WorkspaceShellBindings`：
`workspace.py` 是唯一组装 owner，bootstrap 只消费 `bundle.shell`，其余 workspace、lifecycle、
focus、derived 和 terminal owner 通过 `workspace_bindings_for()` 读取六个 widget 引用。
bundle 不携带 application/domain 状态或业务 callback；早期主题 hydration 在 bundle 尚未创建时
安全返回，既有 Tab、滚动提示、专注模式和共享动效生命周期保持不变。

UI-1.137 在同一 typed bundle 中增加 `WorkspaceContextLabel`，把既有四个 Tab index 投影为
“当前页 · 链路配置 / 解析与遥测 / 命令管理 / 能力预览”。`workspace.py` 负责构造，
`workspace_runtime.py` 只在既有 `currentChanged` 回调中同步 index；label 不读取业务状态、
ViewModel、transport 或扩展后端，也不创建 timer、导航动作或第二套状态源。base/variant
controls stylesheet 对称使用 ThemeSpec token，bounded route strip 保持 31px 高度。

ARCH-6r 将 UART 表单的纯 Qt wiring 收敛到 `UartControlBindings`：
`connection_builder.py` 是唯一组装 owner，`connection_bindings.py` 只保存 frozen/slots
widget 引用；连接、preset、协议 timing、生命周期、组合和终端 runtime 通过
`uart_bindings_for()` 消费，不再直接读取 UART 动态字段。bundle 不包含业务状态、DTO、
callback、timer、设备句柄或传输策略；初始化早期缺失安全返回，配置路径显式报错。builder
内部保留迁移期兼容字段，后续可沿同一边界继续收敛网络/BLE/RTT。

ARCH-6s 沿同一模式增加 `NetworkControlBindings`：它由 `connection_builder.py` 唯一组装，
把 TCP Client/Server、UDP、RTT 共用的网络端点 panel、label、timeout、allowlist 和 peer
selector 作为一个 frozen/slots wiring bundle。connection/runtime、preset、composition、
lifecycle、commands 和 terminal runtime 通过 `network_bindings_for()` 消费；
`server_target_explicit`、默认值标志和 peer snapshot 仍留在原 owner，不进入 bundle。
这样同一表单不会被拆成三套重复 accessor，同时构造细节不再跨 controller 泄漏。

ARCH-6t 沿同一模式增加 `BleControlBindings`：它由 `connection_builder.py` 唯一组装与接线，
把 BLE panel/title/hint、扫描与连接 timeout、过滤、设备、缓存、特征、通知和写入模式的
Qt 引用收敛为 frozen/slots wiring bundle。BLE selection、action、connection/runtime、preset、
composition、commands、lifecycle 和 terminal runtime 通过 `ble_bindings_for()` 消费；
`_ble_notification_pending`、`_ble_notification_ref`、`_ble_notification_timer` 及设备快照、
backend handle 和写策略仍留在原 owner，不进入 bundle。这样 Qt wiring 与真实生命周期状态
保持高内聚边界，同时不复制 BLE option catalog、timer 或业务状态。

ARCH-6u 沿同一模式增加 `TerminalControlBindings`：它由 `bootstrap.py` 在 terminal/send
 builders 完成后唯一组装，把实时观测 band、终端 viewport/empty state、记录、发送、快捷命令
 和发送历史的 Qt 引用收敛为 frozen/slots wiring bundle。terminal runtime、connection、commands、
 command selection、send context、composition、lifecycle 和 focus transition 通过
 `terminal_bindings_for()` 消费；preview buffer/pending、history/quick snapshot、recording/session
state、timer、MotionController、ViewModel 和 callbacks 仍留在原 owner，不进入 bundle。这样
高频 UI wiring 不再跨 controller 泄漏，同时保留原有发送 gate、键盘、空态和生命周期语义。

ARCH-6w 将命令管理页剩余的批量命令 Qt wiring 收敛到
`CommandBatchControlBindings`：`bootstrap.py` 在 `workspace.py` 完成命令页构建后
唯一组装 `command_bindings.py` 的 frozen/slots bundle；commands、connection、command
selection、composition 和 lifecycle 通过 `command_batch_bindings_for()` 消费。bundle
只包含 combo、批量动作按钮、状态 rail、结果表和空态，不携带 batch snapshot、batch
catalog、ViewModel、timer、callback、transport 或执行策略。`terminal.py` 仍是控件
构建与 signal 接线 owner，保留迁移期装配字段；跨 controller 不得重新读取这些动态
window 属性。这样命令批处理 UI 与终端发送/历史 bundle 分开，后续 OTA/debug station
页面可沿同一组合根边界扩展而不引入共享 MainWindow facade。

ARCH-6x / UI-1.140 将共享链路控制外壳收敛到 `ConnectionShellBindings`：
`connection_builder.py` 在创建传输选择、快速配置、上下文提示、保存/删除、状态轨道和
连接按钮后、刷新 preset combo 前唯一组装 `connection_bindings.py` 的 frozen/slots bundle。
connection、connection_runtime、connection_presets、commands、composition、derived_data、
protocol_config、replay、protocol_scope 和 lifecycle 通过 `connection_shell_bindings_for()`
消费；构建器仍可在组装期间使用临时动态字段，但跨 controller 不得重新读取它们。bundle
只包含 QWidget 引用与既有 `hint_label` 投影，不携带 preset catalog/store、session state、
ViewModel、callbacks、timer、transport handle 或策略。这样链路 UI wiring 与连接事实、预设
持久化和协议来源 gate 保持高内聚低耦合，同时保留 120Hz shared MotionController、主题、焦点、
accessibility、响应式布局和 OTA/debug 独立边界。

ARCH-6y / UI-1.141 将全局 header 的状态、动效和主题 wiring 收敛到
`HeaderChromeBindings`：`workspace.py` 在创建 status cluster、brand mark、signal field、
motion controls 和 theme controls 后、首次 `on_theme_changed()` 前唯一组装
`chrome_bindings.py` 的 frozen/slots bundle。lifecycle 与 composition 通过
`header_chrome_bindings_for()` 消费状态标签/指示器、低动效/暂停开关、主题选择器、调色板
swatch、品牌徽记和信号场；偏好、`MotionController`、主题策略、`app_root`、error bar、
status footer 和业务状态仍留在各自 owner。这样 header 装饰与 lifecycle 的 shared frame
边界清晰，120Hz、主题切换、关闭静态回退、焦点和 accessibility 语义不被动态 MainWindow
facade 重新串联。

ARCH-6z / UI-1.142 将工作区的“配置页拥挤”处理收敛到既有
`workspace_focus_transition.py` owner：`workspace_runtime.on_workspace_tab_changed()` 只消费
bounded Tab index，并把 index 0 的“链路 / 连接”保留为总览路由；协议 / 遥测、命令管理和扩展 / 工具站
路由自动进入既有 focus mode，收起实时观测、终端和发送三块下方 surface，回到 index 0 时自动恢复总览。
该同步只调用既有 `set_workspace_focus_mode()`，沿用 220ms transition、reduced-motion 静态回退、
手动 focus button 和关闭/隐藏 lifecycle，不新增 timer、splitter、业务状态或第二套布局策略；下方 surface
只是 presentation 隐藏，session、接收、记录、发送和后台 worker 继续运行。这样配置页在 980×720 的首屏
viewport 从约 79px 提升到 471px，避免把控件压成不可读的窄条，同时保留默认启动总览和主题/120Hz
shared MotionController 边界。

ARCH-70 / UI-1.143 将空终端 onboarding 的“打开链路连接”明确建模为
`workspace_runtime.open_connection_setup()` presentation 入口：它先选择 index 0，再复用既有
`set_workspace_focus_mode(True)`，让第一次需要连接的用户直接看到完整 UART/网络/BLE 表单；默认启动的
index 0 仍是总览，普通 Tab route 仍由 ARCH-6z 的同步策略管理。`bootstrap.py` 只负责把
`TerminalEmptyState.connection_requested` 接到该入口，入口不读取 session/ViewModel、不发起连接、不改变
连接 gate 或状态；focus button、reduced-motion、主题、120Hz shared frame 和关闭/隐藏 lifecycle 继续由
既有 owner 管理。这样 onboarding CTA 与配置 intent 对齐，同时不把业务动作塞进空态组件。

ARCH-71 / UI-1.144 将 workspace 导航动效收敛为“一个主过渡”：
`workspace_runtime.on_workspace_tab_changed()` 在 Tab route 改变 overview/configuration mode 时复用既有
`set_workspace_focus_mode()` 的 220ms 几何过渡；随后 `animate_workspace_transition()` 发现有效的
`_workspace_focus_transition` 时，会先清理旧 page opacity effect 并跳过新的 180ms page fade。这样同一
workspace surface 不会同时被高度动画和透明度动画拉扯；当 Tab 仍处于同一 focus mode 时，原有 180ms page
fade 保留，普通页切换仍有轻量反馈。该判断只消费既有 transition lifecycle，不新增 timer、状态、event bus、
splitter、ViewModel/application 依赖或第二个动画时钟；reduced-motion、隐藏/最小化/关闭静态回退和
MotionController 约 120Hz fan-out 继续由原 owner 管理。

进入 focus 时，下方实时观测、terminal、send 三块先由既有 focus owner 立即隐藏并清零 maximum-height，
只让 tabs 进行高度扩展；因此过渡帧不会把内部控件压缩成窄条。返回总览仍沿用原有 bounded 0→目标高度展开，
控件以 reveal 方式恢复，最终静态布局与 ARCH-6z 相同。

ARCH-72 / UI-1.145 将 header 状态摘要的“长文案不应撑布局”收敛到
`presentation/bounded_text_label.py` 的 `BoundedTextLabel` owner：它只负责单行文本的可用宽度
省略、resize/style 变化后的重新计算，以及 tooltip/accessibility 中的完整值保留；它不保存业务状态、
不读取 ViewModel、不创建 timer，也不参与主题或连接判断。`workspace.py` 只在 header 组装三个
bounded label，`HeaderChromeBindings` 继续提供 typed widget 引用，lifecycle 仍通过原有 `setText()`
路径更新状态。标签的 size hint 有界、minimum width 可收缩，因此 status cluster 不会被端点/来源/状态
长文案撑大，也不会挤压 120Hz 动效和主题控制区；980/1240、三主题、focus/reduced-motion 与
accessibility 语义由原 owner 继续负责。

ARCH-73 / UI-1.146 将主题切换的视觉 sweep 限定在 presentation shell 顶部带：
`theme_transition.py` 仍由同一个 owner 管理 root opacity effect、一次性 `QPropertyAnimation`、
overlay 清理和异常生命周期；只把 `ThemeTransitionSurface` 的高度从全 root 收敛为 bounded 的
220px shell 区域。terminal、send 和下方数据 surface 不再被斜线 veil 直接覆盖，root fade 仍让
全局 palette 变化可感知。该调整不新增 MotionController、timer、业务状态、事件总线或新主题状态，
`reduced_motion`、暂停、隐藏、最小化、关闭和快速主题切换继续复用原有 stop/finish 路径。

ARCH-74 / UI-1.147 将主题过渡的两个视觉责任显式分开：
`start_theme_transition(window, *, target, sweep_host)` 中 `target` 只负责 `appRoot` 的全局 opacity
fade，`sweep_host` 只允许是 root 子树内的 header chrome。`bootstrap.py` 是 host 的唯一组装 owner，
`lifecycle.py` 负责传递边界；`ThemeTransitionSurface` 只在 host 坐标中计算宽高，因此不会因为 workspace
扩展而触及 terminal/send。`MainWindow.resizeEvent()` 仍是薄 façade，先由 lifecycle 幂等停止一次性
theme transition，再交给 Qt 重新布局，避免 180ms 动画保存过期终点。该切片不新增 timer、MotionController、
业务状态、事件总线或外部依赖；root fade、header sweep、reduced-motion、暂停、隐藏、最小化、关闭、
快速切换和 120Hz shared MotionController 继续由各自 owner 管理。

ARCH-75 / UI-1.148 将工作区 focus 几何过渡收敛到 `workspace_focus_transition.py` 单一 owner：进入/退出
先测量当前 widget 实际高度，再在 root layout 重新计算后测量目标高度，`QPropertyAnimation` 只覆盖真实的
`maximumHeight` 区间，不再把 `_MAX_HEIGHT` 哨兵值当作动画终点。进入 focus 仍立即隐藏实时观测、terminal、
send 三块下方 surface；退出仍复用既有 reveal，Tab/page、业务状态、滚动页 contract 和 accessibility 不变。
`_apply_static_layout()` 统一清理 min/max/visible 约束，`lifecycle.resizeEvent()` 在 Qt 基类重排前停止 focus
geometry transition，避免窗口尺寸变化后继续写旧目标。该切片不新增 timer、MotionController、业务状态、
splitter、事件总线或外部依赖；reduced-motion、暂停、隐藏、最小化、关闭、快速反转和 120Hz shared clock
继续由既有 owner 管理。

ARCH-76 / UI-1.149 将“首次未连接用户看不到完整连接表单”的体验问题放在 workspace runtime 解决：
`bootstrap.py` 只在所有 terminal bindings 完成后、初始 `SessionState.CLOSED` 时启动一次 presentation-only
`_connection_onboarding`；`workspace_runtime.py` 负责幂等进入/退出并继续调用既有 `set_workspace_focus_mode()`，
`workspace_focus_transition.py` 不增加业务语义。`terminal_runtime.py` 只在传入首次 `OPEN` 时通知 leave；leave
只有当前仍在 connection Tab 才返回总览，非连接 Tab 继续保持原有自适应 focus。手动“返回总览”清除一次性标志，
连接失败/ERROR 不清除标志，以便用户继续重试。该切片不增加 shell min-height、splitter、timer、MotionController、
ViewModel/domain 状态、历史回放耦合或第二套布局策略；hidden/show、resize、reduced-motion、关闭和 accessibility
继续由既有生命周期 owner 管理。

ARCH-77 / UI-1.150 把“动效节奏不稳、协议页控件挤在一行”的问题限定在 presentation：
`widgets.py:MotionController` 仍是唯一的共享时钟，只把原 single-shot 重臂简化为一个
`PreciseTimer`、8ms 周期调度（约 120Hz target；不宣称显示器精确 120fps），phase 继续按真实
elapsed time 推进，reduced-motion、暂停、隐藏、最小化和关闭仍复用既有 lifecycle fence。协议页
由 `controllers/protocol.py` 负责语义分组的 field/action rows，`form_fields.py` 只提供标签与控件的
纯 presentation 组合 helper；ProtocolPanelWidgets、callback、itemData、Tab 顺序、外层 QScrollArea
和 application/domain contract 均保持不变。该切片不新增控件 timer、业务状态、设备 I/O、OTA/debug
依赖或第二套滚动容器；980×720/1240×820、三主题和横向溢出检查由真实组合根验证。

ARCH-6v / UI-1.138 把共享动效调度和站台布局继续留在 presentation owner：`MotionController` 使用
120Hz 目标、PreciseTimer 与 8/9ms 交替 slot，按 elapsed time 推进相位；lifecycle 只向当前可见
surface fan-out，并使用确定性小相位差避免所有 signal rail 同步拥挤。`terminal.py` 的实时观测、
发送和历史布局使用固定标题边界、明确行高和 bounded spacing；bootstrap 的默认窗口为 1240×820、
最小高度为 720。不得为单个控件新增时钟、复制业务状态、改变发送/连接语义或把响应式布局下沉到
application/domain；隐藏页、低动效、暂停、最小化和关闭仍由既有 lifecycle fence 静态收敛。

UI-1.97 保持 UART 配置的 typed value 与 presentation label 分离：`connection_builder.py` 只把数据位、校验、停止位、流控的用户文案
本地化，并显式设置波特率不可编辑；`currentData()` 仍由既有 domain enum/value 消费，快速配置只改变 combo value，不复制或转换业务配置。
五个 selector 的 tooltip/accessibility description 属于 presentation affordance，不进入 `UartTransportConfig`、session 或 transport adapter。

UI-1.98 延续同一边界到网络/BLE/RTT/TCP Server：`connection_builder.py` 只拥有字段 label、tooltip、accessibility description、
不可编辑 selector 和零值显示文案；既有端口/超时范围、`itemData()`、`BleGattWriteMode`、连接 preset、runtime controller 与 transport
adapter 不被 presentation 文案反向解释。allowlist、LAN 确认、BLE filter/cache/pair 和 RTT channel 的提示只描述既有控件事实，
不创建网络探测、vendor 工具启动、连接动作或第二个状态源。

UI-1.99 将协议/遥测页的用户文案 owner 按实际刷新链路收敛：`protocol.py` 负责 preset/selector 的 presentation label 与 affordance，
`protocol_config.py` 负责既有 framing gate 的动态提示，`derived_data.py`/`dataset_curve.py` 负责 Dataset/Curve snapshot 的动态文本，
`lifecycle.py` 只把既有状态聚合为 localized pipeline summary。各层均不改变 `ProtocolPreset`、`FramingKind`、`ChecksumKind`、Dataset field
或 CurveSnapshot contract；初始化与动态刷新复用同一中文 fallback，避免 UI state drift，也不把 domain policy 复制进 presentation。

UI-1.100 将 `PipelineSurfaceLabel` 的绘制策略收敛到 presentation surface 自己的有限状态映射：`lifecycle.py` 仍是
`state/source` 动态属性的唯一写入 owner，`pipeline_surface.py` 只校验属性、读取 `ThemeSpec` 并消费 `_motion_surfaces()` 提供的共享
`phase/animated`。节点数量、颜色、历史来源、阻塞叉标和 pulse gate 都是纯渲染映射；不新增 view model 字段、domain DTO、事件订阅、
QTimer、线程或设备 I/O。这样 Pipeline 轨道与既有 summary 状态事实保持一致，同时保留 `MainWindow`/controller、application 和
transport 的高内聚低耦合边界。

UI-1.101 将二次元视觉继续限制在既有 `SignalFieldWidget` presentation owner：星芒位置、彗尾透明度和轨道光点只由共享
`phase/animated` 驱动，颜色来自 `ThemeSpec`，不创建新的动画源或业务状态。`workspace.py` 只组装这个 widget，`lifecycle.py` 继续通过
已有 `_motion_surfaces()` fan-out 负责暂停、隐藏、最小化和关闭；因此视觉增强不会让 `MainWindow`、application、transport 或未来
OTA/debug adapter 互相耦合。

UI-1.102 将扩展站的卡片表面规则放回 presentation theme owner：`theme_stylesheet_base.py` 提供稳定 selector，
`theme_variant_shell.py` 只覆盖 `ThemeSpec` palette；`embedded_extension_panel.py` 继续只负责组装 application-owned capability DTO，
不解释 `contract_only/attach_only` policy。能力卡和概览的状态色带只是既有 dynamic property 的视觉投影，不新增状态源、action、timer、
vendor 依赖或 OTA/debug adapter；base/variant selector 对称使三主题不会回落到透明或系统原生默认表面。

UI-1.103 将工作区外壳的视觉边界继续收敛到 controls theme owner：`QFrame#workspaceShell` 与
`QFrame#workspaceRouteStrip` 在 base stylesheet 中提供稳定 surface/border/圆角与 history→info 渐变，
`theme_variant_controls.py` 只覆盖已有 `ThemeSpec` token。`workspace.py` 仍只组装真实 `QTabWidget`、
layout-owned route strip 和 `WorkspaceRouteSurface`；不把样式、路由状态或动效时钟带入 controller。这样新增工作区页
复用同一 shell contract，route 的 NoFocus/鼠标透明/空 accessibility、Tab index 与 lifecycle fan-out 都保持原有 owner。

UI-1.104 将底部 chrome 的最后一层原生边界收敛到 controls theme owner：`QStatusBar::item` 与
`QStatusBar QLabel` 只负责透明背景、零边框和既有 muted token；`StatusFooterSurface` 仍是唯一 presentation-only
几何 rail，native status text 仍是可访问的事实文本。controller/lifecycle 不读取 QSS、不拥有第二个 footer 状态源，
因此原生 Qt item 不会把系统白色分隔线重新带回三主题，也不会影响未来 OTA/debug 页面复用同一 status surface。

UI-1.105 将连接页的传输入口视觉策略收敛到 theme owner：`QComboBox#transportCombo` 在
`theme_stylesheet_base.py` 提供稳定的 info→input surface、字重和状态 selector，`theme_variant_shell.py` 只覆盖
`ThemeSpec` token。`connection_builder.py` 继续拥有 combo 的 `TransportKind` itemData、不可编辑和 signal wiring，
`TransportModeSurface` 只做装饰性 glyph；不把传输 policy、状态源或 palette 复制进 controller，六种入口共享同一可扩展
presentation contract。

UI-1.106 将主题入口的视觉策略收敛到同一 theme owner：`QComboBox#themePicker` 只消费 history/input/focus/disabled
semantic token，`workspace.py` 继续拥有三项 `ThemeSpec` itemData、主题 icon 刷新和 `on_theme_changed()` wiring；
`ThemePaletteSwatch` 仍是鼠标透明的共享 frame renderer，`theme_transition.py` 仍是一次性过渡唯一 owner。这样主题
选择器的 visual state 不会成为第二套主题状态源，也不会把应用连接/数据状态带入 presentation theme。

UI-1.107 继续沿用 connection status 的 presentation owner：`ConnectionStatusRail` 在既有共享帧分支内绘制
彗尾、外环和中心光点，只消费 `_phase`、`_animated`、`_state`、节点位置与 `ThemeSpec` 状态色；不创建第二个
时钟，也不把连接 policy 复制到自绘控件。`lifecycle.py` 仍是 stop/reduced-motion/隐藏/最小化/关闭的统一门，
`connection_status_surface.py` 不增加业务状态或 transport 依赖，因此状态 rail 的增强不会改变组合根、连接 controller
或未来 OTA/debug 页面边界。

UI-1.108 将用户选择器的 affordance 契约留在各自 presentation owner：`connection_builder.py` 负责 transport 与
UART 端口，`controllers/terminal.py` 负责终端/发送/历史/批量入口，`command_batch_editor.py` 负责编辑器内部选择器。
这些 owner 显式声明 editable、tooltip 和 accessibility 语义，但不抽取万能 registry，也不复制 itemData、枚举、signal
或业务状态；UART 端口是唯一允许手输的 combo。这样选择器说明与控件生命周期同域维护，application/domain 不承载 Qt 文案，
连接 gate、批量执行和未来 OTA/debug 页面边界保持独立。

UI-1.109 延续 owner-local affordance：`controllers/terminal.py` 负责清除错误、终端预览、历史与快捷命令动作，
`connection_builder.py` 负责自定义连接保存/删除和 UART 读写超时。新增内容只描述既有副作用、单位和生效时机，
不引入全局文案 registry、第二套动作状态、跨层 DTO 或 callback；application/domain 继续不感知 Qt accessibility 文案。
因此提示增强不会把 UI 责任、连接/发送/记录策略或未来 OTA/debug adapter 带入 `MainWindow` 或组合根。

ARCH-6n 将连接控件 affordance 的刷新回调绑定收敛到组合根：`bootstrap.py` 通过一个显式命名的
`partial(update_connection_controls, window)` 安装 `window._refresh_connection_controls`，各 owner controller 直接消费这个已绑定回调，
不再经过 `MainWindow._update_connection_controls` 纯转发 facade。该回调只在窗口 shell 完成控件组装前后按既有时序使用，仍由
`controllers/connection.py` 持有 enable/hint/busy 规则；没有新增状态源、事件总线或跨层依赖，关闭时仍由现有 lifecycle fence 管理窗口。

ARCH-6o 复用同一组合根 callback 边界：transport projection 与 protocol framing projection 分别由 `bootstrap.py` 绑定
`partial(on_transport_changed, window)` 与 `partial(on_protocol_framing_changed, window)`；`MainWindow` 不再保留这两个纯转发方法。
connection builder、preset、replay、composition 和 protocol owner 仍消费同名显式 callback，Qt signal payload、首屏 hydration、协议草稿副作用和 transport panel
生命周期不变。该切片只删除无状态 facade，不把 controller policy 或 feature DTO 搬进 bootstrap。

ARCH-6p 将剩余 feature callback wiring 按 owner 分域放入 `bootstrap.py` 的显式 `_bind_*_callbacks()`：protocol、derived、replay、BLE、terminal、lifecycle
分别绑定已有 controller 函数，组合根只负责 `partial` 接线，不解释业务状态。`MainWindow` 现在只保留构造和 Qt `closeEvent`/`hideEvent`/`showEvent`/`changeEvent`
生命周期覆写；窗口不再承载一长串同名业务转发。绑定在首个 timer/signal 使用前完成，保留既有 callback 名称、Qt payload、首屏 hydration、关闭 fence 和 owner policy，
不引入无限制 registry、`__getattr__`、mixin、第二个状态源或跨层业务依赖。

`presentation/data_activity_surface.py:DataActivitySurface` 继续继承原生 `QLabel`，先让 QLabel 绘制既有接收文案，再在底部
padding 内绘制有界的实时/历史 activity rail。它只接受 `DataActivityProjection` 的 source、最近字节和窗口字节，以及 lifecycle
投影的 active/frame/stop；不复制 RX buffer、不绘制吞吐/百分比、不读取 ViewModel、不创建 timer。`terminal_runtime.py` 仍拥有
文案和 projection 组装，`lifecycle.py` 统一 frame/stop 与 active 复位，低动效、暂停、隐藏、最小化和关闭时保留静态轨道。

`presentation/send_state_surface.py:SendStateSurface` 继续继承原生 `QLabel`，只读取 `connection.py` 已投影到 `state`
property 的 `blocked/waiting/ready/busy/history`，在原生发送状态文字之后绘制四节点状态 rail。它不读取 ViewModel、不改变发送 gate、
不复制文案、不创建 timer；`terminal.py` 只负责装配，`lifecycle.py` 负责共享 frame/stop fan-out，`connection.py` 继续作为 state owner。

`presentation/send_context_surface.py:SendContextSurface` 是发送控制带的第二个 presentation-only 观察面：
`controllers/send_context.py` 读取原生发送格式、输入文本和 CRLF checkbox，使用 domain 的
`MAX_COMMAND_PAYLOAD_BYTES` 作为 canonical 上限，生成 immutable `SendContextProjection`。surface 只展示 UTF-8/HEX、
payload/wire 字节数、CRLF 和格式/超限错误，并在原生 QLabel 文案之后绘制低对比度 byte rail；它不调用发送动作、不改变
`connection.py` 的 send gate、不读取 ViewModel、不保存 payload、不创建 QTimer。`connection.py` 仍是发送可用性 owner，
`terminal.py` 只负责装配和把 mode/CRLF 变更接入既有输入刷新回调，`lifecycle.py` 统一共享 frame/stop fan-out。
surface 鼠标透明、不可聚焦、不进入 Tab 顺序；空态/错误态保留明确文案，reduced-motion、暂停、隐藏、最小化和关闭回到静态 rail。

发送控制带的 `QLabel#sendShortcutHint` 是 presentation-only 的静态可发现性提示：它只说明既有
`Ctrl+Enter` 发送路径，不拥有快捷键注册、发送 action、发送 gate、ViewModel 状态或窗口 facade。它由
`terminal.py` 装配为 `NoFocus` 控件，不进入 Tab 顺序；`composition.py`/快捷键 owner 继续是唯一的动作注册边界，
默认 stylesheet 与每个 theme override 共同提供 keycap 的语义色和非白色 surface。提示尺寸保持 176–184 px，
避免在 980/1180 目标窗口下裁切文案，同时不新增 timer、事件过滤器或依赖。

`controllers/terminal.py:_field_label()` 是实时观测工具栏的局部字段装配 helper：它只创建“显示”普通标签并设置
`role="muted"`，让终端显示模式 combo 使用既有主题次级对比度。`实时观测` section、暂停/记录/接收状态、display mode signal、
快捷键、焦点和 Tab 顺序仍由原 owner 负责；helper 不读取 ViewModel、不接收 signal、不创建 timer，也不复制状态。

连接配置页的普通参数标签由 `controllers/connection_builder.py:_field_label()` 统一装配为
`QLabel[role="muted"]`。它只提供字段层级和主题对比度，不拥有配置值、signal、状态或焦点；UART/网络/BLE 的
section 标题继续使用 `role="section"`，hint/状态摘要继续保留各自的 `muted`/`status`/`subtle` 契约。该 helper
局部归属 connection builder，不上提为跨页面 widget 工厂，避免把连接表单语义泄漏到共享 presentation API。

`presentation/theme_picker_icons.py` 是主题选择器的 resource-free palette preview owner：它只把每个
`ThemeSpec` 的 surface/accent token 画成原生 `QComboBox` item icon。`workspace.py` 仅在 combo 条目组装后注入 icon，
原生 label、`UserRole` key、tooltip、currentIndexChanged、焦点和 Tab 顺序仍由 combo owner 负责；图标不读取当前业务状态、
不创建 timer、不进入 accessibility 树，也不需要 lifecycle 刷新，因为每个 icon 表示的是目标主题本身而非当前 session 状态。

`presentation/protocol_config_context_surface.py:ProtocolConfigContextSurface` 是协议配置面板内的第二个
presentation-only 观察面：`controllers/protocol_context.py` 只读取既有 framing/checksum/max-frame 控件和
`protocolStatus` 已投影的 `state`，生成 immutable `ProtocolConfigContextProjection`，显示“草稿/已应用”、
framing、checksum 与最大帧长。它不重新判断 `derived_source_supported()`、不应用配置、不清理 parser、
不读取 ViewModel、不保存编辑状态、不创建 timer；`protocol_config.py` 仍是协议 editor/gate/status owner，
`protocol.py` 只负责装配，`lifecycle.py` 统一共享 MotionController frame/stop。摘要以 190–520 px 有界宽度
避免窄窗口横向撑破，默认 stylesheet 与每个 theme override 覆盖 waiting/active/draft/blocked/history/idle
语义态，reduced-motion、暂停、隐藏、最小化和关闭只冻结装饰 rail。

`presentation/component_empty_state.py:ComponentEmptyStateSurface` 是 Component 表无数据时的独立
presentation-only 空态卡片：它保留 `derived_data.py` 通过 `setText()`/`text()` 投影的完整用户文案，
把首要下一步“加载 Profile / Codec”放到空态附近，并通过无参数 `load_requested` intent 复用既有
`ProtocolPanelCallbacks.on_load_component_codec`。卡片不读取 ViewModel、不判断 `derived_source_supported()`、
不解析字段、不持有 rows；`derived_data.py` 仍根据已有 visible rows 决定 table/empty 的互斥显隐，
`protocol_config.py` 只同步既有 source gate 到 CTA enabled。卡片 glyph 与 CTA 只消费共享 MotionController
frame/stop，默认与每个 theme override 覆盖 componentEmptyState/eyebrow/title/hint，避免空表头与系统白色
palette 分裂；非空时恢复原生只读表格与选择行为。

`presentation/dataset_curve.py:DatasetCurveWidget` 在保留 `CurveSnapshot`、snapshot debounce `QTimer`、
`flush()`、`shutdown()`、`set_suspended()` 和焦点环契约的前提下，为 `field_name=None`、等待 sample、无可绘制点
增加资源无关的空态画布：glyph、说明层级和共享 signal rail 均由当前 immutable snapshot 与 `ThemeSpec` 决定。
`set_frame()`/`stop()` 只影响空态装饰，不改变数据点、坐标、采样计数或曲线绘制；`lifecycle.py` 是唯一 frame/stop
fan-out owner，组件不创建第二个动效时钟。

`presentation/observation_viewport.py:ObservationViewport` 是原生 `QPlainTextEdit` 观察视口的共享装饰边界：
它只消费控件自身已经承载的文本/滚动状态、`ThemeSpec` 和 lifecycle 共享 frame，绘制主题化上下标尺、扫描信号与
Component/Dataset 的 scope 节点，不持有业务 snapshot、解析结果或本地 timer。`terminal_surface.py:TerminalViewport`
保留既有类型并作为 `scope="terminal"` 的兼容子类；`protocol.py` 使用 `scope="component"`/`"dataset"` 替换普通
预览编辑器，但保留 `QPlainTextEdit` 的只读、placeholder、AccessibleName、滚动和 controller 文本更新契约。
`lifecycle.py` 统一把三个视口纳入同一 `MotionController` fan-out；scope 只决定 presentation accent，不改变数据事实。

`presentation/connection_preset_context_surface.py:ConnectionPresetContextSurface` 是连接带内独立的 preset identity/next-step
summary surface：它只接受 `connection_preset_surface.py` 投影的既有 `ConnectionPreset`，显示内置/自定义来源、传输类型与描述，
并保留原有 `connection_hint` QLabel 作为下一步和无障碍文案 owner。它不应用 preset、不触发连接、不保存数据、不读取 ViewModel，
只消费 `ThemeSpec` 和共享 `MotionController` frame/stop 绘制底部 signal rail；`connection_builder.py` 负责组合，
`controllers/connection_presets.py` 只负责 selection projection，低动效/暂停/隐藏/最小化/关闭时静态回退。

`presentation/transport_mode_surface.py:TransportModeSurface` 是连接带内紧凑的 transport glyph：它只镜像
`_transport_combo.currentData()` 的六个有界模式值，使用 `ThemeSpec` 绘制 UART、TCP Client/Server、UDP、BLE GATT、RTT
的几何提示，并消费 lifecycle 共享 `MotionController` frame/stop。相邻 `QComboBox` 仍是唯一的键盘/无障碍选择器，header
context、connection hint、SessionState 和配置 builder 继续由原 owner 负责；glyph 不创建业务状态、连接副作用或本地 timer，
`connection_runtime.py` 只在既有 transport selection projection 后同步 mode，`lifecycle.py` 负责统一 fan-out。

批量命令的首屏引导由 `command_batch_empty_state.py:CommandBatchEmptyState` 独立拥有：它只发出无参数
`new_requested` presentation intent，`controllers/terminal.py` 负责显式接线到 `commands.py` 的既有新建 action；
它不读取 ViewModel、snapshot 或 transport。`commands.py` 在无 batch 时互斥隐藏 `CommandBatchSurfaceLabel` 和空结果表，
让 CTA 卡片完整占用有限 workspace viewport；选中 batch 后恢复 status surface/只读结果表。空态 glyph 与 CTA 都只消费
`MotionController` frame，通用 `action_surface.py:ActionRailButton` 不创建 timer，也不绕过 connection gate。

`presentation/command_batch_editor.py:_field_label()` 是批量命令编辑对话框的局部字段装配 helper：它只创建普通
`QLabel` 并设置 `role="muted"`，让既有主题 stylesheet 统一控制名称、快捷命令、当前步骤、延时和辅助说明的次级对比度。
dialog 的 `_DraftStep`、bounded text editor、校验、popup theme、按钮 action、错误/空态角色和显式 Tab 顺序仍由原 owner 负责；
该 helper 不读取 ViewModel/DTO、不接收 signal、不创建 timer，也不引入跨 controller 的共享 label 服务。

模块名称只是边界提示，不能为了凑目录把一个职责拆成大量空文件。只有存在稳定变化理由时才新增端口或子包。

本轮新增的 `ota/`、`debug/` 和 `presentation/` 是显式的变化边界：OTA 和 debug 目前只提供
contract/adapter 槽位，不进入 composition root，也不会被 UI 当成已启用能力。未来实现必须由
application worker 编排，domain/feature contract 保持 Qt/vendor/secret-free；infrastructure
适配器只实现一个明确端口。三种 OTA 协议暂定为 XMODEM、YMODEM、TFTP，选择只是产品路线上的
占位，真实 bootloader 兼容性必须按目标型号单独确认。

OTA 安全不是一个“加 AES”按钮：激活前需要通过镜像摘要、签名、目标身份、版本/anti-rollback、
nonce/密钥策略和掉电恢复检查；安全端口只接收 key reference，不接收原始密钥。RTT/J-Link 继续
沿用 M6 的 attach-only 原则，`debug/` 不能扩大成 probe 控制、内存读写、烧录或 vendor SDK 分发。

企业级架构的 owner、依赖规则、主题 token、源码行数门禁、OTA/debug 后置策略和交接入口详见
[`docs/adr/0025-enterprise-modular-architecture.md`](adr/0025-enterprise-modular-architecture.md)。

Recorder 和 RTT 的端口会在对应里程碑进入，而不是提前把未实现能力伪装成现成插件系统。BLE M3 已进入独立发现端口、GATT transport port 和会话管理器；M4a 已进入 source-aware decoder contract、bounded parser worker 和组件帧预览；M4b 增加声明式 profile、字段 worker 和表格/CSV 视图；M5a 增加独立 schema v2 JSON/TLV RX codec router，保留 v1 适配和 raw truth；M5b 在 component event 与 UI 之间增加独立 DatasetPipelinePort，使用 typed value 和有限变换链；M5c 增加独立 ReplayPipelinePort 与 `DataOrigin.HISTORICAL`，让 recorder JSONL RX 通过既有 protocol/component/dataset worker；M5d 在 presentation 增加 immutable curve projection 与 QPainter widget，不改变 Dataset/transport 端口；M5e 增加独立 `CommandBatchPort`、immutable batch DTO 和 application enqueue worker，不把批量语义塞入 transport worker；M5f 增加 domain-only `ProtocolPreset` 目录和显式 UI 应用边界，不增加协议库或传输耦合；M5g 在既有 domain decoder 中增加独立 NMEA 0183 Line checksum mode，保留 raw truth，不把字段语义塞入 framing；M5h 增加独立 `domain.modbus` 已分帧 ADU validator 和显式 component codec，字段错误阻止坏 CRC 进入 Dataset，但不承担 t1.5/t3.5 stream 分帧；M5i 增加独立 `domain.mavlink` 已分帧 v1/v2 validator、显式 CRC_EXTRA profile mapping 和 codec router 分支，不把方言生成、签名认证或 stream resync 混入 component codec；M5j 在 application event DTO 和三段 worker 之间传播 generation chain，并在 ViewModel 施加 freshness gate，不改变 transport/raw truth；M5k 在 `ProtocolIngressUnit` 与 protocol worker 内增加 parser-only replay segment boundary，不改变 source identity、component/Dataset 或 recorder；M5l 在 presentation 统一 `ErrorInfo` owner 和 local validation 路径，不增加 domain/application error bus；M6 进入 attach-only RTT Telnet 配置、适配器和标准 stream/session 复用。

Packaging G0 也保持独立于业务分层：scripts/provenance.py 只读取唯一版本源并检查构建 payload，
scripts/package.ps1 负责版本资源、PyInstaller staging、manifest/hash/archive sidecar 和变体隔离；
它不进入 domain/application/infrastructure/presentation，也不把 BLE 或 RTT vendor 依赖偷偷注入
核心组合根。core/BLE 与 onedir/onefile 的差异由构建配置表达，运行时端口边界保持不变。

M2a 会在不改变 UART 组合根的前提下增加 TCP Client 和 UDP 单播适配器。TCP Client
使用独立的 stream read result（DATA/TIMEOUT/EOF），UDP 使用独立的 datagram port；
两者共享 application 层的终端/记录服务，但不共享一个无类型的网络 transport。
TCP Server 的 acceptor 和多 peer 生命周期由独立的 M2b/M2c server manager 管理。

M2b 的 server listener 不复用 `SessionManager`；M2c 在同一独立边界上扩展为有界多 client：
`TcpServerSessionManager` 管理 listener、peer 状态、逐 peer 队列和显式目标，
`TcpServerTransportPort` 只暴露 typed accept/read/write-completion/peer-error 结果，accepted
stream 不向 presentation 暴露原始 socket。M2a 的 UART/TCP Client/UDP manager 保持不变；
组合根通过 session router 选择 server manager。

BLE 不复用 `StreamTransportPort`：`BleGattDiscoveryPort` 只返回不可变的设备广告快照；
`BleGattTransportPort` 只暴露服务快照、read、带明确 response mode 的 write 和
notify/indicate 订阅。`BleGattSessionManager` 独占一个 dedicated asyncio loop，负责
Bleak client 的打开、回调复制、操作串行化和关闭；Qt 线程只能投递 typed command 或消费
`BleGatt*Event`。特征实例身份使用 service UUID、characteristic UUID 和 handle，避免仅用
重复 UUID 误命中；MTU 与 `max_write_without_response_size` 分开显示，M3 不自动分片。

## 核心端口

### StreamTransportPort / DatagramTransportPort

Transport 负责发现端点、打开/关闭会话、接收入站单元和执行有能力约束的出站操作。UI 不能接触 pyserial、bleak 或 socket 实例。当前组合根用
`RoutingTransportFactory` 按配置类型选择适配器，`SessionManager` 只消费 typed
port/result，不创建 socket。

不同传输的语义必须保留；UART 的现有 stream 端口不能为了容纳 UDP 而退化成
`bytes` 万能接口：

| 传输 | 入站单元 | 出站操作 |
|---|---|---|
| UART/TCP Client | `StreamChunk`（读结果区分 DATA/TIMEOUT/EOF） | `StreamWrite` |
| TCP Server | `ServerReadResult`（accept/data/EOF/reject/write-completed/peer-error 分型） | `TcpServerSend(PeerId, StreamWrite)` |
| UDP | `Datagram(peer, payload)` | `DatagramSend(peer, payload)` |
| BLE | `GattNotification(characteristic, payload)` | `GattWrite(characteristic, mode, payload)` |
| RTT Telnet | `StreamChunk`（channel 固定在 endpoint metadata） | `RttDownWrite(channel, StreamWrite)` |

不要用一个无上下文的 `send(bytes)` 覆盖所有设备类型。

### DecoderPort / ProtocolPipelinePort

Decoder 只负责原始数据到 `DecodedFrame` 的转换，不拥有连接、不写文件、不操作 Qt。当前
`StreamingFrameDecoder` 支持 Raw、Line、Delimiter、Length prefix 和固定 checksum
preset；partial buffer、frame、input queue 都有上限，校验失败仍产生带状态的 frame。
`ProtocolSource` 由 session、transport、peer/peer_id、channel 和 direction 组成，
`ProtocolPipelineWorker` 为每个 source 维护独立 decoder，避免 TCP Server peer 或 BLE
characteristic 共享 partial buffer。parser 输出走可丢弃的 preview event，原始 recorder
路径不经过 parser；M4a 只把 UART/TCP Client RX 接入组件入口，其他传输保留 source-aware
扩展边界。M4b 的 `ProtocolEventBridge` 由 application event bus 观察 raw stream event，
不再由 presentation 触发 parser；`ComponentEventBridge` 进一步把协议帧送入独立字段 worker。
profile 只允许固定字段声明，component table/CSV 是有界 preview/export，不改变 raw truth。

### ComponentCodecPort / DatasetPipelinePort

`ComponentCodecPort` 只把 `DecodedFrame` 转换为 `ComponentFrameRow`，并保留 raw、display、
typed value 与错误状态。`DatasetPipelinePort` 只接受 `ComponentFramesDecodedEvent`，按自己的
`DatasetConfig` 选择字段、执行不含用户代码的 transform chain，并通过独立 worker 保留有限
sample window。Dataset worker 的 generation、输入队列和 retention 与 component worker 分开；
重配置不会把旧任务混入新配置，也不会把 dataset 失败反向传播到 parser、recorder 或 transport。

```text
ProtocolFramesDecodedEvent
          │
          ▼
ComponentPipelineWorker ──> ComponentFramesDecodedEvent ──> DatasetPipelineWorker
          │                         │                              │
          ├─> component table       └─> raw/component truth         └─> DatasetBatchEvent
          └─> codec/field error                                      └─> bounded CSV/UI preview
```

历史回放是独立的 ingress producer，不复制任何传输或解析实现：

```text
recorder JSONL
    │ strict bounded decode
    ▼
ReplayPipelineWorker ──(historical ProtocolSource)──> ProtocolPipelineWorker
                                                        │
                                                        └─> component ──> dataset
```

`ReplayRecordCodec` 只接受当前 recorder 的固定字段集合；worker 只投递 RX 的 UART/TCP
Client stream，TX、UDP、BLE、RTT 和无效记录分别跳过或计为可见错误。回放拥有独立 UUID，
`ProtocolSource.origin` 和 UI source gate 防止历史事件混入当前实时会话；时间轴使用原始
捕获时间差但用 `time.monotonic()` 调度，暂停不会推进播放时钟。回放行读取、文件字节、
记录数、protocol ingress 和下游窗口都受预算约束，背压由 replay worker 等待/停止处理，
不把失败伪装成成功。

### RecorderPort（UART/TCP/UDP/BLE）

Recorder 接收带时间、方向、session、transport、peer/channel 和原始 payload 的记录项。记录器不依赖 UI 文本控件；CSV 只是导出格式，原始记录优先保存。

当前实现使用有界队列和后台 JSONL writer；记录队列同时受记录数和 16 MiB 字节上限约束，队列满、文件达到 64 MiB 或写入失败都会产生可观察状态。UI 暂停只影响预览，不影响 worker 到记录器的原始字节路径。

## 会话生命周期

```text
Discovered -> Opening -> Open -> Closing -> Closed
                         │
                         └──────────────> Error
```

每个会话必须有：

- 一个明确的 Transport 生命周期和取消令牌；
- 一个后台 worker，负责阻塞/异步设备 I/O；
- 一个有界入站队列；
- 一个串行化出站队列；
- 一个连接身份；自动重连策略必须由后续传输专门定义，当前 UART 只做稳定身份热插拔诊断和人工恢复；
- 一个关闭后不会继续发信号或访问 Qt 对象的收尾步骤。

## Qt 与后台任务

Qt 主线程只负责控件和短状态转换。串口、BLE 扫描/通知、网络读写、RTT 轮询和文件落盘都在后台 worker 中执行，通过不可变事件回到 UI。

推荐数据流：

```text
device worker
   ├─> recorder queue      完整记录优先，失败可见
   ├─> parser queue         有界，带丢弃计数
   └─> UI batch queue       允许丢弃预览，不得阻塞设备 worker
protocol worker
   ├─> FrameDecodedEvent    有界、按 source 隔离
   └─> component preview    latest UI batch，可丢弃但有 backpressure 状态
component worker
   └─> dataset worker       typed value -> bounded transform chain -> sample window
```

终端按时间窗口或字节批次刷新，而不是每个字节刷新。暂停滚屏只暂停预览，不应默认停止原始记录。

## 配置与组合根

`composition.py` 是唯一创建具体适配器、服务和窗口的地方。应用层接收端口对象，不读取环境变量来偷偷创建全局连接。

`TransportConfig` 使用显式类型和 schema 版本，UART、BLE、TCP/UDP、RTT 分别校验自己的字段。自动重连必须匹配稳定身份；VID/PID、名称或 IP 不能单独作为设备身份。

`ProtocolConfig` 同样有 schema 版本、最大帧长、delimiter、length byteorder 和 checksum
byteorder；它不承载 component codec。M4a/M5a 不支持任意 Python、表达式、动态 codec、
自动 TX 编码或自动迁移。schema v1 profile 保持原形状；schema v2 由 `domain.codecs`
严格加载，内置 JSON/TLV 通过显式 router 选择。协议配置/重置只清理 parser partial state，
component codec 配置只清理派生 preview，不触碰 transport 生命周期。

网络配置也必须保持显式类型：TCP Client 配置远端 host/port 与有限 timeout；TCP Server
配置 IPv4 literal bind、1–16 个 client、allowlist 和有限 read/write timeout；UDP
配置本地 bind、固定远端 peer 与 payload 上限。DNS、连接失败、EOF、非法 peer 和
超限 datagram 都通过结构化错误回到 presentation；presentation 不直接导入
`socket`。

BLE 配置只接受明确的 device id/name、连接超时、配对和 service filter；扫描是用户发起的
有界任务。Windows 后端要求 `ble` optional extra，主线 UART/network EXE 不收集 Bleak；
BLE-enabled EXE 通过 `scripts/package.ps1 -Ble` 显式收集 Bleak/WinRT。

Component codec 只消费 `DecodedFrame`，不访问 transport、Qt、文件或发送队列。旧
`ComponentProfile` schema v1 继续使用固定字段 codec；schema v2 把 JSON/TLV 的字段绑定和
方言设置作为不可变配置整体切换，worker 只在锁内快照配置并在解码后检查 generation。字段、
路径、JSON 节点、TLV 项、队列和显示结果均有上限；坏 payload 只影响派生 row/error。

RTT 配置只接受明确的 Telnet host/port、channel 0/1 和有限超时；它不接受 probe selector、
芯片/内存范围、command file 或 DLL 路径。`RttTransport` 在 socket 建立后的短窗口发送
`$$SEGGER_TELNET_ConfigStr=RTTCh;<channel>$$`，随后只处理原始 bytes。外部 J-Link 工具和活动
debug session 的生命周期不归 SerialForge 所有。

## UI-1.118 scrollbar signal rail owner 边界

`theme_stylesheet_controls.py` 与 `theme_variant_controls.py` 是 scrollbar presentation 的唯一样式
owner。stable QSS 为 vertical/horizontal thumb 提供可见的 `BORDER_STRONG` 默认色、`ACCENT_PINK`
hover 和 `ACCENT` pressed；每个 ThemeSpec override 对称映射为 `accent_purple`、`accent_pink` 和
`accent`，并沿用既有 surface border。QScrollArea 的箭头隐藏、滚动范围、viewport、焦点/键盘和
QPlainTextEdit/terminal 原生行为不由该改动解释或接管。

该切片不新增控件、timer、MotionController、状态源、DTO、callback、线程、I/O 或业务判断；scroll
thumb 只是 native Qt 状态的主题化投影。`presentation/controllers/composition.py` 仍负责
`settingsScroll` 的布局策略，滚动策略不搬入 QSS 之外的 owner。三主题必须覆盖 default/hover/pressed
和 disabled/无滚动静态边界，不得产生白色或透明系统回退。

## UI-1.119 motion fanout 性能边界（实验撤回）

共享 `MotionController.frame_changed` 仍由 `controllers/lifecycle.py:on_motion_frame()` 统一向
显式列出的 presentation surface 广播。UI-1.119 先尝试逐帧 `isVisible()` 过滤，再尝试由
workspace runtime 在 Tab 切换时缓存“全局 surface + 当前页 surface”；后者的低层 dispatch 次数
确实从 56 降到协议页 39，但同一 offscreen 合成根含 `processEvents()` 的端到端测量为全量
`781.29ms/120 帧`、缓存 `821.50ms/120 帧`，因此两种实现均已撤回。

该结论保留了高内聚边界：workspace runtime 不拥有动效 surface registry，lifecycle 不读取页面
业务状态，MotionController 不承载业务进度；生产代码继续使用简单的全量 `frame/stop` contract。
未来性能切片必须先证明用户端瓶颈，再选择可回滚的最小改动；不得仅凭减少调用计数就新增全局
registry、逐帧可见性查询、常驻 timer 或第二套动效时钟。完整证据见 `docs/PERFORMANCE.md` 与
`docs/handoffs/2026-08-11-ui-1-119-motion-fanout-cache-experiment.md`。

## UI-1.120 扩展能力卡 affordance owner 边界（历史基线，已由 ARCH-83 supersede）

`embedded_extension_panel.py` 仍是扩展工具站卡片的组装 owner；UI-1.120 时能力卡是不可操作的
`QFrame#extensionCapabilityCard`，只渲染 application-owned capability DTO 的标题、摘要、边界和
`contract_only/attach_only` 状态，不进入 Tab 焦点链。ARCH-83 已将这条历史 affordance 限制替换为
整卡 `QPushButton` 语义，但仍只做 presentation selection，不添加业务动作。
两列 `QGridLayout` 显式使用等权 column stretch，保证 980/1180 宽度下卡片保持对称，不把最长文案
传播为固定宽度。

`theme_stylesheet_base.py` 与 `theme_variant_shell.py` 是该卡片 hover 的唯一样式 owner。普通卡使用
`accent_blue` 边界提示，contract-only 保持 info/blue 语义，attach-only 保持 history/purple 语义；
hover 不改变背景语义、文本、DTO、无障碍名称/描述或卡片可聚焦性。页面一次性淡入仍由
`workspace_runtime.py` 的既有 transition owner 管理，不新增 timer、MotionController、状态源、
callback、线程、I/O 或 OTA/J-Link 行为。

## UI-1.125 链路连接状态轨道终态 owner 边界

`presentation/connection_status_surface.py:ConnectionStatusRail` 继续是链路连接轨道的唯一
presentation owner。它只读取 lifecycle 已投影的 `SessionState.value`、共享 `(phase, animated)`
和当前 ThemeSpec：`open` 在最后一个节点绘制完成勾，`error` 在第一个节点绘制叉号，opening/open/
closing 的移动彗尾保持既有共享帧逻辑，closed/discovered 维持原有节点路径。

终态 marker 是纯几何 affordance，不创建状态源、不解释 transport、不触碰 controller、ViewModel、
连接动作或 I/O；`stop()`、reduced-motion、暂停、隐藏、最小化、关闭和 NoFocus/鼠标透明契约不变。
marker 使用 success/error/surface_input 既有语义 token，三主题与 980/1180 响应式边界继续禁止
白色系统回退。

## UI-1.126 响应式工作区 viewport owner 边界

`presentation/controllers/workspace.py:build_workspace_tabs()` 继续是工作区外壳与路线条的唯一
组装 owner。`QTabWidget` 不再声明与最小窗口冲突的 220px 硬最小高度：在 980×680 下，外层
`workspaceShell` 只有约 144px 可用空间时，tab viewport 会收缩到 route strip 之前的可用高度，
每个页面既有的 `QScrollArea#settingsScroll` 负责承载其余纵向内容。

该边界修复的是 Qt layout 的 presentation 几何，不创建 responsive 业务状态、resize timer、
controller facade 或第二套滚动策略；`workspaceRouteStrip` 仍为固定 31px 的装饰路线条，
terminal/send/live surfaces、Tab index、焦点顺序、主题 transition、MotionController 和
连接/协议状态不变。1180×780 仍保持原有可用高度，980/1180 两个尺寸都必须无 tab/route overlap、
无横向滚动和无系统白色回退。

## UI-1.128 连接 section 标题几何 owner 边界

`presentation/controllers/connection_builder.py:_section_label()` 是连接页 UART、网络和 BLE
section 标题的唯一构造 helper。它只设置既有 `role="section"` 语义和横向可扩展、纵向固定的
presentation size policy，避免最小窗口或专注模式的剩余高度被标题吸收，造成标题下方的视觉空白。
该修复属于 connection builder 的组装边界，不进入 QSS、连接 runtime、transport DTO 或共享
layout policy；标题文字、焦点/无障碍语义、主题 token 和 panel 可见性保持原 owner 契约。

验收固定覆盖三主题、980/1180、UART/TCP Client/TCP Server/UDP/BLE/RTT 六种连接方式，标题
高度必须保持约 26px，且不得与 active panel 重叠、产生横向滚动或白色系统回退。

## UI-1.134 UART selector 响应式 width owner 边界

`presentation/controllers/connection_builder.py:_configure_bounded_combo()` 是 UART
selector presentation width 的唯一 owner。UI-1.134 只调整 port/data/parity/stop/flow
combo 的 min/max size policy，解决八列 UART grid 在 980px 下最小宽度超过 viewport 的问题；
波特率边界沿用已有 bounded policy。该 helper 不读取 transport/session，不改变 itemData、
signals、editable 语义、连接 gate、DTO 或真实选项。

下拉闭合文本允许由 Qt elide，完整选择继续由 popup、tooltip 与 accessibility 提供；
980/1180、六传输、三主题的 horizontal maximum 与白色回退是固定验收项。动态 resize
状态、第二套响应式布局、业务 timer 和 OTA/debug 依赖均不进入 builder。

## UI-1.133 扩展工具站接入路线与 typed panel bundle owner 边界

`presentation/embedded_station_overview.py` 继续是扩展站接入概览的 presentation
owner，并同时拥有 `EmbeddedStationRouteGlyph` 的几何绘制、`set_frame()`/`stop()`
转发和静态回退。glyph 只消费 `ThemeSpec` 与共享 `MotionController` frame，不读取
capability policy、后端激活事实、密钥或设备状态。

`presentation/embedded_extension_panel.py` 通过 frozen `ExtensionPanelWidgets` 暴露
layout 与 `EmbeddedStationOverview` 两个组装结果；`controllers/workspace.py` 只负责
把 overview 引用交给窗口，`controllers/lifecycle.py` 只负责把它接入既有
`_motion_surfaces()`。该边界避免 `findChild()` 字符串搜索和页面对 lifecycle 的反向
依赖，同时保持扩展页仍是只读规划层。

三主题、980/1180、无横向滚动、静态回退和 accessibility 属于本 owner 的固定验收项；
OTA XMODEM/YMODEM/TFTP、AES-GCM/CCM、RTT/J-Link 的 contract-only/attach-only 语义
与 application DTO 不变。

## UI-1.132 批量命令空态 copy 几何 owner 边界

`presentation/command_batch_empty_state.py:CommandBatchEmptyState` 继续拥有命令空态卡片的
组装、copy、glyph、CTA 和 presentation-only signal。卡片外壳保持 `Expanding/Preferred`，以
承载专注模式的可用空间；只有 `_eyebrow`、`_title`、`_hint` 三个 copy QLabel 在 owner 内使用
纵向 `Fixed`，不再让文本控件吸收卡片剩余高度。

`CommandBatchEmptyState` 不读取命令目录或执行状态，只消费 controller 投影的 title/hint，
继续复用 `ActionRailButton` 与共享 MotionController frame。焦点、CTA、`new_requested`、
无障碍描述、主题 token、长 hint word-wrap 和命令 DTO 不变；三主题、980/1180、空态/长文案、
无白色回退和无横向滚动是固定验收项。

## UI-1.131 网络与 RTT responsive hint 几何 owner 边界

`controllers/connection_builder.py:_configure_responsive_hint()` 是 TCP Client、TCP Server、
UDP 和 RTT 网络说明 hint 的共享 presentation 组装策略。它只负责 word-wrap、有限横向宽度、
tooltip/accessibility 和纵向 `Fixed` size policy；说明文案由各传输 owner 提供，helper 不读取
transport state、不创建状态源，也不改变 panel 的连接 gate。

横向 `Ignored` 允许 grid 在 980/1180 下提供可用宽度，纵向固定到当前文案 size hint，避免稀疏
网络行吸收剩余高度。TCP Server allowlist、BLE/UART panel、主题 token、focus transition、
Tab/焦点链、连接 runtime 和 DTO 保持原 owner 契约；三主题、六传输、两种尺寸的无白色回退、
无横向滚动和 title/panel 几何闭合是固定验收项。

## UI-1.130 UART 参数摘要 rail 几何 owner 边界

`presentation/uart_timing_surface.py:UartTimingSummarySurface` 是 UART 参数只读摘要 rail 的
唯一几何 owner。它继续消费 builder 投影的 baud/data/parity/stop/flow 选择，只把纵向 size policy
固定到自身 size hint（当前真实组合根为 38px）；横向保持 `Ignored`，以适配 980/1180 的可用宽度。
该 owner 不读取 transport、session 或设备状态，不拥有连接动作，不通过 QSS 或 resize handler
修复父级布局。

builder 只负责把摘要放入 UART grid，`connection.py`/`connection_runtime.py` 与连接 DTO 不变。
摘要文案、tooltip、accessible description、主题 role 和现有 selector signal 保持原契约；三主题、
980/1180、专注模式稳定布局、horizontal maximum=0 和 exact-white=0 是固定验收项。

## UI-1.129 UART 空端口 affordance owner 边界

`presentation/controllers/connection_builder.py` 继续是 UART 端口 selector 的唯一 presentation
组装 owner。空端口时由同一处定义语义 placeholder，并同时投影到 `QComboBox` 与其内部
`QLineEdit`；这样既修复 Qt editable combo 的局部可见性，又不把端口枚举状态、endpoint DTO 或
连接动作引入主题层。`connection.py`/`connection_runtime.py` 仍只读取 `currentText()`，所以
placeholder 不会成为真实端口值，也不会自动连接。

端口 combo 仍是唯一允许手动输入的 selector；刷新按钮、焦点链、accessible name/description、
主题 token 和现有可枚举列表保持原 owner 契约。该切片不新增状态源、timer、线程、I/O、QSS
分支或全局提示面板；后续要扩展设备发现 empty state，应沿同一 builder→runtime 边界继续拆分。
三主题、980/1180、手输/清空和 horizontal maximum=0、exact-white=0 是固定验收项。

## UI-1.127 专注设置模式 owner 边界

`presentation/workspace_focus_transition.py` 是专注设置模式的唯一状态/过渡 owner，
`controllers/workspace.py` 只负责创建 `QToolButton#workspaceFocusButton`、把它放入路线条并接线。
模式是 window 上的 presentation-only 布尔值，默认总览、不写入 `QSettings`；按钮文案和
accessible description 明确表达“临时收起显示区，不会断开连接或停止记录”。

进入专注模式时，owner 只对既有 `liveObservationBand`、`terminalSurface`、`sendControlBand`
的 `maximumHeight` 运行一次 `QParallelAnimationGroup`，结束后隐藏三者并放开 `workspaceTabs` 的
350px 上限；退出时反向执行，结束后恢复三块 surface 和原上限。隐藏、最小化、关闭、暂停或
reduced-motion 会停止 group 并直接套用当前模式的静态目标，避免悬挂 effect/animation。

该模式不停止 session worker、terminal ingestion、recording、pipeline 或设备连接；只改变可见
布局。它不新增业务状态、持久化偏好、QTimer、滚动策略或 controller facade，按钮仍在焦点链中，
主题颜色由 base/variant semantic token 对称覆盖。专注/总览、三主题和 980/1180 均必须保持
无白色回退、无横向滚动和 route/tab 几何闭合。

## UI-1.124 连接快速配置上下文主题 owner 边界

`presentation/connection_preset_context_surface.py:ConnectionPresetContextSurface` 继续是连接页
快速配置上下文的唯一 presentation owner。它只消费既有 `ConnectionPreset | None` 投影：空值映射为
`source="none"` / `state="empty"`，内置 key 映射为 `source="builtin"`，用户 key 映射为
`source="custom"`，不创建新的连接状态源，也不调用 apply/connect。

`property_refresh.py:refresh_dynamic_property()` 负责 source/state 变化后的 Qt re-polish/update；
surface 自绘只选择已有 ThemeSpec 的 neutral/info/history surface、border 与 accent token。base 与
variant stylesheet 维护同一组语义 selector，标题、提示和“只填表单、不自动连接”的 accessible
description 仍由当前 owner 保持。controller、preset DTO/catalog、连接 gate、焦点/Tab、共享 frame/
stop、线程、I/O、OTA/AES/RTT/J-Link 边界均不变；980/1180 响应式和三主题无白色回退继续是固定验收项。

## UI-1.123 Component 空态 gate affordance owner 边界

`presentation/component_empty_state.py:ComponentEmptyStateSurface` 继续是 Component 空态卡片的
唯一 presentation owner。它不读取 ViewModel 或重新计算 `derived_source_supported()`，只消费
`protocol_config.py:set_derived_controls_enabled()` 已经传入的 `enabled` gate：`True` 投影为
`state="waiting"` / `COMPONENT / WAITING`，`False` 投影为 `state="blocked"` /
`COMPONENT / BLOCKED`。`setText()`、`text()`、完整用户文案和无参数 `load_requested` intent 不变。

`property_refresh.py:refresh_dynamic_property()` 只负责 state property 的 Qt re-polish/update；
`theme_stylesheet_base.py` 与 `theme_variant_shell.py` 对称提供 blocked 的 neutral surface/warning
边界。CTA enabled/visible、glyph available、共享 MotionController frame/stop、焦点与无障碍树
仍由原有 owner/lifecycle 契约负责，不新增 controller 分支、timer、状态源、线程、I/O 或 OTA/
AES/RTT/J-Link 依赖；980/1180 响应式和三主题非白色回退继续是固定验收项。

## UI-1.122 分析状态 marker owner 边界

`presentation/analysis_status_surface.py:AnalysisStatusLabel` 是协议、Component、Dataset、Curve
和 Replay 状态标签的共享 presentation owner。它先让原生 `QLabel` 完成文字、QSS、无障碍和
布局绘制，再从自身已有的 `state`/`source` property 解析 marker 颜色；`history` 仍使用既有
purple source 语义，active/waiting/draft/empty/error/blocked/idle 仍使用既有 `_state_color()`
映射。移动态低对比度 pulse 只消费 lifecycle fan-out 的 `(phase, animated)`。

五个 controller、`StatusSurfaceController` 的 state/source 投影、pipeline generation、统计文本、
结果表和 protocol/component/Dataset/replay 业务 owner 均不变。base stylesheet 只为共享 label
增加 marker 所需左 padding/字重，variant stylesheet 继续提供主题色，不把状态判断下沉到 QSS。
surface 仍 NoFocus、鼠标透明、无本地 timer；`stop()`、reduced-motion、暂停、隐藏、最小化和关闭
继续保留静态五节点 rail。该切片不连接 OTA、AES、RTT、J-Link 或设备 I/O。

## UI-1.121 批量命令状态 affordance owner 边界

`presentation/command_batch_surface.py:CommandBatchSurfaceLabel` 继续是批量命令状态 rail 的唯一
绘制 owner。它只消费 `controllers/commands.py` 从既有 `CommandBatchSnapshot` 生成的
`CommandBatchSurfaceProjection`，保留状态栏文字、步骤轨道和 `empty/ready/running/completed/
stopped/failed` 投影；新增的左侧状态点与 running 呼吸环只使用 `ThemeSpec` 语义色和 lifecycle
已经广播的 `(phase, animated)`，不读取 ViewModel、设备 ACK、真实吞吐或业务进度。

`controllers/terminal.py`、`controllers/commands.py`、`lifecycle.py` 的 owner、signal、结果表、
批量执行 gate 和 shared MotionController fan-out 不变。`theme_stylesheet_base.py` 只为该 label
预留状态点的左侧 padding，`theme_variant_shell.py` 继续覆盖三主题状态色；QSS 不承载状态判断。
surface 仍 NoFocus、鼠标透明、无本地 timer，`stop()`/reduced-motion/隐藏/最小化/关闭继续回到
静态 rail。该切片不连接 OTA、AES、RTT、J-Link 或任何设备 I/O。

## UI-1.117 字体运行时适配 owner 边界

`presentation/font_runtime.py` 是唯一的字体发现与选择边界。`presentation/main.py` 在创建
`QApplication` 后、组合 application context 前调用 `configure_application_font()`；该函数只读取
Qt 当前字体族，必要时在 Qt 字体数据库没有 CJK family 的情况下检查标准本机字体路径并通过
`QFontDatabase.addApplicationFont()` 注册，不复制、写入或打包字体资产。成功时只向 QApplication
设置一个可用字体族；找不到字体时返回 `None` 并保留 Qt 默认，不阻断启动。

`qt.py` 仅暴露 `QFont`/`QFontDatabase` 作为 presentation Qt 边界，theme stylesheet 只声明有序
fallback。该模块不读取 ViewModel/DTO，不创建 timer、线程、设备 I/O、持久化、网络、业务状态或
全局字体 registry；MainWindow、controller、domain/application 和 OTA/debug contract-only/attach-only
边界不感知字体实现。系统字体优先，离屏/受限 Qt 环境的本机字体注册只为渲染可用性服务。

## UI-1.116 主窗口 action signal rail owner 边界

连接、协议、回放、错误、终端、发送、快捷命令和批量动作继续由各自 controller builder 创建，但统一复用 `presentation/action_surface.py:ActionRailButton`；该类只在 native `QPushButton` paint 后绘制主题 signal rail，不改变 `clicked`、checked/payload、objectName、enabled gate、焦点或 Tab 顺序。`BusyActionButton` 仍由异步 busy owner 使用，未被普通 action rail 替代。

`controllers/lifecycle.py` 通过显式名称收集 16 个新增 action surface，并沿用已有 shared frame/stop fan-out。动作 owner 不读取 MotionController，lifecycle 不解释业务状态，`ActionRailButton` 不创建 timer、registry、DTO、线程或 I/O；`MainWindow` 仍只承担组合 shell。该切片是 presentation-only 复用，不把“动作可用”与“装饰正在动画”混成同一状态。

## UI-1.115 工作区 Tab signal underline owner 边界

`presentation/workspace_tab_surface.py:AnimatedWorkspaceTabBar` 继承原生 `QTabBar`，只在 native `paintEvent` 完成后为当前选中 Tab 叠加主题 signal underline/halo。选中索引、键盘导航、焦点、`currentChanged`、Tab icon、滚动按钮与 QSS 仍由 Qt `QTabWidget/QTabBar` 保持；该 surface 不创建导航信号、不读取 ViewModel，也不拥有业务状态。

`controllers/workspace.py` 是唯一组装 owner，通过 `setTabBar()` 显式注入该 widget；`controllers/lifecycle.py` 仅将它加入既有 `_motion_surfaces()`，统一消费 `set_frame()`/`stop()`。没有新增 timer、MotionController、registry、跨层 callback、线程或 I/O；低动效、暂停、隐藏、最小化和关闭继续由同一 lifecycle policy 收敛。

## UI-1.114 派生动作 signal rail owner 边界

四个既有 Profile/Codec、组件 CSV、Dataset 配置和 Dataset CSV 动作由 `controllers/protocol.py` 创建为原生 `ActionRailButton`，继续注入原有 callback 和 accessibility 语义。`action_surface.py` 只拥有 signal rail 的绘制与 `set_frame()`/`stop()` 动效契约，不读取 ViewModel、协议状态或业务 gate；`controllers/protocol_config.py` 继续拥有 derived source enabled/disabled 投影。`controllers/lifecycle.py` 只把这四个 widget 纳入已有 `_motion_surfaces()` fan-out，在隐藏、最小化、暂停、低动效和关闭边界调用既有 stop 规则。

该切片不创建新的 timer、MotionController、状态源、动作 registry、DTO、线程或 I/O。`MainWindow` 仍是组合 shell，组合根与既有 callback、source gate、主题 token 和静态回退保持不变；未来扩展动作必须先复用该 presentation contract，并由 owner module 显式注册生命周期成员。

## UI-1.113 派生动作 affordance owner 边界

协议/组件/Dataset 的 Profile/Codec 加载与 CSV 导出动作继续由 protocol.py 创建，由 protocol_config.py 在既有 derived source gate 中投影 enabled、tooltip 和 accessible description。启用态说明实际动作及不影响范围；禁用态只追加既有来源原因。该边界不引入 registry、第二套状态或 application/domain Qt 文案。

## UI-1.110～UI-1.112 affordance owner 边界

回放、连接、录制、UART、发送、批量编辑、preset、确认对话框和快捷命令 QAction 的说明继续归属于各自 presentation owner。静态控件在构建 owner 设置基础 tooltip/accessibility；动态状态由既有 controller projection 同步，不建立全局文案 registry、第二套动作状态或跨层 Qt 文案服务。

发送输入、格式选择、CRLF payload 修饰和实际发送按钮保持四个明确职责；UART 字节间超时的 0 仍表示不设定。对话框保存/取消/确认与 QAction 填入只解释既有 DTO 或界面范围，不启动连接、发送、执行、OTA 或 J-Link 行为。该边界保持 MainWindow 为 composition shell，application/domain 不感知 Qt affordance 文案。

## 错误与安全

错误至少区分：发现、权限/驱动、打开、参数、超时、断开、身份变化、peer 未授权、解析、记录、背压和用户取消。UI 只展示可行动的摘要，诊断日志保留结构化原因。

TCP Server 默认回环监听；LAN 监听需要显式配置最大 client 数和 allowlist。每个 accepted
connection 使用独立 `PeerId`，`PeerAddress` 只用于显示、allowlist 和审计；旧连接的目标
不能转投重连连接。server worker 独占 socket 关闭权，accepted socket non-blocking，partial
write 只在完整写入后发布 TX。设备 profile 不执行任意 Python、命令或 DLL。J-Link RTT
首版只允许受限的 RTT 读取和显式 Down channel，不暴露通用内存读写、halt、run、reset 或 flash。
