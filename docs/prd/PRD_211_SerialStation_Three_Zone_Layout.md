# PRD-210 Serial Station 三区分栏布局 + 图标接入 + 卡片化面板

## 目标

把 B1 之后仍是单列堆叠 + 一行塞 18 个控件的 MVP 主窗口重排为 VOFA+ 风格的
三区分栏布局（QSplitter：左连接配置卡 / 中波形+日志主区 / 右命令/Profile 卡），
接入 IconManager + lucide SVG 给按钮加图标，实现卡片化面板。
这是达成"比 VOFA+ 精美"的核心视觉要素。

## 范围

- 新增 `python/embeddebug/serial_station/ui/icons.py`：IconManager 单例，
  用 QSvgRenderer 加载 `resources/icons/lucide/*.svg`，按 palette 着色后转 QIcon，
  缓存到 `(name, color)` 键。
- 新增 `python/embeddebug/serial_station/ui/layout_cards.py`：卡片化面板构建器。
  `serialStationCard` 是 QFrame（圆角+边框+背景），含可选标题行（图标+标题）与主体；
  `wrap_layout` 把现有 layout builder 包进卡片，`card_body` 按 objectName 取主体。
- 新增 `python/embeddebug/serial_station/ui/layout_main.py`：三区分栏装配。
  `assemble_three_zone` 接收 sections 用字面量构建的各 layout，包进 6 张卡片：
  左区 Connection + Command，中区 Waveform + Log，右区 Log Tools + Profile + Clear。
- 新增 `python/embeddebug/serial_station/ui/button_icons.py`：按 objectName 装饰按钮图标，
  连接类用强调青、断开用错误红、其余用次文本色，与 QSS 三态视觉一致。
- 重构 `sections.build_main_layout`：保留字面量委托（`build_connection_toolbar(owner, controller, root)`
  与 `command_section.build_send_row(owner, root)`），把返回的 layout 交给 layout_main 装配，
  log_view 注入中区日志卡主体。sections.py 保持 183 行（< 260 门禁）。
- 改动 `main_window.py`：构建后调用 `button_icons.apply_button_icons(self)`，窗口默认尺寸
  由 960x640 调到 1180x720 以容纳三区。
- QSS 新增 `cards_zones_section`：serialStationCard/Header/Title/Icon、三区透明容器、
  splitter 把手；程序化生成器与外部参考源同步。

## 非目标

- 不改任何 `serialStation*` objectName（冒烟测试硬断言 9 个仍可达）。
- 不改任何 action 模块委托边界（架构测试全绿）。
- 不动 controller/core/protocols/services 业务逻辑。
- 不做面板过渡动画、波形游标、响应式断点（留给 B3）。
- 不引入新第三方依赖。

## 设计决策

### sections 保持字面量委托

架构测试 `test_connection_toolbar_builder_lives_outside_main_sections` 要求
`sections.build_main_layout` 源码含字面量 `build_connection_toolbar(owner, controller, root)`
且不含 `serialStationProtocolCombo`；`test_command_row_builder_lives_with_command_section`
要求含 `command_section.build_send_row(owner, root)` 且 sections 源码不含 `serialStationSendEdit`。
因此 sections 先用字面量调用 builder 拿到 layout，再交给 layout_main 装配，
避免在 sections 里直接持有控件细节。

### 卡片主体按 objectName 定位

`build_card` 给主体 layout 设 objectName `serialStationCardBody`，
`card_body(card)` 按该名查找，避免依赖 header 是否存在导致的索引漂移。

### 图标着色复用 palette

IconManager 把 SVG 的 `currentColor` 与裸 `stroke="..."` 替换为目标颜色，
结果缓存。连接类主按钮用 `TEXT_ON_ACCENT`（深底），与 QSS 青色按钮文字一致。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 三区布局 + 卡片 + 图标落地，17 个新单测覆盖结构/图标/装饰，全门禁通过 |
| 用户 | `U3→U4` | 主入口启动即呈现三区卡片化布局，按钮带 lucide 图标，分区信息密度对齐 VOFA+ |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实设备未验证 |

## 验收

- `uv run test-embeddebug-py` → 232 测试全绿（215 既有 + 17 新增）。
- `uv run start-embeddebug --smoke` → 退出码 0。
- `cmd /c EmbedDebug.bat --smoke` → 退出码 0。
- QSS 覆盖率守护测试自动覆盖新增 objectName（serialStationCard/zone/splitter）。
- 架构测试全绿（sections<260、字面量委托、action 边界不变）。

## 架构检查

- 新增模块（icons/layout_cards/layout_main/button_icons）只依赖 PyQt6 + 标准库 + theme，
  不 import controller/core/protocols/services。
- sections.py 保持主 layout 编排 + 字面量委托，不承接控件细节。
- main_window 仅增加 1 行 button_icons 调用，不做业务编排。
- 所有 `serialStation*` objectName 保留，冒烟测试 9 个 critical 仍可达。

## 后续批次

- **B3**：面板过渡动画、波形游标/多通道图例、响应式断点（<900 折叠侧栏）、命令面板。
