# Batch 43+ — UI/动画质量提升永续迭代（VOFA+ / MobaXterm 对标）

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 1/∞（ralph-loop active）
> 来源：用户目标「目前组件 UI 很丑 / 动画不好 / 跟 VOFA+ MobaXterm 差距很大 / 无死角迭代 / 永远不允许停止」
> 关联：Batch 40-42 已落地 18 个新模块；本批起转向**质量提升**而非新功能堆叠
> 评分目标：668 → ∞（每批 +1）

---

## 1. 用户痛点（精确解读）

| 痛点 | 表现 | 对标竞品 |
|---|---|---|
| **UI 丑** | 色彩/排版/图标/留白不专业 | VOFA+ 的工业风深色调 / MobaXterm 的精致对比 |
| **动画不好** | 时长不一致 / 缓动生硬 / 缺编排 / 缺状态反馈 | VOFA+ 波形游标顺滑 / MobaXterm 标签切换微动 |
| **差距大** | 整体完成度与商业软件有质感落差 | 两者都是装机必备级别 |

## 2. 改进维度（待研究 agent 落地后定稿）

### ⚠️ 关键认识（2026-06-22 主线预审计）

**项目基础已经齐全，不是「从零做」而是「精修一致性 + 编排 + 状态完整」**：

| 维度 | 当前实际状态（预审计） | 之前的误判 |
|---|---|---|
| Lucide SVG 图标 | **146 个** in `resources/icons/lucide/` | CLAUDE.md 写 14 个（已过时） |
| palette 色板常量 | **64 个** | — |
| tokens 系统 | **完整** RADIUS×7 + SPACING×7 + PADDING×4 + BORDER×N | — |
| 单元测试基线 | **1603 passed** | — |

**真正的问题**（待审计细化）：
- **Consistency**：是否所有控件都用了 token？是否有散落硬编码？
- **Composition**：三区 QSplitter 布局密度、卡片层次、视觉重心是否合理？
- **State completeness**：QSS 是否所有 QPushButton/QLineEdit/QComboBox 都齐全 hover/pressed/disabled/focus 四态？
- **Animation choreography**：单个动画已有 13 个工厂，但是否有「编排」？面板入场是同时突现还是错峰？
- **Panel-level polish**：连接卡 / 波形卡 / 日志卡 / 命令卡是否达到 VOFA+ 单卡的精致度？

### A. 视觉系统一致性（不再扩 primitives，改用对齐 + 编排）
- A1 **审计**：哪些源文件散落硬编码颜色/尺寸/圆角（grep `[0-9]px` / `#[0-9a-f]{6}` 不在 palette/tokens 内）
- A2 **修复**：把硬编码迁移到 palette/tokens 引用
- A3 palette 语义复审（INFO_SOFT 单独定义在 info_banner 顶部，应进入 palette）
- A4 QSS 分区合并审计（11 个 qss_sections_*.py 是否有重复/冲突 selector）

### B. 控件状态完整性（hover / pressed / disabled / focus）
- B1 全量审计：所有 QPushButton/QLineEdit/QComboBox/QSpinBox 是否齐全 4 态
- B2 focus 状态可视化（键盘导航可发现性）
- B3 disabled 视觉降饱和处理
- B4 错误状态视觉强调（红边框 + 错误图标）

### C. 动画编排（choreography 是关键）
- C1 时长 token 复审（DURATION_INSTANT/FAST/NORMAL/SLOW/SLOWER 是否合理）
- C2 缓动曲线扩展（cubic/quart/quint/OutBack/OutElastic/InOutCubic 已 7 个，是否够用？是否要加 Spring/InOutQuart）
- C3 缺失交互补齐（tab 切换、list 增删、菜单展开、连接状态变化）
- C4 **编排**（错峰入场：多卡片用 60ms 步长 stagger，避免同时突现）
- C5 可打断性（新动画取消旧的，不堆叠）

### D. 关键面板精修（直接对标 VOFA+ MobaXterm，3 选 1-2）
- D1 连接配置卡片（`connection_toolbar.py` / `connection_sidebar.py`，对标 VOFA+ 顶部 config bar）
- D2 波形显示区（`waveform_preview.py` + 11 个 waveform_*.py，对标 VOFA+ 波形卡片）
- D3 日志/终端区（`log_view_content.py`，对标 MobaXterm 终端字体 + 配色 + 滚动条）
- D4 命令输入区（`command_section.py` + `command_palette.py`，对标 VOFA+ 命令历史 + 自动补全 popup）
- D5 状态栏（`status_actions.py`，对标 MobaXterm 多段状态栏：连接/字节/FPS/时间）

### E. 空态/加载态/错误态精修（铁律 5.9 三轴口径已要求）
- E1 空态：无连接/无数据/无配置 → 图标 + 文案 + CTA 按钮
- E2 加载态：连接中/解析中/导出中 → 骨架屏（SkeletonAnimation Batch 41）+ 进度环（ProgressRing Batch 40）
- E3 错误态：连接失败/协议错误/解析异常 → 错误卡片 + 重试按钮

### F. 微交互细节（最后打磨）
- F1 按钮波纹（Ripple）一致性审计
- F2 输入框聚焦动画（边框颜色过渡，subtle 200ms EASE_OUT）
- F3 卡片 hover 隆起（subtle translateY -2px + shadow 强化）
- F4 图标状态过渡（hover 切色/缩放 1.05x）
- F5 Tab 切换滑动指示器（slide indicator，240ms EASE_OUT_QUART）

## 3. 验收标准（量化）

| 维度 | 当前 | 目标 |
|---|---|---|
| Lucide SVG 图标数 | 14 | ≥ 100 |
| QSS 覆盖 objectName | 已覆盖 | 持续 100% 覆盖率守护通过 |
| 控件 4 态完整率 | 待审计 | 100%（hover/pressed/disabled/focus） |
| 动画覆盖关键路径 | 待审计 | 100%（连接/收发/解析/导出/切换） |
| 全量 pytest | 1626 passed | 持续不退化 |
| smoke 启动 | exit 0 | 持续 exit 0 |
| 评分 | 668 | 每批 +1 |

## 4. 永续策略

- **每批规模**：5-10 个独立模块或 1-2 个共享文件改造
- **共享文件改造**（palette/tokens/qss）：串行单 agent，前一批先备份
- **新增独立模块**：10 个并行 agent（参考 Batch 40 范式）
- **每批 ≥ 500 行净增**（铁律 3）
- **每批必须 commit**（铁律 5.10）
- **永不停止**（用户铁令）

## 5. 待研究 agent 落地后定稿

具体批次切分、API 契约、文件路径、并行边界将在 3 个研究 agent（bg_71dd983f / bg_abd705a3 / bg_ce392478）落地后由 Plan agent 输出。

---

## 6. VOFA+ / MobaXterm 对标 8 层审计清单（bg_71dd983f 落地后补充）

研究 agent 已交付具体 token 名 + RGB 值 + 改进版本号，转化为以下可审计清单：

### Tier 1 — Token 系统基础（成本低、ROI 高）
- [ ] **T1.1** 定义单一 `Theme` 类，≥ 20 个命名 color role（对齐 VOFA+ `appTheme`：bgColor/barColor/mainColor/fontColor/lineColor/lightColor/goodColor/badColor/cbColor1-4/iconColor1-3/iconSColor1-3/objColor/textAnsiErr/Tx/Rx/Time）
- [ ] **T1.2** 分离 **log palette**（Tx/Rx/Err/Time）/ **chart palette**（cbColor1-4）/ **icon palette**（iconColor/iconSColor × 3）
- [ ] **T1.3** Theme 类外禁止 `QColor("...")` 硬编码，CI grep 守护
- [ ] **T1.4** Theme 支持导出/导入 JSON 或 INI（对标 MobaXterm `.mxtcolors`）
- [ ] **T1.5** OS 暗色模式自动检测首次启动（对标 MobaXterm v20.0）
- [ ] **T1.6** 主题切换动画 ≤ 300ms

### Tier 2 — Tab 系统专业化
- [ ] **T2.1** Tab 抗锯齿（active tab 指示器无锯齿）
- [ ] **T2.2** Tab 圆角可配置
- [ ] **T2.3** 3 态可见性：active / inactive-idle / **inactive-modified**（蓝点 + 蓝标题）
- [ ] **T2.4** 拖拽 detach tab 到独立窗口
- [ ] **T2.5** 每 tab 用户可分配 color + icon
- [ ] **T2.6** `Ctrl+Tab/Ctrl+Shift+Tab` 循环；`Ctrl+Alt+T` 新建；`Ctrl+Alt+Q` 关闭

### Tier 3 — 波形精修（VOFA+ 强项）
- [ ] **T3.1** 所有波形 `Antialiasing` ON（1px 对角无锯齿）
- [ ] **T3.2** 通道行密度：`[toggle] [name] [color] [precision] [gain] [offset]` ≤ 140px
- [ ] **T3.3** X 轴双击 → 测量游标
- [ ] **T3.4** 鼠标滚轮按光标区域缩放（X-only/Y-only/both）
- [ ] **T3.5** "Auto" 按钮一键 Y 轴适配可视窗口
- [ ] **T3.6** 缓冲窗口 4 色渐变 scrubber（对标 cbColor1-4）
- [ ] **T3.7** 基准：100k samples/channel 持续 ≥ 30 FPS
- [ ] **T3.8** 视图模式切换：line ↔ histogram ↔ FFT

### Tier 4 — 连接状态微动画
- [ ] **T4.1** 连接按钮：**深蓝 → 浅蓝** 状态过渡（对标 VOFA+ quick_start）
- [ ] **T4.2** 状态栏 live 计数器更新 ≤ 100ms（无抖动）
- [ ] **T4.3** RX/TX 字节计数用等宽数字（数字变化时无布局抖动）

### Tier 5 — 终端/日志视图精修（MobaXterm 领域）
- [ ] **T5.1** 16 色 ANSI 调色板，**Bold 变体独立**（不是 8 色复用）
- [ ] **T5.2** 默认字体：Cascadia Code / JetBrains Mono / Fira Code（不是 Consolas）
- [ ] **T5.3** 日志视口 1-2px inset 边框（对标 MobaXterm v20.0）
- [ ] **T5.4** 等宽光标，`CursorColour` ≠ `ForegroundColour`
- [ ] **T5.5** `Ctrl+Plus / Ctrl+Minus` 运行时字号调整
- [ ] **T5.6** `Ctrl+Shift+F` 终端内查找，高亮匹配

### Tier 6 — 设置对话框架构
- [ ] **T6.1** 全局设置：4-5 个分类 tab（General / Connection / Terminal / Display / Shortcuts）
- [ ] **T6.2** 每会话覆盖（font/colors/baud）
- [ ] **T6.3** 设置对话框内搜索框
- [ ] **T6.4** 全部设置单文件导出/导入

### Tier 7 — HDPI 与 flicker（2026 年不可妥协）
- [ ] **T7.1** `Qt::AA_EnableHighDpiScaling` + `Qt::AA_UseHighDpiPixmaps` 在 `QApplication` 构造前设置
- [ ] **T7.2** 所有图标 SVG 或 `@2x` PNG
- [ ] **T7.3** 字号一律 `pt`，禁止 `px`
- [ ] **T7.4** Resize 测试：图表动画期间无白闪
- [ ] **T7.5** 自绘控件 `WA_OpaquePaintEvent` 防 flicker

### Tier 8 — 空/加载/错误态精修
- [ ] **T8.1** 空波形画布显示 "拖拽图表到此" 提示（对标 VOFA+ drag-native）
- [ ] **T8.2** 连接错误 → 内联红色 banner（非模态弹窗）
- [ ] **T8.3** 加载态用 goodColor/badColor token（依赖 Tier 1）
- [ ] **T8.4** "已断开" 与 "尚无数据" 视觉区分

---

## 7. Batch 43 候选范围（待 Plan agent 切分）

按 ROI/成本排序，每批选 5-10 项独立模块或 1-2 项共享文件改造：

| 候选 | 类型 | 估时 | 风险 |
|---|---|---|---|
| T1.3 hardcoded color 守护测试 | 新增测试 + grep 工具 | 0.5 天 | 低 |
| T1.5 OS 暗色检测 | 改 main_window.py / app 入口 | 0.5 天 | 低 |
| T4.1 连接按钮 2 色过渡 | 改 connection_toolbar + 新增 color tween 动画 | 0.5 天 | 低 |
| T7.1 HDPI 验证 + 修复 | 改 app/main.py + 字号审计 | 0.5 天 | 中（可能触发回归） |
| T2.3 tab 3 态可见性 | 改 dashboard_tabs QSS + state marker | 1 天 | 中 |
| T1.4 theme JSON 导出/导入 | 新增 theme_serializer.py | 1 天 | 低 |

**待 Plan agent 落地后切具体批次边界**。

---

## 8. 动画审计 P0 路线图（bg_ce392478 落地后补充）

动画审计揭露若干 **P0 关键 bug + 大量死代码**。原 Batch 43 计划（ColorTween/Stagger/ThemeSerializer/Guardian）保持推进，但 **Batch 44 必须优先修 P0**：

### Batch 44 P0 — 修复破损核心（必须）
- [ ] **P0-1** 修 `AppShell._switch_to` leave 动画隐形 bug（`setCurrentIndex` 在 `fade_out` 渲染前同步执行；方案：`setCurrentIndex` 推迟到 `fade_out.finished` 信号，或并行 cross-fade + z-order）
- [ ] **P0-2** `set_connection_control_state` 加连接状态动画（`GlowAnimation.pulse` 3 次循环 + 按钮文字 cross-fade "Connect" ↔ "Disconnect"）
- [ ] **P0-3** `DashboardTabs._on_tab_changed` 接 `PageSlideAnimation`（按 tab 索引差决定方向；Batch 41 的 PageSlide 死代码激活）
- [ ] **P0-4** tokens.py 加 `EASE_OUT_QUINT` / `EASE_IN_QUINT` / `EASE_MATERIAL_EMPHASIZED` + `DURATION_CONTAINER=300` + `KEYFRAME_PREVIEW=0.3`；page_slide.py:151 迁移到 `AnimationTokens.EASE_OUT_QUINT`
- [ ] **P0-5** 删除 `panel_animations.fade_in/out/slide_in` 重复工厂，全部路由到 `FadeTransition` / `SlideAnimation`

### Batch 45 P1 — 编排与润色
- [ ] **P1-6** `stagger_fade` 的 `QTimer.singleShot` 改 `QSequentialAnimationGroup`（GC-safe 整体）
- [ ] **P1-7** `command_palette._rerank` 结果 stagger slide-in
- [ ] **P1-8** 日志新条目 fade-in（rate-limited 防突发）
- [ ] **P1-9** `ConfigurableButton` 默认开 ripple（gated by `dense` flag）

### Batch 46 P2-P3 — 激活死代码
- [ ] **P2-10** `install_tooltip` 接到所有工具栏按钮
- [ ] **P2-11** `SkeletonAnimation.shimmer` 改用 `QGraphicsOpacityEffect`（非 windowOpacity），激活面板加载态
- [ ] **P2-12** `RotateAnimation` spinner 接 connect 按钮 "connecting" 中间态
- [ ] **P2-13** `BouncePathAnimation.drop_in` 用于新放置的 dashboard 控件
- [ ] **P2-14** `ElasticSnapAnimation` 用于浮动面板拖拽释放吸附
- [ ] **P2-15** `TypewriterAnimation` 用于状态消息/欢迎文本

### Batch 47 — 死代码守护测试
- [ ] **DC-1** `test_animation_tokens.py`：每个 easing/duration token 至少被一个 factory 引用（防 Batch 40 式孤儿工厂）
- [ ] **DC-2** `test_no_dead_animations.py`：每个 Batch 40+ animation module 至少有 1 个 production 调用点

### 破损 bug 清单（动画审计附赠）
| 文件 | 行 | 问题 |
|---|---|---|
| `app/app_shell.py` | 200-217 | leave 动画隐形（setCurrentIndex 同步覆盖） |
| `animations/fade.py` | 71-77 | cross_fade 用 QSequentialAnimationGroup 导致中间透明间隙 |
| `panel_animations.py` | 94-97 | stagger_fade QTimer + lambda 有 GC 风险 |
| `controls/configurable_button.py` | 116 | 用 merged `press()` 不是 separated press_down/up（pressed 反馈延迟到 release 后） |
| `controls/configurable_button.py` | 52 | `_ripple_enabled = False` 默认关 |
| `animations/skeleton.py` | 26-29 | windowOpacity 对内嵌控件是 no-op |
| `panel_animations.py` | 29 | `DURATION_SLOW = DURATION_NORMAL` 别名误导 |
| `animations/scale.py` | 119-128 | hover_in scale 1.03 < 1 JND |
| `animations/page_slide.py` | 19-22 | TODO 自承 EASE_OUT_QUART 缺 token |

---

## 9. UI 审计 P0 路线图（bg_abd705a3 落地后补充）

UI 审计的核心结论：**palette 已经是 VOFA+ 级别，"丑"的根因不是颜色，而是**：

### 9.1 QSS 状态完整度（~30 个按钮缺 :pressed / :disabled）— 最高优先
- ✅ 已修：tool buttons (refresh/export/replay/save/load/clear) + send/inject 共 8 个按钮（Batch 43 主线修）
- ❌ 仍缺：domain primary 5 个（RTT start, CAN send, BLE connect, Auto run, Settings apply）
- ❌ 仍缺：domain secondary 11 个
- ❌ 仍缺：dashboard buttons 5 个
- ❌ 仍缺：OTA browse/start
- ❌ 仍缺：ConfigurableButton :disabled
- ❌ 仍缺：EmptyStateCTA :pressed/:disabled
- ❌ 仍缺：QTabBar::tab :hover
- ❌ 仍缺：所有 QPushButton 的 :focus 键盘焦点 outline

### 9.2 卡片 hover-lift 全员缺失（P0，signature 动画只接了 2 个控件）
- ✅ 已接：`ConfigurableButton`, `EmptyStateCTA`
- ❌ 缺：6 个主要 `serialStationCard`（连接 / 波形 / 日志 / 命令 / Profile / 状态）
- 方案：在 `main_window` 装配处或新 helper 内对每个 card 调 `install_hover_lift`

### 9.3 图标覆盖（27+ 按钮零图标）
- ❌ BLE scan/connect/read/write/notify (5)
- ❌ CAN send/clear/demo (3)
- ❌ RTT start/clear (2)
- ❌ Automation run/fire/refresh (3)
- ❌ SVD load/demo (2)
- ❌ OTA browse/start (2)
- ❌ Dashboard addTab/clear/save/load/grid (5)
- ❌ Settings apply (1)
- ❌ Tools: timestamp now, hex format, byte analyze/clear (4+)
- 方案：扩展 `button_icons.py:_BUTTON_ICON_MAP`，从 146 个 lucide SVG 选对应图标

### 9.4 其他 UI 审计 P0
- **Toast Unicode 字形** → 应替换为 lucide SVG（info/check-circle/alert-triangle/x-circle 已存在但未用）
- **InfoBanner / Chip 关闭 ×** → 应替换为 lucide `x` SVG（当前手绘线条）
- **QComboBox 下拉箭头** → 当前默认 Qt 三角，应改 `chevron-down.svg`
- **QSpinBox 上下箭头** → 完全未样式，应改 `chevron-up.svg` / `chevron-down.svg`
- **light theme palette 缺 6 个 CARD_* token**（CARD_SHEEN_TOP/MID/BOTTOM, CARD_INNER_TOP_EDGE, CARD_GROUND_SHADOW, CARD_HOVER_RING）→ light theme 卡片无玻璃质感
- **无 INFO 语义色**（INFO 复用 ACCENT cyan）→ 应加 INFO = "#3b82f6" 蓝
- **install_nav_hover_scale 死代码**（定义从不调用）
- **无 Connect / Refresh 加载态**（最痛的 UX 缺口）

### 9.5 排版/间距 token 逃逸（系统性问题，需要 P1 批次统一修）
- `setContentsMargins(12,12,12,12)`, `setSpacing(10/14)` 等魔法数字遍布 layout_cards/layout_main/top_bar/sections/toast/info_banner/chip/segmented/empty_state
- `setPointSize(40/13/10)` 在 empty_state 完全绕过 FONT_* token
- 13 个文件用 inline `setStyleSheet` 绕过全局 QSS（dashboard/palette, controls/slider, controls/value_display, widgets/toast, tools/*, panels/_accent_row 等）
- 字体族 `"Microsoft YaHei UI", "Segoe UI", sans-serif` 太通用，应引入 Inter 或 Sarasa Gothic

---

## 10. 合并后的批次规划（动画 + UI 两份审计综合）

| Batch | 主题 | 关键项 | 类型 |
|---|---|---|---|
| **Batch 43**（运行中） | 基础工具：ColorTween / Stagger / ThemeSerializer / Guardian tests | 新增 4 模块 | 并行 agent |
| **Batch 44** | UI P0：补全 QSS 状态 + card hover-lift + 扩展 button_icons | 共享文件改造 | 串行主线 |
| **Batch 45** | 动画 P0：修 AppShell leave + 接 PageSlide 到 DashboardTabs + 接 GlowAnimation 到 connect + 删 panel_animations 重复 | 共享文件改造 | 串行主线 |
| **Batch 46** | 图标 + glyph 替换：Toast SVG 化 + InfoBanner/Chip close SVG 化 + ComboBox 箭头 SVG 化 | 多文件并行 | 并行 agent |
| **Batch 47** | 排版/间距 token 化：FONT_ROLE_* / LETTER_SPACING_* / 统一 setContentsMargins 引 token | 共享文件 + 13 个调用方 | 串行主线 |
| **Batch 48** | 状态完整：INFO 色 + light theme 补 6 个 CARD_* + Connect/Refresh 加载态 + 空 log/waveform/dashboard 态 | 多文件并行 | 并行 agent |
| **Batch 49** | 动画激活：tooltip / skeleton fix / rotate spinner / bounce_path / elastic_snap / typewriter 接到生产 | 多文件并行 | 并行 agent |
| **Batch 50** | 死代码守护 + token 覆盖率 +严格化 | 测试加强 | 并行 agent |

**永续策略**：每批 ≥ 500 行净增 / 每批必须 commit / 不停止。
