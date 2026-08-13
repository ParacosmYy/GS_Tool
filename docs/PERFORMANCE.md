# SerialForge 性能实验记录

本文件只记录已经运行过的性能假设，避免把没有收益的优化再次引入。测量均为 Windows
offscreen Qt 的短时合成根实验，不代表真实硬件、HIDPI 或发行包的正式性能验收。

## ARCH-128 / UI-1.201 协议页与命令页外层宽度审计

真实组合根离屏脚本覆盖三主题 `star_trail`、`moonlit_ocean`、`sakura_night`，宽度
`546/547/560/600/640/768/900/1120/1240`，连接/协议/命令/扩展四页共 108 行。所有页面的
`horizontalScrollBar().maximum()` 均为 `0`，且 content width 不超过 viewport width。协议页进一步将 component table、component preview、
Dataset preview 与 curve 同时设为可见，额外 27 行仍为 `hmax=0` 且无越界。数据叶子的内部滚动没有被共享页面滚动策略替代。

本轮没有新增 timer、QPropertyAnimation、线程、scroll owner 或动效帧源；既有唯一共享 MotionController 的 `TARGET_HZ=120` contract 保持。
Qt offscreen 的字体目录 fallback warning 不影响本次几何结果；这些数字不外推为真实 Windows/HIDPI、显示器 120fps、EXE startup、硬件链路或正式
发行验收。独立代码审查最终 `APPROVE`（Critical/Required=0，2 条 advisory），简化评估无必须项；架构师明确不因 advisory 预防性扩大源码范围。

完整窗口动效用事件循环内部的 `QTimer.singleShot(2100, finish)` 采样，避免把 `app.exec()` 返回后的 close/hide 停表阶段混入统计：
`timer_active=True`、`suspended=False`、`ambient=True`、`rearm_required=False`、`visible=True`，共 `249 frames / 119.063Hz`。
孤立 `MotionController` 同环境约 `119.775Hz`。这证明当前是唯一共享时钟的 scheduler/frame cadence 证据，不是显示器/compositor 的精确 FPS 保证；
没有新增 timer、fan-out 或 per-widget clock，也没有修改 `lifecycle_motion.py`。

本轮 `local-arch-128` onefile provenance verify 通过；canonical/root/root-latest 为 `48,079,010` bytes，SHA-256
`720131D96530341D0FFD9A271588A2BEDE1FF9829C9D3DE95D230E1DE5FE3688`，archive listing SHA-256
`489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`。签名 `NotSigned`、`release_eligible=false`、
`hardware_acceptance=not_run`。

## ARCH-127 / UI-1.200 UART 字段列级响应式审计

`ResponsiveUartForm` 在三主题与 `546/547/560/600/640/768/900/1120/1240px` 下按字段 hard/preferred contract 选择
`NARROW/COMPACT/REGULAR`，字段宽度不低于 `minimumSizeHint()`，无 sibling overlap；FontChange、StyleChange、LayoutRequest 与往返 resize
均保持稳定。该切片未改变 MotionController，仍由唯一共享 `TARGET_HZ=120` scheduler 提供动效帧源。

## ARCH-126 / UI-1.199 Header 控制区响应式几何与可读性

真实 Qt offscreen 组合根覆盖三主题 `star_trail`、`moonlit_ocean`、`sakura_night` 和宽度
`546/547/560/640/768/900/1120/1180/1240`。Header owner 按实际 contract 进入窄/紧凑/宽屏三态：546–560 为三行，640–900 为 status 独占一行且
动效/主题同排，1120–1240 为三组同排；`900→546→900` 往返无残留 stretch。三条状态 visible text 在所有宽度和主题下均无省略号，三组 QFrame
parent identity 保持为同一 owner；四个 workspace 页面 horizontal maximum 均为 `0`。

FontChange、StyleChange、LayoutRequest、Polish、resize、主题切换、焦点保持、hide/show/close 均通过；本轮没有新增 timer、线程、scroll owner 或
动效帧源，唯一共享 MotionController 的 `TARGET_HZ=120` contract 未改变。截图保存在本机临时审计目录 `arch126-<theme>-<width>.png`，用于本轮视觉检查。
Qt offscreen 仍可能输出 fonts-directory fallback warning；应用入口会先调用 `font_runtime.py` 选择本机可用中文字体。所有 offscreen 数字只说明
当前 composition 的 geometry/event contract，不等同真实 Windows GUI/HIDPI、显示器实际 120fps、EXE startup、硬件连接或正式发行验收。

架构师 `019ff613-3aea-7f92-b6db-2567ca74cee7` 最终 `APPROVE`；独立代码审查
`019ff62a-986e-71c0-98f0-13007776a724` 与 `019ff62a-3bae-7cd2-a4c2-c82732a8110a` 均为
`APPROVE WITH ADVISORIES`（Critical/Required=0）；简化评估 `019ff62a-98bb-7851-a19d-56c59142fded`
为 `APPROVE WITH ADVISORIES`（无必须项）。本轮重新运行的 owner contract audit 为 `pass`：546/640/1180px 分别为
`NARROW/COMPACT/REGULAR`，horizontal spacing 为 `6/6/8`；未运行单元测试或新增测试资产。

## ARCH-125 / UI-1.198 窄窗口响应式几何与 sizing invalidation

真实 Qt offscreen 组合根矩阵覆盖三主题 `star_trail`、`moonlit_ocean`、`sakura_night`，以及请求宽度
`520/546/560/640/768/900/980/1180`。窗口在自然 floor 下可进入 `546px`，UART owner 在 `546px` 保持 `REGULAR`，在
`560px` 与 `640px` 进入 `COMPACT`，`768px` 以上恢复 `REGULAR`；连接带在中间宽度使用 `COMPACT`，`900px` 以上恢复 `REGULAR`。
连接页、协议页、命令页和扩展页的水平滚动条最大值均为 `0`；三主题下无 sibling overlap。事件矩阵的 FontChange、StyleChange、LayoutRequest
均保持模式稳定，反复 resize 和 hide/show/close 生命周期通过。

本轮没有新增 `QTimer`、`QPropertyAnimation`、线程、滚动 owner 或动效时钟；既有共享 `MotionController` 的 `120Hz` scheduler contract 保持不变。
离屏结果只证明当前 Qt composition 的几何与事件契约，不等同真实 Windows GUI/HIDPI、显示器实际 120fps、EXE startup、硬件连接或正式发行验收。
环境仍可能输出 Qt offscreen font-directory fallback warning；应用启动路径会先通过 `font_runtime.py` 选择本机可用中文字体。

架构师 `019ff5c9-8eff-7cd3-9c2f-ab0a40ec9ef0` APPROVE；独立审查 `019ff5df-ef65-7f00-9b30-03c62fad4be9` 最终
`APPROVE WITH ADVISORIES`（Critical/Required=0）；简化审查 `019ff5df-efac-7453-96b5-f9d61b07c0ff` 最终
`APPROVE WITH ADVISORIES`（无必须项）。compileall、Ruff、`scripts/check.ps1`、source-limit、theme audit 与最终矩阵均已重跑并通过。

## ARCH-124 / UI-1.197 命令上下文带几何与动效回归

`CommandContextBand` 不新增 timer、线程、动画帧源、滚动 owner 或业务状态；只在已有 Qt sizing 事件中幂等地重排两个 selector cell。
离屏几何 contract 记录为 compact minimum width `160px`、wide switch threshold `330px`；`320/329px` 为单列，`330px` 起为双列，
style relayout 后列数稳定。workspace 生命周期样本在 `980×720` 与 `1180×820`、`star_trail`/`moonlit_ocean`/`sakura_night` 三主题下均为
`band height=92px`、`columns=2`、focusable nodes `10`，对象 identity、cell 非重叠、accessible name、hide/show/close 均通过。

共享动效回归：10.2 秒 active offscreen 样本 `1224` frames、`120.000Hz`，所有 frame 均为 animated；本轮没有新增 `QTimer`、
`QPropertyAnimation` 或 per-widget clock。该证据不代表真实 Windows GUI/HIDPI、显示器合成、EXE startup、硬件连接或 OTA/RTT 实连性能。
独立 reviewer `019ff5bc-af7c-7f91-bc2d-891292e96db0` 无 Critical/Required，结论 `APPROVE WITH ADVISORIES`；简化审查
`019ff5bc-b25e-7743-9a2b-f0ae71f185f8` 无必须简化项。embedded C/C++ applicability=N/A。

## ARCH-123 / UI-1.196 命令空态首帧与响应式节奏

本轮没有新增 timer、线程、动画帧源、滚动 owner 或业务状态。命令空态由自然高度承载，三步引导在初始化和 resize 时通过同一幂等
`_relayout()` 完成 3/1 列切换；命令页的 `ShortPageVerticalRhythm.TOP` 是共享滚动 helper 的显式 opt-in，连接/协议/扩展页默认 `CENTER`
行为保持不变。

真实 Qt offscreen 几何证据：首次显示覆盖 `520×720`、`640×720`、`720×720`、`980×720`、`1180×820`，三张卡片均有唯一坐标且无 overlap；
反复 `1180→720→520→980→640→1180` resize 后布局幂等；命令页 content top 为 `0`；空态 accessible description 非空。三主题
`star_trail`、`moonlit_ocean`、`sakura_night` 的 palette window/base 均非白色，hide/show/close 通过。

共享动效回归：10.2 秒 active offscreen 样本 `1224` frames、`120.000Hz`，所有 frame 均为 animated；没有新增 `QTimer`、
`QPropertyAnimation` 或 per-widget clock。该证据不代表真实 Windows GUI/HIDPI、显示器合成、EXE startup、硬件连接或 OTA/RTT 实连性能。
独立 reviewer `019ff595-c839-7311-8c47-1a4006951912` 无 Critical/Required，结论 `APPROVE WITH ADVISORIES`；简化审查
`019ff595-cb11-7370-b84c-6d088909da57` 无必须简化项。embedded C/C++ applicability=N/A。

## ARCH-122 / UI-1.195 高刷新时钟与响应式指标

本轮确认 Windows 当前 Python `time.monotonic()` 使用 `GetTickCount64()`、分辨率约 `15.625ms`；4ms PreciseTimer 若继续用该时间源计算
frame budget，实际 `frame_changed` 会退化到约 64Hz。架构批准后，`MotionController` 内部完整时间域切换为 `time.perf_counter()`（QPC），
包括 tick elapsed、activity deadline 和动画资格判断；业务层时间源不变。4ms 仍是唯一 scheduler slot，出帧后 frame budget 取模保留小数，
不追赶过期整数预算。

真实 offscreen 证据：10.2 秒 `1224` active frames，`119.992Hz`，平均间隔 `8.334ms`，p95 `12.097ms`；独立 50ms 卡顿注入后单次
`_tick` 只产生 `1` 次 emit，预算为 `0.259` 且满足 `0 <= budget < 1`；activity 30ms deadline、pause/suspend/close 停表通过。
扩展概览合法 52 字符动作文本在 640/800/980px 中 value label 可收缩，grid 不超过 parent，完整 accessibility description 保留。

该证据是 Qt offscreen scheduler/frame signal 与 presentation geometry，不代表真实显示器 120fps、Windows GUI/HIDPI、paint 合成 CPU、EXE startup、
硬件连接、OTA/RTT 实连或签名验收。独立 reviewer `019ff57c-bd7a-79b3-838c-6471f5a8d87c` APPROVE，无 Critical/Required findings；简化审查
无必须简化项。历史 ARCH-121 及更早的 8ms 记录保留为历史实验，不代表当前实现口径。

## ARCH-121 / UI-1.194 总览态高度预算与首帧收敛

本轮没有新增常驻时钟、动画帧源、滚动 owner 或业务状态。`_ResponsiveWorkspaceShell` 根据 root sibling 的最小高度预算
设置 overview `minimumHeight`：980×720 稳定 floor 为 `168px`、连接 viewport 约 `97px`；1180/1240 宽屏 floor 为 `220px`，
实际 shell 可自然扩张到 `221/301px`，这是剩余空间分配而不是 floor 泄漏。首显和快速 resize 使用最多两轮 coalesced queued
settle，最终 pending/round 均清零。

真实 Qt offscreen 矩阵 `TERMINAL_OVERVIEW_THEME_LIFECYCLE` 共 `312` checks、`0` failures，覆盖三主题
(`star_trail`、`moonlit_ocean`、`sakura_night`)、正常/低动效、`980×720`/`1180×820`/`1240×900`、快速宽屏→980 resize、
focus↔overview、hide/show/close；检查 root sibling 无重叠/越界、连接 viewport、下方 observation/terminal/send 可见性和
queued settle 状态。额外的冷启动/快速 resize采样确认 980px overlap 始终为 `0px`，settle 最多两轮。

没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件连接、OTA/RTT 实连或签名验收；offscreen 证据不等同真实
显示器 120fps。既有 PySide6 font-directory warning 只影响运行环境提示，不代表主题白块或布局失败。

## ARCH-120 / UI-1.193 实时观测与发送栏 sizing

本轮没有新增时钟、timer、重绘循环、滚动 owner 或业务状态；实时观测栏只收敛现有两行 layout spacing 和
动作 QWidget policy，发送栏只收敛既有 mode/button/newline/quick/save 的 intrinsic policy，并保留 input/context
stretch 与 state bounded width。三主题×总览态窗口 `980×720`、`1000×720`、`1080×720`、`1180×820`、
`1240×820`、`1440×900` 及往返 resize、低动效、hide/show/close 共 `36` checks、`0` failures；控件 identity、
无重叠、无越界、按钮未压缩均通过。

本轮未修改共享 120Hz MotionController；没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件连接、
OTA/RTT 实连或签名验收。offscreen 结果不等同真实显示器 120fps。

## ARCH-119 / UI-1.192 连接控制带响应式几何

本轮没有新增时钟、timer、重绘循环或滚动 owner；`_ResponsiveConnectionBand` 只在自身 resize 时按控件
最小尺寸合同切换常规/紧凑布局。三主题×六尺寸（`980×720`、`1000×720`、`1080×720`、`1180×820`、
`1240×820`、`1440×900`）×正反向 resize 共 `72` checks、`0` failures；连续往返 resize、低动效、
hide/show/close 生命周期共 `21` checks、`0` failures。控件边界、无重叠、glyph parent geometry、对象 identity
均通过；980px 进入紧凑四行，1180px 及以上保持常规布局。

本轮未新增或修改共享 120Hz MotionController；没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、
硬件连接、OTA/RTT 实连或签名验收。offscreen 结果不等同真实显示器 120fps。

## ARCH-118 / UI-1.191 过渡互斥与共享调度

本轮没有新增动画时钟；`transition_coordinator.py` 只在 theme/workspace/focus/transport 一次性过渡之间做
互斥和全量 cleanup。真实 Qt offscreen 的共享 MotionController 样本为 1.2s、142 active callbacks，约
`118.33Hz`，平均间隔 `8.393ms`，最大间隔 `16.942ms`；这是调度器/事件循环证据，不等同真实显示器 120fps。

快速切换与生命周期矩阵 `TRANSITION_MUTEX_MATRIX` 共 75 checks、0 failures，覆盖四类过渡各 4 次、四类早退、
旧 finish callback、低动效、暂停、hide/show、幂等 stop 和 close；任意采样最多一个 transition slot 和一个
graphics effect。没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件连接、OTA/RTT 实连或签名验收。

## ARCH-117 / UI-1.190 扩展能力分区节奏

本轮没有新增动画、timer、重绘循环或滚动 owner；每组标题、说明和既有响应式网格由一个轻量
`QFrame` 统一承载。section 水平 margin 为 0，避免 1180px 的 OTA 三卡被容器 padding 从三列压回两列；
上下 margin 10px、spacing 7px 只改善视觉分组，不改变卡片最小宽度和列算法。

真实 Qt offscreen 分区矩阵为三主题×`980×720`、`1180×820`、`1240×900`，并覆盖
`1→2→3→2→1→3` resize，共 318 checks、0 failures；生命周期矩阵 38 checks、0 failures。视觉截图
确认三组分区边界、三主题无白色泄漏；焦点/详情、卡片 identity、原生纵向滚动和 HMAX=0 保持。

本轮未修改共享 120Hz MotionController；没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件
连接、OTA/RTT 实连或签名验收。offscreen 结果不等同真实显示器 120Hz。

## ARCH-116 / UI-1.189 响应式能力卡网格

本轮只在 card 列数发生变化时重排已有 QWidget；resizeEvent 不启动 timer、不触碰业务状态、不创建第二
滚动 owner，也不接入共享 MotionController。980px 使用两列，宽屏 OTA 三卡使用三列，双卡组保持两列，
降低单卡孤岛和过宽卡片带来的信息稀释。

真实 Qt offscreen 网格矩阵为三主题×`980×720`、`1180×820`、`1240×900`，resize 序列
`1→2→3→2→1→3`，共 627 checks、0 failures；扩展生命周期矩阵 662 checks、0 failures。覆盖边界宽度、
非法 card width、卡片 identity/selection/focus、无重叠、HMAX=0、focus/overview、隐藏/恢复、最小化和关闭。
视觉截图确认窄屏两列、宽屏 OTA 三列；同一行 1px 像素宽差来自 Qt 等权 stretch rounding，不是布局错位。

本轮未修改共享 120Hz MotionController；没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件
连接、OTA/RTT 实连或签名验收。offscreen 结果不等同真实显示器 120Hz。

## ARCH-115 / UI-1.188 短页面垂直节奏与滚动边界

本轮没有新增 timer、动画或重绘循环；`ResponsiveScrollArea` 只在 resize/show/rangeChanged 时做一次
对齐状态判断，短页面切为垂直居中，超长页面切回顶部并继续使用原生 vertical scrollbar。横向策略仍为
`ScrollBarAlwaysOff`，没有第二滚动 owner。

真实 Qt offscreen 响应式矩阵为三主题×`980×720`、`1180×820`、`1240×900`×四 workspace，共 217 项
检查、0 failures；生命周期与滚动组合矩阵为 752 项检查、0 failures，覆盖 focus/overview、resize、
隐藏/恢复、最小化、关闭、command empty `Expanding`、长页滚动范围和 HMAX=0。视觉截图确认短连接页
内容重心居中，长扩展页仍顶部开始并保留滚动条。

本轮没有真实 Windows GUI/HIDPI、显示器 120fps、EXE startup、硬件连接、OTA/RTT 实连或签名验收；
offscreen 结果不等同真实显示器 120Hz。共享 MotionController 未被修改。

## ARCH-114 / UI-1.187 主题工作区伸缩与动效边界

本轮消除 focus/overview 之间的错误 stretch 归属：shell 立即接收可用高度，terminal slot 立即隐藏并
保留可恢复快照；不执行逐帧 geometry 动画，因此不会在动画帧之间反复挤压 QTabWidget viewport。route
strip 保持 31px，短页不会留下白色占位，长页继续由原生 vertical scrollbar 承担溢出；横向 scrollbar
保持 `maximum=0`。

真实 Qt offscreen focus matrix 为三主题（`star_trail`、`moonlit_ocean`、`sakura_night`）×
`980×720`、`1180×820`、`1240×900` × 四 workspace，共 36 cases、72 个 focus/restore 状态检查，
失败 `0`；lifecycle matrix 为 `118` 项检查、失败 `0`，覆盖重复切换、隐藏/恢复、最小化、关闭、
route/page、唯一 MotionController 和 snapshot 清理。静态 `check.ps1`、compileall、ruff、source-limit
与 theme-token audit 均通过。

本轮没有新增 timer 或调度器；共享 MotionController 仍以 `TARGET_HZ=120` 为目标。没有运行真实 Windows
GUI/HIDPI、显示器 120fps、EXE startup、硬件连接、OTA/RTT 实连或签名验收；offscreen 结果不等同真实
显示器 120Hz。

## ARCH-113 / UI-1.186 专注页自然高度与 slack

本轮不新增逐帧任务、timer、布局动画或设备工作。focus 模式的静态布局只在 mode 切换、workspace
route 变化或 native resize 后重新结算；`_FocusSpacerVisibilityFilter` 只处理 terminal 子控件的
Show 事件，避免刷新逻辑把它们重新显示到透明 slack 中。既有唯一 `MotionController`、120Hz target
和 opacity transition 不变。

真实 Qt offscreen 定向矩阵为三主题（`star_trail`、`moonlit_ocean`、`sakura_night`）×
`980×720`、`1180×780`、`1240×820` × 四 workspace，共 36 cases：focus/overview、before-show
focus、重复切换、route 切换、resize、暂停/隐藏/恢复/关闭均通过；横向 `HMAX=0`，focus 下 terminal
子内容隐藏、live/send 高度为 0，overview 恢复原始可见性与高度，唯一 MotionController 保持 1 个。

代表性 geometry（`1240×820`）为 connection shell `536px` / page `462px` / `vmax=0`，command
shell `331px` / page `257px` / `vmax=58`，extension shell `485px` / page `411px` / `vmax=623`；
长页面继续使用原生滚动，短连接页不再被下方 surface 挤压。`980×720` connection page 为
`411px` viewport、`vmax=51`，属于受窗口高度约束后的可滚动结果，不是内容截断。

本轮静态 `check.ps1`、compileall、ruff、source-limit、theme-token audit 均通过。没有运行真实
Windows GUI/HIDPI/显示器 120fps、EXE startup、硬件连接、OTA/RTT 实连或签名验收；这些仍是
`not-run`/未授权，offscreen 结果不等同真实显示器 120Hz。

## ARCH-112 / UI-1.185 紧凑顶栏密度

本轮只在实际 `header.width() < 1120` 时隐藏重复的“连接状态”视觉标题，没有逐帧逻辑、额外 timer、
布局动画或设备工作。三主题×`980×720`、`1040×720`、`1120×720`、`1240×820` 的真实 Qt offscreen
矩阵中，紧凑态 status cluster 为约 `536px`，常规宽度恢复完整标题且约 `598px`；`HMAX=0`，
状态组可访问性、resize 状态切换和现有唯一 `MotionController` timer 保持。ARCH-111a 的 shared
120Hz scheduler 未被本轮改动；它仍是调度目标证据，不等同真实显示器 120Hz。

本轮没有真实 Windows GUI/HIDPI、EXE startup、硬件连接、OTA/RTT 实连或签名验收；这些项目仍为
未运行或未授权。

## ARCH-111a / UI-1.184 elapsed frame budget

共享 `MotionController` 仍只拥有一个 8ms `PreciseTimer`，本轮把固定 `0.96` frame/tick 改为
已有实测 elapsed 的 `elapsed × TARGET_HZ`，并保留每 tick 最多一帧的 catch-up fence。真实 Qt
offscreen 主循环样本为 `87` frames、均值 `8.360ms`、p95 `16.000ms`、有效约 `119.61Hz`；
timer interval 仍为 `8ms`，固定预算常量已移除。暂停/隐藏 timer inactive，恢复 active，关闭
inactive；三主题×980/1240×四 workspace 横向 scroll maximum 为 `0`。

这是 scheduler 目标的长期预算修正，不是显示器 FPS 保证；真实 Windows GUI、高刷新显示器、
HIDPI、高负载、EXE startup、硬件连接、OTA/RTT 实连和签名验收仍未运行或未授权。

## ARCH-110 / UI-1.183 命令空态水平居中

本轮只改变 `CommandBatchEmptyState` 的 horizontal stretch 分配：内容组从左贴边改为自然宽度
居中，不增加逐帧绘制、timer、thread、layout animation 或设备工作。真实 Qt offscreen 的
`980×720`、`1240×820`、三主题中，glyph/eyebrow/title/hint/CTA union 中心偏差为 `1px`，
横向/纵向 scrollbar maximum 为 `0`；低动效、暂停、隐藏/恢复、关闭路径保持。

共享 `MotionController` 未改变；本轮 `65` frames、均值 `8.261ms`、p95 `9.299ms`、有效约
`121.05Hz`，仍是 `TARGET_HZ=120`、8ms `PreciseTimer` 的 offscreen scheduler evidence，不等同
真实显示器刷新率、HIDPI 或 EXE 正式性能。真实 Windows GUI、签名、硬件连接和 OTA/RTT 实连仍未
运行或授权。

## ARCH-109 / UI-1.182 专注/总览首帧淡入

本轮只将 overview lower-surface reveal 的起始 opacity 从 `0.0` 提升为 `0.82`，不增加逐帧绘制、
timer、thread、layout allocation 或设备工作；几何仍在动画前静态结算，避免组件挤压和空白首帧。
真实 Qt offscreen 的 `980×720`、`1240×820` 过渡中，三个 surface 在首帧均为 `0.82`、约 110ms
达到 `0.976~0.977`、约 220ms 达到 `1.0`，geometry 稳定且可见；反向 focus、低动效、暂停、隐藏、
恢复和关闭路径通过。

共享 `MotionController` 仍为 `TARGET_HZ=120`、8ms `PreciseTimer` target；独立 0.65 秒样本为
`frames=77`、均值 `8.322ms`、p95 `9.919ms`、有效约 `120.16Hz`。这是 offscreen scheduler
evidence，不等同真实显示器刷新率、HIDPI 或 EXE 正式性能；真实 Windows GUI、签名、硬件连接和
OTA/RTT 实连仍未运行或未授权。

## ARCH-108 / UI-1.181 扩展焦点滚动结算

本轮只在能力卡 focus/click 路径同步结算现有 scroll content，并按实际 geometry 修正 scrollbar；不
进入 per-frame 绘制，不增加 timer、thread、paint loop 或设备 I/O。三主题、980×720/1240×820、7 张
卡逐一聚焦均完整可见，横向 scrollbar maximum 为 0；共享 MotionController 不变，仍为 120Hz/8ms
target，不将 offscreen 证据外推为真实显示器刷新率或 EXE 性能。

## ARCH-107 / UI-1.180 扩展详情按需展开

本轮将首张能力详情从扩展页初始布局移出，改为第一次 focus/click 后按需显示；减少首屏重复内容，
不增加每帧工作、timer、thread、paint loop 或设备 I/O。三主题、`980×720`、`1240×820` 的真实
Qt offscreen 矩阵中，初始 detail hidden、focus/click detail reveal、卡片无 overlap，scroll hint
保持正确。共享 MotionController 不变，仍为 `TARGET_HZ=120` / 8ms scheduler target；不把该证据
外推为真实显示器刷新率或 EXE 正式性能。

## ARCH-106 / UI-1.179 工作区模式上下文

本轮只更新已有 `WorkspaceContextLabel` 的文案、动态 mode 属性和 accessibility 说明，不增加每帧
工作、timer、thread、paint loop、scroll owner 或 layout allocation。三主题、focus/overview、四
workspace 的真实 Qt offscreen 矩阵通过；最长文案 font metrics 为 `85px`，context geometry 为
`158×22`，scroll hint geometry 为 `112×22`（命令页 complete 文案实际 `126×22`）。共享
MotionController 未改变，仍保持 `TARGET_HZ=120` / 8ms scheduler target；本轮不宣称真实显示器
刷新率，也未运行 EXE startup、真实 Windows 可见窗口、HIDPI、高负载或硬件验收。

## ARCH-105 / UI-1.178 滚动位置提示

本轮只改变 `WorkspaceScrollHint` 的文案和静态字重，不改变 scrollbar 信号连接、布局分配、
绘制时钟或页面内容。三主题、980×720/1240×820、四 workspace 的 top/middle/bottom 状态矩阵
均通过；提示 geometry 为 `112×22`，最长文案 font metrics `89px`，横向 scroll 最大值均为 `0`。
这是一次 presentation-only 语义增强，没有新增 per-frame work、timer、thread 或 paint loop。

共享 MotionController 仍保持 ARCH-104 的唯一 `TARGET_HZ=120` / 8ms scheduler contract；本轮没有
改变动画代码，不把既有 offscreen 采样外推为真实显示器刷新率。EXE 启动、真实高刷新显示器、
HIDPI、高负载、硬件连接和 OTA/RTT 实连仍未运行或未授权。

## ARCH-104 / UI-1.177 命令空态画布与动效策略收敛

命令空态画布化只改变布局分配与 presentation 排版，不增加每帧绘制、timer、线程或业务工作。
真实 Qt offscreen 三主题、980×720/1240×820、四 workspace 切换中，command empty state 在 focus
模式下占据可伸缩画布；sibling geometry 无重叠。`1240×820` focus command page 的 empty state
为 `(14,151,1176,399)`，空态内容在卡片内部形成垂直焦点；overview 仍由外层 scroll owner 承载。

本轮对 1ms/4ms/8ms scheduler 和 fixed/elapsed budget 做了非破坏性对照。1ms 原始 QTimer 约 893
tick/s，但真实 `app.exec()` 下 MotionController 仍约 96Hz；elapsed budget 在平台 0ms/16ms
交错回调下会丢弃预算，不能作为改进。该实验因此撤回，最终保持 8ms `PreciseTimer` 与既有
`TARGET_HZ=120` nominal budget。最终主循环样本为 `frames=116`、均值 `8.536ms`、p95 `10.288ms`、
有效约 `117.16Hz`；暂停、低动效和隐藏状态观察到 timer inactive。以上只代表当前 offscreen
scheduler，不能外推真实显示器刷新率、HIDPI、EXE startup 或硬件性能。

## ARCH-103 / UI-1.176 路线条模式视觉层

focus/overview 路线条只通过 QSS 读取已有 `workspaceShell[mode]` 属性，新增渐变和上沿，不改变
固定高度、布局分配、Tab 导航或动效 fan-out。三主题下 focus/overview 截图均显示正确的
`返回总览`/`专注设置`文案与模式颜色；980/1240 四 workspace 横向 scroll 均为 `0`，焦点和
accessibility 文案保持非空。

共享 MotionController 0.5 秒离屏采样：`frames=60`、均值 `8.41ms`、有效采样约 `118.8Hz`。
这只是当前 8ms/120Hz scheduler target 的短时证据，不等同真实显示器 FPS；真实 Windows 可见窗口、
HIDPI、高负载、EXE startup、硬件/HIL、连接/OTA/RTT 实连和签名验收仍未运行或未授权。

## ARCH-102 / UI-1.175 先结算布局再淡入

中间帧截图发现旧的 focus height track 会让 `QTabWidget` 视口逐帧收缩，连接配置表单因此出现
“组件挤在一起”的视觉裁切。修复后 focus/overview 切换先静态结算最终 sibling 几何，再只对 tabs
或实时/终端/发送 surface 使用 opacity track；不再动画 `maximumHeight`，也没有新增 timer、线程、
paint loop 或 scroll owner。

980×720 的 25/55/95/140ms 中间帧均报告 `overlap=False`；focus 连接页 viewport 保持 `514px`。
三主题×980/1240 的四 workspace horizontal scroll 均为 `0`。共享时钟 1 秒采样为 `119` frames，
均值 `8.27ms`、中位数 `8.03ms`、p95 `10.02ms`、最大 `17.08ms`，有效采样约 `120.9Hz`；最大值是
offscreen 事件调度抖动，不等同真实显示器 FPS。真实 Windows 可见窗口、HIDPI、高负载、EXE startup、
硬件/HIL、连接/OTA/RTT 实连和签名验收仍未运行或未授权。

## ARCH-101 / UI-1.174 一次性过渡统一 cadence

page、theme、transport、dialog、focus 各自的 finite owner 均由共享 `MotionController.frame_changed`
驱动；连续活动窗口的离屏均值分别为 page `8.13–8.95ms`、theme `9.40–9.89ms`、transport
`8.21–9.05ms`、dialog `8.21–8.72ms`、focus `8.67ms`。所有样本低于 `13ms` 的工程阈值，目标仍为
`8ms/120Hz` scheduler；不把 offscreen 调度或真实显示器合成宣称为精确 120fps。

验证还覆盖 theme sweep overlay 的完成释放、transport panel 的实际可见切换、两个 dialog hide、
focus 进入以及窗口 hide；过渡期间没有新增 timer/线程/paint loop。第一次脚本选择未变化 transport
或不可见 panel 时采不到 transition，属于验证前置条件错误；修正为连接页 + 实际不同 transport 后
通过，产品代码未为脚本失败做行为性修改。真实显示器/HIDPI、高负载 GUI、EXE startup、硬件/HIL、
连接/OTA/RTT 实连和签名验收仍未运行。

## ARCH-100 / UI-1.173 专注过渡共享帧驱动

审计发现共享 `MotionController` 的装饰帧约为 120Hz target，但专注模式原有
`QPropertyAnimation.valueChanged` 实际约 35Hz，造成高度/opacity 在中间帧集中更新。现在
`MotionDrivenAnimationGroup` 保留 Qt easing/interpolation，只在共享 `frame_changed` 上按单调
时间推进 track，并由 `request_activity(transition + 120ms)` 保证时钟活动；没有新增 timer 或
常驻 repaint loop。最新真实组合根离屏采样中，活动样本平均间隔约 `9.04ms`（约 111Hz 的实际采样窗口，
目标为 8ms/120Hz），220ms 过渡最终稳定；不把离屏 scheduler 或显示器合成宣称为精确 120fps。

验证覆盖专注进入/退出、快速反转、显式 stop、隐藏、最小化、关闭、reduced-motion、pause、resize、
三主题、四 workspace 和横向 scroll；其他 page/theme/dialog 一次性 fade 尚未迁移到该 driver，必须
在后续切片单独测量，避免把 focus 证据外推到全局动画。当前 PySide6 font directory warning 仅作
环境记录；GUI/EXE 启动、真实显示器/HIDPI、硬件/HIL、连接/OTA/RTT 实连和签名验收仍未运行。

## ARCH-97 / UI-1.170 扩展卡片内容安全高度

本轮只增加一个 QSS 专属 selector 的 `min-height: 122px`，避免通用 `QPushButton` 样式把已有
能力卡从安全高度压到约 `83px`；没有新增 paint loop、timer、线程、重绘 fan-out 或 backend。
三主题 × 980/1240 离屏检查中 7 张卡的实际最小高度为 `144px`，当前页横向滚动范围为 `0`，
纵向增长仍交给既有外层 scroll owner。共享 MotionController `TARGET_HZ=120`、`8ms` scheduler
不变。

## ARCH-96 / UI-1.169 有限整数选项与页面宽度

本轮把四类有限数字控件改为不可编辑 option combo，没有新增 timer、paint loop、线程、worker 或
传输层计算。每个 selector 只在构造/协议范围切换时生成有限 item；`setRange()` 在一次屏蔽 signal
区间内完成 rebuild，避免清空/重建造成 dirty fan-out。紧凑宽度为 96–148px，实测不会把 980px
四个工作区撑出横向滚动。

离屏证据：`ARCH96_NUMERIC_SELECTOR_BASE_PASS frame_options=18 frame_value=300 timeout_options=11
timeout_value=0.0`；`ARCH96_NETWORK_OPTION_PASS udp_options=10 clients_options=5 values=(65507,16)`；
`mavlink 280 False 10 108 132`；`ARCH96_PROTOCOL_OPTION_PASS value=12345 options=19`；
`ARCH96_COMMAND_DELAY_OPTION_PASS options=11 value=1234`；`ARCH96_RANGE_SIGNAL_PASS range_value=256
selection_signals=[256]`。逐页切换后的 visible scroll 结果为
`horizontal_max=[0, 0, 0, 0]`，UDP 页数值控件宽度为 `148px`。

## ARCH-95 / UI-1.168 超时选项控件

本轮将八个可编辑 `QDoubleSpinBox` 替换为不可编辑 `BoundedFloatCombo`，没有新增 timer、动画、
paint loop、线程、worker、网络访问或常驻 repaint。每个控件只在构造时生成 10–16 个轻量 item，
运行时连接配置仍读取既有秒数；选项显示格式不会增加传输层计算或 120Hz MotionController
fan-out。`setValue()` 的范围裁剪是一次性列表查找，未知程序化值最多插入一项，不会产生增长型
刷新循环。

真实组合根离屏验证：
`ARCH95_TIMEOUT_SELECTOR_PASS controls=8 option_counts=[10, 11, 12, 16, 16, 16, 16, 16]
defaults=[2.0, 1.0, 0.123, 3.0, 0.2, 3.0, 5.0, 30.0] editable=[False, False, False,
False, False, False, False, False] width=980 height=720`。其中 `0.123` 是验证程序化旧值
的临时选项；原始默认值和 runtime 读取契约另行断言通过。中文方框仅来自当前 PySide6 缺失
font directory 的环境 warning，不归因于 theme/layout。

## ARCH-94 / UI-1.167 工作区 reveal 过渡

本轮没有新增 timer、paint loop、frame fan-out、线程、transport worker 或滚动 owner。总览 reveal
仍由既有一次性 220ms `QParallelAnimationGroup` 驱动；三块下方 surface 只在 reveal 期间各持有
一个临时 opacity effect，完成/反转/静态/隐藏/关闭时立即解绑。这样不改变布局目标，只避免低高度
中间帧把控件内容压缩到同一块区域。

离屏组合根验证 `ARCH94_FOCUS_MOTION_PASS mid_effects=3 settled_heights=(96, 162, 98)
frames=185 target_hz=120 interval_ms=8 effects_clean=True`；同一向量覆盖进入/退出、快速反转、
显式 stop、隐藏和 close 清理。`frames=185` 是该脚本等待窗口内的 callback 计数，不是显示器 FPS。
唯一 `MotionController` 的 120Hz/8ms scheduler contract 保持，仍不宣称精确 120fps。

## ARCH-93 / UI-1.166 推荐 preset hydration

推荐 preset 只在窗口初始化阶段执行一次 combo selection 和已有字段投影，没有新增 timer、线程、
网络访问、设备探测或常驻 repaint loop。实测启动默认选择 `uart-115200-8n1`，7 个 builtin profile
逐项选择/应用均保持 `session_active=False`；手动切换 TCP 后 selection/context 清空，4 个工作区
在 980px 下可见 scroll 的 `horizontal_max=0`。最终视觉样本为 980×720、连接带 `138px`、UART panel
`217px`；推荐摘要与 accessibility description 同步显示。

静态证据：`uv run ruff check src scripts`、`uv run python -m compileall -q src`、`scripts/check.ps1`
（177 files ≤1000、3 themes/22 semantic tokens pass）和真实组合根 preset/transport/accessibility/
scroll contract 通过。未运行：EXE 启动、真实显示器/HIDPI/FPS、硬件连接、OTA/RTT 实连和签名验收。

## ARCH-92 / UI-1.165 响应式几何与 120Hz fan-out

本轮先用真实组合根做 Windows offscreen 几何采样，再收敛布局：连接带从“父布局可垂直吸收
剩余空间”的 `254px` 运行时高度回到自然高度 `138px`；UART panel 在最终样本为 `203px`，
连接页在 980/1240 宽度的可见 scroll 均为 `horizontal_max=0`。四个工作区逐页切换时只保留
当前页的可见 scroll 参与断言，隐藏页的临时 `100×30` geometry 不作为用户溢出证据。

动效仍只有一个 `MotionController`：8ms `PreciseTimer` slot 负责调度，`0.96` nominal frame
budget 将 8ms 整数 slot 校准到 120Hz 的平均样本，phase 继续按 elapsed time 前进。最终离屏
1200ms 样本为 `145` 个 animated frame、`120.83Hz`；这是当前 Qt/offscreen scheduler 的样本，
不代表显示器合成或真实刷新率，也没有宣称精确 120fps。静态 surface predicate 只减少无动态
glyph 的逐帧 update，不改变 `animated=False` 的完整停止广播。

已运行：`uv run ruff check src scripts`、`uv run python -m compileall -q src`、`scripts/check.ps1`、
source-limit/theme audit，以及真实组合根离屏几何、横向滚动、状态切换、隐藏/显示 timer 和 cadence
contract。未运行：EXE 启动、三主题逐像素截图回归、真实显示器/HIDPI/FPS、硬件/HIL、连接/OTA/RTT
验收和代码签名。

## ARCH-91 / UI-1.164 ViewModel worker owner 拆分

本轮只调整 Python module ownership：`viewmodel_jobs.py` 复用原有 blocking discovery job，
`SessionViewModel` 继续复用 `QThreadPool.globalInstance()`、取消 `Event`、queued signal 和
生命周期 stop。没有新增线程池、timer、poll loop、transport worker、分配策略或 UI 动画时钟，
因此不改变数据接收、BLE 扫描、端口枚举或 120Hz presentation scheduler。

静态证据：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、30-signal/2-job
contract、source-limit、theme token audit、provenance verify 和 canonical/root/root-latest hash
equality 通过。`TARGET_HZ=120` 仍是既有 scheduler target，不代表显示器真实 120fps；本轮未启动
GUI/EXE，也未采样 FPS、HIDPI、硬件/HIL 或连接/OTA/debug 路径。

## ARCH-90 / UI-1.163 共享动效 owner 拆分

本轮只改变 Python module ownership，不改变 frame cadence 或绘制算法。`lifecycle_motion.py` 复用
既有唯一 `MotionController`；没有新增 timer、paint loop、frame fan-out、线程、transport worker、
动态分配或 scroll owner。catalog 顺序、visible snapshot cache、activity-only gate、phase offset、
status footer/data activity projection 由原实现原样迁移。

静态证据：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、motion re-export/cadence
contract、source-limit、theme token audit 和 provenance verify 通过。`TARGET_HZ=120` 与 8ms 是既有
scheduler target，不代表显示器真实 120fps；本轮未启动 GUI/EXE，也未采样 FPS、HIDPI 或硬件。

## ARCH-89 / UI-1.162 协议页密度与动效不变量

本轮只调整 Qt layout hierarchy 和 spacing：协议细节从 7 列网格改为三块 labeled field，Dataset
和 Replay 的动作带分行；没有新增 timer、paint loop、frame fan-out、线程、transport worker 或
额外滚动 owner。既有唯一 `MotionController`、`TARGET_HZ=120`、8ms scheduler target 和 reduced
motion 生命周期均未改动。

静态证据：`uv run ruff check src scripts`、`uv run python -m compileall -q src`、`scripts/check.ps1`、
37-field import contract、source-limit、theme token audit 和 provenance verify 通过。未启动 GUI/EXE，
因此没有把布局静态预算写成真实几何，也没有宣称显示器精确 120fps；真实窗口、三主题截图、HIDPI、
键盘焦点和 FPS 采样仍待授权。

## ARCH-88 / UI-1.161 命令页横向密度收敛

本轮没有新增 timer、paint loop、frame fan-out、线程、transport worker 或滚动 owner。批量命令的
选择器与五个动作从一个横向 row 拆为两行，减少按钮与下拉框的最小宽度竞争；状态、结果表和空态
仍由同一 typed binding 组合。

静态证据：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、import contract
通过；源文件 175 个均不超过 1000 行，3 主题 token audit 通过。真实窗口 geometry、键盘焦点、
三主题截图和显示器 FPS 未运行，因此本轮不把布局分行写成运行时几何或 120Hz 证据。

## ARCH-87 / UI-1.160 live band 密度收敛

本轮没有新增 timer、paint loop、frame fan-out、线程或 transport worker。实时观测与发送 band 从
固定多列 grid 改为两行语义布局：发送输入获得独立 stretch，状态/快捷动作不再与输入控件争夺同一
行宽；两个 builder 都只创建原有控件和信号。

静态证据：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、import contract
通过；源文件 174 个均不超过 1000 行，3 主题 token audit 通过。基于控件最小宽度的静态预算，
980px 窗口内两行均保留正向余量；本轮未启动 GUI/EXE，因此不把该预算写成真实几何或显示器
刷新率证据。真实窗口、三主题截图、HIDPI、FPS 和硬件连接仍待授权。

## ARCH-86 / UI-1.159 动效降噪与 endpoint 组合拆分

本轮没有新增 timer、paint loop、frame fan-out、线程或 transport worker。`MotionController` 的
`TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target 未改；`SignalFieldWidget` 由 5 个移动点和
密集网格改为 3 个等间距移动点、单条导引线和 2 个 sparkle，单帧绘制工作量下降且视觉间距增大。
网络 builder 拆分只改变 Python import/owner 边界，网络控件默认值、范围和信号仍是一套。

本轮静态证据：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、presentation
import contract 均通过；源文件 172 个均不超过 1000 行，主题 token audit 通过。未在当前授权下
启动 GUI/EXE 或采样真实显示器帧间隔，因此 ARCH-85 的 `124 frame/s` 是最近一次既有 8ms
scheduler 离屏观测，本轮不追加新的 120Hz 运行时声明。真实窗口、HIDPI、显示器合成和硬件连接
仍待后续明确授权。

## ARCH-85 / UI-1.158 派生空态表面收敛

本轮没有新增 timer、paint loop、frame fan-out 或业务状态；只让两个有界文本预览在有内容时参与
布局。真实组合根 980×720、Microsoft YaHei UI、协议 Tab 的结果为：
`component_preview_visible=False`、`component_empty_visible=True`、`dataset_preview_visible=False`、
`horizontal_max=0`、`vertical_max=778`；注入一个合法 `DecodedFrame` 后，组件预览恢复
`visible=True`。相同路径 close 后 `motion_timer=False`。

本轮单独开启既有共享动效时钟并采样 1 秒，观测 `124` 次 `frame_changed`，配置为
`TARGET_HZ=120`、`PreciseTimer`、`interval=8ms`。这是 Qt scheduler/回调证据，不是显示器实际
刷新率；恢复路径、HIDPI、真实窗口合成与硬件连接仍不在本轮授权范围。

## ARCH-84 / UI-1.157 BLE 配置密度

本轮没有修改 `MotionController`、`frame_changed` fan-out 或任何 transport worker；BLE 重排只增加
语义 field container，并由既有外层 scroll 承载纵向高度。980×720 的 BLE vertical range 为 333，
1240×820 为 233，两个尺寸和三主题的 horizontal maximum 均为 0，字段 overlap=0，截图 exact-white=0。
因此记录为可读性/布局收益，不把布局验证解释为 120Hz 或真实硬件性能提升。

## ARCH-83 / UI-1.156 扩展能力卡选择与展示密度

本轮没有改变 `MotionController`、frame fan-out 或 8ms scheduler；卡片选择只在 click/focus/键盘事件
发生时遍历固定 7 项并刷新一个详情叶子，不创建控件 timer、线程、动画或第二套状态源。详情复用扩展页
既有外层 `QScrollArea`，三主题 × 980×720/1240×820 的离屏布局中卡片 overlap=0、扩展页 horizontal
scrollbar maximum=0，980×720 截图 exact-white pixel 为 0。该结果证明当前展示布局没有横向白带或卡片
挤压，不代表真实显示器刷新率。

## ARCH-82 / UI-1.155 动效快照、fan-out 与恢复调度

日期：2026-08-12。基线是窗口真实组合根在当前 Windows/offscreen 环境约 62–65 帧/s、事件间隔约
15.6ms；单独的 8ms `PreciseTimer` 首次显示可达到约 124–125 帧/s。实现后，稳定 idle 样本为
`frames=100, mean=8.001ms, p50=8.043ms, p95=8.981ms`，activity 样本为
`frames=81, mean=7.998ms, p50=8.020ms, p95=8.963ms`；这是 scheduler/回调观测，不是显示器刷新率。

动效 catalog 共 56 项；24 组 `980/1240 × 3 themes × 4 tabs` route/theme 矩阵通过，快照数量随路由
正确变化，横向溢出为 0。将可见性查询包装计数后，idle/activity 稳定帧期间查询不增长；查询只在 route、
theme、resize、visibility/state 边界触发 snapshot rebuild。pause、reduced-motion、hide、minimize、
close 均无帧继续流出且 timer inactive。

固定两跳 queued rearm 保证恢复路径不在 native show 回调内部直接重建；当前 offscreen/Qt 在 hide/show
以及 minimize/restore 后仍可能回到约 `13.5ms mean / 15.2ms p50 / 15.9ms p95`，直接创建 raw
`PreciseTimer` 的对照也出现同样平台粒度，因此没有引入 sleep、processEvents、deadline 或自适应重试。
真实窗口、不同显示器刷新率、HIDPI 和硬件连接不在本轮授权范围内，不能据此宣称实际面板 120Hz。

## ARCH-81 / UI-1.154 UART 字段密度

日期：2026-08-12  
症状：UART 参数原先挤在八列网格中，980px 链路页虽然通过了边界宽度，但标签、下拉框和超时控件
缺少稳定的视觉分组；不同选项的最大宽度还会让窄窗口更容易产生压缩感。

实现：`connection_builder.py` 改用三行 `QVBoxLayout` + `build_field_row()`。端口/刷新按钮和
线路 checkbox 各自保留局部 QWidget ownership，field helper 负责垂直标签、均匀行间距和可扩展
宽度；没有新增 timer、paint fan-out、业务 callback 或滚动 owner。UART 面板的 size hint 约为
217px，1240×820 实际高度为约 248px，增高来自显式标签与留白，不是空白卡片被父布局拉伸。

实测：`ARCH81_FIELD_GEOMETRY_PASS` 确认 10 个字段矩形无交叠；`ARCH81_TAB_ORDER_PASS` 确认
端口、刷新、波特率、数据格式、超时和线路控制的目标顺序；`ARCH81_TYPED_VALUE_PASS` 确认
`115200/8N1/无流控` → `230400/7O2/RTS/CTS` → 恢复；980×720 与 1240×820 均为
`horizontal_max=0`；三主题均通过；close 清理通过。该切片不改变 MotionController 的
`TARGET_HZ=120 / PreciseTimer / 8ms` 配置，也不把 offscreen 事件间隔解释成真实 120fps。

## ARCH-80 / UI-1.153 顶部 header 密度

日期：2026-08-12  
症状：980px 级别窗口的三组 header 控制旁仍有两个纯装饰 rail，占据可让给主题和状态控制的
横向空间；语义控件没有越界，但视觉密度偏高。

实现：`_AdaptiveHeader.resizeEvent()` 在 header logical width `<1120` 时收起固定尺寸的
`SignalFieldWidget(148×34)` 与 `ThemePaletteSwatch(62×22)`，并将 controls row spacing 从 8px
收敛到 6px；header 大于等于阈值后恢复。没有新 timer、frame signal、状态源、布局重排或业务
回调，原有共享 motion fan-out 会跳过不可见装饰件。

实测：`ARCH80_HEADER_DENSITY` 在 980/1119/1120/1240 四个宽度边界通过，三主题的主题 combo、
低动效和暂停控件仍可见；`ARCH80_LAYOUT_PASS sizes=2 tabs=4 visible_horizontal_max=0`、
`ARCH80_SETTLED_SCREENSHOTS_PASS count=3`、`ARCH80_CLOSE_PASS` 通过。共享动效仍为一个
`PreciseTimer`，配置 `120Hz/8ms`；本轮 29 帧 offscreen 样本均值 `15.607ms`，这是 Windows/Qt
事件粒度观测，不代表显示器精确 120fps。

## ARCH-79 / UI-1.152 命令空态密度与调度边界

日期：2026-08-12  
症状：命令管理页在空批量命令状态下将剩余 viewport 高度分配给 `CommandBatchEmptyState`，
1240×820 时空态卡高度约 451px，内容被拉散。

实现与结果：`build_send_panel()` 在空态之后增加一个 trailing stretch；卡片按 size hint 收敛为
108px，1240×820 与 980×720 均通过，三主题 × 四 Tab 的可见 horizontal scrollbar maximum
均为 0。没有改变空态 CTA、signal、batch state、accessibility 或 scroll owner。

动效重新测得 `TARGET_HZ=120`、`PreciseTimer`、interval `8ms`、motion timer 数 `1`；当前
Windows offscreen 事件回调样本为 46 帧、均值 `15.748ms`、范围 `13.843–18.676ms`。该数字记录
为平台/运行环境事件粒度，不宣称真实显示器 FPS；未经真实可见 GUI/HIDPI 基准，不引入 deadline、
0ms 或 adaptive scheduler。

## ARCH-78 / UI-1.151 动效层级与 activity fan-out

日期：2026-08-12  
目标：解决空闲时 action rail、按钮和空态 composite 同时持续运动造成的视觉拥挤，同时保留
持续环境装饰、短时连接/页面/忙碌反馈和统一生命周期清理。

实现：`MotionController` 仍只有一个 `PreciseTimer`，`activity_active()` 只读输出非环境活动窗口。
生命周期 owner 将 56 个共享 surface 中的 28 个标记为 activity-only（19 个 `ActionRailButton`、
6 个 `BusyActionButton`、terminal/batch/component 三个 composite）；默认链路路由实测只有 4 个
activity surface 可见。activity 结束采用一次 true→false stop，隐藏 activity surface 在活动期间只
停止一次；没有新增 timer、registry、业务状态或 catch-up burst。`AnalysisStatusLabel` 保持环境
语义动效，不把持续状态误判为短时 activity。

验证：offscreen 真实组合根通过 `MOTION_ACTIVITY_SURFACES total=28 visible=4`、单一
`MOTION_CLOCK_TIMERS 1`、ambient 全部 activity surface 静态、activity on/off、隐藏页 stop、
route 切换恢复、pause/resume、hide/show 和 close 清理；`ARCH78_LAYOUT_PASS sizes=2
horizontal_overflow=0` 通过。截图确认链路页无白色横条。离屏 Qt 仍出现既有 font-directory warning，
不代表 Windows 系统字体缺失或发行包失败。

## ARCH-77 / UI-1.150 动效调度与协议页密度

日期：2026-08-12  
目标：解决动效体感不稳和协议/组件配置控件横向拥挤，同时保持单一动效 owner 与既有滚动边界。

实现：`MotionController` 继续使用一个 `PreciseTimer`，改为非 single-shot 的 8ms 周期调度；phase
仍按 `time.monotonic()` 的实际 elapsed time 推进。8ms 是四舍五入的约 120Hz scheduler target，不能
推导出显示器或 compositor 的精确 120fps。协议页只在 presentation builder 内重排为语义 field/action
rows，保留 `ProtocolPanelWidgets`、回调、typed `itemData()` 和外层 `QScrollArea`。

验证：真实组合根在 980×720 与 1240×820、三主题下 horizontal maximum 均为 `0`，9 个关键协议/派生/回放
控件均可见；离屏 cadence probe 为 `156 frames / 1.25s`，平均间隔 `7.996ms`，范围 `4.668–11.670ms`。
该离屏数字只说明 8ms 调度目标在当前环境可运行，不作为真实显示器 FPS 或硬件验收。隐藏/显示恢复、focus
状态、compileall、Ruff、source-limit 和 theme audit 均通过。

## UI-1.119 可见 surface 每帧过滤（reverted）

日期：2026-08-11  
目标：减少共享 MotionController frame 对隐藏 Tab surface 的无效 `set_frame/update()` 调用。

基线真实状态：`_motion_surfaces()` 共 56 个 surface；四个 Tab 可见 surface 分别为
`24/37/24/17`。先用全量 `set_frame()` 作为旧行为参考，再用改动后的 `on_motion_frame()`，
两组都运行 120 帧并在每帧 `processEvents()`，同一进程、同一 1180×780 窗口和三轮样本。

| 测量 | 样本（ms / 120 帧） | 均值 |
|---|---:|---:|
| 全量广播参考 | 603.93 / 604.31 / 565.14 | 591.13 ms |
| 每帧 `QWidget.isVisible()` 过滤 | 562.71 / 737.82 / 736.15 | 678.89 ms |

结论：reverted。每帧 Python/Qt 可见性查询的成本超过了隐藏 widget update 在该环境中的成本，
没有形成稳定收益；不能以“隐藏控件少刷新”为理由保留该改动。`_stop_motion_surfaces()`
全量停止的正确性向量为 `stop_residual_animated=0`，但不作为性能收益证据。

## UI-1.119 Tab 切换缓存可见 surface（reverted）

日期：2026-08-11  
假设：在 Tab 切换时把 56 个 surface 分为 19 个全局 surface 与当前页 surface，逐帧只广播
当前页集合，可以减少隐藏页 `set_frame/update()`。

向量：四页归属为 `9/20/8/0`，缓存后的 active 集合为 `28/39/27/19`；全量 stop 仍能覆盖
全部 56 个 surface。低层直接 fan-out 在 120 帧中从约 `7.25ms` 降到 `4.96ms`，但这不是
用户端到端指标。将同一 offscreen 1180×780 合成根、协议页、120 帧和每帧 `processEvents()`
纳入测量后，全量广播 5 轮均值为 `781.29ms`，缓存广播均值为 `821.50ms`，缓存方案更慢。

结论：reverted。隐藏 QWidget 的 update 在当前环境中并未成为端到端瓶颈，缓存 registry 的
分组/调用开销也没有带来稳定收益；生产代码继续保留 UI-1.118 的简单全量 frame/stop contract。
下一轮性能改动必须先找到可观测的用户端瓶颈，不得为了减少计数而新增全局 registry、逐帧可见性查询、
常驻 timer 或业务状态。

## ARCH-6z / UI-1.142 自适应工作区与 120Hz cadence

日期：2026-08-11  
症状：980×720 时默认总览布局把 workspace tab viewport 压到约 79px，配置页首屏控件拥挤；不能用新增
控件时钟或更复杂的 surface registry 解决布局问题。

基线与结果：980×720 的 workspace shell 为 150px、protocol viewport 为 79px；切换到协议/命令/扩展页后，
既有 focus transition 将 shell 提升到 `506/542px`，page viewport 提升到 `435/471px`，并隐藏下方三块
presentation surface；回到链路页恢复 `150px` 总览和 `96/162/98px` 下方 surface。1240×820 对应 shell
为 `220px` 总览、`606/642px` 配置 focus，三主题和四页均无横向滚动或 root child overlap。

动效验证：同一 offscreen 组合根开启既有 `MotionController.set_ambient_active(True)`，250ms 采到 29 个
animated frame（约 116fps），目标仍为 `TARGET_HZ=120`、PreciseTimer、8/9ms slot；这是 Qt offscreen
短时观测，不宣称真实显示器锁定 120Hz。结论：保留自适应 focus；未新增 timer、splitter 或逐帧策略，
低动效/暂停/隐藏/关闭路径继续复用既有 lifecycle。
