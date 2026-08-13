# SerialForge 最新交接

> 本文件是 `docs/handoffs/` 下的最新交接入口。交接文件不放在仓库根目录；每轮还必须新增一份不覆盖历史的归档。

日期：2026-08-12  
当前增量：ARCH-128 / UI-1.201 协议页与命令页局部响应式宽度 + ARCH-127 / UI-1.200 UART 列级 sizing。  
当前增量状态：`source=pass`；`ARCH128_ALL_PAGES=pass`（108 rows）；`ARCH128_VISIBLE_LEAVES=pass`（27 rows）；
`MOTION_120HZ=pass`（249 frames / 119.063Hz，事件循环内）；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮将 UART 参数列宽、协议页局部 row/leaf 和命令页 action/empty header 收敛为局部高内聚 presentation owner。离屏矩阵覆盖三主题、
546/547/560/600/640/768/900/1120/1240px；四页外层共 108 行、协议叶子可见追加 27 行，均 `hmax=0` 且 content 不超 viewport。
架构师 `019ff646-9603-70e3-9d75-5e40bb286b44` APPROVE；独立代码审查 `019ff66c-28af-7561-9958-a4da1fea6c48` APPROVE（Critical/Required=0，
两条 advisory）；简化评估 `019ff66c-290b-7cd0-9d06-736a955dd567` 无必须项；独立性能审查 `019ff680-81dd-71f3-acff-93b9b614a319` 更新为 APPROVE（无 Required）。
静态门禁与最新 onefile 已完成：canonical/root/root-latest 均为 `48,079,010` bytes，SHA-256
`720131D96530341D0FFD9A271588A2BEDE1FF9829C9D3DE95D230E1DE5FE3688`；archive listing SHA-256
`489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`；`local-arch-128`、engineering build、
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  
归档：[`2026-08-12-arch-127-uart-column-sizing.md`](2026-08-12-arch-127-uart-column-sizing.md)、
[`2026-08-12-arch-128-responsive-page-widths.md`](2026-08-12-arch-128-responsive-page-widths.md)。

日期：2026-08-12  
当前增量：ARCH-126 / UI-1.199 Header 三态响应式控制带。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`RESPONSIVE_HEADER_AUDIT=pass`；`HEADER_EVENTS_FOCUS_LIFECYCLE=pass`；`HEADER_OWNER_CONTRACT_AUDIT=pass`；`package=pass`；`root-exe=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮修复 Header 状态摘要在窄宽度下被省略的问题：三个既有 QFrame 由独立 owner 按 `REGULAR/COMPACT/NARROW` sizing-driven 排列，状态文本在
546–1240px 和三主题下均完整；不改变 HeaderChromeBindings、signals、focus/Tab、accessibility、主题/动效 wiring、唯一 MotionController 或业务链路。
架构师 `019ff613-3aea-7f92-b6db-2567ca74cee7` 最终 APPROVE；代码审查 `019ff62a-986e-71c0-98f0-13007776a724`、修复复核
`019ff62a-3bae-7cd2-a4c2-c82732a8110a` 与简化评估 `019ff62a-98bb-7851-a19d-56c59142fded` 均为
`APPROVE WITH ADVISORIES`，Critical/Required=0。onefile 已以 `local-arch-126` 构建并覆盖根目录 `SerialForge.exe` 与
`SerialForge-latest.exe`：canonical/root/root-latest 均为 `48,064,034` bytes，SHA-256
`B55EF7C6BF173E5EEDB13CBD1F5355DA7249F0592684FABBEC85D85C3B7D6D91`；archive listing SHA-256
`489EE0ADAE68F2A57212C6A7776E7BF16E73598425936D81B45768C90BC0FDDA`，provenance verify pass；engineering build、签名
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  
归档：[`2026-08-12-arch-126-responsive-header.md`](2026-08-12-arch-126-responsive-header.md)。

日期：2026-08-12  
当前增量：ARCH-125 / UI-1.198 窄窗口响应式连接带与 UART 参数表单。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`UART_RESPONSIVE_AUDIT=pass`；`RESPONSIVE_WINDOW_AUDIT=pass`；`LIFECYCLE=pass`；`package=pass`；`root-exe=pass`；`root-latest=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮移除 `980×720` 硬窗口最小尺寸；连接带由 sizing contract 驱动的三态 owner 分行，UART 参数由独立三态 owner 排列；不改变业务、bindings、
signals、focus/Tab、scroll owner、主题 token 或唯一共享 MotionController。三主题 `520/546/560/640/768/900/980/1180px` 矩阵、事件与生命周期
通过；独立审查与简化评估最终均 `APPROVE WITH ADVISORIES`，Critical/Required=0。  
架构师 `019ff5c9-8eff-7cd3-9c2f-ab0a40ec9ef0` APPROVE；独立 reviewer `019ff5df-ef65-7f00-9b30-03c62fad4be9` 与简化 reviewer
`019ff5df-efac-7453-96b5-f9d61b07c0ff` 均完成最终复核；embedded C/C++ applicability=N/A。  
本轮 onefile 使用 `local-arch-125` 构建并覆盖根目录两个 EXE：canonical/root/root-latest 均为 `48,057,767` bytes，SHA-256 为
`C8F19055CA977477BA33C087E3CEEA92ACD99F328A2F7F2410189DF96D748BBA`；archive listing SHA-256 为
`0642B8F37772873BF7D7DB9590125ACA77B319F3E06B2308BBE86826951C538F`，provenance verify 通过；签名 `NotSigned`、`release_eligible=false`、
`hardware_acceptance=not_run`。  
归档：[`2026-08-12-arch-125-responsive-window-uart.md`](2026-08-12-arch-125-responsive-window-uart.md)。

日期：2026-08-12  
当前增量：ARCH-124 / UI-1.197 command context band + independent action row。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`COMMAND_CONTEXT_BAND_FINAL_GEOMETRY=pass`；`COMMAND_CONTEXT_WORKSPACE_LIFECYCLE=pass`；`MOTION_120HZ_ARCH124=pass`（1224 frames / 120.000Hz）；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮将发送历史与批量命令选择交由 `CommandContextBand` 按 cell sizing contract 在单列/双列间响应式重排；五个批量操作按钮保持独立 action row，
不改变业务、bindings、signals、焦点/Tab 顺序、滚动 owner、主题语义或唯一共享 MotionController。wrapper 不增加 `role=surface`、QSS selector、
timer、动画时钟或业务接口。三主题、320–1180 宽度几何阈值、style relayout、对象 identity、accessibility 与 hide/show/close 通过。  
架构师 `019ff5a5-cb5d-75b3-9fa6-c41bc2a66802`、`019ff5a8-f256-7002-b8fc-86de2dab2dd0`、`019ff5b5-7c29-7fa3-9c27-14b251e0e951`、
`019ff5c0-2320-7ec1-99d1-2b7d90444be1` 均 APPROVE；独立 reviewer `019ff5bc-af7c-7f91-bc2d-891292e96db0` 无 Critical/Required、
结论 `APPROVE WITH ADVISORIES`；简化 reviewer `019ff5bc-b25e-7743-9a2b-f0ae71f185f8` 无必须简化项；embedded C/C++ applicability=N/A。  
本轮 onefile 使用 `local-arch-124` 构建并覆盖根目录两个 EXE：canonical/root/root-latest 均为 `48,052,563` bytes，SHA-256 为
`B52DD4ADFC7C06CEAF9E2B46BCA1FB9AFB2DE6AA996B286AAF80ED9E12107B16`；archive listing SHA-256 为
`060BCD1C9D88F7C19657FA6A66794FB333F24BE0B8676605BACFBB4A121F9D1D`，provenance verify 通过；签名、正式发行资格与硬件验收为
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  
归档：[`2026-08-12-arch-124-command-context-band.md`](2026-08-12-arch-124-command-context-band.md)。

日期：2026-08-12  
当前增量：ARCH-123 / UI-1.196 command empty-state rhythm + first-frame deterministic relayout。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`COMMAND_EMPTY_FIRST_FRAME=pass`；`COMMAND_EMPTY_RESIZE_IDEMPOTENT=pass`；`COMMAND_EMPTY_THEMES=pass`；
`COMMAND_EMPTY_FIRST_FRAME_LIFECYCLE=pass`；`MOTION_10S=pass`（1224 frames / 120.000Hz）；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮空态改为自然高度，新增不抢焦点三步引导；`CommandBatchStepRail` 在初始化和 resize 共用幂等 `_relayout()`，命令页通过
`ShortPageVerticalRhythm.TOP` 顶部对齐，连接/协议/扩展页默认 `CENTER` 保持不变。不改变批处理 signals、ViewModel、执行链路、QScrollArea、
主题语义或唯一共享 MotionController。  
架构师 `019ff58a-9d3d-75b0-9638-95529059138a`、follow-up `019ff59a-996b-76e0-adbf-48a8633392a1` APPROVE；独立 reviewer
`019ff595-c839-7311-8c47-1a4006951912` 无 Critical/Required、结论 `APPROVE WITH ADVISORIES`；最终 reviewer
`019ff5a0-c897-7791-8c69-aeb9ffb23b47` 复核最终 `_relayout()` 后仍无 Critical/Required、结论 `APPROVE WITH ADVISORIES`；简化 reviewer
`019ff595-cb11-7370-b84c-6d088909da57` 与最终简化 reviewer `019ff5a0-cb7f-7fe3-aee0-f0c5a25920bc` 均无必须简化项；embedded C/C++ applicability=N/A。  
本轮 onefile 使用 `local-arch-123` 构建并覆盖根目录两个 EXE：canonical/root/root-latest 均为 `48,047,222` bytes，SHA-256 为
`803114A03FA0FBEBF440005A1C79C6970FAB85C0863911CA2CF7B87DDCCB8054`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、正式发行资格与硬件验收为
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  
归档：[`2026-08-12-arch-123-command-empty-state.md`](2026-08-12-arch-123-command-empty-state.md)。

日期：2026-08-12  
当前增量：ARCH-122 / UI-1.195 high-refresh motion + responsive overview metrics。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`MOTION_10S=pass`（1224 frames / 119.992Hz）；`MOTION_BURST_FENCE=pass`；`MOTION_LIFECYCLE=pass`；`METRIC_RESPONSIVE=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮 `MotionController` 继续是唯一共享时钟，4ms `PreciseTimer` 内部时间域统一为 `time.perf_counter()`，frame budget 出帧取模且每回调
最多一帧；扩展概览指标由 `_ResponsiveMetricGrid` 按 6/3/2/1 列响应式排布，长文本收缩换行并保留完整 accessibility description。
不改变 DTO、业务、主题、scroll owner、OTA/AES/RTT/J-Link 边界。  
架构师 `019ff579-3e62-7551-9cf5-4907a3638a9a` APPROVE；独立 reviewer `019ff57c-bd7a-79b3-838c-6471f5a8d87c` APPROVE、无
Critical/Required findings；简化 reviewer `019ff57c-bdd6-7f30-965b-cb05faefd0b1` 无必须简化项；embedded C/C++ applicability=N/A。  


日期：2026-08-12  
当前增量：ARCH-121 / UI-1.194 overview workspace height budget（承接 ARCH-120 / UI-1.193）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`TERMINAL_OVERVIEW_THEME_LIFECYCLE=pass`（312 checks，0 failures）；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮在 `presentation/controllers/workspace.py` 增加 overview height budget owner：按 root sibling 最小高度预算动态设置
workspace floor，980×720 约 168px；focus snapshot restore 后重同步，首显/resize 最多两轮 queued settle，避免连接页和实时观测栏
短暂挤压。实际宽屏 shell height 可自然超过 220px，但 `minimumHeight` floor 不超过 220px；不改变 bindings/signals、业务、
QScrollArea、MotionController、滚动或 OTA/AES/RTT/J-Link 边界。架构师 `019ff53b-61e5-7a10-8ad9-a1f153e5903a`、
`019ff549-d471-71b3-90aa-d8ba7f08933e`、`019ff54e-0953-7c21-a93f-3f77d09ce5e4`、`019ff550-f612-7fe1-b11e-bee17163d8e9`
均批准；独立 reviewer `019ff553-2d64-7330-8ca5-fa873de955f7` 等待超时关闭，未形成外部结论；父代理完成六轴 review 与
simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。  

本轮使用 `local-arch-121` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和 `SerialForge-latest.exe` 均为
`48,040,231` bytes，SHA-256 为 `0524910A444C68B5437E94A73481F590E2F4865072E33158C703D1AFB79B80EC`；archive listing
SHA-256 为 `5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、正式发行资格与
硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-120 / UI-1.193 terminal/send toolbar intrinsic sizing（承接 ARCH-119 / UI-1.192）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`TERMINAL_SEND_RESPONSIVE=pass`；`TERMINAL_SEND_THEME_LIFECYCLE=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮在 `terminal_toolbar_builder.py` 内增加无状态 `_ResponsiveObservationBand`，在 `send_bar_builder.py` 内
增加 sizing helper：动作控件保持 intrinsic width，输入/摘要吸收剩余宽度；不改变 bindings、signals、业务、
timer、动画、MotionController、滚动或 OTA/AES/RTT/J-Link 边界。三主题总览态多尺寸、往返 resize、低动效、
hide/show/close 共 36 checks、0 failures。架构师 `019ff51d-9728-75b2-9f96-c144beeec9b1` 批准，Terra
`019ff523-c27e-7bd2-8c2c-c46c4b99527e` 批准发送栏；独立 reviewer `019ff526-3e94-7761-bccd-96153086bfcb`
等待超时关闭，未形成外部结论；父代理完成六轴 review 与 simplification assessment。本轮无嵌入式 C/C++ 改动，
public-vendor-source applicability 为 N/A。  

本轮使用 `local-arch-120` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,038,525` bytes，SHA-256 为
`05A72BF2599736C65BA4B43CFB525F72FED4EB2C2CA4E47672BAC46B2E07165B`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、
正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-119 / UI-1.192 connection shell responsive owner（承接 ARCH-118 / UI-1.191）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`CONNECTION_THEME_RESPONSIVE=pass`；`RESPONSIVE_LIFECYCLE=pass`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮在 `presentation/controllers/connection_builder.py` 内增加无状态 `_ResponsiveConnectionBand`，依据
控件 sizing contract 在窄宽度切换四行 compact layout，宽屏恢复常规布局；保留既有控件 identity、
`ConnectionShellBindings`、signals/callbacks、focus/accessibility、业务状态、唯一 120Hz MotionController
和 OTA/AES/RTT/J-Link 边界。三主题×六尺寸×正反向 resize 共 72 checks、生命周期矩阵 21 checks 均 0 failures。
架构裁决 `019ff507-084e-7942-9583-f60b8fbda2ca` 为 `APPROVE`；局部伸缩权修正 `019ff50d-87f5-7110-98c9-75f7d35b03fc`
为 `APPROVE`。独立 reviewer `019ff50f-a11e-7042-b7e9-08f03167909e` 等待超时关闭，未形成外部结论；父代理完成
六轴 review 与 simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。  
本轮使用 `local-arch-119` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,036,144` bytes，SHA-256 为
`CBED1FB08241C4D938796CED3F2AECF84C1A8BBFEC793A9832D489AAFBE495DD`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、
正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-118 / UI-1.191 shell transition 互斥 owner（承接 ARCH-117 / UI-1.190）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`TRANSITION_MUTEX_MATRIX_PASS=pass`；`MOTION_SCHEDULER_SAMPLE=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮新增 `presentation/transition_coordinator.py` 作为窗口级一次性 shell transition 互斥 owner：theme、
workspace、focus、transport 任意时刻最多一个 active；有效启动前停止竞争过渡，无效目标、低动效、隐藏/
最小化、resize/hide/close 和重复 stop 统一全量清理。原有 owner 继续持有动画 effect 与 finish identity，
不新增 timer/MotionController、布局、业务、滚动或 OTA/AES/RTT/J-Link coupling。共享调度样本为 142 callbacks /
1.2s、约 118.33Hz；互斥矩阵 75 checks、0 failures。  
架构师 `019ff4e7-5a7d-7341-8dce-e14395627a4e` 要求 coordinator，`019ff4e9-d12d-7f50-af4f-35558bc91b89`
确认调用链，`019ff4f0-2690-7fe1-92fe-e247d76b774e` 批准早退全量清理；独立 reviewer `019ff4f2-615d-7843-8b37-82353dd6b266`
两次等待超时后关闭，未形成外部结论；父代理完成六轴 review 与 simplification assessment。本轮无嵌入式 C/C++
改动，public-vendor-source applicability 为 N/A。  

本轮使用 `local-arch-118` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,033,518` bytes，SHA-256 为
`83A3EFCB92D13D45C234C5D17D213B8AF22953DA2C8FA267F674EE250683B00F`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-117 / UI-1.190 扩展工具站能力分区节奏 owner（承接 ARCH-116 / UI-1.189）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`EXTENSION_SECTION_MATRIX_PASS=pass`；`EXTENSION_SECTION_LIFECYCLE_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮在 `embedded_extension_panel.py` 内增加 `_build_capability_section()`，用 `QFrame#extensionCapabilitySection`
统一承载每组标题、说明和既有 `ResponsiveCapabilityGrid`；上下 margin 10px、spacing 7px、水平 margin 0，
因此 980px OTA 三卡保持两列、1180/1240px 保持三列，同时把三组能力从连续长流分成明确分区。容器不聚焦、
不拥有 selection/detail/accessibility/Tab、滚动、动画或业务状态；唯一 120Hz MotionController 与 OTA/AES/
RTT/J-Link contract 边界不变。三主题×三尺寸、反向 resize 分区矩阵 318 checks、生命周期矩阵 38 checks
均 0 failures。  
架构师 `019ff4d6-6e24-74a2-941a-b406376c5763` 批准 section owner，`019ff4dd-c011-7891-bcd4-5924116a0263`
批准水平 margin 修正；独立 reviewer `019ff4df-b76b-7c81-8cbd-2ea3ae392e5d` 等待窗口超时并关闭，未形成
外部结论；父代理完成六轴 review 与 behavior-preserving simplification assessment。本轮无嵌入式 C/C++ 改动，
public-vendor-source applicability 为 N/A。  

本轮使用 `local-arch-117` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,031,424` bytes，SHA-256 为
`4488BDB5E31EEBF6C61D9BE4925C829E64A090E9C158396E6CA3B1A9F98400C0`；archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-116 / UI-1.189 扩展工具站响应式能力卡网格 owner（承接 ARCH-115 / UI-1.188）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`CAPABILITY_GRID_MATRIX_PASS=pass`；`EXTENSION_LIFECYCLE_MATRIX_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮新增 `ResponsiveCapabilityGrid` 作为扩展能力卡的唯一 presentation 排布 owner：按扣除 margins/
spacing 后的可用宽度在 1/2/3 列间切换，980px 为两列，1180/1240px 的 OTA 三卡为三列；只重排已有
card QWidget，不改变 selection/detail/accessibility/Tab 顺序、原生 QScrollArea、focus/overview、
唯一 120Hz MotionController 或 OTA/AES/RTT/J-Link contract 边界。三主题×三尺寸、`1→2→3→2→1→3`
反向 resize 的网格矩阵 627 checks、生命周期矩阵 662 checks 均 0 failures。  
架构师 `019ff4c3-7140-7ac1-b7ec-de6e736da7b1` 条件批准 helper 边界，`019ff4c8-5889-7ed3-b938-4c90949ab328`
批准 margins/spacing、正宽度校验和 1 列保底；独立 reviewer `019ff4cc-806a-7380-bb99-680d604241ca` 等待
窗口超时关闭，未形成外部结论；父代理完成六轴 review 与 behavior-preserving simplification assessment。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。  

本轮使用 `local-arch-116` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,030,866` bytes，SHA-256 为
`334406DFEA71D5E4B25FB67088DD3B907250EE8529827CE4AB644FB6FEF780DA`；archive listing SHA-256 为
`ED4576D41DB716016BD327B502599A4E3580510BD990902B3F71C997C1A53621`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-115 / UI-1.188 短页面垂直节奏 owner（承接 ARCH-114 / UI-1.187）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`RESPONSIVE_SCROLL_MATRIX_PASS=pass`；`RESPONSIVE_LIFECYCLE_MATRIX_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮新增 `ResponsiveScrollArea` 作为共享 `scroll_page()` 的薄 presentation wrapper：短页面按
内容高度垂直居中，长页面顶部对齐并继续使用原生 QScrollArea；水平填充、页面 layout stretch、
size policy、命令空态 Expanding owner、route 31px、focus snapshot 和唯一 120Hz MotionController
均保持。三主题×980/1180/1240×四 workspace 的响应式矩阵 217 checks、生命周期矩阵 752 checks，
全部 0 failures。  
架构师 `019ff4ad-186a-7fb2-8a28-83e47bf9fa20` 有条件批准 helper 边界，`019ff4b1-dc64-76c1-85ec-0c3858663ba7`
批准移除不存在的 viewport resized 信号；独立 reviewer `019ff4b4-c995-7fb0-9ad2-c0b403706716` 等待
窗口超时关闭，未形成外部结论；父代理完成六轴 review 与 behavior-preserving simplification assessment。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。

本轮使用 `local-arch-115` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,028,531` bytes，SHA-256 为
`8E98DB669C51C64813B701015A9DC2E2C9C7BAE4729025278BEECAF218713924`；archive listing SHA-256 为
`CD3C60D336971C88071C3A0D1C3B74D6AD090D1D2704F753C13B058E236913A0`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-114 / UI-1.187 主题工作区根布局伸缩 owner（承接 ARCH-113 / UI-1.186）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`FOCUS_MATRIX_PASS=pass`；`LIFECYCLE_MATRIX_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮将 focus 根布局 stretch 所有权明确交给 themed workspace shell（`shell=1, terminal=0`），
terminal slot 作为可恢复隐藏 slack；shell 内 tabs 为唯一可伸缩 owner，route strip 固定 31px，长页
继续使用原生 QScrollArea。overview 通过 `_FocusLayoutSnapshot` 恢复 root/shell/tabs/route stretch、
policy、min/max、terminal parent/child hidden 和鼠标透明；几何立即结算，opacity 只复用唯一共享
MotionController，避免动画帧间组件挤压和白色占位。  
本轮真实 Qt offscreen focus matrix 为三主题×980/1180/1240×四 workspace，共 36 cases、72 checks、
0 failures；lifecycle matrix 为 118 checks、0 failures。架构师 `019ff489-8b27-77a2-b059-bc160805c0fb`
给出 conditional pass 方案结论，`019ff497-1040-7721-8aa4-865f176e9e1d` 确认 bool 返回契约并已落地；
独立 reviewer `019ff499-0005-76b3-9e7c-e042c2777518` 等待窗口超时关闭，未形成外部结论；父代理完成
五轴 review 与 behavior-preserving simplification assessment。本轮无嵌入式 C/C++ 改动，public-vendor-source
applicability 为 N/A。共享 120Hz scheduler 抽样：142 callbacks / 1.202s，约 118.14Hz，平均
8.445ms、p95 14.662ms；这是调度器证据，不等同真实显示器 120fps。

本轮使用 `local-arch-114` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,026,927` bytes，SHA-256 为
`966C16A74D04454CE8780F424E19D99773E3E39B1CBE42A0A5490812CB605F2C`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-113 / UI-1.186 专注工作区自然高度与透明 slack（承接 ARCH-112 / UI-1.185）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH113_LAYOUT_PASS=pass`；`ARCH113_RESPONSIVE_PASS=pass`；
`ARCH113_VISIBILITY_PASS=pass`；`ARCH113_LIFECYCLE_PASS=pass`；`ARCH113_120HZ_OWNER_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮由 `workspace_focus_transition.py` 单一 presentation owner 结算当前页自然高度：focus 隐藏实时观测
与发送区，复用已有透明 `terminalSurface` 作为 layout slack；短页按 `minimumSizeHint()` 收敛，长页
保留原生滚动；terminal 子控件 hidden、父 surface、鼠标透明、size policy 和 min/max 均有快照恢复。
route 变化与 native resize 只调用同一公开 refresh，不改 root child 顺序、业务状态、滚动 owner、
`MotionController`、timer、QSS 或 OTA/AES/RTT/J-Link 边界。三主题×980/1180/1240×四 workspace
共 36 个真实 Qt offscreen cases 通过，横向 `HMAX=0`、focus/overview、before-show、重复切换、
resize、暂停/隐藏/恢复/关闭和唯一 MotionController 均通过。  
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。架构师 Hubble、Pasteur、Russell
的方案/约束结论已纳入；Kant 与独立 reviewer Pauli 均超时关闭，未形成外部结论；父代理完成
correctness、architecture、security、performance、readability 五轴 review 与 behavior-preserving
simplification assessment。  
本轮使用 `local-arch-113` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,024,709` bytes，SHA-256 为
`ED76F5E158A5B951B4A58B4EE1732277B3093C8A08FDD270EA19D57DB539703A`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

当前增量：ARCH-111a / UI-1.184 共享动效时钟 elapsed budget（承接 ARCH-110 / UI-1.183）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；
`source-limit=pass`；`theme-audit=pass`；`ARCH111A_MOTION_BUDGET_PASS=pass`；
`ARCH111A_LIFECYCLE_PASS=pass`；`ARCH111A_LAYOUT_PASS=pass`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮只修正 presentation 层唯一 `MotionController` 的 frame budget：由固定 `0.96` frame/tick
改为已有实测 monotonic elapsed 的 `elapsed × TARGET_HZ`，继续使用 8ms `PreciseTimer`，每次 tick
最多发出一帧，避免事件循环恢复后突发补帧。phase cap、共享 frame fan-out、暂停/低动效/隐藏/恢复/
关闭生命周期和所有业务/transport/OTA/AES/RTT/J-Link 边界不变。真实 Qt offscreen 样本为 87 frames、
均值 8.360ms、p95 16.000ms、有效约 119.61Hz；三主题×980/1240×四 workspace 横向 scroll 为 0，
focus/overview 可见性保持。本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。
架构师调用 `019ff438-6600-7a80-89ed-fc7694a209d6` 与独立 reviewer 调用
`019ff439-b809-7db3-b3ff-36157aa8dabf` 均超时关闭，未形成外部结论；父代理完成五轴 review 与
behavior-preserving simplification assessment。

本轮使用 `local-arch-111a` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,021,974` bytes，SHA-256 为
`56133D65BBE514AB18AF505D0446354CA948B96A6BBCEE39D16CCCBA2EC0084F`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-110 / UI-1.183 命令空态内容居中（承接 ARCH-109 / UI-1.182）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH110_EMPTY_CENTER_PASS=pass`；`ARCH110_RESPONSIVE_PASS=pass`；
`ARCH110_ACCESSIBILITY_PASS=pass`；`ARCH110_LIFECYCLE_PASS=pass`；`ARCH110_120HZ_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；
`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮修复命令管理空批量命令时内容贴左造成的无语义空白：`CommandBatchEmptyState` 保留唯一
vertical stretch owner，组件内部仅调整 horizontal stretch，使 glyph、说明和 CTA 自然宽度居中；
不改变 command controller、ViewModel、命令执行、scroll owner、CTA signal、accessibility、共享
MotionController 或 OTA/AES/RTT/J-Link 边界。三主题、980×720/1240×820 内容中心偏差 1px，滚动
maximum 0；隔离 CTA signal、低动效、暂停、隐藏/恢复、关闭与约 121.05Hz scheduler evidence 通过。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。架构师与独立 reviewer 调用
均超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。

本轮使用 `local-arch-110` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,019,942` bytes，SHA-256 为
`EFE27AA251E89A9D857CD0E6D8D11B914A9AB24C730F802601225DD3465145A4`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-109 / UI-1.182 专注/总览首帧可读淡入（承接 ARCH-108 / UI-1.181）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH109_REVEAL_FIRST_FRAME_PASS=pass`；`ARCH109_LAYOUT_PASS=pass`；
`ARCH109_LIFECYCLE_PASS=pass`；`ARCH109_120HZ_PASS=pass`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮修复 focus→overview 过渡中下方 surface 从完全透明开始造成的大块空白：布局仍先静态结算，
现有共享 opacity track 从 `_REVEAL_START_OPACITY=0.82` 淡入 `1.0`，不新增时钟、timer、布局
动画、业务状态、scroll owner 或 OTA/AES/RTT/J-Link 后端。真实 offscreen 中 980×720/1240×820
首帧可读、geometry 稳定；低动效、暂停、隐藏/恢复、关闭通过；共享时钟样本约 120.16Hz。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。架构师与独立 reviewer 调用
均超时关闭，未形成外部结论；父代理完成五轴 review 与行为保持简化评估。

本轮使用 `local-arch-109` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,019,597` bytes，SHA-256 为
`35DE8906B0838D29562F85E2518035C64DA5B69FAA65F9AFA7BCB8B065ED9DD7`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-108 / UI-1.181 扩展能力焦点滚动结算（承接 ARCH-107 / UI-1.180）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH108_FOCUS_SCROLL_PASS=pass`；`ARCH108_ACCESSIBILITY_PASS=pass`；
`ARCH108_RESPONSIVE_PASS=pass`；`ARCH107_DETAIL_ON_DEMAND_PASS=pass`；`package=pass`；`root-exe=pass`；`root-latest=pass`；
`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮在 ARCH-107 详情按需展开基础上，补齐能力卡键盘焦点可见性：详情改变 content 高度后同步
结算现有 QScrollArea，再进行原生 ensure 和有界 geometry correction。三主题、`980×720`、
`1240×820`、7 张卡逐一聚焦矩阵通过，全部 visible/focus，横向 scrollbar 为 0，scroll hint 中段
状态正确；应用层 catalog、OTA 三协议、AES 两种安全槽位、RTT/J-Link attach-only/contract-only
边界未改变。
本轮无嵌入式 C/C++ 改动，public-vendor-source applicability 为 N/A。架构师与独立 reviewer 调用
均在等待窗口内超时关闭，未形成外部结论、未伪造 PASS；父代理完成 correctness、architecture、
security、performance、readability 五轴 review 与 behavior-preserving simplification assessment。
本轮使用 `local-arch-108` 完成 onefile 并覆盖根目录两个 EXE：canonical、`SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,020,617` bytes，SHA-256 为
`2300FFD431657EA96E67A43F1470199671883CAC9AA5FC711F0CA1AB6330A217`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-104 / UI-1.177 命令空态工作区画布（承接 ARCH-103 / UI-1.176）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH104_LAYOUT_PASS=pass`；`ARCH104_THEME_PASS=pass`；
`ARCH104_LIFECYCLE_PASS=pass`；`package=pass`；`root-exe=pass`；`root-latest=pass`；
`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮将 `CommandBatchEmptyState` 设为命令页唯一 vertical stretch owner，组件内部只负责空态排版；
`controllers/commands.py` 继续拥有 batch/snapshot/visible 投影与执行语义。三主题×980/1240、四
workspace 真实 Qt offscreen 组合无 sibling overlap；focus command page `1240×820` empty state
为 `(14,151,1176,399)`，scroll hint 为 `内容已全部显示` / `complete`。  
动画继续由唯一 `MotionController`、`TARGET_HZ=120`、8ms `PreciseTimer` target 驱动；1ms/elapsed
实验在真实 `app.exec()` 中没有改善 cadence，已撤回。当前主循环样本 `frames=116`、均值 `8.536ms`、
有效约 `117.16Hz`，只代表 offscreen scheduler 证据，不等同真实显示器 FPS。  
架构师调用均在等待窗口内超时关闭，未形成外部结论；父代理完成 correctness、readability、architecture、
security、performance 五轴 review 与 behavior-preserving simplification assessment。嵌入式 C/C++
public-vendor-source applicability 为 N/A；本轮无固件/C/C++ 改动，不作 MISRA、ISO 26262、ASIL、
ASPICE 或认证声明。  
交付包为 `local-arch-104` onefile：canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`
均为 `48,018,981` bytes，SHA-256 为
`58421049705382E4CEB343B6B2414D7B329A0957966AE02D62966B59C6423E29`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-103 / UI-1.176 路线条 focus/overview 模式主题层级（承接 ARCH-102 / UI-1.175）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH103_ROUTE_MODE_PASS=pass`；`ARCH103_LAYOUT_PASS=pass`；
`ARCH103_ACCESSIBILITY_PASS=pass`；`ARCH103_MOTION_PASS=pass`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮复用 `workspaceShell[mode="focus|overview"]`，为路线条增加 focus 的 info→history 渐变和
accent-blue 上沿，并保留 overview 的 neutral 上沿；不改变固定高度、Tab/焦点顺序、滚动提示、
transport/session 状态或动效时钟。三主题 focus/overview 实跑通过；980/1240 四 workspace
horizontal scroll 均为 `0`；共享时钟 0.5 秒采样 `frames=60`、均值 `8.41ms`、有效约 `118.8Hz`，
不等同真实显示器 FPS。架构师 `019ff3ac-0f16-7633-8304-1cb3a1cecb54` 与独立 reviewer
`019ff3b1-8e10-7b63-94c3-af41db79e1a5` 等待超时关闭，未形成外部结论，未伪造 PASS；父代理完成
五轴 review 与行为保持简化评估。嵌入式 C/C++ public-source applicability 为 N/A。  
交付包为 `local-arch-103` onefile：canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`
均为 `48,016,499` bytes，SHA-256 为
`B51F8C0D0402CF692F41DC16CCAD70E183F2C764850DCAA08890CF7709292CEE`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-102 / UI-1.175 紧凑顶栏密度与 focus 布局先结算（承接 ARCH-101 / UI-1.174）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH102_LAYOUT_SETTLEMENT_PASS=pass`；`ARCH102_MOTION_PASS=pass`；
`ARCH102_SCROLL_PASS=pass`；`ARCH102_ACCESSIBILITY_PASS=pass`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮将紧凑 header 的 margins、cluster spacing、brand spacing 和语义圆角收敛到 `workspace.py` 的
header owner；focus/overview 切换先应用最终布局并激活根布局，再只对完整尺寸的 tabs 或 overview
surface 做 opacity 过渡，不再动画 `QTabWidget.maximumHeight`。980×720 中间帧 25/55/95/140ms 的
sibling geometry 均无重叠，focus 连接页 viewport 保持 `514px`；三主题×980/1240 四 workspace 的
horizontal scroll 均为 `0`。共享时钟 1 秒采样 `frames=119`、均值 `8.27ms`、p95 `10.02ms`、
最大 `17.08ms`、有效采样约 `120.9Hz`；最大值仅记录为 offscreen scheduler jitter，不等同真实
显示器 FPS。  
架构师 `019ff394-bb5d-7c60-9ce6-c60b65f1ac83`、布局复核 Terra `019ff39e-e6da-7ec3-8f96-7666b00d5574`
和独立 reviewer `019ff39b-8db0-7cc1-833e-009e2d0129cb` 均等待超时关闭，未形成外部结论，未伪造
PASS；父代理完成 correctness/architecture/security/performance/readability 五轴 review 与行为保持
简化评估。嵌入式 C/C++ public-source applicability 为 N/A。  
交付包为 `local-arch-102` onefile：canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`
均为 `48,019,127` bytes，SHA-256 为
`73593393356CC3938291E62D4DD5F66DBE09B3AA300C64AB6BA65BA55A6200E7`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。

日期：2026-08-12  
当前增量：ARCH-101 / UI-1.174 一次性过渡共享帧时钟（承接 ARCH-100 / UI-1.173）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH101_SHARED_CADENCE_PASS=pass`；`ARCH101_LIFECYCLE_CLEAN_PASS=pass`；
`ARCH101_QT_FINAL_PASS=pass`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮将 page fade、theme root+sweep、transport panel fade、dialog fade 和 focus transition 迁移到各自
`MotionDrivenAnimationGroup` finite owner；所有 owner 复用唯一 `MotionController.frame_changed`，
不启动独立 Qt animation clock。真实组合根连续活动窗口均值为 page `8.13–8.95ms`、theme `9.40–9.89ms`、
transport `8.21–9.05ms`、dialog `8.21–8.72ms`、focus `8.67ms`；三主题、四 workspace、六 transport、
dialog hide、theme overlay cleanup、focus/window hide 均通过。未新增 timer、thread、业务状态、scroll owner、
设备 I/O 或 OTA/debug coupling；这些 offscreen 样本不等同真实显示器 FPS。  
架构复核记录：ARCH-101 架构师 `019ff382-2d72-7d32-a067-09b13c9a32d5` 建议“每功能一个 finite owner、
全窗口一个共享时钟”；独立 reviewer `019ff389-959a-77b0-8f26-1e444ee50ac6` 超时关闭，未形成外部结论，未伪造 PASS；父代理完成
correctness/architecture/security/performance/readability 五轴 review 与行为保持简化评估。嵌入式
C/C++ public-source applicability 为 N/A。  
交付包为 `local-arch-101` onefile：canonical、根目录 `SerialForge.exe` 和 `SerialForge-latest.exe`
均为 `48,019,302` bytes，SHA-256 为
`CAABF434DE8A04B49249775B36C897F1AA73B71CAAAD3CE5A57F070DB1C8B558`；archive listing SHA-256 为
`EA7B1AB0312CFB143C71AE507AFEADC3603682E018C72F2E10BD6259363CAFD1`，provenance verify 通过。
签名、正式发行资格与硬件验收为 `NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-99 / UI-1.172 紧凑窗口连接页自适应专注（承接 ARCH-98 / UI-1.171）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH99_FOCUS_POLICY_PASS=pass`；`ARCH99_CTA_FOCUS_PASS=pass`；
`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮修复 980×720 总览下连接配置页被下方三个 surface 压到约 115px 的问题：紧凑窗口自动复用既有
focus/reveal，连接页 viewport 约 471px；用户显式“返回总览”后 resize 和 Tab 往返均保持 overview，
宽屏仍保留既有行为。策略不新增 timer、thread、业务状态、scroll owner 或设备 I/O；三主题六种
传输横向 scroll 最大值均为 0。共享 MotionController 仍是 120Hz target、8ms PreciseTimer。  
架构复核记录：Luna `019ff35f-fbde-7ce2-ac72-f7ef90d5c962`、Terra
`019ff361-cb49-7fa1-81b2-6f20c6b0337b` 与独立 reviewer `019ff366-a4db-7161-a137-04543c61423f`
均超时关闭，未形成外部结论，未伪造 PASS；父代理完成 correctness/architecture/security/performance/
readability 五轴 review 与行为保持简化评估。嵌入式 C/C++ public-source applicability 为 N/A。  
交付包为 `local-arch-99` onefile：canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,016,245` bytes，SHA-256 为
`3A44AC36190D0A275815D2CB0674B81C86C141F03B1E350EB105B33D9D4AC173`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过，签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-98 / UI-1.171 共享动作按钮高度契约（承接 ARCH-97 / UI-1.170）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH98_ACTION_HEIGHT_PASS=pass`；`MOTION_120HZ_SAMPLE=119Hz`；`package=pass`；
`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮把 `ActionRailButton` 的 36px 内容安全最小高度归口到共享 presentation leaf，并移除命令空态、
组件空态和终端空态的局部 `setFixedHeight(30)`；三主题四 workspace 的相关按钮均恢复为
`min=36 / max=16777215`，横向 scroll 最大值均为 `0`。共享 MotionController 仍是 `TARGET_HZ=120`、
`8ms` PreciseTimer；当前离屏环境 1 秒采样 `119Hz`，不将其表述为物理显示器精确 FPS。  
架构复核记录：架构师 `019ff352-98d1-79f2-a457-1dc781315fac` 与独立 reviewer
`019ff355-5852-7742-93e6-43193230a952` 均超时关闭，未形成外部结论，未伪造 PASS；父代理完成
correctness/architecture/security/performance/readability 五轴 review 与行为保持简化评估。嵌入式
C/C++ public-source applicability 为 N/A。  
交付包为 `local-arch-98` onefile：canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,015,845` bytes，SHA-256 为
`A4E63A9E17EC00DECB51ABB4B93EC3A2FABD2D8FB580843BFAD1FFB9C92006AB`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过，签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-97 / UI-1.170 扩展能力卡内容安全高度（承接 ARCH-96 / UI-1.169）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH97_CARD_PASS=pass`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮修复 `extensionCapabilityCard` 的 QSS 优先级回归：builder 已声明的 `122px` 内容安全下限被
通用按钮规则压成约 `83px`，现由专属 selector 明确保留。三主题 × 980/1240 离屏实际最小高度为
`144px`，横向滚动为 `0`；卡片选择、详情、焦点、Enter/空格、只读 extension DTO、OTA/AES/RTT/
J-Link contract-only/attach-only 和 120Hz/8ms 动效边界保持。  
架构复核记录：架构师 `019ff342-7abf-7e22-bc8e-40816e7aeef3` 与独立 reviewer
`019ff346-ded8-70b2-9843-fb9845b50952` 均超时关闭，未形成外部结论，未伪造 PASS；父代理完成
correctness/architecture/security/performance/readability 五轴 review 与行为保持简化评估。嵌入式
C/C++ public-source applicability 为 N/A。最终组合根与 package hash 已完成。  
交付包为 `local-arch-97` onefile：canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,015,901` bytes，SHA-256 为
`5599524451084AF460D267FDAF87049C83F94261CD89EAE940769F2983484BE6`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过，签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  

日期：2026-08-12  
当前增量：ARCH-96 / UI-1.169 有限整数选项化（承接 ARCH-95 / UI-1.168）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH96_COMPOSITION_PASS=pass`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮范围：协议最大帧长、UDP 最大报文、TCP Server 最大客户端数和批量命令步骤延时改为不可编辑
有限整数选项；端口仍保留自由输入。共享 selector 保持 `.value()`/`.setValue()`/`.setRange()`，
程序化旧值可精确恢复；协议动态上限重建期间抑制中间 signal；compact width 防止 980px 横向溢出。
未新增 timer、线程、业务状态、scroll owner、设备 I/O 或 OTA/debug coupling。  
架构复核记录：架构师 `019ff32a-7b39-7c50-a28b-ab39345c386c` 与独立 reviewer
`019ff332-291b-7273-a6fd-11faa93e7aa5` 均在等待窗口内超时关闭，未形成外部结论，未伪造 PASS；
父代理完成 correctness/architecture/security/performance/readability 五轴 review、行为保持简化评估，
并修正了 signal fan-out 与 numeric combo 宽度问题。嵌入式 C/C++ public-source applicability 为 N/A。  
验证口径：基础 selector、网络/协议/命令交互、协议固定 `280` disabled、range signal、980×680
逐页 visible scroll `horizontal_max=[0, 0, 0, 0]`、UDP 控件宽度 `148px` 均通过；组合根输出为
`ARCH96_COMPOSITION_PASS ... udp=65507 server_clients=16 protocol=12345 command_delay=1234
motion=120Hz/8ms`。当前环境的 PySide6 font directory warning 仅作环境记录。  
交付包为 `local-arch-96` onefile：canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe` 均为 `48,013,765` bytes，SHA-256 为
`192CED543D4860E8DE2659377A67B5CC3E539D37AE62066D242C4E61F9EC9DBC`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过，签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-96-integer-options.md`](2026-08-12-arch-96-integer-options.md)；
[`0147-bounded-integer-options.md`](../adr/0147-bounded-integer-options.md)。

日期：2026-08-12  
当前增量：ARCH-95 / UI-1.168 超时选项化（承接 ARCH-94 / UI-1.167）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH95_TIMEOUT_SELECTOR_PASS`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮范围：UART、TCP/UDP/RTT、BLE 共八个超时控件从可编辑 `QDoubleSpinBox` 改为不可编辑
`BoundedFloatCombo`；统一显示 `ms/s` 选项，运行时仍读取秒数，保留默认值、上下界、超界裁剪、
`0=未设置` 和既有 DTO/runtime contract。没有新增 timer、MotionController、线程、业务状态、
scroll owner、设备 I/O 或 OTA/debug 依赖。  
架构复核记录：架构师调用 `019ff31a-b844-7ca1-b77d-3d75984a90bc` 超时后关闭；独立 reviewer
`019ff320-947e-7850-bba9-ea631f12335a` 同样超时后关闭；两者均未形成外部结论，未伪造 PASS。
父代理完成 correctness/architecture/security/performance/readability 五轴 review、行为保持简化
评估与离屏验证。嵌入式 C/C++ public-source applicability 为 N/A。  
验证口径：八个控件 `editable=False`，option counts 为 `[10, 11, 12, 16, 16, 16, 16, 16]`，
默认秒数为 `[2.0, 1.0, 0.0, 3.0, 0.2, 3.0, 5.0, 30.0]`；程序化 `0.123` 秒值精确读取；
980×720 真实组合根无新增横向挤压；Ruff、compileall、`scripts/check.ps1` 通过。当前 PySide6
缺少 font directory 的 warning 已记录，不作为 UI 失败。  
交付物：`local-arch-95` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,011,368` bytes，SHA-256 为
`3CDC74EDDE5ED914E76436972FE4FB466A0AC754575F12D834A393C8C05D2E2E`；archive listing SHA-256 为
`39F8F8912A12D60FDF1FA521C17D6654D96B5D227D245BD628160C745E7540DA`，provenance verify 通过；签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-95-timeout-selector.md`](2026-08-12-arch-95-timeout-selector.md)；
[`0146-timeout-option-selector.md`](../adr/0146-timeout-option-selector.md)。

日期：2026-08-12  
当前增量：ARCH-94 / UI-1.167 工作区 reveal 防拥挤（承接 ARCH-93 / UI-1.166）。  
当前增量状态：`source=pass`；`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；
`theme-audit=pass`；`ARCH94_FOCUS_MOTION_PASS`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；
`hardware=not-run`；`release=ineligible`。  
本轮范围：退出专注设置时，为既有三块下方 presentation surface 增加一次性临时 opacity reveal，
避免高度让位期间内部控件挤在一起；完成、快速反转、显式 stop、隐藏和 close 均恢复/解绑 effect。
没有新增 timer、MotionController、业务状态、线程、scroll owner、设备 I/O 或 OTA/debug 依赖。  
架构复核记录：架构师调用 `019ff30a-ce6a-7a53-b7eb-cfd39d0c584d` 超时后关闭，未形成外部结论；
独立 reviewer `019ff311-7fa0-7c71-98cc-89a97522b908` 同样超时后关闭，未伪造外部 PASS。父代理完成
fresh correctness/architecture/security/performance/readability review 与行为保持简化评估。嵌入式
C/C++ public-source applicability 为 N/A。  
验证口径：Ruff、compileall、`scripts/check.ps1`（177 files ≤1000、3 themes/22 semantic tokens）、
真实组合根 980×720 focus reveal/settle/reverse/stop/hide/close 和唯一 MotionController `TARGET_HZ=120`
、8ms interval 通过；`frames=185` 仅为离屏等待窗口 callback 计数，不是显示器 FPS。  
交付物：`local-arch-94` onefile 已覆盖 canonical、根目录 `SerialForge.exe` 和
`SerialForge-latest.exe`；三者均为 `48,009,747` bytes，SHA-256 为
`BE74331C2BFAE11F0DDBD2DC5CC2D89D9581AFD08ACAA125CCE67C1516E283C3`；archive listing SHA-256 为
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过；签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-94-focus-reveal.md`](2026-08-12-arch-94-focus-reveal.md)；
[`0145-focus-reveal-opacity.md`](../adr/0145-focus-reveal-opacity.md)。

日期：2026-08-12  
当前增量：ARCH-93 / UI-1.166 推荐连接 preset 与 stale context 清理（承接 ARCH-92 / UI-1.165）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH93_PRESET_CONTRACT_PASS`；`ARCH93_ALL_PRESETS_PASS`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮范围：启动时通过既有 preset signal/apply path 预选 `UART · 115200 8N1`，仅填入安全 UART 值，
不自动连接、不保存 selection；用户手动切换到不匹配 transport 时清除 stale preset/context。7 个
builtin profile、accessibility、session gate、4 个 workspace visible scroll 和 980px geometry 通过。  
架构复核记录：ARCH-93 架构师与独立只读 reviewer 调用在服务窗口内超时并关闭，未伪造外部 PASS；父代理
完成 fresh correctness/architecture/security/performance/readability review 与简化评估，无 Required
finding。嵌入式 C/C++ public-source applicability 为 N/A，本轮只修改 Python/PySide6 presentation，
不作固件、MISRA、ISO 26262 或认证合规声明。  
验证口径：Ruff、compileall、`scripts/check.ps1`、source-limit/theme audit、真实组合根 preset/transport/
accessibility/scroll contract 均通过；未启动 GUI/EXE、未连接硬件、未运行真实显示器/HIDPI/FPS、
OTA/RTT 实连或签名验收。  
交付物：package 命令为 `./scripts/package.ps1 -Mode onefile -SourceRevision local-arch-93`；canonical、
根目录 `SerialForge.exe`、根目录 `SerialForge-latest.exe` 均为 `48,008,840` bytes，SHA-256
`14B679D1F92E03BCB1F189E3DD9012DF8D7C0ACE3C0934D75B5C285F0DC13E9A`，archive listing SHA-256
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过；签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-93-recommended-preset.md`](2026-08-12-arch-93-recommended-preset.md)；
[`0144-recommended-connection-preset.md`](../adr/0144-recommended-connection-preset.md)。

日期：2026-08-12  
当前增量：ARCH-92 / UI-1.165 响应式连接带与 120Hz 动效 fan-out（承接 ARCH-91 / UI-1.164）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH92_UI_PASS layout=pass cadence=120.83Hz lifecycle=pass`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；
`release=ineligible`。  
本轮范围：`scroll_page()` 采用 `AlignTop`；连接带/UART panel 使用水平 `Expanding`、垂直 `Fixed`，
连接带从此前约 254px 收敛到 138px；唯一 `MotionController` 使用 120Hz target、8ms PreciseTimer
和 0.96 nominal frame budget，1200ms offscreen sample 为 145 frames/120.83Hz；四个动态表面增加
局部 `motion_active()`，静态 animated fan-out 退出但 `animated=False` 全量 stop 广播保持。  
架构复核记录：ARCH-92 架构师与独立只读 review 调用在服务窗口内超时并关闭，未伪造外部 PASS；父代理
完成五轴 fresh review、行为保持与简化评估，无 Required finding。嵌入式 C/C++ public-source applicability
为 N/A，本轮只修改 Python/PySide6 presentation，不作固件、MISRA、ISO 26262 或认证合规声明。  
验证口径：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、source-limit/theme audit、
真实组合根 980/1240 可见 scroll 横向范围、连接带 geometry、状态切换、隐藏/显示 timer 与 cadence
contract 均通过；未启动 GUI/EXE，真实显示器/HIDPI/FPS、三主题逐像素、硬件/HIL、连接/OTA/debug
与签名验收未运行。字体目录 warning 仅为当前 PySide6 环境提示。  
交付物：package 命令为 `./scripts/package.ps1 -Mode onefile -SourceRevision local-arch-92`；canonical、
根目录 `SerialForge.exe`、根目录 `SerialForge-latest.exe` 均为 `48,008,038` bytes，SHA-256
`A2C75788258BB1AE19B23E881049C1AD6F902DD069C78F7E926F15CEF92FD6E8`，archive listing SHA-256
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`，provenance verify 通过；签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-92-responsive-motion.md`](2026-08-12-arch-92-responsive-motion.md)；
[`0143-responsive-motion-pacing.md`](../adr/0143-responsive-motion-pacing.md)。

日期：2026-08-12  
当前增量：ARCH-91 / UI-1.164 ViewModel worker owner 拆分（承接 ARCH-90 / UI-1.163）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH91_VM_CONTRACT_PASS signals=30 jobs=2`；`package=pass`；`root-exe=pass`；`root-latest=pass`；
`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮范围：新增 `presentation/viewmodel_jobs.py`，独占 `_DiscoverySignals/_DiscoveryJob` 与
`_BleScanSignals/_BleScanJob` 的阻塞 discovery 调用、queued result signals 和错误映射；
`SessionViewModel` 继续拥有全局 `QThreadPool`、两个 cancellation Event、busy/closing 状态、
回调与 30 个 Qt signal，`refresh_ports()`、`scan_ble()` 的入队与生命周期语义保持。源码行数为
`viewmodels.py=918`、`viewmodel_jobs.py=94`，均低于 1000 行。  
架构复核记录：ARCH-91-FMT 架构师调用与 ARCH-91 独立只读 review 调用在服务窗口内超时并关闭，未
伪造外部 PASS；父代理基于依赖方向、job 行为、signal/线程池/cancellation/lifecycle 契约完成
fresh-pass，无 Required finding。嵌入式 C/C++ public-source applicability 为 N/A，本轮只修改
Python/PySide6 presentation，不作固件、MISRA、ISO 26262 或认证合规声明。  
验证口径：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、30-signal/2-job
contract、source-limit、theme token audit、onefile provenance verify 和三 artifact hash equality
均通过；未启动 GUI/EXE，真实窗口、三主题截图、键盘焦点、显示器 FPS、硬件/HIL、连接/OTA/debug
与签名验收未运行。  
交付物：canonical、根目录 `SerialForge.exe` 与 `SerialForge-latest.exe` 均为 `48,007,307` bytes，
SHA-256 `ED765A8442C2DAB1FCD11AC36FC78E53032EDCD7CC17E90EBAB96915C932932E`；archive listing SHA-256
`032E6E39CDD8EE32A03CC66612F0699685358D5B8C5FB8D6BEAE7BC2C3A8D19E`；source revision `local-arch-91`；
签名 `NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-91-viewmodel-jobs.md`](2026-08-12-arch-91-viewmodel-jobs.md)；
[`0142-viewmodel-worker-owner.md`](../adr/0142-viewmodel-worker-owner.md)。  

日期：2026-08-12  
当前增量：ARCH-90 / UI-1.163 共享动效 catalog/可见性快照/frame fan-out owner 拆分（承接 ARCH-89 / UI-1.162）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH90_CONTRACT_PASS motion_reexports=3 target_hz=120 interval_ms=8`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮范围：新增 `controllers/lifecycle_motion.py`，负责共享 motion catalog、visible snapshot、activity
gate、phase-offset fan-out 与 live-RX activity projection；`lifecycle.py` 保留 shell 状态/主题/错误和
Qt 生命周期，并兼容导出既有 motion symbols。`MotionController` 唯一时钟、`TARGET_HZ=120`、8ms
`PreciseTimer` target、phase/stop/rearm/close 语义保持；未新增 timer、线程、backend、scroll owner
或状态源。  
架构复核记录：ARCH-90 Luna/max 架构师只读调用在服务窗口内超时并关闭；独立只读 review 调用同样
超时并关闭，未伪造外部 PASS；父代理基于 owner、依赖、兼容符号、行为保持、简化和 cadence 不变量
完成 fresh-pass，无 Required finding。嵌入式 C/C++ public-source applicability 为 N/A。  
验证口径：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、motion re-export/cadence
contract、source-limit、theme token audit、onefile provenance verify 和三 artifact hash equality 均通过；
未启动 GUI/EXE，真实窗口、三主题截图、键盘焦点、显示器 FPS、硬件/HIL 和签名验收未运行。  
交付物：canonical、根目录 `SerialForge.exe` 与 `SerialForge-latest.exe` 均为 `48,005,887` bytes，
SHA-256 `E3EAE6FCC3CEB46C498EBB3094B70551211C80168BD54D327B4FCE178CD1EF9E`；archive listing SHA-256
`0FC3A33559DC02B0C14313BE9A76FDE3C0FAA881A5C7861C9DFE146A0307775A`；source revision `local-arch-90`；
签名 `NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-90-motion-owner.md`](2026-08-12-arch-90-motion-owner.md)；
[`0141-shared-motion-owner.md`](../adr/0141-shared-motion-owner.md)。  

日期：2026-08-12  
当前增量：ARCH-89 / UI-1.162 协议/遥测页协议细节、Dataset、历史回放语义分行（承接 ARCH-88 / UI-1.161）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；
`ARCH89_CONTRACT_PASS fields=37 detail_layout=QVBoxLayout`；`package=pass`；`root-exe=pass`；
`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮范围：保留 `ProtocolPanelWidgets` 37 字段、全部 callbacks、业务 controller、状态投影和唯一
MotionController；协议细节从 7 列 `QGridLayout` 改为三字段语义行，Dataset 配置状态/动作、Replay
速度/动作分行，四个 protocol surface 的垂直节奏放宽。未新增 timer、线程、backend、scroll owner
或第二个 motion clock。  
架构复核记录：ARCH-89 Luna/max 架构师只读调用在服务窗口内超时并关闭；随后独立只读 review 调用
同样超时并关闭，未伪造外部 PASS；父代理基于 binding 完整性、依赖方向、布局行为保持、简化和动效
不变量完成 fresh-pass，无 Required finding。  
验证口径：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、37-field import contract、
source-limit、theme token audit、onefile provenance verify 均通过；未启动 GUI/EXE，真实 980×720/1240×820
几何、三主题截图、键盘焦点、显示器 FPS、硬件/HIL 和签名验收未运行。  
交付物：canonical、根目录 `SerialForge.exe` 与 `SerialForge-latest.exe` 均为 `48,004,714` bytes，
SHA-256 `1D08E351935E0CA828C5AB095CD0712042F3C5F1DE6F0BD291595D508A6D3A44`；archive listing SHA-256
`A152062F579FF9CCA9D2421EFDC6FA5D2FA6918A0691BE46EEE359AE451A47FF`；source revision `local-arch-89`；
签名 `NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`。  
证据归档与 ADR：[`2026-08-12-arch-89-protocol-density.md`](2026-08-12-arch-89-protocol-density.md)；
[`0140-protocol-density-boundary.md`](../adr/0140-protocol-density-boundary.md)。  

日期：2026-08-12  
当前增量：ARCH-88 / UI-1.161 命令管理页选择/动作分行与 command workspace owner 拆分（承接 ARCH-87 / UI-1.160）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；`ARCH88_IMPORT_PASS command_workspace_owner=command_workspace_builder binding_fields=9`；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮范围：`terminal.py` 收敛为错误通知 owner，`command_workspace_builder.py` 负责历史、批量选择、命令动作、状态/结果和空态 CTA；批量选择与五个动作拆为两行，外层 `commandPage` scroll 和 `CommandBatchControlBindings` 不变。  
架构复核记录：ARCH-88 Luna/max 架构师只读调用在服务窗口内超时并关闭，未产生独立 PASS；父代理基于 binding/runtime/accessor、Tab order、callback 唯一性和无循环依赖完成 fresh-pass，无 Required finding。  
验证口径：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、import contract 均通过；本轮未启动 GUI/EXE，真实 980×720/1240×820 几何、三主题截图、键盘焦点、显示器 FPS、硬件/HIL 和签名验收未运行。  
证据归档与 ADR：[`2026-08-12-arch-88-command-density.md`](2026-08-12-arch-88-command-density.md)；[`0139-command-workspace-owner.md`](../adr/0139-command-workspace-owner.md)。  
当前增量：ARCH-87 / UI-1.160 终端 live band 语义两行布局与 owner 拆分（承接 ARCH-86 / UI-1.159）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；`ARCH87_IMPORT_PASS terminal_toolbar_owner=terminal_toolbar_builder send_bar_owner=send_bar_builder`；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮范围：实时观测与实时发送从 `terminal.py` 的固定多列 QGridLayout 拆到两个 builder，采用输入/控制行与状态/动作行；保留 `TerminalControlBindings`、window staged refs、signals/callbacks、dynamic properties、focus/accessibility、shared MotionController 和 close lifecycle。错误、历史、批量命令仍由 `terminal.py` 组合。  
架构复核记录：ARCH-87 Luna/max 架构师只读调用在服务窗口内超时并关闭，未产生独立 PASS；独立 code-review/simplification Luna/max 调用同样超时并关闭；父代理按 terminal binding/runtime 依赖、信号唯一性、静态最小宽度预算和无循环依赖完成 fresh-pass，无 Required finding。  
验证口径：`uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、import contract 均通过；本轮未启动 GUI/EXE，真实 980×720/1240×820 几何、三主题截图、显示器 FPS、硬件/HIL 和签名验收未运行。  
证据归档与 ADR：[`2026-08-12-arch-87-terminal-density.md`](2026-08-12-arch-87-terminal-density.md)；[`0138-terminal-live-band-owner.md`](../adr/0138-terminal-live-band-owner.md)。  
当前增量：ARCH-86 / UI-1.159 动效疏朗与 network builder owner 拆分（承接 ARCH-85 / UI-1.158）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；`ARCH86_IMPORT_PASS target_hz=120 interval_ms=8 network_owner=network_builder`；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup=not-run`；`hardware=not-run`；`release=ineligible`。  
本轮范围：`SignalFieldWidget` 在不新增 timer/phase 的前提下降低装饰堆叠，将移动点从 5 个改为 3 个均匀轨道点；`connection_builder.py` 只保留 shell/UART/组合与 endpoint callback wiring，TCP/UDP/RTT 控件迁移到 `controllers/network_builder.py`，通用 label/combo/hint helper 迁移到 `connection_primitives.py`。保留 `MotionController` 的 `TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target、所有 `NetworkControlBindings` 字段、window 兼容引用、默认值和 runtime 信号。  
架构复核记录：ARCH-86 动效/布局架构师 Luna/max 后续返回 `PASS / Required=0`，确认唯一计时器、3 点等距轨道、27 个 `NetworkControlBindings` 字段、peer/endpoint 信号未重复或遗漏；风险仅为 8ms 名义约 125Hz、GUI/EXE 未运行。此前网络 owner 架构师调用超时并关闭；独立 code-review/simplification Luna/max 调用在服务窗口内超时并关闭，未伪造 PASS；父代理依据五轴 review 与行为保持简化清单完成 independent fresh-pass，无 Required finding。  
验证口径：已授权的 `uv run ruff check src scripts`、`compileall`、`scripts/check.ps1`、import contract 均通过；本轮未启动 GUI/EXE、未执行真实显示器 FPS、连接、硬件、HIL 或签名验收，避免将静态结果写成运行时证据。  
证据归档与 ADR：[`2026-08-12-arch-86-motion-network.md`](2026-08-12-arch-86-motion-network.md)；[`0137-motion-network-owner.md`](../adr/0137-motion-network-owner.md)。  
当前增量：ARCH-85 / UI-1.158 协议/组件/Dataset 空态表面收敛（承接 ARCH-84 / UI-1.157）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；`ARCH85_EMPTY_PASS`；`ARCH85_DATA_PASS`；`ARCH85_MOTION120_PASS frames=124/1s`；`package=pass`；`root-exe=pass`；`provenance=pass`；`hardware=not-run`；`release=ineligible`。  
本轮范围：`protocol.py` 将组件帧/Dataset 预览初始为 hidden，`derived_data.py` 依据已有帧/样本投影恢复可见；无帧时保留组件空态，无 Dataset 样本时保留状态、配置动作和曲线空态。没有修改 domain/application/infrastructure、typed binding、transport、业务 callback 或动画时钟。  
架构复核记录：本轮 Luna/max 架构师只读调用在服务窗口内超时，已关闭，未产生独立 PASS；父代理按“protocol builder 仅声明初始状态、derived_data 唯一运行时可见性 owner”的最小边界完成 architecture、correctness、readability/simplicity、security、performance fresh-pass，无 Required finding。  
120Hz 口径：既有 `TARGET_HZ=120`、`PreciseTimer`、8ms scheduler target 保持；强制开启共享动效的 1 秒离屏回调采样为 124 帧。该结果不是显示器真实 120Hz 承诺；真实显示器、HIDPI、硬件连接/传输、OTA/debug 执行与签名验收未运行。  
证据归档：[`2026-08-12-arch-85-derived-density.md`](2026-08-12-arch-85-derived-density.md)；ADR：[`0136-derived-empty-surface-visibility.md`](../adr/0136-derived-empty-surface-visibility.md)。  
交付物：canonical artifact、根目录 `SerialForge.exe` 与 `SerialForge-latest.exe` 均为 `47,997,051` bytes，SHA-256 `6794C51C907D5A2FFA32E0A1F9B841CB920A9F970D6B5C4CDAAB38405B6B51F5`；archive listing SHA-256 `9991DD041A86C1828261B9D8731E17A04BA20F90739193F11F6DE12423455D78`；source revision `local-arch-85`；签名 `NotSigned`；`release_eligible=false`；provenance verify、root EXE startup/shutdown 均通过。  
当前增量：ARCH-84 / UI-1.157 BLE GATT 配置密度与 builder owner 拆分（承接 ARCH-83 / UI-1.156）。  
当前增量状态：`static=pass`；`compile=pass`；`ruff=pass`；`source-limit=pass`；`theme-audit=pass`；`BLE157_CONTRACT_PASS`；`BLE157_LAYOUT_PASS sizes=2 overlap=0 horizontal_max=0 exact_white=0`；`BLE157_THEME_PASS themes=3`；`BLE157_REDUCED_PASS`；`BLE157_CLOSE_PASS`；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`ARCH84_EXE_STARTUP_PASS`；`ARCH84_EXE_SHUTDOWN_PASS`；`hardware=not-run`；`release=ineligible`。  
本轮范围：BLE GATT 7 列网格改为“扫描与筛选 / 设备发现与连接 / GATT 特征与收发”语义字段分组，新增 `controllers/ble_builder.py`；`connection_builder.py` 只接入既有 `BleControlBindings`。没有改变 BLE runtime、itemData、callback、默认值、手动扫描/显式连接/不自动重连边界，不新增 timer、backend、线程或硬件动作。  
架构复核记录：Luna/max 与 Terra/max 两次只读架构师服务窗口均超时，子代理已关闭，未伪造 PASS；父代理基于现有 typed binding/accessor 与 connection/ble owner 关系完成 architecture、correctness、readability/simplicity、security、performance fresh-pass，无 Required finding。  
120Hz 口径：本轮没有改变 ARCH-82 的唯一 `PreciseTimer`/8ms scheduler target；BLE 布局不新增动画时钟。真实显示器刷新率、HIDPI、读屏、硬件连接、BLE 扫描、OTA/debug 执行和正式签名验收均未运行，不能据此宣称实际面板 120Hz。  
交付物：canonical/root/root-latest 均为 `47,998,877` bytes，SHA-256 `07E5CF0BD1F9D0ADFD9C9C9CC23A61FFD70F8673DBA3257BE9F820A379E7D623`；archive listing SHA-256 `9991DD041A86C1828261B9D8731E17A04BA20F90739193F11F6DE12423455D78`；source revision `local-arch-84`；签名 `NotSigned`；`release_eligible=false`；provenance verifier、root EXE startup/shutdown 均通过。  
交付物：canonical/root/root-latest 均为 `47,994,096` bytes，SHA-256 `64A43DB59AA18A8B7F683DDFD99D2EE7531B223D4E60DEFFC5727D544547E1DD`；archive listing SHA-256 `C5FCC1126F66741D61881491EA6A44056C8842DD5BD997DE81CC8B484615B3C8`；source revision `local-arch-83`；签名 `NotSigned`；`release_eligible=false`；provenance verifier、root EXE startup/shutdown 均通过。  
范围：UI-1.102 扩展工具站能力卡与接入概览主题表面、UI-1.101 SignalField 二次元星芒与共享动效、UI-1.100 Pipeline 状态语义化节点与共享动效、UI-1.99 协议/遥测页选项与动态文案、UI-1.98 连接页选项 affordance、UI-1.97 UART 选项用户化、ARCH-7d / UI-1.96 扩展站摘要边界、UI-1.95 扩展工具站接入概览、UI-1.94 UART 参数摘要 rail、UI-1.93 extension capability accessibility semantics、UI-1.76 快速配置选择确认 rail、UI-1.75 自定义 preset metadata surface、UI-1.74 自定义 dialog 入口过渡、UI-1.73 终端显示字段层级、UI-1.72 批量命令编辑器字段层级、UI-1.71 协议/遥测字段标签层级、UI-1.70 主题选择器 ThemeSpec 色盘 icon、UI-1.57 主题切换一次性 root fade + ThemeTransitionSurface 语义色 sweep、UI-1.56 连接快速配置上下文摘要、UI-1.55 send state surface、UI-1.54 data activity surface、UI-1.53 batch/replay operation surfaces、UI-1.52 connection action surface、UI-1.51 recording action surface、UI-1.50 error surface、UI-1.49 send input surface、UI-1.48 QStatusBar session/RX 状态 footer signal rail、UI-1.47 UART 刷新/BLE 扫描共享 MotionController 忙碌信号轨、UI-1.46 文件选择器主题桥、UI-1.45 ComboBox popup 顶层 QFrame 白带修复、UI-1.43 连接快速配置自定义选中态与语义上下文、UI-1.42 批量编辑器原生控件主题回退、UI-1.41 批量命令空态 CTA/结果表互斥可见性、UI-1.40 终端空态 CTA 共享 frame signal rail、UI-1.39 终端空态连接导航 CTA、UI-1.38 三节点工作区路线 beacon、UI-1.37 共享环境动效、企业级模块化架构迁移、ARCH-6a/6b/6c/6d/6e/6f/6g/6h/6i/6j/6k/6l/6m owner-module contract、ARCH-7b/7c preset catalog 边界、MainWindow/Controller 拆分、主题语义 token/runtime 渲染、UI-1.36 真实数据终端观测标尺、UI-1.35 主题切换过渡、ARCH-7a 偏好端口、BLE 初始化委托修复、协议/连接响应式布局、内置与自定义连接快速配置、UI-1.33 连接状态路径、UI-1.34 终端空态观测画布、批量命令状态表面、表格白色回退修复、工作区 TabBar 白线修复、UI-1.26 终端空态与两行工具栏、UI-1.27 协议/遥测派生 surface 分区、UI-1.28 协议流水线主题化轨道、UI-1.29 顶栏状态指示器几何动效、UI-1.30 批量步骤/回放活动轨道、UI-1.31 四类分析状态轨道、UI-1.32 主题语义色点阵、源码行数门禁、交接目录治理，以及既有 UI-1.10～UI-1.25 / OTA-debug contract-only 基线  
状态：`static=pass`；`compile=pass`；`ruff=pass`；`UI102_EXTENSION_SURFACE_VECTOR_PASS`；`UI101_SIGNAL_FIELD_VECTOR_PASS`；`UI100_PIPELINE_STATE_VECTOR_PASS`；`UI199_PROTOCOL_OPTIONS_VECTOR_PASS`；`UI198_CONNECTION_OPTIONS_VECTOR_PASS`；`UI197_UART_OPTIONS_VECTOR_PASS`；`ARCH7D_UI196_SUMMARY_BOUNDARY_VECTOR_PASS`；`UI195_STATION_OVERVIEW_VECTOR_PASS`；`UI194_UART_SUMMARY_VECTOR_PASS`；`ARCH71_CTA_PASS`；`ARCH71_MATRIX_PASS=6`；`FOCUS_ENTER_NO_SQUEEZE_PASS`；`FOCUS_RETURN_SETTLED_PASS`；`FOCUS_RAPID_REVERSAL_PASS`；`ARCH71_REDUCED_MOTION_PASS`；`ARCH71_MOTION_INVARIANT_PASS=120/8-9/56`；`package=pass`；`root-exe=pass`；`root-latest=pass`；`provenance=pass`；`GUI/EXE-startup/hardware=not-run`；`release=ineligible`  
补充状态：UI110_ACTION_TOOLTIP_VECTOR_PASS；UI111_SEND_UART_AFFORDANCE_VECTOR_PASS；UI112_DIALOG_ACTION_AFFORDANCE_VECTOR_PASS；UI113_DERIVED_ACTION_AFFORDANCE_VECTOR_PASS；UI114_DERIVED_ACTION_RAIL_VECTOR_PASS；UI115_WORKSPACE_TAB_RAIL_VECTOR_PASS；UI116_ACTION_RAIL_VECTOR_PASS；UI117_FONT_RUNTIME_PASS；UI117_AUDIT_RENDER_PASS；UI117_RESPONSIVE_VECTOR_PASS；UI118_SCROLL_SIGNAL_VECTOR_PASS；三主题渲染与 provenance 通过；root-exe 等待用户关闭 PID 46108、49236。
补充状态：UI110_ACTION_TOOLTIP_VECTOR_PASS；UI111_SEND_UART_AFFORDANCE_VECTOR_PASS；UI112_DIALOG_ACTION_AFFORDANCE_VECTOR_PASS；UI113_DERIVED_ACTION_AFFORDANCE_VECTOR_PASS；UI114_DERIVED_ACTION_RAIL_VECTOR_PASS；UI115_WORKSPACE_TAB_RAIL_VECTOR_PASS；UI116_ACTION_RAIL_VECTOR_PASS；UI117_FONT_RUNTIME_PASS；UI117_AUDIT_RENDER_PASS；UI117_RESPONSIVE_VECTOR_PASS；UI118_SCROLL_SIGNAL_VECTOR_PASS；UI119_PERF_EXPERIMENTS_REVERTED；UI120_EXTENSION_CARD_VECTOR_PASS；三主题渲染与 provenance 通过；root-exe 等待用户关闭 PID 46108、49236。
本轮最新范围：协议/组件/Dataset 的 Profile/Codec 加载、组件 CSV 导出、Dataset 配置加载和 Dataset CSV 导出，启用态与禁用态均使用动作专属提示。  
本轮补充范围：回放/连接/录制动作、UART 字段、发送区、批量编辑器、preset/确认对话框和快捷命令 QAction 均由原 presentation owner 提供细粒度 tooltip 与 accessible description。  
父代理：Codex；父代理是本轮唯一写入者  
共享 checkout：`D:\Workplace\Agent_Workplace\SerialForge`；未检测到 `.git`，未创建/操作 worktree

## 2026-08-12 ARCH-81 / UI-1.154 UART 参数字段密度

### 实现

`src/serialforge/presentation/controllers/connection_builder.py` 将 UART 参数区从八列
`QGridLayout` 重排为三行 `QVBoxLayout`：端口/波特率、数据格式、超时/线路控制。端口 combo 与
刷新按钮封装为 `port_controls`，独占/DTR/RTS 封装为 `line_controls`，两者都是局部
presentation ownership；字段标签与均匀伸缩复用 `form_fields.py` 的 `build_labeled_field()` /
`build_field_row()`。没有修改 domain/application/infrastructure、网络/BLE panel、MotionController
或任何硬件路径。

### 不变量

`UartControlBindings` 字段、combo `itemData`/枚举、默认值、timeout range、summary callback、
连接 gate、Tab 顺序和 accessibility 文案保持原样。`UartTimingSummarySurface` 仍由已有 combo
投影更新；用户仍可选择常用波特率，端口仍支持手动输入 COMx。

### 验证证据

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
ARCH81_FIELD_GEOMETRY_PASS fields=10 overlap=0
ARCH81_TAB_ORDER_PASS targets=13
ARCH81_TYPED_VALUE_PASS 115200/8N1/none -> 230400/7O2/RTS/CTS -> restore
ARCH81_LAYOUT 980x720  horizontal_max=0
ARCH81_LAYOUT 1240x820 horizontal_max=0
ARCH81_THEME_LAYOUT themes=3 horizontal_max=0
ARCH81_CLOSE_PASS
```

稳定截图暂存于 `C:\Users\Gs\AppData\Local\Temp\serialforge_arch81_uart_980.png` 与
`serialforge_arch81_uart_1240.png`，已完成视觉检查；Qt 离屏环境仍有既有 font-directory warning，
不代表 Windows 系统字体缺失。真实显示器 FPS、HIDPI、读屏、硬件连接/传输、刷写、部署与 HIL
未运行；本轮没有操作目标硬件。

### 审查与 assurance

架构师为 Luna/max，只读审阅后批准“仅 `connection_builder.py` + 复用 `form_fields.py` helper”
方案。独立只读审查线程首轮因为当前 checkout 没有 `.git`，无法使用 `git diff`，报告的是证据
获取限制而非已确认缺陷；父代理随后补充了当前源码行段、form helper、运行向量和截图证据，最终
返回 `PASS / Required=0`，Optional 仅保留环境限制和未运行单测/硬件。当前为 Python/PySide6 presentation-only，没有 embedded C/C++、
MCU、SDK、RTOS、driver、OTA implementation 或 target hardware change；public first-party vendor
source applicability 为 N/A，没有声称 MISRA/ISO 26262/ASIL/ASPICE/认证合规。embedded simplification
assessment 为 N/A；本轮只做 presentation 代码的五轴 review 与无行为改变的简化评估。Ruff、
compileall、静态门禁、真实组合根、视觉截图、EXE 启动/关闭和 close 均为已授权非破坏性验证。

### 包交付

`local-arch-81` onefile 已覆盖 canonical、根目录和 root-latest，三者字节一致：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 47,985,982 bytes
sha256: 81C8993A8D7A8723B074FD0BD37BC7D23822B9BB84070AE12C6E08F26D07B6DA
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; source revision local-arch-81
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
root startup/shutdown: pass; exact root PID 45164 safely terminated after 3 seconds
```

## 2026-08-12 ARCH-80 / UI-1.153 顶部 header 响应式密度

### 实现

`src/serialforge/presentation/controllers/workspace.py` 新增 `_AdaptiveHeader`，由 header 自身
`resizeEvent()` 调用 `_sync_header_density()`。当 header logical width 小于 1120 时，隐藏
`SignalFieldWidget` 与 `ThemePaletteSwatch`；两者原本均为 NoFocus、鼠标透明、空
accessibility 的纯装饰 surface。controls row spacing 从 8px 收到 6px。statusCluster、
motionControls、themeControls、主题下拉、低动效/暂停动效、连接状态、Tab 顺序和
`HeaderChromeBindings` 均保留；宽屏恢复两枚装饰件。

### 验证证据

```text
uv run ruff check src                         pass
uv run python -m compileall -q src            pass
scripts/check.ps1                             pass
ARCH80_HEADER_DENSITY 980/1119/1120 compact; 1240 regular
ARCH80_THEME_CONTROL_PASS themes=3
ARCH80_LAYOUT_PASS sizes=2 tabs=4 visible_horizontal_max=0
ARCH80_SETTLED_SCREENSHOTS_PASS count=3
ARCH80_MOTION_BOUNDARY target_hz=120 interval_ms=8 timer_type=PreciseTimer frames=29 mean_ms=15.607
ARCH80_PAUSE_PASS timer_active=0
ARCH80_HIDE_SHOW_PASS resume_active=1
ARCH80_ACTIVITY_SURFACES total=28 visible=4
ARCH80_CLOSE_PASS timer_active=0
```

离屏 Qt 仍有既有 font-directory warning；它来自当前 `.venv` 的字体目录。实际显示器 FPS、
HIDPI、读屏、真实硬件连接/传输、闪写、部署和 HIL 均未运行；本轮没有操作目标硬件。

### 包交付

`local-arch-80` onefile 已覆盖 canonical、根目录和 root-latest，三者字节一致：

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
size: 47,987,629 bytes
sha256: CB829EBBF27D9C48E1993040E28E07A57A9092EE3EBDC2A1BF32082A9D30F598
archive listing sha256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; signature=NotSigned; release_eligible=false; hardware_acceptance=not_run
root startup/shutdown: pass; verified PID 6044 and then safely terminated
```

### Assurance gate

本轮是 Python/PySide6 presentation-only 变更，没有 embedded C/C++、MCU、SDK、RTOS、driver、
OTA implementation 或 target hardware 变更；public first-party vendor source applicability 为 N/A，
没有声称 MISRA/ISO 26262/ASIL/ASPICE/认证合规。Luna 架构线程先超时后返回 workspace-local
结论，Terra 升级线程返回 helper 备选结论；父代理完成独立五轴 review，差异已记录。没有嵌入式
代码可简化；本轮只保留 header-local 几何 owner。Ruff、compileall、静态门禁、真实组合根、
动效生命周期、provenance 和 root EXE 启停均为授权的非破坏性验证；硬件、HIDPI 实机、读屏、
连接传输、刷写、部署和 HIL 未运行。

## 2026-08-12 ARCH-79 / UI-1.152 命令空态密度与调度边界

命令管理页原本在空批量命令状态下将 `CommandBatchEmptyState` 拉伸到约 451px，内容被稀释在大
面积空白中。`controllers/terminal.py:build_send_panel()` 在空态后增加一个 trailing stretch，
把剩余高度交给 spacer；空态按自身 size hint 呈现为 108px，保留 `new_requested`、CTA、
batch state、accessibility、hidden/visible contract 和 shared motion。

```text
ARCH79_THEME_LAYOUT_PASS themes=3 sizes=2 tabs=4 visible_horizontal_overflow=0 command_empty_height=108
ARCH79_ACTIVITY_PASS total=28 visible=4 single_motion_timer=1
ARCH79_LIFECYCLE_PASS pause=1 hide_show=1 close=1
ARCH79_MOTION_BOUNDARY_PASS target_hz=120 interval_ms=8 timer_type=PreciseTimer frames=46 observed_mean_ms=15.748 observed_min_ms=13.843 observed_max_ms=18.676
ARCH79_EXE_STARTUP_PASS
ARCH79_EXE_SHUTDOWN_PASS
scripts/check.ps1: pass
```

架构师批准 trailing stretch 为最小修复，并明确调度器不改：8ms 是 scheduler target，Windows/offscreen
约 15.6ms 是当前运行环境的观测边界。当前为 Python/PySide6 presentation-only，没有 embedded
C/C++、硬件写入、刷写、部署或 target 操作，未宣称任何认证合规。

### ARCH-79 包交付

```text
PACKAGE_ARCH79_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,984,076 bytes
SHA256: 085B1BEF964C3E2861183950E28689DAC798E853CA8A2C38E26A279144C324D9
archive listing SHA-256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; source revision local-arch-79
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

根目录交付文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。

## 2026-08-12 ARCH-78 / UI-1.151 动效层级与 activity fan-out

`MotionController` 仍是唯一 presentation clock、唯一 PreciseTimer 和唯一 frame signal；新增的
`activity_active()` 只读投影非 ambient 活动窗口。`ActionRailButton`、`BusyActionButton`、
`TerminalEmptyState`、`CommandBatchEmptyState` 和 `ComponentEmptyStateSurface` 通过
`MOTION_MODE="activity"` 声明短时动效，`AnalysisStatusLabel` 保持持续语义状态动效。

`controllers/lifecycle.py` 是唯一 fan-out/cleanup owner：activity 结束按 true→false 下降沿停止
activity surface，隐藏页在活动期间只停止一次，重新可见后恢复；ambient surface 继续消费共享帧。
全局 hidden/minimized/paused/reduced-motion/close 仍走全量 stop，不改变 callbacks、session、
protocol、typed itemData、焦点/accessibility 或 OTA/debug contract-only/attach-only 边界。

```text
MOTION_ACTIVITY_SURFACES total=28 visible=4
MOTION_CLOCK_TIMERS 1
MOTION_ACTIVITY_ON visible_animated=4
MOTION_ACTIVITY_OFF all_activity_static=1
MOTION_HIDDEN_ACTIVITY_PASS hidden=24
MOTION_ROUTE_ACTIVITY_PASS visible=7 animated=6
ARCH78_LAYOUT_PASS sizes=2 horizontal_overflow=0
MOTION_PAUSE_RESUME_PASS
MOTION_HIDE_SHOW_PASS
MOTION_CLOSE_PASS
scripts/check.ps1: pass
```

架构师批准 activity-only owner 边界；post-implementation 只读审查结论 `PASS / Required=0`，确认
activity gate、隐藏一次性 stop、single motion timer 与 pause/hide/minimize/close fence；Optional=2
（首帧未知→静态的一次冗余 stop、single timer 术语限定）不阻断。两轮外部 Luna/max 独立审查线程因
服务侧超时未返回，未伪造独立通过；父代理五轴 review/简化评估记录为 fallback。当前为 Python/PySide6 presentation-only：public MCU vendor source applicability=N/A；
没有 embedded C/C++、BSP/HAL/RTOS、硬件写入、刷写、部署或 target 操作，未宣称任何认证合规。

### ARCH-78 包交付

```text
PACKAGE_ARCH78_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,986,125 bytes
SHA256: BA907F2587004E2FE797B0AC6397B286AF034929DD05C888964C27BC3B4EC8B1
archive listing SHA-256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; source revision local-arch-78
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

根目录交付文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。

## 2026-08-12 ARCH-77 / UI-1.150 Motion cadence 与页面密度

`MotionController` 仍是唯一 presentation clock、唯一 PreciseTimer 和唯一 frame signal。原 single-shot
重臂改为 8ms 周期 scheduler slot，phase 继续按照 monotonic elapsed 推进；8ms 是约 120Hz 的调度目标，
不等同于显示器或 compositor 的精确 120fps。低动效、暂停、隐藏、最小化、关闭和 queued timeout 继续由既有
lifecycle fence 静态收敛，未新增 renderer timer 或 catch-up burst。

协议页只在 presentation builder 内重排：协议主字段/动作与状态、分隔符细节、组件/Dataset/曲线/回放工具条
使用更宽松的 field/action rows；`form_fields.py` 只负责无状态标签控件组合。`ProtocolPanelWidgets`、回调、
typed `itemData()`、显式 Tab 顺序、accessibility、外层 `settingsScroll` 和业务/application contract 不变。

```text
ARCH77_LAYOUT_PASS sizes=2 themes=3 horizontal_overflow=0 controls=9
ARCH77_MOTION_PASS frames=156 mean_ms=7.996 min_ms=4.668 max_ms=11.670
ARCH77_LIFECYCLE_PASS hidden_show=1 focus=True
scripts/check.ps1: pass
source line limit: pass (167 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

架构师只读结论为 conditional approve；独立审查在文案修正前 Required=1，修正后最终无阻断，确认
MotionController、协议布局、滚动 fallback、callback/itemData、生命周期和文件行数边界。绝对 deadline 试验
出现离屏 17–18ms 跳帧，因此采用更简单的单一周期 timer 方案作为行为保持型简化。

本轮为 Python/PySide6 presentation-only：public MCU vendor source applicability=N/A；没有 embedded C/C++、
BSP/HAL/RTOS、硬件写入、刷写、部署或 target 操作。未宣称 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。

### ARCH-77 包交付

```text
PACKAGE_ARCH77_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,984,250 bytes
SHA256: 11E505D955E750AD5062D5A1B552E9D92AE2DF9A8D3751DEAE0612040E9140D9
archive listing SHA-256: A3E1A7EF48915CD483DC56C5C27D616A894A9F4F45ABDBE82E91382953F24073
provenance: pass; source revision local-arch-77
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

根目录交付文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。

## 2026-08-12 ARCH-76 / UI-1.149 Connection onboarding focus

980×720 的真实字体 geometry 显示总览 `workspaceShell=150px`、`tabs=115px`、connection page viewport 约
`79px`，而连接控制带本身约 `149px`；总览模式必然裁切首次连接表单。ARCH-76 不增加 shell 高度，改为在
terminal bindings 完成后、初始 `SessionState.CLOSED` 时启动一次 presentation-only `_connection_onboarding`，
复用既有 focus geometry 隐藏 live/terminal/send，完整展示连接配置。

`workspace_runtime.py` 是 onboarding owner：connection Tab + onboarding + CLOSED 时保持 focus；首次 `OPEN`
只清除标志，且只有当前仍在 connection Tab 才调用既有 `set_workspace_focus_mode(False)`。用户手动点击“返回总览”
先清除标志；协议/命令/扩展 Tab 的自适应 focus 不被首次 OPEN 强制收起。ERROR/失败不清除标志，允许用户在当前
连接页继续重试；历史回放不触发该入口。

```text
ARCH76_ONBOARDING_PASS tabs=507 page=471 connection_band_height=199
ARCH76_MANUAL_OVERVIEW_PASS
ARCH76_OPEN_RETURN_PASS
ARCH76_OPEN_OTHER_TAB_PASS
ARCH76_THEME_ROUTE_PASS size=980x720 themes=3 tabs=4
ARCH76_THEME_ROUTE_PASS size=1240x820 themes=3 tabs=4
ARCH76_THEME_ROUTE_LIFECYCLE_PASS
ARCH76_LIFECYCLE_PASS
ARCH76_SCREENSHOT_PASS onboarding=1 overview=1
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

架构师对 owner/时序边界 conditional approve；独立 Luna/max 只读审查 Required=0、Optional=2、FYI=5，未修改文件。
Optional 记录为重复 start 的幂等 guard和 ERROR 切 Tab 的 UX 约定；现有生产调用链只有一次 bootstrap start，失败重试路径
保持 focus，均不构成阻塞。相关文件均低于 1000 行：bootstrap 501、workspace 343、workspace_runtime 235、
terminal_runtime 571。

本轮为 Python/PySide6 presentation-only：public MCU vendor source applicability=N/A；没有 embedded C/C++、BSP/HAL/RTOS、
硬件写入、刷写、部署或 target 操作。未宣称 MISRA、ISO 26262、ASIL、ASPICE 或任何认证合规。

### ARCH-76 包交付

```text
PACKAGE_ARCH76_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,982,523 bytes
SHA256: 6607C1BFD68E8B9F5148B19D827D1B000C3C9528DE616D63A7ED5F6E9E3A9AC9
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-76
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

根目录交付文件：[`SerialForge.exe`](../../SerialForge.exe)、[`SerialForge-latest.exe`](../../SerialForge-latest.exe)。

## 2026-08-12 ARCH-74 / UI-1.147 Explicit chrome sweep host

root fade 与 header sweep host 已解耦：`ThemeTransitionSurface` 只挂在 bootstrap 组装的 header chrome，
`target`/`sweep_host` 通过 keyword-only API 显式传递并校验 ancestor；窗口 resize 由 lifecycle 先 stop
主题过渡再交给 Qt 重排，避免 stale geometry。没有新增 timer、业务状态、MotionController、事件总线或外部依赖。

```text
ARCH74_HOST_GEOMETRY_PASS 980x720 host=948x115 overlay=180x115 terminal_top=413 send_top=587
ARCH74_HOST_GEOMETRY_PASS 1240x820 host=1208x115 overlay=180x115 terminal_top=483 send_top=687
ARCH74_RESIZE_CONSTRUCTOR_PASS
ARCH74_RESIZE_ABORT_PASS 980x720 -> 1240x820
ARCH74_RESIZE_ABORT_PASS 1240x820 -> 980x720
ARCH74_RESIZE_CLEANUP_PASS
ARCH74_THEME_MATRIX_PASS themes=3
ARCH74_MOTION_GATE_PASS reduced=True paused=True
ARCH74_THEME_LIFECYCLE_PASS hidden=True close=True
ARCH74_MOTION_PASS 120 PreciseTimer [8,9ms cadence]
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

架构师对 target/host 与 resize stop 给出 conditional approve；独立最终审查返回 Required=0、Optional=0，
确认薄 façade、ancestor guard、stop/finish identity guard、queued cleanup、单一 120Hz MotionController。
本轮为 Python/Qt presentation-only，public MCU vendor source applicability=N/A；无 embedded C/C++、硬件写入、
刷写或部署。真实 GUI、HIDPI、读屏、设备、OTA 安全后端、J-Link/RTT 和硬件验收未执行。

### ARCH-74 包交付

```text
PACKAGE_ARCH74_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,981,373 bytes
SHA256: 42C2451F5315CB8273CEEA45985ED6E595CF7E495B8E26E8434B41B2B4B45E34
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-74
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-12 ARCH-73 / UI-1.146 Bounded theme sweep

实测主题切换的旧 sweep 在 1240×820 窗口中为 `180×797`，会穿过 header、workspace、terminal 和
send。`theme_transition.py` 现在只把 `ThemeTransitionSurface` 的高度限制为
`min(220px, root.height())`；root opacity fade、180ms 一次性 animation、stop/finish 清理和
reduced-motion/暂停/隐藏/最小化/关闭生命周期保持。视觉复核显示 sweep 仍能表达主题变化，但
terminal/send 区不再被斜线 veil 覆盖。

```text
THEME_TRANSITION_BASELINE overlay=180x797
ARCH73_SWEEP_BOUNDS_PASS overlay=(-180,0,180,220) root_effect=0.86 sweep=Running
ARCH73_SWEEP_SETTLED_PASS overlay=None animation=None
ARCH73_HEADER_REGRESSION_PASS status=(13,60,496,46) motion=(517,60,165,46) theme=(690,60,245,46)
ARCH73_THEME_LIFECYCLE_PASS sweep_height<=220 settled=True
ARCH73_MATRIX_PASS 980/720 and 1240/820 × 4 tabs
ARCH73_CTA_PASS state=closed tabs_max=350
ARCH73_MOTION_PASS 120 PreciseTimer [8,8,9,8,8,9,8,8,9,8,8,9]
ARCH73_GEOMETRY_PASS 980x720 overlay=180x220 terminal_top=413 send_top=587; 1240x820 terminal_top=483 send_top=687
ARCH73_GEOMETRY_CLEANUP_PASS
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

首轮回归脚本曾把隐藏页面的 `settingsScroll` 横向范围当作失败；诊断确认隐藏页 viewport 只有
88px，当前可见 scroll area 的 maximum 均为 0，随后按项目约束修正为只检查可见页面并通过。
首轮架构师只读线程已调用但超时关闭，未伪造独立结论；随后独立 Luna/max 只读审查返回 `risk=medium`：
确认 stop/finish 的 active-animation 身份检查、两条动画停止、opacity 恢复和 overlay 延迟删除路径，
并指出 root 顶部父级与 queued cleanup 是残余风险，建议未来有明确 chrome 容器时再收敛父级。父代理随后
用 980/720 与 1240/820 的真实几何证据确认 terminal/send 均在 sweep 下方，并完成主题 owner、Qt effect、
生命周期、视觉和行为保持型简化复核。Python/Qt presentation-only，public MCU vendor source applicability=N/A；
无 embedded C/C++、硬件写入/刷写/部署。真实 GUI、HIDPI、读屏、设备和硬件验收未执行。

### ARCH-73 包交付

```text
PACKAGE_ARCH73_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,981,132 bytes
SHA256: DBD554999C65C18DDDC66643EA336CD90EFDF7B41DC43629194BAA044752F886
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-73
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-12 ARCH-72 / UI-1.145 Header bounded status text

顶部 header 的连接上下文、数据来源、连接状态改用 `BoundedTextLabel`。组件保持单行显示，按当前
label 宽度使用右侧省略，并在 `full_text`、tooltip、accessible description 中保留完整值；`resizeEvent`
以及字体/style 变化会重新计算，不把省略结果写回业务状态。它使用稳定的 bounded size hint 和可收缩
minimum width，使 status cluster 在 980px 下约 496px、1240px 下约 600px，motion controls 与
theme controls 保持独立间隙。`workspace.py` 只负责组装，`HeaderChromeBindings` 继续提供 typed
引用，lifecycle 沿用原有 `setText()` 更新路径。

```text
BOUNDED_LABEL_PASS narrow=elided/full_text+tooltip+accessible preserved wide=full
ARCH72_HEADER_PASS status=(13,60,496,46) motion=(517,60,165,46) theme=(690,60,245,46)
ARCH72_SOURCE_METADATA_PASS source value included in tooltip and accessible description
ARCH72_CTA_FOCUS_PASS focus=True lower_surfaces_hidden tabs_max=16777215
ARCH72_MATRIX_PASS star_trail/moonlit_ocean/sakura_night × 980/720/1240/820 × 4 tabs
ARCH72_MOTION_PASS 120 PreciseTimer [8,8,9,8,8,9,8,8,9,8,8,9]
scripts/check.ps1: pass
source line limit: pass (166 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, 19 selectors, legacy_qss_literals=0)
```

架构师只读线程已调用但超时关闭，未伪造独立报告；父代理完成独立调用链/生命周期/五轴复核与
行为保持型简化评估。Python/Qt presentation-only，public MCU vendor source applicability=N/A；
无 embedded C/C++、硬件写入/刷写/部署。离屏字体目录 warning 为既有环境提示，已通过
`configure_application_font()` 的真实字体路径复核；真实 GUI、HIDPI、读屏、设备和硬件验收未执行。

### ARCH-72 包交付

```text
PACKAGE_ARCH72_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,980,962 bytes
SHA256: 6925FDEAD6A98131F5ADD20BD46E9082DF5532AB3C733EAD4A67B2E2F2FD162C
archive listing SHA-256: C9C66CBFDA9102E0A13C6A95DD168ECDF3DC358009DFA125EBCB9D4CF636A732
provenance: pass; source revision local-arch-72
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-11 ARCH-71 / UI-1.144 Workspace navigation motion orchestration

导航模式变化时，`workspace_runtime` 让 focus geometry 成为唯一主过渡，并清理/跳过同一表面的
page opacity fade；同一 focus mode 内的普通 Tab 切换仍保留 180ms page fade。进入 focus 时
下方 live observation、terminal、send 立即隐藏并清零 maximum-height，过渡帧不再挤压内部控件；
返回总览通过 0→bounded target reveal，快速反向切换无残留 group。

```text
ARCH71_CTA_PASS viewport=514 state=closed
ARCH71_MATRIX_PASS star_trail/moonlit_ocean/sakura_night × 980/720/1240/820 × 4 tabs
NAV_FOCUS_PRIMARY_PASS focus_active=True page_fade=None
NAV_SAME_MODE_FADE_PASS focus_active=None page_fade_active=True
NAV_RETURN_PRIMARY_PASS focus_active=True page_fade=None
ARCH71_REDUCED_MOTION_PASS
FOCUS_ENTER_NO_SQUEEZE_PASS [0, 0, 0] 350 True
FOCUS_ENTER_SETTLED_PASS 514 [0, 0, 0]
FOCUS_RETURN_SETTLED_PASS 115 [96, 169, 98]
FOCUS_RAPID_REVERSAL_PASS
ARCH71_MOTION_INVARIANT_PASS 120 [8, 9] 56
scripts/check.ps1: pass
source line limit: pass (165 files <= 1000)
theme token audit: pass (3 themes, 22 semantic tokens, legacy_qss_literals=0)
```

架构师只读线程已调用但超时关闭，未伪造独立报告；父代理完成独立调用链/生命周期/五轴复核与
行为保持型简化评估。Python/Qt presentation-only，public MCU vendor source applicability=N/A；
无 embedded C/C++、硬件写入/刷写/部署。真实 GUI、HIDPI、读屏、设备和硬件验收未执行。

### ARCH-71 包交付

```text
PACKAGE_ARCH71_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,978,127 bytes
SHA256: D3D73A8EFC8D4A1531C335CC1D5280C9F24A047A4484D4DBB523A1C1E715E5B6
archive listing SHA-256: FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969
provenance: pass; source revision local-arch-71
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-11 ARCH-70 / UI-1.143 Connection onboarding focus

终端空态 CTA 现在进入链路页的 presentation focus：先选择 index 0，再复用既有
`set_workspace_focus_mode(True)`，因此 980×720 这类紧凑窗口也能直接看到完整 UART
配置表单。用户点击既有 focus button 可返回总览；ViewModel session state 保持
`closed`，协议/遥测页的自动 focus 路由仍可进入并返回。bootstrap 只负责连接 signal，
workspace runtime 负责展示导航，未引入连接、会话、持久化或传输业务耦合。

CTA 离屏验证：`CONNECTION_CTA_FOCUS_PASS viewport=514 lower_visible=[False, False, False]
state=closed`；`CONNECTION_CTA_RETURN_PASS overview=115`；`TAB_ROUTE_COMPAT_PASS
focus_restored`。源代码修改后完整三主题 × 两尺寸 × 四 Tab 矩阵、可见 scroll area 无横向溢出、
root child 无重叠、低动效静态 focus/overview 通过；冷启动 motion 为 105 frames/1007ms、
104.3 FPS，`PreciseTimer` 与 8/9ms slots 通过。离屏 PySide6 仍有既有 fonts directory warning；
真实 GUI、HIDPI、读屏、设备和硬件验收未执行。

架构师只读线程本轮已调用但在时限内超时并关闭，独立 Luna 只读评审线程随后同样超时并关闭，
二者均未伪造结论或视为通过；父代理基于源码、真实组合根和产物证据完成五轴复核与行为保持型
简化评估。Python/Qt presentation-only 变更，公共 MCU 厂商资料
适用性：N/A；无嵌入式 C/C++、无硬件写入/刷写/部署。包交付：`local-arch-70` onefile 已生成并覆盖
canonical/root/root-latest；三者均为 `47,976,952` bytes，SHA-256
`17F57F53EA2F3D49EB37E3E2581773F46F9528A243259D287E542EFE64FD3245`；archive listing SHA-256
`FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969`；provenance pass，签名
`NotSigned`，`release_eligible=false`，`hardware_acceptance=not_run`。

## 2026-08-11 ARCH-6z / UI-1.142 Adaptive workspace focus

配置路由自动复用既有 focus transition：980×720 总览 workspace 为 150px，配置 focus 为
506/542px；1240×820 总览为 220px，配置 focus 为 606/642px。三主题 × 两尺寸 × 四 Tab
无 root child overlap，horizontal maximum=0；reduced-motion 静态回退、手动 focus button、
120Hz target/8-9ms slot 和 250ms offscreen 29 frames 通过。架构师线程已调用但超时关闭，
父代理完成五轴与简化复核；真实 GUI、HIDPI、读屏、设备和硬件验收未执行。

### ARCH-6z 包交付

```text
PACKAGE_ARCH6Z_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,976,464 bytes
SHA256: C7D8D93ED5907648BAF348BD560410BAF0348C5A02705F9880157E68FCD5C9CB
archive listing SHA-256: FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969
provenance: pass; source revision local-arch-6z
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-11 ARCH-6y / UI-1.141 Header Chrome typed binding

`HeaderChromeBindings` 已完成真实组合根 identity、980×720/1240×820 两种尺寸、四个
workspace page 横向滚动检查、三主题切换和 56 个无重复 motion surface 检查。`MotionController`
目标仍为 120Hz，`PreciseTimer` 与 8/9ms slot vector 通过。离屏环境报告既有 PySide6 fonts
directory warning，不代表 Windows 系统字体缺失。

### ARCH-6y 包交付

```text
PACKAGE_ARCH6Y_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,978,616 bytes
SHA256: 842BF2981DBC4EAFC47C30819904DD290BC087A16EB1565E633FD0C5C4028C8C
archive listing SHA256: FD2322723A63AD7B425D9F9B763D96DB2730E0C558B642548AF0BAEC86323969
provenance: pass; source revision local-arch-6y
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-11 ARCH-6x / UI-1.140 链路控制外壳绑定边界

`ConnectionShellBindings` 已完成真实组合根 identity、980×720/1240×820 两种尺寸、四个
workspace page 横向滚动检查，以及三主题切换检查。`MotionController` 目标仍为 120Hz，
`PreciseTimer` 与 8/9ms slot vector 通过。离屏环境报告既有 PySide6 fonts directory warning，
不代表 Windows 系统字体缺失。

### ARCH-6x 包交付

```text
PACKAGE_ARCH6X_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,975,290 bytes
SHA256: AE11EE6B8BED8F04CD844994B2FA60820232391084DDD57978B2CB2FB4D4BCE8
archive listing SHA256: 60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C
provenance: pass; source revision local-arch-6x
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

## 2026-08-11 ARCH-6w / UI-1.139 命令批处理绑定边界

新增 `presentation/command_bindings.py` 的 frozen/slots `CommandBatchControlBindings`，由
`bootstrap.py` 在 `workspace.py` 完成命令管理页构建后唯一组装。commands、connection、command
selection、composition 和 lifecycle 通过 `command_batch_bindings_for()` 读取 combo、动作按钮、
状态 rail、结果表和空态；batch catalog/snapshot、ViewModel、执行策略、timer、callback 和
transport handle 不进入 bundle。`terminal.py` 保留构建阶段动态字段作为装配桥，新的跨 controller
读取点已清零。

`COMMAND_BINDINGS_STATIC_PASS`、`COMMAND_BINDINGS_RUNTIME_PASS`、`COMMAND_BINDINGS_LAYOUT_PASS`、
`scripts/check.ps1`、compileall、Ruff 和 980×720 真实组合根 binding vector 通过；未新增 timer、
状态源、业务动作或测试资产。离屏环境仍报告既有 PySide6 fonts directory warning；不代表 Windows
字体缺失。

### ARCH-6w 包交付

```text
PACKAGE_ARCH6W_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,974,086 bytes
SHA256: 3BCACA7C90A39C23FF7E4C4605F021E463174AF2BE8032C75496C6E10A0F6327
archive listing SHA256: 60CADFDE893442CAC3AE9797A0EA53F8233813B1D4D66C458DEE413E8474BD0C
provenance: pass; source revision local-arch-6w
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

架构师角色线程 `019ff141-f6a2-74e2-b885-57d12f3c11fa` 已调用并限定为只读审查，等待窗口内未返回
独立报告，已安全关闭；父代理完成 owner、初始化时序、依赖方向、Qt 生命周期、Tab/accessibility、
共享动效、性能和行为保持型简化审查。

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public vendor
source；不作 MISRA/ISO/认证声明，未执行硬件、刷写、部署或破坏性操作。

## 2026-08-11 ARCH-6v / UI-1.138 120Hz 动效与站台布局

`MotionController` 从 96ms 粗定时器迁移到 120Hz 目标的 PreciseTimer；8/9ms slot 交替，按 elapsed
time 推进 phase。lifecycle 只向当前可见装饰 surface fan-out，并对相邻 signal rail 使用确定性小相位差，
避免 Header、route、状态 rail 和发送 rail 同步撞帧；低动效、暂停、隐藏、最小化和关闭仍静态收敛。
实时观测/发送/历史布局去除固定提示宽度反向撑列，补齐 bounded 标题宽度、row height、horizontal/
vertical spacing；默认窗口 1240×820，最小高度 720。

`MOTION_LAYOUT_STATIC_PASS`、`UI138_MIN_SCREENSHOT_PASS`、`UI138_SCREENSHOT_PASS` 已通过；offscreen
cadence probe 为 `TARGET_HZ=120`、`PreciseTimer`、`singleShot=True`、26 frames/240ms。980×720 与
1240×820 geometry 均确认 station bands 无 overlap，terminal/send 控件没有负间距；PySide6 仍报告
既有 fonts 目录 warning。`local-arch-6v` 最新 onefile 已通过 provenance verify 并覆盖 canonical、根目录
`SerialForge.exe` 与 `SerialForge-latest.exe`。

### ARCH-6v 包交付

```text
PACKAGE_ARCH6V_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,972,295 bytes
SHA256: C9ABCAE17940617A6C81A01F56B91BE09828FC5B996E6181176A82F8656A9DA5
archive listing SHA256: E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876
provenance: pass; source revision local-arch-6v
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

架构师线程 `019ff133-ef65-7270-862c-792cfe1ab8d3` 已调用但在限定窗口超时关闭，未计为独立通过；父代理
完成依赖方向、单时钟、可见 fan-out、Qt 生命周期、布局、accessibility、性能与行为保持型简化审查。

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public vendor source；
不作 MISRA/ISO/认证声明，未执行硬件、刷写、部署或任何破坏性操作。

最终 assurance gate：`EMBEDDED_VENDOR_SOURCE_APPLICABILITY=N/A`；
`EMBEDDED_INDEPENDENT_REVIEW=not_applicable_to_python_ui; architect_timeout_recorded`；
`SIMPLIFICATION_ASSESSMENT=passed_parent_review_no_behavior_change`；
`AUTHORIZED_NON_DESTRUCTIVE_VERIFICATION=compileall+ruff+source-limit+theme-audit+offscreen-geometry+cadence-probe+visual-screenshot+provenance+hash`。

## 2026-08-11 ARCH-6u 终端/发送控件组合绑定

新增 `presentation/terminal_bindings.py` 的 `TerminalControlBindings` 与
`terminal_bindings_for()`。`bootstrap.py` 在 terminal/send widgets 构造完成后一次性组装；
terminal runtime、connection gate、commands、command selection、send context、composition、
lifecycle 和 workspace focus transition 不再直接读取终端/发送/历史 Qt 动态字段。bundle 只持有
实时观测、终端、记录、发送、快捷命令和历史控件的 Qt 引用；preview buffer/pending、history/quick
snapshot、recording/session state、timer、MotionController、ViewModel 和 callbacks 仍留在原 owner。

架构师角色线程 `019ff129-d626-78b0-9852-fe743f2b7d99` 已调用，在限定窗口内超时并关闭，未计为
独立通过；父代理完成 owner、依赖方向、状态隔离、Qt 生命周期、行为保持、键盘/accessibility、
主题、响应式、性能与行为保持型简化审查。复用了 UART/网络/BLE accessor 模式，没有新增 timer、
事件总线、backend、业务状态或测试资产。

`TERMINAL_BINDINGS_COMPILE_PASS`、`TERMINAL_BINDINGS_COMMAND_SLICE_PASS`、
`TERMINAL_BINDINGS_RUNTIME_SLICE_PASS`、`TERMINAL_BINDINGS_LIFECYCLE_SLICE_PASS`、
`TERMINAL_DYNAMIC_WIDGET_OWNER_ONLY_PASS` 和真实组合根
`ARCH6U_TERMINAL_VECTOR_PASS 24` 已通过；三主题 × 980/1180 × 四工作区的 terminal/send
binding、发送上下文和可见 scroll page 均 `hmax=0`、exact-white=0、near-white=0。PySide6
仍报告既有 fonts 目录 warning，但系统字体离屏渲染正常；GUI/EXE 启动、读屏、真实设备、硬件、
刷写和部署验收未运行。`local-arch-6u` onefile 已通过 provenance verify，并覆盖 canonical、根目录
`SerialForge.exe` 与 `SerialForge-latest.exe`。

### ARCH-6u 包交付

```text
PACKAGE_ARCH6U_FINAL                         pass
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
root-latest: SerialForge-latest.exe (byte-identical)
size: 47,969,870 bytes
SHA256: 485AFE4B7A581E27154BFA1043290D872FDB862CA61F35219945D88FA72839A8
archive listing SHA256: E01F1A9B8689B822A7806272966DEF1637F1F5F6828ED7EBC8F739E925816876
provenance: pass; source revision local-arch-6u
signature: NotSigned; release_eligible=false; hardware_acceptance=not_run
ROOT_EXE_OVERWRITE_PASS: canonical/root/root-latest size and SHA-256 identical
```

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public
vendor source；不作 MISRA/ISO/认证声明。未执行硬件、刷写、部署或任何破坏性操作。

## 2026-08-11 ARCH-6t BLE GATT 控件组合绑定

新增 `presentation/connection_bindings.py` 的 `BleControlBindings` 与
`ble_bindings_for()`。`connection_builder.py` 是唯一构造与 signal wiring owner；BLE selection、
action、connection/runtime、connection gate、preset、composition、commands、lifecycle 和
terminal runtime 不再直接读取 BLE 动态 widget 字段。bundle 只持有 panel/title/hint、timeout、
filter、device/cache/pair、characteristic/properties、read/notify/write-mode 的 Qt 引用；
`_ble_notification_pending`、`_ble_notification_ref`、`_ble_notification_timer` 仍由原 owner
管理，不进入 bundle。

架构师角色线程 `019ff11a-1ba7-7fb1-ba14-7d9442051e7f` 已调用，在两个限定等待窗口内超时并
关闭，未计为独立通过；父代理完成 owner、依赖方向、状态隔离、Qt 生命周期、行为保持、主题、
accessibility、性能与行为保持型简化审查。复用了 UART/网络 accessor 模式，没有新增 timer、
backend、密钥、业务状态或测试资产。

本轮追加的独立架构师终审线程 `019ff123-7d9f-7dd0-81a8-e03b42397730` 同样在限定窗口内超时
并关闭，未计为独立通过；因此最终结论以父代理审查和可重复的静态/离屏/打包证据为准。

`ARCH6T_COMPILE_PASS`、`ARCH6T_RUFF_PASS`、`BLE_DYNAMIC_WIDGET_OWNER_ONLY_PASS` 和真实组合根
`ARCH6T_BLE_VECTOR_PASS 30` 已通过；三主题 × 980/1180 × 四工作区与 BLE 模拟设备/特征投影均
`hmax=0`、exact-white=0、near-white=0。`local-arch-6t` onefile 已生成并通过 provenance verify；
canonical/root/root-latest 均为 `47,968,345` bytes，SHA-256 为
`019CB04DE1400D4A03F5B51C662C31B7A4C25195EB83C844FA1EB8ECD0B23024`，archive listing SHA-256 为
`C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`，签名 `NotSigned`，
`release_eligible=false`，hardware acceptance `not_run`。PySide6 仍报告既有 fonts 目录 warning，
但系统字体离屏渲染正常；GUI/EXE 启动、读屏、真实 BLE 设备、硬件、刷写和部署验收未运行。

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public
vendor source；不作 MISRA/ISO/认证声明。未执行硬件、刷写、部署或任何破坏性操作。

最终门禁：`ARCH6T_FINAL_VALIDATION_PASS`；`BLE_DYNAMIC_WIDGET_OWNER_ONLY_PASS`；
`EMBEDDED_VENDOR_SOURCE_APPLICABILITY=N/A`；`EMBEDDED_INDEPENDENT_REVIEW=not_applicable_to_python_ui`
（架构师超时记录见上）；`SIMPLIFICATION_ASSESSMENT=passed_parent_review_no_behavior_change`；
`AUTHORIZED_NON_DESTRUCTIVE_VERIFICATION=static+offscreen+provenance+hash`。

## 2026-08-11 ARCH-6s 网络端点控件组合绑定

新增 `presentation/connection_bindings.py` 的 `NetworkControlBindings` 与
`network_bindings_for()`。builder 是唯一构造 owner，TCP Client/Server、UDP、RTT 共用的
网络 panel wiring 收敛到 typed bundle；bundle 只持有 Qt 引用，不包含
`server_target_explicit`、默认值标志、peer snapshot、业务状态、DTO、callback、timer、
设备句柄或网络策略。BLE 保持独立 feature boundary。

架构师线程 `019ff110-8695-7383-b627-5c85caee76c4` 已调用，在限定窗口内超时并关闭，未计为
独立通过；父代理完成 owner、依赖方向、状态隔离、Qt 生命周期、行为保持、主题、
accessibility、性能与简化审查。

`ARCH6S_COMPILE_PASS`、`ARCH6S_RUFF_PASS`、`NETWORK_DYNAMIC_WIDGET_OWNER_ONLY_PASS` 和
真实组合根 `ARCH6S_NETWORK_VECTOR_PASS 30` 已通过；三主题 × 980/1180 × 四工作区与五种
网络相关模式均 `hmax=0`、exact-white=0、near-white=0。PySide6 仍报告既有 fonts 目录
warning，但系统字体渲染正常；GUI/EXE 启动、读屏、硬件和部署验收未运行。

onefile canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)；
source revision `local-arch-6s`；canonical/root/root-latest 均为 `47,965,543` bytes；SHA-256
`E9F3D230B75FFA54D61F5190556CE3393F308A4CC75E53B303DEA01E74BD71CA`；archive listing SHA-256
`C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`；provenance verify pass；
签名 `NotSigned`，`release_eligible=false`，hardware acceptance `not_run`。根目录两个 EXE
已覆盖并通过字节/哈希一致性校验。

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public
vendor source；不作 MISRA/ISO/认证声明。GUI/EXE 启动、读屏、真实设备、刷写和部署验收未运行。

## 2026-08-11 ARCH-6r UART 控件组合绑定

新增 `presentation/connection_bindings.py` 的 `UartControlBindings` 与
`uart_bindings_for()`。builder 是唯一构造 owner，跨 controller 的 UART widget 读取收敛到
typed bundle；bundle 只持有 Qt 引用，不包含业务状态、DTO、callback、timer、设备句柄、密钥
或传输策略。builder 内保留兼容字段，作为后续 TCP/BLE/RTT typed bundle 迁移的安全过渡。

架构师线程 `019ff103-cb0d-7610-8306-66dffee0b675` 已调用，在限定窗口内超时并关闭；独立
嵌入式 assurance reviewer `019ff108-4743-7c21-989d-861a1dd8a496` 未在限定窗口内返回；父代理
完成 owner、依赖方向、Qt 生命周期、行为保持、主题、accessibility、性能与简化审查。

`ARCH6R_COMPILE_PASS`、`ARCH6R_RUFF_PASS`、源码 `162 files <= 1000`、`scripts/check.ps1`、
`UART_DYNAMIC_FIELDS_OWNER_ONLY_PASS` 和真实组合根 `ARCH6R_UART_VECTOR_PASS 24` 已通过；
三主题 × 980/1180 × 四 Tab 均 `hmax=0`、exact-white=0、near-white=0。PySide6 已报告既有
fonts 目录 warning，但系统字体渲染正常；GUI/EXE 启动、读屏、硬件和部署验收未运行。

onefile canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)；
source revision `local-arch-6r`；canonical/root/root-latest 均为 `47,963,397` bytes；SHA-256
`BF0EDF73CC7BDFBA5567DC9CC6A55D8365F6979D65BDEF56EB55637790095529`；archive listing SHA-256
`C8FE3A7CE1807A4E6B164CEB0EACB4D51120E15B9F8937FD4BBC8C604F57EBE4`；provenance verify pass；
签名 `NotSigned`，`release_eligible=false`，hardware acceptance `not_run`。根目录两个 EXE
已覆盖并通过字节/哈希一致性校验。

嵌入式 C/C++/固件适用性：N/A；本轮只有 Python/PySide6 presentation 变更，无适用 public
vendor source；不作 MISRA/ISO/认证声明。GUI/EXE 启动、读屏、真实设备、刷写和部署验收未运行。

## 2026-08-11 UI-1.137 工作区当前页上下文表面

新增 `presentation/workspace_context_surface.py:WorkspaceContextLabel`，把真实的四个 Tab
index 投影为“当前页 · 链路配置 / 解析与遥测 / 命令管理 / 能力预览”。表面只读、NoFocus、
鼠标透明，state、文本、AccessibleDescription 和 tooltip 一次性同步；不读取 ViewModel、
Session、transport、OTA/debug DTO，不创建 timer、导航动作、第二套状态源或设备 I/O。

`WorkspaceShellBindings` 增加 typed `context_label`，`workspace.py` 唯一组装，
`workspace_runtime.py` 在既有 Tab change 回调中同步。base/variant controls stylesheet
对称复用 ThemeSpec token，route strip 仍为固定 31px，label 宽度限定 158～218px。

架构师角色线程 `019feddb-896c-7833-be05-c7d2052a767d` 在限定窗口内超时并关闭；独立只读
reviewer `019ff0f1-88f0-7d60-911f-a12180c02b1e` 在本交接截点未返回，未计为独立通过；父代理
完成 owner、依赖方向、index 边界、Qt 生命周期、QSS 对称性、accessibility、简化审查。
真实组合根三主题 × 980/1180 × 四 Tab 共 24 组通过，当前页 horizontal maximum=0，
exact-white=0、near-white=0；1180×780 连接页/扩展页截图已人工复核。

canonical onefile：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)；
source revision `local-ui-1.137`；`47,961,992` bytes；SHA-256
`7509738A8CD05655E8899497DCB67143A05C34999189426B3A2482EF1961829B`；archive listing SHA-256
`12D69406BC04C668C3621D831004A420FFFD331AA3EB409479472BF10A93C2C4`；provenance pass。
根目录 `SerialForge.exe` 与 `SerialForge-latest.exe` 已与 canonical 字节一致；签名
`NotSigned`，`release_eligible=false`，hardware acceptance `not_run`。

嵌入式 C/C++/固件适用性：N/A；本轮仅 Python/PySide6 presentation 变更，无厂商目标资料适用，
未运行 GUI/EXE 启动、读屏、硬件、刷写或部署验收，不作 MISRA/ISO/硬件合规声明。

## 2026-08-11 UI-1.136 工作区滚动可发现性提示

新增 `presentation/workspace_scroll_hint.py:WorkspaceScrollHint`，在既有
`workspaceRouteStrip` 显示顶部 `↓ 向下查看`、中部 `↕ 上下滚动`、底部 `↑ 返回顶部`
和无溢出 `内容已全部显示` 四态。`workspace.py` 只负责组装，
`workspace_runtime.on_workspace_tab_changed()` 在 Tab 切换时解绑旧 scrollbar signal 并绑定
当前 settings page；提示提供动态 tooltip/AccessibleDescription，NoFocus、鼠标透明，不拥有
业务状态、导航模型、timer 或第二套滚动策略。

架构师线程 `019fedc5-ee6c-7a43-8178-9ee97bb0559d` 与独立代码审查线程
`019fedcb-bcd4-7fe0-abb5-ff663995cd9b` 均在两个限定等待窗口内未返回并已关闭，未计为独立通过；
父代理完成 owner、signal 解绑、Tab 边界、Qt API、无障碍、主题 token、性能与简化审查。
三主题 × 980/1180 × 四 Tab 共 24 组通过，当前页 horizontal maximum=0，exact-white/near-white=0；
`scripts/check.ps1`、compileall、Ruff、provenance pass。未创建/修改/运行 unit test/mock/fixture/harness，
未运行 GUI/EXE 启动、读屏、硬件、刷写或部署验收。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.136`；47,957,987 bytes；SHA-256
`D028C8D939A57A64BCD991CF6E0AAF404883450307974049566891560696DACC`；archive listing SHA-256
`B9F1EB9352EC1F155CE7E8F334DD34FCE442221281D32C4CBE59AE3276B7FE9E`；签名 `NotSigned`，
`release_eligible=false`，hardware acceptance `not_run`。根目录 `SerialForge.exe` 仍被 PID
46108/49236 锁定；未强制终止，已额外生成根目录 `SerialForge-latest.exe`，与 canonical 字节一致。

嵌入式 C/C++/固件适用性：N/A；本轮仅 Python/PySide6 presentation 变更，无厂商目标资料适用，
不作 MISRA/ISO/硬件合规声明。

## 2026-08-11 ARCH-6q Workspace shell typed bindings

新增 frozen/slots `WorkspaceShellBindings`，由 `controllers/workspace.py` 唯一组装，bootstrap
只消费 `bundle.shell`；workspace runtime、focus transition、焦点顺序、lifecycle、derived 和
terminal runtime 通过 `workspace_bindings_for()` 消费六个 Qt widget 引用，移除散落的
`window._workspace_*` facade。bundle 不包含业务状态、timer、transport、导航策略或 callback，
header 主题初始化时 bundle 尚未创建也会安全返回。

架构师线程 `019fedd1-fa7c-72a3-81c4-96ac4cb0f808` 与独立审查线程
`019fedd6-db35-7d12-a58b-f7405b884fa9` 均在两个限定等待窗口内未返回并已关闭；父代理完成
owner、依赖方向、生命周期、行为保持、性能与简化审查。真实组合根四 Tab、专注展开/恢复、三主题
× 980/1180 × 四 Tab 共 24 组 vector 通过，横向 maximum=0，exact-white/near-white=0；
static/compileall/ruff/source-limit/theme-token/provenance pass。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-arch-6q`；47,960,028 bytes；SHA-256
`BACE302A92900CBC7A8D27D41D6BDF1082C70AE58F64F3F0D305AAF737701E63`；archive listing SHA-256
`CFC788B805981C451920D9757F1FE48CD618C9049058B8ED52B20B70CF53EAC0`；签名 `NotSigned`，
`release_eligible=false`，hardware acceptance `not_run`。根目录原名仍被 PID 46108/49236 锁定，
未强制终止；`SerialForge-latest.exe` 已更新且与 canonical 字节一致。

## 2026-08-11 UI-1.135 扩展工具站能力分组摘要

`application/extension_station.py` 新增 frozen/bounded 的 `ExtensionStationGroupSummary`，
由 `extension_station_summary()` 从既有 catalog 生成 OTA 传输、OTA 安全和调试输出三组能力计数，
并校验分组计数与总能力/激活后端计数一致。`presentation/embedded_station_overview.py` 只消费
该 DTO，在六列 metrics grid 中展示分组数量；分组超过六项时自动换行，不引入第二套状态源。

架构师角色线程 `019fedb2-4d88-79b3-ba2d-08cca3ccf335` 在两个等待窗口内未返回，已关闭，
未计为独立通过；首屏裁剪修复线程 `019fedb9-d420-76d0-a6a0-154c8dac31f2` 同样两个窗口未返回并已关闭。
API 兼容性架构审查线程 `019fedc0-9eaa-7c33-873d-1007434a816d` 已完成，确认新增
`group_summaries` 位于 dataclass 末尾、旧 positional 前缀保持不变；父代理完成 owner、DTO 不变量、
presentation 单向依赖、响应式、无障碍、行为保持、接口兼容与简化审查。三主题/980/1180 runtime vector、静态门禁和 onefile 打包已完成；截图为
`build/ui_review_ui135_extension_groups.png`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.135`；47,953,759 bytes；SHA-256
`5E5A71B3BAA59D581CD7093B53225395AB59C26011EA32696254D4DD994CF22C`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。
根目录覆盖仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.134 UART selector 响应式宽度

真实 Qt 审计发现 980×680 链路 / 连接页的 UART 八列 grid 最小宽度为 1013px，而可用内宽
为 908px，horizontal maximum=105。根因是 port combo 240px presentation 上限与长流控
option 的 218px size hint；在 `connection_builder.py` 内统一使用 bounded combo policy，
将 port 上限收敛为 200px，并为 data/parity/stop/flow 设置 84/110、84/150、84/130、
110/160px 边界，波特率沿用 110/150px。itemData、COMx 手输、选项、signals、tooltip、
accessibility、连接 gate 和 DTO 不变。

验证：三主题 × 980×680/1180×780 × UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT，
并覆盖四个工作区，共 60 组 `hmax=0`、`exact-white=0`；UART layout minimum=872px，
视觉证据为 `build/ui_review_ui134_connection_responsive.png`。scripts/check.ps1、compileall、
Ruff、源码行数门禁与 onefile provenance 通过。架构师线程
`019feda9-d39f-7d11-9433-1a6c34ea1ef3` 超时，未计为独立通过；父代理完成六轴、响应式、
可读性、可访问性、行为保持和简化复核。未修改嵌入式 C/C++；`embedded-enterprise-workflow`
与 `embedded-code-review-simplifier` 不适用；public vendor applicability=N/A；真实硬件验证
保持 `not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.134`；47,950,558 bytes；SHA-256
`A5282CDCFF9FF31321C46DD382E8A04CA1BCE39BE16B05D6A3CC4004C0B91057`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。
根目录覆盖仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.133 扩展工具站接入路线 glyph

扩展 / 工具站页的接入概览现在拥有资源无关的 96×64 三节点路线 glyph。glyph 由
`EmbeddedStationOverview` owner 持有，只读取父链 `ThemeSpec` 和共享 `MotionController`
frame；`stop()`、隐藏窗口、低动效和暂停保持静态路线。`embedded_extension_panel.py` 通过
frozen `ExtensionPanelWidgets` 显式返回 layout 与 overview，`workspace.py` 只负责把 overview
交给窗口，`lifecycle.py` 复用既有 `_motion_surfaces()` fan-out，不使用 `findChild()` 或第二个
动效时钟。

验证：三主题 × 980×680/1180×780，overview geometry 为 908/1108px 宽、101px 高，route
geometry 为 96×64，extension scroll horizontal maximum=0；共享帧 animated/stop 与隐藏窗口
静态回退通过。视觉证据为 `build/ui_review_ui133_extension.png`。scripts/check.ps1、
compileall、Ruff、源码行数门禁与 onefile provenance 通过。架构师线程
`019fed9e-6e2f-7722-9a21-b53cebea67ac`、独立审查线程 `019feda2-220e-7e13-b4da-7067f2f32bfa`
均超时，未计为独立通过；父代理完成六轴、owner、可访问性、响应式、行为保持与简化复核。
未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与 `embedded-code-review-simplifier` 不适用；
public vendor applicability=N/A；真实硬件验证保持 `not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.133`；47,951,506 bytes；SHA-256
`1E2004935AFC936E9BD569C0063552EFC57DEE2C0F20B6DE64A494CB4D0F7BE6`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。
根目录覆盖仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.132 批量命令空态 copy 几何

真实 Qt 审计发现命令空态的 eyebrow/title/hint QLabel 在 1180×780 专注模式下分别被拉伸到
约 119px，文本实际只需约 14/19/30px。`CommandBatchEmptyState` 保留可扩展卡片外壳，
仅让三个 copy label 使用纵向 `Fixed`，使文案紧凑而 glyph、CTA 与卡片空间继续保持原有
presentation 组合和共享动效。

验证：三主题 × 980×680/1180×780，空态与长 hint 均通过；copy 高度 14/19/30px，当前页
horizontal maximum=0、exact-white=0；视觉证据为 `build/ui_review_ui132_command_empty.png`。
scripts/check.ps1、compileall、Ruff、onefile provenance 通过。架构师线程
`019fed95-29fe-7e73-b091-0acf973dbd97` 超时，未计为独立通过；父代理完成 owner、响应式、
可访问性、五轴质量和简化复核。未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 不适用；public vendor applicability=N/A；真实硬件验证保持
`not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.132`；47,945,280 bytes；SHA-256
`68B7C0E6387C621FAAA889D8440DFCDB8F509DF407A01F833C36980BCC9254D2`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.131 网络与 RTT responsive hint 几何

六传输真实 Qt 审计发现 TCP Client 网络说明 QLabel 在专注模式稳定后约 151px、RTT 说明约
111px，文本 size hint 约 50–67px。共享 `_configure_responsive_hint()` 保留 word-wrap、
120/520px 宽度边界、tooltip/accessibility 与横向 `Ignored`，将纵向 policy 改为 `Fixed`，
使说明不再吸收父级剩余高度。

验证：三主题 × 980×680/1180×780 × UART/TCP Client/TCP Server/UDP/BLE GATT/J-Link RTT 共
36 组，说明按 size hint 收敛，标题/active panel 无重叠，horizontal maximum=0、exact-white=0；
视觉证据为 `build/ui_review_ui131_tcp_hint.png`、`build/ui_review_ui131_rtt_hint.png`。
scripts/check.ps1、compileall、Ruff、onefile provenance 通过。架构师线程
`019fed8d-6fdc-7532-8fdd-387290c71924` 超时，未计为独立通过；父代理完成 owner、响应式、
可访问性、五轴质量和简化复核。未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 不适用；public vendor applicability=N/A；真实硬件验证保持
`not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.131`；47,947,904 bytes；SHA-256
`61379AF7545EBF6745B726D56BFA41121640055BCAEE92EA82B47351EA238AF2`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.130 UART 参数摘要 rail 几何

真实 Qt 几何审计发现 `UartTimingSummarySurface` 在专注设置模式稳定后高度约 109px，而摘要
内容的 size hint 约 38px，造成蓝色状态 rail 吸收剩余高度。将该 presentation owner 的纵向
`QSizePolicy` 从 `Preferred` 改为 `Fixed`，保留横向 `Ignored`，不设置硬编码高度、不改变
摘要投影、连接行为或无障碍文案。

验证：星轨霓虹、月影深海、樱雾夜航 × 980×680/1180×780，摘要高度均为 38px，当前页
horizontal maximum=0，exact-white=0；视觉证据为 `build/ui_review_ui130_uart_summary.png`。
scripts/check.ps1、compileall、Ruff、onefile provenance 通过。架构师线程
`019fed87-2dc8-7160-9148-192ecd61726f` 在限定窗口内超时，未计为独立通过；父代理完成 owner、
响应式、可访问性、五轴质量和简化复核。未修改嵌入式 C/C++；`embedded-enterprise-workflow`
与 `embedded-code-review-simplifier` 不适用；public vendor applicability=N/A；真实硬件验证
保持 `not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.130`；47,947,485 bytes；SHA-256
`3AC144396C9EB88AD9B447572A3B01965E5EF07641A95621AEEEF2E2210DF52C`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.129 UART 空端口输入引导

`controllers/connection_builder.py` 在 UART 端口 combo 仍保持可编辑的前提下，定义
`未发现端口 · 点击刷新或输入 COMx` 并同时写入 `QComboBox.placeholderText()` 与内部
`QLineEdit.placeholderText()`。首轮运行时检查确认 Qt 不会自动在 editable combo 内传播这两个值，
因此没有把空态提示误认为已完成验证；修正后真实组合根三主题 × 980/1180 均通过，`COM99` 手动
输入和清空仍保持 currentText 兼容，当前页 horizontal maximum=0，截图 exact-white=0。

验证证据：`build/ui_review_ui129_uart_empty_port.png`；scripts/check.ps1、compileall、Ruff、
onefile provenance 通过。架构师线程 `019fed79-c377-7823-a89e-c1c3127cf77d`、
`019fed7b-a8ca-7540-b9d7-68257c3985f5`、`019fed7d-f207-7342-9cd5-a9f00a3ca086` 均在限定窗口内
超时，未计为独立通过；父代理完成 owner、兼容性、生命周期、五轴质量和简化复核。未修改嵌入式
C/C++；`embedded-enterprise-workflow` 与 `embedded-code-review-simplifier` 不适用；public vendor
applicability=N/A；真实硬件验证保持 `not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.129`；47,947,205 bytes；SHA-256
`7E6EA48736EDF112EEDEBA86B65BCBE1BF4C9DF8522AACEF6D0C61E140272620`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.128 连接页 section 标题几何

`controllers/connection_builder.py:_section_label()` 统一构造 UART、网络和 BLE section 标题，
保留 `role="section"`，并设置横向 `Preferred`、纵向 `Fixed` 的 size policy。修复前 980×680
专注设置视图中 `UART 参数` 标题被父级剩余高度拉伸到约 106px，形成明显空白；修复后真实运行时
三主题 × 980/1180 × UART/TCP Client/TCP Server/UDP/BLE/RTT 共 36 组标题高度均为 26px。

验证：active title/panel 无重叠、当前页 horizontal maximum=0、exact-white=0，专注模式和总览模式
均正常收敛；静态门禁、compileall、Ruff、onefile provenance 通过。架构师线程
`019fed71-9d02-7731-b268-c48f59327b28` 在限定窗口内超时，未计为独立通过；父代理完成
correctness/readability/architecture/security/performance 和简化复核。未修改嵌入式 C/C++；
`embedded-enterprise-workflow` 与 `embedded-code-review-simplifier` 对本轮源码不适用；public vendor
applicability=N/A；真实硬件验证保持 `not_run`。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.128`；47,947,626 bytes；SHA-256
`D7EB3B79B0E86F46AB24AF5B951496B25F1797627021DA4A7786545A87A951B1`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；根目录旧包仍为 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.127 专注设置模式与总览模式过渡

`workspaceFocusButton` 位于既有 route strip，默认显示“专注设置”；切换后显示“返回总览”，
并更新 tooltip/accessibility description。`workspace_focus_transition.py` 只管理当前 window 的
presentation mode 和一次性 height animation：专注时隐藏 live/terminal/send 三个下方 surface，
workspace tabs 释放 350px 上限；退出时反向恢复。所有 session worker、terminal ingestion、recording、
pipeline 和连接状态继续由原 owner 运行，模式不写入 QSettings。

验证：动态中间帧高度从 `94/144/96` 平滑降至 `0/0/0`，反向恢复通过；reduced-motion 下 transition
为 `None` 且静态收敛；hide 清理 transition，show 保持模式；三主题 × 980/1180 × 四 tab 的
exact-white=0、horizontal maximum=0、route/tab overlap=false。架构师线程
`019fed62-45b0-7dc0-9a44-736aad31aff3` 在限定窗口内超时，未计为独立通过；父代理完成五轴质量、
简化和生命周期复核。未修改嵌入式 C/C++；`embedded-enterprise-workflow` 与
`embedded-code-review-simplifier` 对本轮源码不适用；public vendor applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.127`；47,947,582 bytes；SHA-256
`85E4B0695D12DD7287E142E0E8E7E1B08159A66363E69DB82AF14AD21D75ED5A`；archive listing SHA-256
`2B6EBCDFF66E67A5689B41EA3696F109EC0DE38D6B7341F311B4708DC6E85936`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 退出；当前根目录仍为旧包 47,931,714 bytes，SHA-256
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`；签名 `NotSigned`，
`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.126 响应式工作区 viewport 边界

`presentation/controllers/workspace.py` 将工作区 `QTabWidget` 的硬最小高度从 220px 收敛为可让位
的 0px，保留最大高度 350px；980×680 下实际 `workspaceShell=144px`、`tabs=109px`、固定
`workspaceRouteStrip=31px`，因此 tab 不再伸出父级并覆盖路线条。页面原有 `QScrollArea` 继续承载
剩余纵向内容，terminal/live/send、Tab index、焦点、主题和 shared motion lifecycle 不变。

验证：三主题 × 980×680/1180×780 × 四 tab，exact-white=0、当前页 horizontal maximum=0、
`tab_route_overlap=false`、`route_shell_overflow=false`；scripts/check.ps1、compileall、Ruff、
provenance verify 和 onefile package 通过。架构师线程 `019fed57-c98f-7330-a61a-7a69ce726a67`、
独立审查线程 `019fed5e-5086-72c3-9ddb-7c4051d3fd31` 均在限定窗口内超时，未计为独立通过；父代理
完成五轴质量、简化和 assurance 复核。未修改嵌入式 C/C++；public vendor applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.126`；47,943,352 bytes；SHA-256
`5C5F093587686FF2A56C409CC3D1F9A84823ECF1F1322D6F83EB7D4692133563`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`；provenance pass。根目录覆盖
仍等待 PID 46108、49236 释放；根目录旧包 SHA-256 为
`971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7`。

## 2026-08-11 UI-1.125 链路连接状态轨道终态 marker

`ConnectionStatusRail` 继续只消费 lifecycle 已提供的 state 与 shared `(phase, animated)`。`open`
在末节点绘制 success 完成勾，`error` 在首节点绘制 error 叉号；opening/closing/open 的既有彗尾
与脉冲保持不变，closed/discovered 使用原有静态节点路径，`stop()` 只冻结动效，不改变状态。

验证：三主题真实 production host（`QWidget#appRoot` + `QFrame#connectionControlBand`）六状态、
opening/open/closing 动态帧、stop 静态回退、exact-white=0 和截图通过。首轮独立 rail 抓图误用
普通 QWidget，只截取透明子控件而呈现白色背景；已定位为验证宿主问题，修正生产父级后重新通过，
未修改源码以迎合错误探针。`scripts/check.ps1`、compileall、Ruff、package、provenance verify
通过；架构师线程 `019fed4f-d2a8-7d33-9cdd-b536efe3e106` 超时，未计为独立通过；父代理完成
owner、token、绘制、accessibility、性能与简化审查。未修改嵌入式 C/C++；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.125`；47,942,017 bytes；SHA-256
`ACC5FCA833E17EF0D648FCBE7CB7DB1ECB8557B43BD4C920371EF2CCDEF49142`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.124 连接快速配置上下文主题 surface

`ConnectionPresetContextSurface` 继续只消费既有 `ConnectionPreset | None` projection。空态映射为
`source=none/state=empty`，内置配置映射为 `source=builtin/state=selected`，自定义配置映射为
`source=custom/state=selected`；`set_preset()` 通过 `refresh_dynamic_property()` 让 source/state
切换立刻触发 Qt re-polish，不创建连接动作或新的状态源。

自绘 panel/border 与 base/variant stylesheet 统一复用已有 ThemeSpec：空态 neutral、内置 info/blue、
自定义 history/purple；提示文本仍明确“只填入表单，不会自动连接”，标题、tooltip、AccessibleDescription、
NoFocus、鼠标透明、shared frame/stop 和 controller owner 不变。三主题空态/内置/自定义离屏 vector
通过，采样中心像素分别为非白色 token，截图已人工查看。

验证：`scripts/check.ps1`、compileall、Ruff、三主题 UI124 vector、package 和 provenance verify
通过；生产入口/真实 GUI、EXE startup/close、硬件验收仍未运行。架构师线程
`019fed48-9164-72c3-91f3-7472ab0cf517` 在限定窗口内超时，未计为独立通过；父代理完成 owner、token、
Qt property polish、accessibility、性能与行为保持、简化审查。未修改嵌入式 C/C++；embedded
applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.124`；47,941,349 bytes；SHA-256
`79A366634CF0B87C629416194AC8891F7BFE6BB08CB680F3DE050D9A99D3E3BC`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.123 Component 空态 gate affordance

`ComponentEmptyStateSurface` 继续只消费 `set_action_enabled()` 的既有 presentation gate；enabled
投影为 `waiting/COMPONENT / WAITING`，disabled 投影为 `blocked/COMPONENT / BLOCKED`。blocked 使用
三主题对称的 neutral surface/warning border，CTA 隐藏、glyph 静态降级；waiting 保留蓝色边界、
加载 CTA 和共享 frame 动效。`setText()/text()`、`load_requested`、controller、pipeline、焦点和
无障碍契约未改变。

验证：`scripts/check.ps1`、compileall、Ruff、provenance verify 通过；生产入口字体为
`Microsoft YaHei UI`，三主题两态离屏图、state property、眉题、CTA visible/enabled、NoFocus、
鼠标透明、AccessibleDescription 断言通过，截图已人工查看。向量脚本首轮因误用不存在的公开
`action_button` 成员失败，已定位为验证脚本问题并改用现有内部 presentation 成员重新通过；未修改源码
API。架构师线程 `019fed42-2796-7fb2-9fe8-0e09a22026c8` 在限定窗口内超时，未计为独立通过；父代理
完成 owner、token、Qt property polish、accessibility、简化和生命周期审查。未修改嵌入式 C/C++；
embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.123`；47,939,601 bytes；SHA-256
`BC3D81E7714111A63B18DC9679B34A6F7D874830BEF4537F56116F07ED5B36D1`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.122 分析状态 marker

`AnalysisStatusLabel` 保留原生 QLabel 文字、QSS、无障碍和底部五节点 rail，只在既有 `state/source`
property 上增加左侧 semantic marker；active/waiting/draft 复用 shared frame 绘制低对比度 pulse，
history 继续使用 purple source 语义，error/blocked 保持原有静态 marker。五个 controller、
`StatusSurfaceController`、统计/解析/回放事实来源和 lifecycle fan-out 不变；base QSS 左 padding
调整为 `22px`，三主题无白色回退。

验证：`scripts/check.ps1`、compileall、Ruff、provenance verify 通过；生产入口字体为
`Microsoft YaHei UI`，真实组合根 `980x680/1180x780`、三主题 protocol page 的 horizontal range
均为 `0`，五类 status width 均不低于 minimum size hint；孤立 active/waiting/history/error 图和
真实组合根截图已人工查看。架构师线程 `019fed3c-101d-7873-bef8-bde944acc6b8` 在限定窗口内超时，
未计为独立通过；父代理完成 owner、token、Qt 绘制、accessibility、简化和生命周期审查。未修改
嵌入式 C/C++；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.122`；47,941,295 bytes；SHA-256
`E881FA27DD8B0564C195A95A2B94A1CF521772438AA18A6EEAD1A6BF0957FB55`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.121 批量命令状态 affordance

`CommandBatchSurfaceLabel` 保留既有文字和步骤 rail，只在已有 projection 上增加左侧语义状态点；
`running` 状态在共享帧可用时增加低对比度呼吸环。`controllers/commands.py` 继续是 snapshot →
projection 的唯一 owner，`lifecycle.py` 继续 fan-out frame/stop，未新增 timer、业务状态、设备
进度、ACK、线程、I/O 或后端依赖。QSS 左 padding 调整为 `28px`，避免 marker 与中文文案重叠，三主题
继续使用 semantic token 且无白色回退。

验证：`scripts/check.ps1`、compileall、Ruff、provenance verify 通过；生产入口字体为
`Microsoft YaHei UI`，三主题离屏渲染尺寸 `720x120`，label `688x88`，状态 projection、NoFocus、
鼠标透明和主题背景检查通过，截图已人工查看。架构师线程 `019fed34-6372-7050-ab70-437f34378271`
在限定窗口内超时，未计为独立通过；父代理完成 owner、token、Qt 绘制、accessibility、简化和生命周期
审查。未修改嵌入式 C/C++；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.121`；47,941,026 bytes；SHA-256
`7A9DC10ED43B2B9876B2FCF03F422A70E14765901E616206AF9041940DF2D20F`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.120 扩展工具站能力卡 affordance

`embedded_extension_panel.py` 的 7 张能力卡继续保持静态只读 `QFrame`，只新增 base/variant
stylesheet 的 hover selector：普通卡使用 accent-blue 边界，contract-only 保持 info/blue，
attach-only 保持 history/purple。两列 grid 使用等权 stretch，卡片不进入 Tab 焦点链，不新增
动作、timer、MotionController、状态源或后端依赖。

真实组合根使用生产入口字体初始化，验证 980/1180、三主题、四工作区切换、7 张卡片、vertical
scroll range、horizontal range=0、NoFocus 和三条 hover selector；截图已人工查看。架构师线程在
限定窗口内超时，未计为独立通过；父代理完成 owner/token/accessibility/简化审查。未修改嵌入式
C/C++，embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.120`；47,940,170 bytes；SHA-256
`A5C9FFB974C712D65B7212A252F860972924AF3817455BC5B8432FD8AE8E0C46`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.118 scrollbar signal rail 可见性

现有 QScrollBar 的 vertical/horizontal thumb 复用主题语义色：default 为 `BORDER_STRONG`/
`accent_purple`，hover 为 `ACCENT_PINK`/`accent_pink`，pressed 为 `ACCENT`/`accent`，并保留
surface border、无箭头和 native scroll range。真实组合根在 980/1180、三主题、四 Tab 检查 scroll
range、可见 thumb 和 pressed selector，通过 `UI118_SCROLL_SIGNAL_VECTOR_PASS`；24 张截图已人工查看。

架构师与独立质量审查线程在限定窗口内超时，未计为通过；父代理完成 token/selector/native behavior、
白色回退、对比度与简化审查。未修改嵌入式 C/C++；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.118`；47,938,827 bytes；SHA-256
`F04CC42821DFC6F3BEB3F7DEE143C83B51D22D19025EF22DCD5C736C55C18A3C`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.119 motion fanout 性能实验（全部撤回）

两种候选均已在同一类 offscreen 合成根中实测并撤回：逐帧 `isVisible()` 过滤为 `591.13ms` 对
`678.89ms`；Tab 切换缓存为 `781.29ms` 对 `821.50ms`。生产代码保持 UI-1.118 的全量
frame/stop contract；完整记录见 `docs/PERFORMANCE.md` 和独立 handoff。

## 2026-08-11 UI-1.117 字体运行时 fallback 与中文可读性

新增 `presentation/font_runtime.py`，在 QApplication 创建后优先选用 `Microsoft YaHei UI`、
`Microsoft YaHei`、Noto CJK 或 Segoe UI；当 Qt 字体数据库没有 CJK family 时，才检查标准本机字体
路径并通过 `QFontDatabase.addApplicationFont()` 进行进程内注册。不存在、注册失败或没有可用 family
时无阻断回退到 Qt 默认。字体不复制、不下载、不进入 PyInstaller 发行物。

真实组合根产生 3 themes × 4 tabs 的 1180×780 截图和 980×680 响应式截图；中文正常显示，未见
白色背景带，内部 settings scroll surface 无横向溢出。架构师与独立质量审查线程在限定窗口内超时，
未计为通过；父代理完成 owner 边界、失败回退、重复注册、资源/权限、简化和行数审查。未修改嵌入式
C/C++；embedded applicability=N/A。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision
`local-ui-1.117`；47,940,611 bytes；SHA-256
`A8382CDA77F2601BDD4465468881C0F056EB86FD307090BDC89B02C4C7162333`；archive listing SHA-256
`D9F453CC66303DFEB931B7B7868F0D45848C62D409BD520952D2541D2005BE51`。根目录覆盖仍等待 PID
46108、49236 退出；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.116 主窗口 action signal rail 统一

连接配置保存/删除、BLE 读取、协议应用/重置、历史回放开始/停止、错误清除、终端清空、发送、快捷命令保存、发送历史清除和批量新建/编辑/删除/执行共 16 个普通动作统一复用 `ActionRailButton`。`BusyActionButton` 的连接、刷新、扫描、记录、回放暂停和批量停止 busy contract 保持不变。

真实组合根三主题 vector 验证 16 个 widget 都是原生 `QPushButton` 子类，所有 widget 都进入既有 lifecycle frame/stop fan-out，`stop()` 可冻结；1180x780 命令管理页视觉审计通过。回调、objectName、enabled gate、焦点/Tab、payload、连接/发送/记录/回放/批量语义不变。

架构师与独立质量审查线程在限定窗口内超时，未计为通过；父代理完成 owner 边界、行为保持、复用/简化、生命周期和行数审查。未修改嵌入式 C/C++；embedded applicability=N/A，不声明 MISRA、ISO 26262、硬件或安全认证合规。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision `local-ui-1.116`；47,937,619 bytes；SHA-256 `D80631FE5B2F9DCF213ADCAB16083BBAC7BAEA49EDB1CC39A683F7BFCE494448`；archive listing SHA-256 `D11AE67CCE33F851AC4DA36450BFAADF5247F1FF5776A428677A2F27E662E49A`。根目录覆盖仍等待 PID 46108、49236 退出，未强制终止用户进程；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.115 工作区 Tab signal underline 与 halo

新增 `presentation/workspace_tab_surface.py:AnimatedWorkspaceTabBar`，在原生 `QTabBar` 绘制完成后，为当前选中 Tab 叠加主题化 signal underline/halo。Tab 的 currentChanged、键盘/焦点、icon、滚动按钮和 QSS 仍由 Qt 原生控件负责；workspace builder 只显式注入 TabBar，lifecycle 只复用已有 shared frame/stop fan-out。

真实组合根三主题/四 Tab vector 和离屏视觉审计通过；未新增 timer、业务状态、MotionController、registry、线程、I/O 或跨层回调。架构师与独立质量审查线程在限定窗口内超时，未计为通过；父代理完成 owner 边界、行为保持、复用/简化、绘制边界和行数审查。未修改嵌入式 C/C++；embedded applicability=N/A，不声明 MISRA、ISO 26262、硬件或安全认证合规。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision `local-ui-1.115`；47,937,677 bytes；SHA-256 `8B10ABABAB674833AE4FCA663D9ACD81AF59225F53C046F11BC130410249B786`；archive listing SHA-256 `D11AE67CCE33F851AC4DA36450BFAADF5247F1FF5776A428677A2F27E662E49A`。根目录覆盖仍等待 PID 46108、49236 退出，未强制终止用户进程；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.114 派生动作 signal rail 与 lifecycle fan-out

四个既有协议/组件/Dataset 派生动作复用 `ActionRailButton` 的主题化 signal rail，并加入既有 lifecycle `_motion_surfaces()` 的统一 frame/stop fan-out。按钮仍由 `protocol.py` 创建，动作回调与无障碍语义保持不变；`protocol_config.py` 仍是 enabled/disabled gate 的唯一副作用 owner。没有新增 timer、状态源、registry、DTO、线程、I/O 或业务判断。

三套主题下，真实 offscreen protocol panel vector 已验证四个按钮均为原生 `QPushButton` 子类；enabled/disabled tooltip 与 accessible description 同步，`set_frame()` 可绘制，lifecycle 可收集，`stop()` 可冻结。静态门禁、compileall、Ruff 和 provenance 通过。

架构师角色和独立 UI 复核均在限定等待窗口内超时，超时不计为通过；父代理依据既有 `ActionRailButton`、`protocol_config` gate 与 lifecycle contract 完成行为保持、复用/简化、owner 边界和源码行数审查。未修改嵌入式 C/C++；embedded assurance applicability=N/A，不声明 MISRA、ISO 26262、硬件或安全认证合规。

canonical onefile：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`；source revision `local-ui-1.114`；47,934,511 bytes；SHA-256 `7608148F5BE4E3F30F82379C78A94B749320E30809A2B32A5B7ECBEF8CBC14C6`；archive listing SHA-256 `5137DE350B55E756B354569A7ADABD60F5C069FAC0074A37A25E20B5DD78A8A2`。根目录覆盖仍等待 PID 46108、49236 退出，未强制终止用户进程；签名 `NotSigned`，`release_eligible=false`，硬件验收 `not_run`。

## 2026-08-11 UI-1.113 协议与派生数据动作 affordance

四个既有派生数据动作补齐 owner-local 的动作说明与禁用原因；不新增按钮、状态、signal、timer、依赖、I/O 或业务 DTO。三主题 enabled/disabled offscreen vector、静态检查、compileall、ruff 和 provenance 通过。

运行时尝试调用架构师、六角色前置评审和独立复核，但协作线程持续返回 agent thread limit reached；独立复核不计为通过。父代理完成五轴审查并保留该限制记录。未修改嵌入式 C/C++。

canonical onefile：dist/release/0.1.0/core/onefile/app/SerialForge.exe；source revision local-ui-1.113；47,934,870 bytes；SHA-256 3CBA66937B7006831920BBD395604EA7B838E70E5600B64A22B2D01E432278AA。根目录仍被 PID 46108、49236 占用。

## 2026-08-11 UI-1.110～UI-1.112 动作与字段 affordance 完整化

本轮将回放、连接、录制、UART 字段、发送区、批量编辑器、preset/确认对话框和快捷命令 QAction 的提示收敛到各自 presentation owner；动态状态同步 tooltip 与 accessible description，0 秒超时和 CRLF payload 保留既有语义。三套主题、按钮/selector/dialog offscreen vector、近白像素、关闭生命周期、静态检查、compileall、ruff 和 provenance 均通过。

独立复核代理在限定窗口内超时，未计为通过；父代理完成五轴审查、架构边界和行为保持的简化评估。未修改嵌入式 C/C++，不声明硬件或安全认证合规。详细记录见 docs/handoffs/2026-08-11-ui-1-110-112-affordance-completion.md 与 ADR-0100。

canonical onefile：dist/release/0.1.0/core/onefile/app/SerialForge.exe；source revision local-ui-1.112；47,932,571 bytes；SHA-256 EB975C422B68D618C6B0E09669D8843689752C0153E540193F213B664FE0254E。根目录旧实例由 PID 46108、49236 占用，root-exe=pending-user-close；未强制终止进程。

## 2026-08-11 UI-1.109 Action Affordance Contract

### 结果与边界

- 清除错误、清空终端预览、清除发送历史、快捷命令菜单、保存快捷命令、保存/删除自定义连接均补齐 tooltip 与 accessible description。
- UART 读超时、写超时补齐秒单位、连接后生效与等待范围说明；既有范围、默认值、runtime callback、DTO、状态源和动作语义不变。
- 不新增 signal、timer、线程、依赖、设备 I/O 或跨层 helper；连接/发送/原始记录、OTA/debug contract-only/attach-only 边界保持独立。
- 两轮六角色前置评审和最终独立复核均在限定等待窗口内超时并关闭；超时不视为通过。父代理完成五轴审查、架构边界、复用/简化评估和生命周期验证。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run --locked --extra dev python -m compileall -q src scripts  PASS
uv run --locked --extra dev ruff check src scripts              PASS
UI109_ACTION_AFFORDANCE_VECTOR_PASS                   actions=9 combos=27 editable=('UART 端口',)
                                                       themes=3 responsive_sizes=2 near_white=0 close_lifecycle=pass
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

首轮组合根断言定位到保存/删除自定义连接的 accessible description 缺失，随后仅在 `connection_builder.py` owner 内补齐；未改变 callback、状态或业务边界。未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收；仅有 PySide6 环境字体目录提示，不影响断言。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.109
size: 47,931,714 bytes
SHA-256: 971FA1DB0A08D7C40A423869486CA00BF54823168E04C9DC89C437F1CBA7BBF7
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

本轮 Python/Qt presentation 代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规；embedded applicability、independent review、behavior-preserving simplification assessment 与 authorized non-destructive verification 已记录，未修改嵌入式 C/C++。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.108 Selector Affordance Contract

### 结果与边界

- 真实组合根的 27 个 `QComboBox` 中，只有 UART 端口保持可编辑；transport、终端显示/发送、历史、批量和编辑器选择器均显式不可编辑。
- 所有选择器均提供 accessible name、tooltip、accessible description；文案明确不会自动连接、发送或执行。
- itemData、signal、连接 gate、批量执行、主题表面、焦点顺序和 OTA/debug 边界保持不变；未新增 timer、DTO、状态源、依赖或跨层 helper。
- 六角色前置评审两轮及独立复核均超时并关闭；超时不视为通过。父代理完成五轴审查、复用/简化评估和生命周期验证。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI108_SELECTOR_AFFORDANCE_VECTOR_PASS                   combos=27 editable=('UART 端口',) hints=all themes=3 responsive_sizes=2 near_white=0 close_lifecycle=pass
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收；本轮 Python/Qt presentation 代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.108
size: 47,930,586 bytes
SHA-256: 27EBF48272AACF3E5CCC3586ABFC8988272A737E80A6D24D57FD001A8B9E6FA4
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.107 Connection Rail Comet

### 结果与边界

- `ConnectionStatusRail` 在既有动态光点后增加三枚渐隐彗尾，并增加外环与中心光点；颜色继续来自 `ThemeSpec` 状态语义色。
- 只消费既有 `_phase`、`_animated`、`_state`、节点位置和共享生命周期；没有新增 timer、业务状态、公开 API、资源、线程、I/O 或 transport/OTA/debug 依赖。
- `opening`、`open`、`closing` 动态；`discovered`、`closed`、`error` 静态；停止、低动效、隐藏、最小化和关闭回退保持稳定。
- 六个前置角色和独立复核均超时并关闭；超时不视为通过。父代理完成五轴审查、复用/简化评估和 embedded applicability 记录。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI107_CONNECTION_RAIL_VECTOR_PASS                       themes=3 states=6 animated_states=3 comet_trail=checked stop=checked near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收；本轮 Python/Qt presentation 代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.107
size: 47,929,693 bytes
SHA-256: 6785E9F51A9E907A647E2B84A050EE54F93B1925C91B3581DDEEABFC3292434D
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.106 Theme Picker Surface

### 结果与边界

- `QComboBox#themePicker` 获得 history→input 渐变、强调字重以及 hover/focus/disabled semantic 状态，主题切换入口更易发现。
- 三个主题 key、icon、itemData、不可编辑、主题 signal、一次性 transition、palette swatch 和 runtime 行为均不变。
- 六个前置角色和独立复核均超时并关闭；超时不视为通过。父代理完成五轴审查与复用/简化评估，没有新增状态源、timer、依赖或动作。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI106_THEME_PICKER_VECTOR_PASS                          themes=3 keys=3 icons=3 editable=0 theme_signal=preserved disabled=checked near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收；本轮 Python/Qt presentation
代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.106
size: 47,929,814 bytes
SHA-256: 82668145CA2F8AA903E3D2BD2A02D8AC4B3B00AAE2DCAD87C9D6F6B42711C2C5
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.105 Transport Selector Surface

### 结果与边界

- `QComboBox#transportCombo` 获得 info→input 渐变、强调字重以及 hover/focus/disabled semantic 状态，六种传输选项更易扫描。
- `TransportKind` itemData、不可编辑、accessible name、连接 signal、TransportModeSurface glyph、runtime 和生命周期均不变。
- 六个前置角色和独立复核均超时并关闭；超时不视为通过。父代理完成五轴审查与复用/简化评估，没有新增状态源、timer、依赖或动作。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI105_TRANSPORT_SELECTOR_VECTOR_PASS                    themes=3 modes=6 editable=0 item_data=preserved disabled=checked near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收；本轮 Python/Qt presentation
代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.105
size: 47,929,551 bytes
SHA-256: D4B68CBE1D4BB00C4C8F35C4BDC8ED9E4B79BE5EC782C865EE39B31D37A1A7B1
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.104 StatusBar Chrome Surfaces

### 结果与边界

- base/variant controls theme 对称覆盖 `QStatusBar::item` 与 `QStatusBar QLabel`，消除 Qt 原生 item 的透明/白色边界回退。
- `StatusFooterSurface`、native status text、accessibility、状态源和 lifecycle owner 不变；没有新增状态源、timer、资源、依赖或业务动作。
- 六个前置角色和独立复核均超时并关闭；超时不视为通过。父代理完成五轴审查与复用/简化评估。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI104_STATUSBAR_CHROME_VECTOR_PASS                      themes=3 selectors=2 status_footer=116x18 near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收；本轮 Python/Qt presentation
代码不适用 embedded vendor public source，不声明 MISRA/ISO/硬件合规。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.104
size: 47,928,792 bytes
SHA-256: 97E43617E1F0B148F713A6049C5338DD3FBF94AE8CE7ACFE9B0F127BCBB8C087
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.103 Workspace Shell Surfaces

### 结果与边界

- `QFrame#workspaceShell` 获得稳定的 surface、border 和圆角，`QFrame#workspaceRouteStrip` 获得 history→info 渐变、上下边界和底部圆角。
- base stylesheet 与 theme variant controls selector 对称，均只复用既有 `ThemeSpec` token；真实 Tab、route beacon、index、焦点、无障碍和共享动效不变。
- 六个前置角色和独立复核均在运行时超时并已关闭；超时不视为通过。父代理完成五轴审查与简化评估，确认没有新增状态源、timer、资源、依赖或业务耦合。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
uv run python -m compileall -q src scripts              PASS
uv run ruff check src scripts                           PASS
UI103_WORKSPACE_SURFACE_VECTOR_PASS                    themes=3 selectors=2 tabs=4 route=148x28 near_white_pixels=0
uv run --locked --extra dev python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

离屏环境仅有 PySide6 fonts 目录 warning；未运行可见 GUI/EXE startup、硬件、真实 UART/网络/BLE/RTT/J-Link、OTA、签名或正式发行验收。
本轮 Python/Qt presentation 代码不适用 embedded vendor public source；不声明 MISRA/ISO/硬件合规。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical; overwritten)
source revision: local-ui-1.103
size: 47,928,961 bytes
SHA-256: 4F0300C4764F94F25825C8B5795D348BD04D1AFF8A43E6078B7676FA909B70D1
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.102 Extension Card Surfaces

### 结果与边界

- 为 `QFrame#extensionStationOverview` 增加接入概览的 info surface、accent 左色带和状态牌样式；为 `QLabel#extensionStationOverviewValue` 增加 metric 层级。
- 为 `QFrame#extensionCapabilityCard` 增加基础卡片表面，并按既有 `state="contract_only"` / `state="attach_only"` 使用 info/history semantic surface 与蓝/紫左色带，7 张卡片不再回落透明或系统原生外观。
- base stylesheet 与 theme variant shell 保持 selector 对称；只消费已有 `ThemeSpec` token，不增加 token、DTO 字段、状态源、动作、timer、资源或依赖。

### 角色与独立复核

```text
产品角色       019fec98-e2a0-71f0-9cd6-bcfef160c94d  called; wait timed out; closed
架构角色       019fec98-e2eb-7892-97e5-f10ae3e69c04  called; wait timed out; closed
UI 设计角色    019fec98-e336-7523-9080-9885b562b9c6  called; wait timed out; closed
开发角色       019fec98-e382-78e0-a0ab-76ef24853141  called; wait timed out; closed
验证角色       019fec98-e3d1-7a92-bd39-a93685c7bba9  called; wait timed out; closed
打包角色       019fec98-e41e-7dd2-979e-b79efb9d8cf7  called; wait timed out; closed
修正前置六角色另行调用：019fec9a-44fd-76b1-a97f-cd476ee2444f / 019fec9a-4550-7083-88a2-890fd5630136 / 019fec9a-459c-7c43-85ec-1784857468c9 / 019fec9a-45eb-7fc1-9215-79ea6e580f8b / 019fec9a-463c-71d0-88e7-7c4701540907 / 019fec9a-468a-7980-80d3-12f5f5049b65  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec9a-f3f2-7040-b9d1-7c6f69dfb102  called after implementation; wait timed out; closed
```

所有角色与独立复核均未返回完整报告，超时不视为通过。首轮门禁发现变体 QSS 错用了不存在的 `ThemeSpec.border_strong` 字段；父代理停止扩展、定位到 token contract 后，经第二轮六角色复核改为已有 `theme.border` 并重新通过全套门禁。父代理完成 correctness/readability/architecture/security/performance 五轴复核，简化评估为复用既有 semantic token 和动态 state，不新增 token 或通用卡片框架。

Embedded R&D 记录：本轮只有 Python/Qt QSS presentation 代码，没有嵌入式 C/C++、MCU、SDK、RTOS、bootloader、Flash、真实 OTA 或 debug target 修改；vendor public source applicability=N/A，未声明厂商要求、MISRA/ISO 认证或硬件合规。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
.venv\Scripts\python.exe -m compileall -q src        PASS
.venv\Scripts\python.exe -m ruff check src           PASS
UI102_EXTENSION_SURFACE_VECTOR_PASS themes=3 cards=7 contract_only=5 attach_only=2 overview=1 renders=24 near_white_pixels=0
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

向量使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未显示主窗口；仅有 PySide6 缺少 fonts 目录 warning，不影响断言。未运行可见 Windows GUI/HIDPI/读屏/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、EXE 启动、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.102
size: 47,928,081 bytes
SHA-256: 717F20E69CFA0120A1DB833509FEEBCDB37AA66199559A6273B75FEF3E1DD9F1
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.101 SignalField Anime Accent

### 结果与边界

- `SignalFieldWidget` 保持现有 148×34、NoFocus、鼠标透明、空 accessibility 和 `set_frame()/stop()` API，仅在内部增加三枚资源无关的四点星芒、移动光点彗尾和既有轨道层次。
- 星芒、光点和彗尾全部复用 `ThemeSpec` 的 accent/pink/blue/purple token；动态位置只消费共享 `MotionController` phase，静态帧使用固定位置和低透明度。
- 没有新增 QTimer、线程、业务状态、图片/字体/网络资源、controller callback 或应用层依赖；隐藏、最小化、关闭、暂停和 reduced-motion 仍由既有 lifecycle fan-out 调用 `stop()`/静态帧。

### 角色与独立复核

```text
产品角色       019fec92-a043-7951-bca3-f7f1c867997f  called; wait timed out; closed
架构角色       019fec92-a08d-7121-aad4-bcdb19bab818  called; wait timed out; closed
UI 设计角色    019fec92-a0dc-77a2-aab7-3dfc668a8954  called; wait timed out; closed
开发角色       019fec92-a129-78b0-814e-b633984194ca  called; wait timed out; closed
验证角色       019fec92-a179-7db3-b8b3-420dd6dcdcdb  called; wait timed out; closed
打包角色       019fec92-a1c3-7692-9084-62a42b6a8ae6  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec93-79cc-77b2-88bc-69e9bae609a9  called after implementation; wait timed out; closed
```

所有角色/独立复核均因运行时超时而未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴复核：确认绘制逻辑留在既有 presentation widget、主题 token 有界、QPainter 状态在每段绘制前显式设置、无障碍与生命周期 API 不变。简化评估为复用现有 SignalField 与 MotionController，不新增通用装饰框架或第二套动画时钟。

Embedded R&D 记录：本轮只有 Python/Qt presentation 代码，没有嵌入式 C/C++、MCU、SDK、RTOS、bootloader、Flash、真实 OTA 或 debug target 修改；vendor public source applicability=N/A，未声明厂商要求、MISRA/ISO 认证或硬件合规。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
.venv\Scripts\python.exe -m compileall -q src        PASS
.venv\Scripts\python.exe -m ruff check src           PASS
UI101_SIGNAL_FIELD_VECTOR_PASS themes=3 frames=2 renders=6 near_white_pixels=0 shared_motion_surface=1 stop=1
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

向量使用真实 `create_application()` / `create_main_window()` 与 Qt offscreen，未显示主窗口；第一次内联向量因验证脚本错误地从控件实例读取 `WidgetAttribute` 而失败，修正验证脚本后复跑通过，产品代码未因此修改。环境只报告 PySide6 缺少 fonts 目录 warning，不影响断言。未运行可见 Windows GUI/HIDPI/读屏/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、EXE 启动、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.101
size: 47,928,874 bytes
SHA-256: C8A3B9581503479EAFCE01C7675C2F1BDF0D9250CFF45C13ACF4C45B0AC700E7
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.100 Pipeline State Motion

### 结果与边界

- `PipelineSurfaceLabel` 继续保持 presentation-only：不读取 view model、domain 或 transport，仅消费生命周期已经写入的 `state/source` 动态属性、主题 `ThemeSpec` 和共享 `MotionController` frame。
- `active`、`transition`、`draft`、`history`、`blocked`、`idle` 六种既有状态现在分别投影到节点数量/颜色；历史来源使用历史色，阻塞状态显示静态叉标，未完成节点降低透明度，流水线摘要不再所有状态都使用同一套全量彩色节点。
- 只有 `active`、`transition`、`draft` 继续使用共享 frame 脉冲；`idle`、`history`、`blocked` 保持静态，`stop()`、隐藏/最小化、关闭和 reduced-motion 的既有冻结路径无需新增 timer 或 controller。
- `state/source` 非法值 fail-open 到 `idle/live`；QPainter 只在已有尺寸安全条件下绘制，颜色全部来自三主题 semantic token，不新增白色 literal、资源、依赖或业务状态源。

### 角色与独立复核

```text
产品角色       019fec89-84a6-7183-bfb2-5d110ae0ebbf  called; wait timed out; closed
架构角色       019fec89-84f1-7052-87ca-612d94f8df52  called; wait timed out; closed
UI 设计角色    019fec89-8540-7a71-9d94-bfe749371d5f  called; wait timed out; closed
开发角色       019fec89-858a-7df3-8bc4-88c9cae1298f  called; wait timed out; closed
验证角色       019fec89-85db-7532-b593-da91d1b86165  called; wait timed out; closed
打包角色       019fec89-8622-7fc3-8d94-73b4a56f07f2  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec8b-ccf5-7de1-ae98-ed04169b6195  called after implementation; wait timed out; closed
```

角色与独立复核未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查：确认绘制只依赖已存在的 presentation properties 和共享帧，颜色/节点映射有界，QPainter 生命周期安全，既有文本与 accessibility 不变，且无新增常驻 timer、线程、I/O、外部依赖。简化评估为复用 `refresh_dynamic_property`、`ThemeSpec`、`MotionController` 与既有 `_motion_surfaces()`，没有新增状态源或行为性删减。

Embedded R&D 记录：本轮没有嵌入式 C/C++、MCU、SDK、RTOS、bootloader、Flash、真实 OTA 或 debug target 修改；vendor public source applicability=N/A，未声明任何厂商要求、MISRA/ISO 认证或硬件合规。

### 验证

```text
scripts/check.ps1                                      PASS (154 files <=1000; 3 themes; 22 tokens; 19 selectors; legacy_qss_literals=0)
.venv\Scripts\python.exe -m compileall -q src        PASS
.venv\Scripts\python.exe -m ruff check src           PASS
UI100_PIPELINE_STATE_VECTOR_PASS themes=3 states=6 sources=2 renders=36 moving=3 fallback=1 stop=1
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

向量使用真实 `create_application()` / `create_main_window()` 与 `QApplication`/Qt offscreen，未显示主窗口、未接触硬件或网络；只出现 PySide6 环境缺少 fonts 目录的 warning，不影响断言。未运行 Windows 可见 GUI/HIDPI/读屏/视觉差分、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、EXE 启动、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.100
size: 47,925,850 bytes
SHA-256: 7038F0C57040B6B518A051E1B779D1F58F01DF0B452A8991C54586F62D33BCA5
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.99 Protocol Options and Dynamic Copy

### 结果与边界

- 协议 preset、帧格式、校验、最大帧、分隔符 Hex、长度前缀、字节序和回放速度改为更清晰的中文 bounded selector/辅助说明；下拉均不可手动输入。
- `ProtocolPreset`、`FramingKind`、`ChecksumKind`、长度字节 int、字节序 string、回放速度 float 等底层 `itemData` 保持不变；应用、重置、解析 gate、专用帧动态启停不变。
- Component/Dataset/Curve 的 status、空态、流水线摘要、曲线可访问描述和 Dataset 动态刷新路径统一使用“序列/样本/原始终端”等用户文案，消除了动态刷新后重新出现的英文 fallback。
- 本轮没有新增协议能力、设备 I/O、业务状态源、timer、依赖或 OTA/debug backend；既有 `DatasetCurveWidget` 动效和 lifecycle owner 保持不变。

### 角色与独立复核

```text
产品角色       019fec7e-0688-76a3-8fe0-3408724f14f2  called; wait timed out; closed
架构角色       019fec7e-06d0-74f0-a713-f9408676035c  called; wait timed out; closed
UI 设计角色    019fec7e-071f-72b3-878f-272d5e19b55c  called; wait timed out; closed
开发角色       019fec7e-076c-7872-8a60-bccb988c5c2c  called; wait timed out; closed
验证角色       019fec7e-07b8-7a50-92f4-8baf61525d61  called; wait timed out; closed
打包角色       019fec7e-0803-7ad1-851d-1c163441998f  called; wait timed out; closed
产品角色（动态刷新复核） 019fec81-4914-7040-abf0-470d9cb65183  called; wait timed out; closed
架构角色（动态刷新复核） 019fec81-4964-7e01-9ee6-c0a448f22bc5  called; wait timed out; closed
UI 设计角色（动态刷新复核） 019fec81-49ac-7d53-b8c2-8078474f9761  called; wait timed out; closed
开发角色（动态刷新复核） 019fec81-4a00-77e0-b93b-9e3d7e969e8b  called; wait timed out; closed
验证角色（动态刷新复核） 019fec81-4a4a-74e2-b9ab-5d505f16a881  called; wait timed out; closed
打包角色（动态刷新复核） 019fec81-4a96-72a1-8b0a-17ea53c3b561  called; wait timed out; closed
独立质量复核 / embedded simplifier 019fec84-778b-72f3-a66f-5844c72a4cf7  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查；简化评估为复用既有 bounded combo、currentText projection 与 DatasetCurveWidget owner，未做行为性删减。Embedded applicability=N/A：没有嵌入式 C/C++、MCU、SDK、RTOS 或真实 OTA/debug target 修改，也没有适用的 vendor public source constraint。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI199_PROTOCOL_OPTIONS_VECTOR_PASS themes=3 preset_labels=8 framing_labels=6 checksum_labels=5 typed_data=1 dynamic_gates=1 dataset_refresh=1 replay_affordance=1
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

真实组合根通过 Qt offscreen 内存向量，未显示主窗口；仅出现 PySide6 环境缺少 fonts 目录的 Qt warning，不影响断言；未运行真实协议设备、GUI/EXE 启动、HIDPI、读屏、硬件、签名或正式发行验收。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.99
size: 47,925,745 bytes
SHA-256: 5DA3C31E0009294AAC921427A94F01BCA578C6A0AD58E2971C1D8193D319F0C4
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded

## 2026-08-11 UI-1.98 Connection Options Affordance

### 结果与边界

- 网络主机/端口、本地绑定、连接/读写超时、UDP 报文上限补齐 tooltip/accessibility description，保留既有类型范围与默认值。
- RTT 通道改为 `0 · 终端` / `1 · 数据`，BLE 写入模式改为 `写入 · 等待响应` / `写入 · 不等待响应`，并显式不可编辑；底层 `itemData` 未改变。
- TCP Server allowlist、LAN 确认、最大客户端、发送目标，以及 BLE 扫描/过滤/设备/缓存/配对/特征/通知控件补齐上下文提示。
- 超时控件的零值特殊状态由英文 `None` 改为“未设置”，zero-as-unbounded 语义保持不变。
- 本轮没有新增 OTA/debug backend、网络探测、vendor 工具启动、连接动作、状态源、timer 或依赖；`main.py`/`MainWindow` 仍保持 composition/lifecycle shell 边界。

### 角色与独立复核

```text
产品角色       019fec75-bdf2-7c70-b198-1605e01aad20  called; wait timed out; closed
架构角色       019fec75-be38-76b3-a53b-3fcec46cdf89  called; wait timed out; closed
UI 设计角色    019fec75-be84-75b1-8d74-276dacb9f6bc  called; wait timed out; closed
开发角色       019fec75-bed3-78a1-8f8c-1dd9751abad1  called; wait timed out; closed
验证角色       019fec75-bf27-7e30-a1dd-ac98480a640c  called; wait timed out; closed
打包角色       019fec75-bf72-73e0-a49b-e61257489863  called; wait timed out; closed
独立质量复核   019fec79-110e-7c22-8664-c0c0395430b8  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查；嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI198_CONNECTION_OPTIONS_VECTOR_PASS themes=3 rtt_localized=1 ble_write_localized=1 typed_data=1 none_localized=1 affordances=16
python scripts/provenance.py verify --manifest dist/release/0.1.0/core/onefile/PROVENANCE.json  PASS
```

真实组合根通过 Qt offscreen 内存向量，未显示主窗口；Qt 报告 PySide6 环境缺少 fonts 目录的 warning，不影响断言；未运行真实串口、网络、BLE、HIDPI、读屏、硬件、EXE 启动、签名或正式发行验收。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.98
size: 47,923,829 bytes
SHA-256: DECC92C2D15BB110FB0977AEAB57A8BDD9E814B36F7037BBAFF0F0E5FBE2D753
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-11 UI-1.97 UART Option Labels

### 结果与边界

- 波特率保留 25 个常用预设并显式不可编辑，继续避免用户手输任意数字。
- 数据位显示 `5 位/6 位/7 位/8 位`；校验显示无/奇/偶/Mark/Space；停止位显示 `1 位/1.5 位/2 位`；流控显示无流控、软件流控和两种硬件流控。
- 所有五个 selector 均补齐 tooltip/accessibility description；itemData、domain enum、快速配置与连接行为保持不变。

### 角色与独立复核

```text
产品角色       019fec6f-4f9a-7322-9ef0-89983bf1a185  called; wait timed out; closed
架构角色       019fec6f-4ffb-7971-9c36-75cb8d6eef32  called; wait timed out; closed
UI 设计角色    019fec6f-5051-78e1-93d6-6a84d9b9823d  called; wait timed out; closed
开发角色       019fec6f-50a4-7ab2-8558-b944892bdf0a  called; wait timed out; closed
验证角色       019fec6f-50f3-7162-8dd5-18b4027ce669  called; wait timed out; closed
打包角色       019fec6f-513e-72f3-80db-a10f64652aae  called; wait timed out; closed
独立质量复核   019fec70-a9d0-7a21-8ef9-729885af926a  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查；嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI197_UART_OPTIONS_VECTOR_PASS themes=3 baud_presets=25 localized=data/parity/stop/flow non_editable=1 typed_data=1
python scripts/provenance.py verify --manifest ...    PASS
```

真实组合根通过 Qt offscreen 内存向量，未显示主窗口；仅出现 PySide6 环境缺失 fonts 目录的 Qt warning，不影响 selector/value 断言；未运行真实串口、HIDPI、读屏、硬件、签名或正式发行验收。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.97
size: 47,920,572 bytes
SHA-256: D4014B1E307BABA9926339CA04FEDCE37F6E7FFD8813D9CBCEEE30AE58AC54BE
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-11 ARCH-7d / UI-1.96 Application Extension Station Summary

### 结果与边界

- 新增 `application/extension_station.py`，提供 frozen/bounded `ExtensionStationSummary` 与唯一派生函数 `extension_station_summary()`。
- `presentation/embedded_station_overview.py` 只接收 summary DTO 并渲染 `7` 个能力槽位、`0` 个已激活后端和“无”当前动作。
- 保持 contract-only / attach-only、只读规划层、无设备探测、无 OTA/J-Link 动作的安全边界；未新增后端、状态源、timer 或依赖。

### 角色与独立复核

```text
产品角色       019fec69-1c74-7a32-9b98-2e6bf4e5bea8  called; wait timed out; closed
架构角色       019fec69-1d19-74e3-912c-6077a0bd49ab  called; wait timed out; closed
UI 设计角色    019fec69-1cc9-7a90-84b0-a2742c32a555  called; wait timed out; closed
开发角色       019fec69-1d62-7200-983c-af024eb515df  called; wait timed out; closed
验证角色       019fec69-1db0-7ed1-b9d6-837d062769c7  called; wait timed out; closed
打包角色       019fec69-1dfe-7d53-8c0e-3150241e1cfe  called; wait timed out; closed
独立质量复核   019fec6a-31ae-7f90-9d76-892bec633b3a  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查；嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
ARCH7D_UI196_SUMMARY_BOUNDARY_VECTOR_PASS themes=3 capabilities=7 active=0 action=无 ui_consumes=immutable-summary
python scripts/provenance.py verify --manifest ...    PASS
```

未显示主窗口；GUI/EXE 启动、HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.96
size: 47,921,659 bytes
SHA-256: 23B00CF14295A6F5AC399913ED1C8E46EDD8EB1262A4EC3EB1BE2A88367BD950
archive listing SHA-256: C0E5DAA9B2DA969D1AF5AE10125A207407E19123EC19EAFB3B06C17C82323AC6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.95 Extension Station Overview

### 结果与边界

- 新增 `presentation/embedded_station_overview.py`，从既有 immutable capability catalog 派生能力槽位数、已激活后端数和当前动作。
- 扩展页顶部明确显示“只读规划层”、`7` 个能力槽位、`0` 个已激活后端和“无”当前动作，并说明真实接入前置条件。
- 只增加 presentation composition；没有新增 OTA/J-Link 后端、设备探测、动作按钮、密钥、vendor SDK、socket、pyserial、crypto 或 timer。

### 角色与独立复核

```text
产品角色       019fec62-f401-7d43-aafe-d8247779ed79  called; wait timed out; closed
架构角色       019fec62-f44f-72c0-ab0d-a2a31bdf64b1  called; wait timed out; closed
UI 设计角色    019fec62-f49f-7f72-aca1-23dd792e1370  called; wait timed out; closed
开发角色       019fec62-f4e7-7c61-a88c-73200f2ce6e3  called; wait timed out; closed
验证角色       019fec62-f536-7fe3-acc9-91ffbf6b094b  called; wait timed out; closed
打包角色       019fec62-f58a-74a2-9f5d-a48c54af8d30  called; wait timed out; closed
独立质量复核   019fec64-ffb9-7481-b01a-a9245c1cea96  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查；嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI195_STATION_OVERVIEW_VECTOR_PASS themes=3 capabilities=7 values=7,0,无 state=readonly history-source
python scripts/provenance.py verify --manifest ...    PASS
```

未显示主窗口；GUI/EXE 启动、HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.95
size: 47,919,227 bytes
SHA-256: DBABFB57AD3D63393D57B1C3281C1A1C16CC0E7C572928ABFD940FFE660E5618
archive listing SHA-256: A11F34E225D750036A4AEE1DF2D77E2AE1B09B1C79F71719DB10FE6512852CA6
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.94 UART Timing Summary Surface

### 结果与边界

- 新增 `presentation/uart_timing_surface.py`，以只读 rail 展示 `baud · data/parity/stop · flow`，并同步 tooltip 与 accessible description。
- `connection_builder.py` 只绑定现有 UART combo 的 `currentIndexChanged`；默认值、手动选择和 ConnectionPreset 填入都会即时刷新。
- 摘要位于 UART panel 内，复用 semantic QSS；不写入 ViewModel/TransportConfig，不改变连接、协议、Tab、业务状态或新增 timer/线程。

### 角色与独立复核

```text
产品角色       019fec5a-2724-7a83-8ab4-4e4c82420642  called; wait timed out; closed
架构角色       019fec5a-2768-77d3-a8a1-ba564ab15ca1  called; wait timed out; closed
UI 设计角色    019fec5a-27b4-7401-8fe3-f97f275173ee  called; wait timed out; closed
开发角色       019fec5a-2803-7031-9881-7c6e0efdff63  called; wait timed out; closed
验证角色       019fec5a-2851-7852-ba14-5b3cceab2a4a  called; wait timed out; closed
打包角色       019fec5a-289e-7280-8bf4-0c8403eb9d7a  called; wait timed out; closed
独立质量复核   019fec5c-b357-70b1-8ae0-1941cd296bd8  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查；嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI194_UART_SUMMARY_VECTOR_PASS default=1 manual=1 preset=1 themes=3
python scripts/provenance.py verify --manifest ...    PASS
```

未显示主窗口；GUI/EXE 启动、HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.94
size: 47,916,119 bytes
SHA-256: E6D317841A8983C81CACED40C9798DB70444016E661418589436BBA844F5328D
archive listing SHA-256: D41985C7165A66BD66752652C63615CA83A29BC83F22CEED3360D0E176A7F09F
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.93 Extension Capability Accessibility Semantics

### 结果与边界

- capability title 不再设置 `role="status"`，避免标题与能力成熟度混淆。
- capability state badge 保留 `role="status"`、`state` 属性和 contract-only/attach-only 三主题样式。
- DTO、卡片布局、无障碍文案、只读边界和 OTA/debug owner 不变；本轮没有嵌入式 C/C++ 变更。

### 角色与独立复核

```text
产品角色       019fec53-6bbb-7b93-bf5b-bd09b52fa51c  called; wait timed out; closed
架构角色       019fec53-6c07-7d93-b99d-c6f2dd2df36f  called; wait timed out; closed
UI 设计角色    019fec53-6c58-7f23-b977-832e98d56a30  called; wait timed out; closed
开发角色       019fec53-6ca1-7180-a17c-d57c0b13678f  called; wait timed out; closed
验证角色       019fec53-6cf6-7291-bb3b-c8c169db5518  called; wait timed out; closed
打包角色       019fec53-6d3e-7130-bb15-21d1343d41b3  called; wait timed out; closed
独立质量复核   019fec54-681b-7b82-b68b-befc10967c5c  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查。嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI193_EXTENSION_SEMANTIC_VECTOR_PASS themes=3 titles=7 title_status_roles=0 state_status_roles=7 states=contract_only,attach_only
```

未显示主窗口；GUI/EXE 启动、HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.93` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致，provenance verify 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.93
size: 47,913,940 bytes
SHA-256: 69D6713DFD819D1569792A82A5870D8B2927A52D831674E01B3EC94DF2A699D3
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.92 Workspace Transition Effect Lifecycle

### 结果与边界

- `workspace_runtime.py` 以 `_release_workspace_effect()` 统一恢复 opacity、验证 Qt 对象并解绑 page graphics effect。
- stop、自然完成、快速切换和低动效回退不再留下旧 effect；Tab 路由、业务状态、焦点、无障碍和动画时长不变。
- 该切片仅修改 presentation runtime，无 OTA/debug、I/O、设备、密钥或嵌入式 C/C++ 变更。

### 角色与独立复核

```text
产品角色       019fec4b-8e28-7c91-8966-e21760978ea5  called; wait timed out; closed
架构角色       019fec4b-8e71-78f2-8615-999d28f4d8fe  called; wait timed out; closed
UI 设计角色    019fec4b-8ed5-72e2-a8f0-e06705a3ff81  called; wait timed out; closed
开发角色       019fec4b-8f1c-7091-9154-6cd917d6d68a  called; wait timed out; closed
验证角色       019fec4b-8f68-7381-a6a8-99b3d65bf02f  called; wait timed out; closed
打包角色       019fec4b-8fb7-7160-a9be-cb60f5f556fd  called; wait timed out; closed
独立质量复核   019fec4d-33a2-73a3-a3a7-f408e893211d  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不视为通过。父代理完成 correctness/readability/architecture/security/performance 五轴审查。嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI192_WORKSPACE_EFFECT_VECTOR_PASS natural=1 stop=1 rapid_switch=1 reduced_motion=1 detached=4
```

未显示主窗口；GUI/EXE 启动、HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.92` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致，provenance verify 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.92
size: 47,915,018 bytes
SHA-256: E983B2B6EC35B1877F4E46A5063181E4E0DF99262B344DF38FCE8E261035D7B7
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.91 Extension Capability Card Hierarchy

### 结果与边界

- `QLabel#extensionCapabilityTitle` 从通用 `role="status"` 胶囊规则中分离，恢复正文标题层级。
- `QLabel#extensionCapabilityState` 独立表达能力成熟度：`contract_only` 使用 info surface/border，`attach_only` 使用 history surface/border 与 purple accent。
- 样式只复用已有 `ThemeSpec` token；不新增 DTO 字段、颜色字面量、业务状态源、动画时钟、交互动作或 OTA/debug 依赖。

### 角色与独立复核

```text
产品角色       019fec44-437e-7612-9b50-e4deda0e40fe  called; wait timed out; closed
架构角色       019fec44-43d0-7a92-bf32-0f423c836506  called; wait timed out; closed
UI 设计角色    019fec44-4422-7e11-bee3-7f9d71188fcc  called; wait timed out; closed
开发角色       019fec44-447d-71b1-b8c6-acaac1be3f85  called; wait timed out; closed
验证角色       019fec44-44c4-7aa0-b1b2-6b47a011f88d  called; wait timed out; closed
打包角色       019fec44-4516-7c30-ae51-7897cdb5658b  called; wait timed out; closed
独立质量复核   019fec44-f13c-7620-9751-d4eb703118c9  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认 7/7 卡片标题和状态 selector 匹配；readability/simplicity 确认只有三条稳定 QSS 规则且无新 token；architecture 确认样式留在 presentation theme owner、DTO 与 OTA/debug owner 不变；security 确认无 I/O、设备句柄、密钥或 vendor SDK；performance 确认无 timer、动画或每帧开销。嵌入式 C/C++ 适用性：N/A。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI191_EXTENSION_HIERARCHY_PASS themes=3 titles=7 states=contract_only,attach_only
```

向量使用 Qt offscreen 内存对象且未显示主窗口；GUI/EXE 启动、HIDPI、读屏、真实 OTA/AES/RTT/J-Link、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.91` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致，provenance verify 通过。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.91
size: 47,914,013 bytes
SHA-256: 30AB41BD37BCE478565413BEB43A19644F608AA0865C023868A77A64889D55AB
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.90 Embedded Extension Station

### 结果与边界

- `application/extension_capabilities.py` 提供 7 个不可变、有限的 capability DTO：OTA 传输 3 项、OTA 安全 2 项、debug 输出 2 项。
- `presentation/embedded_extension_panel.py` 只渲染 DTO；XMODEM/YMODEM/TFTP 和 AES-GCM/CCM 为“契约预留”，RTT/J-Link 为“仅附着”。
- 不提供 OTA、解密、签名激活、memory/halt/run/reset/flash、vendor 命令或设备动作，不导入 adapter、socket、pyserial、SEGGER SDK/DLL、加密库或原始密钥。
- 工作区新增第四个“扩展 / 工具站”Tab；前三个业务页、`protocol_tab_index`、焦点、ViewModel 与 transport 生命周期不变；route/icon 只扩展 bounded navigation visuals。

### 角色与独立复核

```text
第一轮产品角色       019fec3b-351b-72c0-9e06-22dc2d68628f  called; wait timed out; closed
第一轮架构角色       019fec3b-3562-7e30-ac51-9c449133675b  called; wait timed out; closed
第一轮 UI 设计角色   019fec3b-35b2-71f2-886a-6cb9efd3438c  called; wait timed out; closed
第一轮开发角色       019fec3b-35fd-7451-bac4-177f3e3d4d7b  called; wait timed out; closed
第一轮验证角色       019fec3b-364a-7400-867f-ddc724332973  called; wait timed out; closed
第一轮打包角色       019fec3b-3696-7683-8472-c7dde4d74583  called; wait timed out; closed
修复轮产品角色       019fec3c-cbfd-7200-932c-03d1fccba840  import-fix review; wait timed out; closed
修复轮架构角色       019fec3c-cc47-7430-8595-d78578ff405e  import-fix review; wait timed out; closed
修复轮 UI 设计角色   019fec3c-cc9b-76e0-80df-f681274dcf08  import-fix review; wait timed out; closed
修复轮开发角色       019fec3c-ccf1-7b81-aeea-e2db7e0b0f96  import-fix review; wait timed out; closed
修复轮验证角色       019fec3c-cd4a-77a1-93ca-18ef9bc22735  import-fix review; wait timed out; closed
修复轮打包角色       019fec3c-cd9d-7ad3-a7bc-d47dfa258415  import-fix review; wait timed out; closed
独立质量复核         019fec3d-4458-79c2-bfd3-75ea396f7f59  called after implementation; wait timed out; closed
```

所有角色和独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认 DTO group/reference/state 一致、四节点 route/icon 有界且前三页索引不变；readability/simplicity 确认静态 catalog 与单一 panel builder 足够；architecture 确认 application DTO → presentation 单向依赖、OTA/debug owner 不反向依赖 UI；security 确认无设备/I/O/密钥/vendor 能力；performance 确认只有 7 个有限卡片和一次性 icon 绘制。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
UI190_EXTENSION_CATALOG_PASS groups=ota-transfer:3,ota-security:2,debug-output:2
UI190_EXTENSION_SURFACE_PASS themes=3 cards=7 tab-icons=4 route-nodes=4
UI190_COMPOSITION_PASS tabs=4 protocol_tab_index=1 icons=4
```

向量使用 Qt offscreen 且未显示主窗口；覆盖 7 项 DTO、3 组 panel、三主题、第四个 Tab glyph、四节点 route 和真实组合根的四 Tab/`protocol_tab_index`/icon parity。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实 OTA/加密/RTT/J-Link、硬件、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

### 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.90` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.90
size: 47,911,727 bytes
SHA-256: 82EC5FC027001D4CD34D0114AB0BDC0FC3B5793B14E3B332AD7D0DA5DCC5A36
archive listing SHA-256: E4E9E18BDC1AE62D7C631C119612CA5CB5BB93C7F7EE8509C4313692C9832994
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.89 Shared Motion Lifecycle Fence

### 结果与边界

- `controllers/lifecycle.py:on_motion_frame()` 现在统一检查 `_closing`、不可见、最小化和 `workspace_motion_enabled()`。
- 阻断态复用 `_stop_motion_surfaces()` 并清除 data-activity presentation property；正常可见且允许动效时保持原有 shared frame fan-out。
- 不改变 SessionViewModel、transport、recorder、parser、send queue、业务状态、文字、accessibility、主题 token 或动画时序；不新增 timer、状态源、事件总线或设备依赖。

### 角色与独立复核

```text
产品角色       019fec34-01a7-7de0-bef1-242485dba41e  called; wait timed out; closed
架构角色       019fec34-01f8-7ea1-8736-50adaf44a20c  called; wait timed out; closed
UI 设计角色    019fec34-024d-7962-9d7f-5edf92a55424  called; wait timed out; closed
开发角色       019fec34-02a0-71c0-b757-6e8fd3aa8fe1  called; wait timed out; closed
验证角色       019fec34-02f3-72f1-94c2-fa20638796c1  called; wait timed out; closed
打包角色       019fec34-033a-7011-b000-3f6d5b1367f2  called; wait timed out; closed
独立质量复核   019fec34-9627-7fb1-9ded-fd836d94d024  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认四个生命周期门禁和正常 fan-out；readability/simplicity 确认复用现有 policy 与 stop fan-out；architecture 确认 lifecycle/`MotionController` owner 边界不变；security 确认无输入、I/O、网络、密钥或依赖变化；performance 确认仅增加低成本状态判断。

### 验证

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src scripts  PASS
.venv\Scripts\python.exe -m ruff check src scripts    PASS
MOTION_LIFECYCLE_VECTOR_PASS blocked=4 allowed_frames=1
```

向量使用 Qt offscreen 内存对象且未显示主窗口，覆盖不可见、最小化、低动效、关闭和正常可见五种路径。真实 GUI、HIDPI、读屏、EXE 启动、硬件/网络/OTA、签名和正式发行仍未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

### 打包

本轮 onefile 已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.89` 生成并覆盖根目录 `SerialForge.exe`；canonical/root 字节一致。

```text
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.89
size: 47,896,526 bytes
SHA-256: 8D18CF6A28BA1EF0C3062ABDB097AA3F5A12D764B930FBA1F208F131FF8BE565
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.88 Stable Shell Semantic Tokens

### 结果与边界

- `theme_stylesheet_base.py` 的 app shell、section/error/status、连接状态带、实时/历史观测带、发送/流水线/协议/组件/回放 status 和 preset/context badge 已全部使用已有 semantic token。
- base 与 controls 两个稳定模板的 hex literal count 均为 `0`；稳定模板只保留结构、几何、状态 selector 和 token 引用，`theme_variant_shell.py`/`theme_variant_controls.py` 继续拥有三套主题的具体覆盖。
- 未改变 property state、文案、焦点/禁用/选择、布局、动效时序、业务 projection、设备/网络/密钥依赖；未新增 token、palette registry、timer 或事件总线。

### 角色与独立复核

```text
第一轮产品角色       019fec2b-b9dc-7ba0-9d6c-b6137eb8ba46  called; wait timed out; closed
第一轮架构角色       019fec2b-ba28-7a62-b0d4-104b671e2a90  called; wait timed out; closed
第一轮 UI 设计角色   019fec2b-ba74-7140-8f4c-4027b048592d  called; wait timed out; closed
第一轮开发角色       019fec2b-bac2-7421-9243-3ee890413c91  called; wait timed out; closed
第一轮验证角色       019fec2b-bb0e-7d72-82ee-1ae86c5cec59  called; wait timed out; closed
第一轮打包角色       019fec2b-bb60-7a81-bf27-82e1b26f7c5d  called; wait timed out; closed
第二轮产品角色       019fec2c-df18-70f3-93b7-66d0a490f8e4  import-fix review; wait timed out; closed
第二轮架构角色       019fec2c-df67-7f62-9c37-1f75e37865cb  import-fix review; wait timed out; closed
第二轮 UI 设计角色   019fec2c-dfb1-7a93-a6d9-7d784753f40d  import-fix review; wait timed out; closed
第二轮开发角色       019fec2c-dfff-7e22-9e16-b27906688184  import-fix review; wait timed out; closed
第二轮验证角色       019fec2c-e04d-7af2-889c-d09ad4ca8625  import-fix review; wait timed out; closed
第二轮打包角色       019fec2c-e098-7f71-b7d0-b60f63ee51c9  import-fix review; wait timed out; closed
独立质量复核         019fec2e-afc4-7c83-aa13-be5a25b15c61  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认 token 全部来自现有 `theme_tokens` 且 stylesheet 可解析；readability/simplicity 确认删除稳定模板颜色分叉而不增加抽象；architecture 确认 base/variant owner 边界不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只改变 QSS 字符串，不增加 timer、线程、事件总线或绘制路径。

简化评估结论：将稳定 shell 颜色统一映射到已有角色是最小完整实现；不新增 palette registry 或第二套主题文件。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
UI188_SHELL_STATE_VECTOR_PASS themes=3 states=connection,observation,send,pipeline,status,replay
theme token audit                                      PASS (legacy_qss_literals=0)
```

向量使用 Qt offscreen 内存 widget，未显示主窗口；覆盖连接、实时/历史、发送、流水线、协议/组件/回放 property-state。首次探针把合法 `#fff4ff` 选中文字误当成白色而失败，修正为只检查 `white/#ffffff` 背景 fallback 后通过；Qt 字体目录警告不影响 QSS 验证。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
UI188_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.88
size: 47,895,484 bytes
SHA-256: 78DB186E8BFD92E48F09A1E8A9830DA16B45F2995912D8A63682C689944A87D4
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.87 Stable Controls Semantic Tokens

### 结果与边界

- `theme_stylesheet_controls.py` 的状态标签、surface/errorBar、输入控件及禁用态、SpinBox/ComboBox popup、Menu、按钮、checkbox、workspace Tab、terminal、终端空态和批量空态已改用已有 semantic token。
- 仅改变稳定模板的颜色来源；selector、property state、焦点/禁用/选择交互、布局尺寸、业务 projection、variant owner 均未改变。
- `theme_variant_controls.py` 继续负责三套主题的实际覆盖，未新增 token、palette registry、timer、事件总线、控制器依赖或设备/密钥访问。

### 角色与独立复核

```text
第一轮产品角色       019fec25-5f57-7320-b0ed-e793815f07cd  called; wait timed out; closed
第一轮架构角色       019fec25-5fad-76a3-8009-b46bc5a632cd  called; wait timed out; closed
第一轮 UI 设计角色   019fec25-5ffc-7761-8de9-214a15e5b3b3  called; wait timed out; closed
第一轮开发角色       019fec25-604d-7600-bc43-bce63ee61a12  called; wait timed out; closed
第一轮验证角色       019fec25-609b-7c42-8ea4-4b85068222a2  called; wait timed out; closed
第一轮打包角色       019fec25-60e3-7af2-83a8-e84b0a3f6d41  called; wait timed out; closed
第二轮产品角色       019fec26-db34-7912-b595-2d36852e5b72  import-fix review; wait timed out; closed
第二轮架构角色       019fec26-db82-7082-942d-0f2a14acafbb  import-fix review; wait timed out; closed
第二轮 UI 设计角色   019fec26-dbcf-7a13-9378-7caf6a50896a  import-fix review; wait timed out; closed
第二轮开发角色       019fec26-dc1d-7ac0-9182-b753dfb5eede  import-fix review; wait timed out; closed
第二轮验证角色       019fec26-dc6b-7880-b60c-9e5db022e8fa  import-fix review; wait timed out; closed
第二轮打包角色       019fec26-dcb9-7573-8dba-d746f988e907  import-fix review; wait timed out; closed
独立质量复核         019fec27-bc61-7ba1-bdd5-cb94e2e73ffd  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认只引用已有 token 且 QSS 可解析；readability/simplicity 确认颜色来源收敛而未引入新抽象；architecture
确认 stable template 与 variant owner 边界不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认仅改变 QSS 字符串，不增加 timer、线程、事件总线或绘制路径。

简化评估结论：复用已有语义 token 是本轮最小完整实现；不新建 palette registry 或重复主题 stylesheet。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src/serialforge/        PASS
UI187_CONTROL_SURFACE_VECTOR_PASS themes=3 controls=line,combo,button,checkbox,tab,terminal,table
theme token audit                                      PASS (legacy_qss_literals=208)
```

向量使用 Qt offscreen 内存 widget，未显示主窗口；三主题均完成输入、ComboBox、按钮、checkbox、Tab、terminal、table 组合渲染。Qt 报告当前环境缺少 PySide6 font directory，
不影响本轮 QSS/token 验证，也不代表目标 Windows 字体缺失。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
UI187_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.87
size: 47,894,441 bytes
SHA-256: 632E63F3D038A15571194AD89567D66B40C4D0C4452C4C54AD5804AEAF2E4638
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.86 Stable Control Surface Tokens

### 结果与边界

- `theme_stylesheet_controls.py` 的 `QTableWidget`、`QHeaderView::section`、`QStatusBar`、双向 `QScrollBar`、corner 和 `QToolTip` 已改用 `BACKGROUND`、`SURFACE`、`SURFACE_INPUT`、`BORDER`、`HISTORY_*`、`NEUTRAL_BORDER` 等已有语义 token。
- 表头与 Tooltip 保留渐变层级；表格选中/悬停继续复用 selection/interaction token；没有新增颜色 token、selector、状态源、计时器或业务依赖。
- `theme_variant_controls.py` 仍是三套主题的控件覆盖 owner，稳定模板只提供结构与默认 token 绑定。

### 角色与独立复核

```text
产品角色       019fec1d-7654-72a3-ad58-53ccb04107bd  called; wait timed out; closed
架构角色       019fec1d-76a5-78f0-a504-44f2197f861f  called; wait timed out; closed
UI 设计角色    019fec1d-76f5-7b13-afeb-2f4264e81790  called; wait timed out; closed
开发角色       019fec1d-7744-7551-a4c6-ce6909e91af5  called; wait timed out; closed
验证角色       019fec1d-778e-7393-a12e-642ee0897013  called; wait timed out; closed
打包角色       019fec1d-77ed-77b2-bc2c-5ad51a323d73  called; wait timed out; closed
独立质量复核   019fec1f-6f85-77c0-a620-560e824c0ae5  called after implementation; wait timed out; closed
```

角色和独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认所有新引用来自已有 `theme_tokens` 且模板可解析；readability/simplicity
确认删除散落稳定颜色而不增加抽象层；architecture 确认 stable template 与 variant override 边界不变；security 确认无输入、网络、存储、密钥或依赖变化；performance
确认仅改变 QSS 字符串，不增加 timer、线程、事件总线或绘制路径。

简化评估结论：复用已有 semantic token 是本轮最小完整改动；不新增 palette registry 或重复主题 stylesheet。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src/serialforge/        PASS
UI186_STABLE_QSS_NEAR_WHITE_PASS count=0
UI186_THEME_SURFACE_VECTOR_PASS themes=3 selectors=table,statusbar,scrollbar,tooltip
```

向量使用 Qt offscreen 内存 widget，未显示主窗口；三主题均完成 table/statusbar/scrollbar surface render。首次尝试的检查命令误指向不存在的 `scripts/check.py`，已改用项目实际
`scripts/check.ps1`，不属于源码失败。Qt 报告当前环境缺少 PySide6 font directory；字体目录不影响本轮颜色/QSS 验证，也不代表目标 Windows 字体缺失。
未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
UI186_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-ui-1.86
size: 47,892,639 bytes
SHA-256: 18C089F9C700E230F676F1AB1EEDF5F777ED3584D0352C789800CC8AC0BA1683
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 ARCH-6p MainWindow Composition Shell

### 结果与边界

- `bootstrap.py` 新增 `_bind_protocol_callbacks()`、`_bind_derived_callbacks()`、`_bind_replay_callbacks()`、`_bind_ble_callbacks()`、`_bind_terminal_callbacks()`、`_bind_lifecycle_callbacks()`；每个函数只用 `partial(owner_function, window)` 做显式接线。
- `MainWindow` 删除协议、派生、回放、BLE、终端、生命周期的业务转发，只保留 `__init__` 与 `closeEvent`/`hideEvent`/`showEvent`/`changeEvent` 四个 Qt lifecycle override。
- 首个 timer/signal 使用前完成绑定，保留现有 callback 名称、Qt signal payload、首屏 hydration、焦点/Tab/accessibility、关闭 fence、transport/protocol/derived policy；不新增状态源或业务逻辑。

### 角色与独立复核

```text
第一轮产品角色       019fec14-21d1-75d3-9a9a-bbdadfb96141  called; wait timed out; closed
第一轮架构角色       019fec14-221c-74e1-a8f7-d154988710c3  called; wait timed out; closed
第一轮 UI 设计角色   019fec14-226b-7811-92fe-5dc9915786c8  called; wait timed out; closed
第一轮开发角色       019fec14-22b9-7433-86fb-838099a9db6d  called; wait timed out; closed
第一轮验证角色       019fec14-2300-7830-892a-950e58e98d61  called; wait timed out; closed
第一轮打包角色       019fec14-234f-7a72-a472-6990ce09eb85  called; wait timed out; closed
第二轮产品角色       019fec16-07fe-7043-919a-6be043ec56f9  import-fix review; wait timed out; closed
第二轮架构角色       019fec16-084e-7852-8053-5469a5c6cd80  import-fix review; wait timed out; closed
第二轮 UI 设计角色   019fec16-089b-76f2-b0eb-25341ade417f  import-fix review; wait timed out; closed
第二轮开发角色       019fec16-08e7-7971-b68c-5e61f14fea26  import-fix review; wait timed out; closed
第二轮验证角色       019fec16-093a-7100-bdc4-c2e34857b546  import-fix review; wait timed out; closed
第二轮打包角色       019fec16-0981-7c91-8fbe-344a3a20c5ba  import-fix review; wait timed out; closed
独立质量复核         019fec17-0283-7b33-a115-6df1fdd9cb70  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认 14 个代表性 callback 在首个 Qt 使用前绑定；readability/simplicity 确认删除长 facade、保留具名分域 wiring；
architecture 确认 bootstrap 只做 composition，业务规则仍留在 owner；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只创建有限 partial，不增加 timer、线程、事件总线或绘制路径。

简化评估结论：分域 `_bind_*_callbacks()` 是本轮最小完整实现；未引入 WindowPort/feature DTO 全量重构。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
ARCH6P_COMPOSITION_PASS callbacks=14 main_window_lifecycle_methods=4 state=closed
```

向量使用真实 `create_application()`/`create_main_window()` 与 Qt offscreen，主窗口未 `.show()`；确认代表性 callback 可调用、MainWindow 无业务私有方法并完成关闭。
首次探针的自动 Python 特殊成员计数断言过严，修正断言后向量通过；这不是源码失败。未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
ARCH6P_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-arch-6p
size: 47,891,078 bytes
SHA-256: 70604D4065895FAF07EE86D675FFF8845E7020C82556D0D18ED41188FCA6CA86
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 ARCH-6o Transport and Protocol Framing Composition Callback

### 结果与边界

- `controllers/bootstrap.py` 通过 `partial(on_transport_changed, window)` 绑定 `window._on_transport_changed`，通过
  `partial(on_protocol_framing_changed, window)` 绑定 `window._on_protocol_framing_changed`。
- `MainWindow` 删除两个同名纯转发方法与对应 controller imports；connection builder、preset、replay、composition、protocol config 继续消费显式 callback。
- transport panel 可见性、六种链路 projection、connection refresh 顺序、protocol framing 草稿/状态/timing side effects、首屏 hydration、Qt signal payload、lifecycle 和 accessibility 均保持不变。

### 角色与独立复核

```text
产品角色       019fec0f-316a-7f92-a7e8-36adcaa2ea87  called; wait timed out; closed
架构角色       019fec0f-31a8-7c93-8e55-85a5f7e26a28  called; wait timed out; closed
UI 设计角色    019fec0f-31f8-7882-bc62-bef5e6cc7ad7  called; wait timed out; closed
开发角色       019fec0f-3245-7982-aa02-2e0af4726bb7  called; wait timed out; closed
验证角色       019fec0f-3294-7150-b3bf-57657243529d  called; wait timed out; closed
打包/流程角色  019fec0f-32dd-7f21-8d1d-3080ab2f91a7  called; wait timed out; closed
独立质量复核   019fec10-2795-7383-bb26-2239d853e6eb  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认 callback 在所有使用前绑定且只替换转发目标；readability/simplicity 确认删除无状态 facade、保留各 owner；
architecture 确认组合根接线与 connection/protocol 依赖方向不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只创建两个 partial，不增加 timer、线程、事件总线或绘制路径。

简化评估结论：两个命名 callback 是本轮最小完整迁移；不引入 WindowPort/feature DTO 全量重构。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
ARCH6O_COMPOSITION_PASS transport=bound framing=bound facade_removed=True state=closed
```

向量使用真实 `create_application()`/`create_main_window()` 与 Qt offscreen，主窗口未 `.show()`；确认两个 callback 可调用、类上旧 facade 不存在并完成关闭。
未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
ARCH6O_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-arch-6o
size: 47,893,668 bytes
SHA-256: 4DC716FA8B652158058F777284968B8D5E8CC6C3C12DDEF52CBE0B3A919891F8
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 ARCH-6n Connection Refresh Composition Callback

### 结果与边界

- `controllers/connection.py` 继续唯一拥有连接控件 enable/hint/busy projection；`controllers/bootstrap.py` 在组合根通过
  `partial(update_connection_controls, window)` 绑定 `window._refresh_connection_controls`。
- `ble.py`、`commands.py`、`connection_presets.py`、`connection_runtime.py`、`lifecycle.py`、`replay.py`、`terminal_runtime.py` 与 bootstrap 首屏调用均已迁移到显式 callback。
- `MainWindow` 删除 `_update_connection_controls` 纯转发、`SessionState` import 和 `connection` controller import；没有新增状态源、事件总线、动态 callback map、mixin 或业务规则复制。
- 首屏 hydration、session/replay gate、BLE/TCP Server readiness、Qt signal/lifecycle 和 accessibility 语义保持不变；本轮未改变 UI 样式或业务协议。

### 角色与独立复核

```text
第一轮产品角色       019fec06-f1f9-7900-a7ca-7ec90858cdb7  called; wait timed out; closed
第一轮架构角色       019fec06-f249-7f10-a751-ce4492ce48e8  called; wait timed out; closed
第一轮 UI 设计角色   019fec06-f293-7a32-9602-05ae275387a3  called; wait timed out; closed
第一轮开发角色       019fec06-f32d-7ad1-8cc2-239a2826b47d  called; wait timed out; closed
第一轮验证角色       019fec06-f2e4-7e40-bc7f-57e3f6462b85  called; wait timed out; closed
第一轮打包角色       019fec06-f378-71f2-87bd-dcdf595df837  called; wait timed out; closed
第二轮产品角色       019fec0a-b9ab-7a50-9bfe-cd5db36e6661  import-fix review; wait timed out; closed
第二轮架构角色       019fec0a-b9f3-7a92-9a62-6d97c8ea52d5  import-fix review; wait timed out; closed
第二轮 UI 设计角色   019fec0a-ba41-7f63-bde9-a9963c1e1ad1  import-fix review; wait timed out; closed
第二轮开发角色       019fec0a-ba89-7a80-b481-982e82a48ddd  import-fix review; wait timed out; closed
第二轮验证角色       019fec0a-bad9-7802-9c76-0ab0cd021eb1  import-fix review; wait timed out; closed
第二轮打包角色       019fec0a-bb2a-7211-811c-dd29f5971c3f  import-fix review; wait timed out; closed
独立质量复核         019fec0b-98ed-7762-b7ab-9c2569497fcb  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，超时不被视为通过。父代理完成五轴审查：correctness 确认绑定发生在所有调用前且只替换转发目标；readability/simplicity 确认删除无状态 facade、保留单一 policy owner；
architecture 确认组合根接线、controller 依赖方向和生命周期边界不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只创建一个 partial，不增加 timer、线程、事件总线或重绘路径。

简化评估结论：命名 callback 是本轮最小完整迁移；未扩大为 WindowPort/feature DTO 全量重构。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
.venv\\Scripts\\ruff.exe check src scripts              PASS
ARCH6N_COMPOSITION_PASS callback=bound facade_removed=True state=closed
```

向量使用真实 `create_application()`/`create_main_window()` 与 Qt offscreen，主窗口未 `.show()`；确认 callback 可调用、旧 facade 不存在并完成关闭。
未运行可见 GUI、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
ARCH6N_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
source revision: local-arch-6n
size: 47,894,630 bytes
SHA-256: A56986CF93FB1B3B100C78B3AA172ECF5542BB14BF4BABB8D07537DB32925DB7
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.85 Theme Text Tokens

### 用户结果与范围

- stable QSS 中八处直接写死的近白 `#fff…` 文字/选择色现在分别复用 `TEXT` 与 `SELECTION_TEXT`，月影深海/樱雾夜航不再继承星轨的近白高光。
- checkbox、combo popup、table selection、workspace tab selected 的 selector、焦点、选择背景、文字、Tab/accessibility 和业务状态保持不变。
- 这轮只收敛主题颜色来源，不新增 token、状态、动画时钟或控制器依赖。

### 实际修改文件

- `src/serialforge/presentation/theme_stylesheet_base.py`：station section 近白文字改用 `TEXT`。
- `src/serialforge/presentation/theme_stylesheet_controls.py`：selection/combo/menu/tab/table 近白文字改用 `SELECTION_TEXT` 或 `TEXT`。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0072-theme-text-tokens.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、约束、评审和交付状态。

### 评审与简化

```text
产品角色       019febfd-9bef-7990-844f-f7548ab9c6fc  called before source edit; wait timed out; closed
架构角色       019febfd-9c3d-7131-80fd-ddd70fb762f7  called before source edit; wait timed out; closed
UI 设计角色    019febfd-9c88-7243-88e4-658ca28ceda8  called before source edit; wait timed out; closed
开发角色       019febfd-9cd5-74e3-850c-fc03af336921  called before source edit; wait timed out; closed
验证角色       019febfd-9d23-72c1-a557-d8d5a463dcdb  called before source edit; wait timed out; closed
打包/流程角色  019febfd-9d74-7d92-bd0d-5ae21777e257  called before source edit; wait timed out; closed
独立质量复核   019febff-5692-7d73-9d24-fb3d4dda9fa7  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，父代理未将超时视为通过，并完成五轴复核：correctness 确认八处替换只改变 token 来源；readability/simplicity 确认复用已有 token、无新抽象；
architecture 确认 stable template 与 ThemeSpec variant 责任不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只改变 QSS 字符串，不增加运行时对象或绘制负担。

简化评估结论：直接复用 `TEXT`/`SELECTION_TEXT` 是最小完整实现，不新增 `NEAR_WHITE`、颜色注册表或 selector 层。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI185_STABLE_NEAR_WHITE_PASS file=theme_stylesheet_base.py count=0
UI185_STABLE_NEAR_WHITE_PASS file=theme_stylesheet_controls.py count=0
UI185_THEME_WIDGET_RENDER_PASS theme=star_trail selection=#fff4ff
UI185_THEME_WIDGET_RENDER_PASS theme=moonlit_ocean selection=#e9fbff
UI185_THEME_WIDGET_RENDER_PASS theme=sakura_night selection=#fff1f8
UI185_THEME_TOKEN_VECTOR_PASS themes=3 selectors=selection,checkbox,combo,table,tab
```

向量使用 Qt offscreen 内存 widget render 和三套 `ThemeSpec`；未启动可见 GUI。HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收未运行；未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。

### 打包

```text
UI185_PACKAGE_FINAL                         PASS
artifact: dist/release/0.1.0/core/onefile/app/SerialForge.exe
root: SerialForge.exe (byte-identical)
size: 47,895,205 bytes
SHA-256: 48A166688A58393D9A060B56DC7C8CCDB3FB9A7F0666EDD6EE3B4C7983AEF6CE
archive listing SHA-256: 29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D
python / PyInstaller: 3.12.13 / 6.22.0
signature / release_eligible: NotSigned / false
hardware_acceptance: not_run
source revision: local-ui-1.85
```

canonical artifact 已覆盖根目录 `SerialForge.exe`，两者 size/SHA-256 一致；包是未签名 `engineering_build`，不能等同正式发行或硬件验收通过。

## 2026-08-10 UI-1.84 Scroll Surface

### 用户结果与范围

- 三个设置页统一使用横向可扩展 content 和 `HorizontalScrollBarPolicy=ScrollBarAlwaysOff`，避免工作区底部出现像白线的横向滚动/亮色槽。
- 纵向滚动仍为 `ScrollBarAsNeeded`；页面 objectName、widgetResizable、内容、键盘 Tab 顺序、AccessibleDescription、controller action 和主题切换保持不变。
- 980、1180、1440 宽度均通过策略 vector；三套主题切换后横向滚动条仍保持隐藏。

### 实际修改文件

- `src/serialforge/presentation/controllers/composition.py`：在共享 `scroll_page()` 设置 content `QSizePolicy.Expanding` 与横向 `ScrollBarAlwaysOff`。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0071-scroll-surface.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、约束、评审和交付状态。

### 评审与简化

```text
产品角色       019febf6-81cf-7e03-9188-341bc8eb6787  called before source edit; wait timed out; closed
架构角色       019febf6-820e-70e1-afa7-4d08e10c3b3f  called before source edit; wait timed out; closed
UI 设计角色    019febf6-825e-7fc0-b60d-1d2e811fe1d1  called before source edit; wait timed out; closed
开发角色       019febf6-82aa-7ff2-8f36-74a75fe270d9  called before source edit; wait timed out; closed
验证角色       019febf6-82fe-7ac1-843c-021533e99357  called before source edit; wait timed out; closed
打包/流程角色  019febf6-8348-71f1-b164-8a9ee0aec156  called before source edit; wait timed out; closed
独立质量复核   019febf8-8382-7730-866a-68b78a3e614c  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，父代理未将超时视为通过，并完成五轴复核：correctness 确认三个 settings page 都经过同一入口、横向策略关闭、纵向策略保留；
readability/simplicity 确认只增加一个 size policy 和一个 scroll policy，没有复制 feature controller 逻辑；architecture 确认 `scroll_page()` 仍是 composition owner，
页面 controller、content layout、焦点和 lifecycle owner 不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认不创建新 timer/effect，只减少一个无意义滚动槽。

简化评估结论：共享入口的两项明确配置是最小完整修复；不引入自定义滚动条、页面级 override 或额外布局代理。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI184_SCROLL_VECTOR_PASS width=980 pages=3 horizontal=hidden vertical=as-needed
UI184_SCROLL_VECTOR_PASS width=1180 pages=3 horizontal=hidden vertical=as-needed
UI184_SCROLL_VECTOR_PASS width=1440 pages=3 horizontal=hidden vertical=as-needed
UI184_THEME_SCROLL_PASS theme=star_trail
UI184_THEME_SCROLL_PASS theme=moonlit_ocean
UI184_THEME_SCROLL_PASS theme=sakura_night
UI184_SCROLL_SURFACE_VECTOR_PASS responsive=980,1180,1440 themes=3
```

向量使用真实 `create_application()`/`create_main_window()` 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；验证了三个页面的横/纵滚动策略、widgetResizable、content size policy、页面名称和三主题刷新。
HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.84` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.84`
- size：`47,892,763` bytes
- SHA-256：`D10B2F4972EF9D4115B45B87DB770DC66D5E9E7AE91EFE40B40DE070BB838E17`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.83 Analysis Status Marker

### 用户结果与范围

- Protocol/Component/Dataset/Curve 共用 `AnalysisStatusLabel` 的 `error`、`blocked`、`history` 状态现在有主题化静态几何确认：叉、双横栏、回退箭头。
- active、waiting、draft 继续使用既有 shared frame 动效；empty、idle、bootstrap hydration、隐藏/最小化、暂停和 reduced-motion 保持静态。
- marker 只辅助既有文字和 accessibility，不改变 `state/source`、controller projection、结果表、预览、曲线、焦点、objectName 或 QSS。

### 实际修改文件

- `src/serialforge/presentation/analysis_status_surface.py`：在既有 rail 上增加三类静态 marker，继续使用 `ThemeSpec` 语义色和 shared frame。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0070-analysis-status-marker.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、约束、评审和交付状态。

### 评审与简化

```text
产品角色       019febed-f35f-7c83-a3cc-bf4cdbb07b63  called before source edit; wait timed out; closed
架构角色       019febed-f3ad-70c3-bbc8-c1b7b479eba9  called before source edit; wait timed out; closed
UI 设计角色    019febed-f3f7-76f1-b5b6-e92a9869e39b  called before source edit; wait timed out; closed
开发角色       019febed-f44c-7023-9d02-6b82564d128d  called before source edit; wait timed out; closed
验证角色       019febed-f49b-7cb2-9483-1119f96423cb  called before source edit; wait timed out; closed
打包/流程角色  019febed-f4ef-71c3-8cf5-fe4774de3564  called before source edit; wait timed out; closed
独立质量复核   019febf0-59fe-71b2-9953-1c6d75687cf4  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，父代理未将超时视为通过，并完成五轴复核：correctness 确认 marker 只由已有 `state/source` 决定、三状态几何分支和未知状态静态回退；
readability/simplicity 确认只增加一个状态集合与一个绘制分支，不新增 DTO、timer、事件总线或抽象层；architecture 确认 `AnalysisStatusLabel` 仍是 presentation renderer，
controller/status projection 与 lifecycle owner 不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认 marker 不随 frame 改变，动态状态仍只复用共享时钟。

简化评估结论：直接在既有 `paintEvent()` rail 末端增加三种 bounded 几何分支是最小完整实现；不引入通用 icon registry、状态 DTO 或第二个绘制时钟。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI183_ANALYSIS_MARKER_RENDER_PASS theme=star_trail state=error
UI183_ANALYSIS_MARKER_RENDER_PASS theme=star_trail state=blocked
UI183_ANALYSIS_MARKER_RENDER_PASS theme=star_trail state=history
UI183_ANALYSIS_MARKER_RENDER_PASS theme=moonlit_ocean state=error
UI183_ANALYSIS_MARKER_RENDER_PASS theme=moonlit_ocean state=blocked
UI183_ANALYSIS_MARKER_RENDER_PASS theme=moonlit_ocean state=history
UI183_ANALYSIS_MARKER_RENDER_PASS theme=sakura_night state=error
UI183_ANALYSIS_MARKER_RENDER_PASS theme=sakura_night state=blocked
UI183_ANALYSIS_MARKER_RENDER_PASS theme=sakura_night state=history
UI183_ANALYSIS_SURFACE_VECTOR_PASS states=error,blocked,history themes=3 static=pass
```

向量使用真实 `create_application()`/`create_main_window()` 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；marker 是独立 presentation label 的内存 QPixmap render，未启动可见 GUI。
HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.83` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.83`
- size：`47,891,907` bytes
- SHA-256：`2DD17ED968B0BC7045C5443B08B161DD6123EEE1CE6CDDA4D064E8CDDACCBE1D`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.82 Batch Terminal Activity

### 用户结果与范围

- 批量命令 RUNNING 继续使用 520ms shared activity；COMPLETED、STOPPED、FAILED 到达可见窗口时增加一次 480ms terminal confirmation，让用户能确认批量动作已经完成、停止或失败。
- `CommandBatchSurfaceLabel` 在既有 bounded `CommandBatchSurfaceProjection` 上绘制主题化静态步骤 marker：completed 为勾、stopped 为横线、failed 为叉；不把 accepted steps 解释成时间百分比或设备处理进度。
- IDLE、bootstrap hydration、隐藏/最小化、暂停和 reduced-motion 保持静态；不改变 batch snapshot、结果表、按钮、发送队列、焦点或 accessibility；关闭由既有 lifecycle stop/close fence 清理。

### 实际修改文件

- `src/serialforge/presentation/controllers/commands.py`：增加唯一批量 activity helper，复用既有 `MotionController`，区分 RUNNING 520ms 与终态 480ms，并保留可见/最小化/关闭 guard。
- `src/serialforge/presentation/command_batch_surface.py`：从既有 bounded projection 为三个终态绘制主题语义色的静态勾/横线/叉 marker，不增加 timer 或状态源。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0069-batch-terminal-activity.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录模块边界、约束、评审和交付状态。

### 评审与简化

```text
产品角色       019febe3-b30c-7542-9eda-b95de90f2273  called before source edit; wait timed out; closed
架构角色       019febe3-b355-7be0-87e2-eb86bf3dbf08  called before source edit; wait timed out; closed
UI 设计角色    019febe3-b3a1-7190-b7cf-c5d18a6d8060  called before source edit; wait timed out; closed
开发角色       019febe3-b3f3-7c63-a997-9daf4454c62b  called before source edit; wait timed out; closed
验证角色       019febe3-b43d-7bf0-bdf9-a6bcea4d928c  called before source edit; wait timed out; closed
打包/流程角色  019febe3-b48a-7130-895b-5e439e3f1aad  called before source edit; wait timed out; closed
独立质量复核   019febe6-08a3-7dc0-9337-30e2d5daf2fc  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，父代理未将超时视为通过，并完成五轴复核：correctness 确认 RUNNING/终态时长映射、可见/最小化/关闭 guard 和 IDLE 静态行为；
readability/simplicity 确认 controller 只有一个边界 helper、surface 只消费 bounded projection，无新 timer/状态源；architecture 确认 commands controller 仍是动作和
snapshot owner，surface 仍是绘制 owner，lifecycle 仍是 shared clock owner；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只复用 96ms shared clock，
最大 activity 仍为 520ms，终态 marker 不暗示设备处理进度。

简化评估结论：一个共享时钟入口加一个静态 marker 分支是最小完整实现；不引入 terminal event bus、局部 `QTimer`、百分比/时间进度、额外 DTO 或第二个动画时钟。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI182_HIDDEN_BATCH_STATE_PASS state=idle
UI182_HIDDEN_BATCH_STATE_PASS state=running
UI182_HIDDEN_BATCH_STATE_PASS state=completed
UI182_HIDDEN_BATCH_STATE_PASS state=stopped
UI182_HIDDEN_BATCH_STATE_PASS state=failed
UI182_VISIBLE_ACTIVITY_PASS running=520 terminal=480
UI182_LIFECYCLE_GUARD_PASS hidden=minimized=closing
UI182_TERMINAL_MARKER_RENDER_PASS theme=star_trail
UI182_TERMINAL_MARKER_RENDER_PASS theme=moonlit_ocean
UI182_TERMINAL_MARKER_RENDER_PASS theme=sakura_night
UI182_SURFACE_VECTOR_PASS terminal_states=completed,stopped,failed themes=3
UI182_BATCH_TERMINAL_VECTOR_PASS hidden_static=pass visible_probe=pass render=pass
```

向量使用真实 `create_application()`/`create_main_window()` 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；验证了五种批量状态的隐藏静态 guard、可见 520/480
shared-controller probe、三套主题终态 marker 绘制。可见 Windows GUI 动态帧、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收未运行；未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.82` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.82`
- size：`47,894,052` bytes
- SHA-256：`BD39D94F914E3AF6DC24E44C04A0B02C0BF8F5553C87911D44CE773CBF6489FB`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.81 Replay Terminal Activity

### 用户结果与范围

- 历史回放 PLAYING 继续有 520ms shared activity；EOF、STOPPED、ERROR 到达可见窗口时增加一次 480ms terminal confirmation，让用户更容易确认回放已结束、被停止或失败。
- `ReplayActivityLabel` 在既有 history/error projection 上绘制主题化静态终态 marker；不把记录数转成百分比或进度，不改变 replay DTO、按钮 enable、source badge、历史数据、文案、焦点或 accessibility。
- PAUSED/EMPTY、bootstrap hydration、隐藏/最小化、暂停和 reduced-motion 保持静态；关闭由既有 lifecycle stop/close fence 清理。

### 实际修改文件

- `src/serialforge/presentation/controllers/replay.py`：新增可见窗口 replay activity owner，区分 PLAYING 520ms 与终态 480ms。
- `src/serialforge/presentation/replay_activity_surface.py`：为 history/error projection 增加静态终态 marker。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0068-replay-terminal-activity.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、评审与交付状态。

### 评审与简化

```text
产品角色       019febdc-66ec-7d62-a2d2-b7fd81cedd22  called before source edit; wait timed out; closed
架构角色       019febdc-6738-7ab3-853c-fa021d0382b6  called before source edit; wait timed out; closed
UI 设计角色    019febdc-6789-7bb2-8c9c-a8c0b39546c3  called before source edit; wait timed out; closed
开发角色       019febdc-67e2-71a1-b6d7-4719210c61b6  called before source edit; wait timed out; closed
验证角色       019febdc-6832-78f0-b4da-16a752d411f6  called before source edit; wait timed out; closed
打包/流程角色  019febdc-687e-78a1-b37b-dcc9aa7fadc7  called before source edit; wait timed out; closed
独立质量复核   019febdf-5451-77a1-9b3d-d4199c6e6537  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，父代理未将超时视为通过，并完成五轴复核：correctness 确认 PLAYING/终态的时长分工和可见/最小化/关闭 guard，
PAUSED/EMPTY 不请求；readability/simplicity 确认 replay controller 只投影 activity、surface 只绘制 marker，无新 timer/状态源；architecture 确认
回放 DTO、按钮和 lifecycle owner 不变；security 确认无输入、网络、存储、密钥或依赖变化；performance 确认只复用 96ms shared clock，无常驻资源，
终态 marker 不暗示进度。

简化评估结论：controller 的一个可见 guard helper 加 surface 的静态终态 marker 是最小完整实现；不引入 terminal event bus、百分比、局部 QTimer 或第二个时钟。
嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI181_HIDDEN_REPLAY_STATE_PASS state=playing
UI181_HIDDEN_REPLAY_STATE_PASS state=paused
UI181_HIDDEN_REPLAY_STATE_PASS state=eof
UI181_HIDDEN_REPLAY_STATE_PASS state=stopped
UI181_HIDDEN_REPLAY_STATE_PASS state=error
UI181_VISIBLE_ACTIVITY_PASS playing=520 terminal=480
UI181_LIFECYCLE_GUARD_PASS hidden=minimized=closing
UI181_TERMINAL_MARKER_RENDER_PASS theme=star_trail
UI181_TERMINAL_MARKER_RENDER_PASS theme=moonlit_ocean
UI181_TERMINAL_MARKER_RENDER_PASS theme=sakura_night
UI181_SURFACE_VECTOR_PASS terminal_states=history,error themes=3
UI181_REPLAY_TERMINAL_VECTOR_PASS hidden_static=pass visible_probe=pass render=pass
```

向量使用真实 `create_application()`/`create_main_window()` 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；验证了五种回放状态的隐藏静态 guard、
可见 520/480 shared-controller probe、三套主题终态 marker 绘制。可见 Windows GUI 动态帧、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行
验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.81` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.81`
- size：`47,893,847` bytes
- SHA-256：`DA71E46BE6B626E644F6E907FC75CCE44C8E9C4E174BCA0FC8E6BA9B4B7893D8`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.80 Derived Activity

### 用户结果与范围

- Protocol → Component → Dataset → Curve 派生链路收到真实非空 `ComponentFrameRow` 或 `DatasetSample` 快照时，可见 Protocol 页的既有 status rail、pipeline surface 与相关 signal consumers 获得一次 360ms shared activity pulse，帮助用户确认解析结果已经进入界面。
- 空快照、bootstrap hydration、后台 Protocol 页、隐藏/最小化/关闭、暂停和 reduced-motion 保持静态；不改变 DTO、stats、表格/预览/曲线数据、renderer throttle、清空语义或 accessibility。

### 实际修改文件

- `src/serialforge/presentation/controllers/derived_data.py`：新增单一派生 activity owner 与真实非空快照入口 guard，复用唯一 `MotionController`。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0067-derived-activity.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、评审与交付状态。

### 评审与简化

```text
产品角色       019febd2-d13a-7e50-a975-b807b8ab5427  called before source edit; wait timed out twice; closed
架构角色       019febd2-d183-7772-a53e-1bda9ed177b3  called before source edit; wait timed out twice; closed
UI 设计角色    019febd2-d1ce-7272-99c3-f14df1a3655c  called before source edit; wait timed out twice; closed
开发角色       019febd2-d21a-7772-87f6-6c8d803fc423  called before source edit; wait timed out twice; closed
验证角色       019febd2-d267-7cc3-83ac-0285f3bf9bcb  called before source edit; wait timed out twice; closed
打包/流程角色  019febd2-d2b7-73f0-969d-10e7a2e242de  called before source edit; wait timed out twice; closed
独立质量复核   019febd7-1d96-79c0-a54e-44f1b1923ad9  called after implementation; wait timed out; closed
```

角色与独立复核均未返回完整报告，父代理未将超时视为通过，并完成五轴复核：correctness 确认只对非空、类型正确快照且可见 Protocol 页请求 activity，
空/初始化/后台/生命周期路径静态；readability/simplicity 确认一个短 helper 复用既有控制器，无新 timer、状态或 DTO；architecture 确认
`derived_data.py` 只拥有既有快照的 presentation projection，Component/Dataset worker 与 lifecycle owner 不变；security 确认没有输入、网络、
存储、密钥或依赖变化；performance 确认请求上限 360ms，不增加常驻时钟，并沿用 shared MotionController 的 paused/reduced-motion/suspend fence。

简化评估结论：在 `on_component_rows_changed()` 与 `on_dataset_samples_changed()` 的非空快照入口复用一个本地 helper 是最小完整实现；不新增
derived event bus、snapshot hash/cache、error counter、QTimer 或第二套动画状态。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI180_HIDDEN_HYDRATION_GUARD_PASS
UI180_VISIBLE_ACTIVITY_PASS duration=360
UI180_LIFECYCLE_GUARD_PASS hidden=minimized=other_tab
UI180_SHARED_CONTROLLER_PASS no_local_timer
UI180_DERIVED_ACTIVITY_VECTOR_PASS themes=default-composition hidden=static visible-probe=360
UI180_THEME_APPLY_PASS theme=star_trail
UI180_THEME_APPLY_PASS theme=moonlit_ocean
UI180_THEME_APPLY_PASS theme=sakura_night
UI180_THEME_VECTOR_PASS themes=3 main_window_hidden
```

向量使用真实 `create_application()`/`create_main_window()` 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；验证了真实隐藏组合根 hydration guard、
三主题主题应用，以及不绕过共享控制器的可见 probe。可见 Windows GUI 的实际动态帧、HIDPI、读屏、EXE 启动、真实硬件/网络、OTA、签名和正式发行验收
未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.80` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.80`
- size：`47,890,929` bytes
- SHA-256：`E47A06C61064CC934B0812FEDCAD462AD61E3DFB664A3593C37705CEEE76365E`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.79 Error Activity

### 用户结果与范围

- 非空 `ErrorInfo` 投影到已可见主窗口时，既有 error beacon、Header signal field 与 status rail 获得一次 520ms shared activity pulse，帮助用户确认错误已被界面接收。
- 清除错误不请求新 pulse；错误 DTO、严重性、文案、清除按钮、焦点、Tab 顺序和 accessibility 语义保持不变。
- 初始化、隐藏/最小化、暂停、reduced-motion 和关闭路径保持静态；错误 activity 不会成为业务计时器、错误数量或进度源。

### 实际修改文件

- `src/serialforge/presentation/controllers/lifecycle.py`：在确认非空 `ErrorInfo` 后，复用窗口唯一 `MotionController` 的 520ms activity 请求，并以可见性/生命周期 guard 保护。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0066-error-activity.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、评审与交付状态。

### 评审与简化

```text
产品角色       019febc9-2cb9-7891-842f-a4f6370d5535  called before source edit; wait timed out; closed
架构角色       019febc9-2d04-7330-b38c-3c24fb156390  called before source edit; wait timed out; closed
UI 设计角色    019febc9-2d51-7ea1-9bcc-5947d06b53fa  called before source edit; wait timed out; closed
开发角色       019febc9-2da3-79f1-bb71-0cadacb5894b  called before source edit; wait timed out; closed
验证角色       019febc9-2def-7af0-9446-438bd9345b69  called before source edit; wait timed out; closed
打包/流程角色  019febc9-2e3a-76f0-9fc5-43890483f5e2  called before source edit; wait timed out; closed
独立质量复核   019febca-91bb-7993-a19a-38663802b5ea  called after implementation; wait timed out; closed
```

六角色与独立复核没有返回完整报告，父代理未将其视为通过，并完成五轴复核：correctness 确认只对非空 `ErrorInfo` 且可见窗口请求 activity，
清除路径不请求 pulse，隐藏/生命周期 guard 保持静态；readability/simplicity 确认只复用已有控制器，无新增状态、timer 或抽象层；architecture
确认 lifecycle 负责投影时机，ErrorSignalSurface 继续负责渲染，错误模型边界不变；security 确认没有输入、网络、存储、密钥或依赖变化；performance
确认复用 96ms MotionController，最长请求 520ms，不引入常驻计时器。

简化结论：把一次 `request_activity(520)` 放在既有错误投影入口是最小完整实现；不新增 `QTimer`、error pulse counter、错误事件总线、自动聚焦或
重复的 view-model 状态。嵌入式 C/C++ 适用性：N/A。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\\Scripts\\python.exe -m compileall -q src          PASS
UI179_ERROR_PROJECTION_PASS theme=star_trail beacon=active hidden_motion=static
UI179_ERROR_CLEAR_PASS theme=star_trail
UI179_ERROR_PROJECTION_PASS theme=moonlit_ocean beacon=active hidden_motion=static
UI179_ERROR_CLEAR_PASS theme=moonlit_ocean
UI179_ERROR_PROJECTION_PASS theme=sakura_night beacon=active hidden_motion=static
UI179_ERROR_CLEAR_PASS theme=sakura_night
UI179_MOTION_POLICY_PASS paused=static
UI179_ERROR_ACTIVITY_VECTOR_PASS themes=3 show_clear=pass hidden_guard=pass
```

向量使用真实 composition root 与 `QApplication`/Qt offscreen，主窗口未 `.show()`；因此验证了三套主题的错误出现/清除投影、隐藏 guard、暂停和
低动效静态策略，但没有把“可见窗口实际 520ms 脉冲”标记为已运行。未运行完整 GUI、读屏、HIDPI、EXE 启动、真实硬件/网络、OTA、签名或正式发行验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

### 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.79` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.79`
- size：`47,892,391` bytes
- SHA-256：`DF374A4497AA1608478CEF24CE89B61038A7BF5AA0445AB6807C83EAADEE07E5`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.78 Workspace Activity

### 用户结果与范围

- 用户切换“链路 / 连接、协议 / 遥测、命令管理”时，既有路线 beacon 和共享 signal rail 会获得一次 320ms activity pulse，点击/键盘导航都有即时确认。
- 这只是 presentation feedback：不改变 Tab 文案、index、焦点、读屏顺序、derived renderer suspension 或 180ms workspace page fade。
- 首次 hydration、隐藏/最小化/关闭、暂停和 reduced-motion 不触发或延长 pulse。

### 实际修改文件

- `src/serialforge/presentation/controllers/workspace_runtime.py`：新增可见导航 activity owner，复用 `MotionController.request_activity(320)`。
- `src/serialforge/presentation/controllers/workspace.py`：将 owner 接入现有 `QTabWidget.currentChanged` signal。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0065-workspace-activity.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：记录边界、
  评审与交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019febc2-c989-7733-b588-eed517607c84  called before source edit; wait timed out; closed
架构角色       019febc2-c9e0-79a1-b144-1df0cda062d7  called before source edit; wait timed out; closed
UI 设计角色    019febc2-ca2e-7c83-a1d4-c50f5498af8c  called before source edit; wait timed out; closed
开发角色       019febc2-ca88-77a3-8cff-12725a6f24f9  called before source edit; wait timed out; closed
验证角色       019febc2-cad7-71a3-aee8-ff5718ca1ee1  called before source edit; wait timed out; closed
打包/流程角色  019febc2-cb24-7710-be77-cf53bfaf6646  called before source edit; wait timed out; closed
独立质量复核   019febc4-9d7f-7da2-84d8-79ad64f03400  called after implementation; wait timed out; closed
```

六角色与独立复核均未返回完整报告，未把超时当成通过。父代理完成五轴审查：correctness 确认 signal 接线只在工作区切换时请求 activity，隐藏
主窗 hydration 向量未触发；readability/simplicity 确认新增单一、短小的 runtime owner，没有复制 transition 或 route state；architecture
确认 workspace builder 只接线、runtime 只消费 shared clock，Tab/derived renderer owner 不变；security 确认没有输入、网络、存储、密钥或依赖
变化；performance 确认复用 96ms MotionController、最长 320ms，无新 timer，生命周期 guard 完整。

简化评估结论：把 `request_activity(320)` 放进独立 signal consumer 是最小完整实现；不新增 active-tab 状态、QPropertyAnimation、route event bus 或
第二套动画时钟。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src          PASS
UI178_HIDDEN_GUARD_PASS
UI178_WORKSPACE_ACTIVITY_PASS theme=star_trail duration=320
UI178_WORKSPACE_ACTIVITY_PASS theme=moonlit_ocean duration=320
UI178_WORKSPACE_ACTIVITY_PASS theme=sakura_night duration=320
UI178_LIFECYCLE_GUARD_PASS hidden=minimized=closing
UI178_REDUCED_MOTION_POLICY_PASS paused=static
UI178_WORKSPACE_ACTIVITY_VECTOR_PASS themes=3 tab_routes=3 hydration=static
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根的 Qt offscreen 实例，并用可见导航 probe 验证 signal consumer；未显示主窗口、
未启动 EXE/后台服务、未接入硬件或网络。环境提示缺失 PySide6 虚拟字体目录；未据此宣称 Windows 字体/HIDPI/真实窗口通过。完整 GUI/HIDPI/读屏/视觉帧
差分、EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.78 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.78` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.78`
- size：`47,890,905` bytes
- SHA-256：`BFE9346ED94B84E88A871314ACAACE33E5E52D51FBBF0EF86CEBF0AA3CCD1C26`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.77 Transport Section Visibility

### 用户结果与范围

- 连接方式切换时，UART、网络、BLE 的 section 标题现在与实际可见参数 panel 严格同步，不再留下隐藏 panel 的孤立标题行或额外空白行。
- UART 使用 UART 标题/panel；TCP Client、TCP Server、UDP、J-Link RTT 共享网络标题/panel；BLE GATT 使用 BLE 标题/panel。
- 标题继续使用 `role="section"`，panel transition、连接配置、焦点、Tab 顺序、AccessibleName/Description、主题 token 和 reduced-motion 语义不变。

### 实际修改文件

- `src/serialforge/presentation/controllers/connection_builder.py`：保存三个 section QLabel 的 presentation 引用。
- `src/serialforge/presentation/controllers/connection_runtime.py`：在既有 panel projection 旁同步三个标题的可见性。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0064-transport-section-visibility.md`、`tasks/plan.md`、`tasks/todo.md`、
  `README.md`：记录 owner 边界、parity 契约、复核与交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019febba-d48a-7ce3-8f92-c4280b62007f  called before source edit; wait timed out; closed
架构角色       019febba-d4d9-76d3-84b5-c21d67acea2c  called before source edit; wait timed out; closed
UI 设计角色    019febba-d521-7201-a652-e57bc2e906cb  called before source edit; wait timed out; closed
开发角色       019febba-d56d-7913-b11e-5c0ac3762849  called before source edit; wait timed out; closed
验证角色       019febba-d5bb-7c73-947e-2a1bf2997ed6  called before source edit; wait timed out; closed
打包/流程角色  019febba-d60d-7001-a0a0-db1a060ff5df  called before source edit; wait timed out; closed
独立质量复核   019febbc-a397-73c1-9531-e662d1ff7d99  called after implementation; wait timed out; closed
```

六角色与独立复核均未返回完整报告，未把超时当成通过。父代理完成五轴审查：correctness 确认标题与 panel 使用同一组 transport booleans，
且首次 hydration 与切换路径均由既有 `on_transport_changed()` 覆盖；readability/simplicity 确认仅增加三个明确的 presentation 引用和
三次对称 `setVisible`，没有引入 helper、状态机或重复逻辑；architecture 确认 builder 只装配、runtime 只做 projection，domain/ViewModel/DTO
不变；security 确认没有新增输入、网络、存储、密钥或依赖；performance 确认没有新增 timer，沿用已有 active-panel transition 和生命周期 fence。

简化评估结论：标题与 panel 共用既有 `is_uart/is_ble` projection 是最小完整修复；不新增可见性状态、事件总线、动画模块或业务 facade。

### 验证与未运行项目

```text
scripts/check.ps1                                      PASS
.venv\Scripts\python.exe -m compileall -q src          PASS
UI177_TRANSPORT_SECTION_PASS theme=star_trail modes=6 parity=pass
UI177_TRANSPORT_SECTION_PASS theme=moonlit_ocean modes=6 parity=pass
UI177_TRANSPORT_SECTION_PASS theme=sakura_night modes=6 parity=pass
UI177_TRANSPORT_SECTION_VECTOR_PASS themes=3 modes=6 startup=pass lifecycle=pass
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根的短时 Qt offscreen 实例，未显示主窗口、未启动 EXE/后台服务、
未接入硬件或网络。环境提示缺失 PySide6 虚拟字体目录；未据此宣称 Windows 字体/HIDPI/真实窗口通过。完整 GUI/HIDPI/读屏/视觉帧差分、
EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.77 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.77` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.77`
- size：`47,892,808` bytes
- SHA-256：`CBA24073D49B6CA1C6FDC82C873DEBE19FBCB619D2C2CC4580CD5406D9772A88`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.76 Preset Activity Rail

### 用户结果与范围

- 用户在连接页选择内置或自定义快速配置后，既有 `ConnectionPresetContextSurface` 的底部 rail 会短暂扫过一次，提供明确的“选择已接收”反馈。
- 清空快速配置也使用同一 shared activity window；它只表达 UI selection acknowledgement，不表示已连接、不自动连接、不保存配置。
- 低动效、显式暂停、隐藏、最小化和关闭保持静态；combo popup、当前值、apply callback、tooltip/AccessibleDescription、焦点和 Tab 顺序不变。

### 实际修改文件

- `src/serialforge/presentation/controllers/connection_builder.py`：在既有 preset selection owner 完成 projection 后请求 `request_activity(420)`。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0063-preset-activity-rail.md`、`tasks/plan.md`、`tasks/todo.md`、
  `README.md`：记录共享动效时钟边界、静态回退与交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019febb3-6c48-7561-afd1-3a55aa12cf80  called before source edit; wait timed out; closed
架构角色       019febb3-6c98-7423-9ae1-61eb509607db  called before source edit; wait timed out; closed
UI 设计角色    019febb3-6ce3-7b80-8208-8c3e2790390c  called before source edit; wait timed out; closed
开发角色       019febb3-6d30-7853-94dc-a561a9c720b8  called before source edit; wait timed out; closed
验证角色       019febb3-6d86-7f31-9264-658692d09200  called before source edit; wait timed out; closed
打包/流程角色  019febb3-6dcf-79b3-ba09-d298ae2b5c1a  called before source edit; wait timed out; closed
独立质量复核   019febb5-25aa-7640-9acd-9d66d3f0157c  called after implementation; wait timed out; closed
```

六角色与独立复核均未返回完整报告，未把超时当成通过。父代理完成五轴审查：correctness 确认选择/清空路径只增加 shared activity request；
readability/simplicity 确认只增加一个边界调用，不复制 rail 绘制或状态机；architecture 确认 MotionController 仍是唯一 presentation clock，
context surface 不反向依赖 controller；security 确认没有输入、存储、网络、密钥或依赖变化；performance 确认活动窗口最多 420ms，复用已有
96ms timer 并受 reduced-motion/暂停/生命周期 fence 约束。

简化评估结论：复用既有 `request_activity()` 是最小完整实现；不新增 `QTimer`、transition object、业务状态字段、selection event bus 或独立动画模块。

### 验证与未运行项目

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI176_PRESET_ACTIVITY_PASS theme=star_trail frames=2 clear=pass
UI176_PRESET_ACTIVITY_PASS theme=moonlit_ocean frames=3 clear=pass
UI176_PRESET_ACTIVITY_PASS theme=sakura_night frames=3 clear=pass
UI176_REDUCED_MOTION_PASS activity=static
UI176_PAUSED_MOTION_PASS activity=static
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根的短时 Qt offscreen 实例，未显示主窗口、未启动 EXE/后台服务/持续 GUI、
未接入硬件或网络。环境提示缺失 PySide6 虚拟字体目录；未据此宣称 Windows 字体/HIDPI/真实窗口通过。完整 GUI/HIDPI/读屏/视觉帧差分、
EXE 启动、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.76 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.76` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.76`
- size：`47,892,668` bytes
- SHA-256：`F726E12FD00E0718FC3CBA66883CC783018244184A6F54CA493C81FAE9508593`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.75 Preset Metadata Surface

### 用户结果与范围

- 自定义连接配置编辑 dialog 的名称/备注字段现在位于 `presetMetadataFields` 主题 surface 内，和主连接页的 section/field/hint 层级一致。
- “名称”“备注”字段标签使用 `QLabel[role="muted"]`，输入仍为原生 bounded `QLineEdit`；不再依赖系统 palette 的裸 label 对比度。
- `ConnectionPresetMetadata`、key 生成、空名称/长度校验、保存/取消 action、初始 label focus、Tab/accessibility、连接 session gate 与上一轮 150ms dialog 淡入均保持不变。

### 实际修改文件

- `src/serialforge/presentation/connection_preset_editor.py`：新增 bounded metadata surface 与局部 muted field label helper。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0062-preset-metadata-surface.md`、`tasks/plan.md`、`tasks/todo.md`、
  `README.md`：记录 owner、主题契约、验证和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019febab-a9b8-78f3-96b2-dcf7480985da  called before source edit; wait timed out; closed
架构角色       019febab-a9fb-7e50-977a-1898eda3e6ad  called before source edit; wait timed out; closed
UI 设计角色    019febab-aa4c-7de2-a064-edee5214da90  called before source edit; wait timed out; closed
开发角色       019febab-aa9c-7152-9510-d25f9be76d4b  called before source edit; wait timed out; closed
验证角色       019febab-aae4-77d2-b800-e692bf9ea2ae  called before source edit; wait timed out; closed
打包/流程角色  019febab-ab32-7c33-bd09-f7da51455795  called before source edit; wait timed out; closed
独立质量复核   019febad-460e-7ac2-9e75-1b64c0b01172  called after implementation; wait timed out; closed
```

六角色与独立复核均未返回完整报告，未把超时当成通过。父代理完成五轴审查：correctness 确认只包装 layout，不改变 DTO/校验/action；
readability/simplicity 确认仅新增一个局部 helper 与一个可复用既有 selector 的 surface；architecture 确认变化停留在 presentation，
controller/store 依赖不变；security 确认没有输入边界、存储、网络、密钥或依赖变化；performance 确认只增加静态 QFrame/QLabel，
不增加 timer、动画或热路径。

简化评估结论：复用 `QWidget[role="surface"]` 和已有 `QLabel[role="muted"]` 是最小完整方案；不引入 dialog 基类、全局表单工厂、
新增 token、第二套 QSS 或额外动效。

### 验证与未运行项目

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI175_PRESET_SURFACE_PASS theme=star_trail fields=2 focus=label validation=pass
UI175_PRESET_SURFACE_PASS theme=moonlit_ocean fields=2 focus=label validation=pass
UI175_PRESET_SURFACE_PASS theme=sakura_night fields=2 focus=label validation=pass
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根的短时 Qt offscreen 实例，仅构造/显示/隐藏自定义 preset dialog，
未显示主窗口、未启动 EXE/后台服务/持续 GUI、未接入硬件或网络。offscreen 环境提示缺失 PySide6 虚拟字体目录及
`propagateSizeHints()` 不支持；未据此宣称 Windows 字体/HIDPI/真实窗口通过。完整 GUI/HIDPI/读屏/视觉帧差分、EXE 启动、真实
UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.75 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.75` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.75`
- size：`47,891,717` bytes
- SHA-256：`01E1C090EA020523DCDA4D63FEAEBB1E84BD4EF48B27CE598CE78EBC4F1072D4`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.74 Dialog Entrance Transition

### 用户结果与范围

- 批量命令编辑器和自定义连接 preset editor 在显示时使用一次性 150ms opacity 淡入，opacity 从 `0.90` 到 `1.0`，曲线为 `OutCubic`。
- 正常动效展示过渡；低动效环境与显式暂停保持静态显示；dialog hide/close 会停止动画、恢复 opacity 并解除临时 graphics effect。
- 已有 graphics effect 不会被覆盖；draft、校验、按钮 action、popup、焦点、Tab/accessibility、尺寸和 native `QMessageBox`/`QFileDialog` 行为保持不变。

### 实际修改文件

- `src/serialforge/presentation/dialog_transition.py`：新增自定义 dialog 一次性入口过渡与生命周期清理 owner。
- `src/serialforge/presentation/command_batch_editor.py`：接入 show/hide transition hook。
- `src/serialforge/presentation/connection_preset_editor.py`：接入 show/hide transition hook。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0061-dialog-entrance-transition.md`、`tasks/plan.md`、
  `tasks/todo.md`、`README.md`：固化模块边界、参数和交接记录。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feba1-478d-7873-baf5-b0a740a4f3e0  called before source edit; wait timed out; closed
架构角色       019feba1-47d6-7e50-9ea3-a64186215d79  called before source edit; wait timed out; closed
UI 设计角色    019feba1-481e-7300-af1a-93d002ecc927  called before source edit; wait timed out; closed
开发角色       019feba1-4870-7022-8a58-94dafb91de85  called before source edit; wait timed out; closed
验证角色       019feba1-48ba-78f2-b72a-0965e2005731  called before source edit; wait timed out; closed
打包/流程角色  019feba1-490b-7022-8a58-94dafb91de85  called before source edit; wait timed out; closed
独立质量复核   019feba2-e2bb-7223-a7f4-5a4c3755d454  called after implementation; wait timed out; closed
```

六角色与独立复核均在运行时窗口内超时，未把超时当成通过。父代理完成五轴审查：correctness 确认 show/hide 与 effect 生命周期配对；
readability/simplicity 确认单一小模块和两个明确 owner hook；architecture 确认只位于 presentation 并复用 motion policy；
security 确认没有输入、存储、网络、密钥或依赖变化；performance 确认每次显示最多一段 150ms 动画，不创建 timer/循环或热路径状态。

简化评估结论：复用已有 `decorative_motion_enabled()` 和一次性 `QPropertyAnimation` 是最小完整实现；不引入全局 dialog 基类、
第二套 motion clock、常驻 timer、业务 transition state 或 native dialog 替换。

### 验证与未运行项目

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI174_DIALOG_TRANSITION_PASS theme=star_trail dialog=command animation=active
UI174_DIALOG_TRANSITION_PASS theme=star_trail dialog=preset animation=active
UI174_DIALOG_TRANSITION_PASS theme=moonlit_ocean dialog=command animation=active
UI174_DIALOG_TRANSITION_PASS theme=moonlit_ocean dialog=preset animation=active
UI174_DIALOG_TRANSITION_PASS theme=sakura_night dialog=command animation=active
UI174_DIALOG_TRANSITION_PASS theme=sakura_night dialog=preset animation=active
UI174_REDUCED_MOTION_PASS dialog=command animation=static
UI174_REDUCED_MOTION_PASS dialog=preset animation=static
UI174_PAUSED_MOTION_PASS dialog=preset animation=static
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根的短时 Qt offscreen 实例，仅构造并显示/隐藏两个 dialog，
未显示主窗口、未启动 EXE/后台服务/持续 GUI、未接入硬件或网络。offscreen 环境提示缺失 PySide6 虚拟字体目录及
`propagateSizeHints()` 不支持；未据此宣称 Windows 字体/HIDPI/真实窗口通过。完整 GUI/HIDPI/读屏/视觉帧差分、EXE 启动、真实
UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.74 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.74` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.74`
- size：`47,891,949` bytes
- SHA-256：`5514973AF8FB8275A8510DA7159523E494370902114FBF091CF4FC8FF0F57916`
- archive listing SHA-256：`29EC8DE9EE334CB8ADAE1802391D18BA980954D8901F7C500841CD70109A720D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.73 Terminal Display Field Label Hierarchy

### 用户结果与范围

- 实时观测工具栏的“显示”字段现在使用 `QLabel[role="muted"]`，与终端显示模式 combo 形成清晰的字段/控件层级。
- `实时观测` section 继续是 `role="section"`；暂停、记录和接收活动状态继续是 `role="status"`，不被普通字段 helper 降级。
- display mode combo 的 2 个选项、`currentIndexChanged` → `_rerender_preview`、暂停/记录动作、快捷键、焦点、Tab 顺序和布局保持不变。

### 实际修改文件

- `src/serialforge/presentation/controllers/terminal.py`：新增局部 `_field_label()`，替换裸“显示”标签。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0060-terminal-display-field-label-hierarchy.md`、`tasks/plan.md`、
  `tasks/todo.md`、`README.md`：记录模块边界、主题契约和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb98-8f9f-7442-8641-03f05dc4a76d  called before source edit; wait timed out; closed
架构角色       019feb98-8fe9-7a02-a234-f6cb2c885044  called before source edit; wait timed out; closed
UI 设计角色    019feb98-9032-7de3-ba1f-9f7c4ffcc82e  called before source edit; wait timed out; closed
开发角色       019feb98-9080-7353-8d72-0ab8d5cff758  called before source edit; wait timed out; closed
验证角色       019feb98-90d0-7581-9cc2-e058b26ad90b  called before source edit; wait timed out; closed
打包/流程角色  019feb98-9123-7382-8234-cb9c67833d42  called before source edit; wait timed out; closed
独立质量复核   019feb9a-75d9-7892-86df-dd257735b192  called after implementation; wait timed out; closed
父代理         bounded audit GO：helper 只创建 QLabel/设置 role；单点替换不触碰 display mode、状态 signal、快捷键、focus、Tab 或布局
简化评估       4 行局部 helper 复用既有 `QLabel[role="muted"]` selector；不新增 shared form factory、依赖、timer、状态或动画
```

六角色和独立复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理按 correctness/readability/architecture/security/performance
五轴完成静态审查。审查重点包括 terminal.py 行数、Qt owner 边界、原生 combo 交互和系统 palette fallback。

### 验证与未运行项目

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI173_TERMINAL_TOOLBAR_PASS theme=star_trail labels=5 display_modes=2
UI173_TERMINAL_TOOLBAR_PASS theme=moonlit_ocean labels=5 display_modes=2
UI173_TERMINAL_TOOLBAR_PASS theme=sakura_night labels=5 display_modes=2
```

vector 使用真实 `create_application()` + `create_main_window()` 组合根，仅在 Qt offscreen 短时构建窗口并关闭，未启动 SerialForge 主窗口、
EXE、后台服务或持续 GUI。环境提示 PySide6 虚拟字体目录缺失；未据此宣称完整字体/HIDPI 通过。完整 GUI/HIDPI/读屏/视觉帧差分、EXE 启动、
真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.73 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.73` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.73`
- size：`47,888,984` bytes
- SHA-256：`6D9EDF9E9AC4B717450260C83E6EB34D144A390FF59F74B45AA8E16AC4283AE9`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.72 Command Editor Field Label Hierarchy

### 用户结果与范围

- 批量命令编辑对话框的 5 个静态字段/辅助标签现在统一使用 `QLabel[role="muted"]`：名称、快捷命令、固定顺序说明、当前步骤和延时。
- `_table_empty` 仍为 `role="subtle"`，错误 label 仍为 `role="error"`；空态、错误、编辑校验、popup、焦点和 Tab 顺序没有被批量改写。
- 这是 presentation-only 的主题层级修复，不改变 `_DraftStep`、bounded text payload、CommandBatch 规则、按钮 action 或保存结果。

### 实际修改文件

- `src/serialforge/presentation/command_batch_editor.py`：新增局部 `_field_label()`，替换 5 个静态字段/说明标签。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0059-command-editor-field-label-hierarchy.md`、`tasks/plan.md`、
  `tasks/todo.md`、`README.md`：记录边界、主题契约和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb91-c936-7b60-99bf-7e4ceef14360  called before source edit; wait timed out; closed
架构角色       019feb91-c984-7d92-be1c-ef66a4f3c49c  called before source edit; wait timed out; closed
UI 设计角色    019feb91-c9d0-76f2-8558-e0630ee75c28  called before source edit; wait timed out; closed
开发角色       019feb91-ca22-7a31-a4a9-6fe80fc606f5  called before source edit; wait timed out; closed
验证角色       019feb91-ca6b-7f52-b8e1-583999a2dac5  called before source edit; wait timed out; closed
打包/流程角色  019feb91-cab7-7813-b084-ef975a717a2f  called before source edit; wait timed out; closed
独立质量复核   019feb93-267a-7f63-8d16-d9eff1471964  called after implementation; wait timed out; closed
父代理         bounded audit GO：helper 只创建 QLabel/设置 role；5 个替换不触碰 draft、validation、signals、popup、focus、Tab 或错误/空态角色
简化评估       局部 helper 复用既有 `QLabel[role="muted"]` selector；不新增全局表单工厂、依赖、timer、状态源或动画
```

六角色和独立复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理按 correctness/readability/architecture/security/performance
五轴完成静态审查。审查重点包括 576 行文件门禁、对话框原生交互、bounded payload 校验、空态/错误角色和主题 fallback。

### 验证与未运行项目

```text
.venv\Scripts\python.exe -m compileall -q src  PASS
UI172_COMMAND_LABELS_PASS theme=star_trail fields=5 labels=7
UI172_COMMAND_LABELS_PASS theme=moonlit_ocean fields=5 labels=7
UI172_COMMAND_LABELS_PASS theme=sakura_night fields=5 labels=7
```

vector 只创建短时 Qt offscreen `CommandBatchEditorDialog`，使用真实空 batch/default draft，不启动 SerialForge 主窗口、EXE、后台服务或持续 GUI。
环境提示 `QFontDatabase` 找不到虚拟环境字体目录；这不是生产代码错误，也未据此宣称 Windows 字体/HIDPI 通过。完整 GUI/HIDPI/读屏/视觉帧差分、
EXE 启动、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.72 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.72` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.72`
- size：`47,888,957` bytes
- SHA-256：`2688420A64A7BA7CF20877C259EBF6181F9FA46884061F099A416C75FFFF980B`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.71 Protocol Field Label Hierarchy

### 用户结果与范围

- 协议/遥测配置页的 8 个普通字段标签现在统一使用 `QLabel[role="muted"]`：预设、Framing、校验、最大帧(B)、Delimiter Hex、
  长度字节、字节序和过滤。
- 5 个 section 标题继续使用 `role="section"`，5 个状态 owner、hint 和动态说明保持原有语义；没有留下系统白色 label fallback。
- 这是 presentation-only 的语义层级修复，不改变协议配置值、parser/apply/reset、callback、DTO、焦点、Tab 顺序或布局列。

### 实际修改文件

- `src/serialforge/presentation/controllers/protocol.py`：新增局部 `_field_label()` 并替换 8 个普通字段标签。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0058-protocol-field-label-hierarchy.md`、`tasks/plan.md`、
  `tasks/todo.md`、`README.md`：记录模块边界、主题契约和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb88-e8ff-7ea0-976f-2d522663555e  called before source edit; wait timed out; closed
架构角色       019feb88-e94e-79a3-9bc9-be2bd9cb306f  called before source edit; wait timed out; closed
UI 设计角色    019feb88-e9a7-75b3-b3f3-b00d2b87b52b  called before source edit; wait timed out; closed
开发角色       019feb88-e9f1-7231-8d96-379eab93f407  called before source edit; wait timed out; closed
验证角色       019feb88-ea3c-7c62-8957-ec6fbfd2d25b  called before source edit; wait timed out; closed
打包/流程角色  019feb88-ea85-7213-beb2-877a7466556d  called before source edit; wait timed out; closed
独立质量复核   019feb8b-9a95-73f2-9966-87d7c0fc2a7f  called after implementation; wait timed out; closed
父代理         bounded audit GO：helper 只创建 QLabel/设置 role；8 个替换不触碰 widget ref、callback、DTO、状态、焦点、Tab 或布局
简化评估       局部 helper 只有一个职责，复用既有 `QLabel[role="muted"]` stylesheet；不新增全局工厂、依赖、timer 或主题状态源
```

六角色和独立复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理按 correctness/readability/architecture/security/performance
五轴完成静态审查。审查重点包括文件行数、原生控件交互、协议状态 owner 和系统 palette fallback。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  148 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
protocol-page vector                                             PASS  UI171_PROTOCOL_LABELS_PASS × star_trail/moonlit_ocean/sakura_night; fields=8 sections=5 statuses=5
```

第一次短时 vector 脚本把预期状态 owner 数误写为 4；实际 `protocol/component/dataset/curve/replay` 为 5，修正验证断言后重新运行通过。
该过程没有修改生产代码，也没有创建测试资产。vector 只创建短时 Qt offscreen 协议 panel；没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI。
完整 GUI/HIDPI/读屏/真实视觉帧差分、EXE 启动、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.71 打包

本轮已通过 `scripts/package.ps1 -Mode onefile -SourceRevision local-ui-1.71` 重新生成并覆盖根目录文件：

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.71`
- size：`47,888,719` bytes
- SHA-256：`6331DE6F5145CB9670A3CEB4D1379E280EB6099D1FCB155A8C96122B2371A1E4`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.70 Theme Picker Palette Icons

### 用户结果与范围

- 主题选择器的三个原生下拉项现在显示各自 ThemeSpec 的 surface/accent 色盘 icon，用户切换前即可区分星轨霓虹、月影深海、
  樱雾夜航。
- icon 是 18×18 的无外部资源 QPainter renderer，提供 Normal/Selected/Disabled mode；不进入 accessibility 树、不抢焦点、
  不改变 Tab 顺序，不拥有主题偏好或业务状态。
- 原生主题文字、`UserRole` key、tooltip、`currentIndexChanged` 和 `on_theme_changed` owner 完全保留；icon 表达目标主题
  的固定 palette，因此不新增 lifecycle frame/stop 接线。

### 实际修改文件

- `src/serialforge/presentation/theme_picker_icons.py`：新增 ThemeSpec → QIcon 纯 renderer。
- `src/serialforge/presentation/controllers/workspace.py`：主题条目组装完成后注入 icon。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0057-theme-picker-palette-icons.md`、`tasks/plan.md`、
  `tasks/todo.md`、`README.md`：记录边界和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb83-6904-7203-8e10-edb92192f220  called before source edit; wait timed out; closed
架构角色       019feb83-695a-7f02-98d3-a31d6c63451d  called before source edit; wait timed out; closed
UI 设计角色    019feb83-69a4-74d0-8bbb-9b652d8fbb90  called before source edit; wait timed out; closed
开发角色       019feb83-69ee-7320-aba0-1e963968231a  called before source edit; wait timed out; closed
验证角色       019feb83-6a3c-7fb2-b14e-876754732055  called before source edit; wait timed out; closed
打包/流程角色  019feb83-6a8b-72a2-9205-48bc1b8de69a  called before source edit; wait timed out; closed
独立质量复核   019feb84-e8aa-7fe2-bb31-c861ccc8c0bd  called after implementation; wait timed out; closed
父代理         bounded audit GO：renderer 只消费 ThemeSpec；workspace 仅装配 item icon；无新状态、timer、依赖、生命周期 fan-out、
               focus/tab/accessibility 或 combo action 改动
简化评估       一个小型 18×18 renderer 复用现有 Qt boundary 和 ThemeSpec，不新增图片资源、图标服务或第二套主题状态源
```

六角色和独立复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理完成 correctness/readability/architecture/security/
performance 五轴审查。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  148 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
theme picker icon vector                                           PASS  UI170_THEME_ICON_VECTOR_PASS × star_trail/moonlit_ocean/sakura_night; actual=16x16
theme picker import vector                                         PASS  UI170_THEME_PICKER_IMPORT_PASS count=3
provenance.py verify                                               PASS  local-ui-1.70 onefile manifest
root/canonical hash equality                                       PASS
```

vector 验证仅创建短时 Qt offscreen 内存 combo/icon，没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI；完整 GUI/HIDPI/读屏/
真实视觉帧差分、EXE 启动、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、
mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.70 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.70`
- size：`47,888,056` bytes
- SHA-256：`793977A0B46FA2DC61EC7691601CAEC1B0D5C71E6E376B5E08601EEA3DBE6066`
- archive listing SHA-256：`82DC9600ACBCDE584DF62692ED08DD276CB6F20CBB8446A5C88B071DCD3EBC83`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.69 Connection Field Labels

### 用户结果与范围

- UART、网络、BLE 参数区的普通字段标签统一使用 `QLabel[role="muted"]`，在三套主题中保持稳定的次级视觉层级。
- `UART 参数`、`网络端点`、`BLE GATT 设备` 三个 section 标题继续使用 `role="section"`；TCP/RTT/BLE hint、BLE 属性摘要、
  连接状态、快捷键提示没有被批量降级。
- 改动只发生在 `connection_builder.py` 的局部 `_field_label()` 装配 helper，不读取 ViewModel、不接 signal、不写配置、不创建
  timer，不改变 preset/transport/BLE/network 业务、Tab order 或无障碍路径。

### 实际修改文件

- `src/serialforge/presentation/controllers/connection_builder.py`：新增 `_field_label()` 并迁移普通表单字段标签。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0056-connection-field-label-hierarchy.md`、`tasks/plan.md`、
  `tasks/todo.md`、`README.md`：记录层级边界和交付约束。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb7d-a982-7e33-902f-53adb9b292c4  called before source edit; wait timed out; closed
架构角色       019feb7d-a9cc-7221-8e01-5e1834a522b6  called before source edit; wait timed out; closed
UI 设计角色    019feb7d-aa17-7883-930c-5fc3a0a2f7ea  called before source edit; wait timed out; closed
开发角色       019feb7d-aa64-7481-a2e8-e1ca2e15bde9  called before source edit; wait timed out; closed
验证角色       019feb7d-aab3-7bd1-bfc1-13acfc656225  called before source edit; wait timed out; closed
打包/流程角色  019feb7d-aafe-7bb2-9401-d297c8d52d07  called before source edit; wait timed out; closed
独立质量复核   019feb7f-8abd-7b22-a841-3734a85fbab0  called after implementation; wait timed out; closed
父代理         bounded audit GO：局部 helper 只设置 muted presentation property；section/hint/status role、Qt parent、preset/transport/
               BLE/network wiring、Tab order、配置值和业务 owner 保持；无新依赖、状态、timer 或共享工厂
简化评估       用一个十行以内的局部 helper 消除连接 builder 内重复 label property 设置，不新增 QWidget、QSS 分支或生命周期接线
```

六角色和独立复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理完成 correctness/readability/architecture/security/
performance 五轴审查。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  147 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
connection-page vector                                             PASS  UI169_CONNECTION_LABELS_PASS × star_trail/moonlit_ocean/sakura_night; muted=37; sections=3
provenance.py verify                                               PASS  local-ui-1.69 onefile manifest
root/canonical hash equality                                       PASS
```

向量验证仅创建短时 Qt offscreen 内存连接页，没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI；Qt 环境缺失 `.venv` 字体目录
的提示不代表 Windows 字体结论。完整 GUI/HIDPI/读屏/真实视觉帧差分、EXE 启动、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名
和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.69 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.69`
- size：`47,886,264` bytes
- SHA-256：`63CD7F3E0C219B7034C5F18F96ADB04B4A5EAE490831495C3AC6A128AAE2A7E9`
- archive listing SHA-256：`C90B0C21DBC68E1B814A2DE8560D675CDFC90B5881A06AC1C1BDFC727338C600`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.68 Send Shortcut Hint

### 用户结果与范围

- 发送控制带底部新增 `Ctrl+Enter 发送` 主题 keycap，补足既有快捷键的可发现性。
- 提示是静态 `QLabel#sendShortcutHint`，拥有一致的 AccessibleName/Description 和 tooltip；`NoFocus`，不接收动作，
  不进入 Tab 顺序，也不暴露为窗口 facade。
- 默认 stylesheet 与 `star_trail`、`moonlit_ocean`、`sakura_night` 三套 theme override 均覆盖文字、背景、边框和圆角，
  没有系统 palette 白色 fallback；尺寸固定在 176–184 px，兼顾 CSS 内边距和文字宽度。
- `Ctrl+Enter` 注册、send action、send gate、ViewModel、状态投影和共享动效生命周期均保持原 owner 不变。

### 实际修改文件

- `src/serialforge/presentation/controllers/terminal.py`：装配无焦点快捷键提示并设置 176–184 px 宽度边界。
- `src/serialforge/presentation/theme_stylesheet_base.py`：增加默认主题 keycap 语义样式。
- `src/serialforge/presentation/theme_variant_shell.py`：增加三套可选主题的 keycap token 覆盖。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0055-send-shortcut-hint.md`、`tasks/plan.md`、`tasks/todo.md`、
  `README.md`：固化 presentation-only 边界、尺寸/主题约束与交接入口。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb72-b0c9-7181-adf5-2e0a97253fc5  called before source edit; wait timed out; closed
架构角色       019feb72-b113-7ef0-b9c2-517dda3ad7d6  called before source edit; wait timed out; closed
UI 设计角色    019feb72-b169-7a32-a17a-309ccf64978b  called before source edit; wait timed out; closed
开发角色       019feb72-b1bd-7c33-a7b9-b460016732b6  called before source edit; wait timed out; closed
验证角色       019feb72-b212-77a3-9aa7-4d5236c2a724  called before source edit; wait timed out; closed
打包/流程角色  019feb72-b259-7920-9d2b-15113acd882d  called before source edit; wait timed out; closed
架构修正复核   019feb75-6193-7ec0-baa5-017658ef7ed0  called before width/style correction; wait timed out; closed
独立质量复核   019feb77-5dca-7b51-9a9b-9db1e5fb3113  called after implementation; wait timed out; closed
父代理         bounded audit GO：presentation-only QLabel；不新增依赖、状态、timer、事件过滤器或跨层调用；NoFocus、
               role=subtle、AccessibleName/Description、tooltip 和现有 shortcut/action owner 保持；三套主题 token 覆盖；
               760/952/1152 向量确认文字区不裁切
简化评估       直接在既有 send bar 空列装配一个 QLabel，不新增 facade、组件状态模型、快捷键代理或独立动画；主题表现放入
               已有 base/variant stylesheet 边界，未把 QSS 分支塞入 controller
```

六角色、架构修正复核和独立质量复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理完成 correctness/
readability/architecture/security/performance 五轴审查。布局向量首次发现 132 px 上限会裁切约 156 px 字体文案，
随后在同一架构边界内修正为 176–184 px，并以 CSS 内边距计入回归向量。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  147 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
presentation import/layout vector                                 PASS  UI168_SEND_HINT_LAYOUT_PASS width=760,952,1152; NoFocus; role=subtle
theme stylesheet vector                                            PASS  UI168_THEME_OVERRIDE_PASS star_trail/moonlit_ocean/sakura_night
provenance.py verify                                               PASS  local-ui-1.68 onefile manifest
root/canonical hash equality                                       PASS
```

向量验证仅创建短时 Qt offscreen 内存控件，没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI；Qt 环境提示缺失 `.venv` 字体
目录，不代表 Windows 字体结论。完整 GUI/HIDPI/读屏/真实视觉帧差分、EXE 启动、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名
和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A；本轮
为 Python/PySide6 presentation 与 QSS 变更，无 MCU/RTOS/厂商约束适用。

### UI-1.68 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.68`
- size：`47,885,847` bytes
- SHA-256：`A69CE9AD35442595573E2063D9831D57EE9CC7DCE7287D1C8A2D6F0860DBF12E`
- archive listing SHA-256：`C90B0C21DBC68E1B814A2DE8560D675CDFC90B5881A06AC1C1BDFC727338C600`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.67 Header Responsive Shell

### 用户结果与范围

- Header 从单行重组为“品牌层 + 控制层”两层 shell：品牌层承载徽记、wordmark、副标题、信号场；控制层承载连接状态、
  动效偏好和主题选择器。
- 952px 内容宽度（980px 最小窗口扣除根边距）和 1152px 内容宽度（1180px 常用窗口扣除根边距）下，两行均保持在 Header
  边界内，不依赖横向滚动或裁切。
- 现有连接状态、动效开关、主题切换、`window._*` facade、signals、Tab order、无障碍和共享 MotionController 语义保持。

### 实际修改文件

- `src/serialforge/presentation/controllers/workspace.py`：只调整 `build_app_header()` 的外层/内层 layout 归属和间距。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0054-header-responsive-shell.md`、`tasks/plan.md`、`tasks/todo.md`、
  `README.md`：同步响应式边界、ADR 和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb6a-964c-7be1-bba3-7442dddd8d57  called before source edit; wait timed out; closed
架构角色       019feb6a-96a1-7732-a05f-581a2dce6852  called before source edit; wait timed out; closed
UI 设计角色    019feb6a-96ed-7081-8920-0407c4532147  called before source edit; wait timed out; closed
开发角色       019feb6a-973e-7173-a9c6-ad9296048f2a  called before source edit; wait timed out; closed
验证角色       019feb6a-9784-7d43-a644-f4ea69cc03ff  called before source edit; wait timed out; closed
打包/流程角色  019feb6a-97d6-7e91-afc5-362e475c7086  called before source edit; wait timed out; closed
独立质量复核   019feb6c-c74f-72c3-aeb6-77d862465cf5  called after implementation; wait timed out; closed
父代理         bounded audit GO：只改变 layout ownership/geometry；品牌、状态、动效、主题和 Tab order 既有 owner 保持；两行 geometry 向量在 952/1152 宽度内；无新状态、timer、依赖或跨层访问
简化评估       不新增 resize controller、事件过滤器、隐藏控件状态或 QSS 分支；直接用一个外层 QVBoxLayout 加两个职责清晰的 QHBoxLayout
```

六角色和独立复核均在运行时窗口内超时后关闭，未把超时写成子代理通过；父代理按 correctness/readability/architecture/security/
performance 五轴完成 bounded audit。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  147 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
presentation import vector                                        PASS  UI167_FINAL_IMPORT_PASS
layout vector                                                      PASS  UI167_HEADER_LAYOUT_PASS width=952 and width=1152; two rows in bounds
provenance.py verify                                               PASS  final local-ui-1.67 onefile manifest
root/canonical hash equality                                       PASS
```

layout vector 仅创建 Header 相关 Qt widgets 并读取内存 geometry，没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI；Qt 离屏
环境字体目录提示不代表 Windows 字体结论。完整 GUI/HIDPI/读屏/真实视觉帧差分、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名和
正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A；本轮为
Python/PySide6 presentation 布局变更，无 MCU/RTOS/厂商约束适用。

### UI-1.67 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.67`
- size：`47,886,009` bytes
- SHA-256：`0E25AF8B646F9FAF1A19D569D025C588258FA952A3B2E6688EA923B794265DFA`
- archive listing SHA-256：`C90B0C21DBC68E1B814A2DE8560D675CDFC90B5881A06AC1C1BDFC727338C600`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.66 Header Brand Mark

### 用户结果与范围

- Header 的 `SERIALFORGE` wordmark 前新增 38×38 星轨/S 几何徽记，补足首屏品牌锚点，保持现有三套主题的二次元氛围。
- 徽记只使用 `ThemeSpec` 语义色和 QPainter 几何，无角色、IP、GIF、字体、SVG/PNG、网络或第三方资源。
- 动效复用已有共享 MotionController；低动效、暂停、隐藏、最小化、关闭都回到静态徽记，wordmark、状态胶囊、主题选择器、
  焦点和 accessibility 契约保持不变。

### 实际修改文件

- `src/serialforge/presentation/brand_mark_surface.py`：新增固定尺寸、主题化、可停止的几何徽记 renderer。
- `src/serialforge/presentation/controllers/workspace.py`：只在 Header 组装徽记并放置于 wordmark 前。
- `src/serialforge/presentation/controllers/lifecycle.py`：把 `_brand_mark` 纳入既有共享 frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0053-header-brand-mark.md`、`tasks/plan.md`、`tasks/todo.md`、`README.md`：同步架构边界和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb60-bdc1-7153-b8ee-d70c3dec7894  called before source edit; wait timed out; closed
架构角色       019feb60-be12-7ef2-8c08-e43cc86bf8a4  called before source edit; wait timed out; closed
UI 设计角色    019feb60-be5f-7fb1-80c6-11dd52ca5178  called before source edit; wait timed out; closed
开发角色       019feb60-beba-7743-9d08-d70b33183a57  called before source edit; wait timed out; closed
验证角色       019feb60-bf06-7992-9189-f840c5db3bc8  called before source edit; wait timed out; closed
打包/流程角色  019feb60-bf56-71c2-b946-dd257e8a992a8  called before source edit; wait timed out; closed
架构防护复核   019feb62-d568-7012-8814-7321a0e1e1d0  called before Header/lifecycle integration; wait timed out; closed
架构修正复核   019feb64-399d-7772-92bd-7b7a1a669fbb  called before import-order correction; wait timed out; closed
独立质量复核   019feb65-a332-7d51-9366-e02fc607f4d7  called after implementation; wait timed out; closed
父代理         bounded audit GO：BrandMarkSurface 仅依赖 Qt/ThemeSpec/math；固定 38×38；NoFocus、鼠标透明、空 accessibility；workspace 只装配，lifecycle 只复用共享 frame/stop；无业务状态、输入、外部资源、独立 timer 或循环依赖
简化评估       用一个 presentation-only renderer 复用既有 MotionController 和 ThemeSpec；没有把品牌绘制逻辑塞入 workspace/lifecycle，也没有新增资源管线或状态模型
```

六角色、架构防护复核和独立质量复核均在运行时窗口内超时后关闭，未将超时写成子代理通过；父代理按 correctness/readability/
architecture/security/performance 五轴完成 bounded audit。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  147 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
presentation import vector                                        PASS  UI166_PRESENTATION_IMPORT_PASS; workspace/lifecycle import
renderer vector                                                     PASS  UI166_BRAND_MARK_VECTOR_PASS; 三主题、frame、stop、中心像素和 NoFocus
provenance.py verify                                               PASS  final local-ui-1.66 onefile manifest
root/canonical hash equality                                       PASS
```

vector 检查使用短时 Qt offscreen platform 渲染内存中的 38×38 QImage，没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI；
离屏环境字体目录提示不影响几何徽记验证，也不能替代 Windows 字体/HIDPI/读屏/真实视觉验收。真实 UART/TCP/BLE/RTT/J-Link、
OTA、硬件、签名和正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++
适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。

### UI-1.66 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.66`
- size：`47,885,823` bytes
- SHA-256：`8D7418326887039F0A1E53889A653D485F4DCC0A7E7D082B67B90EB0437D1582`
- archive listing SHA-256：`C90B0C21DBC68E1B814A2DE8560D675CDFC90B5881A06AC1C1BDFC727338C600`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.65 Workspace Tab Glyphs

### 用户结果与范围

- 原生 workspace Tab 现在有链路、协议、命令三枚主题化矢量 glyph，帮助用户在紧凑窗口中快速识别工作域。
- 每个 glyph 提供 Normal/Selected/Disabled 三态，并在首次组装和主题切换后重新生成；三套主题均走 `ThemeSpec` 语义色。
- Tab 的原生中文文案、index、`currentChanged`、焦点、键盘导航和 accessibility 语义保持；没有新增导航模型、业务状态或常驻动效时钟。

### 实际修改文件

- `src/serialforge/presentation/workspace_tab_icons.py`：新增 bounded QPainter glyph renderer 与三态 QIcon 刷新入口。
- `src/serialforge/presentation/qt.py`：在现有 presentation Qt 边界暴露 `QIcon`、`QPixmap`。
- `src/serialforge/presentation/controllers/workspace.py`：三个 Tab 组装完成后注入主题化 glyph。
- `src/serialforge/presentation/controllers/lifecycle.py`：主题应用完成后刷新 Tab glyph。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0052-workspace-tab-glyphs.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界、约束、ADR 和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb55-e16e-7881-8138-948bb9060b93  called before source edit; wait timed out; closed
架构角色       019feb55-e1bf-7e13-b5f0-b5e4569080ab  called before source edit; wait timed out; closed
UI 设计角色    019feb55-e214-78b0-a4da-83e5655afaf9  called before source edit; wait timed out; closed
开发角色       019feb55-e262-7fc2-80e5-246397160923  called before source edit; wait timed out; closed
验证角色       019feb55-e2aa-7f73-806a-7c16762c29c7  called before source edit; wait timed out; closed
打包/流程角色  019feb55-e2f6-7fc0-9a18-fdb262d334a8  called before source edit; wait timed out; closed
独立质量复核   019feb5a-71ce-7ed2-a872-7bf519e5865c  called after implementation; wait timed out; closed
父代理         bounded audit GO：glyph 只依赖 Qt/ThemeSpec；三态 QIcon 有界；tabs=None/少于三页安全返回；不改变原生 Tab 文案/index/signal/focus/keyboard/accessibility；无输入边界、外部资源、业务状态或常驻 timer；每个相关文件均小于 1000 行
简化评估       新增单一 `workspace_tab_icons.py` owner，避免把 glyph 分支、主题刷新和绘制逻辑塞回 controller；复用现有 `ThemeSpec`，未引入依赖或第二套导航控件
```

独立复核角色在运行时窗口内未返回，已关闭；未把超时当作通过，父代理按 correctness/readability/architecture/security/performance 五轴完成 bounded audit。

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  146 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
presentation import vector                                        PASS  UI165_PRESENTATION_IMPORT_PASS
renderer vector                                                     PASS  UI165_TAB_ICON_VECTOR_PASS; 三主题、三 Tab、非空 18×18 actualSize、原始标签保持
provenance.py verify                                               PASS  final local-ui-1.65 onefile manifest
root/canonical hash equality                                       PASS
```

短时 vector 检查使用 Qt offscreen platform 但没有启动 SerialForge 主窗口、EXE、后台服务或持续 GUI；Qt 报告环境缺少字体目录，
不影响图标非空/标签契约结果，也不能替代 Windows 字体/HIDPI/读屏视觉验收。真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名和
正式发行验收未运行；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A；本轮
为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。

### UI-1.65 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.65`
- size：`47,882,181` bytes
- SHA-256：`3BC6E142BD96E58DA3F7B3E71412A6C99051EB4887E8E06E05BC9B0573653FAD`
- archive listing SHA-256：`44E3DDCFEEA85187726432B678EBC54FF0FD8EEC3FF622A9B7B26425BED3FEC2`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.64 Transport Panel Transition

### 用户结果与范围

- 切换 UART、TCP Client、TCP Server、UDP、BLE GATT、J-Link RTT 时，当前连接配置 panel 在已完成既有显隐 projection 后进行低干扰 160ms fade，减少突然跳变。
- 连接配置字段显隐、默认值、连接 gate、transport 选择、Tab 顺序和无障碍仍由原 controller/原生控件负责；动画没有业务状态、连接动作或数据事实。
- 低动效、暂停、主题切换、隐藏、最小化、关闭和快速切换会停止并清理 effect/animation；已有其他 `graphicsEffect()` 不会被接管。

### 实际修改文件

- `src/serialforge/presentation/motion_policy.py`：抽出共享 presentation-only reduced-motion/motion-paused 判定。
- `src/serialforge/presentation/transport_panel_transition.py`：新增单一 transport panel fade owner，最多维护一组 effect/animation。
- `src/serialforge/presentation/controllers/connection_runtime.py`：在原有 panel 显隐完成后传入当前 panel，不改变连接业务。
- `src/serialforge/presentation/controllers/workspace_runtime.py`：复用共享 motion policy，保持既有 workspace transition 行为。
- `src/serialforge/presentation/controllers/bootstrap.py`：初始化 transition 引用字段。
- `src/serialforge/presentation/controllers/lifecycle.py`：主题/动效/窗口生命周期清理 transition。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0051-transport-panel-transition.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构、约束、ADR 和交付状态。

### 六角色评审、独立复核与简化评估

```text
产品角色       019feb4b-2104-7222-b9bd-140d78902bcf  called before source edit; wait timed out; closed
架构角色       019feb4b-2151-7510-ae3b-4d9d7b9c8420  called before source edit; wait timed out; closed
UI 设计角色    019feb4b-219b-73e2-a25b-008d531c54fb  called before source edit; wait timed out; closed
开发角色       019feb4b-21e9-7081-a034-6d251157259f  called before source edit; wait timed out; closed
验证角色       019feb4b-2237-7062-b5fc-7cd5ae746d2a  called before source edit; wait timed out; closed
打包/流程角色  019feb4b-2293-7c72-a088-2d041f734fd4  called before source edit; wait timed out; closed
独立质量复核   019feb4e-347f-7ae2-9105-3953373c102a  called after implementation; wait timed out; closed
架构防护复核   019feb4f-b43b-7910-8413-15e885f53fab  called before effect-ownership fix; wait timed out; closed
父代理         bounded audit GO：correctness/readability/architecture/security/performance 五轴无阻断项；panel visibility/connection gate 仍由 connection_runtime owner；transition 只持有 presentation effect/animation，快速切换为最后一轮，lifecycle 清理路径完整
简化评估       抽出 motion_policy 消除 workspace 与 transport transition 的重复偏好判断；不新增 MotionController、常驻 QTimer、transport state 或业务 controller
```

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  145 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
python -m compileall -q src                                      PASS
presentation import vector                                        PASS  UI164_PRESENTATION_IMPORT_PASS
provenance.py verify                                               PASS  final local-ui-1.64 onefile manifest
root/canonical hash equality                                       PASS
```

按项目约束，本轮没有启动 GUI/EXE、后台服务或真实设备，也未运行 offscreen/HIDPI/读屏/硬件验收；因此 transport fade 的实际视觉帧差分、Windows 字体和设备行为仍属于待用户授权项目，不能写成已通过。未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。

### UI-1.64 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.64`
- size：`47,878,874` bytes
- canonical/root SHA-256：`58CD1A56BF21ADDA900DCFF333C8D47FE7343E15A6DA567A118FAD4C831D4EDA`
- archive listing SHA-256：`76EEB528D705FA474BF5278D5255D0C51C3573EDCFEE7F028C983D25EBCC6CEF`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.63 ObservationViewport

### 用户结果与范围

- Component/Dataset 预览从无装饰的普通 `QPlainTextEdit` 升级为共享 `ObservationViewport`，补齐上下观测标尺、扫描信号和 scope 节点；Component 使用紫色 accent，Dataset 使用 mint/accent，Terminal 保持既有 blue scope。
- 三类视口仍是原生 `QPlainTextEdit` 语义：controller 继续负责 `setPlainText`/`appendPlainText`/`clear`、文本上限、placeholder、滚动和 AccessibleName/Description；renderer 不保存 raw frame、解析状态或 Dataset sample。
- 动效只消费 lifecycle 的共享 `MotionController` frame/stop；无本地 timer，reduced-motion、暂停、隐藏、最小化和关闭时静态回退。

### 实际修改文件

- `src/serialforge/presentation/observation_viewport.py`：新增共享原生文本观察视口 renderer，限定 terminal/component/dataset 三个 scope。
- `src/serialforge/presentation/terminal_surface.py`：`TerminalViewport` 改为 `ObservationViewport(scope="terminal")` 兼容子类，保留既有类型/API。
- `src/serialforge/presentation/controllers/protocol.py`：Component/Dataset 预览使用共享 renderer，保留原控件配置与数据责任。
- `src/serialforge/presentation/controllers/lifecycle.py`：三个预览纳入既有共享 MotionController fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0050-observation-viewport.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界、工程约束、ADR 和交付状态。

### 架构审查、独立复核与简化评估

```text
架构角色  019feb3d-0086-7663-97c5-d59fab58391e  called before source edit; wait timed out; closed
独立质量复核  019feb41-569c-7ec2-a5da-07155e5ab811  called after implementation; wait timed out; closed
简化架构角色  019feb43-6a6d-7530-8629-9fb9a284aaef  called before paint-path simplification; wait timed out; closed
父代理    bounded audit GO：ObservationViewport 只装饰原生 QPlainTextEdit；scope 只影响 accent/节点数量；文本、滚动、placeholder、无障碍、controller 更新和 TerminalViewport 类型兼容保持；lifecycle 仍是唯一 frame/stop owner
简化评估  将已解析的 ThemeSpec.surface_input 作为私有绘制参数传入，删除节点循环中的重复 theme_spec_for_widget()；未新增状态、controller、timer 或依赖
```

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  143 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
UI163_OBSERVATION_VIEWPORT_SMOKE  PASS  三 scope 类型/原生文本 API/共享 motion membership；motion_surface_count=34；component_motion_delta=84；三主题 near_white=0；980/1180 protocol page horizontalScrollBar.maximum=0
provenance.py verify              PASS  final local-ui-1.63 onefile manifest
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 系统字体结论；未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。独立复核角色已调用但因运行时超时关闭，以上结论由父代理按五维清单完成复核并记录。

### UI-1.63 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.63`
- size：`47,874,535` bytes
- canonical/root SHA-256：`55D2682DA194658480DDCB73996D832C12568704A923696967013550BC1CF1BF`
- archive listing SHA-256：`1237F26EAB85E74ACEA75E347C29155E49813297F741AADA75517D8431EAF387`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.62 链路传输模式 glyph

### 用户结果与范围

- 链路页“传输”选择旁新增 42×26 的主题化模式 glyph：UART、TCP Client、TCP Server、UDP、BLE GATT、J-Link RTT 各有独立几何识别。
- glyph 只镜像既有 `_transport_combo.currentData()`；原生 ComboBox 仍是唯一的选择、键盘和无障碍文案 owner，不增加配置步骤，不自动连接。
- 模式 glyph 使用共享 MotionController frame/stop，低动效、暂停、隐藏、最小化和关闭时静态回退；连接状态、面板显隐、preset、header context、connection hint 和配置 builder 语义保持不变。
- 三主题 1180×780 截图无近白像素，980/1180 链路页无横向滚动。

### 实际修改文件

- `src/serialforge/presentation/transport_mode_surface.py`：新增 presentation-only 六模式 glyph 和共享 frame/stop API。
- `src/serialforge/presentation/controllers/connection_builder.py`：将 glyph 放入既有“传输”caption，并为 combo 保留 `transportCombo` objectName。
- `src/serialforge/presentation/controllers/connection_runtime.py`：在既有 transport selection projection 后同步 glyph mode。
- `src/serialforge/presentation/controllers/lifecycle.py`：纳入既有 `_motion_surfaces()` fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0049-transport-mode-glyph.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界。

### 架构审查与父代理整合

```text
架构角色  019feb32-d9d8-77c3-a95a-21efc687f99f  called before source edit; wait timed out; closed
独立质量复核  019feb38-22e0-7fa1-8daa-f05dca1d65d5  called after implementation; wait timed out; closed
父代理    bounded audit GO：glyph 只接收 combo mode 与 shared frame；header/context/hint/combo 保持 owner；无连接副作用、无本地 timer、六模式映射完整、绘制边界安全、980/1180 无横向滚动
简化评估  未新建 transport context state/controller；用单一小型 renderer 提供识别信号，避免复制 preset/SessionState/hint 事实
```

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  142 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
UI162_TRANSPORT_MODE_SMOKE       PASS  six modes; panel visibility; shared motion membership; frame/stop; 980/1180 no horizontal scroll
UI162_TRANSPORT_MOTION_DELTA     PASS  shared-frame phase changes produced 342 changed glyph pixels
UI162_TRANSPORT_PIXEL_AUDIT      PASS  star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0
provenance.py verify              PASS  current onefile manifest
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。独立复核角色已调用但因运行时超时关闭，以上结论由父代理按五维清单完成复核并记录。

### UI-1.62 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.62`
- size：`47,871,279` bytes
- canonical/root SHA-256：`8331F98A3ED790D589C556641EBE3E31B924B535ED4A5748D8ED716A893AEECD`
- archive listing SHA-256：`40BF58857031C5CDF9DA8E661A47E0C28F54E9E66339ABE103E85778B7E5B60D`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.61 Dataset 曲线空态画布

### 用户结果与范围

- Dataset 曲线在未选择数值 series、等待 sample 或没有可绘制数值时，不再显示孤立的普通文字；现在展示主题化 panel、protocol-to-curve glyph、层级说明和底部 signal rail。
- 空态装饰只消费既有 immutable `CurveSnapshot`、`ThemeSpec` 与 lifecycle 共享 frame；不新增业务状态、百分比/进度、series 自动选择或 CTA action。
- 有真实 points 时仍走原有曲线绘制路径；`set_snapshot()` 的 snapshot debounce `QTimer`、`flush()`、`set_suspended()`、`shutdown()`、焦点环和 AccessibleDescription 契约保持不变。
- 三主题 1180×780 离屏截图无近白像素；`stop()`、reduced-motion、暂停、隐藏、最小化和关闭继续静态回退。

### 实际修改文件

- `src/serialforge/presentation/dataset_curve.py`：增加空态 glyph/标题/说明/节点 rail，以及只作用于空态装饰的 `set_frame()`/`stop()`。
- `src/serialforge/presentation/controllers/lifecycle.py`：将既有 `_dataset_curve` 接入共享 MotionController surface fan-out；未增加第二个动效时钟。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0048-dataset-curve-empty-state.md`、`tasks/plan.md`、`tasks/todo.md`：同步模块边界、约束、决策和交付状态。

### 架构审查与父代理整合

```text
架构角色  019feb28-f305-7ca1-b2cc-1411f691ed00  called before source edit; wait timed out; closed
独立质量复核  019feb2b-43aa-7103-bee9-e180a2d6bc2e  called after implementation; wait timed out; closed
父代理    bounded audit GO：CurveSnapshot 仍是唯一数据事实；set_frame/stop 仅改变空态装饰；QTimer/lifecycle/真实 points 路径未被改变；绘制边界与主题色无白色回退
简化评估  不抽取通用曲线空态状态机；当前边界只服务 DatasetCurveWidget，通用化会复制 snapshot/绘制契约并扩大耦合
```

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  141 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
UI161_CURVE_EMPTY_SMOKE          PASS  shared motion membership; frame/stop; empty/waiting snapshots; flush; geometry
UI161_CURVE_EMPTY_PIXEL_AUDIT    PASS  star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0
provenance.py verify              PASS  current onefile manifest
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。独立复核角色已调用但因运行时超时关闭，以上结论由父代理按同一边界完成独立静态复核并记录。

### UI-1.61 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.61`
- size：`47,867,798` bytes
- canonical/root SHA-256：`593C35BFC87C530069351F11C1E122214601EC0561D671DE5B00DE582F2903E2`
- archive listing SHA-256：`82ACA88552876F18E270756D758EDE0C05A64BEA1C3D6BB9B935D0B5224897BE`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.60 Component 空态卡片

### 用户结果与范围

- Component 无数据时不再展示孤立的空表头和一行普通文字；现在以卡片形式说明“等待组件帧”和下一步，并在空态附近提供 Profile / Codec CTA。
- CTA 只复用既有 `on_load_component_codec` callback；空态 surface 不读取 ViewModel、不创建 Profile/Codec 状态、不判断 parser/source gate。
- 空态显示时隐藏空表头；存在可见 rows 时恢复原生只读 QTableWidget，过滤、选择、导出、codec 和无障碍表格行为保持不变。
- 卡片 glyph 和 CTA 接入共享 MotionController；三主题 1180×780 截图无近白像素，reduced-motion/stop 静态回退通过。

### 实际修改文件

- `src/serialforge/presentation/component_empty_state.py`：新增 `ComponentEmptyStateSurface`、兼容 `setText/text/clear` 的文案投影、无参数 `load_requested` intent、资源无关 glyph 与共享 rail。
- `src/serialforge/presentation/controllers/protocol.py`：装配空态卡片，显式连接既有 Profile/Codec callback，初始隐藏空表头。
- `src/serialforge/presentation/controllers/derived_data.py`：依据既有 visible rows 互斥隐藏空表头/显示卡片或恢复表格。
- `src/serialforge/presentation/controllers/protocol_config.py`：将既有 derived source gate 投影到 CTA enabled/visible。
- `src/serialforge/presentation/controllers/lifecycle.py`：接入卡片共享 frame/stop 生命周期。
- `src/serialforge/presentation/theme_stylesheet_base.py`、`theme_variant_shell.py`：覆盖 componentEmptyState、eyebrow、title、hint 三主题。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0047-component-empty-state-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界。

### 架构审查与父代理整合

```text
架构角色  019feb21-0879-76c1-b406-65782c04f14c  called before source edit; wait timed out; closed
独立质量复核  019feb24-3d9d-75f3-b888-246830ade834  called after implementation; wait timed out; closed
父代理    bounded audit GO：surface 保留 controller 文案契约；CTA 只发 intent；table 显隐只使用既有 visible rows；无 gate/state 复制、无本地 timer
简化评估  不抽取通用空态状态机；当前组件空态卡片是单一稳定变化边界，通用化会扩大耦合
```

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  141 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
UI160_COMPONENT_EMPTY_SMOKE         PASS  empty card; table hidden; CTA signal; gate visibility; shared frame/stop; accessibility
UI160_COMPONENT_SCROLL_PIXEL        PASS  star_trail/moonlit_ocean/sakura_night; 1180x780; near_white=0
provenance.py verify                 PASS  current onefile manifest
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。

### UI-1.60 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.60`
- size：`47,865,160` bytes
- canonical/root SHA-256：`C0962904AB5678D525B44F1702E907B4798E198171103081BCC2C38557BC83BB`
- archive listing SHA-256：`82ACA88552876F18E270756D758EDE0C05A64BEA1C3D6BB9B935D0B5224897BE`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.59 协议配置草稿/应用摘要

### 用户结果与范围

- 协议配置面板新增即时摘要：用户可以直接看到当前 framing、checksum、最大帧长，以及配置是“草稿/待应用”还是“已应用/等待接收/活跃”。
- 摘要只观察现有编辑控件和 `protocolStatus` 已投影状态；应用、重置、parser gate、历史来源和派生状态仍由原协议 controller/ViewModel 拥有。
- 摘要宽度限制为 190–520 px，协议页 1180×780 离屏验证没有横向撑破或近白像素；三主题和 reduced-motion/共享 frame 静态回退均通过。

### 实际修改文件

- `src/serialforge/presentation/protocol_config_context_surface.py`：新增 immutable `ProtocolConfigContextProjection`、协议配置摘要 QLabel 与四节点共享 config rail。
- `src/serialforge/presentation/controllers/protocol_context.py`：新增唯一 projection owner，只读取 framing/checksum/max-frame 控件和既有 `protocolStatus.state`。
- `src/serialforge/presentation/controllers/protocol.py`：装配摘要并将其作为 `ProtocolPanelWidgets` 的显式字段暴露给 composition。
- `src/serialforge/presentation/controllers/composition.py`、`lifecycle.py`：接入 QWidget 引用与共享 MotionController frame/stop fan-out。
- `src/serialforge/presentation/controllers/protocol_config.py`：在既有 status/framing 刷新边界触发 projection，不新增业务 gate。
- `src/serialforge/presentation/theme_stylesheet_base.py`、`theme_variant_shell.py`：覆盖 protocolConfigContext 的 waiting/active/draft/blocked/history/idle 三主题语义态。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0046-protocol-config-context-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界。

### 架构审查与父代理整合

```text
架构角色  019feb15-695e-78b0-b1af-6dff06e93d01  called before source edit; wait timed out; closed
独立质量复核  019feb1a-b071-7880-807e-d2929e8b5aef  called after implementation; wait timed out; closed
父代理    bounded audit GO：surface 只消费既有控件和 protocolStatus.state；无重复 derived_source_supported/parser gate；无本地 timer；宽度与 lifecycle/QSS 边界明确
简化评估  不再抽取通用 ConfigSummarySurface；当前只有协议配置一个稳定职责，继续抽象会隐藏状态 owner 并扩大耦合
```

### 验证与未运行项目

```text
pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1  PASS  140 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff/compileall
UI159_SMOKE                         PASS  projection states; accessibility; shared frame/stop; width bound; three theme screenshots; near_white=0
UI159_PROTOCOL_PAGE_SMOKE           PASS  protocol tab; 520x32 context geometry; three themes; 1180x780; near_white=0
provenance.py verify                 PASS  current onefile manifest
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、Windows 原生 HIDPI/读屏、真实 UART/TCP/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮为 Python/PySide6 presentation 变更，无 MCU/RTOS/厂商约束适用。

### UI-1.59 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.59`
- size：`47,856,384` bytes
- canonical/root SHA-256：`ECA8633C71DB6673B62CDC3EA063666E228224ED8757FD360C2735A66F2EE8A6`
- archive listing SHA-256：`349AAC2DB436582FD1D60DCDE3018FA41C169CA3ADBA96BFC1FA90DEA777CB99`
- toolchain：Python `3.12.13`；PyInstaller `6.22.0`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.58 发送表单上下文摘要

### 用户结果与范围

- 发送栏新增即时摘要：UTF-8/HEX、payload 字节数、wire payload 字节数、CRLF/无换行；用户不必自行推断实际发送尺寸。
- 坏 Hex 和超过 domain `MAX_COMMAND_PAYLOAD_BYTES` 的内容在输入过程中直接显示错误状态，并同步 AccessibleDescription/tooltip。
- 原生 mode combo、输入框、CRLF checkbox、发送按钮、快捷命令和历史动作保持原有行为；摘要只是观察面，不代替发送 gate 或设备结果。
- 摘要底部 byte rail 使用共享 MotionController，低动效、暂停、隐藏、最小化和关闭都统一静态回退；三主题没有近白像素泄漏。

### 实际修改文件

- `src/serialforge/presentation/send_context_surface.py`：新增 immutable form projection、canonical payload limit 投影和 QLabel byte rail。
- `src/serialforge/presentation/controllers/send_context.py`：新增发送控件到 surface 的唯一 projection owner。
- `src/serialforge/presentation/controllers/terminal.py`：装配 surface，并把 mode/CRLF 变化接入既有输入刷新回调。
- `src/serialforge/presentation/controllers/connection.py`：在既有发送 gate 更新中刷新 context projection。
- `src/serialforge/presentation/controllers/lifecycle.py`：将 surface 接入共享 MotionController frame/stop fan-out。
- `src/serialforge/presentation/theme_stylesheet_base.py`、`theme_variant_shell.py`：补齐 sendContext empty/ready/invalid 三态主题。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0045-send-context-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

### 架构审查与父代理整合

```text
架构角色  019feb04-b512-7983-9478-0f676f388d42  called before source edit; wait timed out; closed
架构角色  019feb0c-a4bf-7820-a061-3e38f3e7a4bb  called before canonical-limit correction; wait timed out; closed
独立质量复核  019feb0b-6f4c-7491-b49c-8e9ac7afa43c  called after implementation; wait timed out; closed
父代理    bounded audit GO：surface/controller 边界清晰；复核发现独立 1 MiB 显示上限可能遮蔽 domain 64 KiB 约束，已改为复用 MAX_COMMAND_PAYLOAD_BYTES 并将超限提前投影为 invalid
```

### 验证与未运行项目

```text
scripts/check.ps1       PASS 138 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI158_SEND_CONTEXT       PASS empty / valid HEX+CRLF / invalid HEX / canonical over-limit / shared frame / reduced-motion
UI158_THEME              PASS star_trail, moonlit_ocean, sakura_night; 1180x780; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI、EXE 启动/关闭、Windows 原生 HIDPI/读屏、真实 UART/BLE/RTT/J-Link、OTA、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.58 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.58`（历史归档）
- size：`47,851,307` bytes（历史归档）
- canonical/root SHA-256：`1C911A77887234AFF46470F850E1438C95BAA25D17243170380CFC1EE4489D8A`（历史归档）
- archive listing SHA-256：`778FB6B7F5F2651F8297C549F1578C44A59D15221D6EA09C453F3D9DCA07B359`（历史归档）
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.57 主题切换一次性 fade + sweep

### 用户结果与范围

- 主题切换不再只有整体淡入：在既有 root fade 上增加一次性语义色几何 sweep，增强二次元主题切换的可感知反馈。
- sweep 是鼠标透明、不可聚焦、空 accessibility 的 presentation overlay，不改变 `apply_theme()`、焦点、键盘、无障碍树或业务状态。
- fade、sweep、effect 由同一个 `theme_transition.py` owner 管理；快速连续切换、低动效、暂停、隐藏、最小化和关闭均回到静态主题。

### 实际修改文件

- `src/serialforge/presentation/theme_transition_surface.py`：新增基于 ThemeSpec token 的几何 sweep overlay。
- `src/serialforge/presentation/theme_transition.py`：统一创建/停止/完成 root fade 与 sweep geometry animation。
- `src/serialforge/presentation/qt.py`：补充 `QRect` presentation import boundary。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0044-theme-transition-sweep.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

### 架构审查与父代理整合

```text
架构角色  019feafc-8cc9-7560-b70c-2474b935e82a  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：theme_transition.py 保持唯一一次性过渡 owner；ThemeTransitionSurface 只读 ThemeSpec 并由同一 stop/finish 清理
```

父代理整合：overlay 不接入 MotionController，不新增常驻 timer，不承载业务 progress；快速切换只保留最新一次 transition。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 136 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI157_TRANSITION         PASS overlay running; mouse-transparent; empty accessibility; finish cleanup
UI157_REDUCED_MOTION     PASS theme change falls back to no overlay
UI157_PIXEL              PASS three themes; 1180x780; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.57 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.57`
- size：`47,839,961` bytes
- canonical/root SHA-256：`2654BE1BEC460BD05D18871D6CA570023A795CF30C2B9066A4D76E8A0AA6EF85`
- archive listing SHA-256：`70EA034A42EB126D8338A1E3D0E3AFEBB6FFF6EC284AE4FECE9E856EBCBB111D`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.56 连接快速配置可视摘要

### 用户结果与范围

- 快速配置选择后，连接带内直接显示 `内置/自定义 · 配置名 · 传输类型`，并保留原有下一步提示；未选择时显示明确的空态说明。
- 摘要只展示已有 bounded `ConnectionPreset` DTO，不保存密钥、不自动连接；原 `connection_hint` QLabel 的文字、tooltip、AccessibleName/Description 和 controller wiring 保持不变。
- 已选 preset 使用共享 shell frame 绘制低对比度底部 signal rail；低动效、暂停、隐藏、最小化和关闭时静态回退。

### 实际修改文件

- `src/serialforge/presentation/connection_preset_context_surface.py`：新增连接快速配置摘要 surface 与主题化 rail。
- `src/serialforge/presentation/connection_preset_surface.py`：将 combo selection 投影到 context surface。
- `src/serialforge/presentation/controllers/connection_builder.py`：装配 surface，并保留原 `connection_hint` label 引用。
- `src/serialforge/presentation/controllers/connection_presets.py`：保存/删除 catalog 后同步摘要。
- `src/serialforge/presentation/controllers/lifecycle.py`：接入共享 MotionController frame/stop。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0043-connection-preset-context-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

### 架构审查与父代理整合

```text
架构角色  019feaf2-c103-7da1-a686-56ac90a3c965  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：ConnectionPresetContextSurface 只读既有 ConnectionPreset；connection controller 保持动作/状态 owner；共享 frame/stop 为唯一动效入口
```

父代理整合：surface 与 preset projection 均留在 presentation；不改变 preset codec、domain transport port、连接 gate 或自动连接语义。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 135 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI156_STATE_VECTOR       PASS none/builtin/custom/none; hint label identity/accessibility retained
UI156_MOTION             PASS shared frame; selected preset pulse; stop fallback
UI156_PIXEL              PASS three themes; 1180x780; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.56 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.56`
- size：`47,838,432` bytes
- canonical/root SHA-256：`635228DA938D6F55B71369512F81D091B45090A7C3EA705009493F0A5CAF4F58`
- archive listing SHA-256：`3CEB06277CE97D62220E59860944F33BA0831E956F817778337492CF7CA83ED6`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.55 发送状态 activity rail

### 用户结果与范围

- 发送状态带在原有状态文案下方增加 `blocked/waiting/ready/busy/history` 五态主题化 rail；`busy` 随共享动效帧轻微 pulse，其余状态保持稳定静态反馈。
- 原有发送状态文案、`state` property、enabled gate、AccessibleName/Description、`objectName` 与连接控制 owner 保持不变。
- 本轮不新增发送状态源、ViewModel、计时器、进度百分比或发送 gate；surface 只读既有状态事实并负责后置绘制。

### 实际修改文件

- `src/serialforge/presentation/send_state_surface.py`：新增原生 `QLabel` 后置绘制 surface。
- `src/serialforge/presentation/controllers/terminal.py`：发送状态标签改用 `SendStateSurface`，保留既有文案与属性 wiring。
- `src/serialforge/presentation/controllers/lifecycle.py`：纳入共享 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0042-send-state-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步模块边界与交付状态。

### 架构审查与父代理整合

```text
架构角色  019feae7-20b3-70c3-a0af-2e9d9af2a843  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：SendStateSurface 只读 connection.py 既有 state property；不新增 source/gate；共享 frame/stop 保持唯一动效入口
```

父代理整合：状态 owner 仍在 connection.py；surface 只表达已有状态，不改变发送行为、协议语义或控制可用性。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 134 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI155_SEND_STATE         PASS blocked/ready/busy/history/waiting; native text/accessibility/objectName retained
UI155_MOTION             PASS shared frame; busy pulse; static stop fallback
UI155_PIXEL              PASS three themes; ready/busy 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.55 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.55`
- size：`47,834,492` bytes
- canonical/root SHA-256：`F3C7428EB8E9A6B802738B677F43258E1A26F518E71A9BD0649786B60604F37C`
- archive listing SHA-256：`D5F51F895A97BFD9DB21FB42095D71FFE921655F552AFE45A1C0F07CD0404AE8`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.54 实时接收数据 activity rail

### 用户结果与范围

- 实时观测带的接收数据状态现在在原有文字下方增加实时/历史主题化 activity rail；有接收活动时共享 pulse 可见，静止时保留低对比度静态轨。
- 原有文字、字节数、窗口大小、实时/历史来源、tooltip、AccessibleDescription、`objectName` 和布局保持不变。
- 本轮不复制 RX buffer、不绘制吞吐或百分比、不新增接收状态源或计时器。

### 实际修改文件

- `src/serialforge/presentation/data_activity_surface.py`：新增原生 QLabel 后置绘制 surface 与 bounded `DataActivityProjection`。
- `src/serialforge/presentation/controllers/terminal.py`：只替换接收活动 QLabel 构造，保留既有 widget 名称和 wiring。
- `src/serialforge/presentation/controllers/terminal_runtime.py`：从既有文案事实投影 source/最近字节/窗口字节。
- `src/serialforge/presentation/controllers/lifecycle.py`：投影 active 并纳入共享 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0041-data-activity-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界。

### 架构审查与父代理整合

```text
架构角色  019feadf-ecb6-72f3-9f7a-d403d5ccedcf  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：DataActivitySurface 只做 QLabel 后置绘制；terminal_runtime/lifecycle 保持 facts owner
```

父代理整合：字节值只用于确定性装饰形态，不暗示吞吐/进度；共享 frame/stop 仍是唯一动效入口。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 133 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI154_DATA               PASS realtime/history; latest/window projection; native text/accessibility retained
UI154_MOTION             PASS active/static/frame/stop; shared MotionController fan-out
UI154_PIXEL              PASS three themes; 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 UART/BLE/RTT/J-Link、OTA、硬件、
Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.54 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.54`
- size：`47,829,486` bytes
- canonical/root SHA-256：`DB64592CA9AE1A1789AB31B7255219DB3CD35138430356CD0F110FD24FE1BFAE`
- archive listing SHA-256：`A0F6FFE24B100AD6E33ABD750786ECECF8477293716843682F7ADE7E85399543`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.53 操作控制 activity rails

### 用户结果与范围

- 批量命令 `RUNNING` 时，“停止”按钮显示共享 activity rail；`IDLE/COMPLETED` 回到静态。
- 历史回放 `PLAYING` 时，“暂停”按钮显示共享 activity rail；`PAUSED` 的“继续”和其余终态保持静态。
- 原生 click、enabled gate、文案、无障碍语义和命令/回放 owner 不改变；本轮不新增业务时钟、状态源或进度百分比。

### 实际修改文件

- `src/serialforge/presentation/controllers/terminal.py`：批量停止按钮复用 `BusyActionButton`。
- `src/serialforge/presentation/controllers/protocol.py`：历史回放暂停按钮复用 `BusyActionButton`。
- `src/serialforge/presentation/controllers/connection.py`：投影既有 `batch_active`。
- `src/serialforge/presentation/controllers/replay.py`：投影既有 `ReplayState.PLAYING`。
- `src/serialforge/presentation/controllers/lifecycle.py`：两个按钮纳入共享 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0040-operation-control-activity-rails.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构边界。

### 架构审查与父代理整合

```text
架构角色  019fead9-910d-7823-b9d0-8aa757bf80ce  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：batch/replay owner projection remains authoritative; shared BusyActionButton/MotionController sufficient
```

父代理整合：没有新增抽象；暂停/停止回退为静态，视觉 busy 不代表设备完成度、回放总进度或批量设备确认。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI153_BATCH              PASS IDLE/RUNNING/COMPLETED; native BusyActionButton; enabled/accessibility retained
UI153_REPLAY             PASS EMPTY/PLAYING/PAUSED/EOF/STOPPED/ERROR; text/enabled retained
UI153_MOTION             PASS shared frame; busy rail; static/stop fallback
UI153_PIXEL              PASS three themes; replay and batch 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行持续 GUI/EXE 启动、真实 batch/replay worker、UART/BLE/RTT/J-Link、
OTA、硬件、Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A。

### UI-1.53 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.53`
- size：`47,825,866` bytes
- canonical/root SHA-256：`99439BB50574149CA2080814278547F3723929A3ED85384444973868B7868E2C`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.52 连接主按钮 activity rail

### 用户结果与范围

- 连接主按钮现在把既有 `SessionState.OPENING/CLOSING` 显示为共享 activity rail，用户能看见连接或断开请求仍在进行。
- `OPEN/CLOSED/ERROR/DISCOVERED` 保持静态回退；连接文案、enabled gate、click、无障碍与连接动作不改变。
- 本轮只修改 presentation；不改变 session worker、transport、重试、网络授权、硬件或连接状态源。

### 实际修改文件

- `src/serialforge/presentation/controllers/connection_builder.py`：主连接按钮复用既有 `BusyActionButton`。
- `src/serialforge/presentation/controllers/connection.py`：只把既有 `SessionState.OPENING/CLOSING` 投影为 `set_busy()`。
- `src/serialforge/presentation/controllers/lifecycle.py`：将 `_connect_button` 纳入唯一 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0039-connection-button-activity-rail.md`、`tasks/plan.md`、`tasks/todo.md`：同步复用边界。

### 架构审查与父代理整合

```text
架构角色  019fead1-7801-7a50-bcab-b52709c161dc  called before source edit; repeated wait timed out twice; closed
父代理    bounded audit GO：BusyActionButton shared renderer/lifecycle sufficient; connection.py remains SessionState projection owner
```

父代理结论：不新增 connection-specific painter、timer、状态源或进度语义；builder 只组装控件，controller 只投影已有 session state，
lifecycle 只负责共享 frame/stop。原生按钮契约和连接 action callback 保持不变。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI152_CONNECTION         PASS CLOSED/OPENING/CLOSING/OPEN/ERROR; BusyActionButton; click/accessibility retained
UI152_MOTION             PASS shared frame; busy rail; static/stop fallback
UI152_PIXEL              PASS three themes; 980x680; near_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；这些是短时 offscreen evidence，不是持续 GUI/EXE 启动证明。
未运行真实连接 worker、UART/BLE/RTT/J-Link、OTA、硬件、Windows 原生键盘/读屏/HIDPI、签名或正式发行验收；未创建、修改或运行
unit test、mock、fixture、harness 或 test-only 资产。嵌入式 C/C++ 适用性：N/A。

### UI-1.52 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.52`
- size：`47,825,940` bytes
- canonical/root SHA-256：`B150C618B8929943D24B87FC46F1B6FD7F3BF0DD1C8755AF2C3EA355F055458E`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.51 原始记录按钮 activity rail

### 用户结果与范围

- 原始记录按钮现在把既有 `RecordingState.STARTING/ACTIVE/STOPPING` 显示为共享 activity rail，用户能区分记录动作正在建立、进行或停止，
  同时保留原有“停止原始记录/开始原始记录”文案。
- 复用既有 `BusyActionButton`，不新增 renderer、timer、recording 状态、写盘逻辑或传输语义；`ERROR/STOPPED` 保持静态/非 busy。
- 本轮只修改 presentation；不改变 recording worker、文件格式、错误传播、ViewModel、transport、OTA/debug 或硬件行为。

### 实际修改文件

- `src/serialforge/presentation/controllers/terminal.py`：将 record button 构造替换为既有 `BusyActionButton`，保留 signal、文本和无障碍 wiring。
- `src/serialforge/presentation/controllers/terminal_runtime.py`：只把既有 RecordingState 三个进行中状态投影为 `set_busy()`。
- `src/serialforge/presentation/controllers/lifecycle.py`：将 `_record_button` 纳入已有 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0038-recording-button-activity-rail.md`、`tasks/plan.md`、`tasks/todo.md`：同步复用边界与 ADR。

### 六角色审查与父代理整合

```text
产品       019feac9-c929-7c43-936b-49a8107705bc  called; repeated wait timed out; closed
架构       019feac9-c979-7f70-92a9-91a2ba99fa20  called before source edit; repeated wait timed out; parent bounded audit GO
UI 设计    019feac9-c9c8-77e2-a25c-f36fbc960ae2  called; repeated wait timed out; closed
开发       019feac9-ca10-7990-80ea-0aee7d8394f7  called read-only; repeated wait timed out; closed
验证       019feac9-ca5d-7953-9241-77ef5f3a9998  called; repeated wait timed out; closed
打包流程   019feac9-cab0-7a00-b593-c7f88bcaa75d  called; repeated wait timed out; closed
独立复核   019feacc-4119-79c3-b3ef-ef88ab7ead14  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理整合结论：BusyActionButton 的职责扩展为已有异步 presentation action；recording controller 仍唯一拥有 RecordingState projection，
按钮不读取 ViewModel。共享 painter/lifecycle 保持单一实现，无新增抽象或循环依赖。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI151_RECORD             PASS idle/starting/active/stopping/error; native BusyActionButton; text/accessibility retained
UI151_MOTION             PASS shared frame; busy rail; static/reduced stop fallback
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未启动持续 GUI/EXE，未运行真实 recording worker/文件写入、
Windows 原生键盘/读屏/HIDPI、真实 UART/BLE、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮只有 Python/PySide6 presentation 改动。

### UI-1.51 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.51`
- size：`47,824,961` bytes
- canonical/root SHA-256：`636E471AFE973EE703254BD6A0D3D5BEC63E59C29762B67A68F220D0BB004315`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.50 errorBar 故障 beacon

### 用户结果与范围

- errorBar 出现错误时，左侧增加 28×28 的主题化故障 beacon；静态环/叉标和共享 frame pulse 让错误区域更容易被快速定位。
- 错误文字、详情 tooltip、`ErrorInfo` 传播、清除按钮、错误栏可见性和 status message 仍保持原有权威语义。
- 本轮只修改 presentation；不改变错误码、错误恢复、ViewModel、transport、协议、OTA/debug 或真实设备行为。

### 实际修改文件

- `src/serialforge/presentation/error_surface.py`：新增鼠标透明、不可聚焦、空 accessibility 的故障 beacon，提供 `set_active()`、
  `set_frame()`、`stop()`，不持有 ViewModel、不创建 timer。
- `src/serialforge/presentation/controllers/terminal.py`：errorBar 只新增 signal 子控件，保留原有 label/clear button/layout 语义。
- `src/serialforge/presentation/controllers/lifecycle.py`：在既有 ErrorInfo 空/非空分支投影 active/clear，并加入共享 frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0037-error-notification-signal-beacon.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界与 ADR。

### 六角色审查与父代理整合

```text
产品       019feac3-a720-7050-b270-cb46df70ee34  called; repeated wait timed out; closed
架构       019feac3-a760-79c1-b8e5-40cb05331f70  called before source edit; repeated wait timed out; parent bounded audit GO
UI 设计    019feac3-a7af-7880-91b2-51a8560759ad  called; repeated wait timed out; closed
开发       019feac3-a801-7522-9220-b2c03cedf23b  called read-only; repeated wait timed out; closed
验证       019feac3-a84c-7230-bf45-4670e7a96ee6  called; repeated wait timed out; closed
打包流程   019feac3-a896-7463-aa4a-2106fac31b9b  called; repeated wait timed out; closed
独立复核   019feac6-bab1-7890-a0fe-13d9ff297a64  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理整合结论：beacon 只消费既有 ErrorInfo 可见性投影和共享 frame；错误 label/clear action 仍是权威 owner；不新增错误状态源、
timer、事件过滤器或第二套恢复流程。绘制集中在单一 <1000 行 presentation module。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 132 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI150_ERROR              PASS three themes; active/static/stop/clear; label and clear button retained
UI150_PIXEL              PASS three themes; errorBar 424x56; exact_white=0
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未启动持续 GUI/EXE，未运行 Windows 原生键盘/读屏/HIDPI、
真实 UART/BLE、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮只有 Python/PySide6 presentation 改动。

### UI-1.50 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.50`
- size：`47,823,121` bytes
- canonical/root SHA-256：`D4A81A89B4CE6FF31FBB5DA93C4D93CD57921E05C70DD5A674BC05E947392B3C`
- archive listing SHA-256：`3D64C4E65BB5425A6D3F39F9E93AF8A3F38CD9921CB37EF9E05500B336F2AD13`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.49 发送输入框状态轨

### 用户结果与范围

- 发送输入框现在会把已有 `send_band_state` 投影为 blocked/waiting/ready/busy/history 的主题化底部轨道；有焦点且可发送/处理中时，
  共享动效帧可显示低频 pulse，用户无需猜测输入框当前是否可用。
- 仍是原生 `QLineEdit`：文本、选择、光标、剪贴板、placeholder、回车、QSS、无障碍和发送 gate 未改变；不绘制文字、不接管事件。
- 本轮只修改 presentation；不改变 ViewModel、transport、协议、发送 payload、OTA/debug 或实际设备行为。

### 实际修改文件

- `src/serialforge/presentation/send_input_surface.py`：新增原生 `QLineEdit` 子类，绘制状态轨/焦点反馈，提供 `set_surface_state()`、
  `set_frame()`、`stop()`，无 timer/业务状态源。
- `src/serialforge/presentation/controllers/terminal.py`：仅将发送输入框构造替换为 `SendInputSurface`，保留信号、键盘和 accessible wiring。
- `src/serialforge/presentation/controllers/connection.py`：将既有 `send_band_state` 投影给 input surface，不改变 gate/文案。
- `src/serialforge/presentation/controllers/lifecycle.py`：将 input surface 纳入共享 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0036-send-input-surface-state-rail.md`、`tasks/plan.md`、`tasks/todo.md`：同步边界与交接。

### 六角色审查与父代理整合

```text
产品       019feabd-3d4c-7a30-9d7e-54c1dc475517  called; repeated wait timed out; closed
架构       019feabd-3d96-7d11-a9c1-0c96160dcc99  called before source edit; repeated wait timed out; parent bounded audit GO
UI 设计    019feabd-3de6-7190-8dc1-821d1ac8c003  called; repeated wait timed out; closed
开发       019feabd-3e37-7283-ba1b-1ad7bc17bc5f  called read-only; repeated wait timed out; closed
验证       019feabd-3e86-7dd3-b95a-aa96c14694ed  called; repeated wait timed out; closed
打包流程   019feabd-3ecd-7a31-bfd4-fd39a2e0b055  called; repeated wait timed out; closed
独立复核   019feabf-42c3-73b3-9d09-51f8de2bf42e  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理整合结论：input surface 只消费已存在的 `send_band_state`；原生 QLineEdit 先绘制，装饰后置；不新增状态源、timer、事件过滤器
或第二个发送 owner。简化审查未发现重复 renderer，保留单一小型 presentation module。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 131 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI149_FIELD              PASS three themes; five states; native QLineEdit subclass; text=AA 55 preserved
UI149_PIXEL              PASS state screenshots; themed host; no white fallback observed
UI149_SEND_BAND          PASS three themes; themed sendControlBand; exact_white=0; native surface type retained
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未启动持续 GUI/EXE，未运行 Windows 原生键盘/读屏/HIDPI、
真实 UART/BLE、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮只有 Python/PySide6 presentation 改动。

### UI-1.49 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.49`
- size：`47,822,058` bytes
- canonical/root SHA-256：`4998DCEBB54D7464CF19EB2CC19F0319F5013444C35E50C8CF57EE5435B4D35F`
- archive listing SHA-256：`77BE6FBE1BC0155AF3AFAA560B4D1C53803BC4AB2C2BA8D91EEE12AED9F953EE`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.48 QStatusBar 状态 footer signal rail

### 用户结果与范围

- 底部 native `QStatusBar` 右侧新增 116×18 的主题化 session/RX signal rail，使连接状态、短时 RX activity 和应用 fault
  在底部拥有统一、低干扰的视觉锚点。
- 原生 status message 仍然是唯一可读、可访问的权威文案；footer 不复制文字、不进入键盘焦点顺序、不改变状态栏布局语义。
- 本轮只修改 presentation/bootstrap/lifecycle 接线；不改变 UART/TCP/UDP/BLE/RTT、协议、记录、OTA/debug 或 ViewModel 业务行为。

### 实际修改文件

- `src/serialforge/presentation/status_footer_surface.py`：新增无状态边界的自绘 footer，接收 state/fault/activity/frame，固定 116×18，
  不创建 timer，不读取 ViewModel。
- `src/serialforge/presentation/controllers/bootstrap.py`：创建并挂入 native `QStatusBar` permanent widget，完成初始 state 投影。
- `src/serialforge/presentation/controllers/lifecycle.py`：统一投影 state/fault/activity，并将 surface 加入已有 MotionController frame/stop fan-out。
- `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md`、`docs/adr/0035-status-footer-signal-surface.md`、`tasks/plan.md`、`tasks/todo.md`：同步架构约束、ADR 与任务状态。

### 六角色审查与父代理整合

```text
产品       019feab3-099d-7281-83b1-f53e33281310  called; repeated wait timed out; closed
架构       019feab3-09eb-7550-994d-cb15ccbcb781  called before source edit; repeated wait timed out; parent bounded audit GO
UI 设计    019feab3-0a3e-7893-83ef-ab9353d52388  called; repeated wait timed out; closed
开发       019feab3-0a85-7232-a151-83a52dba5e56  called read-only; repeated wait timed out; parent remained sole writer
验证       019feab3-0ad2-7962-bf99-bd9bfc6dc679  called; repeated wait timed out; closed
打包流程   019feab3-0b22-7d83-a923-a080653a0285  completed; package baseline/risk fields returned
独立复核   019feab8-563c-7602-998b-6a46c5794a9b  called read-only; repeated wait timed out; closed; parent five-axis review recorded
```

父代理整合结论：控件只消费 lifecycle projection 与共享 frame；status message 继续承担 accessibility truth；不引入第二状态源、
新时钟或业务依赖。重复 renderer 不存在，故未增加抽象层；绘制几何集中在一个 <1000 行 presentation module。

### 验证与未运行项目

```text
scripts/check.ps1       PASS 130 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
compileall               PASS src
UI148_IMPORT_COMPILE     PASS 67 presentation modules; qapplication_instance=False; AST parse
UI148_SURFACE            PASS three themes; active/fault_static/closed_static; 116x18
UI148_STATUSBAR          PASS three themes; 420x90; exact_white=0; native status bar integration
```

离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未启动持续 GUI/EXE，未运行 Windows 原生键盘/读屏/HIDPI、
真实 UART/BLE、硬件、签名或正式发行验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。
嵌入式 C/C++ 适用性：N/A；本轮只有 Python/PySide6 presentation 改动，无 MCU/固件/vendor source 或认证声明。

### UI-1.48 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.48`
- size：`47,817,520` bytes
- canonical/root SHA-256：`1DC3A5424FC3202DA7708BCA53BFF755317728911B23D1A2D7B74BC680A8C314`
- archive listing SHA-256：`A061158AF68D7B02117AF008AF439548E6675CE67B64AFF99810F5E1AEFF1E8F`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 用户目标与边界

用户要求持续美化 SerialForge UI 组件和动态效果，所有验收要点交付成功后才停止；本轮又要求为 OTA、AES 安全升级、RTT/J-Link 打印和独立 UI 目录预留高内聚低耦合边界。本轮继续遵守：

- UI 变化仍只修改 presentation；新增 OTA/debug 只提供独立 contract-only 包，不接入 domain/application/infrastructure 组合根，不冒充已实现硬件能力。
- 动效只服务装饰性反馈，可暂停、低动效、隐藏/最小化/关闭时停止，并保留静态绘制回退；连续装饰帧只使用共享 MotionController，
  可见空闲 shell 通过 `ambient_active` 使用同一 96ms 共享时钟出帧；主题切换只允许独立的一次性过渡，不新增控件级常驻时钟。
- 工作区路线 beacon 只作为 Tab 文案的视觉锚点，真实 `QTabWidget`、Tab 顺序和无障碍文本保持权威；它由 layout-owned route strip
  承载，不依赖会被当前 Qt style 裁切的 `setCornerWidget()`。
- 不启动 GUI、EXE、服务、网络、真实 UART/TCP/UDP/BLE/RTT 或硬件；不创建、修改或运行测试专用资产。
- 二次元方向采用原创无 IP 的“星轨霓虹”抽象几何主题；没有角色、背景图、GIF、粒子贴图、外部字体或 QRC/QSS 资源。

## 2026-08-10 UI-1.47 刷新/扫描按钮共享忙碌信号轨

- 运行证据确认：UART 刷新与 BLE 扫描已有真实 busy projection，但旧 UI 只有“刷新中…/扫描中…”文字，没有明确进行中视觉反馈。
- `action_surface.py` 新增 `BusyActionButton`，保持原生 `QPushButton` 点击、焦点、disabled、QSS、AccessibleName/Description；
  connection builder 只替换两个按钮，controller 只投影既有 `discovery_busy/ble_scan_busy`，lifecycle 复用共享 MotionController frame。
- 低动效、暂停、隐藏、最小化和关闭时静态 rail 保留；不新增 timer、线程、业务状态、进度或 transport 行为。
- 父级复核发现两个 action surface 重复 painter，已在同一文件提取私有 `_paint_signal_rail()`，保留主题色/透明度/几何差异，降低维护分叉。
- 开发约束、架构说明与 ADR 0034 已同步。

### UI-1.47 验证、审查与简化

```text
UI147_LAYOUT            PASS 980x680; main window exact_white=0; no horizontal overflow
UI147_BUSY              PASS three themes; BusyActionButton type; busy text/enabled/colored rail
UI147_REDUCED           PASS busy remains visible with animated=False/static rail
UI147_IMPORT/COMPILE    PASS 129 modules; compileall; AST pass
scripts/check.ps1       PASS 129 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
ARCH_REVIEW             architecture role called; worker timed out repeatedly and was closed; parent bounded audit GO
QUALITY_REVIEW          independent reviewer called; timed out/closed; parent five-axis review recorded
SIMPLIFICATION          shared private rail renderer removes duplicate painter while preserving both button APIs
EMBEDDED_REVIEW         N/A for Python/PySide6; no MCU/C/C++/firmware/vendor requirement applicable
```

截图：`build/ui_review_ui147_refactored_star_trail.png`、`..._moonlit_ocean.png`、`..._sakura_night.png`。
离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；未运行真实 UART/BLE 扫描、持续 GUI、Windows 键盘/读屏/HIDPI、
硬件或正式签名发布验收；未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## UI-1.47 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.47`
- size：`47,814,301` bytes
- canonical/root SHA-256：`85CAB8E0977BD3FA2A849CA64E891A64E19F3EFFF52D74E6480C746D5740A15D`
- archive listing SHA-256：`314DF07FC1EC7E15C6DF32BBA8776AC83999BA78A371764F3399EFB32D7FD94A`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.46 QFileDialog 目录树/文件列表主题桥

- 根因已确认：现有 `QFileDialog` 的目录树、文件列表、侧栏和部分滚动区域仍消费系统白色 palette；三主题关闭窗口截图无法覆盖
  这个独立对话框 surface，直接构造/展开后才出现大面积白底。
- 新增 `presentation/file_dialog_surface.py`，用两个无状态 wrapper 替换现有七处静态 open/save 调用；保留 Qt 文件系统模型、路径导航、
  ExistingFile/AnyFile、AcceptOpen/AcceptSave、selected filter、取消和保存覆盖确认语义。为保证 QSS 在 Windows 可见，显式使用
  `DontUseNativeDialog`，不实现自绘文件浏览器或业务文件 I/O。
- `theme_stylesheet_controls.py`、`theme_variant_controls.py`、`theme_stylesheet_runtime.py` 新增 bounded QFileDialog
  tree/list/header/sidebar/button/scrollbar 规则；文件对话框内 ComboBox 复用 `popup_surface.py` 的顶层 frame bridge。
- 约束、架构说明和 ADR 0033 已同步；控制器仍只决定文件用途和后续 application 调用，file-dialog helper 不保存路径、不读取 ViewModel/session/transport。

### UI-1.46 验证、审查与简化

```text
UI146_PIXEL_AUDIT       PASS three themed file-dialog screenshots; broad directory/list samples are themed, no white background block
UI146_FILE_FIXED        PASS star_trail/moonlit_ocean/sakura_night; internal ComboBox popup uses comboPopup bridge
UI146_WRAPPER            PASS open/save cancel returns ('', '') without file write
UI146_IMPORT/COMPILE     PASS 129 modules; compileall; AST pass
scripts/check.ps1        PASS 129 files <= 1000; 3 themes; 22 semantic tokens; 19 selectors; Ruff
ARCH_REVIEW              architecture role called; worker timed out repeatedly and was closed; parent bounded audit GO
QUALITY_REVIEW           independent reviewer called; timed out/closed; parent five-axis review found no behavior change
SIMPLIFICATION           seven static calls converge on two explicit wrappers; no stateful abstraction added
EMBEDDED_REVIEW          N/A for Python/PySide6; no MCU/C/C++/firmware/vendor requirement applicable
```

截图：`build/ui_review_ui146_fixed_star_trail.png`、`..._moonlit_ocean.png`、`..._sakura_night.png` 以及
`..._combo_popup.png`。离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论；`near_white` 计数会包含浅色文字字形，
因此同时记录固定背景采样和截图证据。

未运行持续 GUI、EXE 启动、Windows 原生 Shell、真实键盘/读屏/HIDPI、UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或正式签名发布验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## UI-1.46 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.46`
- size：`47,813,182` bytes
- canonical/root SHA-256：`F49C8EEA790D9763C5E9863722A86D8C5AEA16F047FF6B9CECAD9B4831B5B1AF`
- archive listing SHA-256：`314DF07FC1EC7E15C6DF32BBA8776AC83999BA78A371764F3399EFB32D7FD94A`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.45 ComboBox popup 顶层 frame 白带修复

- 根因已确认：Qt 原生 ComboBox 的 `view().window()` 是独立顶层 `QFrame`，父窗口 QSS 不覆盖其默认 palette；展开连接快速配置时上下各出现 6px 近白横带，三主题旧值均 `near_white=5556`。
- 新增 `presentation/popup_surface.py`，只给原生 popup frame 注入 `surfaceRole="comboPopup"` 的小范围 stylesheet；不替换 QListView、不接管键盘/焦点、不创建 timer 或事件过滤器。
- `theme_stylesheet_controls.py`、`theme_variant_controls.py` 和 `theme_stylesheet_runtime.py` 增加 popup frame 的语义 token 规则；
  `theme.py` 主题入口刷新现有 popup，bootstrap 在主窗口组合后补首屏刷新，CommandBatchEditor 在构造完成后补自身 combo 刷新。
- `check_theme_tokens.py` 将 popup selector 纳入 14 项门禁；关闭状态和展开状态均不修改业务连接、preset selection 或原生 item 行为。

### UI-1.45 验证、审查与简化

```text
UI145_POPUP_FIXED       PASS star_trail/moonlit_ocean/sakura_night; theme switch; role=comboPopup; near_white=0
UI145_EDITOR_POPUP      PASS three themes; CommandBatchEditor combo popup; near_white=0
UI145_RESPONSIVE        PASS 980x680/1180x780; no popup hscroll/vscroll; near_white=0
UI145_IMPORT/COMPILE    PASS 127 modules; compileall
scripts/check.ps1       PASS 128 files <= 1000; 3 themes; 22 tokens; 14 selectors; ruff
ARCH_REVIEW             parent-audited; architecture waits timed out; import-order follow-up returned GO
QUALITY_REVIEW          independent read-only reviewer timed out twice and closed; parent five-axis review found no behavior-changing simplification
EMBEDDED_REVIEW         N/A for Python/PySide6; no MCU/C/C++/firmware/vendor requirement applicable
```

旧/新 popup 证据：旧 `build/ui_review_ui145_star_trail_preset_popup.png` 可见上下白带；新
`build/ui_review_ui145_fixed_star_trail_preset_popup.png`、`..._moonlit_ocean_...png`、`..._sakura_night_...png` 近白为 0。
离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。

未运行持续 GUI、EXE 启动、Windows 原生 style、真实键盘/读屏/HIDPI、UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或正式签名发布验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## UI-1.45 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.45`
- size：`47,809,326` bytes
- canonical/root SHA-256：`56B2B61A79A1CB997990D1AC96FACCEEFAEB035FF9D6F88F73E21ACF29473D28`
- archive listing SHA-256：`91A2F43B3917E69DE58A7545F5DB682447B4594F50B82E1A9D0A8B36343E7BAB`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.44 协议确认弹窗主题外壳与无障碍上下文

- 新增 `presentation/dialog_surface.py:configure_confirmation_dialog()`，只负责确认弹窗的
  `surfaceRole="confirmation"`、标准 Ok/Cancel 文案、`dangerButton` 确认语义和
  AccessibleName/AccessibleDescription；不持有 ViewModel、协议 worker、transport、密钥或 timer。
- `controllers/protocol_config.py:confirm_protocol_change()` 仍是确认触发条件、影响范围说明和 `exec()` 返回语义的唯一 owner；
  默认按钮继续为取消，确认仍返回 `QMessageBox.StandardButton.Ok`，没有偷渡改变协议行为。
- 默认 controls stylesheet 与 theme override 新增 warning surface、信息文本和按钮宽度的
  `QMessageBox[surfaceRole="confirmation"]` 规则；`scripts/check_theme_tokens.py` 将 selector 纳入 13 项门禁，三主题均不回落白色 palette。
- 新增 ADR [`0031-themed-confirmation-dialog.md`](../adr/0031-themed-confirmation-dialog.md)，并在约束中记录 helper、焦点、
  标准按钮、低动效及后续“未完成帧确认门/重置文案”非本轮范围。

### UI-1.44 验证、审查与简化

```text
scripts/check.ps1       PASS 127 files <= 1000; 3 themes; 22 semantic tokens; 13 selectors; ruff
UI144_IMPORT/COMPILE    PASS 126 modules; compileall
UI144_CONFIRM           PASS 3 themes; surfaceRole; default_cancel=True; standard button text;
                             AccessibleName/Description; near_white=0
ARCH_REVIEW             parent-audited; architecture role called twice for this slice; waits timed out;
                             final token import/fix remained presentation-only and bounded
QUALITY_REVIEW          independent read-only review recorded; helper reuse accepted; no behavior-changing simplification
EMBEDDED_REVIEW         N/A for Python/PySide6; no MCU/C/C++/firmware/vendor requirement applicable
```

截图：`build/ui_review_ui144_confirmation_star_trail.png`、`build/ui_review_ui144_confirmation_moonlit_ocean.png`、
`build/ui_review_ui144_confirmation_sakura_night.png`。离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。

未运行持续 GUI、EXE 启动、Windows 原生 style、真实键盘/读屏/HIDPI、UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或正式签名发布验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## UI-1.44 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.44`
- size：`47,804,976` bytes
- canonical/root SHA-256：`6D58AF332F7BA492973C0075DF1ECE12086A4A6892C7FE95862B29CDE304AC38`
- archive listing SHA-256：`83B52335762EE094ACE2816DC018D5E7D4AFCB48A6555893C0A0D8FAC0FF3331`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.43 连接快速配置自定义选中态与语义上下文

- `connection_preset_surface.py` 新增 `is_custom_connection_preset()` 与
  `update_connection_preset_context()`，统一 refresh、用户选择两条路径的 `customSelected`、tooltip 和
  `AccessibleDescription`；未选择、内置、自定义三态不再共享固定“内置”文案。
- `connection_builder.py` 复用 surface predicate/context，不复制 builtin key 判断；`connection_presets.py` 不再在
  `apply_connection_preset()` 中覆盖 ComboBox 上下文 tooltip，只继续更新 connection hint。
- `theme_stylesheet_base.py` 与 `theme_variant_shell.py` 消费
  `QComboBox#connectionPresetCombo[customSelected="true"]` 的 normal/hover/focus/disabled 规则；
  `scripts/check_theme_tokens.py` 将该 selector 纳入 12 项必需 selector，未新增 timer、资源、token、transport 或业务状态。
- 新增 ADR [`0030-connection-preset-custom-state-context.md`](../adr/0030-connection-preset-custom-state-context.md)。

### UI-1.43 验证、审查与简化

```text
scripts/check.ps1       PASS 126 files <= 1000; 3 themes; 22 semantic tokens; 12 selectors; ruff
UI143_CONTEXT            PASS 3 themes; empty/builtin/custom AccessibleDescription + tooltip; controller dispatch;
                         custom/disabled visual delta; near_white_custom=0; near_white_disabled=0
UI143_IMPORT/COMPILE     PASS modules=125 qapplication_instance=False; compile pass
ARCH_REVIEW              parent-audited; architecture agents 019fea60-ce0c-7c42-bd98-519bb7bb8ec1 returned
                         conditional GO; follow-up 019fea66-c9eb-7fa3-bc24-0146edf60ee7 timed out twice and closed
QUALITY_REVIEW           parent five-axis review; independent reviewer 019fea65-5184-7913-9b40-6cd5dcffc761 timed out twice and closed
SIX_ROLE_REVIEW         product 019fea60-cdc5-7c11-9c4c-fb82f6d3eb13; UI 019fea60-8bfd-7641-af97-fde9c36abc99;
                         development 019fea60-8c4a-7640-8e5c-4a0ccd714969; verification 019fea6b-434f-7661-bc7c-97fd518dd1d1
                         and packaging 019fea6b-4392-7120-8ea4-b056e3ce34e3 each timed out twice and closed; parent evidence retained
```

截图：`build/ui_review_ui143_preset_star_trail.png`、`build/ui_review_ui143_preset_moonlit_ocean.png`、
`build/ui_review_ui143_preset_sakura_night.png`。离屏环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。

未运行持续 GUI、EXE 启动、真实 Tab/键盘/读屏/HIDPI、UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或正式签名发布验收；
未创建、修改或运行 unit test、mock、fixture、harness 或 test-only 资产。

## UI-1.43 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.43`
- size：`47,805,312` bytes
- canonical/root SHA-256：`101DA448C2B95061718230B70715370EC0D29815271F8FBF7E741015AD82AD83`
- archive listing SHA-256：`1DE0907DDB8F3A9FA3C0040B991ED18A316A2CD69BDE808402699F8B601DF6B5`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.42 批量编辑器原生控件主题回退

- `CommandBatchEditorDialog` 隐藏与横向“步骤”列重复的 `QTableWidget.verticalHeader()`，消除真实主题下左侧系统白色列，
  保留 row selection、`currentCellChanged`、AccessibleName/Description、步骤数据和保存逻辑。
- 默认 controls stylesheet 与三套 theme override 显式覆盖 `QSpinBox/QDoubleSpinBox` 的 `::up-button`、`::down-button`、
  `::up-arrow`、`::down-arrow` 背景、hover、disabled 和 token 箭头颜色；保留原生上下调节/键盘能力，不影响 QComboBox。
- 新增 ADR 0029，记录白色回退根因、拒绝关闭 stepper/外部图片方案和主题/QSS 边界。

### UI-1.42 验证、审查与简化

```text
scripts/check.ps1       PASS 126 files <= 1000; theme token audit; ruff
UI142_EDITOR             PASS star_trail/moonlit_ocean/sakura_night near_white=0
UI142_HEADER             PASS vertical_header=False; horizontal_header=True; columns=4
UI142_GEOMETRY           PASS table=738x259 at 760x620 dialog
IMPORT/COMPILE           PASS 125 modules/no QApplication; compileall=pass
ARCH_REVIEW              parent-audited; architecture agents 019fea52-20d7-7283-8bb0-dab5e8f2f30d and
                         019fea54-8200-7fc3-b095-a29e03980bda each timed out twice and closed
QUALITY_REVIEW           parent five-axis review; independent reviewer 019fea57-6b07-7d72-b14d-62d09b6ec056 timed out twice and closed
```

截图：`build/ui_review_ui142_editor_star_trail.png`、`build/ui_review_ui142_editor_moonlit_ocean.png`、
`build/ui_review_ui142_editor_sakura_night.png`。offscreen 环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。

## UI-1.42 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.42`
- size：`47,804,190` bytes
- canonical/root SHA-256：`9B00BD6BB307B4D16C7665ADB1557ED347917B8DE50F0AADB359D443A783F452`
- archive listing SHA-256：`1DE0907DDB8F3A9FA3C0040B991ED18A316A2CD69BDE808402699F8B601DF6B5`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.41 批量命令空态 CTA 与结果表互斥可见性

- 新增 `presentation/action_surface.py:ActionRailButton`，保留原生 `QPushButton`、`primaryButton` QSS、点击/焦点/无障碍契约，
  只在原生绘制后叠加主题 `ThemeSpec.accent` 底部 signal rail；`terminal_surface.py` 的 `TerminalActionButton` 保留为兼容导出，
  不再复制 painter 实现。
- 新增 `presentation/command_batch_empty_state.py:CommandBatchEmptyState` 与 resource-free step glyph；组件只发出无参数
  `new_requested` intent，`controllers/terminal.py` 显式接入 `commands.py` 的既有 `new_command_batch_action`，不接触 ViewModel、
  domain、transport 或 dialog 业务状态。
- `commands.py` 在 `batch is None` 时隐藏 `CommandBatchSurfaceLabel` 与空 `QTableWidget`，完整显示空态 CTA；选中 batch 后恢复 status
  surface/只读结果表并隐藏空态。`connection.py` 同时 gate 顶部和空态两个“新建”入口，历史/执行/BLE notification pending 期间不绕过锁。
- `lifecycle.py` 只把已有 MotionController frame/stop fan-out 到新 surface；没有新增 QTimer、业务字段、第二套状态源或外部资源。
  ADR：[`0028-command-batch-empty-state-cta.md`](../adr/0028-command-batch-empty-state-cta.md)。

### UI-1.41 验证、审查与简化

```text
scripts/check.ps1       PASS 126 files <= 1000; theme token audit; ruff
IMPORT_SMOKE            PASS modules=125 qapplication_instance=False
UI141_EMPTY             PASS component_signal; shared frame motion; CTA visible
UI141_VISIBILITY        PASS empty:status=False/table=False -> batch:status=True/table=True; row restore=1
UI141_RESPONSIVE        PASS 980/1180 sizes=(910x98,1110x98); CTA width=106; viewport clipping=none
UI141_THEME             PASS star_trail/moonlit_ocean/sakura_night near_white=0
UI141_LIFECYCLE         PASS pause static; hide static; close timer=False
ARCH_REVIEW             parent-audited; architecture agents 019fea3f-85ac-7f12-bc5c-5ccbb81d5b96,
                        019fea46-8669-7943-9c6c-74a4a908e5a4, 019fea48-ea6c-7d73-afcc-a3fe37e77e67 each timed out twice and closed;
                        no proxy conclusion treated as GO
QUALITY_REVIEW          parent five-axis review; independent reviewer 019fea4b-2da9-7d21-9b5c-a73117959a8a timed out twice and closed;
                        no proxy conclusion treated as GO
```

截图：

- `build/ui_review_ui141_command_empty_980.png`
- `build/ui_review_ui141_command_empty_1180.png`
- `build/ui_review_ui141_command_empty_star_trail.png`
- `build/ui_review_ui141_command_empty_moonlit_ocean.png`
- `build/ui_review_ui141_command_empty_sakura_night.png`

offscreen 环境缺少 PySide6 fonts directory，中文方框仅为环境告警；没有创建、修改或运行 unit test、mock、fixture、harness 或
test-only 资产。未运行持续 GUI、HIDPI/读屏、真实 UART/TCP/UDP/BLE/RTT/J-Link、OTA、硬件或正式签名发布验收。

## UI-1.41 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录覆盖：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.41`
- size：`47,803,326` bytes
- canonical/root SHA-256：`D21835D2C436FA2245AF58CF4063D4B6C03A2562E83A726FF82CCF12ACCC107A`
- archive listing SHA-256：`1DE0907DDB8F3A9FA3C0040B991ED18A316A2CD69BDE808402699F8B601DF6B5`
- provenance：manifest verify pass；`signature=NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.40 终端空态 CTA 共享 frame signal rail

- `TerminalActionButton` 继续使用原生 `QPushButton`、`primaryButton` QSS、原文字和无障碍契约；原生按钮绘制完成后，
  在底部叠加低对比度主题 accent signal rail。rail 只消费 `(phase, animated)` 和 `ThemeSpec.accent`，不参与导航或连接。
- `TerminalEmptyState.set_frame/stop` 转发到 CTA，复用现有 `lifecycle._motion_surfaces` 与唯一 `MotionController`；
  没有新增 QTimer、业务字段、lifecycle 注册、外部资源或第二套动画时钟。paused/history CTA 隐藏时不绘制可见 rail。
- 新增 ADR 0027，约束 QPainter/QSS 叠加、静态回退、主题与生命周期边界。

### UI-1.40 验证、审查与简化

```text
scripts/check.ps1       PASS 124 files <= 1000; theme token audit; ruff
IMPORT_SMOKE            PASS modules=123 qapplication_instance=False
UI140_RAIL              PASS phase=π/2↔3π/2 moving_diff=34; stop animated=False
UI140_SHARED_FRAME      PASS 0.000 -> 0.300; animated=True; timer=True
UI140_RESPONSIVE        PASS 1180x780 surface=1152x192 card=378x128; hscroll=False; maximum=0
UI140_LIFECYCLE         PASS pause timer=False/animated=False; hide static; close timer=False
THEME_PIXEL_AUDIT       PASS star_trail/moonlit_ocean/sakura_night near_white=0
SCREENSHOTS             build/ui_review_ui140_cta_rail_star_trail.png; build/ui_review_ui140_cta_rail_moonlit_ocean.png;
                        build/ui_review_ui140_cta_rail_sakura_night.png; build/ui_review_ui140_cta_rail_1180.png
ARCH_REVIEW             parent-audited; architecture agent 019fea2f-34fd-7f91-9122-599a2f641b12 timed out twice and closed
QUALITY_REVIEW          parent five-axis review; independent reviewer 019fea37-90b8-7191-b136-fcd4537b2869 timed out twice and closed
```

第一次 rail 像素 smoke 使用 `0` 与 `π`，因两者 `sin()` 都为 0 而产生 false negative；调试后改用 `π/2` 与 `3π/2`，
定位和验证均通过。简化结论：只增加一个 terminal-owned renderer，复用既有 frame/stop fan-out，不复制业务状态和时钟。
offscreen 环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。

## UI-1.40 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录副本：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.40`
- size：`47,793,505` bytes
- SHA-256：`B81A72954F75835E11CE38832AC1F5CEE4768CD6310D4A306352ABE1F8D67A6B`
- archive listing SHA-256：`0A1F0D57EB1DA293E6ABFD1384653D9CAB802DEC5AF7FC91091702B4F618C26E`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.39 终端空态连接导航 CTA

- `TerminalEmptyState` 新增无参数 `connection_requested` signal 和可访问的 `primaryButton` CTA；只在
  `idle/waiting/transition` 显示，`paused/history` 隐藏。它只表达“打开链路连接页”的导航意图，不自动连接，
  不读取 transport/session 配置，不改变 ViewModel 状态。
- `bootstrap.py` 通过显式 `partial` 接入 `workspace_runtime.select_workspace_tab(window, 0)`；runtime 是唯一
  workspace navigation owner，负责 closing fence、Tab count/index、真实 Tab 切换和焦点恢复。
- 空态 overlay 不再对所有鼠标事件透明，以便 CTA 可点击；父级仍只承担 presentation 背景绘制，shared MotionController、
  theme token、静态 stop 和 existing terminal content contract 保持不变。详见 [`ADR 0026`](../adr/0026-terminal-empty-state-navigation-intent.md)。

### UI-1.39 验证、审查与简化

```text
scripts/check.ps1       PASS 124 files <= 1000; theme token audit; ruff
IMPORT_SMOKE            PASS modules=123 qapplication_instance=False
EMPTY_CTA_980           PASS idle/waiting/transition visible; paused/history hidden; click -> Tab 0 + focus; session unchanged
EMPTY_RESPONSIVE_1180   PASS surface=1152x192 card=378x128 hscroll_visible=False maximum=0; click -> Tab 0
THEME_PIXEL_AUDIT       PASS star_trail/moonlit_ocean/sakura_night near_white=0
SCREENSHOTS             build/ui_review_ui139_empty_cta_star_trail.png; build/ui_review_ui139_empty_cta_moonlit_ocean.png;
                        build/ui_review_ui139_empty_cta_sakura_night.png; build/ui_review_ui139_empty_cta_1180.png
ARCH_REVIEW             parent-audited; architecture agent 019fea22-d380-7181-a745-0c70ae618a09 timed out twice and closed
QUALITY_REVIEW          parent five-axis review; independent reviewer 019fea28-6ca3-72c1-938c-ca5dd09f1ff4 timed out twice and closed
```

简化结论：使用现有 `primaryButton` 和 shared MotionController，没有新增 token/timer；把导航 action 放入
`workspace_runtime.py`，避免 TerminalEmptyState 持有 Tab widget 或连接副作用；按钮是唯一新增交互控件，状态条件、
无障碍元数据和焦点路径均显式可审计。offscreen 环境缺少 PySide6 fonts directory，中文方框不代表 Windows 字体结论。

## UI-1.39 打包

- canonical：[dist/release/0.1.0/core/onefile/app/SerialForge.exe](../../dist/release/0.1.0/core/onefile/app/SerialForge.exe)
- 根目录副本：[SerialForge.exe](../../SerialForge.exe)，与 canonical 字节一致
- source revision：`local-ui-1.39`
- size：`47,796,629` bytes
- SHA-256：`4FE6A2C69C12EDCBD5639FB163A332B5E38D62CFE4803C7F9B731F34AED63988`
- archive listing SHA-256：`0A1F0D57EB1DA293E6ABFD1384653D9CAB802DEC5AF7FC91091702B4F618C26E`
- signature：`NotSigned`；`release_eligible=false`；`hardware_acceptance=not_run`

## 2026-08-10 UI-1.38 工作区路线 beacon 与 Qt corner 裁切修复

- 新增 `presentation/workspace_route_surface.py:WorkspaceRouteSurface`，绘制三节点工作区路线、当前 Tab 高亮和共享 frame pulse；
  只读取 bounded index、ThemeSpec 和 `(phase, animated)`，不读 ViewModel/domain，不创建 QTimer，不进入焦点/无障碍顺序。
- 首次实现放在 `QTabWidget.setCornerWidget()` 后，离屏实测 geometry 为 `x=953`、父宽 `952`，导致 beacon 被裁切；已按根因改为
  `workspaceShell` + `workspaceRouteStrip`，真实 Tab widget 仍由 `window._workspace_tabs` 持有，route strip 只做右对齐装饰承载。
- 架构师 Luna max `019fea16-2615-75b0-99fc-8c75b786f434` 两次等待未返回结论后关闭；父代理完成 Qt ownership、响应式空间、focus/accessibility、
  共享时钟和可回滚性审计。格式修复的架构师调用 `019fea18-6a20-7a53-8f27-8efad61c45d1` 也超时关闭，未把超时当作 GO。

### UI-1.38 验证与简化

```text
ROUTE_RESPONSIVE  980x680 shell=952 tabs=952x220 route=814,3,132,28 visible=True hscroll=0
ROUTE_RESPONSIVE  1180x780 shell=1152 tabs=1152x220 route=1014,3,132,28 visible=True hscroll=0
UI138_ROUTE       PASS indices=[0,1,2] frames=15 hidden_timer=False restored_timer=True closed_timer=False
UI138_CONTROLS    PASS pause=(False,False) resume=(True,True) reduced=(False,False) restore=(True,True) close=(False,False)
THEME_AUDIT       star_trail/moonlit_ocean/sakura_night near_white=0
IMPORT_SMOKE      PASS modules=124 qapplication=none
CODE_REVIEW       PASS tab-index-owner / focused-renderer / workspace-shell / no-input / shared-frame / no-focus
STATIC            PASS scripts/check.ps1; 124 files <= 1000; theme token audit; ruff
SCREENSHOTS       build/ui_review_ui138_route_980.png; build/ui_review_ui138_route_1180.png
```

简化结论：没有把 Tab index 复制成业务状态，也没有引入第二个时钟；用一个独立 renderer 和一个 layout-owned strip 修复 Qt style
几何问题，保留静态选中节点和所有原有 Tab 行为。offscreen 环境缺少 PySide6 fonts 目录，中文方框是环境告警，不代表 Windows 字体结果。

## 2026-08-10 UI-1.37 共享环境动效与生命周期边界

- `MotionController` 增加 `ambient_active` 旗标；它只控制可见空闲 shell 是否持续使用既有 96ms 共享时钟出帧，未新增控件级
  `QTimer`、业务字段、RX/transport 事实或第二套 motion controller。
- `lifecycle.showEvent()` 开启 ambient；`hideEvent()`、最小化分支继续通过 `set_suspended(True)` 停止；低动效、显式暂停和
  close fence 仍优先，关闭时先关闭 ambient 再 `close()`。所有现有 signal-field/status surface 都保留静态回退。
- 架构师 Luna max `019fea0c-02b7-7e72-b8a8-4f6712a3ec3c` 两次等待共约 90 秒未返回结论，已关闭；未将超时当作 GO。父代理完成
  只读依赖/生命周期/性能/无障碍审计后才实施最小切片。

### UI-1.37 独立复核、简化与验证

```text
BASELINE_MOTION       timer_active=False frames=0 animated=0
UI137_MOTION          PASS baseline_timer=False baseline_frames=0 ambient_timer=True ambient_frames=3 animated=3
                      suspended_timer=False resumed_timer=True reduced_timer=False restored_timer=True closed_timer=False
UI137_COMPOSITION     PASS visible=True ambient=True ambient_timer=True animated_frames=5 hidden_timer=False restored_timer=True closed_timer=False
IMPORT_SMOKE          PASS modules=124 qapplication=none
PIXEL_AUDIT           PASS size=980x680 near_white=0 white=0 bytes=51086
scripts/check.ps1     pass (123 files <= 1000, theme token audit pass, ruff pass)
SCREENSHOT            build/ui_review_ui137_ambient_motion.png (人工查看无白色大块/白色链路行)
```

简化结论：新增状态只有 shared clock 的生命周期开关，未把 ambient 逻辑分散到 renderer；`set_motion_enabled`、`set_paused`、
`set_suspended` 和 `close` 继续作为单一停止入口。性能只记录同条件离屏帧数，未声称 CPU/内存预算通过。

嵌入式适用性：本切片仅修改 Python/PySide6 presentation，不涉及 MCU、BSP/HAL/C/C++、RTOS、固件或厂商 SDK；
`embedded-enterprise-workflow` 与 `embedded-code-review-simplifier` applicability=`N/A`，不声称 MISRA、ISO、WCAG、认证或硬件合规。

## 2026-08-10 ARCH-6d 连接动作 callback owner 收窄

- `contracts.py` 新增 `ConnectionActionCallback`；`workspace.py` 将 `connection_runtime.toggle_connection` 显式注入
  `connection_builder.py`，builder 只负责连接 Qt button signal，不再读取 `MainWindow._toggle_connection`。
- `connection_runtime.toggle_connection()` 在 owner action 边界吸收 Qt clicked 的 checked/payload 参数；session gate、错误栏、
  按钮文案、transport config 和关闭时序保持不变。`MainWindow._toggle_connection` 已删除，主窗口从 811 行降至 803 行。
- 架构师 Luna max `019fe9ca-cdd0…` 在等待/立即返回窗口内未给出结论，已关闭；主代理按最小 callback 边界继续，不把超时当作 GO。

### ARCH-6d 验证与限制

```text
targeted compileall / ruff                       pass
ARCH6D_CONNECTION_CALLBACK                       pass (main-facade=removed, themes=3, sizes=2, motion-stop=pass)
ARCH6D_SCREENSHOT                                pass (build/ui_review_arch6d_connection_callback.png)
package                                          pass

ARCH6D_PACKAGE_FINAL                             pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,785,910 bytes
  SHA256: `674AF78618BEFE9C61618FFF4B2FECE39C617D067C13CEFC3F7DC590F1A1B8F3`
  provenance/archive/hash: pass; signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；离屏环境缺少 PySide6 fonts 目录，截图中文方框不代表
Windows 字体缺失。未创建、修改或运行 unit test、mock、fixture、harness。

## 2026-08-10 ARCH-6e 纯展示 formatter owner 收窄

- 新增 `presentation/formatters.py`，集中无状态的 `endpoint_label()` 与 `history_label()`；`connection_runtime.py` 和
  `terminal_runtime.py` 直接导入，删除 `MainWindow._endpoint_label` / `_history_label` 两个静态 facade。
- formatter 只依赖 domain DTO，不持有窗口、Qt 对象或 controller；从 lifecycle 移出后避免 lifecycle 与 connection runtime
  形成反向循环依赖。MainWindow 从 803 行降至 789 行，行为、端点身份、历史预览、signal 和生命周期保持不变。
- 架构师 Luna max `019fe9d1-1441-7702-b6fe-2388b0ac65a9` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。

### ARCH-6e 验证与限制

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (118 files <= 1000)
ARCH6E_FORMATTER_BOUNDARY                   pass (facade=removed, formatters=2, themes=3, sizes=2, hscroll=0,
                                                  motion-stop=pass, near-white=0, screenshot=pass)
PACKAGE_ARCH6E_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,414 bytes
  SHA256: `40EDB44BF15AEC6F78295933BDAB379B731CC9B49C1343F1EF6F1BC7E9691D62`
  provenance/archive/hash: pass; archive listing SHA256 `E047A35E47FA63807D59624CBB16ADD3599CEC810590EC314F3CBD69C8688D7D`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

截图：`build/ui_review_arch6e_formatter.png`。离屏环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 打包环境字体缺失。
未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；未创建、修改或运行 unit test、mock、fixture、harness。
项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6f 动态属性刷新 owner 收窄

- 新增 `presentation/property_refresh.py`，提供唯一 `refresh_dynamic_property()`：值未变化时跳过 polish，变化时按
  `setProperty` → `unpolish/polish` → `update` 顺序刷新 QSS。bootstrap 直接把它注入 `StatusSurfaceController`；
  lifecycle、connection、commands、terminal controller 显式导入它。
- 删除 `MainWindow._set_dynamic_property` 与 lifecycle 中的同名 facade；不改变状态投影、主题 selector 命中、Qt signal 或
  生命周期。MainWindow 从 789 行降至 782 行，新文件 20 行。
- 架构师 Luna max `019fe9d8-ffe3-7443-8ef9-75369bcfccb1` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。

### ARCH-6f 验证与限制

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (119 files <= 1000)
ARCH6F_PROPERTY_REFRESH                      pass (facade=removed, property-module=explicit, qss-refresh=pass,
                                                  status-callback=pass, themes=3, sizes=2, hscroll=0,
                                                  motion-stop=pass, near-white=0, screenshot=pass)
PACKAGE_ARCH6F_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,788,203 bytes
  SHA256: `C69A8343055B383B83954C65F852F3FA3B22A4428E591C95FEE7B48899065847`
  provenance/archive/hash: pass; archive listing SHA256 `9836C898423B2A3E131EC2C30EBC55D40B70ED775FDCE3FE442ED8DFF05B2B52`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

截图：`build/ui_review_arch6f_property_refresh.png`。离屏环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 打包环境字体缺失。
未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；未创建、修改或运行 unit test、mock、fixture、harness。
项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6g owner 归还：状态来源与 TCP Server readiness

- `status_surface_source()` 归还 `controllers/status_surfaces.py`，bootstrap 通过显式 source callback 接入
  `StatusSurfaceController`；删除 `MainWindow._status_surface_source`。
- `tcp_server_readiness()` 归还 `controllers/connection.py`，保留 IPv4、回环/LAN 确认、allowlist 数量和原有错误文案；
  connection controls 直接调用，删除 lifecycle 中的错误归属函数和 `MainWindow._tcp_server_readiness`。
- MainWindow 从 782 行降至 770 行；未改变连接按钮、LAN 授权 gate、状态 source、Qt signal 或生命周期。
- 架构师 Luna max `019fe9dd-5a2c-7a23-a36e-4a60c9d98521` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。

### ARCH-6g 验证与限制

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (119 files <= 1000)
ARCH6G_OWNER_MIGRATION                       pass (facades=2-removed, status-source=owner, tcp-readiness=owner,
                                                  loopback=pass, lan-gate=pass, themes=3, sizes=2, hscroll=0,
                                                  motion-stop=pass, near-white=0, screenshot=pass)
PACKAGE_ARCH6G_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,788,341 bytes
  SHA256: `840F1EC82602147B021E513BC164D79732D53C9F5498DBAF14841AD9F779D103`
  provenance/archive/hash: pass; archive listing SHA256 `9836C898423B2A3E131EC2C30EBC55D40B70ED775FDCE3FE442ED8DFF05B2B52`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

截图：`build/ui_review_arch6g_owner_migration.png`。离屏环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 打包环境字体缺失。
未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；未创建、修改或运行 unit test、mock、fixture、harness。
项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6h typed command selector owner 收窄

- 新增 `presentation/command_selection.py`，集中 `current_send_mode()` 与 `selected_command_batch()` 两个只读 selector；
  `CommandMode` fallback、batch `Qt.ItemDataRole.UserRole` 和 `None` 语义保持不变。
- `commands.py`、`connection.py`、`terminal_runtime.py` 显式依赖 selector；删除 `MainWindow._current_send_mode` /
  `_selected_command_batch`，不引入 controller-to-controller 依赖。MainWindow 从 770 行降至 758 行，新文件 24 行。
- 架构师 Luna max `019fe9e2-966e-75a3-a0a9-ba961238e0f9` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。

### ARCH-6h 验证与限制

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (120 files <= 1000)
ARCH6H_COMMAND_SELECTION                     pass (facades=2-removed, mode=hex, batch-selection=pass,
                                                  qt-data=preserved, themes=3, sizes=2, hscroll=0,
                                                  motion-stop=pass, near-white=0, screenshot=pass)
PACKAGE_ARCH6H_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,535 bytes
  SHA256: `EB8D47D5F0262CE5BD0E5DDE78BB3A2999EFE7EC7B4CAC1AB217AE1CAE481B6D`
  provenance/archive/hash: pass; archive listing SHA256 `55C209CD1E9B0ECD92A27A863EDF767BF563F740DEADC1F359BA25A7FF56AC77`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

截图：`build/ui_review_arch6h_command_selection.png`。离屏环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 打包环境字体缺失。
未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；未创建、修改或运行 unit test、mock、fixture、harness。
项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6i 发送/批量 action owner 收窄

- commands owner 增加 `send_current_action()`、`new/edit/delete/run_command_batch_action()` 五个 Qt action adapter，统一
  吸收 checked/payload 后调用原 action；terminal builder 通过 `partial` 接入发送和批量按钮，composition 的 Ctrl+Enter
  直接接入发送 action。
- 删除 `MainWindow._send_current`、`_new_command_batch`、`_edit_command_batch`、`_delete_command_batch`、
  `_run_command_batch`；原有 dialog parent、ViewModel batch semantics、发送 gate、快捷键和生命周期保持不变。
- MainWindow 从 758 行降至 728 行；架构师 Luna max `019fe9e7-b9d2-7a03-ab38-8528f9730ea6` 在等待窗口内未返回结论，
  已关闭；未把超时解释为 GO。

### ARCH-6i 验证与限制

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (120 files <= 1000)
ARCH6I_COMMAND_ACTIONS                       pass (facades=5-removed, qt-click=pass, payload-adapter=pass,
                                                  batch-action-boundary=pass, shortcut-boundary=pass, themes=3,
                                                  sizes=2, hscroll=0, motion-stop=pass, near-white=0, screenshot=pass)
PACKAGE_ARCH6I_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,788,394 bytes
  SHA256: `4818D59EDE3F4984B1DBE6EE566A01BEC7805874CDEC2E70627B18F88D8FE5B8`
  provenance/archive/hash: pass; archive listing SHA256 `55C209CD1E9B0ECD92A27A863EDF767BF563F740DEADC1F359BA25A7FF56AC77`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

截图：`build/ui_review_arch6i_command_actions.png`。离屏环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 打包环境字体缺失。
未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；未创建、修改或运行 unit test、mock、fixture、harness。
项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6m protocol scope owner 边界

- 新增 `presentation/protocol_scope.py`，只读取 transport/history presentation facts，提供
  `parser_pipeline_supported()`、`derived_source_supported()` 和 `derived_source_unavailable_text()`。
- `protocol_config.py` 保留 editor enable/reset/status 等副作用；connection/lifecycle/derived/workspace/terminal 直接消费 query，
  删除 `MainWindow._parser_pipeline_supported`、`_derived_source_supported`、`_derived_source_unavailable_text` 三个 facade。
  MainWindow 从 652 行降至 634 行，UART/TCP Client 派生和 UDP/raw-only gate 保持不变。
- 架构师 Luna max `019fea00-d301-7750-8ed7-3f7129155315` 在等待窗口内超时并关闭，未把超时解释为 GO；父代理完成调用图审计。

### ARCH-6m 验证与限制

```text
python -m compileall / scripts/check.ps1       pass
source line limit                             pass (123 files <= 1000)
ARCH6M_PROTOCOL_SCOPE                         pass (facades=3-removed, pure-scope=pass,
                                                  uart-gate=pass, udp-raw-only=pass, theme-switch=pass,
                                                  motion-stop=pass, near-white=0, screenshot=pass)
截图                                          build/ui_review_arch6m_protocol_scope.png
PACKAGE_ARCH6M_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,790,125 bytes
  SHA256: `9A82899684B8C162BB8AC3EBF727982CA52F1E4772081BAD9175A21D4E40DF66`
  provenance revision `local-arch-6m`; archive listing SHA256 `2C686B9AFB0B1E4C116CD0A51740BCA02C524E4F485E6F1942078B12C5F822BF`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏环境提示 PySide6 fonts 目录缺失，中文可能显示为方框；这不等同于 Windows 字体验收。未创建、修改或运行 unit test、mock、
fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件。项目为 Python/PySide6 桌面应用，不含
嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。

### Embedded R&D assurance gate

- 适用性：N/A。当前仓库是 Python 3.12/PySide6 Windows 桌面应用，本轮未修改 MCU、BSP/HAL/CMSIS、RTOS、ISR/DMA、固件或
  嵌入式 C/C++；因此没有可适用的 MCU/SDK/RTOS 厂商一手约束，也不宣称 MISRA、ISO 26262、ASIL、ASPICE 或认证合规。
- 独立复核：Luna max `019fea05-3ad1-7aa3-96f2-76e9ec38e3f1` 在等待窗口内超时并关闭，未返回 findings；父代理随后完成只读依赖、
  行为与简化审计，并将该超时保留为未获得代理结论，不视为 GO。
- 简化评估：`protocol_scope.py` 是无副作用查询 owner，移除 3 个纯 MainWindow facade；保留 protocol editor enable/reset/status
  的副作用和现有历史/UART/TCP/UDP gate，未改变生命周期、资源或业务接口。
- 非破坏性验证：`scripts/check.ps1` pass；123 模块导入 smoke pass 且无 QApplication；ARCH6M offscreen pass；onefile provenance、
  archive listing、root/artifact hash 一致 pass。未运行 EXE 启动、持续 GUI、HIDPI/读屏、真实传输、OTA/J-Link/RTT、硬件或正式签名发布。

## 2026-08-10 ARCH-6l BLE selection owner 边界

- 新增 `presentation/ble_selection.py`，只读取 bounded BLE write-mode combo 和 characteristic capability；保留
  `Qt.ItemDataRole.UserRole`、enum fallback、`None`/空 capability 语义。
- `ble.py` 保留修改 combo 的 `normalize_ble_write_mode()` action，并直接导入 selector；`connection.py` 与 `commands.py` 直接
  导入 selector；删除 `MainWindow._selected_ble_write_mode`、`_supported_ble_write_modes`、`_normalize_ble_write_mode` 三个 facade。
  MainWindow 从 672 行降至 652 行。
- 架构师 Luna max `019fe9fb-ef39-74f3-83be-5150507a1231` 在等待窗口内超时并关闭，未把超时解释为 GO；父代理完成调用图审计。

### ARCH-6l 验证与限制

```text
python -m compileall / scripts/check.ps1       pass
source line limit                             pass (122 files <= 1000)
ARCH6L_BLE_SELECTION                          pass (facades=3-removed, pure-selection=pass,
                                                  mode=with_response, theme-switch=pass,
                                                  motion-stop=pass, near-white=0, screenshot=pass)
截图                                          build/ui_review_arch6l_ble_selection.png
PACKAGE_ARCH6L_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,928 bytes
  SHA256: `11B539A8A442A92C01C02FDCD7442E94547F4293500E0C0DC321495F2642C3F1`
  provenance revision `local-arch-6l`; archive listing SHA256 `83E4D141BBD5B7990D22742F91F30E9D13EEC35490092FA822B7C2A659F0DE60`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏环境提示 PySide6 fonts 目录缺失，中文可能显示为方框；这不等同于 Windows 字体验收。未创建、修改或运行 unit test、mock、
fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件。项目为 Python/PySide6 桌面应用，不含
嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6k motion callback 直连与 workspace runtime 边界

- `workspace.py` 的 MotionController frame、低动效、暂停动效和主题 combo 直接用显式 `partial(owner, window)` 接线；主题首屏
  直接调用 `on_theme_changed(window)`。删除 `MainWindow._on_motion_toggled`、`_on_theme_changed`、
  `_on_motion_pause_toggled`、`_on_motion_frame`、`_set_data_activity_motion` 五个纯 facade，MainWindow 从 702 行降至 672 行。
- 首次离屏组装发现 `workspace.py -> lifecycle.py -> workspace.py` 循环导入。新增 `controllers/workspace_runtime.py`，收拢
  Tab 可见性、派生 surface 挂起、workspace motion policy 和一次性淡入过渡；`workspace.py` 只组装/接线，`lifecycle.py` 只消费
  runtime helper，修复 ImportError 而不使用延迟动态导入或 MainWindow facade。
- 本轮 Luna max `019fe9f2-5c68-7061-b5bb-b38046c5b141` 与 Terra max `019fe9f5-6d49-78d3-893d-fae5298c3be5`
  均在等待窗口内超时并关闭，未把超时解释为 GO；父代理按完整错误证据完成依赖图审计和最小 owner 拆分。

### ARCH-6k 验证与限制

```text
python -m compileall / scripts/check.ps1       pass
source line limit                             pass (121 files <= 1000)
ARCH6K_MOTION_LIFECYCLE                       pass (facades=5-removed, runtime-boundary=pass,
                                                  theme-switch=pass, frame-signal=pass, motion-toggles=pass,
                                                  near-white=0, screenshot=pass)
截图                                          build/ui_review_arch6k_motion.png
PACKAGE_ARCH6K_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,789,020 bytes
  SHA256: `39B258DDB9DB43FFF15F13C744EAA9731E48DE8DE3A49A4945724FE10C96F7E4`
  provenance revision `local-arch-6k`; archive listing SHA256 `6483837FF1A50CDEE49E5B7DE49BC29FC607F96B808897AC641E78AA1A17730A`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

离屏环境提示 PySide6 fonts 目录缺失，中文可能显示为方框；这不等同于 Windows 字体验收。未创建、修改或运行 unit test、mock、
fixture、harness；未启动持续 GUI/EXE、真实 UART/TCP/UDP/BLE/RTT/J-Link/OTA 或硬件。项目为 Python/PySide6 桌面应用，不含
嵌入式 C/C++/固件；embedded-enterprise-workflow 与 embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-6j command signal/结果 owner 直连

- commands controller 内部直接调用 `on_command_batch_selection_changed()` 和
  `render_command_batch_results()`；terminal batch combo 使用显式 `partial` 接入 selection handler，bootstrap 用
  `partial` 接入 ViewModel batch signals，并以相同 owner 函数完成首屏 projection。
- 删除 `MainWindow._on_command_batches_changed`、`_on_command_batch_selection_changed`、`_on_command_batch_changed`、
  `_render_command_batch_results`；保留 signal 顺序、首屏 snapshot、批量状态轨道和 connection controls refresh。MainWindow
  从 728 行降至 702 行。
- 架构师 Luna max `019fe9ed-3ddb-7492-a242-1aab107c7e11` 在等待窗口内未返回结论，已关闭；未把超时解释为 GO。

### ARCH-6j 验证与限制

```text
python -m compileall -q src                  pass
scripts/check.ps1                            pass (120 files <= 1000)
ARCH6J_COMMAND_SIGNALS                       pass (facades=4-removed, bootstrap-projection=pass, combo-signal=pass,
                                                  themes=3, sizes=2, hscroll=0, motion-stop=pass, near-white=0, screenshot=pass)
PACKAGE_ARCH6J_FINAL                         pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,787,662 bytes
  SHA256: `10591D3A3316F6775701331958F828C9DB8187F165821950DB31481C9EE531AB`
  provenance/archive/hash: pass; archive listing SHA256 `55C209CD1E9B0ECD92A27A863EDF767BF563F740DEADC1F359BA25A7FF56AC77`
  signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`
```

截图：`build/ui_review_arch6j_command_signals.png`。离屏环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 打包环境字体缺失。
未启动真实设备、GUI/EXE 持续进程、串口、网络、BLE、RTT/J-Link 或 OTA；未创建、修改或运行 unit test、mock、fixture、harness。
项目为 Python/PySide6 桌面应用，不含嵌入式 C/C++/固件；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A。

## 2026-08-10 ARCH-7b preset catalog store 边界

- 新增 `presentation/connection_preset_codec.py`：schema v1 的显式 JSON 白名单 codec，只序列化安全显示/连接选项，拒绝未知
  字段、未知 transport、超长 payload、坏 catalog，以及 bool/float 冒充整数；DTO 和 enum 的严格校验在 presentation DTO
  构造层完成。
- 新增 `presentation/connection_preset_store.py`：`ConnectionPresetCatalogStore` port 与唯一 Qt
  `QSettingsConnectionPresetCatalogStore` adapter。组合根加载 catalog 后，把 immutable snapshot 注入 `MainWindow`；
  `connection_builder.py` 只接收显式 catalog，不直接读取 QSettings。坏值 fail-open 到七项 builtin catalog；保存时 begin/set/end
  任一步骤失败都跳过 sync，避免把半写入状态当成成功。
- 当前切片刻意不包含自定义 preset editor、builtin/custom merge 语义或显式 save action；这些不是“已提供用户编辑”，而是后续
  ARCH-7c 的独立产品/架构切片。没有把密钥、设备句柄、BLE 身份或自动连接动作引入 persistence boundary。

### ARCH-7b 架构与独立复核

- 架构师 Luna max `019fe95e-3b96…`：最小 port/codec/adapter/组合根注入边界 GO；用户编辑器、合并
  语义与 save action 明确留到后续切片。
- 独立复核 Luna max `019fe964-eda5…`：修正严格 schema/DTO integer 和 QSettings group lifecycle 后 GO，
  Critical=0、Required=0、Optional=2；建议真实 QSettings 环境、GUI、HIDPI 和自定义编辑器另行验收。

### ARCH-7b 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (113 files <= 1000)
ARCH7B_CATALOG_BOUNDARY                          pass (roundtrip=7, invalid fallback, 980/1180, hscroll=0)
ARCH7B_STRICT_FINAL                              pass (bool/float rejection, schema=2, DTO=6, store load)
```

未创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、HIDPI、真实焦点/读屏、性能或 UART/TCP/UDP/BLE/
RTT/J-Link 硬件。该切片不涉及嵌入式 C/C++、固件或 MCU；embedded-enterprise-workflow 与
embedded-code-review-simplifier 的厂商源适用性为 N/A，不声称硬件、认证或正式发行合规。

## 2026-08-10 ARCH-7c 自定义连接 preset editor、合并与显式保存

- 新增 `presentation/connection_preset_editor.py`：presentation-only 对话框只收集名称/备注，新项内部 key 自动生成；
  不让用户手填设备句柄、端口身份、BLE 设备或密钥，也不提供自动连接动作。
- `controllers/connection_presets.py` 从当前表单快照生成匹配 transport 的 typed DTO；`connection_builder.py` 负责
  bounded combo 组装和保存/删除按钮组合；`connection_preset_surface.py` 接收明确 `QComboBox`，只负责 builtin + custom
  展示刷新。自定义项按 builtin + custom 顺序展示，builtin key 保留不可覆盖，controller 之间不直接互相导入。
- `composition.py` 创建唯一 `QSettingsConnectionPresetCatalogStore` 实例并向 bootstrap/MainWindow 注入；store 只写 custom
  项，读取时与七项 immutable builtin 合并，旧 schema v1 完整 catalog 会被规范化。保存失败返回 `False`：当前会话保留
  候选项但显式提示可能丢失；删除失败保持原 catalog/UI。活动连接和历史回放期间两个动作均禁用。

### ARCH-7c 架构与独立复核

- 架构师 Luna max `019fe9b6-df24…` 与 `019fe9b9-e5b7…` 均在超时窗口内未返回结论，已关闭；主代理按预先审计的
  editor → controller → store → composition 边界继续，未把超时当作 GO，也未授权子代理写入。
- 独立质量复核 Luna max `019fe9bf-749c…`：两次等待与立即返回请求均超时，未返回结论，已关闭；不把未返回当作 GO，父代理已完成本地五轴边界审查并保留唯一写入权。
- 后续架构复核 Luna max `019fe9c4-1186…`：`ARCHITECTURE_GO`；建议已整合：去掉 combo 重复初始化，并让
  `connection_preset_surface.py` 直接接收 `QComboBox`，消除 controller-to-controller 依赖。

### ARCH-7c 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (117 files <= 1000)
ARCH7C_UI_FINAL                                  pass (themes=3, sizes=2, custom/store/bad-persistence=pass)
ARCH7C_SCREENSHOT                                pass (build/ui_review_arch7c_custom_presets.png)
package                                          pass
```

ARCH7C_PACKAGE_FINAL                             pass
  artifact: `dist/release/0.1.0/core/onefile/app/SerialForge.exe`
  root: `SerialForge.exe` (byte-identical)
  size: 47,785,521 bytes
  SHA256: `3ABE60DB76C0796A465167BD7EE38E0FADAE4594CB4D2DDC846EAD00AB644B80`
  provenance/archive/hash: pass; signature `NotSigned`; release eligible `false`; hardware acceptance `not_run`

截图已人工查看：连接控制带仍为主题化深色 surface，保存/删除动作在 980/1180 宽度下可见，无白色横带或系统 palette
回退。offscreen 环境缺少 PySide6 fonts 目录，中文显示方框不代表 Windows 运行时字体缺失；未创建、修改或运行 unit test、
mock、fixture、harness，未连接真实设备或启动 J-Link/RTT/OTA。

## 2026-08-10 UI-1.33 连接控制带四节点状态路径

- 新增 `presentation/connection_status_surface.py:ConnectionStatusRail`，在连接控制带独立底行绘制端点→传输→会话→数据
  四节点路径。它只消费 lifecycle 显式传入的既有 `SessionState`，不读取 ViewModel/domain、不复制 RX 计数、不暗示真实数据
  进度，也不改变连接按钮、信号、无障碍文案或 Tab 顺序。
- `connection_builder.py` 只负责创建和布局该 renderer；`lifecycle.py` 将其加入现有 `_motion_surfaces()`，并在
  `update_state_badge()` 显式调用 `set_state()`。renderer 只提供 `set_state()/set_frame()/stop()`，不创建 QTimer、不持有
  MotionController，取色全部来自 `ThemeSpec`。

### UI-1.33 架构与独立复核

- 架构师 Luna max `019fe975-c439…`：条件 GO；要求独立底行、四文件以内的最小接线、逻辑坐标和共享生命周期，禁止扩大到
  MainWindow/ViewModel/domain/QSS。
- 独立复核 Luna max `019fe97a-7a16…`：代码静态 GO，首次验证门为 NO-GO；补齐本轮 HIDPI、六状态、三主题和 lifecycle
  证据后，Required=0、Critical=0，代码无需追加修改。

### UI-1.33 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (114 files <= 1000)
UI1.33_CONNECTION_RAIL                          pass (states=6, themes=3, 980/1180, hscroll=0, motion-stop=pass)
UI1.33_HIDPI_FINAL                               pass (dpr=1.50, states=6, themes=3, 980/1180, hide/static stop=pass)
UI1.33_SCREENSHOT                                pass (build/ui_review_ui133_connection_980/1180.png)
```

截图已人工查看：轨道位于连接带底行，没有覆盖首行控件或制造白色系统回退。offscreen 环境缺少 PySide6 fonts 目录，中文可能
显示方框，不代表 Windows 运行时字体缺失。未创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、
真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/RTT/J-Link 硬件。

## 2026-08-10 UI-1.34 终端空态观测网格与扫描线

- `presentation/terminal_surface.py:TerminalEmptyState` 增加主题化的观测画布背景：32px 逻辑网格、四角取景括线、
  中心扫描线和五个节点。它只绘制装饰层，不改变既有空态标题、提示、glyph、复制边界或无障碍文案。
- 所有颜色来自 `ThemeSpec` 语义 token；绘制限制在 `contentsRect()`，使用逻辑坐标并由 Qt 处理 HIDPI，避免白色 QWidget
  回退、越界绘制和重复 QSS。动态相位继续由现有共享 `MotionController` 的 `set_frame()/stop()` 驱动；隐藏、最小化、关闭、
  reduced-motion 和显式 stop 均保留静态回退。
- 没有新增 QTimer、线程、业务字段、ViewModel/domain/infrastructure 依赖或生命周期接线；文件保持 1000 行以下。

### UI-1.34 架构与独立复核

- 架构师 Luna max `019fe980-924d…`：条件 GO；限定为单文件 presentation-only 绘制，要求保留现有子控件语义和共享动效
  生命周期，不得把状态源或定时器下沉到空态组件。
- 独立代码审查 Luna max `019fe985-2b4e…`：GO，Critical=0、Required=0、Optional=1；确认父控件绘制顺序、clip、逻辑
  坐标、主题取色、静态回退和文件行数均符合边界。可选项是未来抽取共享状态色 helper，不影响本轮交付。

### UI-1.34 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (114 files <= 1000)
UI1.34_TERMINAL_AMBIENT_FINAL                  pass (dpr=1.50, contexts=5, themes=3, 980/1180, hscroll=0)
UI1.34_SCREENSHOT                               pass (build/ui_review_ui134_terminal_hidpi.png)
```

验证覆盖 idle/waiting/transition/paused/history 五类空态、标题/提示和 AccessibleDescription、glyph、三主题、980/1180px
宿主、1.5x HIDPI、隐藏停止、显式静态 stop、无 QTimer 子控件和横向滚动。offscreen 环境缺少 PySide6 fonts 目录，中文方框
不代表 Windows 运行时字体缺失。未创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、真实焦点/读屏、
动效性能或 UART/TCP/UDP/BLE/RTT/J-Link 硬件。

## 2026-08-10 UI-1.35 主题切换一次性淡入过渡

- 新增 `presentation/theme_transition.py`，只对 bootstrap 持有的 `appRoot` 做一次性 180ms opacity fade；`apply_theme()` 仍是
  主题事实入口，过渡不复制 ThemeSpec、不读取 ViewModel/domain、不改变焦点、Tab 顺序或无障碍树。
- `bootstrap.py` 只保存 root 与 transition owner；`lifecycle.py` 在主题选择变更后启动，并在 reduced-motion、暂停、隐藏、
  最小化和关闭时停止清理。连续装饰帧仍由唯一共享 `MotionController` 提供，UI-1.35 没有新增常驻 QTimer。
- helper 不覆盖已有 graphics effect；`shiboken6.isValid()` 保护 Qt wrapper 已被回收的边界，快速连续切换、自然完成和显式停止
  都回到完整不透明的静态主题。

### UI-1.35 架构与质量复核

- 架构 follow-up Luna max `019fe999-79f3…`：GO，Required=0；确认 `effect.parent()` 在 `isValid(effect)` 后访问，且
  PySide6 已通过锁文件提供 `shiboken6`，不增加依赖风险。假设生命周期调用保持 Qt GUI 线程，这与当前调用模型一致。
- 独立审查 worker 在本轮运行时未能在等待窗口返回；父代理完成 correctness/readability/architecture/security/performance
  五轴复核，未发现 Critical/Required 项。该事实不被写成独立 worker GO。

### UI-1.35 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (115 files <= 1000)
UI1.35_THEME_TRANSITION_FINAL                   pass (dpr=1.50, themes=3, active cycle, reduced/pause/hide stop, effect-clean)
UI1.35_THEME_TRANSITION_SETTLE                  pass (rapid-switch, natural-finish, effect-clean, 1180x780)
UI1.35_SCREENSHOT                                pass (build/ui_review_ui135_theme_transition.png)
```

验证覆盖三主题、980/1180 宿主、1.5x HIDPI、快速连续主题切换、自然完成、低动效、暂停、隐藏、关闭和 effect 清理；未创建、
修改或运行 unit test、mock、fixture、harness。offscreen 环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 运行时字体缺失；
未启动持续 GUI/EXE、真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/RTT/J-Link 硬件。

## 2026-08-10 UI-1.36 真实数据终端观测标尺

- `presentation/terminal_surface.py:TerminalViewport` 保留现有 `QPlainTextEdit` 的 objectName、只读、真实文本、选择、滚动条、键盘和
  无障碍契约；`paintEvent()` 先让 Qt 完成文本绘制，再在 viewport 已有上下 padding 内画上下标尺、四角 bracket 和共享 frame 信号。
- 标尺是 presentation-only 低对比度装饰，不读取/复制 RX 字节、时间、吞吐、连接进度或业务状态；不新增控件、QTimer、控制器、主题 token
  或 lifecycle 分支。bootstrap 仅注入同契约子类，lifecycle 将它加入现有 `_motion_surfaces()`，隐藏、暂停、关闭和低动效仍静态停止。
- 首次架构师调用 `019fe99f-0484…` 在当前运行时等待窗口内未返回，父代理按已确定的边界继续完成；针对实际 74px viewport 的高度 guard 微修复由
  架构师 Luna max `019fe9a3-4210…` 静态 GO，确认保留 `width < 120` 退化保护且 `height < 28` 可容纳现有几何。

### UI-1.36 架构与质量复核

- 独立审查 worker Luna max `019fe9a5-3acf…` 在等待窗口内未返回，随后关闭；不将其写成独立 GO。父代理已按
  correctness/readability/architecture/security/performance 五轴审阅，当前未发现 Critical/Required 项。

### UI-1.36 验证与限制

```text
targeted py_compile / ruff                       pass
UI1.36_TERMINAL_VIEWPORT_FINAL                  pass (dpr=1.50, themes=3, 980/1180, data-preserved, padding-ruler, empty-layer, hide-stop)
UI1.36_SCREENSHOT                                pass (build/ui_review_ui136_terminal_data_hidpi.png)
```

验证覆盖真实示例文本保持不变、空态/数据态、三主题、980×680/1180×780、1.5x HIDPI、实际 74px logical viewport、横向滚动条不可见、
共享 frame 动态/静态标尺和隐藏停止。截图已人工查看；offscreen 环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 运行时字体缺失。
未创建、修改或运行 unit test、mock、fixture、harness；未启动持续 GUI/EXE、真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/RTT/J-Link 硬件。

### UI-1.36 包交付

```text
PACKAGE_FINAL                                   pass
artifact size                                   47,776,768 bytes
canonical/root SHA256                           CF0319DE46BCC1988B163D1D32AF3C89706BD23E672AF2E5FB7170286B3D7D68
archive module verification                      pass (terminal_surface, connection_status_surface, theme_transition, theme)
signature / release_eligible                    NotSigned / false
hardware_acceptance / vendor_binary_matches      not_run / 0
```

canonical `dist/release/0.1.0/core/onefile/app/SerialForge.exe` 已覆盖根目录 `SerialForge.exe`，两者已完成字节级 hash/size 一致性校验。

## 2026-08-10 ARCH-6c composition/workspace owner-module contract

- `MainWindow` 删除 20 个纯 composition/workspace facade：header、workspace Tab、滚动页、connection/protocol/send/error/terminal page
  builder、bounded combo/timeout helper、shortcut/tab-order 安装以及 workspace transition/tab-change 转发。
- `bootstrap.py` 直接调用 shell/page/shortcut owner；`workspace.py` 直接组合现有 connection/protocol/send builder，并用自身函数连接
  Tab change 与 180ms workspace transition；`lifecycle.py` 直接调用 workspace motion/stop/tab-change helper；`connection_builder.py`
  直接调用 `composition.py` 的 `enum_combo()`/`timeout_spin()`。
- 没有引入动态注册、mixin、`MainWindowContext` 或无限制 callback map；业务 ViewModel signal/callback wrapper 未在本切片扩大迁移面，
  便于下一轮按 feature DTO 继续收窄。

### ARCH-6c 架构与质量复核

- 架构师 Luna max 调用 `019fe9aa-9b96…` 在等待窗口内未返回，随后关闭；父代理按已审计的最小 owner-module 边界完成实现。
- 独立审查 worker Luna max `019fe9af-5a33…` 在等待窗口内未返回，随后关闭；不将其写成独立 GO。父代理完成
  correctness/readability/architecture/security/performance 五轴审阅，未发现 Critical/Required 项。

### ARCH-6c 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (115 files <= 1000)
ARCH6C_COMPOSITION_FINAL                         pass (dpr=1.50, tabs=3, scrolls=3, hscroll=0, facades=0, lifecycle-stop=pass)
ARCH6C_SCREENSHOT                                pass (build/ui_review_arch6c_composition.png)
```

截图已人工查看；offscreen 环境缺少 PySide6 fonts 目录，中文方框不代表 Windows 运行时字体缺失。未创建、修改或运行 unit test、
mock、fixture、harness；未启动持续 GUI/EXE、真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/RTT/J-Link 硬件。代码变更已重打 onefile
包并覆盖根目录，UI-1.36 旧包不作为 ARCH-6c 交付包。

### ARCH-6c 包交付

```text
PACKAGE_ARCH6C                                pass
artifact size                                  47,775,103 bytes
canonical/root SHA256                          7E15E2B7A0887D0A91F7D968942221BB7CADCC0050A1292978F69B4C986111EF
archive module verification                    pass (main_window, bootstrap, workspace, lifecycle, connection_builder, terminal_surface)
signature / release_eligible                   NotSigned / false
hardware_acceptance / vendor_binary_matches     not_run / 0
```

canonical `dist/release/0.1.0/core/onefile/app/SerialForge.exe` 已覆盖根目录 `SerialForge.exe`，两者已完成字节级 hash/size 一致性校验。

## 2026-08-10 UI-1.20 主题语义化结果

- `ThemeSpec` 增加 success/info/warning/error/history/neutral/disabled、hover/pressed/focus、selection
  和 on-accent 语义 token；三套主题各自提供完整值，默认 Star Trail 保持稳定 QSS 基线。
- 新增纯 presentation 渲染边界 `theme_stylesheet_runtime.py`；`theme_variant_shell.py` 与
  `theme_variant_controls.py` 分别负责 shell/状态和原生控件/popup/viewport/Tab overflow/scrollbar
  override，避免把主题策略塞入 MainWindow 或 controller，也避免按十六进制颜色做盲目全局替换。
- 三套主题的 ComboBox popup、settings viewport、scroll corner、Tab overflow、禁用态、表格、终端、
  状态胶囊、链路控制带和白色背景静态审计通过；自绘控件仍通过主题上下文取得同一 ThemeSpec。
- 架构师 `019fe840-5bea-7001-9c7d-55c82ad1071c` 评审确认纯 ThemeSpec→QSS 边界、显式语义 token
  和 QSS 优先级/自绘/对话框继承回归点；父代理完成最终集成。

## 2026-08-10 UI-1.21 TabBar 白线根因与复核

- 实际 `QT_QPA_PLATFORM=offscreen` 画面复核定位到：`QTabWidget#workspaceTabs` 的 `QTabBar` 默认
  `drawBase=True` 在 Tab 区顶部绘制 `#bcbcbc/#ffffff` 原生基线；页面 pane 和主题 QSS 本身没有白色背景。
- `presentation/controllers/workspace.py` 在组装当前工作区 Tab 后调用 `tabs.tabBar().setDrawBase(False)`。
  这是 workspace owner 的局部 Qt presentation 策略，不修改全局 palette、主题 token 或其他 Tab 容器。
- 三主题（`star_trail`、`moonlit_ocean`、`sakura_night`）在 1180×780 offscreen 画面中 `drawBase=False`，
  y=90/91 目标行 near-white 像素均为 0；760×780 窄窗口 y=90/91/92 near-white 像素也均为 0。
- 临时截图 `build/ui_review_current.png` 已人工查看：工作区 Tab 区与下方链路控制带连续，无原先白色横线。
  offscreen 环境缺少 PySide6 fonts 目录导致中文显示为方框，这只影响该临时 Linux/offscreen 渲染，不代表 Windows
  运行时字体缺失；正式 GUI/EXE 启动仍未宣称验收。

### UI-1.21 架构复核记录

- 架构师 `019fe840-5bea-7001-9c7d-55c82ad1071c`（Luna max/Fast）确认：局部 `tabBar().setDrawBase(False)`
  属于 workspace 组装职责，QSS 无需全局兜底；建议覆盖三主题、窄宽度、Tab overflow/scroll buttons。
- 父代理完成静态门、三主题像素审计和视觉截图复核；没有新增业务状态、timer、依赖或跨层引用。

## 2026-08-10 UI-1.22～UI-1.24 响应式与执行状态结果

- 协议/遥测页首行由单一 QHBoxLayout 改为两行 QGridLayout；preset/framing/checksum 使用 bounded
  `AdjustToMinimumContentsLengthWithIcon`，Profile/Dataset/状态标签允许换行、向兄弟控件让出空间并保留完整 tooltip/AccessibleDescription。
- 980px 宿主下协议页 root hint=818px、page=938px、horizontal scrollbar=False；最长协议选项和 180 字符动态配置文本仍无横向滚动，Profile/Dataset 保持 140/160px 可见下限。
- 批量命令状态新增 `empty/ready/running/completed/stopped/failed` presentation state；状态文字继续来自现有 snapshot，切换 batch 会清除旧 snapshot 视觉状态。三主题状态像素与表格行数复核通过。
- 批量结果表和 Component 表隐藏无业务意义的 Qt vertical header；批量表纵向 header `visible=False`、viewport 扩展到 x=1，左侧灰白回退像素为 0；业务“步骤/序号”列保留。
- UART、TCP Client、TCP Server、UDP、BLE GATT、RTT 六类传输在 980×680 offscreen 下均 `horizontalScrollBar=False`。UART 端口、BLE device/characteristic/write mode、RTT channel、TCP peer 均受 bounded combo 约束；TCP/RTT 长提示可换行且实际宽度大于 0。

### UI-1.22～UI-1.24 架构复核记录

- 架构师 `019fe840-5bea-7001-9c7d-55c82ad1071c`（Luna max/Fast）逐项批准协议网格、动态标签下限、批量状态投影、默认主题基线 selector、表格纵向 header 隐藏和连接 bounded-combo/helper 边界。
- `scripts/check_theme_tokens.py` 的 REQUIRED_SELECTORS 新增精确 `QLabel#commandBatchStatus[state="failed"]`，三主题静态门现覆盖 11 个关键 selector；本轮未新增业务测试资产。
- 回放 `empty/playing/paused/eof/stopped/error` 在三主题下均完成 state、按钮 gate、history source 投影复核；未新增回放 timer 或 progress 业务字段。

## 2026-08-10 UI-1.25 连接快速配置结果

- `presentation/connection_presets.py` 新增 schema v1 的 immutable `ConnectionPresetCatalog` 与 typed values，
  提供 UART 115200 8N1、UART 9600 8E1、TCP Client、TCP Server、UDP、RTT Terminal、BLE GATT 扫描七个内置
  入口；catalog 具备 `1..16` 数量、唯一 key、transport/settings 类型匹配和 DTO 范围校验，不携带密钥、
  设备句柄或自动连接动作。
- 连接带改为两行响应式网格，新增主题化 bounded `connectionPresetCombo`；选择 preset 只填入现有控件，
  明确显示“已填入快速配置 · 确认端点后点击连接”，并加入稳定键盘 Tab 顺序；选择器宽度被限制为
  180–290px。
- builder 通过 `ConnectionPresetApplyCallback` 进入 MainWindow typed wrapper；应用 controller 阻断 transport
  signal 后只显式刷新一次。`connection_runtime.py` 继续消费规范化控件值构造 domain config，不直接依赖
  presentation DTO；TCP Server 在传输切换、监听 host 或监听 port 变化时清除旧 LAN 确认和 allowlist。
- 三套主题、七个 preset、980px offscreen 连接页均通过：无横向滚动、transport/data/提示同步，快速配置不自动连接。

### UI-1.25 架构复核记录

- 已按用户要求调用只读架构师角色评审 preset DTO、bounded combo、依赖方向和持久化边界。初次复核指出并已修复
  catalog 校验、重复刷新、typed callback、TCP Server 快速配置清权和传输/监听端点变化清权问题；修复后已由
  本地静态与离屏证据重新覆盖。架构师 follow-up 在当前运行时未返回，因此不把它记录为最终批准。runtime 不直接
  引用 presentation DTO 是刻意的依赖方向约束；QSettings 用户自定义 catalog 继续作为 ARCH-7b 后续独立
  port/adapter。

## 2026-08-10 UI-1.26 终端空态与响应式工具栏结果

- 新增 `src/serialforge/presentation/terminal_surface.py`，提供无资源、presentation-only 的 `TerminalEmptyState`
  与 `TerminalOrbitGlyph`；终端空态只投影已有 preview buffer、`SessionState`、历史来源和 paused 事实，RX preview
  一旦存在就隐藏，不持有 transport/application 状态，不新增业务字段或常驻 timer。
- `bootstrap.py` 将终端包装为 `terminalSurface`，真实 `QPlainTextEdit` 与空态层共用同一主题化 canvas；空态提供
  idle/waiting/transition/paused/history 五种上下文，保留可访问标题、提示和静态动效回退。
- `terminal.py` 的实时观测带与发送带由单行 `QHBoxLayout` 调整为两行可收缩 `QGridLayout`：保留现有 widget 名称、
  signal、发送 gate、快捷键/Tab 逻辑顺序和共享 `MotionController`，动态暂停/记录标签使用 `Ignored + min/max`
  宽度边界，长文案不会撑宽布局。
- `terminal_runtime.py` 在 preview/data/history/状态变化时统一刷新空态；暂停 checkbox 使用无参 signal adapter
  复用既有 `_update_data_activity()`，空终端可立即在 waiting/paused 间切换；真实 RX 隐藏空态，清空后恢复上下文。
- `lifecycle.py` 在 close/hide/minimize 时停止终端空态 glyph，恢复最小化时显式解除 `MotionController` suspended；
  不改变接收、记录、发送、回放和 transport 语义。三套主题新增 terminal surface/card/state/label selector，未引入白色回退。

### UI-1.26 角色评审与独立复核

本轮六角色均为只读、父代理为唯一写入者：Product `019fe8b9-fb7d-7062-b8b9-c3815eac1ce7`、Architecture
`019fe8b9-fbc5-7bd3-aee1-00d17a4180bd`、UI `019fe8b9-fc0e-7ee3-b5a5-94baf10482a6`、Development
`019fe8b9-fc4c-7a21-8b0d-82e497b4485a`、Verification `019fe8b9-fc87-7b20-8816-a1a59e77fc99`、Packaging
`019fe8b9-fcc9-7df0-98e5-2433483581ed`。共同结论是保持 presentation-only、无新业务字段/计时器/资源/传输耦合，
并保留对象名、signal、键盘顺序和共享 MotionController。

- 首轮独立质量复核 `019fe8c7-7f3a-7f73-95b4-95c8a8b59ead`：Critical=0；提出生命周期、暂停立即投影、动态标签宽度 3 项 Required。
- 修复前架构师 `019fe8ce-18a7-7122-ac74-32d825d8efd2`：确认只改 lifecycle/terminal/terminal_runtime，不动 VM/domain；
  推荐 stop/suspend 对称、复用 `_update_data_activity()` 和既有 `QSizePolicy` 边界。
- 修复后独立复核 `019fe8d3-21e6-7143-8dde-a6b394d6e793`：GO，Critical=0、Required=0；确认 Qt signal 参数适配、生命周期、
  主题隔离与行为保持无阻断问题。Optional 的基础 QSS token 归一化留作后续主题专轮。

### UI-1.26 验证与限制

- `pwsh.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\check.ps1`：pass；`106 files <= 1000`；三主题、22 semantic
  tokens、11 selectors，`legacy_qss_literals=309`。
- `QT_QPA_PLATFORM=offscreen`：980×680/1180×780 工具栏 geometry 与可读输入宽度通过；切换三个工作区 Tab 后可见
  `QScrollArea` 均无 horizontal scrollbar；idle/waiting/transition/paused/history/RX-hide-clear-show 通过；三主题切换通过。
- Required 修复专项通过：pause immediate waiting→paused→waiting、暂停/记录长动态文案宽度边界、hide/show、minimize/restore
  的 glyph stop 与 shared motion suspended 生命周期通过。截图：`build/ui_review_terminal_bars_980.png`、
  `build/ui_review_terminal_waiting_1180.png`；offscreen 环境缺少 PySide6 font directory，中文可能显示方框，不代表 Windows 字体缺失。
- 本轮未创建或运行 unit test、mock、fixture、harness；未连接真实 UART/TCP/UDP/BLE/RTT/J-Link，未运行 EXE/持续 GUI、
  HIDPI、真实焦点/读屏、性能或硬件验收。

## 2026-08-10 UI-1.27 协议/遥测派生 surface 与响应式工具条结果

- `presentation/controllers/protocol.py` 新增四个静态 `QFrame[role="surface"]`：协议解析配置、组件遥测、Dataset/曲线、
  原始记录回放。surface 只提供主题化层级、边距和可访问名称，不复制状态、不持有 callback/timer、不控制业务可见性，
  没有新增 domain/application/transport 依赖。
- Component、Dataset、Curve、Replay 派生工具条改为显式 `QGridLayout`，通过列 stretch、bounded dynamic label/combo
  和可换行状态文本处理窄窗口；保留 `ProtocolPanelWidgets`、已有 callback、widget reference、Tab 顺序和数据语义。
- 修复离屏几何审计发现的 980px Profile/Dataset 工具条真实控件重叠；980×680 与 1180×780 均无协议页横向滚动，长标签、
  三套主题和 Component/Dataset 预览横向滚动边界均通过。

### UI-1.27 角色评审、简化评估与独立复核

本轮父代理是唯一写入者，六个只读角色先行评审：Product
`019fe8da-c098-74b0-a00f-2bf69203adc0`、Architecture
`019fe8da-c0e0-7ef3-8207-68b5c2663442`、UI
`019fe8da-c11f-7ac2-90a8-3d401116e418`、Development
`019fe8da-c15c-7531-b4e0-c5d382d9cca2`、Verification
`019fe8da-c19e-7e72-8907-cfa00573cb98`、Packaging
`019fe8da-c1d8-7203-ab61-66563ecec26f`。架构结论是静态 surface 与 Grid 只属于 presentation，不能把响应式判断
泄漏到 domain/application；需保持 callback/ref/Tab 兼容，不能新增业务状态、timer、依赖或传输耦合。

最终独立质量复核 `019fe8e2-3d00-7671-959e-05ea1e37236f`（Luna max/Fast）结论：Critical=0、Required=0、GO；
确认四个 surface、Grid 工具条、既有 callback/ref/Tab 顺序无阻断问题。简化评估认为当前边界已足够小，重复 Grid
配置 helper 仅为后续可选可读性优化，不影响本轮交付；未做可能扩大 diff 的抽取。

### UI-1.27 验证与限制

```text
UI1.27_LAYOUT_980 / UI1.27_LAYOUT_1180        pass
UI1.27_LONG_LABELS                            pass
UI1.27_THEME_SURFACE (3 themes)               pass
protocol surface screenshot review             pass
scripts/check.ps1                              pass
source limit                                   pass (106 files <= 1000)
ruff / compileall                              pass
```

截图：`build/ui_review_protocol_surface_980.png`、`build/ui_review_protocol_surface_1180.png`。offscreen 环境缺少
PySide6 fonts 目录，中文可能显示方框；这不代表 Windows 运行时字体缺失。未创建或运行 unit test、mock、fixture、
harness；未启动 EXE/持续 GUI、HIDPI、真实焦点/读屏、性能或硬件，未连接真实 UART/TCP/UDP/BLE/RTT/J-Link。
本轮不涉及嵌入式 C/C++、固件或 MCU，因此 embedded-enterprise-workflow 的厂商源适用性为 N/A；记录的是静态、
离屏和打包等授权的非破坏性验证，不声称硬件/认证合规。

## 2026-08-10 UI-1.28 协议流水线主题化轨道与共享动效结果

- 新增 `src/serialforge/presentation/pipeline_surface.py` 的 `PipelineSurfaceLabel`，保持原有
  `QLabel#pipelineSummary` 的文本、AccessibleDescription、`objectName`、`role/source/state` 动态属性和 QSS 命中。
- 在标签安全底部 padding 绘制输入 → Protocol → Component → Dataset/Curve 的四节点主题化轨道；轨道只消费
  `theme_spec_for_widget()` 和 shell 唯一 `MotionController` 的 `phase/animated`，不读取/复制业务 `state/source`，不创建
  QTimer、不持有 controller、不接入 domain/application/transport、不引入资源。
- `lifecycle.py` 的已有 reduced-motion、motion pause、close、hide、minimize 分支均显式停止轨道，保留静态文字；
  RX、协议、组件、Dataset、回放和连接业务语义未改变。

### UI-1.28 架构与独立复核

架构师 `019fe8ea-e1e9-7992-bbe2-1f56157e1cf5`（Luna max/Fast）先行只读评审，Critical=0、条件 GO；要求新模块
只依赖 Qt facade/主题上下文，公开 `set_frame()`/`stop()`，不复制状态、不创建时钟，保持 QLabel 契约和五个生命周期停止分支。
独立质量复核 `019fe8f2-9072-7031-968e-83d8f5cd0607`（Luna max/Fast）最终结论 GO，Critical=0、Required=0；
五轴 Correctness/Readability/Architecture/Security/Performance 均 GO。Optional 是未来可让轨道基于 `contentsRect()`
计算几何，以防后续 QSS padding 变化；当前不构成阻断。

### UI-1.28 验证与限制

```text
scripts/check.ps1                              pass
source limit                                   pass (107 files <= 1000)
theme token audit                              pass (3 themes, 22 tokens, 11 selectors)
ruff / compileall                              pass
UI1.28_PIPELINE_RESPONSIVE (980/1180)          pass
UI1.28_PIPELINE_THEMES (3 themes)              pass
UI1.28_PIPELINE_MOTION (reduced/pause/hide)    pass
UI1.28_PIPELINE_TEXT                          pass
UI1.28_PIPELINE_BOUNDARY (no state/source/timer) pass
```

截图：`build/ui_review_pipeline_980.png`、`build/ui_review_pipeline_1180.png`，已人工查看；轨道位于安全底部装饰带，
未遮挡换行文字。offscreen 环境缺少 PySide6 fonts 目录，中文可能显示方框，不代表 Windows 字体验收失败。修正过一次
验证脚本的过宽断言：隐藏、尚未布局的 offscreen `QScrollArea` 会暂时宽度为 100 并报告横向最大值；最终只对可见工作区
滚动区检查，协议页可见 scroll area 在 980/1180 均为 0。未创建/运行 unit test、mock、fixture、harness，未运行持续 GUI、
HIDPI、真实焦点/读屏、性能、EXE 启动或硬件。

## 2026-08-10 UI-1.29 顶栏连接状态指示器几何与共享动效结果

- `presentation/widgets.py:StatusIndicator` 保留原有 `_state`、`set_state()`、`set_frame()`、`stop()`、22×22 固定尺寸、
  透明鼠标和相邻状态文字可访问契约；只增强 `paintEvent()` 的主题化几何。
- `opening/closing` 使用共享 frame 驱动旋转弧，`open` 使用边界安全的呼吸环，`discovered` 显示四向定位角标，
  `error` 显示叉标，`closed` 保留静态环；所有状态在 `_animated=False` 时仍有可辨识静态回退。
- 使用 `QRectF` inset 限制绘制不越出 22×22，Qt `drawArc` 的弧度显式换算为命名的 1/16 度常量；没有新增 timer、
  业务字段、生命周期判断、后端依赖或外部资源。

### UI-1.29 架构与独立复核

架构师 `019fe8fd-f9ff-7f30-bd29-ce2e3f826a8b`（Luna max/Fast）条件 GO，Critical=0；要求只修改绘制逻辑、保持
状态/帧/停止契约、动画半径边界和静态回退。独立质量复核 `019fe903-bed0-7801-8761-76649234113c`（Luna max/Fast）
最终 GO，Critical=0、Required=0；正确性、简洁性、架构/契约、安全、性能五轴无阻塞项。

### UI-1.29 验证与限制

```text
scripts/check.ps1                              pass
source limit                                   pass (107 files <= 1000)
theme token audit                              pass (3 themes, 22 tokens, 11 selectors)
ruff / compileall                              pass
UI1.29_STATUS_FINAL (6 states × 3 themes)     pass
UI1.29_STATUS_GEOMETRY (22x22 / arc units)    pass
UI1.29_STATUS_MOTION (reduced/pause/hide)      pass
```

截图：`build/ui_review_status_indicator_1180.png`，已人工查看。offscreen 环境仍缺少 PySide6 fonts 目录，中文可能显示
方框；未运行持续 GUI、HIDPI、真实读屏、性能、EXE 启动、硬件或真实传输。

## 2026-08-10 UI-1.30 批量步骤轨道与回放活动轨迹结果

- 新增 `presentation/command_batch_surface.py:CommandBatchSurfaceLabel` 和
  `presentation/replay_activity_surface.py:ReplayActivityLabel`。前者只接收
  `CommandBatchSurfaceProjection`，以最多 32 个 slot 绘制待执行、当前、已提交、失败和停止后的步骤层级；后者只接收
  `ReplayActivityProjection`，以既有 `records_emitted` 形成活动轨迹，不把缺少总记录数的 replay 伪装成百分比进度。
- `controllers/commands.py`、`controllers/replay.py` 只负责从已有 `CommandBatchSnapshot`/`ReplaySnapshot` 投影 DTO，
  不新增领域字段、不复制 ViewModel 状态、不接触 transport；`terminal.py`/`protocol.py` 只负责组装既有状态 label 契约。
- 两个自绘标签统一通过 `theme_spec_for_widget()` 取色，保留 `QLabel#commandBatchStatus`、`QLabel#replayStatus` 的
  objectName、动态 QSS state/source、文字、tooltip、AccessibleDescription 和表格单元格可读文本；逻辑坐标使用 `QRectF`，
  不手动乘 devicePixelRatio，不写死白色背景。
- `lifecycle.py` 新增统一 decorative motion fan-out/stop 列表；批量/回放活动只调用既有 MotionController 的短 activity
  窗口，组件不创建 QTimer；低动效、暂停、隐藏、最小化和关闭均静态回退。

### UI-1.30 架构与独立复核

- 架构师 `019fe90d-3f39-7420-a233-21b6e6add2fe`（Luna max）只读评审结论：条件 GO，Critical=0；要求 projection-only、
  不伪造 replay 总进度、两独立表现文件、共享 MotionController 生命周期和三主题/高 DPI/无障碍验证，均已落实。
- 独立代码审查 `019fe916-c573-7002-89f3-c92f2604c2e7`（Luna max）结论：GO，Critical=0；确认新组件无业务状态、无新
  timer、无 ViewModel/Transport 依赖，六态/主题/生命周期/无障碍边界无阻断项。Optional：未来可用 `contentsRect()` 适配
  QSS padding，或为 EOF/STOPPED 增加不同装饰色；不影响本轮交付。

### UI-1.30 验证与限制

```text
scripts/check.ps1                              pass
source line limit                              pass (109 files <= 1000)
theme token audit                              pass (3 themes, 22 tokens, 11 selectors)
ruff / compileall                              pass
UI1.30_SURFACES (3 themes × batch/replay states) pass
UI1.30_COMPOSITION (980/1180, 3 tabs, hscroll=0) pass
UI1.30_SCREENSHOTS (commands/protocol)         pass
```

截图：`build/ui_review_ui130_commands.png`、`build/ui_review_ui130_protocol.png`，已人工查看；未见白色条带、重叠或轨道
覆盖状态文本。offscreen 环境缺少 PySide6 fonts 目录，中文可能显示方框；未运行持续 GUI、HIDPI、真实读屏、性能、EXE
启动或真实 UART/TCP/UDP/BLE/RTT/J-Link。`main_window.py` 已在 ARCH-6a 从 1000 行降至 951 行；后续 ARCH-6 继续按
feature contract 拆分，避免再次堆积。

## 2026-08-10 UI-1.31 四类分析状态信号轨道与共享动效

- 新增 `src/serialforge/presentation/analysis_status_surface.py:AnalysisStatusLabel`，复用既有
  `protocolStatus`、`componentStatus`、`datasetStatus`、`datasetCurveStatus` 四个 QLabel 的文字、tooltip、
  AccessibleDescription、objectName 和 `source/state` dynamic property；只在安全底部绘制五节点主题化信号轨道。
- `AnalysisStatusLabel` 不保存 domain/application snapshot，不复制业务状态，不创建 QTimer，不修改 dynamic property；
  仅暴露 `set_frame()`/`stop()`，由 `lifecycle.py` 的共享 MotionController fan-out 统一驱动和停止。replay 专用
  `ReplayActivityLabel` 保持原有 `records_emitted` projection，不与通用 renderer 耦合。
- `protocol.py` 只替换四个 label 构造器，`status_surfaces.py` 和派生业务 controller 未改；未知 state/source 安全静态回退，
  三主题继续从 `theme_spec_for_widget()` 取色，逻辑坐标由 QPainter 处理 HIDPI。

### UI-1.31 架构与独立复核

- 架构师 Luna max `019fe932-ccae-70a1-899f-c4fd75ae02af`：条件式 GO；建议读取已有 `state/source` property，
  不引入 projection DTO、timer、业务状态或通用化 replay；要求安全底部绘制、未知状态回退、QSS 子类命中和 lifecycle fan-out。
- 独立复核 Luna max `019fe93a-e252-70e0-9684-a310ac76fb4b`：修复后 GO，Critical=0、Required=0；确认 12px
  底部内容安全带、`contentsRect()`、200px 最小宽度、QSS/objectName、状态回退和共享生命周期无阻断项；Optional 是
  未来可把 lifecycle 的装饰消费者 fan-out 收窄为更强类型契约。父代理为唯一写入者。

### UI-1.31 验证与限制

```text
targeted py_compile / ruff                       pass
UI1.31_ANALYSIS_STATUS                          pass (4 labels × 9 states × 3 sources)
UI1.31_SAFE_BAND                                pass (margins>=12, long text, themes=3, motion=pass)
UI1.31_COMPOSITION                              pass (980/1180, 3 tabs, visible hscroll=0, min_width=200)
UI1.31_SCREENSHOT                               pass (build/ui_review_ui131_analysis_safe_band.png)
scripts/check.ps1                               pass (110 files <= 1000)
```

截图已人工查看：四类状态轨道位于状态标签底部装饰带，无白色系统回退或长文本覆盖；offscreen 环境缺少 PySide6 fonts
目录，中文可能显示方框，不代表 Windows 运行时字体缺失。未创建或运行 unit test、mock、fixture、harness；未启动持续 GUI、
真实 EXE、HIDPI、真实焦点/读屏、动效性能或 UART/TCP/UDP/BLE/RTT/J-Link 硬件。该切片不涉及嵌入式 C/C++、固件或 MCU，
embedded-enterprise-workflow 的厂商源适用性为 N/A；记录的是静态、离屏和打包等授权的非破坏性验证，不声称硬件/认证合规。

## 2026-08-10 UI-1.32 主题语义色点阵与共享动效

- 新增 `src/serialforge/presentation/theme_palette_surface.py:ThemePaletteSwatch`，固定 62×22 逻辑像素，在主题
  选择器旁绘制 accent/accent_blue/accent_purple/accent_pink 四个语义色点、轨道和共享帧 pulse；不使用图片、字体或外部资源。
- 组件只依赖 Qt facade、`theme_spec_for_widget()` 和标准库；无业务状态、主题 signal、QTimer、ViewModel/domain/transport
  依赖，设置 NoFocus、透明鼠标和空 accessibility，不进入 Tab 顺序。主题切换复用 `apply_theme()` 根属性和 child update。
- `workspace.py` 只负责创建和定位 widget，`lifecycle.py` 将其纳入既有 `_motion_surfaces()`；低动效、暂停、隐藏、最小化、关闭
  均通过统一 `frame/stop` 处理，未修改 main_window/bootstrap/composition/QSS。

### UI-1.32 架构与独立复核

- 架构师 Luna max `019fe953-f5bd-7140-bbb5-8d58dc985204`：条件式 GO；要求 980px 头部不重叠、固定逻辑尺寸、三主题刷新、
  共享生命周期和装饰性无障碍边界。
- 独立复核 Luna max `019fe958-c7dd-78f3-9741-76cc6c0d5012`：GO，Critical=0、Required=0、Optional=1；建议将
  真实 HIDPI/读屏/键盘焦点留到授权 GUI 验收，不阻断本切片。

### UI-1.32 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (111 files <= 1000)
UI1.32_THEME_SWATCH                            pass (62x22, themes=3, 980/1180, hscroll=0)
UI1.32_MOTION_LIFECYCLE                         pass (reduced/pause/hide/close)
UI1.32_SCREENSHOT                               pass (build/ui_review_ui132_theme_swatch.png)
```

截图已人工查看；色点阵出现在主题选择器右侧，无白色系统回退或 header 控件重叠。offscreen 环境缺少 PySide6 fonts 目录，
中文可能显示方框；未创建/运行测试资产，未启动持续 GUI、EXE、真实 HIDPI、读屏、键盘焦点、性能或 UART/TCP/UDP/BLE/RTT/
J-Link 硬件。该切片不涉及嵌入式 C/C++、固件或 MCU；embedded-enterprise-workflow 厂商源适用性 N/A。

## 2026-08-10 ARCH-6b terminal runtime owner-module contract 迁移

- 删除 `presentation/main_window.py` 中仅由 terminal runtime 自身调用的 `_current_entry` 与 `_render` 两个 facade；
  删除对应的 `current_entry`/`render` MainWindow import。
- `controllers/terminal_runtime.py` 的 `save_current_quick()` 直接调用同模块 `current_entry(window)`，
  `rerender_preview()` 直接调用 `render(window, payload)`；异常处理、预览滚动、终端空态与关闭/隐藏/最小化语义保持不变。
- 未触及其他 terminal signal wrapper、Qt parent、Tab、objectName、timer 或 lifecycle；更大范围 terminal `window._*`
  访问保留为后续 ARCH-6 DTO 收窄候选，避免一次性扩大迁移面。

### ARCH-6b 架构与独立复核

- 架构师 Luna max `019fe948-b8bd-7580-bcca-91c99f55689c`：GO，仅限两个 local-only owner-module helper；protocol/
  derived/workspace 更大范围本轮 NO-GO。
- 独立复核 Luna max `019fe94e-d033-7500-be30-d82e90a84034`：GO，Critical=0、Required=0、Optional=1；确认
  MainWindow 939 行、terminal_runtime 494 行、调用方向和生命周期未破坏。

### ARCH-6b 验证与限制

```text
targeted py_compile / ruff                       pass
scripts/check.ps1                               pass (110 files <= 1000)
ARCH6B_TERMINAL_OWNER                            pass (old_facade=0, direct owner calls)
ARCH6B_COMPOSITION                               pass (980/1180, 3 tabs, hscroll=0)
```

offscreen 环境缺少 PySide6 fonts 目录；未创建/运行测试资产，未启动持续 GUI/EXE、HIDPI、真实焦点/读屏或设备硬件。
该切片不涉及嵌入式 C/C++、固件或 MCU；embedded-enterprise-workflow 厂商源适用性 N/A。

## 2026-08-10 ARCH-6a 连接配置 owner-module contract 迁移

- 从 `presentation/main_window.py` 移除 8 个纯配置/解析 facade：`_build_uart_config`、`_build_tcp_config`、
  `_build_rtt_config`、`_build_udp_config`、`_build_server_config`、`_build_ble_discovery_config`、
  `_build_ble_config`、`_parse_ble_uuid_filter`；当前精确字面扫描在 `src` 中均为 0。
- `controllers/connection_runtime.py` 的 `toggle_connection()` 与 BLE config builder 直接调用 owner module 函数；
  `controllers/ble.py` 直接调用 `build_ble_discovery_config(window)`。不引入动态 `__getattr__`、注册表、monkey-patch
  或 mixin，Qt parent、signal、callback、objectName、Tab 顺序和生命周期入口保持不变。
- `main_window.py` 从 1000 行降至 951 行；这一切片是 ARCH-6 的渐进起点，剩余 controller 仍保留显式 window compatibility
  facade，后续按 workspace/terminal/commands/protocol family 继续建立窄 feature contract。

### ARCH-6a 架构与独立复核

- Terra/max 架构师 `019fe925-12ae-7940-8f41-19183bf025d4`：GO（仅限 ARCH-6a），全量 typed facade/mixin 一次性替换
  NO-GO；推荐 owner-module contract 渐进迁移。
- 独立复核 `019fe92a-b62b-7f13-ad55-1533088b2677`：首次 NO-GO 因误把合法组合方法误计入旧 facade；基于当前 checkout
  精确扫描后更正为 GO，Critical=0、Required=0、Optional=2。

### ARCH-6a 验证与限制

```text
pwsh .\scripts\check.ps1                         pass (109 files <= 1000)
ruff / py_compile                                 pass
ARCH6_CONNECTION_OWNER                            pass (980/1180, 3 tabs, hscroll=0, old_facade=0)
exact old facade scan                             pass (8 names = 0)
```

配置 builder 仍属于 presentation controller，未改变 domain/application/transport 语义；未连接真实设备，offscreen 不
等于 EXE 启动、HIDPI、读屏或正式发行验收。

## 2026-08-10 ARCH-7a 与完整 offscreen 组装结果

- `presentation/preferences.py` 新增 frozen `PresentationPreferences`（schema v1、theme key、低动效、
  暂停动效）与 `PreferenceStore` port；`QSettingsPreferenceStore` 是唯一 Qt 持久化 adapter。坏 schema、
  未知主题、非规范 bool fail-open 到安全默认；lifecycle 通过 `replace()` 更新快照，保存失败只记录日志。
- composition root 创建 adapter，bootstrap 负责 load/hydration，workspace 只消费 DTO 初始化控件，
  lifecycle 不直接接触 QSettings。环境变量 `SERIALFORGE_REDUCED_MOTION` 作为启动强制覆盖，不回写持久化值，
  并在 UI 中禁用对应开关以保持状态可解释。
- 完整 `QT_QPA_PLATFORM=offscreen` 组装已通过：真实 `create_application()` / `create_main_window()`、545 个
  QWidget、主题选择、动效偏好初始化、`connectionControlBand` 与 `settingsViewport` 均可完成初始化；未连接硬件。
- 同一运行时复核发现并修复 BLE 纯函数 facade 的真实 TypeError：`supported_ble_write_modes()` 不再接收
  隐式 window，MainWindow wrapper 收窄为显式 typed characteristic 边界；ARCH-6a 后续又移除了 8 个纯 builder facade。
- 架构师确认 ARCH-7a 边界和 BLE 最小修复；ARCH-7b 已完成最小 store boundary，更广义 ARCH-6 与用户编辑器 ARCH-7c 仍是后续切片。

## 2026-08-10 企业级架构迁移结果

- 新增 `docs/adr/0025-enterprise-modular-architecture.md`，把 bounded context、owner、依赖向内、
  组合根、主题 token、OTA/debug 后置条件、源码行数和交付门禁固化为项目规则；“参考字节跳动”仅
  表示公开工程共性，不声称内部规范或认证。
- `presentation/main_window.py` 收敛为 992 行以内的 Qt shell；新增 `controllers/bootstrap.py` 负责
  状态注入、页面组装、ViewModel signal wiring 和首屏投影；新增 `controllers/composition.py` 负责
  页面包装、Protocol panel 接线、快捷键和焦点顺序。其余连接、协议、派生、终端、命令、BLE、回放、
  生命周期逻辑位于高内聚 controller 文件中。
- 修正所有 `presentation/controllers` 到 `domain` 的相对导入层级；导入烟测覆盖 MainWindow、
  bootstrap、composition、OTA security 和 debug contracts。
- 主题上下文由 `ThemeSpec`、`apply_theme()`、`theme_key_for_widget()` 和 `theme_spec_for_widget()`
  统一；StatusIndicator、SignalFieldWidget、DatasetCurveWidget 和批量命令编辑器会跟随根窗口/parent
  主题，不再分别锁定默认主题或固定背景色。三套主题的 semantic runtime、popup、scroll viewport、
  corner、Tab overflow 和白色背景静态审计均通过。
- 新增 `scripts/check_source_limits.py` 并接入 `scripts/check.ps1`；当前 `src` 共 104 个 Python 文件，
  最大 997 行，全部不超过 1000 行。
- 最新交接已从根目录 `handoff.md` 移至本文件；根目录不再放置 handoff 文件，README/AGENTS/WORKFLOW
  已更新为 `docs/handoffs/current.md` 入口。

### 本轮实际文件范围

- 架构/门禁：`docs/adr/0025-enterprise-modular-architecture.md`、`docs/ARCHITECTURE.md`、
  `docs/CONSTRAINTS.md`、`tasks/plan.md`、`tasks/todo.md`、`scripts/check_source_limits.py`、
  `scripts/check_theme_tokens.py`、`scripts/check.ps1`。
- Presentation：`main_window.py`、`controllers/bootstrap.py`、`controllers/composition.py`、
  controllers 中的 domain 相对导入修正、`theme.py`、`widgets.py`、`dataset_curve.py`、
  `command_batch_editor.py`、`theme_tokens.py`、`theme_stylesheet_runtime.py`、
  `theme_variants.py`、`theme_variant_shell.py`、`theme_variant_controls.py`、`preferences.py`、
  `qt.py`、`controllers/workspace.py`、`controllers/lifecycle.py`、`controllers/ble.py`、
  `controllers/protocol.py`、`controllers/derived_data.py`、`controllers/commands.py`、
  `controllers/terminal.py`、`controllers/connection_builder.py`、`controllers/connection.py`、
  `controllers/connection_presets.py`、`connection_presets.py`、`theme_stylesheet_base.py`。
- 交接：`docs/handoffs/current.md`、`docs/handoffs/README.md`；根目录旧 handoff 已移动，不保留副本。

### 验证与交付证据

已运行：

```text
.\scripts\check.ps1                         pass
uv run ruff check src scripts                   pass
python -m compileall -q src\serialforge scripts  pass
uv run python scripts\check_source_limits.py src pass (109 files <= 1000)
MainWindow/controller/OTA/debug/theme/preferences runtime import smoke pass
star_trail/moonlit_ocean/sakura_night semantic QSS audit pass
QT_QPA_PLATFORM=offscreen theme apply smoke       pass (3 themes, 22 widgets)
QT_QPA_PLATFORM=offscreen full composition smoke   pass (516 widgets)
.\scripts\package.ps1 -Mode onefile           pass
```

canonical artifact：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`  
根目录覆盖：`SerialForge.exe`  
SHA-256：`FD07C6E9425F4524A912BFC000352963ECF28E3E3605F8028A8D3F2FD5C14CCC`  
大小：`47,758,161` bytes；canonical 与根目录字节/hash 一致。该包仍是未签名 engineering build，
provenance `release_eligible=false`；没有启动 EXE 或连接硬件。

架构师复核：`019fe840-5bea-7001-9c7d-55c82ad1071c`（Luna max/Fast）。其确认导入修正不改变依赖方向，
controllers → domain 方向正确；并记录 `EMBEDDED_WORKFLOW_COMPLETE`（本轮无 C/C++/MCU/固件，厂商
约束适用性 N/A）。父代理完成最终集成与静态/打包复核。

后续：ARCH-6/7 继续把显式 `window` facade 收窄为 feature state/action DTO，并建立版本化配置/主题
catalog store；OTA/debug 只有目标 bootloader、公开一手资料、安全审查和授权硬件证据齐备后才进入
application orchestration。完整 GUI、EXE 启动、真实 UART/BLE/J-Link、OTA 安全和正式签名发布仍未运行；
已完成的 offscreen 检查还覆盖了工作区 TabBar 白线像素审计和临时画面查看，但不等同于完整人工 GUI 视觉验收。

## UI-1.7 本轮结果

- `QScrollArea` 的真实 viewport 现在拥有项目自有 `settingsViewport` 标识和深色表面；内容页使用 `scrollContent`，并在父子关系就位后局部重新 polish，消除系统 palette 留下的白色大块风险。
- viewport/content 的 QSS 选择器按真实父子链命中，workspace 的 pane、tab、focus 样式限定在 `#workspaceTabs`，避免未来新增 tab 容器发生主题泄漏。
- 补齐横向滚动条、横向滑块、两端按钮、分页区域和 `QAbstractScrollArea::corner` 的暗色规则，消除系统原生白色横线 fallback。
- 工作区导航升级为“链路 / 连接、协议 / 遥测、命令管理”；主观测区与发送区分别显示“实时观测”“发送控制”，把连接、解析、遥测、回放、命令管理的职责边界显式化。
- 补齐“暂停终端显示”和 UART 独占/DTR/RTS 的 AccessibleName/Description；记录按钮统一为“开始/停止原始记录”，没有改变发送、记录、回放或接收业务语义。
- `StatusIndicator` 继续只消费共享动效帧，鼠标透明且使用显式 `QPointF` 绘制浮点圆；没有新增 timer、线程、资源或领域依赖。
- `theme.py` 将深色调试壳重塑为紫黑、薄荷、淡紫、珊瑚粉的星轨霓虹主题：Header、Tab、分区标题、表单控件、按钮、表格、终端、Tooltip、状态徽章和错误条拥有统一圆角/渐变/焦点层级。
- `widgets.py` 增强 StatusIndicator 的环形光晕与 SignalFieldWidget 的星点、轨道、扫描线和信号点绘制；仍由单一 MotionController 驱动，低动效/暂停/隐藏/关闭均静态降级。
- Header 保留“星轨串行实验室”品牌，副标题升级为“嵌入式调试工具站 · 观测 / 解析 / 回放”；连接状态标签按真实 SessionState 使用发现/连接中/已连接/关闭/错误语义色；终端工具栏新增静态“实时/历史 · 最近 +N B · 窗口 N B”接收摘要。
- ViewModel 暴露有界 `preview_snapshot`，高突发 drain 时 MainWindow 复制同一 latest window；`send_entry()` 直接拒绝关闭中、回放中和不可写会话，避免只依赖按钮 gate。
- 补齐主窗口与批量命令编辑器缺失的 AccessibleName；补充关闭态回调 fence 和展示文本/tooltip/AccessibleText 的字符上限，未改变 wire payload、传输、记录、解析或回放语义。
- 通过 `pwsh` 调用唯一入口 `scripts/package.ps1 -Mode onefile` 生成当前源码包，并将 canonical artifact 字节一致复制到仓库根目录 `SerialForge.exe`。

- 错误横幅清除后恢复 MainWindow 记录的最近非错误状态；不会把 ViewModel 仍保存的错误文案重新放回状态栏。
- 发送入口分离“可发送会话”和“已有有效 payload”：空文本、全空白 Hex 会禁用发送按钮；非法 Hex 保持可提交以显示格式错误；回车路径也拒绝零长度 payload。
- TCP Server 监听时没有选中目标 client，单条发送和批量执行入口均保持禁用，并显示“先选择发送目标 client”；选中 peer 后仍只传递该 peer id，不广播。
- 回放开始/暂停/继续/停止和原始记录按钮补齐稳定 AccessibleName 与随状态变化的 AccessibleDescription；回放状态不因 UI 关闭后的迟到信号改写控件。
- Component、Dataset 预览在无数据、raw-only/无可派生 RX 时使用可见 placeholder 与辅助描述；Dataset 只有用户原本停在底部时才跟随最新内容，否则保留阅读位置。
- 派生来源暂不支持时，表格/预览/曲线禁用并移出焦点链；保留 presentation 快照契约，恢复到 UART/TCP Client 或可派生历史来源时统一恢复协议帧、Component rows、Dataset samples 和曲线，不出现部分恢复。
- MainWindow 的接收、预览、回放、记录、暂停字节、状态、错误、send-ready、server-peer、session-state 和装饰帧入口补充关闭早退；DatasetCurveWidget 关闭后不再接受空态消息；窄宽度曲线最小宽度为 260 px。
- 保持最新有界状态：终端、协议帧、Component、Dataset、Curve 的现有上限和 latest-wins/生命周期边界未扩大；不改变 Raw 接收、记录、解析或回放语义。

## UI-1.8 与嵌入式工具站骨架结果

- “实时观测”和“发送控制”从裸 `QHBoxLayout` 收拢为 `QFrame` station band，使用 `role=stationBand` 与 `source=live/history` 动态属性；来源复用既有 `_history_source_active` / `_history_file_selected`，不新增 ViewModel 状态或 timer。
- `theme.py` 对 station band 使用严格的 `QFrame` 作用域和暗色 fallback；实时为薄荷青、历史为紫蓝，避免未知属性回落到系统 palette。连接页原有 viewport、横向 scrollbar 和 corner 暗色规则保留。
- 新增 `src/serialforge/ota/`：有界镜像描述、传输请求/进度和 port；`transports/xmodem`、`ymodem`、`tftp` 为 contract-only 槽位。
- 新增 `src/serialforge/ota/security/`：仅保存 AES-GCM/CCM 策略与 key reference/验证结果端口，不实现密码学、不接受原始 key。
- 新增 `src/serialforge/debug/`：RTT/J-Link 原始日志 port、`rtt/` 与 `jlink/` 槽位；不启动 vendor 工具、不加载 SDK/DLL、不提供 memory/halt/run/reset/flash。
- 新增 `src/serialforge/presentation/README.md`，明确 PySide6/UI 只能消费 application DTO/capability，不向下穿透到 transport、vendor 或 security。
- 以上能力没有接入 composition root，不改变当前 UART/网络/BLE/RTT 运行路径；真实 OTA/加密/硬件均未实现。

## UI-1.9 状态表面结果

- `connectionContext` 现在是深色上下文胶囊；`sourceBadge` 按实时/历史切换薄荷青与紫蓝色轴。
- `pipelineSummary` 独立成为 Protocol → Component → Dataset → Curve 的低对比度摘要带，不再与普通辅助文字混在一起。
- 暂停显示、原始记录分别拥有 idle/paused/active/stopping/error 状态表面；文字仍是权威状态，颜色只做辅助。
- 动态属性刷新统一由 MainWindow 私有 `_set_dynamic_property()` 完成，只在属性值变化时重新 polish；没有新增 timer、ViewModel 字段或业务分支。

## UI-1.10 工作区切换动效结果

- `workspaceTabs.currentChanged` 现在只在 presentation 层触发 180 ms `QPropertyAnimation`，通过 `QGraphicsOpacityEffect` 对当前页面做轻量淡入，使用 `OutCubic` 缓动。
- 动画对象、opacity effect 和清理方法均由 `MainWindow` 私有持有；业务状态、ViewModel、transport、OTA/debug contract 不参与切换动效。
- 已接入既有“低动效”和“暂停动效”策略；低动效/暂停、隐藏、最小化、关闭或快速换页都会停止动画并恢复页面不透明度，不留下半透明白/灰表面。
- 如果页面未来已有非 opacity graphics effect，动画会安全跳过，不覆盖其他 presentation 组件的效果；Qt 导入仍集中在 `presentation/qt.py`，保持 PyInstaller 入口边界稳定。

## UI-1.11 接收活动动效结果

- `dataActivity` 增加 `state=active/idle` presentation 状态：只有实时 `SessionState.OPEN`、共享 `MotionController` 正在发出 animated frame 且最近接收字节大于 0 时才进入 active。
- active 使用薄荷青状态面，活动窗口结束后恢复 idle；暂停显示不影响接收，因此不会错误地把接收活动与终端可见性绑定。
- 低动效、暂停、会话重置、隐藏、最小化和关闭都会显式恢复 idle；动态属性只在 active/idle 边沿重新 polish，不在每帧重复刷新样式。
- 仍未新增 timer、ViewModel 字段或业务状态；QSS 只提供颜色辅助，AccessibleDescription 继续保留“最近字节/有界窗口/暂停不停止接收”的事实说明。

## UI-1.12 链路控制状态带结果

- 连接页首行从裸 `QHBoxLayout` 收拢为 `QFrame#connectionControlBand`，拥有独立深色 surface、内边距、圆角和左侧状态色带。
- 连接控制带复用既有 `SessionState` 投影，覆盖 discovered/opening/open/closing/closed/error；没有新增 ViewModel 状态或连接分支。
- 连接行不再依赖 scroll viewport 或系统 palette 的裸布局 fallback；UART/TCP/BLE/RTT 等传输控件继续复用既有暗色输入/按钮规则。
- 动态属性沿用 `_set_dynamic_property()`，只在状态改变时局部 polish；颜色是辅助信息，顶部状态文字仍是权威表达。

## UI-1.13 发送成功信号反馈结果

- `_send_current()` 只有在 ViewModel 明确返回发送成功后才调用既有 `MotionController.request_activity()`；格式错误、空 payload、未连接或底层拒绝不会触发成功反馈。
- 顶部 SignalFieldWidget 因此同时表达短暂的 RX/TX 活动，但不读取 payload、不修改发送时序、不创建新的业务状态。
- 低动效、暂停、隐藏、最小化和关闭边界继续由共享动效控制器统一处理；没有增加 timer 或第二套动画时钟。

## UI-1.14 发送控制状态带结果

- `sendControlBand` 增加 `state=ready/busy/history/waiting/blocked` presentation 属性，分别表达可发送、批量/通知忙碌、历史回放、等待目标配置和未建立可写会话。
- 状态严格复用 `_update_connection_controls()` 已有的 `send_available`、`historical_active`、`batch_active`、`ble_notification_pending` 和 TCP Server target gate；不新增业务状态或重复发送判断。
- 状态带样式在 source/live/history 规则之后局部覆盖，保持暗色 palette、薄荷 ready、警示 busy、紫蓝 history/waiting 和深紫 blocked 层级。
- 动态属性使用 `_set_dynamic_property()`，只在状态发生边沿时重新 polish；发送按钮原有 AccessibleDescription 仍提供原因文字。

## UI-1.15 原生控件暗色兜底结果

- `QComboBox` 的下拉箭头、弹出列表项 hover/selected/disabled 状态均限定为主题色，避免系统 palette 在展开下拉框时重新出现白色条目。
- `QTabWidget#workspaceTabs` 的原生 Tab 溢出 `QToolButton` 补齐常态、hover、pressed、focus 和 disabled 样式，窄窗口滚动 Tab 时仍保持紫黑主题。
- 本轮只修改 presentation 层 `theme.py` 的 QSS，不新增业务状态、timer、资源或 transport 依赖；`connectionControlBand` 的链路状态带继续沿用 UI-1.12 的深色状态面。
- 关键选择器审计通过，主题白色背景命中为 0；重新生成 onefile，并将 canonical artifact 覆盖到根目录便利副本。

### UI-1.15 架构复核记录

- 架构师调用：`019fe7f3-569b-7ee2-b7a4-00d7d882ddc1`（Luna max/Fast），只读检查 Tab 溢出按钮与 ComboBox popup 的 QSS 作用域；线程在本轮未返回报告后关闭，未将其当作验证证据。
- 父代理完成最终集成检查：变更限定在 `presentation/theme.py`，无跨层状态搬运，原生控件 fallback 与现有动态属性边界保持高内聚、低耦合。

## UI-1.16 顶栏状态与动效控制胶囊结果

- Header 将已有连接上下文、实时/历史来源、状态灯和连接状态文字收拢到 `QFrame#statusCluster`；将“低动效/暂停动效”收拢到 `QFrame#motionControls`，降低顶栏散落控件造成的信息噪声。
- `statusCluster` 只复用已有 `SessionState`，由 `_update_state_badge()` 通过 `_set_dynamic_property()` 同步 discovered/opening/open/closing/closed/error 主题色；没有复制 ViewModel 状态或改变连接流程。
- 两个动效复选框仍由原有槽函数和同一个 `MotionController` 驱动；胶囊只提供视觉分组和辅助描述，不影响终端、接收、记录、回放或发送。
- 新增 QSS 只覆盖 `presentation/theme.py` 的 header cluster 作用域，白色背景审计仍为 0；未新增图片、字体、资源或依赖。

### UI-1.16 架构与质量复核记录

- 架构师调用：`019fe7fc-39a6-7180-a8f0-25a38bae894c`（Luna max/Fast），检查 statusCluster/motionControls 的容器化、动态属性和边界；线程超时关闭，未返回报告，未作为通过证据。
- 独立质量复核调用：`019fe7fe-b3a6-7b43-8c35-8cd3af32c04f`（Luna max/Fast），检查正确性、可访问性、性能和窄宽度风险；线程超时关闭，未返回报告。
- 父代理集成审查：初始化顺序安全，状态胶囊在初次状态刷新前建立；装饰计时器数量未增加；静态检查与 selector 审计通过。窄窗口实际渲染仍待用户授权运行时验收。

## UI-1.17 实时观测带状态表面结果

- `liveObservationBand` 新增 presentation-only `state` 投影：`history`、`transition`、`paused`、`recording`、`error`、`active`、`idle`。
- 状态优先级集中在 `_refresh_live_observation_state()`：历史来源优先，其次连接流程、终端暂停、原始记录/记录错误、接收活动，最后回到 idle；不新增 ViewModel 状态或业务分支。
- 接收活动沿用既有 `dataActivity[state="active"]` 和共享 `MotionController` 窗口；暂停字节、录制快照和历史来源沿用已有事实入口，QSS 只负责视觉表面。
- active/transition/paused/recording/error 状态均使用深色主题边框和左侧色带，未引入白色 palette fallback、第二个计时器、资源或依赖。

### UI-1.17 架构与质量复核记录

- 架构师调用：`019fe802-004b-7d30-88dd-62106ea7278d`（Luna max/Fast），检查状态优先级和投影边界；线程超时关闭，未返回报告，未作为通过证据。
- 独立质量复核调用：`019fe804-b959-7620-a78f-7f402fc3c680`（Luna max/Fast），检查动态属性、生命周期和性能；线程超时关闭，未返回报告。
- 父代理集成审查：状态集中在一个 helper，关闭态早退保留，动态属性只在边沿重新 polish；实际窗口渲染与帧率仍待授权运行时验收。

## UI-1.18 发送状态胶囊结果

- 发送栏新增 `QLabel#sendState`，以紧凑短文案显示“历史回放 / 处理中 / 可发送 / 输入内容 / 等待目标 / 等待连接”。
- 状态胶囊复用 `_update_connection_controls()` 已经计算好的 `send_band_state` 和 `send_hint`，没有复制 TCP Server peer、BLE 写能力、批量或回放判断。
- 完整原因仍写入 tooltip 与 AccessibleDescription；视觉短文案不会替代原有发送按钮禁用逻辑、输入校验或 `send_entry()` 防线。
- QSS 与 `sendControlBand` 使用相同的 ready/busy/history/waiting/blocked 色轴，保持暗色主题和状态层级一致；宽度有上限，不让提示挤压输入框。

### UI-1.18 架构与质量复核记录

- 架构师调用：`019fe808-66fc-78e3-a0df-8d64e283eb68`（Luna max/Fast），检查状态胶囊与 send_hint 边界；线程超时关闭，未返回报告，未作为通过证据。
- 独立质量复核调用：`019fe80a-722a-7813-ac6b-73ff5687c94e`（Luna max/Fast），检查文案、窄宽度和无障碍；线程超时关闭，未返回报告。
- 父代理集成审查：状态文字与业务判断均留在现有 `_update_connection_controls()`，presentation 只增加一个展示控件；静态审计通过。窄窗口实际排版仍待授权运行时验收。

## UI-1.19 协议流水线摘要状态表面结果

- `QLabel#pipelineSummary` 增加 presentation-only `source=live/history` 与 `state=idle/active/transition/blocked/draft/history` 属性；摘要文字、tooltip 和 AccessibleDescription 继续来自同一条既有状态投影。
- 历史回放/历史文件优先显示紫色历史层级；连接流程显示过渡层级；不支持派生来源显示 raw-only 阻断层级；协议编辑器有未应用草稿时显示草稿层级；已连接且可派生时显示薄荷绿色 active；其余回到深色 idle。
- 状态判断集中在 `_update_pipeline_summary()`，主题只消费属性，不复制协议、组件、数据集或曲线业务逻辑；没有新增 ViewModel 字段、计时器、资源或跨层依赖。
- 所有新表面均明确指定深色背景、边框和左侧色带，避免链路/协议摘要行回落到系统白色 palette；完整摘要仍保留文字与辅助描述，颜色仅为辅助信息。

### UI-1.19 架构与质量复核记录

- 架构师调用：`019fe80d-8676-7951-9880-cf7207324b5f`（Luna max/Fast），检查 pipelineSummary 的状态优先级、presentation 边界与暗色 fallback；线程超时关闭，未返回报告，未作为通过证据。
- 独立质量复核调用：`019fe80f-f2a5-7a73-ab75-d8a5cdc3f4a2`（Luna max/Fast），检查动态属性刷新、历史/实时冲突、可访问性和白色回落风险；线程超时关闭，未返回报告，未作为通过证据。
- 父代理集成审查：属性更新复用 `_set_dynamic_property()`，只在边沿重新 polish；历史来源优先于连接/草稿状态；代码、主题和静态边界检查通过。

## 变更范围

本轮父代理实际写入的源码文件：

- `src/serialforge/presentation/main_window.py`
- `src/serialforge/presentation/dataset_curve.py`
- `src/serialforge/presentation/viewmodels.py`
- `src/serialforge/presentation/theme.py`
- `src/serialforge/presentation/widgets.py`
- `src/serialforge/presentation/command_batch_editor.py`
- `src/serialforge/presentation/README.md`
- `src/serialforge/ota/`（contract-only）
- `src/serialforge/debug/`（contract-only）
- `src/serialforge/presentation/main_window.py`（UI-1.9 状态表面）
- `src/serialforge/presentation/theme.py`（UI-1.9 状态 QSS）
- `src/serialforge/presentation/qt.py`（UI-1.10 Qt 动效导出）
- `src/serialforge/presentation/main_window.py`（UI-1.11 接收活动状态）
- `src/serialforge/presentation/theme.py`（UI-1.11 active 状态 QSS）
- `src/serialforge/presentation/main_window.py`（UI-1.12 链路控制状态带）
- `src/serialforge/presentation/theme.py`（UI-1.12 链路状态 QSS）
- `src/serialforge/presentation/main_window.py`（UI-1.13 TX activity feedback）
- `src/serialforge/presentation/main_window.py`（UI-1.14 发送控制状态带）
- `src/serialforge/presentation/theme.py`（UI-1.14 发送状态 QSS；UI-1.15 原生控件暗色兜底；UI-1.16 header cluster QSS；UI-1.17 observation state QSS）
- `src/serialforge/presentation/theme.py`（UI-1.14 发送状态 QSS；UI-1.15 原生控件暗色兜底；UI-1.16 header cluster QSS；UI-1.17 observation state QSS；UI-1.18 sendState QSS）
- `src/serialforge/presentation/main_window.py`（UI-1.16 header cluster 布局；UI-1.17 observation state 投影；UI-1.18 sendState 展示）
- `src/serialforge/presentation/main_window.py`（UI-1.19 pipelineSummary source/state 投影）
- `src/serialforge/presentation/theme.py`（UI-1.19 pipelineSummary 历史/活跃/过渡/阻断/草稿暗色 QSS）

UI-1.7 追加触及的源码范围仍仅为：

- `src/serialforge/presentation/main_window.py`
- `src/serialforge/presentation/theme.py`
- `src/serialforge/presentation/widgets.py`

交接文件：

- `README.md`
- `docs/handoffs/current.md`
- `docs/handoffs/2026-08-10-ui-1.5-state-surface.md`
- `docs/handoffs/2026-08-10-ui-1.6-anime-theme-package.md`
- `docs/handoffs/2026-08-10-ui-1.7-embedded-station.md`
- `docs/handoffs/2026-08-10-ota-debug-boundaries.md`
- `docs/adr/0023-ota-debug-extension-boundaries.md`
- `docs/handoffs/2026-08-10-ui-1.9-state-surfaces.md`
- `docs/handoffs/2026-08-10-ui-1.15-native-controls.md`
- `docs/handoffs/2026-08-10-ui-1.16-header-clusters.md`
- `docs/handoffs/2026-08-10-ui-1.17-observation-state.md`
- `docs/handoffs/2026-08-10-ui-1.18-send-state.md`
- `docs/handoffs/2026-08-10-ui-1.19-pipeline-summary.md`
- `tasks/plan.md`
- `tasks/todo.md`

没有 Git 元数据，以上是当前 checkout 的静态文件范围，不是由 `git diff` 推导的提交清单。

## 六角色只读审查

本轮六角色均使用 `luna_max / max / Fast`，未编辑、未启动 GUI/EXE/服务、未连接网络/硬件、未运行测试。

| 角色 | agent id | 结论 |
|---|---|---|
| 产品/UX | `019fe75c-af21-7aa0-93bf-e46eada621fe` | 建议错误状态恢复、可见空态和窄窗重排；本轮采纳前两项，窄窗视觉仍待授权运行时验证。 |
| 架构/生命周期 | `019fe75c-af67-7830-be4f-2db7f486d0ca` | 确认装饰时钟与渲染 timer 分离；要求关闭 fence，保留 BLE worker/operation token 跨层风险。 |
| UI/accessibility/motion | `019fe75c-afa4-7c40-80bd-1c902e04c746` | 建议状态按钮 AccessibleName、派生表面焦点/描述同步、曲线窄宽度保护；已采纳。 |
| 开发/代码正确性 | `019fe75c-afe0-7d73-a00f-e02049616ec8` | 确认 TCP Server peer gate、空发送和 hotplug 观察；peer gate/空发送已采纳，hotplug 仍保持跨层边界。 |
| 验证 | `019fe75c-b022-78a0-8c4b-d3205a8f0f14` | 要求把资源审计写成“无外部资源文件”，并区分渲染 timer 与 MotionController；已修正文档措辞。 |
| 交付/流程/打包 | `019fe75c-b05f-7680-b6f7-b84df0307d10` | 要求 static/GUI/发行分维度状态、保留历史产物说明和资源 provenance 边界；已采纳。 |

### UI-1.6 视觉与交付复核补充

- 产品/UX `019fe782-1693-7243-b06c-d320e150f23e`：建议原创“夜航信号观测站”抽象几何、限制渐变噪声、保持终端可读性；本轮采用星轨几何并保留调试主路径。
- 架构/动效 `019fe782-16d9-7263-85f8-5a5bf1474fae`：确认 MotionController 仍是唯一共享时钟，建议仅动画绘制参数；本轮未新增控件级 timer。
- UI/API/资源 `019fe782-1756-77a2-b477-09ba57909125`：发现快捷键构造兼容风险和主题 token 导出遗漏；已改为位置参数并补齐 `__all__`。
- 其它本轮只读角色均未编辑文件；其中两份交付报告返回了非当前 checkout 的仓库上下文，未作为 SerialForge 证据采纳。打包结果由父代理直接复核 provenance、PE 版本和 canonical/root SHA-256。

### UI-1.7 架构与独立复核补充

- 架构师 `019fe790-2b37-7110-bc44-0c0f63732cf4`：确认本轮只允许 presentation-only 变更，指出白色来源是 QScrollArea viewport，并要求以项目自有 viewport 标识隔离主题层；本轮遵守。
- UI/UX `019fe790-2b83-75a0-8814-2278ce170e36`：建议以链路连接、协议遥测、命令管理对应工作区导航，并强化实时观测/发送控制；本轮采纳。
- Qt/QSS `019fe790-2bcb-71d3-b831-7b5d542b8d7c`：确认原生 viewport 是白色大块最高概率来源；本轮增加稳定 viewport surface。
- 动效生命周期 `019fe790-2c15-7520-a41b-26c511040afa`：确认单一 MotionController、暂停/低动效/隐藏/关闭降级边界；本轮未增加控件级 timer。
- 增量架构门检 `019fe79e-e2fb-7ea3-9897-2673b4047fab`：对 viewport 层级修正、tab 作用域、局部 polish 和辅助描述补齐给出有条件 GO。
- 独立复核 `019fe79a-2d41-7f50-8d9a-0cf8b1517f0a`、`019fe79a-2d86-7c50-be28-4489ee4e912b`：指出并推动修正 scrollContent 选择器层级、workspace tab 样式作用域和动态属性刷新；修正后静态可合入。
- 可访问性复核 `019fe79a-2dc3-7d33-b2ec-3af9766f045a`：指出暂停显示描述、UART checkbox 辅助信息等静态缺口；本轮已补齐。其运行时 NO-GO 仅表示 GUI/读屏/焦点尚未授权验证，不是代码静态合规声明。

### UI-1.8 / OTA-debug 扩展审查补充

- 架构师 `019fe7ac-3885-7e41-a541-373e8084c61a`：确认 station band 只应包住现有工具条，来源属性必须使用两个历史标志，禁止把业务状态搬入主题层；本轮采纳。
- UI 信息层级复核 `019fe7a8-01fe-7ca1-a935-0bf7ecbe5df5`：建议实时/历史分色、低对比度 band、避免重复胶囊；本轮采用 source 色轴，未增加重复状态动画。
- 新增扩展架构师调用已发起，但因共享线程池限流未在本轮返回；未将其当作验证证据。父代理依据现有 `docs/ARCHITECTURE.md`、`docs/CONSTRAINTS.md` 和 ADR 0023 整合最小 contract-only 方案。
- 嵌入式独立审查调用 `019fe7bb-b9d6-7452-ac15-a32df7b54bb8` 因超时关闭，未返回结论；父代理完成同等只读边界审查并记录 applicability=N/A，不声称 MCU/固件合规。

## 独立复核与简化

- `019fe765-dd8e-7243-a98d-3124c2a5f755`（Goodall）：初次发现状态栏错误恢复、关闭回调和全空 Hex 防御项；已分别补充最近正常状态、多个关闭 fence 与规范化 payload gate。
- `019fe765-ddd3-78c1-8cf6-2e377fd7bee0`（Ampere the 2nd）：发现派生视图恢复不一致；已改为隐藏期间保留快照、恢复时统一恢复协议帧/组件行/Dataset/曲线。
- `019fe76b-e262-7e52-ac17-7e3393612677`（Boyle the 2nd）：最终 adversarial review 结论 P0/P1=0；补出暂停字节回调的 P2 关闭 fence，已修正。

简化结论：本轮没有引入第二个 MotionController，也没有把业务状态写入装饰控件；空态使用已有 QPlainTextEdit placeholder，不新增资源或复杂 overlay；Dataset 滚动和 Component/Protocol 文档仍保留各自不同上限与生命周期，未强行合并。

## 动效、生命周期与资源边界

- `MotionController` 仍是装饰性信号场/状态灯共享的唯一 motion clock；`MainWindow` 的 preview/component timer 与 `DatasetCurveWidget` 的 snapshot timer 是文档渲染调度，不应合并宣传为“整个 UI 只有一个 timer”。
- `closeEvent`、`hideEvent`、最小化变化继续停止文档重绘与装饰动效；恢复时刷新最新有界快照。后台 discovery/BLE worker 的实际阻塞取消、operation id/service epoch、Replay/EventBus 顺序仍属于 application/infrastructure 专项，未在本轮修改。
- 当前 presentation 没有外部图片、GIF、字体文件、QSS/QRC 文件或新增依赖；QSS 仍内嵌在 `theme.py`，字体仍使用系统 fallback。没有 Git 元数据，以上不是提交 diff。
- 本轮是 Python desktop UI + contract-only OTA/debug 边界变更，embedded C/C++/firmware applicability=`N/A`；不声称 MISRA、ISO、WCAG、认证或硬件合规。已按嵌入式流程完成公开来源适用性判定、独立审查尝试、行为保持简化评估和静态验证；没有 MCU/SDK/RTOS/工具链目标可绑定。
- 嵌入式流程收据：`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`（本轮仅为 Python contract/UI 边界，不代表固件或认证合规）。

## 当前源码静态验证证据

以下检查在 UI-1.38 工作区路线 beacon 最后一次源码写入之后执行：

- `scripts/check.ps1` → `All checks passed!`；源码 `124` 个 Python 文件，均不超过 `1000` 行（`main_window.py` 当前 `634` 行）。
- `scripts/check_theme_tokens.py` → `3 themes, 22 semantic tokens, 11 selectors, legacy_qss_literals=309`。
- AST/import smoke → `IMPORT_SMOKE PASS modules=124 qapplication=none`；MainWindow、bootstrap、composition、theme runtime、preferences、OTA/debug contracts 通过。
- 主题 runtime/offscreen → 3 套主题、22 个控件的 QSS 应用通过；无白色背景、未解析占位符和关键 selector 缺失。
- `QSettingsPreferenceStore.load()` offscreen → schema v1、已知 theme key、bool 类型和未知主题 fail-open 通过。
- 完整 `QT_QPA_PLATFORM=offscreen` composition → 真实 application/window 组装、545 个 QWidget、主题偏好
  hydration、BLE 控件初始化、`connectionControlBand` 与 `settingsViewport` 通过。
- UI-1.37 共享环境动效 → 空闲基线 `timer_active=False / frames=0`；开启 ambient 后同条件约 9 帧/秒；完整窗口在
  980×680 下 `ambient_timer=True / animated_frames=5`，隐藏停表、恢复续帧、关闭停表。
- UI-1.37 像素审计 → `near_white=0 / white=0`；截图 `build/ui_review_ui137_ambient_motion.png` 已人工查看，链路连接行及其余
  shell 没有白色大块。
- UI-1.38 工作区路线 → 980/1180 shell geometry 正常，route 分别为 `814,3,132,28` / `1014,3,132,28`，Tab index 0/1/2
  全部投影，横向滚动 `0`；三主题截图 near-white 均为 `0`，`build/ui_review_ui138_route_980.png` 已人工查看可见路线 beacon。
- UI-1.22～UI-1.24 offscreen 复核 → 六类传输在 980px 宿主下均无横向滚动；命令批处理五态、回放六态、
  两张只读结果表 vertical header、三主题状态色和 Tab 动效（含 reduced-motion 跳过）均通过。
- 最终截图人工查看 → `build/ui_review_final_connection.png`、`build/ui_review_final_protocol.png`、
  `build/ui_review_final_commands.png` 均未见白色大块、Tab 基线白线或命令表左侧白色行号回退。
- UI-1.25 offscreen 复核 → catalog schema/count/key/range、三主题切换、7 个内置连接 preset、typed callback、
  transport/data/提示同步和 980px 连接页 horizontal scrollbar 均通过；TCP Server transport/host/port 变化
  均清除旧授权，preset transport refresh 为每次恰好一次，快速配置不自动连接；最终图
  `build/ui_review_connection_preset.png` 已人工查看，快速配置下拉为 info/focus 语义色且没有白色回退。
- UI-1.20/ARCH-7a/BLE 修复专项静态审计：纯 `supported_ble_write_modes(characteristic)` 不再收到隐式
  window；MainWindow facade 继续不超过 1000 行。
- UTF-8 截断、曲线溢出/非法字段、TCP Server 回环/非回环 readiness → `UTF-8 bound / curve validation / TCP Server readiness vectors: passed`

未运行任何单元测试、持续 GUI、HIDPI、真实焦点/读屏、对比度渲染、动效 CPU/内存、真实
UART/TCP/UDP/BLE/RTT、服务、EXE 启动/退出或硬件；offscreen 构造、像素审计和临时截图仅是受控的一次性
检查，不代表人工视觉、性能、硬件或正式发行通过。

## 当前 EXE 交付证据

 - 根目录便利副本：[SerialForge.exe](../../SerialForge.exe)，`47,795,336` bytes，SHA-256 `456722C6D7D5B0F5F64A23FE0C9EEF61E2508E42CFBD2153E38662BCD6C8F735`。
- canonical artifact：`dist/release/0.1.0/core/onefile/app/SerialForge.exe`，与根目录副本逐字节一致。
- provenance：`dist/release/0.1.0/core/onefile/PROVENANCE.json` 已通过脚本内置 verification；source revision `local-ui-1.38`、PyInstaller `6.22.0`、Python `3.12.13`、PE `0.1.0.0`、签名 `NotSigned`；archive listing SHA-256 `0A1F0D57EB1DA293E6ABFD1384653D9CAB802DEC5AF7FC91091702B4F618C26E`。
- 该包是 `engineering_build`，`release_eligible=false`；许可证清单仍为 inventory-only，真实硬件验收为 `not_run`。没有把“生成 EXE”写成“启动通过”或正式发行通过。
- 打包命令必须使用 `pwsh.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\package.ps1 -Mode onefile`；Windows PowerShell 5.1 在当前无 BOM UTF-8 脚本上解析失败，未进入打包阶段。

## 旧产物与发布边界

- `dist/release/0.1.0/core/onefile` 与 `build/pyinstaller/provenance/0.1.0/core/onefile` 是本轮当前源码重建产物；其它更早的 build/dist 变体仍只能标为 `observed-historical`。
- 根目录 `SerialForge.exe` 是 onefile 便利副本；不能把 onedir 目录中的单个 EXE 平铺复制为根目录文件，因为 onedir 依赖同目录 DLL/Qt platform 文件。
- 如果未来用户授权接入图片、字体、QSS/QRC 或第三方视觉资源，必须另开资源专轮并记录路径、hash、来源/许可证、尺寸/大小、静态回退和打包收集边界。

## 下一步与待用户选择

1. 若要继续细化，可在当前无 IP 主题上授权 GUI 截图、HIDPI、焦点/读屏与动效性能验收；offscreen 组装已通过，
   但不替代人工视觉验收。
2. ARCH-6 继续把显式 window facade 收窄为 feature state/action DTO；ARCH-7c 再在独立审查后加入用户 preset 编辑、builtin/custom
   merge 和显式保存动作，继续保持不存储密钥/敏感端点。
3. OTA 真实实现仍需用户提供目标 MCU、bootloader、镜像格式和公开版本资料，并单独走架构/安全/硬件验收门。
4. 若要加入具体角色/插画/背景/字体，需用户提供选择或授权来源，并另建资源 hash、许可证和静态回退记录。
5. 若要解决 BLE 实际取消、operation token/service epoch、Replay/EventBus 顺序或隐藏时 hotplug 轮询，需要扩大跨层范围并重新六角色审查。

## 交接结论

UI-1.38 工作区路线 beacon、UI-1.37 共享环境动效、UI-1.36 终端观测标尺、UI-1.34 终端空态观测网格与扫描线、UI-1.33 连接控制带四节点状态路径、ARCH-7b preset catalog store boundary、UI-1.30 批量步骤/回放活动轨道、UI-1.29 顶栏状态指示器几何动效、UI-1.28 协议流水线主题化轨道、UI-1.27 协议/遥测派生 surface、UI-1.26 终端空态与响应式工具栏、UI-1.20 主题语义 token/runtime、ARCH-7a 偏好端口、BLE facade 初始化修复，以及此前 UI-1.10～UI-1.25
状态表面/动效/contract-only 扩展边界已完成静态验证、offscreen composition、当前源码 onefile 打包和根目录
交付；EXE 启动、人工 GUI 视觉、硬件和正式发行仍未验收。ARCH-6/ARCH-7c 继续作为全局企业化目标，不能把
当前静态与 offscreen 证据写成最终完成声明。
本轮使用 `local-arch-122` 完成 onefile 并覆盖根目录两个 EXE：canonical、root、root-latest 均为 `48,044,048` bytes，SHA-256 为
`CF85FAFB7F757FA09853670D051077A31FABECF52A91A8CF205A8145C2DF0B6D`；archive listing SHA-256 为
`5F69B28029EBCFED0787889EA9538DEF21BE5633BD32BE48380A5592A34C2AF1`，provenance verify 通过；签名、正式发行资格与硬件验收为
`NotSigned`、`release_eligible=false`、`hardware_acceptance=not_run`。
