# SerialForge UI 持续美化计划

## 目标

在不破坏 UART、网络、BLE、RTT 现有语义和 OTA/debug 独立边界的前提下，持续把 SerialForge 打磨成可扩展的二次元嵌入式调试工具站：主题一致、状态可读、动效可暂停、窗口可响应、根目录始终有最新 onefile 包。

## 架构决策

- 所有视觉结构和动效仍限定在 `src/serialforge/presentation/`；业务事实继续由 ViewModel/application 提供。
- 装饰动效只复用一个 `MotionController`；不会为单个控件新增独立常驻时钟。
- 状态胶囊使用已有 `SessionState`、数据来源和发送状态动态属性，不在 QSS 或装饰控件中复制业务判断。
- OTA、AES 安全、XMODEM/YMODEM/TFTP、RTT/J-Link 继续保持独立 contract-only/attach-only 边界，不因 UI 美化提前接入组合根。
- 每个增量切片先由架构师只读复核，再由父代理实现、静态审计、交接和打包。
- `main_window.py` 收敛为 composition shell；工作区逻辑迁移到 `presentation/controllers/`，跨区契约放在 `presentation/contracts.py`，详见 ADR 0024。
- 企业级基线以 ADR 0025 为准：bounded context、owner、依赖向内、组合根、ThemeSpec、显式生命周期、源码行数门禁和可追溯交付；不把未公开的 ByteDance 内部规范当作事实。
- 最新交接统一放在 `docs/handoffs/current.md`，仓库根目录不再放 handoff 文件。

## 分阶段切片

### 已完成

- UI-1.10：工作区切换淡入。
- UI-1.11：接收活动脉冲。
- UI-1.12：链路控制状态带。
- UI-1.13：发送成功信号反馈。
- UI-1.14：发送控制状态带。
- UI-1.15：ComboBox popup 与 Tab 溢出按钮暗色兜底。

### 已完成的最新切片

### ARCH-128 / UI-1.201 协议页与命令页局部响应式宽度

- [x] 在 `protocol.py` 内增加局部 `_ResponsiveProtocolRow`，收敛组件 header、Dataset actions/header 的行列响应式布局；配置上下文与
  table/preview/Dataset preview 采用实例级 `minimumWidth=0`、水平 `Ignored`，不修改共享 scroll owner。
- [x] 在 `command_workspace_builder.py` 与 `command_batch_empty_state.py` 增加局部 action row/empty header owner；保留按钮 identity、signals、
  focus/Tab、accessibility 和现有 empty-state API。
- [x] 三主题×9宽度×四页 108 行通过，协议叶子全可见追加 27 行通过：外层 `hmax=0` 且 content 不超过 viewport；四个相关文件均 ≤1000 行。
- [x] 架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` APPROVE；独立审查 `019ff66c-28af-7561-9958-a4da1fea6c48` 最终 APPROVE，
  Critical/Required=0、保留两条 advisory；简化评估 `019ff66c-290b-7cd0-9d06-736a955dd567` 无必须简化项。
- [x] compileall、Ruff、`scripts/check.ps1`、源码行数与主题审计通过；完整窗口事件循环内动效采样 `249 frames / 119.063Hz`，独立性能审查 APPROVE；
  GUI/EXE startup、真实 HIDPI/显示器 120Hz、硬件仍不在本轮授权范围。
- [x] 使用 `local-arch-128` 构建 onefile 并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest 均为
  `48,079,010` bytes，SHA-256 `720131D96530341D0FFD9A271588A2BEDE1FF9829C9D3DE95D230E1DE5FE3688`，archive listing SHA-256
  `489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`，provenance verify pass；engineering build、NotSigned、
  `release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-127 / UI-1.200 UART 参数表单列级 sizing

- [x] `ResponsiveUartForm` 按 hard/preferred sizing contract 实现 regular 四列、compact 两列、narrow 单列；确定性 water-fill 保留线路控制
  等字段的 minimum width，不改变 builder/runtime wiring。
- [x] 三主题×9宽度、事件、往返 resize、字段 minimum/overlap、focus/identity/lifecycle 通过；源文件行数门禁通过。
- [x] 架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` 修订后 APPROVE；简化评估无必须项；embedded C/C++ applicability=N/A。
- [x] ARCH-127/128 证据已归档；以同一 `local-arch-128` revision 完成静态门禁与 onefile 覆盖。

### ARCH-126 / UI-1.199（2026-08-12）

- [x] 新增 `presentation/responsive_header_controls.py::ResponsiveHeaderControls`，让 Header 的 status/motion/theme 三个既有 cluster 按 sizing contract
  在 `REGULAR/COMPACT/NARROW` 三态布局；紧凑/窄态 status 独占整行，消除 `UA…/实…/…`。
- [x] `workspace.py` 只创建并注入 owner；三个 QFrame 保持同一 owner parent、HeaderChromeBindings、window refs、signals、focus/Tab/accessibility 和主题/动效 wiring。
  `_sync_header_density()` 只负责 density、装饰件显隐和内部间距，再触发 owner sizing invalidation。
- [x] 完成同一 owner grid 的 remove/add 合同、show/polish/FontChange/StyleChange/LayoutRequest/resize 幂等 reflow、状态列 preferred-width 优先分配和多行高度 contract。
- [x] 三主题 × `546/547/560/640/768/900/1120/1180/1240px`、`900→546→900`、状态无省略、页面 hmax=0、焦点、事件、生命周期通过；compileall、Ruff、
  `scripts/check.ps1`、source-limit、theme audit 通过；embedded C/C++ applicability=N/A。
- [x] 架构师 `019ff613-3aea-7f92-b6db-2567ca74cee7` 最终 APPROVE；代码审查 `019ff62a-986e-71c0-98f0-13007776a724`、修复复核
  `019ff62a-3bae-7cd2-a4c2-c82732a8110a` 与简化评估 `019ff62a-98bb-7851-a19d-56c59142fded` 均为 `APPROVE WITH ADVISORIES`，Critical/Required=0。
- [x] 使用 `local-arch-126` 重新构建 onefile，覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest 均为
  `48,064,034` bytes，SHA-256 `B55EF7C6BF173E5EEDB13CBD1F5355DA7249F0592684FABBEC85D85C3B7D6D91`，archive listing SHA-256
  `489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`，provenance verify pass；engineering build、NotSigned、
  `release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-125 / UI-1.198（2026-08-12）

- [x] 去掉 bootstrap 的 `980×720` 硬窗口最小尺寸；以真实 Qt sizing contract 允许窗口进入窄宽。
- [x] 将连接/预设顶部 shell 收敛为 `_ResponsiveConnectionBand` 三态 owner：`REGULAR`、`COMPACT`、`NARROW_COMPACT`；在窄态分离连接、
  预设、保存/删除、上下文与状态 rail，保留既有控件 identity、bindings、signals、Tab 顺序和业务状态。
- [x] 新增 `presentation/responsive_uart_form.py::ResponsiveUartForm`，把十个 UART labeled field wrapper 在 `REGULAR/COMPACT/NARROW`
  三态中响应式排列；builder 继续拥有控件创建和 UART runtime wiring。
- [x] 为 FontChange、StyleChange、LayoutRequest 和 resize 增加幂等 sizing invalidation；不新增 timer、线程、scroll owner 或动画时钟。
- [x] 三主题与 `520/546/560/640/768/900/980/1180` 宽度离屏矩阵通过，连接/协议/命令/扩展页 `hmax=0`，事件与生命周期通过；静态检查、主题审计、
  文件行数门禁通过；embedded C/C++ applicability=N/A。
- [x] 架构师 `019ff5c9-8eff-7cd3-9c2f-ab0a40ec9ef0` APPROVE；独立 reviewer `019ff5df-ef65-7f00-9b30-03c62fad4be9` 与简化 reviewer
  `019ff5df-efac-7453-96b5-f9d61b07c0ff` 最终均为 `APPROVE WITH ADVISORIES`，Critical/Required=0、无必须简化项。
- [x] 使用 `local-arch-125` 重新构建 onefile 并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest 均为 `48,057,767` bytes，
  SHA-256 为 `C8F19055CA977477BA33C087E3CEEA92ACD99F328A2F7F2410189DF96D748BBA`，archive listing SHA-256 为
  `0642B8F37772873BF7D7DB9590125ACA77B319F3E06B2308BBE86826951C538F`，provenance verify 通过；签名 `NotSigned`、`release_eligible=false`、
  `hardware_acceptance=not_run`。

### ARCH-124 / UI-1.197（2026-08-12）

- [x] 将命令页发送历史与批量命令选择收敛为 `CommandContextBand`：由 cell sizing contract 自动在单列/双列间切换；五个批量操作按钮保留独立 action row，
  不改变业务、bindings、signals、焦点/Tab 顺序、滚动 owner 或主题语义。
- [x] 使用 `SetNoConstraint`、compact minimum/current-height contract 和现有 cell identity，移除嵌套 surface 风险；不增加 QSS、timer、动画时钟或业务接口。
- [x] 320–1180 几何阈值、三主题 workspace 生命周期、style relayout、accessibility、对象 identity 与唯一 120Hz scheduler 均通过；compileall、Ruff、
  `scripts/check.ps1`、source-limit、theme audit 通过；embedded C/C++ applicability=N/A。
- [x] 架构师 `019ff5a5-cb5d-75b3-9fa6-c41bc2a66802`、`019ff5a8-f256-7002-b8fc-86de2dab2dd0`、`019ff5b5-7c29-7fa3-9c27-14b251e0e951` 与
  `019ff5c0-2320-7ec1-99d1-2b7d90444be1` APPROVE；独立 reviewer `019ff5bc-af7c-7f91-bc2d-891292e96db0` 无 Critical/Required；简化审查
  `019ff5bc-b25e-7743-9a2b-f0ae71f185f8` 无必须简化项。
- [x] 使用 `local-arch-124` 构建 onefile 并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest 均为 `48,052,563` bytes，SHA-256 为
  `B52DD4ADFC7C06CEAF9E2B46BCA1FB9AFB2DE6AA996B286AAF80ED9E12107B16`，archive listing SHA-256 为
  `060BCD1C9D88F7C19657FA6A66794FB333F24BE0B8676605BACFBB4A121F9D1D`，provenance verify 通过；签名 `NotSigned`、`release_eligible=false`、
  `hardware_acceptance=not_run`。

### ARCH-123 / UI-1.196（2026-08-12）

- [x] 将命令批处理空态从 flexible stretch slot 收敛为自然高度，新增 presentation-only 三步引导轨；保留 `new_requested`、批处理
  bindings、ViewModel、执行链路和唯一共享 MotionController。
- [x] 新增 `ShortPageVerticalRhythm` 显式页面契约：命令页 opt-in `TOP`，连接/协议/扩展页默认 `CENTER`；不改变 QScrollArea、viewport、
  滚动 owner 或页面名称解耦边界。
- [x] `CommandBatchStepRail` 通过幂等 `_relayout()` 在初始化和 resize 时重排 3/1 列；520/640/720/980/1180 首帧、反复 resize、三主题、
  无障碍、hide/show/close 与 120Hz scheduler 验证通过；compileall、Ruff、source-limit、theme audit 通过。
- [x] 架构师 `019ff58a-9d3d-75b0-9638-95529059138a` 与 follow-up `019ff59a-996b-76e0-adbf-48a8633392a1` APPROVE；独立 reviewer
  `019ff595-c839-7311-8c47-1a4006951912` 无 Critical/Required（`APPROVE WITH ADVISORIES`）；简化 reviewer
  `019ff595-cb11-7370-b84c-6d088909da57` 无必须简化项；embedded C/C++ applicability=N/A。
- [x] 使用 `local-arch-123` 构建 onefile 并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest 均为
  `48,047,222` bytes，SHA-256 为 `803114A03FA0FBEBF440005A1C79C6970FAB85C0863911CA2CF7B87DDCCB8054`，archive listing SHA-256 为
  `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名 `NotSigned`，
  `release_eligible=false`，`hardware_acceptance=not_run`。

### ARCH-122 / UI-1.195（2026-08-12）

- [x] 将唯一 `MotionController` 的内部 elapsed/deadline 时间域统一为 `time.perf_counter()`，保留 4ms `PreciseTimer`、120Hz scheduler target、
  phase cap、frame budget 取模和每回调最多一帧；解决 Windows `monotonic` 15.625ms 分辨率导致的约64Hz实际回调与卡顿补帧 burst。
- [x] 新增 `_ResponsiveMetricGrid`，按可用宽度 6/3/2/1 列重排扩展概览指标；长动作值可收缩换行且 accessibility description 保留完整文本；
  不改变 DTO、业务、动画、主题、scroll owner 或 OTA/AES/RTT/J-Link 边界。
- [x] compileall、ruff、`scripts/check.ps1`、10.2s scheduler、卡顿单帧栅栏、activity/lifecycle、长文本 geometry 验证通过；独立 reviewer APPROVE，
  无 Critical/Required findings；简化评估无需扩大简化；本轮 embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-122` 构建 onefile 并覆盖根目录 `SerialForge.exe` 与 `SerialForge-latest.exe`；canonical/root/root-latest 均为
  `48,044,048` bytes，SHA-256 为 `CF85FAFB7F757FA09853670D051077A31FABECF52A91A8CF205A8145C2DF0B6D`，archive listing SHA-256 为
  `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名 `NotSigned`，
  `release_eligible=false`，`hardware_acceptance=not_run`。

### ARCH-121 / UI-1.194（2026-08-12）

- [x] 在 `presentation/controllers/workspace.py` 内增加 `_ResponsiveWorkspaceShell`，以 root sibling 最小高度预算计算总览态
  workspace floor；980×720 floor 约 168px，宽屏 floor 封顶 220px，实际宽屏高度允许自然扩张；既有 QScrollArea 继续承载页面
  overflow，不改变业务、bindings/signals、scroll owner、MotionController 或 OTA/AES/RTT/J-Link 边界。
- [x] 在 focus snapshot restore 后通过兼容同步入口重算 overview floor；resize/首显通过最多两轮 coalesced queued settle 消除首帧
  geometry overlap，加入 pending/round guard 和 `isValid` 生命周期保护；不新增常驻 timer、动画时钟或业务状态。
- [x] 三主题×正常/低动效×多尺寸、快速 resize、focus↔overview、hide/show/close 共 312 checks、0 failures；compileall、ruff、
  source-limit、theme audit 通过；架构师四次正式批准，独立 reviewer 超时未形成外部结论，父代理完成六轴 review 与
  behavior-preserving simplification assessment；本轮 embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-121` 重新构建 onefile 并覆盖根目录 EXE：canonical/root/root-latest 均为 `48,040,231` bytes，SHA-256 为
  `0524910A444C68B5437E94A73481F590E2F4865072E33158C703D1AFB79B80EC`，archive listing SHA-256 为
  `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名 `NotSigned`，
  `release_eligible=false`，`hardware_acceptance=not_run`。

### ARCH-120 / UI-1.193（2026-08-12）

- [x] 在 `terminal_toolbar_builder.py` 内增加无状态 `_ResponsiveObservationBand`，让实时观测动作按钮恢复
  intrinsic width；在 `send_bar_builder.py` 内固定发送模式/发送/CRLF/快捷命令/保存快捷 policy，让输入/摘要
  承担剩余伸缩；不改变 `TerminalControlBindings`、signals、业务、timer、动画或 MotionController。
- [x] 三主题总览态多尺寸 resize、低动效、hide/show/close 共 36 checks、0 failures；check/compileall/ruff/
  source-limit/theme audit 通过；架构师正式批准，独立 reviewer 超时未形成外部结论，已如实记录；本轮
  embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-120` 重新构建 onefile 并覆盖根目录 EXE：canonical/root/root-latest 均为
  `48,038,525` bytes，SHA-256 为
  `05A72BF2599736C65BA4B43CFB525F72FED4EB2C2CA4E47672BAC46B2E07165B`，archive listing SHA-256 为
  `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

### ARCH-119 / UI-1.192（2026-08-12）

- [x] 在 `presentation/controllers/connection_builder.py` 内增加无状态 `_ResponsiveConnectionBand`，按既有
  控件 sizing contract 在窄宽度切换四行 compact layout，宽屏恢复常规布局；不改变控件 identity、
  `ConnectionShellBindings`、signal/callback、业务状态、滚动或 MotionController。
- [x] 三主题×六尺寸×正反向 resize 共 72 checks、连续生命周期矩阵 21 checks 均 0 failures；
  `scripts/check.ps1`、compileall、ruff、source-limit、theme audit 通过；独立 reviewer 等待超时未形成外部结论，
  已如实记录；本轮 embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-119` 重新构建 onefile 并覆盖根目录 `SerialForge.exe`、`SerialForge-latest.exe`；
  canonical、root、root-latest 均为 `48,036,144` bytes，SHA-256 为
  `CBED1FB08241C4D938796CED3F2AECF84C1A8BBFEC793A9832D489AAFBE495DD`，archive listing SHA-256 为
  `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；
  签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-118 / UI-1.191（2026-08-12）

- [x] 新增 `presentation/transition_coordinator.py`，以 `ShellTransitionKind` 统一 theme/workspace/focus/
  transport 一次性过渡互斥；所有有效启动前停止竞争 transition，旧 owner 继续拥有 effect/finish 细节。
- [x] 无效目标、低动效、隐藏/最小化、resize/hide/close 和重复 stop 统一幂等全量清理；不新增 timer、
  MotionController、布局、业务、滚动或 OTA/AES/RTT/J-Link coupling。
- [x] 共享调度样本 142 callbacks/1.2s、约 118.33Hz；快速交互/早退/生命周期矩阵 75 checks、0 failures；
  compileall/ruff/check/source-limit/theme audit 通过；明确不把 offscreen scheduler 样本宣称为真实 120fps。
- [x] 完成架构师 REVISE→coordinator 方案、入口调用链和早退修正批准；独立 reviewer 超时且未形成外部结论，
  已如实记录；本轮 embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-118` 重新构建 onefile 并覆盖根目录 `SerialForge.exe`、`SerialForge-latest.exe`；
  canonical、root、root-latest 均为 `48,033,518` bytes，SHA-256 为
  `83A3EFCB92D13D45C234C5D17D213B8AF22953DA2C8FA267F674EE250683B00F`，archive listing SHA-256 为
  `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；
  签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-117 / UI-1.190（2026-08-12）

- [x] 在 `presentation/embedded_extension_panel.py` 增加 `_build_capability_section()`，用无焦点
  `QFrame#extensionCapabilitySection` 统一承载标题、说明和既有 `ResponsiveCapabilityGrid`；上下 margin
  10px、spacing 7px、水平 margin 0，改善三组能力的视觉层级而不压缩三列网格。
- [x] 保持 capability card identity、selection/detail/accessibility/Tab 顺序、原生 QScrollArea、HMAX=0、
  唯一 120Hz MotionController 与 OTA/AES/RTT/J-Link contract-only/attach-only 边界；不新增 QSS、timer 或动画。
- [x] 三主题×980/1180/1240 与 `1→2→3→2→1→3` resize 的分区矩阵 318 checks、生命周期矩阵 38 checks
  均 0 failures；check/compileall/ruff/source-limit/theme audit 通过；视觉截图确认三组分区和无白色泄漏。
- [x] 完成架构师两轮批准、父代理六轴 review 与行为保持 simplification assessment；独立 reviewer 超时且未形成
  外部结论，已如实记录；本轮 embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-117` 重新构建 onefile 并覆盖根目录 `SerialForge.exe`、`SerialForge-latest.exe`；
  canonical、root、root-latest 均为 `48,031,424` bytes，SHA-256 为
  `4488BDB5E31EEBF6C61D9BE4925C829E64A090E9C158396E6CA3B1A9F98400C0`，archive listing SHA-256 为
  `ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；
  签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-116 / UI-1.189（2026-08-12）

- [x] 新增 `presentation/responsive_capability_grid.py`，按扣除 margins/spacing 后的可用宽度在 1/2/3
  列间切换；980px 为两列，1180/1240px 的 OTA 三卡为三列，双卡组保持两列；只重排已有 card QWidget。
- [x] 保持 `embedded_extension_panel.py` 的 capability catalog、selection/detail/accessibility/Tab 顺序、
  原生 QScrollArea、focus/overview、唯一 120Hz MotionController 和 OTA/AES/RTT/J-Link contract 边界。
- [x] 三主题×三尺寸、`1→2→3→2→1→3` 反向 resize 的网格矩阵 627 checks、扩展生命周期矩阵 662 checks
  均 0 failures；check/compileall/ruff/source-limit/theme audit 通过；视觉截图确认窄屏两列、宽屏 OTA 三列。
- [x] 完成架构师条件批准、父代理六轴 review 与行为保持 simplification assessment；独立 reviewer 超时且未形成
  外部结论，已如实记录；本轮 embedded C/C++ applicability 为 N/A。
- [x] 使用 `local-arch-116` 重新构建 onefile 并覆盖根目录 `SerialForge.exe`、`SerialForge-latest.exe`；
  canonical、root、root-latest 均为 `48,030,866` bytes，SHA-256 为
  `334406DFEA71D5E4B25FB67088DD3B907250EE8529827CE4AB644FB6FEF780DA`，archive listing SHA-256 为
  `ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过；
  签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-115 / UI-1.188（2026-08-12）

- [x] 新增 `presentation/responsive_scroll_area.py`，统一管理短页垂直 `AlignVCenter` 与长页 `AlignTop`；
  保留水平填充和现有原生 QScrollArea 滚动 owner。
- [x] `scroll_page()` 仅替换共享滚动容器；connection/protocol/command/extension builder、页面 API、
  accessibility、command empty `Expanding` owner、focus/root stretch 与唯一 120Hz MotionController 不变。
- [x] 三主题×980/1180/1240×四 workspace 响应式矩阵 217 checks、生命周期矩阵 752 checks，均 0 failures；
  check/compileall/ruff/source-limit/theme audit 通过；截图确认短连接页重心居中、长扩展页顶部滚动保持。
- [x] 完成架构师有条件批准、viewport signal 最小修正批准、父代理六轴 review 与 behavior-preserving
  simplification assessment；独立 reviewer 超时且未形成外部结论，已如实记录。
- [x] 使用 `local-arch-115` 重新构建 onefile 并覆盖根目录 `SerialForge.exe`、`SerialForge-latest.exe`；
  canonical、root、root-latest 均为 `48,028,531` bytes，SHA-256 为
  `8E98DB669C51C64813B701015A9DC2E2C9C7BAE4729025278BEECAF218713924`，archive listing SHA-256 为
  `CD3C60D336971C88071C3A0D1C3B74D6AD090D1D2704F753C13B058E236913A0`，provenance verify 通过；
  签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

### ARCH-114 / UI-1.187（2026-08-12）

- [x] 修正 focus/overview 根布局 stretch 所有权：focus 为 `shell=1, terminal=0`，terminal slot 仅作为
  可恢复隐藏 slack，避免 themed workspace 外围出现白色空行或配置组件被挤压。
- [x] 以 `_FocusLayoutSnapshot` 保存并恢复 root/shell/tabs/route stretch、vertical policy、min/max、
  terminal parent/child hidden 与鼠标透明；shell 内 tabs 唯一伸缩，route strip 固定 31px。
- [x] 保持 native QScrollArea 滚动和共享 `MotionController` 120Hz target；焦点切换只对 opacity 做视觉
  过渡，不新增 timer、geometry animation、业务或 OTA/AES/RTT/J-Link coupling。
- [x] 三主题×980/1180/1240×四 workspace 的 36 focus cases、72 checks 通过；lifecycle 118 checks
  通过；check/compileall/ruff/source-limit/theme audit 通过。
- [x] 完成架构师 conditional-pass 方案复核、bool 返回契约复核、父代理五轴 review 与 behavior-preserving
  simplification assessment；独立 reviewer 本轮超时且未形成外部结论，已如实记录。
- [x] 使用 `local-arch-114` 重新构建 onefile 并覆盖根目录 `SerialForge.exe`、`SerialForge-latest.exe`；
  canonical、root、root-latest 均为 `48,026,927` bytes，SHA-256 为
  `966C16A74D04454CE8780F424E19D99773E3E39B1CBE42A0A5490812CB605F2C`，archive listing SHA-256 为
  `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过；
  签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-113 / UI-1.186（源码与真实 Qt focus/layout/lifecycle 回归完成，2026-08-12）：修复专注页
  只剩大块空白或页面被压缩的问题。`workspace_focus_transition.py` 复用已有透明 terminal slot
  作为 layout slack，按当前页面 `minimumSizeHint()` 结算自然 shell 高度；短页面收敛，长页面保留
  原生滚动。`workspace_runtime.py` 与 `lifecycle.py` 只通过显式 refresh 触发同一 owner，不新增
  root spacer、timer、布局动画、业务状态或 OTA/AES/RTT/J-Link coupling。三主题×980/1180/1240×
  四 workspace 共 36 cases、focus/overview、before-show、重复切换、resize、HMAX、可见性、唯一
  MotionController 通过；架构师 Hubble/Pasteur/Russell 给出方案/约束结论，Kant 与独立 reviewer
  Pauli 超时关闭且未形成外部结论；父代理完成五轴 review 与 behavior-preserving simplification
  assessment。嵌入式 C/C++ applicability 为 N/A；`local-arch-113` onefile 已覆盖 canonical/root/root-latest；
  三者均为 `48,024,709` bytes，SHA-256 为 `ED76F5E158A5B951B4A58B4EE1732277B3093C8A08FDD270EA19D57DB539703A`，
  archive listing SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，
  provenance verify 通过；签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-112 / UI-1.185（源码与真实 Qt 视觉/生命周期回归完成，2026-08-12）：收敛紧凑顶栏的重复
  状态标题。`controllers/workspace.py` 以实际 `header.width()` 为唯一响应式依据，低于 `1120px`
  时隐藏视觉“连接状态”标题，保留状态灯、状态值、状态组 accessibility 和主题/动效控制，宽屏
  自动恢复；不新增状态源、timer、QSS、业务、transport、OTA/AES/RTT/J-Link 耦合。三主题×
  `980/1040/1120/1240` 实际 header 宽度矩阵、`HMAX=0`、状态可访问性、resize、连接状态和唯一
  MotionController 生命周期通过。架构师 `019ff441-ac00-7e51-a0cd-a681d0dd2071` 与独立 reviewer
  `019ff443-dd0d-74c3-b425-ce42af1ec782` 等待窗口超时关闭，未形成外部结论；父代理完成
  correctness、architecture、security、performance、readability 五轴 review 与行为保持简化评估。
  嵌入式 C/C++ applicability 为 N/A；`local-arch-112` onefile 已覆盖 canonical/root/root-latest；
  三者均为 `48,018,696` bytes，SHA-256 为 `2FACEA3043A66841DD1249228AC500EBCBEFC11D122D0C93B71EE35036F52C13`，
  archive listing SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，
  provenance verify 通过；签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-111a / UI-1.184（源码与真实 Qt 调度回归完成，2026-08-12）：修正共享 `MotionController`
  用固定 `0.96` frame/tick 累积造成的长期预算偏差，改为按已有实测 monotonic elapsed 累积
  `elapsed × TARGET_HZ`；保留唯一 8ms `PreciseTimer`、每 tick 最多一帧、phase cap 和暂停/隐藏/
  恢复/关闭复位，不新增 timer、组件动画、线程、业务状态或设备耦合。真实 Qt offscreen 样本为
  `87 frames`、均值 `8.360ms`、p95 `16.000ms`、有效约 `119.61Hz`；三主题×980/1240×四 workspace
  横向滚动为 `0`，focus/overview 可见性保持。架构师与独立 reviewer 调用超时关闭，未形成外部
  结论；父代理完成五轴 review 与行为保持简化评估。嵌入式 C/C++ applicability 为 N/A；onefile/root
  EXE 已由 `local-arch-111a` 覆盖 canonical/root/root-latest；三者均为 `48,021,974` bytes，
  SHA-256 为 `56133D65BBE514AB18AF505D0446354CA948B96A6BBCEE39D16CCCBA2EC0084F`，archive listing
  SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify
  通过；签名 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-110 / UI-1.183（源码、静态与真实 Qt 离屏验证完成，2026-08-12）：修复命令管理无批量命令时
  空态内容贴左、宽屏右侧出现无语义空白的问题。`CommandBatchEmptyState` 保留唯一 vertical stretch
  owner，组件内部仅增加水平两侧 stretch，glyph+copy 使用自然宽度居中；不改 command controller、
  ViewModel、命令执行、滚动 owner、CTA signal、accessibility 或 OTA/AES/RTT/J-Link 边界。三主题、
  980×720/1240×820、中心偏差 1px、滚动 maximum 0、低动效/暂停/隐藏/恢复/关闭与约 `121.05Hz`
  scheduler evidence 通过。架构师与独立 reviewer 调用超时关闭，未形成外部结论；父代理完成五轴
  review 与行为保持简化评估。嵌入式 C/C++ applicability 为 N/A；`local-arch-110` onefile 已覆盖
  canonical/root/root-latest，三者均为 `48,019,942` bytes，SHA-256 为
  `EFE27AA251E89A9D857CD0E6D8D11B914A9AB24C730F802601225DD3465145A4`，archive listing SHA-256 为
  `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。

- ARCH-109 / UI-1.182（源码、静态与真实 Qt 离屏验证完成，2026-08-12）：修复专注设置返回总览时
  下方实时观测、终端和发送 surface 从 `opacity=0.0` 开始导致的空白首帧。布局仍先一次性结算，
  既有共享 opacity track 从命名常量 `0.82` 淡入 `1.0`；不新增 timer、布局 geometry 动画、业务状态、
  scroll owner 或 OTA/AES/RTT/J-Link 后端。980×720/1240×820、三主题、总览/专注、四 workspace、
  低动效/暂停/隐藏/恢复/关闭与约 `120.16Hz` scheduler evidence 通过。架构师与独立 reviewer 调用
  超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。嵌入式 C/C++ applicability
  为 N/A；onefile/root EXE 已由 `local-arch-109` 覆盖 canonical、根目录和 root-latest；三者均为
  `48,019,597` bytes，SHA-256 为 `35DE8906B0838D29562F85E2518035C64DA5B69FAA65F9AFA7BCB8B065ED9DD7`，
  archive listing SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，
  provenance verify 通过；`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-108 / UI-1.181（源码、静态与真实 Qt 离屏验证完成，2026-08-12）：修复扩展工具站能力卡
  键盘焦点不可见问题。详情按需展开改变 scroll content 高度后，扩展页组合 owner 同步结算现有
  `QScrollArea.widget()`，调用原生 `ensureWidgetVisible`，并按实际 viewport 几何做有界 vertical
  scrollbar 校正；不新增 timer、singleShot、processEvents、scroll owner、业务状态或 OTA/AES/RTT/
  J-Link 后端。三主题、980×720、1240×820、7 张能力卡逐一聚焦全部 visible/focus，横向 scrollbar
  为 0，scroll hint 中段状态正确。架构师与独立 reviewer 调用超时关闭，未形成外部结论；父代理
  完成五轴 review 与行为保持简化评估。嵌入式 C/C++ applicability 为 N/A；onefile/root EXE 覆盖
  已由 `local-arch-108` onefile 覆盖 canonical、根目录和 root-latest；三者均为 `48,020,617` bytes，
  SHA-256 为 `2300FFD431657EA96E67A43F1470199671883CAC9AA5FC711F0CA1AB6330A217`，archive listing
  SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify
  通过；`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-107 / UI-1.180（源码、静态与真实 Qt 离屏验证完成，2026-08-12）：扩展工具站的
  `ExtensionCapabilityDetail` 初始隐藏，Tab 聚焦或点击能力卡后按需显示，消除首屏第一张 XMODEM
  详情与 OTA 传输卡片的重复；不改变应用层 catalog、XMODEM/YMODEM/TFTP、AES-128-CCM/AES-256-GCM、
  RTT/J-Link attach-only/contract-only 边界，不新增 timer/thread/设备 I/O。三主题、980×720、1240×820
  初始隐藏、focus reveal、click update、7 卡 overlap 与 scroll hint 矩阵通过。架构师与独立 reviewer
  调用均超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。嵌入式 C/C++ applicability
  为 N/A。`local-arch-107` onefile 已覆盖 canonical、根目录和 root-latest；三者均为 `48,017,630`
  bytes，SHA-256 为 `0264B14093A62DD296B3DF9C753BFFBD9FB66941D75C51227B17727098BFDDEC`，archive listing
  SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify
  通过；`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-106 / UI-1.179（源码、静态与真实 Qt 离屏验证完成，2026-08-12）：复用既有
  `WorkspaceContextLabel` 投影 `workspaceShell[mode]`，总览显示“总览 · 当前页”，专注显示“专注 · 当前页”，
  tooltip/accessibility description 同步解释配置优先与后台工作语义。不新增 ViewModel 字段、focus 状态源、
  timer、scroll owner、业务或 OTA/debug coupling；不改变自动 focus 策略、route strip 31px、Tab/焦点顺序或
  120Hz shared clock。三主题、980×720、四 workspace 的 focus/overview 矩阵通过，最长文案 85px、context
  geometry 158×22、scroll hint 112×22（命令 complete 实际 126×22），无 sibling overlap。架构师与独立
  reviewer 调用均在等待窗口内超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。嵌入式
  C/C++ applicability 为 N/A。`local-arch-106` onefile 已覆盖 canonical、根目录和 root-latest；三者均为
  `48,019,150` bytes，SHA-256 为 `F1F6BFC10CDE0103B9B117C69FC14ED9AA28887F00619AD212FA089570223BE9`，
  archive listing SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，
  provenance verify 通过；`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-105 / UI-1.178（源码、静态、真实 Qt 离屏验证与打包完成，2026-08-12）：将工作区滚动提示升级为
  “位置 · 动作”语义，顶部/中段/底部/无滚动分别显示 `顶部 · ↓ 向下查看`、`中段 · ↕ 上下滚动`、
  `底部 · ↑ 返回顶部`、`全显 · 内容已全部显示`；accessible name 改为“工作区滚动位置”，只复用
  native vertical scrollbar，不新增状态源、timer、scroll owner、业务或 OTA/debug coupling。三主题×
  980/1240、四 workspace 状态矩阵通过，横向 scroll 为 0，最长文案 89px、提示 geometry 112×22；
  真实 MotionController 生命周期的正常、暂停、禁用、隐藏、恢复、关闭路径通过。架构师和独立 reviewer
  调用在等待窗口内超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。嵌入式
  C/C++ applicability 为 N/A。`local-arch-105` onefile 已覆盖 canonical、根目录和 root-latest；三者均为
  `48,018,946` bytes，SHA-256 为 `02D61927294C1BEFC4A23F854125A478EE35CA0903A15A6BBD933B7D314665ED`，
  archive listing SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，
  provenance verify 通过；`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-104 / UI-1.177（源码、静态、真实 Qt 离屏验证与打包完成，2026-08-12）：将命令管理空态改为页面
  唯一 vertical stretch owner；空态组件内部使用 expanding policy、垂直居中 glyph 和 copy 前后
  stretch，解决组件挤在顶部与无语义空白并存的问题。命令状态投影、批量执行、scroll owner、
  120Hz shared clock 和 OTA/debug 独立边界不变。三主题×980/1240、四 workspace 无 sibling
  overlap；focus command page `1240×820` empty state `(14,151,1176,399)`，scroll hint 为
  `complete`。1ms/elapsed 调度试验实测无收益并已撤回，交付仍为 8ms/120Hz target；主循环采样
  `116 frames`、均值 `8.536ms`、约 `117.16Hz`，不宣称显示器 120fps。架构师多次调用在等待窗口
  内超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。嵌入式 C/C++ applicability
  为 N/A。`local-arch-104` onefile 已覆盖 canonical、根目录和 root-latest；三者均为 `48,018,981`
  bytes，SHA-256 为 `58421049705382E4CEB343B6B2414D7B329A0957966AE02D62966B59C6423E29`，archive
  listing SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance
  verify 通过；`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

- ARCH-103 / UI-1.176（源码、静态、离屏验证与打包完成，2026-08-12）：复用已有
  `workspaceShell[mode="focus|overview"]` 动态属性，为路线条增加模式化主题层级。专注设置使用
  info→history 渐变和 accent-blue 上沿，总览保留 neutral 上沿；不改变 31px 路线条高度、Tab
  顺序、焦点路径、滚动提示或任何 transport/session 状态。三主题下 focus/overview 实跑均显示
  正确按钮文案和 `mode`，980/1240、四 workspace 横向滚动为 0；共享 MotionController 0.5 秒
  采样均值 `8.41ms`、有效采样约 `118.8Hz`，仍只作为 120Hz scheduler 证据，不宣称真实显示器
  120fps。架构师 `019ff3ac-0f16-7633-8304-1cb3a1cecb54` 与独立 reviewer
  `019ff3b1-8e10-7b63-94c3-af41db79e1a5` 等待窗口超时关闭，未形成外部结论；父代理完成五轴
  review 与行为保持简化评估。`local-arch-103` onefile 已覆盖 canonical、根目录和 root-latest；
  三者均为 `48,016,499` bytes，SHA-256 为
  `B51F8C0D0402CF692F41DC16CCAD70E183F2C764850DCAA08890CF7709292CEE`，archive listing SHA-256
  为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。

- ARCH-102 / UI-1.175（源码、静态、离屏验证与打包完成，2026-08-12）：修复紧凑顶栏的垂直
  密度，并修复专注设置↔总览过渡期间 `QTabWidget` 视口逐帧压缩导致的组件挤压。`workspace.py`
  作为 header owner 在 980px 宽度收敛顶栏上下留白、控制簇内边距与圆角，1240px 宽度保留完整
  装饰；`workspace_focus_transition.py` 先结算最终布局，再只对新显现 surface 做 opacity 过渡，
  不再把 `maximumHeight` 交给动画。真实中间帧 25/55/95/140ms 的 sibling 几何均无重叠，
  980×720 连接页 focus viewport 稳定为 514px；三主题×980/1240 横向滚动均为 0，可访问性通过。
  共享 `MotionController` 仍为 120Hz target/8ms PreciseTimer，1 秒离屏采样 119 frames、均值
  8.27ms、有效采样约 120.9Hz；最大间隔 17.08ms 记录为 offscreen 调度抖动，不宣称真实显示器
  120fps。架构师 `019ff394-bb5d-7c60-9ce6-c60b65f1ac83` 与 Terra 布局复核调用在等待窗口内
  超时关闭，未形成外部结论；独立 reviewer `019ff39b-8db0-7cc1-833e-009e2d0129cb` 同样
  超时关闭，未伪造 PASS；父代理完成五轴 review 与行为保持简化评估。`local-arch-102` onefile
  已覆盖 canonical、根目录和 root-latest；三者均为 `48,019,127` bytes，SHA-256 为
  `73593393356CC3938291E62D4DD5F66DBE09B3AA300C64AB6BA65BA55A6200E7`，archive listing SHA-256
  为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。

- ARCH-101 / UI-1.174（源码、静态、离屏验证与打包完成，2026-08-12）：将 page fade、theme root
  fade+sweep、transport panel fade 和两个自定义 dialog fade 全部迁移到各自的
  `MotionDrivenAnimationGroup` finite owner；所有 owner 复用唯一 `MotionController.frame_changed`，
  不启动独立 Qt animation clock。修复 theme sweep 原先可能独立 `.start()` 的双时钟风险，driver
  对中途失效 target 做 stop/finish 保护并收窄 track API；页面、主题、transport、dialog、focus 的
  连续活动窗口分别采样约 `8.13–9.89ms`，三主题、四 workspace、六 transport、dialog hide 和窗口
  hide 清理通过。首次 cadence 脚本遇到“当前 transport 未变化”和“不可见 panel 不启动”属于脚本前置
  条件错误，产品代码未因该脚本失败而修改；修正顺序后复跑通过。架构师建议保留“每功能一个 owner、
  全窗口一个共享时钟”，独立 reviewer 调用超时关闭，父代理完成五轴 review 与行为保持简化评估。
  `local-arch-101` onefile 已覆盖 canonical、根目录和 root-latest；三者均为 `48,019,302` bytes，
  SHA-256 为 `CAABF434DE8A04B49249775B36C897F1AA73B71CAAAD3CE5A57F070DB1C8B558`，archive listing
  SHA-256 为 `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。

- ARCH-100 / UI-1.173（源码、静态、离屏验证与打包完成，2026-08-12）：修复专注设置↔总览过渡
  使用 Qt 默认 `QPropertyAnimation` 时约 35Hz 的实际属性更新问题。新增
  `presentation/motion_transition.py`，保留 Qt easing/interpolation contract，但由唯一
  `MotionController.frame_changed` 以 8ms、120Hz target 驱动专注模式的高度和 reveal opacity track；
  不新增 timer、thread、业务状态、scroll owner 或设备 I/O。专注进入/退出、快速反转、显式 stop、
  hide、reduced-motion、pause、resize、close 及三主题/四 workspace 布局回归通过；最新真实组合根
  活动帧间隔均值为 `9.04ms`，不把 offscreen scheduler 误称为真实显示器 120fps。其他一次性 page/theme/dialog fade
  暂保留各自 owner，后续迁移必须另开切片并复用同一 driver。架构师与独立 reviewer 调用超时关闭，
  未形成外部结论；父代理完成 correctness/architecture/security/performance/readability 五轴 review
  与行为保持简化评估。`local-arch-100` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
  `SerialForge-latest.exe`；三者均为 `48,018,575` bytes，SHA-256 为
  `F9793EA0A9B9A9C727ABE0E888D4B5598A70DAD23B1D6D8AEDB9451F3A06C255`，archive listing SHA-256 为
  `EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。

- ARCH-99 / UI-1.172（源码、静态、离屏验证与打包完成，2026-08-12）：修复最小窗口总览下连接页
  被实时观测、终端和发送区挤到只剩顶部的问题。`workspace_runtime.py` 将紧凑高度策略、连接页
  focus override 和 resize/tab 同步集中在一个 presentation owner；980×720 新窗口、宽屏缩到
  紧凑高度都会给连接配置页完整首屏，用户显式“返回总览”后不会被后续 resize 或 Tab 往返抢回，
  1240×820 仍保留原有总览行为。未新增 timer、thread、业务状态、scroll owner 或设备 I/O；三主题
  六种传输横向滚动为 0，Ruff、compileall、`scripts/check.ps1` 通过。`local-arch-99` onefile
  已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`，三者均为 `48,016,245`
  bytes，SHA-256 为
  `3A44AC36190D0A275815D2CB0674B81C86C141F03B1E350EB105B33D9D4AC173`，provenance verify 通过。

- ARCH-98 / UI-1.171（源码、静态、离屏验证与打包完成，2026-08-12）：修复共享动作按钮的尺寸
  契约冲突。`ActionRailButton` 统一拥有 `36px` 内容安全最小高度；命令空态、组件空态和终端
  空态不再各自写 `setFixedHeight(30)`，避免当前主题字体下出现 `minimumHeight=36`、
  `maximumHeight=30` 的矛盾约束。三主题四 workspace 横向滚动均为 `0`，相关按钮恢复为
  `min=36 / max=16777215`；共享 MotionController 仍为 `120Hz` 目标、`8ms` PreciseTimer，
  1 秒离屏采样 `119Hz`。Ruff、compileall、`scripts/check.ps1` 通过；`local-arch-98` onefile
  已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`，三者均为
  `48,015,845` bytes，SHA-256 为
  `A4E63A9E17EC00DECB51ABB4B93EC3A2FABD2D8FB580843BFAD1FFB9C92006AB`，provenance verify 通过。

- ARCH-97 / UI-1.170（源码、静态、离屏验证与打包完成，2026-08-12）：修复扩展工具站能力卡的
  QSS 最小高度优先级回归。`embedded_extension_panel.py` 已有 `122px` 内容安全下限，但后置通用
  `QPushButton` 规则使实际卡片约 `83px`；在专属 extension selector 中显式保留高度后，三主题 ×
  980/1240 的 7 张卡实际最小高度为 `144px`，横向滚动仍为 0，卡片选择/详情/accessibility、
  OTA/debug contract-only/attach-only、120Hz MotionController 均不变。Ruff、compileall、
  `scripts/check.ps1` 通过；`local-arch-97` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
  `SerialForge-latest.exe`，三者均为 `48,015,901` bytes，SHA-256 为
  `5599524451084AF460D267FDAF87049C83F94261CD89EAE940769F2983484BE6`，provenance verify 通过。

- ARCH-96 / UI-1.169（源码、静态、离屏验证与打包完成，2026-08-12）：将协议最大帧长、UDP
  最大报文、TCP Server 最大客户端数和批量命令步骤延时改为不可编辑有限整数选项；端口保留自由
  输入。新增共享 numeric selector 的范围/旧值兼容/动态协议上限边界，修正 clear/rebuild signal
  fan-out 和 980px 宽度扩张问题。四个 workspace 逐页横向滚动为 0，网络/协议/命令离屏交互通过；
  Ruff、compileall、`scripts/check.ps1` 通过。架构师/独立 reviewer 超时关闭，父代理完成五轴复核
  与简化评估；`local-arch-96` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
  `SerialForge-latest.exe`，三者均为 `48,013,765` bytes，SHA-256 为
  `192CED543D4860E8DE2659377A67B5CC3E539D37AE62066D242C4E61F9EC9DBC`，provenance verify 通过。

- ARCH-95 / UI-1.168（源码、静态、离屏验证与打包完成，2026-08-12）：将 UART、TCP/UDP/RTT、
  BLE 共八个超时 `QDoubleSpinBox` 收敛为 presentation-only `BoundedFloatCombo`，用户直接选择
  `ms/s` 选项，不再手填小数；`.value()`/`.setValue()`、默认值、范围裁剪和 runtime DTO 读取契约
  保持。选项 catalog、composition factory、typed bindings 分层，未新增业务状态、timer、线程、
  scroll owner 或 OTA/debug coupling。Ruff、compileall、`scripts/check.ps1`、真实组合根离屏验证和
  首屏截图检查通过；架构师/独立 reviewer 调用超时关闭，父代理完成五轴复核与简化评估；onefile
  `local-arch-95` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`；三者
  均为 `48,011,368` bytes，SHA-256 为
  `3CDC74EDDE5ED914E76436972FE4FB466A0AC754575F12D834A393C8C05D2E2E`，archive listing SHA-256 为
  `39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-94 / UI-1.167（源码、静态/离屏验证与打包完成，2026-08-12）：修复专注设置
  切回总览时 Tab 区与实时观测/终端/发送区并行变高造成的拥挤中间帧；仍使用既有一次性
  `QParallelAnimationGroup` 和 220ms 实测高度目标，仅为三块 reveal surface 添加临时 opacity
  effect，并在完成、反转、静态、隐藏、最小化、resize、关闭路径统一清理。唯一
  `MotionController`、`TARGET_HZ=120`、8ms scheduler、业务状态、Tab/focus/accessibility 和
  OTA/debug 边界不变。Ruff、compileall、`scripts/check.ps1`、source-limit、theme audit 及
  `ARCH94_FOCUS_MOTION_PASS` 通过；架构师和独立 reviewer 调用超时关闭，父代理完成五轴复核与
  简化评估。`local-arch-94` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
  `SerialForge-latest.exe`，三者均为 `48,009,747` bytes，SHA-256
  `BE74331C2BFAE11F0DDBD2DC5CC2D89D9581AFD08ACAA125CCE67C1516E283C3`，archive listing SHA-256
  `032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-93 / UI-1.166（源码、静态/离屏验证与打包完成，2026-08-12）：首屏自动预选安全的
  `UART · 115200 8N1` 内置 profile，填入已有 UART 选项但不自动连接；用户仍需选择端口并显式
  点击连接。手动切换 TCP/UDP/BLE/RTT 时清理不匹配旧 preset/context；7 个内置 profile、4 个
  workspace、980px 横向 scroll、accessibility 和 session gate contract 通过。未新增业务状态源、
  timer、线程或硬件访问。`local-arch-93` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
  `SerialForge-latest.exe`，三者均为 `48,008,840` bytes，SHA-256
  `14B679D1F92E03BCB1F189E3DD9012DF8D7C0ACE3C0934D75B5C285F0DC13E9A`，archive listing SHA-256
  `032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-92 / UI-1.165（源码、静态/离屏验证与打包完成，2026-08-12）：修复滚动页短内容
  被垂直拉伸造成的空白段，连接带/UART panel 改为水平扩展、垂直固定的自然高度；共享动效
  保持唯一 `MotionController`，以 `TARGET_HZ=120`、8ms PreciseTimer 和 0.96 nominal frame
  budget 实现约 120Hz scheduler cadence，并让静态 surface 退出逐帧 fan-out。980/1240 连接页
  可见 scroll 横向范围为 0，连接带 138px，离屏样本 120.83Hz；Ruff、compileall、check、source
  limit、theme audit 和状态/隐藏生命周期 contract 通过。GUI/EXE 启动、真实显示器 FPS、硬件/HIL、
  连接/OTA/debug 与签名待授权。`local-arch-92` onefile 已覆盖 canonical、根目录
  `SerialForge.exe` 和 `SerialForge-latest.exe`，三者均为 `48,008,038` bytes，SHA-256
  `A2C75788258BB1AE19B23E881049C1AD6F902DD069C78F7E926F15CEF92FD6E8`，archive listing SHA-256
  `032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-91 / UI-1.164（源码、静态校验与打包完成，2026-08-12）：将
  `viewmodels.py` 中四个 discovery/BLE Qt worker 拆到 `presentation/viewmodel_jobs.py`，由新模块
  独占 blocking job 与结果 signal；`SessionViewModel` 继续独占全局线程池、两个取消 Event、busy/
  closing 状态、回调和 30 个 Qt signal，公共方法与生命周期不变。Ruff、compileall、
  `scripts/check.ps1`、30-signal/2-job contract、source-limit、theme audit、provenance verify 和
  三 artifact hash equality 通过；GUI/EXE 启动、真实 FPS、硬件/HIL/签名待授权。`local-arch-91`
  onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`，三者均为
  `48,007,307` bytes，SHA-256 `ED765A8442C2DAB1FCD11AC36FC78E53032EDCD7CC17E90EBAB96915C932932E`，
  archive listing SHA-256 `032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-90 / UI-1.163（源码、静态校验与打包完成，2026-08-12）：将共享动效 catalog、可见性
  snapshot、frame fan-out 和 live-RX activity projection 从接近 1000 行的 `lifecycle.py` 迁移到
  `controllers/lifecycle_motion.py`；`lifecycle.py` 继续兼容导出既有符号，`workspace.py`、
  `terminal_runtime.py`、`workspace_runtime.py` 调用路径不变。MotionController 唯一时钟、
  `TARGET_HZ=120`、8ms `PreciseTimer` target、phase/stop/rearm/close 语义保持。Ruff、compileall、
  `scripts/check.ps1`、motion re-export/cadence contract、source-limit、theme audit、provenance
  verify 通过；本轮未启动 GUI/EXE，真实窗口/显示器 FPS/硬件/HIL/签名待授权。`local-arch-90` onefile
  已覆盖 canonical、根目录和 root-latest，三者均为 `48,005,887` bytes，SHA-256
  `E3EAE6FCC3CEB46C498EBB3094B70551211C80168BD54D327B4FCE178CD1EF9E`，archive listing SHA-256
  `0FC3A33559DC02B0C14313BE9A76FDE3C0FAA881A5C7861C9DFE146A0307775A`，签名 `NotSigned`，
  `release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-89 / UI-1.162（源码、静态校验与打包完成，2026-08-12）：协议/遥测页保留现有
  `ProtocolPanelWidgets` 与 callbacks 契约，将协议细节从 7 列网格改为三字段语义行，Dataset
  配置状态/动作、历史回放速度/动作分行，并统一四个 surface 的垂直节奏；未新增 timer、业务
  状态、线程、transport 或第二个 scroll/motion owner。Ruff、compileall、`scripts/check.ps1`、
  37-field import contract、source-limit、theme audit、provenance verify 通过；本轮未启动 GUI/EXE，
  真实几何、键盘焦点、显示器 FPS、硬件/HIL 与签名待授权。`local-arch-89` onefile 已覆盖 canonical、
  根目录和 root-latest，三者均为 `48,004,714` bytes，SHA-256 `1D08E351935E0CA828C5AB095CD0712042F3C5F1DE6F0BD291595D508A6D3A44`，
  archive listing SHA-256 `A152062F579FF9CCA9D2421EFDC6FA5D2FA6918A0691BE46EEE359AE451A47FF`，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-88 / UI-1.161（源码与静态校验完成，2026-08-12）：命令管理页新增
  `command_workspace_builder.py`，将批量命令下拉选择与新建/编辑/删除/执行/停止动作拆成两行，
  `terminal.py` 收敛为错误通知 owner。保留 `CommandBatchControlBindings`、所有 callbacks、空态
  CTA、状态/结果 projection、外层 `commandPage` scroll、Tab 顺序和 close/motion 生命周期。全量
  Ruff、编译、import、source-limit、theme audit 通过；本轮未启动 GUI/EXE，真实几何和键盘焦点待授权。
  `local-arch-88` onefile 已覆盖 canonical、根目录和 root-latest，三者均为 `48,002,961` bytes，
  SHA-256 `EC5E4DE1DDDB4537FF3CA00B14542D71736C0DDBA2AA27C6275DE9369098C609`，archive listing SHA-256
  `A152062F579FF9CCA9D2421EFDC6FA5D2FA6918A0691BE46EEE359AE451A47FF`，provenance verify 通过，签名
  `NotSigned`，`release_eligible=false`。

- ARCH-87 / UI-1.160（源码与静态校验完成，2026-08-12）：将实时观测和实时发送 band 从固定多列
  `QGridLayout` 拆为 `terminal_toolbar_builder.py` / `send_bar_builder.py` 两个高内聚 owner，采用
  “输入/控制行 + 状态/动作行”布局，发送输入拥有伸缩优先级，状态与快捷动作不再挤占输入行；
  `terminal.py` 仅保留错误、历史、批量命令 surface。`TerminalControlBindings`、所有 callback、
  dynamic properties、focus/accessibility、shared MotionController 和关闭路径保持。静态、编译、
  Ruff、import、source-limit、theme audit 通过；本轮未启动 GUI/EXE，真实几何和显示器 FPS 待授权。
  `local-arch-87` onefile 已覆盖 canonical、根目录和 root-latest，三者均为 `48,002,962` bytes，
  SHA-256 `860643732A32225DAA19139459DA4DE1DF9027340F9EA313266FD7696DAEF79B`，archive listing SHA-256
  `5E1BAFBD5AC2AAEF5D78AD61BEB3551D8710F34B39AF662B889964FB8BA107C1`，provenance verify 通过，签名
  `NotSigned`，`release_eligible=false`。

- ARCH-86 / UI-1.159（源码与静态校验完成，2026-08-12）：降低头部 `SignalFieldWidget` 的视觉
  堆叠，将 5 个移动点与密集网格改为 3 个均匀轨道点，同时保持共享 `MotionController` 的
  `TARGET_HZ=120 / PreciseTimer / 8ms` 契约；将 TCP/UDP/RTT 面板移到独立
  `controllers/network_builder.py`，抽出无状态连接视觉 primitives，保留 `NetworkControlBindings`、
  window 引用和 endpoint callback。`ruff`、`compileall`、`scripts/check.ps1`、import contract 通过；
  本轮尚未启动 GUI/EXE，真实窗口几何、显示器刷新率、硬件和正式发行验收待授权。`local-arch-86`
  onefile 已覆盖 canonical、根目录和 root-latest，三者均为 `48,001,801` bytes，SHA-256
  `F170A3893B519061953F24A414F9848B8B4D6D862011EDE50C3DA405C70EB280`，provenance verify 通过，签名
  `NotSigned`，`release_eligible=false`。

- ARCH-85 / UI-1.158（源码、空态可见性、真实组合根验证完成，2026-08-12）：收敛协议/组件/Dataset
  的重复空白表面；无协议帧时只保留组件空态，无 Dataset 样本时隐藏空预览，有数据后自动恢复。
  980×720 实测 `horizontal_max=0`、`component_preview_visible=False`、`component_empty_visible=True`、
  `dataset_preview_visible=False`、`vertical_max=778`，注入合法帧后组件预览恢复；既有共享动效
  `TARGET_HZ=120 / PreciseTimer / 8ms` 1 秒采样 124 帧。父级 fresh-pass 无 Required finding，
  架构师独立调用本轮服务超时并已关闭，未伪造 PASS；`local-arch-85` onefile 已覆盖 canonical、
  根目录和 root-latest，provenance、启动/关闭烟测通过，签名 `NotSigned`，`release_eligible=false`。

- ARCH-84 / UI-1.157（源码、BLE owner 拆分、静态审计、真实组合根验证完成，2026-08-12）：将 BLE GATT
  7 列密集 QGridLayout 重排为“扫描与筛选 / 设备发现与连接 / GATT 特征与收发”语义字段分组，新增
  `controllers/ble_builder.py`，`connection_builder.py` 只接入原 `BleControlBindings`。保留所有控件
  字段、默认值、`BleGattWriteMode` itemData、callback、Tab/accessibility、手动扫描/显式连接和无自动
  重连边界，不新增 timer、backend 或硬件动作。980×720/1240×820、三主题、field overlap=0、
  horizontal maximum=0、exact-white=0、reduced-motion/close、Ruff、compileall、source-limit、theme-audit
  通过；`local-arch-84` onefile 已覆盖根目录并写入交接记录。

- ARCH-83 / UI-1.156（源码、架构复核、独立五轴/简化复核、真实组合根验证完成，2026-08-12）：将
  扩展工具站 7 张 contract-only/attach-only 能力卡改为完整 `QPushButton` 语义的单选展示卡，点击、
  焦点、Enter/空格共用同一选择路径；新增独立 `ExtensionCapabilityDetail` 叶子，在卡片前展示
  title/group/reference/key/state/summary/boundary 与固定只读 guard。`ExtensionPanelWidgets` 不暴露
  详情内部控件，panel 仍是唯一组装 owner；不新增 backend、timer、设备 I/O 或 OTA/debug 执行路径。
  980×720/1240×820、三主题、7 cards、focus/click/key、DTO 不变、horizontal maximum=0、exact-white=0、
  reduced-motion/close、Ruff、compileall、source-limit/theme-audit 均通过；`local-arch-83` onefile
  覆盖根目录与交接记录，签名 `NotSigned`、正式发行资格 `false`、硬件验收 `not_run`。

- ARCH-82 / UI-1.155（源码、架构复核、独立只读审查、运行时矩阵完成，2026-08-12）：将共享动效
  fan-out 改为完整 catalog + 可见 snapshot，避免每帧扫描可见性；新增 host visibility rearm state，
  show/restore 经过两跳 queued lifecycle fence 后重建唯一 `PreciseTimer`，pause/reduced-motion 不被
  绕过。56 surface、980/1240 × 三主题 × 四 Tab、focus、idle/activity、pause/reduced/hide/minimize/
  restore/close 均完成非破坏性 offscreen 验证；稳定首显 idle/activity 约 125 帧/s、p50 约 8ms，恢复
  路径受当前 offscreen/Qt 15.6ms 调度粒度影响，未宣称真实显示器 120Hz。`local-arch-82` onefile
  包含在根目录覆盖和交接记录中，签名 `NotSigned`、正式发行资格 `false`、硬件验收 `not_run`。

- ARCH-81 / UI-1.154（源码、静态审计、真实组合根验证完成，2026-08-12）：将 UART 参数从八列
  `QGridLayout` 重排为端口/波特率、数据格式、超时/线路控制三行语义字段；复用
  `form_fields.py` helper，保留 `UartControlBindings`、itemData、默认值、信号、Tab 和
  accessibility 契约。`ARCH81_FIELD_GEOMETRY_PASS`、`ARCH81_TAB_ORDER_PASS`、
  `ARCH81_TYPED_VALUE_PASS`、980/1240 `horizontal_max=0`、三主题、close、Ruff、compileall、
  source-limit/theme-audit 均通过；独立只读复核 PASS / Required=0；本轮未改 MotionController，
  120Hz 仍记录为 scheduler target。`local-arch-81` onefile 已覆盖 canonical/root/root-latest，
  三者均为 `47,985,982` bytes，SHA-256 `81C8993A8D7A8723B074FD0BD37BC7D23822B9BB84070AE12C6E08F26D07B6DA`，
  provenance/root startup-shutdown 通过，签名 `NotSigned`，`release_eligible=false`，
  `hardware_acceptance=not_run`。

- ARCH-80 / UI-1.153（源码、架构审阅、真实组合根视觉/生命周期验证完成，2026-08-12）：将
  顶部 header 的窄窗口策略收敛到 `workspace.py` 的 `_AdaptiveHeader` 自身几何入口；当 header
  logical width `<1120` 时只隐藏纯装饰 `SignalFieldWidget` 与 `ThemePaletteSwatch`，controls
  row spacing 从 8px 收到 6px，所有 status/motion/theme 语义控件、主题 combo、键盘/无障碍和
  shared MotionController 保持不变，宽屏原位恢复。三主题、980/1119/1120/1240 边界、980/1240
  × 四 Tab 可见 horizontal maximum=0、settled screenshots、pause/hide/show/close 通过；
  `TARGET_HZ=120 / PreciseTimer / 8ms` 配置保持，本轮 offscreen 回调均值 `15.607ms`，未宣称
  精确 120fps。Luna 线程先超时后返回“不新增 helper、workspace 单 owner”结论，Terra 升级线程
  返回“可做但建议 helper”结论；本轮选择 workspace-local owner，记录冲突，不伪造一致审阅。
  `local-arch-80` onefile 已覆盖 canonical/root/root-latest，三者均为 `47,987,629` bytes，SHA-256
  `CB829EBBF27D9C48E1993040E28E07A57A9092EE3EBDC2A1BF32082A9D30F598`，archive listing SHA-256
  `A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073`，provenance verify、root
  EXE startup/shutdown pass，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-79 / UI-1.152（源码、架构审阅、真实组合根验证与 onefile 交付完成，2026-08-12）：修复
  命令管理页空批量命令状态下的垂直拉伸，在 `CommandBatchEmptyState` 后增加 trailing stretch，
  使空态从 451px 收敛到 108px（1240×820、980×720），保留 CTA、signal、batch state、
  accessibility、scroll owner 和 shared motion。三主题 × 四 Tab × 两尺寸可见 horizontal overflow=0；
  activity、pause、hide/show、close 通过。架构师结论为 trailing stretch 最小且合适，调度器不改；
  当前真实离屏记录 `TARGET_HZ=120 / PreciseTimer / 8ms`，回调均值 `15.748ms` 作为平台边界，
  未宣称精确 120fps。`local-arch-79` onefile 已覆盖 canonical/root/root-latest，三者均为
  `47,984,076` bytes，SHA-256
  `085B1BEF964C3E2861183950E28689DAC798E853CA8A2C38E26A279144C324D9`，archive listing SHA-256
  `A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073`；provenance verify pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-78 / UI-1.151（源码、真实组合根验证与 onefile 交付完成，2026-08-12）：将共享动效分为
  ambient 与 activity-only 两级。`MotionController` 继续保持单一 8ms/约 120Hz scheduler，新增
  只读 `activity_active()`；28 个 action/空态 surface 仅在短时 activity 或 transition 中接收帧，
  `AnalysisStatusLabel` 保持持续语义动效。lifecycle 负责下降沿 stop、隐藏页一次性 stop 和全局
  cleanup，不改变业务状态、callbacks、typed itemData、焦点/accessibility 或 OTA/debug 边界。
  `MOTION_ACTIVITY_SURFACES total=28 visible=4`、activity on/off、hidden route、route recovery、
  pause/resume、hide/show、close、三主题 × 四 Tab 可见 horizontal overflow=0、source-limit、
  theme-audit、compileall、Ruff 均通过；架构师 post-implementation 只读 review `PASS / Required=0`、
  Optional=2（首帧未知→静态的一次冗余 stop、single motion timer 术语限定）；两轮外部 Luna/max
  独立审查线程因服务侧超时未返回，未伪造独立通过，父代理完成五轴 review 与简化评估并记录该限制。
  `local-arch-78` onefile 已覆盖
  canonical/root/root-latest，三者均为 `47,986,125` bytes，SHA-256
  `BA907F2587004E2FE797B0AC6397B286AF034929DD05C888964C27BC3B4EC8B1`，archive listing SHA-256
  `A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073`；provenance verify pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- UI-1.135：扩展工具站接入概览消费 application-owned 分组摘要，展示 OTA 传输、OTA 安全和调试输出
  三类能力槽位；presentation 只渲染 immutable DTO，六列 metrics 在超过六项时自动换行，不新增业务状态、
  timer、后端动作或 OTA/debug 依赖。

- UI-1.137（源码、真实组合根 vector、视觉复核与 onefile 已完成，2026-08-11）：在 workspace
  route strip 增加只读当前页上下文表面；它只消费真实 Tab index，通过 typed
  `WorkspaceShellBindings` 由 workspace owner 组装，并使用 base/variant 对称主题 token。
  三主题 × 980/1180 × 四 Tab 共 24 组通过，horizontal maximum=0、exact-white=0、near-white=0；
  `scripts/check.ps1`、compileall、Ruff、source-limit、provenance pass。onefile source revision
  `local-ui-1.137`，canonical/root/root-latest `47,961,992` bytes，SHA-256
  `7509738A8CD05655E8899497DCB67143A05C34999189426B3A2482EF1961829B`，archive listing SHA-256
  `12D69406BC04C668C3621D831004A420FFFD331AA3EB409479472BF10A93C2C4`；签名 `NotSigned`，
  `release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6r（源码、真实组合根 vector 与架构边界已完成，2026-08-11）：新增 frozen/slots
  `UartControlBindings`，由 `connection_builder.py` 唯一组装；连接、preset、协议 timing、
  生命周期、组合和终端 runtime 通过 `uart_bindings_for()` 消费。保持 UART preset、连接 gate、
  focus/accessibility、section 显隐和三主题语义不变；`ARCH6R_UART_VECTOR_PASS 24`、
  `hmax=0`、exact-white/near-white=0、compileall、Ruff、scripts/check.ps1 和源码行数门禁通过。
  onefile/root/root-latest 已以 `local-arch-6r` 更新：canonical/root/root-latest 均为 `47,963,397`
  bytes，SHA-256 `BF0EDF73CC7BDFBA5567DC9CC6A55D8365F6979D65BDEF56EB55637790095529`，archive listing
  SHA-256 `C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，provenance verify pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6s（源码、真实组合根 vector 与架构边界已完成，2026-08-11）：新增 frozen/slots
  `NetworkControlBindings`，由 `connection_builder.py` 唯一组装 TCP Client/Server、UDP、RTT
  共用网络端点 panel；connection/runtime、preset、composition、lifecycle、commands 和
  terminal runtime 通过 `network_bindings_for()` 消费。保持 preset、allowlist readiness、
  LAN 确认失效、peer 发送目标、默认端点、连接 gate、focus/accessibility 与三主题语义不变；
  `ARCH6S_NETWORK_VECTOR_PASS 30`、hmax=0、exact-white/near-white=0、compileall、Ruff 和
  owner-only 审计通过。onefile/root/root-latest 已以 `local-arch-6s` 更新：canonical/root/root-latest
  均为 `47,965,543` bytes，SHA-256 `E9F3D230B75FFA54D61F5190556CE3393F308A4CC75E53B303DEA01E74BD71CA`，
  archive listing SHA-256 `C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，
  provenance verify pass，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6t（源码、真实组合根 vector 与架构边界已完成，2026-08-11）：新增 frozen/slots
  `BleControlBindings`，由 `connection_builder.py` 唯一组装与接线；BLE selection、action、
  connection/runtime、preset、composition、commands、lifecycle 和 terminal runtime 通过
  `ble_bindings_for()` 消费。通知 pending/ref/timer、设备快照与 backend handle 仍留在原 owner，
  不进入 bundle；`ARCH6T_BLE_VECTOR_PASS 30`、hmax=0、exact-white/near-white=0、compileall、
  Ruff 和 owner-only 审计通过。onefile/root/root-latest 已以 `local-arch-6t` 更新：canonical/root/root-latest
  均为 `47,968,345` bytes，SHA-256 `019CB04DE1400D4A03F5B51C662C31B7A4C25195EB83C844FA1EB8ECD0B23024`，
  archive listing SHA-256 `C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，
  provenance verify pass，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6u（源码、真实组合根 vector 与架构边界已完成，2026-08-11）：新增 frozen/slots
  `TerminalControlBindings`，由 `bootstrap.py` 在 terminal/send builder 完成后唯一组装；terminal
  runtime、connection、commands、command selection、send context、composition、lifecycle 和
  focus transition 通过 `terminal_bindings_for()` 消费。preview/history/quick snapshot、recording/
  session state、timer、MotionController 和 callbacks 仍留在原 owner；`ARCH6U_TERMINAL_VECTOR_PASS 24`、
  hmax=0、exact-white/near-white=0、compileall、Ruff 和 owner-only 审计通过。onefile/root/root-latest
  已以 `local-arch-6u` 更新：canonical/root/root-latest 均为 `47,969,870` bytes，SHA-256
  `485AFE4B7A581E27154BFA1043290D872FDB862CA61F35219945D88FA72839A8`，archive listing SHA-256
  `E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876`，provenance verify pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6v / UI-1.138（源码、真实组合根与视觉复核已完成，2026-08-11）：`MotionController` 改为
  120Hz 目标的 PreciseTimer 8/9ms cadence，phase 使用 elapsed time，lifecycle 只刷新 visible
  surface 并对装饰 rail 做确定性错峰；实时观测/发送/历史布局去除固定提示宽度反向撑列，补齐
  bounded spacing/row height，默认窗口 1240×820、最小高度 720。`MOTION_LAYOUT_STATIC_PASS`、
  `UI138_MIN_SCREENSHOT_PASS`、`UI138_SCREENSHOT_PASS` 和 26 frames/240ms offscreen cadence
  通过；onefile/root/root-latest 已以 `local-arch-6v` 更新：canonical/root/root-latest 均为
  `47,972,295` bytes，SHA-256 `C9ABCAE17940617A6C81A01F56B91BE09828FC5B996E6181176A82F8656A9DA5`，
  archive listing SHA-256 `E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876`，
  provenance verify pass，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6w / UI-1.139（源码与真实组合根 binding/layout vector 已完成，2026-08-11）：新增
  frozen/slots `CommandBatchControlBindings`，由 `bootstrap.py` 在 workspace command builder
  完成后唯一组装；commands、connection、command selection、composition 和 lifecycle 通过
  `command_batch_bindings_for()` 消费批处理 combo、动作按钮、状态 rail、结果表和空态。batch
  catalog/snapshot、ViewModel、执行策略、timer、callback 和 transport handle 留在原 owner，
  `terminal.py` 仅保留构建阶段兼容字段；跨 controller 动态批处理 widget 读取清零。
  `COMMAND_BINDINGS_STATIC_PASS`、`COMMAND_BINDINGS_RUNTIME_PASS`、`COMMAND_BINDINGS_LAYOUT_PASS`、
  scripts/check.ps1、compileall、Ruff、source-limit 和 theme-audit 通过；架构师已调用但超时未返回
  独立报告，父代理完成五轴及行为保持型简化审查；onefile/root/root-latest/provenance 已通过，canonical/root/root-latest 均为
  `47,974,086` bytes，SHA-256 `3BCACA7C90A39C23FF7E4C4605F021E463174AF2BE8032C75496C6E10A0F6327`，archive listing SHA-256
  `60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C`，签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6x / UI-1.140（源码、静态门、真实组合根与包交付已完成，2026-08-11）：新增 frozen/slots
  `ConnectionShellBindings`，由 `connection_builder.py` 在刷新 preset combo 前唯一组装；链路、
  runtime、preset、commands、composition、derived/protocol、replay、scope 和 lifecycle 通过
  `connection_shell_bindings_for()` 消费传输/快速配置/状态 rail/连接按钮。preset catalog/store、
  session/ViewModel、callbacks、timer、transport handle 和 policy 留在原 owner，builder 动态字段
  仅用于构建阶段。`scripts/check.ps1`、compileall、Ruff、source-limit、theme-audit、真实组合根
  shell identity、四页滚动、三主题/两尺寸 vector 通过；`local-arch-6x` onefile/root/root-latest
  已覆盖，canonical/root/root-latest 均为 `47,975,290` bytes，SHA-256
  `AE11EE6B8BED8F04CD844994B2FA60820232391084DDD57978B2CB2FB4D4BCE8`，archive listing SHA-256
  `60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C`，provenance pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6y / UI-1.141（源码、静态门、真实组合根与包交付已完成，2026-08-11）：新增 frozen/slots
  `HeaderChromeBindings`，由 `workspace.py` 在首次主题投影前唯一组装；lifecycle 与 composition
  通过 `header_chrome_bindings_for()` 消费 header 状态、品牌/信号场、动效控制和主题选择器。
  `app_root`、error bar、status footer、偏好、MotionController 和业务状态留在原 owner；
  `scripts/check.ps1`、compileall、Ruff、source-limit、theme-audit、980/720 与 1240/820、
  四页滚动、三主题、无重复 motion surface vector 已通过；`local-arch-6y` onefile 已覆盖
  canonical/root/root-latest，三者均为 `47,978,616` bytes，SHA-256
  `842BF2981DBC4EAFC47C30819904DD290BC087A16EB1565E633FD0C5C4028C8C`，archive listing SHA-256
  `FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`，provenance pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-6z / UI-1.142（源码、真实组合根与包交付完成，2026-08-11）：工作区在 Tab 路由变化时
  复用既有 focus transition；默认 index 0 保持总览，协议/遥测、命令管理、扩展/工具站自动收起实时观测、
  终端和发送区，为配置页提供完整首屏，返回链路页自动恢复。未新增时钟、splitter、业务状态或手动配置项；
  `scripts/check.ps1`、compileall、Ruff、三主题 × 980/1240 × 四 Tab、自适应焦点、无重叠、无横向滚动、
  reduced-motion、动效 120Hz cadence 真实组合根验证通过；离屏字体目录 warning 仍为既有环境提示，不代表
  Windows 系统字体缺失。`local-arch-6z` onefile 已覆盖 canonical/root/root-latest，三者均为
  `47,976,464` bytes，SHA-256
  `C7D8D93ED5907648BAF348BD560410BAF0348C5A02705F9880157E68FCD5C9CB`，archive listing SHA-256
  `FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`，provenance pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-70 / UI-1.143（源码、真实组合根验证与包交付完成，2026-08-11）：空终端“打开链路连接”
  CTA 改接 presentation-only `open_connection_setup()`，显式进入 index 0 connection focus；默认启动
  总览、普通 Tab route、连接 gate/session state、主题、reduced-motion 和 shared 120Hz 不变。点击 CTA
  后 connection viewport 达到 514px、下方三块隐藏，状态仍为 `closed`；返回总览和普通 Tab 路由通过。
  三主题 × 980/720、1240/820 × 四 Tab 自适应布局、无重叠、可见滚动区无横向溢出、低动效静态路径通过；
  cold-start 离屏 motion cadence 为 105 frames/1007ms（104.3 FPS），`PreciseTimer` 与 8/9ms slot 通过。
  compileall、Ruff、scripts/check.ps1、source-limit、theme-audit 通过。`local-arch-70` onefile 已覆盖
  canonical/root/root-latest，三者均为 `47,976,952` bytes，SHA-256
  `17F57F53EA2F3D49EB37E3E2581773F46F9528A243259D287E542EFE64FD3245`，archive listing SHA-256
  `FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`，provenance pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-71 / UI-1.144（源码、真实组合根验证与包交付完成，2026-08-11）：导航切换时如果既有 focus
  geometry transition 正在运行，`animate_workspace_transition()` 清理旧 page fade 并让位给 220ms
  主过渡；同一 focus mode 内的 Tab 切换仍保留 180ms page fade。进入 focus 时下方三块立即隐藏并清零
  约束，避免内部控件在过渡帧被压扁；返回总览沿用 bounded reveal。三主题 × 两尺寸 × 四 Tab、CTA、
  无挤压、快速反向、低动效、无重叠/可见横向溢出和 120Hz/8-9ms/56 surface 通过；compileall、Ruff、
  scripts/check.ps1、source-limit、theme-audit 通过。`local-arch-71` onefile 已覆盖
  canonical/root/root-latest，三者均为 `47,978,127` bytes，SHA-256
  `D3D73A8EFC8D4A1531C335CC1D5280C9F24A047A4484D4DBB523A1C1E715E5B6`，archive listing SHA-256
  `FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`，provenance pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-72 / UI-1.145（源码、真实组合根验证与包交付完成，2026-08-12）：新增 presentation-only
  `BoundedTextLabel`，对 header 的连接上下文、数据来源、连接状态按当前宽度做右侧省略，完整值保留在
  `full_text`、tooltip 和 accessible description；有界 size hint 与可收缩 minimum width 让 status cluster
  在 980px 保持约 496px、1240px 保持约 600px，不再把长端点文案扩成不可控 header。三主题 × 980/720、
  1240/820 × 四 Tab、CTA focus/no-squeeze、完整 tooltip/accessibility、无重叠、字体配置、
  `MotionController` 120Hz/PreciseTimer/8-9ms 通过；`ARCH72_SOURCE_METADATA_PASS` 确认来源值同时存在于
  full_text、tooltip 和 accessible description；`scripts/check.ps1`、compileall、Ruff、source-limit、
  theme-audit 通过。架构师只读线程已调用但本轮超时关闭，未伪造独立报告；父代理完成静态调用链、生命周期、
  行为保持型简化和视觉复核。`local-arch-72` onefile 已覆盖 canonical/root/root-latest，三者均为
  `47,980,962` bytes，SHA-256 `6925FDEAD6A98131F5ADD20BD46E9082DF5532AB3C733EAD4A67B2E2F2FD162C`，
  archive listing SHA-256 `C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732`，provenance pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-73 / UI-1.146（源码、真实组合根验证与包交付完成，2026-08-12）：实测主题切换时旧 sweep 为
  `180×797` 全高覆盖，视觉上会穿过 terminal/send 内容；只在 `theme_transition.py` 将一次性
  `ThemeTransitionSurface` 高度限制为 `min(220px, root.height())`，保留 root opacity fade、
  180ms sweep、stop/finish 清理和 reduced-motion/隐藏/最小化/关闭路径。三主题轮换、sweep bounded
  `180×220`、settled cleanup、三主题 × 980/720、1240/820 × 四 Tab、focus/no-squeeze、可见 scroll
  横向门禁、CTA session state 隔离和 `MotionController` 120Hz/PreciseTimer/8-9ms 通过；真实几何
  回归确认 980/720 的 terminal/send 顶部为 413/587、1240/820 的 terminal/send 顶部为 483/687，
  sweep 不覆盖两者且 cleanup 通过。首轮架构师只读线程已调用但超时关闭，随后独立 Luna/max 审查返回
  `risk=medium` 并记录 root 父级与 queued cleanup 残余风险，未把超时伪造成独立结论。`scripts/check.ps1`、compileall、Ruff、source-limit、
  theme-audit 通过；`local-arch-73` onefile 已覆盖 canonical/root/root-latest，三者均为
  `47,981,132` bytes，SHA-256 `DBD554999C65C18DDDC66643EA336CD90EFDF7B41DC43629194BAA044752F886`，
  archive listing SHA-256 `C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732`，provenance pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-74 / UI-1.147（源码、运行时验证与包交付完成，2026-08-12）：将 `start_theme_transition()` 的
  `appRoot` fade target 与 header chrome sweep host 设为显式 keyword-only 边界；host 必须属于 root 子树，
  overlay 只使用 header 本地 geometry。`MainWindow.resizeEvent()` 由 lifecycle 先幂等停止主题过渡，再调用
  Qt 基类布局，避免窗口 resize 期间使用旧 sweep 终点。独立审查 Required=0/Optional=0；架构师 conditional
  approve；`ARCH74_HOST_GEOMETRY_PASS`、`ARCH74_RESIZE_ABORT_PASS` 双向、`ARCH74_THEME_MATRIX_PASS`、
  `ARCH74_MOTION_GATE_PASS`、`ARCH74_THEME_LIFECYCLE_PASS`、`ARCH74_MOTION_PASS=120/PreciseTimer/8-9ms`、
  `scripts/check.ps1`、compileall、Ruff、source-limit、theme-audit 均通过；`local-arch-74` onefile 已覆盖
  canonical/root/root-latest，三者均为 `47,981,373` bytes，SHA-256 `42C2451F5315CB8273CEEA45985ED6E595CF7E495B8E26E8434B41B2B4B45E34`，
  archive listing SHA-256 `C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732`，provenance pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-75 / UI-1.148（源码、运行时验证与包交付完成，2026-08-12）：实测发现 focus 进入时只动画
  `tabs.maximumHeight` 的哨兵区间会在 `350→614px` 之间突跳；`workspace_focus_transition.py` 现在先保存
  当前真实高度，隐藏下方 overview surface 后激活 root layout 测量 focus 目标，再对真实高度区间做
  `maximumHeight` 动画；退出同样先测量总览目标。`lifecycle.resizeEvent()` 在 Qt 基类重排前停止 focus group，
  防止 resize 后继续写旧目标。独立最终审查 Required=0/Optional=0/FYI=1；`ARCH75_GEOMETRY_PASS`、
  `ARCH75_THEME_SIZE_PASS`（3 themes × 980/720、1240/820 × 4 tabs）、`ARCH75_RESIZE_ABORT_PASS`、
  `ARCH75_LOW_MOTION_PASS`、`ARCH75_LIFECYCLE_PASS`、`ARCH75_MOTION_SCHEDULE_PASS=120/PreciseTimer/8-9ms`、
  `ARCH75_SCREENSHOT_FONT_PASS`、`scripts/check.ps1`、compileall、Ruff、source-limit、theme-audit 均通过。
  架构师调用按要求执行但未在等待窗口内返回独立结论，未将其超时当作审查证据；父代理保持既有 presentation
  owner 边界并记录该事实。`local-arch-75` onefile 已覆盖 canonical/root/root-latest，三者均为
  `47,981,610` bytes，SHA-256 `6AA8BFE552E357D9FA88B05B8070E42A17CB9B535FD85B93866CEE3C383A1C7E`，
  archive listing SHA-256 `C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732`，provenance pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-76 / UI-1.149（源码、运行时验证与包交付完成，2026-08-12）：980×720 实测总览 workspace 只有
  `150px`，而 connection band 首个完整表面约 `149px`，造成首次启动连接表单裁切。新增 presentation-only
  `_connection_onboarding`：bootstrap 在 terminal bindings 完成后、初始 CLOSED 时启动既有 focus；workspace
  runtime 负责 connection Tab 的保持与 leave；首次 OPEN 且仍在 connection Tab 才返回总览，非连接 Tab 保持原有
  focus；手动返回清除 flag；ERROR/失败保留重试 focus。架构师 conditional approve；独立审查 Required=0、
  Optional=2、FYI=5，Optional 为重复 start 的幂等 guard 与 ERROR 切 Tab 的 UX 约定，均不构成阻塞。`ARCH76_ONBOARDING_PASS`、
  `ARCH76_MANUAL_OVERVIEW_PASS`、`ARCH76_OPEN_RETURN_PASS`、`ARCH76_OPEN_OTHER_TAB_PASS`、`ARCH76_THEME_ROUTE_PASS`
  （3 themes × 980/720、1240/820 × 4 tabs）、`ARCH76_LIFECYCLE_PASS`、`ARCH76_SCREENSHOT_PASS`、scripts/check、
  compileall、Ruff、source-limit、theme-audit 均通过。`local-arch-76` onefile 已覆盖 canonical/root/root-latest，
  三者均为 `47,982,523` bytes，SHA-256 `6607C1BFD68E8B9F5148B19D827D1B000C3C9528DE616D63A7ED5F6E9E3A9AC9`，
  archive listing SHA-256 `C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732`，provenance pass，
  签名 `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- ARCH-77 / UI-1.150（源码、运行时验证与包交付完成，2026-08-12）：动效保持单一
  `MotionController`，使用一个 `PreciseTimer` 的 8ms 周期调度，phase 继续按 monotonic elapsed
  推进；离屏 1.25s 采到 156 帧、平均 `7.996ms`，记录为约 120Hz scheduler target，不宣称显示器锁定
  120fps。协议、组件、Dataset、曲线和回放工具条改为 presentation-only 语义 field/action rows，新增
  无状态 `presentation/form_fields.py` helper；保留 `ProtocolPanelWidgets`、callbacks、typed
  `itemData`、keyboard/accessibility 与外层滚动容器。`ARCH77_LAYOUT_PASS` 覆盖 980/1240 × 三主题、
  horizontal maximum=0、9 个关键控件可见；`ARCH77_LIFECYCLE_PASS` 覆盖 hide/show 与 focus 恢复；
  scripts/check、compileall、Ruff、source-limit、theme-audit 通过。架构师 A 有条件批准并要求诚实记录
  8ms/约 120Hz；独立审查 Required=1（文案已修正）、Optional=2、FYI=3，最终无阻断。`local-arch-77`
  onefile 已覆盖 canonical/root/root-latest，三者均为 `47,984,250` bytes，SHA-256
  `11E505D955E750AD5062D5A1B552E9D92AE2DF9A8D3751DEAE0612040E9140D9`，archive listing SHA-256
  `A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073`，provenance pass，签名
  `NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

- UI-1.16：顶栏状态胶囊与低动效控制胶囊；只调整 presentation 布局和 QSS 作用域。
- UI-1.17：实时观测带状态表面；复用历史、连接流程、暂停、记录和接收活动事实。
- UI-1.18：发送状态胶囊；复用 `send_band_state` 与 `send_hint`，不复制发送业务判断。
- UI-1.19：协议流水线摘要来源/状态表面；复用历史来源、SessionState、派生来源支持和协议编辑器 dirty 状态，不复制业务判断。
- UI-1.20：主题语义 token 完整化；新增纯 QSS runtime composer，拆分 shell/native-control overrides，覆盖三套主题的 popup、viewport、Tab overflow、禁用态、状态胶囊与滚动条，避免低层控件沿用默认星轨色值。
- UI-1.21：关闭工作区 `QTabBar` 原生 base 绘制，消除 Qt style 在“链路 / 连接”Tab 上方注入的白色横线；作用域仅限当前工作区 TabBar。
- UI-1.22：协议/遥测页改为可收缩网格，限制最长 preset/framing/checksum 选项和动态 Profile/Dataset 文本，980px 下不再横向滚动。
- UI-1.23：批量命令增加 empty/ready/running/completed/stopped/failed 状态表面；隐藏只读结果表无意义的原生纵向行号栏，消除白色 palette 回退。
- UI-1.24：UART/TCP/BLE/RTT 连接控件增加 bounded combo 和可换行提示策略，980px 下六类传输页均无横向滚动。
- UI-1.25：连接带增加版本化内置快速配置 DTO 与 bounded 选择器，覆盖 UART/TCP/UDP/RTT/BLE；只填入表单，
  不自动连接、不保存密钥，并保持 980px 响应式布局；catalog 为 schema v1 immutable tuple（7 项、唯一 key、
  DTO 范围校验），preset action 使用 typed callback，TCP Server 授权在传输/监听 host/port 变化时失效。
- UI-1.26（已完成，2026-08-10）：把终端 placeholder 空白升级为 presentation-only 的上下文空态；将实时观测与发送栏由单行
  `QHBoxLayout` 改为两行可收缩网格，保留现有 widget 名称、信号、发送 gate、键盘顺序和共享 MotionController。
  验收覆盖 980×680/1180×780、三主题、未连接/连接中/已连接未收数/暂停/历史/有数据/发送 blocked-ready-busy-history
  状态，禁止新增业务字段、常驻 timer、外部资源和传输耦合。
- UI-1.27（已完成，2026-08-10）：修复协议/遥测页组件 Profile 与 Dataset 工具条在 980px 下的控件重叠；新增
  协议配置、组件遥测、Dataset/曲线、历史回放四个静态 `role="surface"` 分区，并将派生工具条改为明确的
  响应式 `QGridLayout`。保留 `ProtocolPanelWidgets`、callbacks、widget refs、Tab 顺序和所有数据语义，不新增
  业务字段、timer、依赖或传输耦合；通过 980×680/1180×780、长标签、三主题和无横向滚动离屏验证。
- UI-1.28（已完成，2026-08-10）：将现有 `pipelineSummary` QLabel 升级为 presentation-only
  `PipelineSurfaceLabel`，在安全底部 padding 绘制输入→Protocol→Component→Dataset/Curve 的主题化轨道；复用
  唯一 `MotionController` 帧，不读取/复制 `state/source`，不新增 timer/业务字段/依赖，并覆盖 reduced-motion、暂停、
  隐藏、最小化和关闭静态回退。保留文字、AccessibleDescription、objectName、QSS 属性和 `ProtocolPanelWidgets` 引用。
- UI-1.29（已完成，2026-08-10）：增强 `StatusIndicator` 的六状态几何：连接中旋转弧、已连接呼吸环、发现态定位角标、
  错误叉标、关闭态静态环；保留 22×22、`set_state/set_frame/stop`、主题继承和共享 MotionController，未新增 timer、
  业务字段、生命周期判断或外部资源；通过三主题、六状态、低动效/暂停/隐藏离屏验证。
- UI-1.30（已完成，2026-08-10）：新增 `CommandBatchSurfaceLabel` 与 `ReplayActivityLabel` 两个独立 presentation
  renderer。批量轨道只消费既有 `CommandBatchSnapshot` 投影并表达空态/就绪/执行中/完成/停止/失败步骤；回放轨道只消费
  既有状态和 emitted 计数，表达活动轨迹而不伪造总进度。两者复用唯一 `MotionController`，不新增 timer、业务字段、
  ViewModel/domain/transport 依赖，保留状态文本、QSS objectName 与 accessibility 契约，并覆盖统一生命周期停止。

- UI-1.31（已完成，2026-08-10）：新增 `AnalysisStatusLabel`，为 Protocol、Component、Dataset、Curve 四个既有状态条
  增加主题化底部信号轨道；只读取 `source/state` 动态属性，复用唯一 `MotionController`，不新增业务状态、projection DTO、
  QTimer 或资源。保留文字、tooltip、AccessibleDescription、objectName/QSS 命中和 replay 专用轨迹独立性，覆盖未知状态、
  三主题、低动效、隐藏/暂停/关闭生命周期与 980/1180 响应式门槛。

### 主窗口企业化拆分（第一阶段完成）

- ARCH-1：建立 controllers/contracts 目录和最小类型化边界，保留行为不变的显式委托接线。
- ARCH-2：迁移 Protocol/Component/Dataset/Curve 面板构建与派生状态投影。
- ARCH-3：迁移 Terminal/Recording/Send 与 Command Batch。
- ARCH-4：迁移 Connection/BLE/TCP Server 与异步回调。
- ARCH-5：迁移 Workspace/生命周期和 bootstrap，`main_window.py` 收敛至 1000 行以内。

### 企业化第二阶段（持续门禁）

- ARCH-6：按 feature state/action DTO 继续收窄 controller 对完整 MainWindow facade 的依赖。
- ARCH-6a（已完成，2026-08-10）：按 owner-module contract 移除 UART/TCP/UDP/RTT/BLE 纯配置 builder 与 BLE UUID
  解析的 8 个 MainWindow 转发；`connection_runtime.py` 直接自调用，`ble.py` 显式调用 discovery builder，未引入
  `__getattr__`、动态注册、monkey-patch 或 mixin；MainWindow 从 1000 行降至 951 行。
- ARCH-6b（已完成，2026-08-10）：删除仅由 terminal runtime 自身调用的 `MainWindow._current_entry` 与 `_render`；
  `terminal_runtime.py` 直接调用同模块 `current_entry()`/`render()`，保留 preview signal、Qt parent、Tab 和生命周期
  facade。MainWindow 从 951 行降至 939 行；独立复核 Critical=0、Required=0，剩余 runtime `window._*` 访问列为后续 DTO
  收窄范围，不在本切片扩大迁移面。
- ARCH-6c（已完成，2026-08-10）：bootstrap 直接调用页面/快捷键/焦点 owner，workspace 直接组合既有 page builder 并连接自身
  Tab/transition 函数，lifecycle 直接调用 workspace 生命周期 helper，connection builder 直接使用 bounded composition helper；
  删除 MainWindow 的 20 个纯 composition/workspace facade，保留业务 signal/callback wrapper 不变。
- ARCH-6d（已完成，2026-08-10）：连接面板通过 `ConnectionActionCallback` 注入 `connection_runtime.toggle_connection`，Qt
  checked 参数在 owner action 边界吸收；删除 `MainWindow._toggle_connection`，MainWindow 从 811 行降至 803 行，未改变
  连接按钮、session gate、错误入口或生命周期。
- ARCH-6e（已完成，2026-08-10）：将纯 `endpoint_label`/`history_label` formatter 从 lifecycle controller
  收敛到 `presentation/formatters.py`；connection/terminal controller 显式依赖 formatter，删除两个 MainWindow
  静态 facade，MainWindow 从 803 行降至 789 行，避免 lifecycle↔connection runtime 循环依赖。
- ARCH-6f（已完成，2026-08-10）：将有界 `refresh_dynamic_property()` 收敛到
  `presentation/property_refresh.py`；bootstrap、lifecycle、connection、commands、terminal 显式依赖，删除
  `MainWindow._set_dynamic_property`，MainWindow 从 789 行降至 782 行，保持 QSS re-polish/update 顺序。
- ARCH-6g（已完成，2026-08-10）：将 `status_surface_source()` 归还 `status_surfaces.py`，将
  `tcp_server_readiness()` 归还 `connection.py`；bootstrap/connection 直接调用并删除两个 MainWindow 单调用点 facade，
  MainWindow 从 782 行降至 770 行，保持 IPv4、回环/LAN 确认、allowlist gate 和状态 source 语义。
- ARCH-6h（已完成，2026-08-10）：将 `current_send_mode()` / `selected_command_batch()` 收敛到
  `presentation/command_selection.py`；commands、connection、terminal runtime 显式依赖，删除两个 MainWindow facade，
  MainWindow 从 770 行降至 758 行，保留 HEX fallback 与 batch `UserRole` 选择语义。
- ARCH-6i（已完成，2026-08-10）：在 commands owner 增加发送/批量 `*_action()` payload adapter，terminal/composition
  通过 `partial` 直接接线；删除 `MainWindow._send_current` 与四个批量 action facade，MainWindow 从 758 行降至 728 行，
  保留 Qt signal、dialog parent、ViewModel batch semantics 和 Ctrl+Enter 行为。
- ARCH-6j（已完成，2026-08-10）：commands 内部直接调用 batch selection/render canonical function，bootstrap/terminal
  通过 `partial` 直连 ViewModel/combo signal；删除四个 MainWindow command signal/结果 facade，MainWindow 从 728 行降至 702 行，
  保留首屏 projection、状态轨道和 connection controls 更新顺序。
- ARCH-6k（已完成，2026-08-10）：主题/动效 callback 通过 `partial` 直接接入 workspace/lifecycle owner，删除 5 个 MainWindow
  motion facade；将 Tab 可见性、派生 surface 挂起和一次性过渡迁移至 `workspace_runtime.py`，消除首次离屏组装发现的
  workspace↔lifecycle 循环依赖，MainWindow 从 702 行降至 672 行。
- ARCH-6l（已完成，2026-08-10）：将 BLE bounded write-mode selector/capability helper 收敛到
  `presentation/ble_selection.py`，`ble.py`/`connection.py`/`commands.py` 直接消费，删除 3 个 MainWindow BLE helper facade，
  MainWindow 从 672 行降至 652 行，保留 normalize action、BLE gate 与 Qt signal 语义。
- ARCH-6m（已完成，2026-08-10）：将 parser/derived-source applicability query 收敛到
  `presentation/protocol_scope.py`，connection/lifecycle/derived/workspace/terminal 直接消费，删除 3 个 MainWindow protocol
  scope facade；MainWindow 从 652 行降至 634 行，保留 protocol editor side effects 与 UART/TCP/UDP gate 语义。
- ARCH-6n（已完成源码与真实组合根 offscreen vector 验证，2026-08-10）：连接控件 enable/hint/busy projection 仍由
  `controllers/connection.py` 独立拥有，`controllers/bootstrap.py` 通过 `partial(update_connection_controls, window)` 绑定
  `window._refresh_connection_controls`；各 owner controller 完成调用迁移，删除 `MainWindow._update_connection_controls`、对应 import 和 domain facade 依赖，
  保留首屏 hydration、session/replay gate、BLE/TCP Server readiness、Qt lifecycle 与 accessibility；静态门、compileall、ruff、真实组合根 callback/facade removal vector
  通过，独立复核代理超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,894,630` bytes，SHA-256
  `A56986CF93FB1B3B100C78B3AA172ECF5542BB14BF4BABB8D07537DB32925DB7`，签名 `NotSigned`，正式发行资格 `false`。
- ARCH-6o（已完成源码与真实组合根 offscreen vector 验证，2026-08-10）：`controllers/bootstrap.py` 通过 `partial` 绑定
  `window._on_transport_changed` 与 `window._on_protocol_framing_changed`，分别指向 transport runtime 与 protocol framing owner；删除 MainWindow 两个纯转发方法/import，
  保留 builder/preset/replay/composition/protocol 的现有 callback 名称、Qt payload、首屏 hydration、panel transition、协议草稿副作用和 lifecycle。静态门、compileall、ruff、
  `ARCH6O_COMPOSITION_PASS` 通过；独立复核代理超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,893,668` bytes，SHA-256
  `4DC716FA8B652158058F777284968B8D5E8CC6C3C12DDEF52CBE0B3A919891F8`，签名 `NotSigned`，正式发行资格 `false`。
- ARCH-6p（已完成源码与真实组合根 offscreen vector 验证，2026-08-10）：将 protocol/derived/replay/BLE/terminal/lifecycle
  的剩余 callback 通过 bootstrap `_bind_*_callbacks()` 按 owner 分域用 `partial` 显式接线，`MainWindow` 删除全部业务转发，仅保留构造与四个 Qt lifecycle override；保留既有 callback 名称、
  Qt payload、首屏 hydration、焦点/Tab/accessibility、关闭 fence 和 owner policy。static/compileall/ruff 与 14 callback composition vector 通过；独立复核代理超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,891,078` bytes，SHA-256
  `70604D4065895FAF07EE86D675FFF8845E7020C82556D0D18ED41188FCA6CA86`，签名 `NotSigned`，正式发行资格 `false`。
- ARCH-7a：主题与动效偏好已收敛到版本化 `PresentationPreferences` / `PreferenceStore` port，QSettings 仅存在于唯一 adapter。
- ARCH-7b（已完成，2026-08-10，最小边界切片）：新增 `ConnectionPresetCatalogStore` port、schema v1 显式白名单
  codec 和唯一 `QSettings` adapter；composition root 读取后注入 immutable catalog，坏 payload/未知字段/enum/超长内容/
  非严格整数回退七项 builtin，begin/set/end 任一持久化阶段失败则 fail-open 且跳过 sync。未包含自定义 preset editor、
  builtin/custom merge 或 save action，后续单列 ARCH-7c。

UI-1.25 先交付 presentation 内置 catalog 与一次性填充动作；runtime 继续消费规范化控件值而不导入
presentation DTO，这是依赖方向的显式边界。ARCH-7b 只建立安全的读取/保存 port，不把存储策略和用户编辑器耦合进本轮
UI 组合；用户自定义 catalog 的编辑、合并和显式保存动作仍单独作为 ARCH-7c。
- ARCH-7c（已完成，2026-08-10）：新增 presentation-only 自定义 preset editor，控制器从当前表单生成 typed DTO，
  builtin/custom merge 保持 builtin key 不可覆盖，store 只持久化 custom 项并返回保存结果；保存/删除显式触发，活动会话
  与历史回放锁定，失败状态不伪装成功。通过三主题、980/1180、源码门禁、坏持久化回退和根目录 onefile 重建门后交付。
- ARCH-8：OTA/debug 只有在目标 bootloader、vendor 一手资料、安全审查和授权硬件证据齐备后，才进入 application orchestration。

- UI-1.32（已完成，2026-08-10）：主题选择器旁新增 `ThemePaletteSwatch` 语义色点阵/星轨装饰；固定 62×22、只读
  `ThemeSpec`、不创建 timer/主题信号/业务状态，不进入 Tab/无障碍树，复用共享 MotionController 并覆盖三主题、980/1180、
  低动效/暂停/隐藏/关闭生命周期；独立复核 Critical=0、Required=0。
- UI-1.33（已完成，2026-08-10）：连接控制带新增 presentation-only `ConnectionStatusRail`，在独立底行绘制端点→传输→
  会话→数据四节点路径；由既有 `SessionState` 显式映射，复用唯一 MotionController，不创建 timer、不读取 ViewModel/domain、
  不改变连接按钮/信号/无障碍文案；通过六状态、三主题、980/1180、1.5x HIDPI、隐藏/恢复/静态停止和无横向滚动验证。
- UI-1.34（已完成，2026-08-10）：在现有 `TerminalEmptyState` 父层绘制低对比度观测网格、四角 bracket、状态色扫描线和五节点
  pulse；只消费已有空态 state 与共享 frame，不新增控件、业务字段、timer 或 lifecycle 接线，保留卡片文字/glyph/accessibility；
  通过五种空态、三主题、980/1180、1.5x HIDPI、无横向滚动和隐藏/停止静态回退验证。
- UI-1.35（已完成，2026-08-10）：新增独立 `presentation/theme_transition.py`，主题切换在 `apply_theme()` 后对 bootstrap
  持有的 `appRoot` 做一次性 180ms 淡入；不新增常驻 timer、业务状态或主题 token，lifecycle 在低动效/暂停/隐藏/最小化/关闭
  时停止并清理 Qt effect，快速切换和自然完成均回到静态主题。
- UI-1.36（已完成，2026-08-10）：将真实数据终端替换为保持原生契约的 `TerminalViewport` 子类；原生文本先绘制，装饰只在
  viewport 上下 padding 内绘制主题化观测标尺、四角 bracket 和共享 frame 信号。未复制 RX 数据、未新增 timer/业务状态，
  并通过空态/数据态、三主题、980/1180、1.5x HIDPI、无横向滚动和生命周期停止验证。
- UI-1.37（已完成，2026-08-10）：为既有共享 `MotionController` 增加可暂停的 `ambient_active` 环境动效模式；可见空闲窗口
  维持低频 shell 信号场，隐藏/最小化/低动效/暂停/关闭仍停表，未新增控件级 timer 或业务状态。空闲基线为 0 帧，开启后同条件
  离屏窗口约 9 帧/秒；完整窗口截图 `build/ui_review_ui137_ambient_motion.png` 无近白像素。
- UI-1.38（已完成，2026-08-10）：新增 `WorkspaceRouteSurface` 三节点工作区路线 beacon；真实 `QTabWidget` 与 route strip
  由 workspace shell 共同布局，修复 Qt corner widget 在 980px 下越界裁切；route 只消费 Tab index/frame，不进入焦点/无障碍顺序，
  通过 980/1180、三主题、无横向滚动、生命周期和像素审计。
- UI-1.39（已完成，2026-08-10）：终端无 RX 空态增加 `primaryButton` 连接导航 CTA；只发出无参数
  `connection_requested` presentation intent，由 `workspace_runtime.select_workspace_tab()` 负责关闭门、Tab 索引边界、
  首 Tab 切换和焦点恢复。paused/history 隐藏 CTA；不自动连接、不改变 session/transport，不新增 token/timer；通过
  980/1180、实际 click、无障碍元数据、三主题近白像素和无横向滚动验证，详见 ADR 0026。
- UI-1.40（已完成，2026-08-10）：新增 `TerminalActionButton` 底部 signal rail；只消费共享
  `MotionController` frame 与 `ThemeSpec.accent`，由 `TerminalEmptyState.set_frame/stop` 转发，不新增控件级 timer、
  业务状态或导航副作用。通过 phase 两端像素差、共享 frame、暂停/隐藏/关闭静态回退和三主题像素审计，详见 ADR 0027。
- UI-1.41（已完成，2026-08-10）：将通用 `ActionRailButton` 抽到独立 presentation surface，并新增
  `CommandBatchEmptyState` 首屏空态卡片与 `new_requested` intent；无 batch 时由 `commands.py` 互斥隐藏 status surface/空结果表，
  完整显示“新建批量命令” CTA，选中 batch 后恢复状态带和只读结果表。CTA 仍服从 connection batch-edit lock；所有装饰复用
  唯一 `MotionController`，无新 timer/业务状态/transport 依赖。通过实际 signal、空态↔batch 可见性、980/1180、三主题近白像素和
  pause/hide/close 静态回退验证，详见 ADR 0028。
- UI-1.42（已完成，2026-08-10）：修复批量编辑器真实主题下的白色系统 palette 回退：隐藏与横向“步骤”列重复的
  `QTableWidget` vertical header，并为 `QSpinBox/QDoubleSpinBox` 的 up/down button/arrow 补齐默认与 tokenized 三主题 QSS；
  保留原生键盘/stepper/accessibility/延时值语义。通过真实 MainWindow parent、三主题 near-white=0、表头几何和源码门禁验证，详见 ADR 0029。
- UI-1.43（已完成，2026-08-10）：让连接快速配置的 `customSelected` 状态真正消费于三主题 QSS，并由
  `connection_preset_surface.py` 统一同步空态/内置/自定义 tooltip 与 `AccessibleDescription`；controller 不再覆盖
  选择器上下文。主题静态审计新增 custom selector，三主题 offscreen 状态/近白像素、controller selection、导入编译通过，详见 ADR 0030。
- UI-1.44（已完成，2026-08-10）：新增 `presentation/dialog_surface.py`，统一协议确认 QMessageBox 的
  `surfaceRole="confirmation"`、标准 Ok/Cancel 按钮文案、危险操作视觉语义和 AccessibleName/AccessibleDescription；
  `protocol_config.py` 仍保留确认范围与 `exec()` 返回语义。默认 controls stylesheet 与主题 override 补齐 warning surface、
  informative label、按钮宽度和 13 项 selector 门禁；通过静态门、126 模块导入/compileall、三主题离屏按钮与近白像素验证，详见 ADR 0031。
- UI-1.45（已完成，2026-08-10）：定位并修复 Qt ComboBox popup 顶层 QFrame 不继承父窗口 QSS 导致的上下白带；新增
  `presentation/popup_surface.py` 和纯 runtime popup stylesheet，主题入口、主窗口组合完成、CommandBatchEditor 构造完成时刷新，
  保留原生 QListView/popup/键盘/焦点。已通过三主题 popup near_white=0、theme switch、编辑器 popup、980/1180 响应式验证、文档归档和最终打包。
- UI-1.46（已完成，2026-08-10）：定位并修复 Qt QFileDialog 目录树/文件列表的系统白色 palette 回退；新增
  `presentation/file_dialog_surface.py` 统一 open/save wrapper，保留 Qt 文件系统模型、路径、筛选、键盘和确认语义，
  通过 bounded ThemeSpec QSS 与 `DontUseNativeDialog` 保证三主题一致，并复用 ComboBox popup frame bridge；已通过静态门、
  129 模块导入/compileall、三主题 QFileDialog/内部 popup 离屏截图、open/save 取消语义和最终打包。
- UI-1.47（已完成，2026-08-10）：将 UART 端口刷新与 BLE 扫描的现有 busy projection 接入
  `presentation/action_surface.py:BusyActionButton`；两个按钮仍是原生 QPushButton，只用共享 MotionController 绘制忙碌信号轨，
  低动效/暂停/隐藏/关闭时静态回退，不新增 timer、业务状态或 transport 逻辑；已通过三主题、busy/disabled、reduced-motion、
  980px 离屏验证和最终打包。
- UI-1.48（已完成源码与离屏验证，2026-08-10）：为 native QStatusBar 增加
  `presentation/status_footer_surface.py:StatusFooterSurface`，以 116×18 的 session/RX signal rail 补足底部状态反馈；
  原生 status message 仍是唯一可访问权威文案，surface 只消费 lifecycle 的 state/fault/activity projection 与共享
  MotionController，不新增 timer、业务状态或传输逻辑；三主题状态栏 exact_white=0、静态 fault/closed、导入编译通过。
- UI-1.49（已完成源码与离屏验证，2026-08-10）：将发送区输入框替换为原生兼容
  `presentation/send_input_surface.py:SendInputSurface`，把既有 `send_band_state` 映射为 blocked/waiting/ready/busy/history
  状态轨与焦点反馈；原生文本编辑、选择、placeholder、回车、无障碍和发送 gate 不变，动效只复用 MotionController，
  低动效/暂停/隐藏/关闭静态回退；已通过三主题五状态、原生 QLineEdit 类型/文本保持、静态门和 compileall。
- UI-1.50（已完成源码与离屏验证，2026-08-10）：为 errorBar 增加
  `presentation/error_surface.py:ErrorSignalSurface` 故障 beacon；错误文字/清除按钮/ErrorInfo 可见性保持原 owner，
  beacon 只消费 active 与共享 MotionController frame，active/clear、stop、三主题 error bar exact_white=0 验证通过，不新增 timer、
  错误状态源或业务语义。
- UI-1.51（已完成源码与离屏验证，2026-08-10）：复用
  `presentation/action_surface.py:BusyActionButton` 装饰原始记录按钮，将既有 RecordingState STARTING/ACTIVE/STOPPING 投影为
  activity rail；保留按钮文字、点击、禁用和无障碍语义，不新增 recording 状态/timer/renderer，复用共享 MotionController。
- UI-1.52（已完成，2026-08-10）：复用 `BusyActionButton` 装饰连接主按钮，将既有 SessionState OPENING/CLOSING 投影为
  connection activity rail；只修改 presentation builder/controller/lifecycle 接线，不新增连接状态源或连接业务逻辑；通过真实
  update_connection_controls 状态向量、三主题 980×680 截图、near-white 像素门和 onefile/root EXE provenance 校验。
- UI-1.53（已完成，2026-08-10）：将既有 `batch_active` 与 `ReplayState.PLAYING` 投影到批量停止/历史回放暂停按钮的共享
  `BusyActionButton`，让操作区与状态轨道保持一致，不新增执行状态或业务时钟；通过真实状态向量、三主题操作区截图、near-white
  像素门和 onefile/root EXE provenance 校验。
- UI-1.54（已完成源码与离屏验证，2026-08-10）：新增 `DataActivitySurface` 原生 QLabel 后置 activity rail；由既有接收文案 owner
  投影 source/最近字节/窗口字节，lifecycle 投影 active/frame/stop，不复制 RX buffer、不表达吞吐或进度。
- UI-1.55（已完成，2026-08-10）：新增 `SendStateSurface` 原生 QLabel 后置状态 rail，读取既有 blocked/waiting/ready/busy/history
  property，补齐发送控制带状态反馈，不改变发送 gate 或业务语义；已完成多状态离屏、共享 frame/stop、三主题像素门、onefile
  provenance 与根目录 EXE 覆盖校验。
- UI-1.56（已完成，2026-08-10）：新增 `ConnectionPresetContextSurface`，将既有快速配置 DTO 的内置/自定义来源、传输类型与描述
  变成连接带内可见摘要，同时保留原 `connection_hint` 下一步/无障碍契约；只接入共享 frame/stop，不自动连接、不新增业务状态。
- UI-1.57（已完成，2026-08-10）：扩展既有 `theme_transition.py` 的一次性主题切换为 root fade + `ThemeTransitionSurface` 几何
  sweep；同一 helper 负责 effect/overlay/animation 的 stop/finish 清理，不改变 `apply_theme`、业务状态、焦点或无障碍树。
- UI-1.58（已完成，2026-08-10）：新增 `SendContextSurface` 与 `controllers/send_context.py`，把现有发送 mode、文本/Hex
  payload 字节数、wire payload、CRLF 和格式/容量错误投影为可读上下文摘要与共享 byte rail。复用 domain
  `MAX_COMMAND_PAYLOAD_BYTES`，不改变原生编辑、发送 gate、快捷/历史动作或 ViewModel；mode/CRLF 复用既有输入刷新回调，
  reduced-motion/暂停/隐藏/最小化/关闭统一走 MotionController 静态回退。
- UI-1.59（已完成源码与离屏验证，2026-08-10）：新增 `ProtocolConfigContextSurface` 与
  `controllers/protocol_context.py`，把既有协议 framing/checksum/max-frame 控件和 `protocolStatus` 的已投影 state
  变成“草稿/已应用”配置摘要；不复制 parser gate、不改变 apply/reset 语义，宽度限制为 190–520 px，三主题与共享
  MotionController/reduced-motion 静态回退已验证。
- UI-1.60（已完成源码与离屏验证，2026-08-10）：将 Component 无数据普通 QLabel 升级为
  `ComponentEmptyStateSurface` 空态卡片；复用既有 `on_load_component_codec` 提供近场 CTA，空态时隐藏空表头，
  有可见 rows 时恢复原生表格；文案、过滤、选择、导出和 codec 语义不变，三主题、共享 frame/stop 与 root onefile
  交付待本轮最终打包后归档。
- UI-1.61（已完成源码与离屏验证，2026-08-10）：增强 `DatasetCurveWidget` 的无 series、等待 sample、无可绘制点
  空态画布，加入 ThemeSpec glyph/节点/等待说明与共享 signal rail；真实 `CurveSnapshot`、QTimer debounce、
  flush/suspend/shutdown、焦点环和数据曲线绘制语义保持不变，root onefile 交付待本轮最终打包后归档。
- UI-1.62（已完成源码与离屏验证，2026-08-10）：在链路传输选择旁增加 `TransportModeSurface` 六模式几何 glyph，
  只镜像已有 combo mode 并复用共享 MotionController；保留原 combo、header context、connection hint、面板显隐、连接动作、
  无障碍与 980/1180 响应式边界，root onefile 交付待本轮最终打包后归档。
- UI-1.63（已完成源码与离屏验证，2026-08-10）：将 Component/Dataset 原生预览接入共享
  `ObservationViewport`；TerminalViewport 保留兼容类型，三个 scope 共享上下标尺、信号轨和 MotionController 生命周期，
  保留预览文本、滚动、placeholder、无障碍和 controller 数据责任，三主题、980/1180、动画差分与 root onefile 交付待本轮归档。
- UI-1.64（已完成源码与静态验证，2026-08-10）：连接方式切换时为 UART/TCP/UDP/BLE/RTT 配置 panel 增加共享
  一次性 fade；业务显隐与连接状态仍由 `connection_runtime.py` 负责，transition owner 复用统一 motion policy，
  在低动效、暂停、主题切换、隐藏、最小化、关闭和快速切换时清理 effect；GUI/offscreen、root onefile 和真实设备验收待本轮授权/归档。
- UI-1.65（已完成源码与短时 renderer vector 验证，2026-08-10）：为原生 workspace `QTabWidget` 增加链路/协议/命令三枚
  主题化矢量 glyph，配置 Normal/Selected/Disabled 三态并在组装与主题切换时刷新；Tab 文案、index、signal、焦点、键盘和
  accessibility 语义保持不变。`workspace_tab_icons.py` 只承担 bounded presentation rendering，不读取业务状态、不创建 timer
  或外部资源；完整 GUI/offscreen/HIDPI/读屏/EXE 启动和真实设备验收待授权/归档。
- UI-1.66（已完成源码与短时 renderer vector 验证，2026-08-10）：在 Header 的 `SERIALFORGE` wordmark 前增加固定
  38×38 的无外部资源星轨/S 几何品牌徽记；组件只消费 `ThemeSpec` 和既有共享 `MotionController` frame/stop，低动效/暂停/
  隐藏/最小化/关闭保留静态回退，不读取连接/协议状态、不创建独立 timer、不改变标题、状态胶囊、主题选择器或无障碍路径。
  完整 GUI/offscreen/HIDPI/读屏/EXE 启动和真实设备验收待授权/归档。
- UI-1.67（已完成源码与短时布局 vector 验证，2026-08-10）：将 Header 重组为品牌层 + 控制层两行布局，品牌层承载
  徽记/wordmark/副标题/信号场，控制层承载连接状态/动效偏好/主题选择器；在 952px 内容宽度（对应 980px 最小窗口）和
  1152px 内容宽度（对应 1180px 窗口）下行几何均保持在边界内。未改变现有 widget facade、signal、tab order、业务状态或
  无障碍语义；完整 GUI/HIDPI/读屏/EXE 启动和真实设备验收待授权/归档。
- UI-1.68（已完成源码与短时布局 vector 验证，2026-08-10）：在发送控制带底部增加主题化的 `Ctrl+Enter 发送` 静态提示，
  只提升既有快捷键的可发现性；`NoFocus`、AccessibleName/Description、tooltip、Tab 顺序、快捷键注册、send action、
  send gate 和 ViewModel 语义保持不变。默认 stylesheet 与三套 theme override 使用非白色 keycap surface；176–184px 尺寸
  在 760/952/1152 内容宽度下文字区不裁切。完整 GUI/HIDPI/读屏/EXE 启动和真实设备验收待授权/归档。
- UI-1.69（已完成源码与短时连接页布局 vector 验证，2026-08-10）：连接配置页 UART/网络/BLE 的普通参数标签统一通过
  `connection_builder._field_label()` 使用 `role="muted"`，建立清晰的字段/section/hint 层级；不改变 preset、transport、BLE、
  network signal wiring、Tab order、配置值或业务状态。3 个 section 标题保持 `role="section"`，37 个字段/辅助标签与三主题
  均通过验证；完整 GUI/HIDPI/读屏/EXE 启动和真实设备验收待授权/归档。
- UI-1.70（已完成源码与短时 icon vector 验证，2026-08-10）：为原生主题选择器的三个 item 增加无外部资源的 ThemeSpec 色盘
  icon，保留原生主题文案、UserRole、tooltip、currentIndexChanged、焦点和 Tab 顺序；icon renderer 不进入 lifecycle、不创建
  timer、不读取业务状态。三主题 icon actualSize、静态检查、compileall、provenance 与根目录 onefile 均通过；完整 GUI/HIDPI/
  读屏/EXE 启动和真实设备验收待授权/归档。
- UI-1.71（已完成源码与短时协议页 vector 验证，2026-08-10）：协议/遥测页 8 个普通字段标签统一通过
  `controllers/protocol.py:_field_label()` 使用 `role="muted"`，5 个 section 标题与 5 个状态 owner 保持原有语义；不改变
  callback、DTO、协议状态、焦点、Tab 顺序或布局。静态检查、compileall 与三主题协议页 vector 通过；独立复核代理在窗口内超时，
  由父代理完成五轴审查；根目录 onefile 交付待本轮打包归档。
- UI-1.72（已完成源码与短时对话框 vector 验证，2026-08-10）：批量命令编辑器的 5 个静态字段/辅助标签统一通过
  `command_batch_editor.py:_field_label()` 使用 `role="muted"`；空态 `subtle`、错误 `error`、draft/校验、popup、焦点和 Tab 顺序
  保持原有契约。三主题 dialog vector 与 compileall 通过；独立复核代理在窗口内超时，父代理完成五轴审查；根目录 onefile 交付待
  本轮已打包归档，canonical 与根目录 EXE 字节一致，provenance verify 通过；完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.73（已完成源码与真实组合根三主题 vector 验证，2026-08-10）：实时观测工具栏“显示”字段统一通过
  `controllers/terminal.py:_field_label()` 使用 `role="muted"`；`实时观测` section、display mode combo、暂停/记录/接收状态、
  signal、焦点和 Tab 顺序保持原有契约。compileall 与三主题组合根 vector 通过；独立复核代理在窗口内超时，父代理完成五轴审查；
  根目录 onefile 已打包归档，canonical 与根目录 EXE 字节一致，provenance verify 通过；完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.74（已完成源码与真实组合根 dialog transition vector 验证，2026-08-10）：新增
  `presentation/dialog_transition.py`，为批量命令编辑和自定义连接 preset 两类 presentation-owned dialog 提供 150ms
  一次性淡入；show/hide 清理、已有 effect 不覆盖、正常动效、reduced-motion、显式暂停和三套主题均已验证。dialog 行为、
  draft/validation、按钮 action、焦点、Tab/accessibility 与 native dialog 保持不变；compileall 与静态边界审计通过，独立复核
  代理在窗口内超时，父代理完成五轴审查；本轮 onefile 已重新打包并覆盖根目录，canonical 与根目录 EXE 字节一致，provenance
  verify 通过；完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.75（已完成源码与真实组合根三主题 dialog vector 验证，2026-08-10）：为
  `ConnectionPresetEditorDialog` 增加复用既有 `role="surface"` 的 bounded metadata surface，名称/备注字段使用
  `role="muted"`，保持 `ConnectionPresetMetadata`、空名称/长度校验、保存/取消、初始焦点、Tab/accessibility 与 dialog transition
  契约不变。静态检查、compileall 与三主题 vector 通过；独立复核代理在窗口内超时，父代理完成五轴审查；本轮 onefile 打包与根目录
  覆盖已完成，canonical 与根目录 EXE 字节一致，provenance verify 通过；完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.76（已完成源码与真实组合根三主题 activity vector 验证，2026-08-10）：快速配置下拉的用户选择与清空选择在既有
  `ConnectionPresetContextSurface` rail 上请求一次 420ms shared `MotionController` activity pulse；不新增 timer、不自动连接、不改变
  combo value、apply callback、tooltip/accessibility 或 preset DTO。三主题选择/清空、reduced-motion、显式暂停均通过；独立复核代理
  在窗口内超时，父代理完成五轴审查；本轮 onefile 已打包并覆盖根目录，canonical 与根目录 EXE 字节一致，provenance verify 通过；
  完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.77（已完成源码与真实组合根三主题六链路 section parity vector 验证，2026-08-10）：修复连接方式切换后隐藏 panel 仍留下
  UART/网络/BLE section 标题的孤立空行；`connection_builder.py` 保存标题引用，`connection_runtime.py` 与既有 panel projection 同步
  可见性，保留 `role="section"`、焦点/Tab/accessibility、既有 panel fade 和高内聚边界。静态检查、compileall 与三主题 × 六链路
  parity vector 通过；独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 字节一致，provenance
  verify 通过；完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.78（已完成源码与真实组合根导航 activity vector 验证，2026-08-10）：工作区三 tab 的可见用户切换现在复用唯一
  `MotionController` 请求 320ms activity pulse，route beacon/共享 signal rail 获得即时反馈；首次 hydration、隐藏、最小化、关闭、暂停和
  reduced-motion 均静态，Tab 语义、derived renderer suspension 和既有 page fade 保持不变。静态检查、compileall、三主题导航与生命周期
  vector 通过；独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 字节一致，provenance verify
  通过；完整 GUI/HIDPI/读屏/硬件验收仍待授权。
- UI-1.79（已完成源码与真实组合根三主题错误 projection vector 验证，2026-08-10）：错误栏在非空 ErrorInfo 投影时对可见窗口请求一次
  520ms shared activity pulse，让既有 fault beacon/status rail 即时确认；错误模型、文案、清除、焦点和 accessibility 不变，隐藏 hydration、
  清除路径、暂停和低动效保持静态。静态检查、compileall、三主题出现/清除 vector、onefile 打包和根目录覆盖通过；独立复核代理在窗口内超时，
  父代理完成五轴审查。canonical/root 大小 `47,892,391` bytes，SHA-256
  `DF374A4497AA1608478CEF24CE89B61038A7BF5AA0445AB6807C83EAADEE07E5`，签名 `NotSigned`，正式发行资格 `false`。
- UI-1.80（已完成源码与真实组合根派生快照 activity vector 验证，2026-08-10）：ComponentFrameRow/DatasetSample 非空快照进入可见 Protocol 页时
  复用唯一 `MotionController` 请求一次 360ms activity pulse；空快照、bootstrap hydration、后台页、隐藏/最小化/关闭、暂停和低动效保持静态，
  不改变 DTO、stats、表格/预览/曲线、renderer throttle 或 accessibility。静态检查、compileall、三主题主题应用、隐藏 guard 和可见 probe 通过；
  独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,890,929` bytes，SHA-256
  `E47A06C61064CC934B0812FEDCAD462AD61E3DFB664A3593C37705CEEE76365E`，签名 `NotSigned`，正式发行资格 `false`。
- UI-1.81（已完成源码与真实组合根回放终态 vector 验证，2026-08-10）：历史回放 PLAYING 继续使用 520ms shared activity；EOF/STOPPED/ERROR 在可见窗口
  追加一次 480ms terminal confirmation，`ReplayActivityLabel` 为 history/error 增加主题化静态终态 marker；PAUSED/EMPTY、隐藏 hydration、最小化、暂停和低动效
  保持静态，不改变 replay DTO、按钮、source badge、记录数量或 accessibility。静态检查、compileall、五状态隐藏 guard、可见 520/480 probe、三主题
  marker render 通过；独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,893,847` bytes，SHA-256
  `DA71E46BE6B626E644F6E907FC75CCE44C8E9C4E174BCA0FC8E6BA9B4B7893D8`，签名 `NotSigned`，正式发行资格 `false`。
- UI-1.82（已完成源码与真实组合根批量命令终态 vector 验证，2026-08-10）：RUNNING 继续使用 520ms shared activity；COMPLETED/STOPPED/FAILED 在可见窗口
  追加一次 480ms terminal confirmation，`CommandBatchSurfaceLabel` 为三个终态增加主题化勾/横线/叉 marker；IDLE、隐藏 hydration、最小化、暂停和低动效保持静态，
  不改变 batch snapshot、结果表、按钮、发送队列或 accessibility。静态检查、compileall、五状态隐藏 guard、可见 520/480 probe、三主题 marker render 通过；
  独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,894,052` bytes，SHA-256
  `BD39D94F914E3AF6DC24E44C04A0B02C0BF8F5553C87911D44CE773CBF6489FB`，签名 `NotSigned`，正式发行资格 `false`。
- UI-1.83（已完成源码与真实组合根分析状态 marker vector 验证，2026-08-10）：Protocol/Component/Dataset/Curve 共用的
  `AnalysisStatusLabel` 为 `error`、`blocked`、`history` 增加主题化静态叉/双横栏/回退箭头 marker；active/waiting/draft 继续使用共享 frame，empty/idle、
  hidden/minimized/closing、暂停和低动效保持静态，不改变文字、AccessibleDescription、objectName、QSS 或 controller projection。静态检查、compileall、
  三主题 × 三状态 marker render 通过；独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已打包并覆盖根目录，canonical/root 大小 `47,891,907` bytes，
  SHA-256 `2DD17ED968B0BC7045C5443B08B161DD6123EEE1CE6CDDA4D064E8CDDACCBE1D`，签名 `NotSigned`，正式发行资格 `false`。
- UI-1.84（已完成源码与真实组合根 scroll-surface vector 验证，2026-08-10）：三个 `settingsScroll` 统一使用 content 横向
  `QSizePolicy.Expanding` 与 `HorizontalScrollBarPolicy=ScrollBarAlwaysOff`，纵向保持 `ScrollBarAsNeeded`，避免横向滚动/亮色横带，不改变页面 objectName、
  widgetResizable、内容、焦点顺序或业务 controller。980/1180/1440 三宽度、三主题和三页策略 vector 通过；独立复核代理在窗口内超时，父代理完成五轴审查；
  onefile 已打包并覆盖根目录，canonical/root 大小 `47,892,763` bytes，SHA-256 `D10B2F4972EF9D4115B45B87DB770DC66D5E9E7AE91EFE40B40DE070BB838E17`，
  签名 `NotSigned`，正式发行资格 `false`。
 - UI-1.85（已完成源码、真实主题 widget vector 与 onefile 交付，2026-08-10）：稳定 QSS 中八处近白文字/选择色全部复用 `TEXT`/`SELECTION_TEXT`，
   保持 selector、selection background、焦点、Tab、表格和 accessibility 不变；三主题 stylesheet 无 white background fallback，checkbox/combo/table/tab 内存 render
   通过，stable near-white literal count=0；独立复核代理在窗口内超时，父代理完成五轴审查；onefile 已重新打包并覆盖根目录，canonical/root 大小
   `47,895,205` bytes，SHA-256 `48A166688A58393D9A060B56DC7C8CCDB3FB9A7F0666EDD6EE3B4C7983AEF6CE`，签名 `NotSigned`，正式发行资格 `false`。
- UI-1.86（已完成源码、三主题 surface vector 与 onefile 交付，2026-08-10）：将 stable controls QSS 的 table/header/statusbar/scrollbar/corner/Tooltip
  颜色统一映射至已有语义 token，保留表头/Tooltip 渐变与 variant override 边界；stable near-white count=0，`scripts/check.ps1`、compileall、ruff
  通过，独立复核代理超时，父代理完成五轴审查；onefile 已重新打包并覆盖根目录，canonical/root 大小 `47,892,639` bytes，SHA-256
  `18C089F9C700E230F676F1AB1EEDF5F777ED3584D0352C789800CC8AC0BA1683`，archive listing SHA-256
   `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.87（已完成源码、三主题真实控件组合 vector 与 onefile 交付，2026-08-10）：将 stable controls 的状态面、输入/禁用态、按钮、checkbox、workspace Tab、terminal
  和终端/批量空态颜色统一映射至已有语义 token，保留 selector、焦点、选择、键盘、accessibility 和 variant owner；static/compileall/ruff 通过，theme token audit
  `legacy_qss_literals=208`，独立复核代理超时，父代理完成五轴审查；onefile 已重新打包并覆盖根目录，canonical/root 大小 `47,894,441` bytes，SHA-256
  `632E63F3D038A15571194AD89567D66B40C4D0C4452C4C54AD5804AEAF2E4638`，archive listing SHA-256
  `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.88（已完成源码、三主题 shell state vector 与 onefile 交付，2026-08-10）：将 stable base shell 的全部 hex literal 映射至已有语义 token，base/controls literal audit=0，
  static/compileall/ruff 与连接/观测/发送/pipeline/status/replay state vector 通过；独立复核代理超时，父代理完成五轴审查；onefile 已重新打包并覆盖根目录，canonical/root 大小 `47,895,484` bytes，SHA-256
  `78DB186E8BFD92E48F09A1E8A9830DA16B45F2995912D8A63682C689944A87D4`，archive listing SHA-256
  `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.89（已完成源码、offscreen motion lifecycle vector 与 onefile 交付，2026-08-10）：在 `lifecycle.on_motion_frame()` 增加关闭/不可见/最小化/共享动效策略门禁；阻断排队过期帧时统一停止 presentation consumers 并清除 data-activity 投影，不改变 ViewModel、传输和发送语义。独立复核代理超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,896,526` bytes，SHA-256 `8D18CF6A28BA1EF0C3062ABDB097AA3F5A12D764B930FBA1F208F131FF8BE565`，archive listing SHA-256 `29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.90（已完成源码、application catalog、三主题 offscreen capability vector 与 onefile 交付，2026-08-10）：新增只读“扩展 / 工具站”Tab，application DTO 明确 OTA 传输/安全候选和 RTT/J-Link attach-only 状态；presentation 不导入 adapter、socket、pyserial、SDK/DLL 或密钥。route beacon 与 Tab glyph 从三节点扩展为四节点，未改变前三个业务页的 `protocol_tab_index`、焦点或业务状态；独立复核代理超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,911,727` bytes，SHA-256 `82EC5FC027001D4CD34D0114AB0BDC0FC3B5793B14E3B332AD7D0DA5DCC5A36`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.91（已完成源码、三主题 offscreen hierarchy vector 与 onefile 交付，2026-08-10）：为扩展 capability card 增加独立标题 selector 与状态 badge selector；标题使用正文 token，`contract_only` 使用 info surface/border，`attach_only` 使用 history surface/border 和 purple accent。没有新增 token、DTO 字段、业务动作或动画时钟；独立复核代理超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,914,013` bytes，SHA-256 `30AB41BD37BCE478565413BEB43A19644F608AA0865C023868A77A64889D55AB`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.92（已完成源码、offscreen workspace effect lifecycle vector 与 onefile 交付，2026-08-10）：统一 `workspace_runtime.py` 的 opacity effect 释放路径，stop/finish 均恢复 opacity、验证 Qt 对象并解绑 page graphics effect；快速切换会先清理上一页，低动效/隐藏/关闭仍静态回退。未改变 Tab 路由、业务状态、动画时长或新增 timer；六角色和独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,915,018` bytes，SHA-256 `E983B2B6EC35B1877F4E46A5063181E4E0DF99262B344DF38FCE8E261035D7B7`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.93（已完成源码、三主题 semantic vector 与 onefile 交付，2026-08-10）：移除 capability title 的 `role=status`，保留 state badge 的 `role=status`/`state`；QSS objectName、DTO、卡片布局、只读和 OTA/debug 边界不变。六角色与独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,913,940` bytes，SHA-256 `69D6713DFD819D1569792A82A5870D8B2927A52D831674E01B3EC94DF2A699D3`，archive listing SHA-256 `E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.94（源码、三主题 UART summary vector 与 onefile 交付已完成，2026-08-10）：新增 `UartTimingSummarySurface` 只读投影当前 UART 下拉值，支持默认值、手动改变和连接快速配置；摘要不进入 ViewModel/TransportConfig，不新增 timer 或业务状态。六角色与独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,916,119` bytes，SHA-256 `E6D317841A8983C81CACED40C9798DB70444016E661418589436BBA844F5328D`，archive listing SHA-256 `D41985C7165A66BD66752652C63615CA83A29BC83F22CEED3360D0E176A7F09F`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.95（源码、三主题扩展站概览 vector 与 onefile 交付已完成，2026-08-10）：新增 `embedded_station_overview.py`，从既有 immutable capability catalog 派生能力槽位数、已激活后端数和当前动作，并明确只读规划层与接入前置条件；不新增后端、动作、状态源、timer 或依赖。六角色与独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,919,227` bytes，SHA-256 `DBABFB57AD3D63393D57B1C3281C1A1C16CC0E7C572928ABFD940FFE660E5618`，archive listing SHA-256 `A11F34E225D750036A4AEE1DF2D77E2AE1B09B1C79F71719DB10FE6512852CA6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- ARCH-7d / UI-1.96（源码、三主题 summary boundary vector 与 onefile 交付已完成，2026-08-11）：新增 `application/extension_station.py` 的 frozen `ExtensionStationSummary` 与派生函数，presentation overview 只渲染 DTO，不再解释 `contract_only/attach_only`。边界校验、静态门禁、compileall、ruff 与 offscreen vector 通过；六角色与独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,921,659` bytes，SHA-256 `23B00CF14295A6F5AC399913ED1C8E46EDD8EB1262A4EC3EB1BE2A88367BD950`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.97（源码、真实组合根 UART options vector 与 onefile 交付已完成，2026-08-11）：将数据位、校验、停止位、流控下拉改为中文用户文案，波特率明确不可编辑，并为五个 UART selector 补齐 tooltip/accessibility description；currentData 的 domain enum/string contract、快速配置和连接行为不变。六角色与独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,920,572` bytes，SHA-256 `D4014B1E307BABA9926339CA04FEDCE37F6E7FFD8813D9CBCEEE30AE58AC54BE`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.98（源码、真实组合根 connection options vector 与 onefile 交付已完成，2026-08-11）：细化网络/BLE/RTT/TCP Server 的 tooltip/accessibility affordance，将 RTT/写入模式/客户端等用户文案本地化，并把零值超时状态显示为“未设置”；端口/范围、itemData、连接行为和后端边界不变。六角色与独立复核均超时，父代理完成五轴审查；onefile 已覆盖根目录，canonical/root 大小 `47,923,829` bytes，SHA-256 `DECC92C2D15BB110FB0977AEAB57A8BDD9E814B36F7037BBAFF0F0E5FBE2D753`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.99（源码、真实组合根 protocol options vector 与 onefile 交付已完成，2026-08-11）：本地化协议 preset/帧格式/校验/长度字段/字节序/回放速度，并统一 Component/Dataset/Curve 的动态刷新、空态、状态与流水线摘要文案；typed enum/int/string/DTO、解析 gate、snapshot、既有动效和连接行为不变。两轮六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,925,745` bytes，SHA-256 `5DA3C31E0009294AAC921427A94F01BCA578C6A0AD58E2971C1D8193D319F0C4`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.100（源码、真实组合根 pipeline state vector 与 onefile 交付已完成，2026-08-11）：`PipelineSurfaceLabel` 复用既有 `state/source` 动态属性和共享 MotionController，将六种状态映射到节点数量/语义色/静态阻塞标记，并仅让 active/transition/draft 产生脉冲；无新增 timer、状态源、业务依赖或 I/O。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,925,850` bytes，SHA-256 `7038F0C57040B6B518A051E1B779D1F58F01DF0B452A8991C54586F62D33BCA5`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.101（源码、真实组合根 SignalField vector 与 onefile 交付已完成，2026-08-11）：在既有 `SignalFieldWidget` 内增加三枚资源无关星芒、共享帧驱动的彗尾和轨道光点，复用 ThemeSpec、MotionController、NoFocus/鼠标透明和生命周期静态回退；无新增 timer、业务状态、资源、线程、I/O 或依赖。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,928,874` bytes，SHA-256 `C8A3B9581503479EAFCE01C7675C2F1BDF0D9250CFF45C13ACF4C45B0AC700E7`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.102（源码、真实组合根 extension surface vector 与 onefile 交付已完成，2026-08-11）：为 `extensionStationOverview`、`extensionCapabilityCard` 及 contract-only/attach-only 状态补齐 base/variant shell QSS，复用已有 ThemeSpec token，修复透明/原生默认卡片回退；无新增 DTO、状态源、timer、依赖或动作。首轮发现并修复不存在的 `ThemeSpec.border_strong` 字段后，六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,928,081` bytes，SHA-256 `717F20E69CFA0120A1DB833509FEEBCDB37AA66199559A6273B75FEF3E1DD9F1`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.103（源码、真实组合根 workspace surface vector 与 onefile 交付已完成，2026-08-11）：为 `workspaceShell` 与 `workspaceRouteStrip` 增加 base/variant controls QSS，复用已有 ThemeSpec token，建立外壳/路线条稳定主题边界；Tab、route beacon、生命周期与业务状态不变。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,928,961` bytes，SHA-256 `4F0300C4764F94F25825C8B5795D348BD04D1AFF8A43E6078B7676FA909B70D1`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.104（源码、真实组合根 statusbar chrome vector 与 onefile 交付已完成，2026-08-11）：为 `QStatusBar::item` 与 `QStatusBar QLabel` 增加 base/variant controls QSS，消除 Qt 原生 item 的透明/白色边界回退；StatusFooterSurface、native status text、状态源、lifecycle 和业务行为不变。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,928,792` bytes，SHA-256 `97E43617E1F0B148F713A6049C5338DD3FBF94AE8CE7ACFE9B0F127BCBB8C087`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.105（源码、真实组合根 transport selector vector 与 onefile 交付已完成，2026-08-11）：为 `QComboBox#transportCombo` 增加 base/variant shell QSS，复用已有 ThemeSpec token，覆盖六种传输入口的默认/hover/focus/disabled 层级；itemData、不可编辑、accessibility、连接行为和业务状态不变。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,929,551` bytes，SHA-256 `D4B68CBE1D4BB00C4C8F35C4BDC8ED9E4B79BE5EC782C865EE39B31D37A1A7B1`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.106（源码、真实组合根 theme picker vector 与 onefile 交付已完成，2026-08-11）：为 `QComboBox#themePicker` 增加 base/variant shell QSS，复用已有 ThemeSpec token，覆盖三个主题入口的默认/hover/focus/disabled 层级；key、icon、itemData、不可编辑、主题 signal、transition 和业务状态不变。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,929,814` bytes，SHA-256 `82668145CA2F8AA903E3D2BD2A02D8AC4B3B00AAE2DCAD87C9D6F6B42711C2C5`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.107（源码、真实组合根 connection rail vector 与 onefile 交付已完成，2026-08-11）：在既有 `ConnectionStatusRail` 内增加三枚共享帧彗尾、外环与中心光点，复用 `ThemeSpec` 状态语义色和现有生命周期静态回退；三主题六状态通过，未新增 timer、业务状态、公开 API、资源、线程、I/O 或传输依赖。六角色与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,929,693` bytes，SHA-256 `6785E9F51A9E907A647E2B84A050EE54F93B1925C91B3581DDEEABFC3292434D`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.108（源码、真实组合根 selector affordance vector 与 onefile 交付已完成，2026-08-11）：为 transport、终端显示/发送、历史、批量和编辑器选择器补齐显式不可编辑、tooltip 与 accessible description；UART 端口保持唯一可编辑 combo 并补齐 accessibility 文案。27 个组合根 selector、三主题 980/1180、关闭生命周期、静态门禁、compileall、ruff 与 provenance 通过；itemData、signal、连接 gate、批量执行和主题行为不变。六角色前置评审两轮与独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,930,586` bytes，SHA-256 `27EBF48272AACF3E5CCC3586ABFC8988272A737E80A6D24D57FD001A8B9E6FA4`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。
- UI-1.109（源码、真实组合根 action affordance vector 与 onefile 交付已完成，2026-08-11）：为清除错误/终端预览/历史、快捷命令、自定义连接保存删除以及 UART 读写超时补齐副作用提示与 accessible description；动作、范围、回调、DTO、状态源和连接/发送/记录语义不变。9 个目标控件、27 个 selector、三主题 980/1180、关闭生命周期、静态门禁、compileall、ruff 与 provenance 通过；首轮验证发现并修复保存/删除自定义连接的两条缺失描述，六角色两轮与最终独立复核均超时，父代理完成五轴及 embedded applicability/simplification 审查；onefile 已覆盖根目录，canonical/root 大小 `47,931,714` bytes，SHA-256 `971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`，archive listing SHA-256 `C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6`，签名 `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`。

### 后续

- ARCH-6q（源码、真实组合根 vector 与 onefile 已完成，2026-08-11）：新增 frozen/slots `WorkspaceShellBindings`，由
  `workspace.py` 唯一组装；bootstrap、workspace runtime、focus transition、焦点顺序、lifecycle、derived 和
  terminal owner 通过 `workspace_bindings_for()` 消费，移除散落 `_workspace_tabs/_workspace_route/...` 访问。
  不改变 Tab、滚动提示、专注模式、共享动效或业务状态；三主题 × 980/1180 × 四 Tab 共 24 组、horizontal
  maximum=0、exact-white/near-white=0、static/compileall/ruff/source-limit/theme-token/provenance 门禁通过。
  canonical `47,960,028` bytes，SHA-256 `BACE302A92900CBC7A8D27D41D6BDF1082C70AE58F64F3F0D305AAF737701E63`，
  archive listing SHA-256 `CFC788B805981C451920D9757F1FE48CD618C9049058B8ED52B20B70CF53EAC0`；根目录原名仍等待
  PID 46108/49236 退出，`SerialForge-latest.exe` 与 canonical 字节一致。
- UI-1.136（源码、真实组合根滚动提示 vector 与 onefile 已完成，2026-08-11）：为共享 `workspaceRouteStrip` 增加
  `WorkspaceScrollHint`，只观察 active settings page 的 native vertical scrollbar，提供顶部/中部/底部/无溢出
  四态的短文案、tooltip 与 AccessibleDescription；Tab 切换时由 `workspace_runtime` 解绑旧 signal 并绑定当前页。
  不新增业务状态、DTO、timer、导航模型或第二套滚动策略；三主题 × 980/1180 × 四 Tab 共 24 组、horizontal
  maximum=0、exact-white/near-white=0、source/static/compileall/ruff/provenance 门禁通过。canonical onefile
  `47,957,987` bytes，SHA-256 `D028C8D939A57A64BCD991CF6E0AAF404883450307974049566891560696DACC`，
  archive listing SHA-256 `B9F1EB9352EC1F155CE7E8F334DD34FCE442221281D32C4CBE59AE3276B7FE9E`；根目录原名仍等待
  PID 46108/49236 退出，另生成 `SerialForge-latest.exe` 与 canonical 字节一致。
- UI-1.134（源码、UART selector 响应式宽度与三主题六传输 vector 已完成，2026-08-11）：
  定位并修复 980px 链路页 UART 八列 grid 的 105px 横向溢出；将 port 上限收敛到 200px，
  data/parity/stop/flow selector 分别 bounded 为 84/110、84/150、84/130、110/160px，
  波特率沿用 110/150px。itemData、不可手输、端口 COMx、tooltip/accessibility、连接 gate、
  DTO 和选项语义不变。三主题 × 980/1180 × 6 transport + 4 workspace 共 60 组通过，
  `hmax=0`、`exact-white=0`，截图为 `build/ui_review_ui134_connection_responsive.png`；
  scripts/check.ps1、compileall、Ruff、源码行数与 provenance pass。架构师线程超时，父代理
  完成六轴与简化审查。onefile source revision `local-ui-1.134`，canonical 47,950,558 bytes，
  SHA-256 `A5282CDCFF9FF31321C46DD382E8A04CA1BCE39BE16B05D6A3CC4004C0B91057`，archive listing
  SHA-256 `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，签名
  `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`；根目录仍等待 PID 46108、49236 释放。
- UI-1.133（源码、扩展工具站路线 glyph 与三主题真实组合根 vector 已完成，2026-08-11）：
  在 `EmbeddedStationOverview` 内加入 96×64 资源无关三节点接入路线 glyph，复用既有
  `ThemeSpec` 与共享 `MotionController` frame/stop；通过 frozen `ExtensionPanelWidgets`
  把 layout 与 overview typed 暴露给 workspace，lifecycle 继续使用显式 motion surface
  fan-out。只读 `ExtensionStationSummary`、7 张能力卡、contract-only/attach-only、
  accessibility、OTA/debug 边界和业务状态不变。三主题 × 980/1180 overview 验证通过，
  horizontal maximum=0，隐藏窗口静态回退，截图为 `build/ui_review_ui133_extension.png`；
  scripts/check.ps1、compileall、Ruff、源码行数与 provenance 通过。架构师线程和独立审查
  均超时，父代理完成六轴与简化审查。onefile source revision `local-ui-1.133`，canonical
  47,951,506 bytes，SHA-256
  `1E2004935AFC936E9BD569C0063552EFC57DEE2C0F20B6DE64A494CB4D0F7BE6`，archive listing
  SHA-256 `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，签名
  `NotSigned`，正式发行资格 `false`，硬件验收 `not_run`；根目录仍等待 PID 46108、49236 释放。
- UI-1.132（源码、命令批量空态 copy 几何与三主题真实组合根 vector 已完成，2026-08-11）：
  为 `CommandBatchEmptyState` 的 eyebrow/title/hint QLabel 统一设置纵向 `Fixed`，保留空态
  卡片 `Expanding/Preferred`、glyph、CTA、word-wrap、共享 frame、signal、命令/无障碍边界，
  修复 980/1180 下文案被拉伸为 85/119px 的纵向断裂。三主题 × 980/1180 空态/长 hint 验证
  通过，copy 高度稳定 14/19/30px，horizontal maximum=0、exact-white=0；截图为
  `build/ui_review_ui132_command_empty.png`。静态门禁、compileall、Ruff 与 provenance pass；
  架构师线程超时，父代理完成五轴与简化审查。onefile 已生成，source revision `local-ui-1.132`，
  canonical 47,945,280 bytes，SHA-256
  `68B7C0E6387C621FAAA889D8440DFCDB8F509DF407A01F833C36980BCC9254D2`，archive listing SHA-256
  `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，签名 `NotSigned`，
  正式发行资格 `false`，硬件验收 `not_run`；根目录仍等待 PID 46108、49236 释放。
- UI-1.131（源码、网络/RTT hint 几何与六传输三主题真实组合根 vector 已完成，2026-08-11）：
  将共享 `_configure_responsive_hint()` 的纵向 size policy 从 `Preferred` 收敛为 `Fixed`，
  修复 TCP Client/RTT 说明 QLabel 在专注模式分别拉伸到约 151/111px 的空白；保留 word-wrap、
  120/520px 宽度边界、tooltip/accessibility、TCP Server allowlist、UDP/BLE/UART 行为和连接
  DTO，不新增状态源、timer、I/O 或依赖。三主题 × 980/1180 × UART/TCP Client/TCP Server/
  UDP/BLE GATT/J-Link RTT 共 36 组通过，标题/active panel 无重叠、horizontal maximum=0、
  exact-white=0；TCP/RTT 截图为 `build/ui_review_ui131_tcp_hint.png`、
  `build/ui_review_ui131_rtt_hint.png`。静态门禁、compileall、Ruff 与 provenance pass；架构师
  线程超时，父代理完成五轴与简化审查。onefile 已生成，source revision `local-ui-1.131`，
  canonical 47,947,904 bytes，SHA-256
  `61379AF7545EBF6745B726D56BFA41121640055BCAEE92EA82B47351EA238AF2`，archive listing SHA-256
  `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，签名 `NotSigned`，
  正式发行资格 `false`，硬件验收 `not_run`；根目录仍等待 PID 46108、49236 释放。
- UI-1.130（源码、UART 摘要 rail 几何与三主题真实组合根 vector 已完成，2026-08-11）：
  将 `UartTimingSummarySurface` 的纵向 size policy 从 `Preferred` 收敛为 `Fixed`，修复专注
  模式稳定后摘要 rail 从 size hint 约 38px 被拉伸到约 109px 的空面板；保留横向响应式、摘要
  文案、tooltip/accessibility、selector signal、连接 gate 和业务 DTO，不新增状态源、timer、
  I/O 或依赖。三主题 × 980/1180 的真实组合根摘要高度均为 38px，horizontal maximum=0、
  exact-white=0；截图为 `build/ui_review_ui130_uart_summary.png`。静态门禁、compileall、Ruff
  与 provenance pass；架构师线程超时，父代理完成五轴与简化审查。onefile 已生成，source
  revision `local-ui-1.130`，canonical 47,947,485 bytes，SHA-256
  `3AC144396C9EB88AD9B447572A3B01965E5EF07641A95621AEEEF2E2210DF52C`，archive listing SHA-256
  `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，签名 `NotSigned`，
  正式发行资格 `false`，硬件验收 `not_run`；根目录仍等待 PID 46108、49236 释放。
- UI-1.129（源码、UART 空端口可用性与三主题真实组合根 vector 已完成，2026-08-11）：
  为可编辑 UART 端口 combo 同时设置 `QComboBox` 与内部 `QLineEdit` 的主题化 placeholder，
  修复无端口时输入区看起来完全空白的问题；保留 `COMx` 手动输入、currentText/连接 gate、
  刷新动作、焦点和 accessibility，不增加状态源、timer、I/O 或业务语义。首轮运行时确认了
  Qt editable combo 不会自动把 combo placeholder 传给 line edit，随后在同一 builder owner 内
  修正；三主题 × 980/1180 placeholder、手输/清空、horizontal maximum=0、exact-white=0 通过，
  截图为 `build/ui_review_ui129_uart_empty_port.png`。静态门禁、compileall、Ruff 与 provenance
  pass；三轮架构师线程均超时，父代理完成五轴与简化审查。onefile 已生成，source revision
  `local-ui-1.129`，canonical 47,947,205 bytes，SHA-256
  `7E6EA48736EDF112EEDEBA86B65BCBE1BF4C9DF8522AACEF6D0C61E140272620`，archive listing SHA-256
  `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，签名 `NotSigned`，
  正式发行资格 `false`，硬件验收 `not_run`；根目录仍等待 PID 46108、49236 释放。
- UI-1.128（源码、连接 section 几何与三主题真实组合根 vector 已完成，2026-08-11）：
  抽取 `controllers/connection_builder.py:_section_label()`，让 UART/网络/BLE section 标题
  保持 `role="section"` 和纵向 26px 左右的固定高度，修复 980×680 专注设置视图中 UART
  标题被拉伸为约 106px 的空白；不改变 panel 可见性、连接/transport 状态、焦点或主题语义。
  三主题 × 980/1180 × UART/TCP Client/TCP Server/UDP/BLE/RTT 共 36 组通过，标题/active panel
  无重叠、horizontal maximum=0、exact-white=0；静态门禁、compileall、Ruff 和 onefile provenance
  pass。source revision `local-ui-1.128`，canonical 47,947,626 bytes，SHA-256
  `D7EB3B79B0E86F46AB24AF5B951496B25F1797627021DA4A7786545A87A951B1`，archive listing SHA-256
  `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；根目录仍等待 PID
  46108、49236 释放。
- UI-1.127（源码、专注/总览过渡与三主题真实组合根 vector 已完成，2026-08-11）：
  在路线条加入可访问的“专注设置 / 返回总览”按钮；专注模式只隐藏实时观测、终端和发送区，
  通过一次性 `QParallelAnimationGroup` 释放连接/协议/命令/扩展设置空间，session、接收、记录、
  pipeline 与设备连接保持运行。低动效/暂停/隐藏/最小化/关闭静态收敛并清理动画，不持久化模式，
  不新增业务状态、QTimer、滚动策略或 OTA/AES/RTT/J-Link 依赖。动态中间帧、反向恢复、低动效、
  hide/show、三主题 × 980/1180 × 四 tab、exact-white=0 和 horizontal maximum=0 已验证；
  架构师线程 `019fed62-45b0-7dc0-9a44-736aad31aff3` 在限定窗口内超时，未计为独立通过；
  父代理完成 owner、生命周期、焦点/无障碍、性能和简化审查。source revision `local-ui-1.127`，
  canonical 47,947,582 bytes，SHA-256
  `85E4B0695D12DD7287E142E0E8E7E1B08159A66363E69DB82AF14AD21D75ED5A`，archive listing SHA-256
  `2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`，package/provenance pass；
  根目录仍等待 PID 46108、49236 释放。
- UI-1.126（源码、三主题 980/1180 真实组合根响应式几何 vector 已完成，2026-08-11）：
  取消 `build_workspace_tabs()` 中与最小窗口父级空间冲突的 `QTabWidget` 220px 硬最小高度，
  让现有 settings scroll viewport 在 980×680 下收缩，并保持固定 route strip 不被 tab 内容覆盖。
  三主题四工作区 exact-white=0、当前页 horizontal maximum=0、tab/route overlap=false、
  route shell overflow=false；未新增业务状态、timer、滚动策略、设备 I/O 或 OTA/AES/RTT/J-Link
  依赖。架构师线程 `019fed57-c98f-7330-a61a-7a69ce726a67` 在限定窗口内超时，未计为独立通过；
  父代理完成 owner、几何、可滚动性、可访问性、性能与简化审查。source revision
  `local-ui-1.126`，canonical 47,943,352 bytes，SHA-256
  `5C5F093587686FF2A56C409CC3D1F9A84823ECF1F1322D6F83EB7D4692133563`，archive listing SHA-256
  `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`，package/provenance pass；
  根目录仍等待 PID 46108、49236 释放。
- UI-1.125（源码、三主题连接轨道终态 marker vector 与 canonical onefile 已完成，2026-08-11）：
  在 `ConnectionStatusRail` 既有节点路径上增加 open 完成勾与 error 叉号，opening/open/closing
  继续复用共享 MotionController frame，closed/discovered、stop、reduced-motion、暂停、隐藏/
  最小化/关闭保持静态；不新增状态源、业务动作、timer、线程、I/O、OTA/AES/RTT/J-Link 依赖，
  controller/ViewModel/连接行为不变。真实 production host 三主题六状态、animated transition、
  stop 与 exact-white=0 vector 通过；首轮独立 rail 抓图误用通用 QWidget 导致白色背景，已修正为
  `QWidget#appRoot` + `QFrame#connectionControlBand` 生产宿主后重新通过。architect 线程
  `019fed4f-d2a8-7d33-9cdd-b536efe3e106` 在限定窗口内超时，父代理完成 owner/token/绘制边界/
  accessibility/性能/简化审查。source revision `local-ui-1.125`，canonical 47,942,017 bytes，
  SHA-256 `ACC5FCA833E17EF0D648FCBE7CB7DB1ECB8557B43BD4C920371EF2CCDEF49142`，archive listing SHA-256
  `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`，provenance pass；根目录仍
  等待 PID 46108、49236 释放。
- UI-1.124（源码、三主题连接快速配置上下文 vector 与 canonical onefile 已完成，2026-08-11）：
  `ConnectionPresetContextSurface` 继续只消费既有 preset projection，使用
  `refresh_dynamic_property()` 同步 `source/state`，并将空态、内置、自定义来源分别渲染为
  neutral/info/history surface、border 与 accent；不自动连接、不新增状态源、timer、线程、I/O、
  OTA/AES/RTT/J-Link 依赖，hint/accessible description、shared frame/stop、NoFocus 和焦点路径不变。
  三主题空态/内置/自定义 vector、非白色采样像素、静态门禁、compileall、ruff 与 provenance pass；
  architect 线程 `019fed48-9164-72c3-91f3-7472ab0cf517` 在限定窗口内超时，父代理完成 owner/token/
  QSS/polish/accessibility/简化审查。source revision `local-ui-1.124`，canonical 47,941,349 bytes，
  SHA-256 `79A366634CF0B87C629416194AC8891F7BFE6BB08CB680F3DE050D9A99D3E3BC`，archive listing SHA-256
  `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`，provenance pass；根目录仍等待
  PID 46108、49236 释放。
- UI-1.123（源码、三主题 Component waiting/blocked vector 与 canonical onefile 已完成，2026-08-11）：
  将 `ComponentEmptyStateSurface.set_action_enabled()` 的既有 gate 显式投影为 `waiting` 或 `blocked`，
  同步 `COMPONENT / WAITING` / `COMPONENT / BLOCKED` 眉题，并由 base/variant QSS 赋予 blocked 的
  neutral surface/warning 边界；CTA/glyph、setText/text、load intent、shared frame/stop 和焦点/无障碍
  行为不变。三主题两态图、source gate、CTA visible/enabled、NoFocus、鼠标透明和中文可读性通过；source
  revision `local-ui-1.123`，canonical 47,939,601 bytes，SHA-256 `BC3D81E7714111A63B18DC9679B34A6F7D874830BEF4537F56116F07ED5B36D1`，
  archive listing SHA-256 `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`，provenance pass；
  根目录仍等待 PID 46108、49236 释放。
- UI-1.122（源码、三主题分析状态 vector 与 canonical onefile 已完成，2026-08-11）：在共享
  `AnalysisStatusLabel` 中为 protocol/component/dataset/curve/replay 既有 `state/source` 增加
  semantic marker；active/waiting/draft 移动态复用 shared MotionController frame 绘制低对比度
  pulse，保留原生文字、AccessibleDescription、底部五节点 rail、NoFocus、鼠标透明和静态回退。
  真实组合根 `980x680/1180x780` 横向 scroll range 均为 0，五类 status width 均不低于 minimum
  size hint；三主题截图与孤立状态图通过。source revision `local-ui-1.122`，canonical 47,941,295
  bytes，SHA-256 `E881FA27DD8B0564C195A95A2B94A1CF521772438AA18A6EEAD1A6BF0957FB55`，archive listing
  SHA-256 `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`，provenance pass；根目录
  仍等待 PID 46108、49236 释放。
- UI-1.121（源码、三主题 command status vector 与 canonical onefile 已完成，2026-08-11）：在
  `CommandBatchSurfaceLabel` 内为既有 `empty/ready/running/completed/stopped/failed` 投影增加
  semantic status marker；running 时复用共享 MotionController frame 绘制呼吸环，状态文字、步骤
  rail、snapshot、结果表、batch gate、NoFocus、鼠标透明和 reduced-motion/static fallback 不变。
  没有新增 timer、业务状态、设备进度、ACK、backend、OTA/debug 依赖。三主题离屏渲染与可访问性/生命周期
  vector 通过；source revision `local-ui-1.121`，canonical 47,941,026 bytes，SHA-256
  `7A9DC10ED43B2B9876B2FCF03F422A70E14765901E616206AF9041940DF2D20F`，archive listing SHA-256
  `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`，provenance pass；根目录
  仍等待 PID 46108、49236 释放。
- UI-1.120（已完成源码、三主题响应式 vector 与 canonical onefile，2026-08-11）：为扩展工具站
  contract-only / attach-only 能力卡增加三主题对称 hover selector，并为三组两列 grid 增加等权
  stretch；保持静态 NoFocus、无动作、原 DTO/accessibility、既有 page fade 和 OTA/J-Link 边界。
  真实组合根 980/1180、三主题、四 Tab、7 cards、vertical range、horizontal range=0、selector
  和视觉截图通过；tooltip/accessibility 共用边界说明；source revision `local-ui-1.120`，canonical
  47,940,170 bytes，SHA-256 `A5C9FFB974C712D65B7212A252F860972924AF3817455BC5B8432FD8AE8E0C46`，provenance pass；根目录
  仍等待 PID 46108、49236 释放。
- UI-1.119（性能实验已完成并全部撤回，2026-08-11）：逐帧 `isVisible()` 过滤从全量参考均值 `591.13ms/120帧` 退化到 `678.89ms/120帧`；第二个“Tab 切换时缓存可见 surface”实验虽然把协议页集合从 56 降到 39，但同一 offscreen 合成根含 `processEvents()` 的端到端均值从 `781.29ms` 退化到 `821.50ms`，因此同样撤回。两次证据均记录在 `docs/PERFORMANCE.md`，生产动效广播保持 UI-1.118 contract，未因负收益打新包。
- UI-1.118（源码与 canonical onefile 已完成，2026-08-11）：增强 stable/theme controls 的 vertical/horizontal `QScrollBar` thumb，默认复用 `BORDER_STRONG`/`accent_purple`，hover 复用 `accent_pink`，pressed 复用 `ACCENT`/`theme.accent`；保持无 arrow button、QScrollArea/QPlainTextEdit 滚动范围、焦点/键盘和业务状态不变。2 尺寸 × 3 主题 × 4 Tab 的真实 scrollbar range/可见 thumb/pressed selector vector、24 张视觉截图、source limit、compileall、ruff、theme token audit 和 provenance verify 通过；架构师与独立审查线程超时，父代理完成 token/selector/native behavior/简化审查。source revision `local-ui-1.118`，canonical 47,938,827 bytes，SHA-256 `F04CC42821DFC6F3BEB3F7DEE143C83B51D22D19025EF22DCD5C736C55C18A3C`，archive listing SHA-256 `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；根目录仍等待 PID 46108、49236 释放。
- UI-1.117（源码与 canonical onefile 已完成，2026-08-11）：新增 `presentation/font_runtime.py`，在 QApplication 创建后选择可用 CJK 系统字体；仅当 Qt 字体数据库没有 CJK family 且标准本机字体文件存在时通过 `QFontDatabase.addApplicationFont()` 注册，不复制字体资产、不改变业务状态。stylesheet fallback 顺序补齐 `Microsoft YaHei UI`、`Microsoft YaHei`、`Segoe UI`、`Noto Sans SC`。字体向量、三主题 × 四 Tab 1180×780 视觉审计、980×680 无横向溢出向量、source limit、compileall、ruff、theme token audit 和 provenance verify 通过；架构师与独立审查线程超时，父代理完成边界/失败回退/简化审查。source revision `local-ui-1.117`，canonical 47,940,611 bytes，SHA-256 `A8382CDA77F2601BDD4465468881C0F056EB86FD307090BDC89B02C4C7162333`，archive listing SHA-256 `D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；根目录仍等待 PID 46108、49236 释放。
- UI-1.116（源码与 canonical onefile 已完成，2026-08-11）：将连接配置、协议应用/重置、历史回放、错误清除、终端清空、发送、快捷命令保存、发送历史和批量命令动作统一复用 `ActionRailButton`；`BusyActionButton` 保持原 busy contract，lifecycle 显式收集 16 个新增 surface。回调、objectName、enabled gate、焦点/Tab、payload 和业务状态不变。三主题真实组合根 vector、1180x780 视觉审计、静态门禁、compileall、ruff 和 provenance 通过。source revision local-ui-1.116，canonical 47,937,619 bytes，SHA-256 D80631FE5B2F9DCF213ADCAB16083BBAC7BAEA49EDB1CC39A683F7BFCE494448；根目录仍等待 PID 46108、49236 释放。
- UI-1.115（源码与 canonical onefile 已完成，2026-08-11）：新增独立 `AnimatedWorkspaceTabBar`，保留原生 `QTabBar` 绘制、currentChanged、键盘/焦点和 icon 语义，仅叠加当前 Tab 的主题 signal underline/halo；`workspace.py` 负责显式组装，`lifecycle.py` 复用已有 frame/stop fan-out，不新增 timer、业务状态或 MotionController。真实组合根三主题/四 Tab vector、视觉审计、静态门禁、compileall、ruff 和 provenance 通过。source revision local-ui-1.115，canonical 47,937,677 bytes，SHA-256 8B10ABABAB674833AE4FCA663D9ACD81AF59225F53C046F11BC130410249B786；根目录仍等待 PID 46108、49236 释放。
- UI-1.114（源码与 canonical onefile 已完成，2026-08-11）：复用既有 `ActionRailButton` 为四个协议/组件/Dataset 派生动作增加 signal rail 绘制，并由 `lifecycle._motion_surfaces()` 统一接入 frame/stop fan-out；不新增 timer、业务状态、回调、DTO、依赖或 I/O。三主题 enabled/disabled offscreen vector、静态门禁、compileall、ruff 和 provenance 通过。source revision local-ui-1.114，canonical 47,934,511 bytes，SHA-256 7608148F5BE4E3F30F82379C78A94B749320E30809A2B32A5B7ECBEF8CBC14C6；根目录仍等待 PID 46108、49236 释放。
- UI-1.113（源码与 canonical onefile 已完成，2026-08-11）：为组件 Profile/Codec 加载、组件 CSV 导出、Dataset 配置加载和 Dataset CSV 导出补齐动作专属 enabled/disabled tooltip 与 accessible description；三主题两态四动作 offscreen vector、共享文案 contract、静态门禁、compileall、ruff 和 provenance 通过。source revision local-ui-1.113，canonical 47,934,870 bytes，SHA-256 3CBA66937B7006831920BBD395604EA7B838E70E5600B64A22B2D01E432278AA；根目录仍等待 PID 46108、49236 释放。
- UI-1.110～UI-1.112（源码与 canonical onefile 已完成，2026-08-11）：按 owner-local 原则补齐回放/连接/录制动作、UART 字段、发送区、批量/preset/确认对话框和快捷命令 QAction 的 tooltip/accessibility 语义；真实组合根 offscreen vectors、三主题渲染、近白像素、关闭生命周期、静态门禁、compileall、ruff 与 provenance 通过。source revision local-ui-1.112，canonical 47,932,571 bytes，SHA-256 EB975C422B68D618C6B0E09669D8843689752C0153E540193F213B664FE0254E；两个旧运行实例占用根目录，root-exe 等待用户关闭后覆盖。
- 继续细化协议、Component、Dataset、Curve 的真实 busy/error/history 状态动效，但只投影已有 snapshot/状态事实。
- 继续细化主题过渡与工作区切换的节奏，但保持一次性动画与共享 MotionController 分工，不把过渡变成业务时钟。
- 继续细化协议、Component、Dataset、Curve 的空态、busy、历史来源与摘要层级；UI-1.19 已先完成流水线摘要条的来源/状态轴。
- 主窗口拆分优先于继续向 `main_window.py` 添加新 UI 逻辑；每个 controller 迁移完成后再继续下一轮视觉切片。
- 后续继续细化命令批量编辑/回放的文案层级与真实 snapshot 反馈，不新增 UI 进度源或伪造回放总进度。
- 后续视觉切片必须继续复用现有状态表面和 MotionController；禁止用新的常驻 timer 表达业务执行进度。
- 每个切片完成静态审计、代码质量复核、handoff 归档和 onefile 根目录覆盖。
- 获得授权后再做 GUI/offscreen/HIDPI/读屏/动效性能/EXE 启动/真实硬件验收。

## 全局验收门

- `ruff`、compileall、项目既有检查通过。
- `scripts/check_source_limits.py src` 通过，所有 Python 源文件不超过 1000 行。
- presentation 边界、无白色系统 palette 回落、共享动效时钟和资源审计通过。
- 不新增测试专用资产；不启动服务、网络、硬件或持续运行 GUI，除非用户单独授权。
- canonical EXE 与根目录 `SerialForge.exe` 字节一致，provenance verification 通过。
