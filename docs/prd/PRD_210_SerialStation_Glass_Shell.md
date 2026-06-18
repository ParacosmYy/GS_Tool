# PRD-210 Serial Station 玻璃卡片三栏 Shell 收口

## 背景

PRD-209 交付了 VOFA+ 精致工业风深色主题子系统（palette/tokens/qss_builder/ThemeManager），`apply_theme`
已在 `app/main.py` 接入。工作区随后出现一个**未收口的三栏 shell WIP**：

- 新增 `python/embeddebug/serial_station/ui/icons.py`（IconManager：SVG 着色 + 缓存）。
- 新增 `python/embeddebug/serial_station/ui/layout_cards.py`（卡片构建器 `build_card` / `wrap_layout`
  / `card_body` + 图标标题行）。
- 新增 `python/embeddebug/serial_station/ui/layout_main.py`（三栏 QSplitter 装配：
  左=连接+命令卡、中=波形+日志卡、右=日志工具+Profile+Clear）。
- `sections.py` 已改写为调用 `layout_main.assemble_three_zone`。

**问题**：WIP 引入了 6 个新 `serialStation*` objectName（`serialStationMainSplitter` /
`serialStationLeftZone` / `serialStationCenterZone` / `serialStationRightZone` /
`serialStationLogCardBody` / `serialStationCardIcon`），但 `qss_builder` 未覆盖，
触发 `test_theme_qss_coverage` 守卫，连锁导致 **24 个 UI smoke 测试红**（`qapp` fixture
在 `apply_theme` 时即断言失败）。

参考竞品 `D:\Tool\Embedded_Tool\EK-OmniProbe`（Tauri+React）的设计语言：玻璃拟态卡片
（大圆角 28–36px、分层阴影、标题行 + 副标题 + badge、折叠卡、分组工具栏、三栏 TopBar+Sidebar+Workspace）。
本次把 WIP 收口并按 EK-OmniProbe 玻璃卡片语言深化视觉。

## 目标

1. **修红基线**：补齐 QSS 覆盖，让 24 个红测试转绿，恢复 commit 门禁。
2. **视觉深化**：把卡片/分栏按 EK-OmniProbe `surface-card` 玻璃卡片语言打磨（大圆角、
   分层阴影、标题行分隔线、图标着色、强调青聚焦），在深色工业风色板上落地。
3. **新增 TopBar**：品牌区 + 状态/Profile 药丸，装配在 `serialStationPyRoot` 顶部、splitter 之上。
4. **可维护性**：所有新文件 ≤300 行，单一职责，palette/tokens 保持唯一真相源，
   action 模块委托边界零改动。

## 范围

### 改动
- `python/embeddebug/serial_station/ui/theme/tokens.py`：新增 `RADIUS_XL` / `RADIUS_2XL` /
  `SHADOW_CARD` / `SHADOW_CARD_HOVER` 字符串 token，`all_tokens()` 同步登记。
- `python/embeddebug/serial_station/ui/theme/qss_sections_layout.py`（**新增**）：
  `splitter_section()`、`zones_section()`、`cards_section()` 三段分区，覆盖全部
  layout objectName + 玻璃卡片样式。
- `python/embeddebug/serial_station/ui/theme/qss_builder.py`：追加三段 import 与 `build_qss()` 调用。
- `python/embeddebug/serial_station/ui/top_bar.py`（**新增**）：`build_top_bar(owner, root)`
  返回 `serialStationTopBar` QFrame，内含品牌区 + 复用 `_status_label` / `_profile_label`。
- `python/embeddebug/serial_station/ui/sections.py`：在 root layout 顶部装配 TopBar（+约 8 行），
  保持 `<260` 行、`SerialStationSectionsHost` Protocol、字面量 builder 调用不变。
- `resources/themes/serial_station_dark.qss`：重新生成（保持外部编辑路径与程序化一致）。

### 测试（新增）
- `tests/python/unit/test_layout_cards.py`：断言 `build_card` / `wrap_layout` / `card_body`
  的 objectName 与 body 取回契约。
- `tests/python/unit/test_qss_sections_layout.py`：断言三个新分区函数返回含全部新 objectName 选择器。
- `tests/python/ui_smoke/test_serial_station_shell_layout.py`：断言三栏 splitter + 三 zone
  + TopBar + 9 个核心 objectName 全部 findChild 可达（不重复既有行为断言，只验布局壳）。
- 既有 `test_theme_qss_coverage.py` / `test_theme_manager.py` 自动覆盖新 objectName（扫描即生效）。

## 非目标

- 不改任何既有 `serialStation*` objectName 文本。
- 不改任何 action 模块（`*_actions.py`）委托边界与文本格式。
- 不动 controller/core/protocols/services 业务逻辑。
- 不引入新第三方依赖。
- 不做命令折叠卡、面板过渡动画、响应式断点、波形游标/图例（留给 B2 批次）。
- 不提升真实设备验证口径（维持 D2 替身口径）。

## 设计决策

### 程序化 QSS 主路径不变
沿用 PRD-209 决策：`qss_builder.build_qss()` 程序化生成为主路径，外部
`resources/themes/serial_station_dark.qss` 为设计期编辑路径。本次扩展新增分区后重新生成外部文件。

### 玻璃卡片在 QSS 中的近似
QSS 无 `backdrop-filter: blur()`，玻璃感通过以下手段近似（对齐 EK-OmniProbe `surface-card`）：
- 大圆角（`RADIUS_2XL=16px`）。
- 分层 `border`（半透明强调边）+ `background-color`（`BG_PANEL`）。
- 卡片标题行下分隔线（`QFrame#serialStationCardHeader` 底边框）。
- 悬浮微抬升（`:hover` 背景提亮到 `BG_PANEL_RAISED` + 边框转 `ACCENT_BORDER`）。
- 图标用 IconManager 着色（强调青 / 次文本色），由 `serialStationCardIcon` 间接体现。

### TopBar 装配点
`serialStationPyRoot` 必须保持是 `setCentralWidget` 目标（`test_pyqt_app_smoke` 硬断言），
所以 TopBar 与 splitter 都装在 root 的 QVBoxLayout 内：TopBar 在顶、splitter 占主体。

## 不可破坏的硬契约

- `sections.py` `<260` 行；含 `class SerialStationSectionsHost(Protocol):`；不含 `Any`；
  含字面量 `build_connection_toolbar(owner, controller, root)` 与 `command_section.build_send_row(owner, root)`。
- `serialStationPyRoot` 仍是 `setCentralWidget` 目标。
- 44 个既有 + 9 个核心 objectName 全部 `findChild` 可达。
- action 模块委托边界与 status/log/profile/stats 文本格式逐字不变。
- 每个 `python/embeddebug/**/*.py` ≤300 行。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 收口 WIP，补齐 QSS 覆盖，新增布局/分区单测，全门禁绿 |
| 用户 | `U3` | 三栏 shell + 玻璃卡片观感落地；连接/发送/日志/波形入口不变 |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实设备未验证 |

## 验收

- `uv run test-embeddebug-py` → 24 红转绿，新增 ~6-8 测试全绿（目标 220+ 全绿）。
- `uv run start-embeddebug --smoke` → 退出码 0。
- `cmd /c EmbedDebug.bat --smoke` → 退出码 0。
- QSS 覆盖率守护测试自动覆盖全部新 objectName（扫描即生效，无需手改测试）。
- `uv run python -c "from embeddebug.serial_station.ui.theme.qss_builder import build_qss as b; print(len(b()))"`
  → 输出 > 1500。

## 风险与回退

- **新 objectName 仍漏覆盖** → QSS 覆盖率守护测试在 `qapp` fixture 即拦截，commit 前必现。
- **外部 QSS 与程序化漂移** → 重新生成外部文件；缺失时 `ThemeManager` 自动回退到 `build_qss`。
- **回退点**：删除 `sections.py` 的 TopBar 装配与 `qss_sections_layout` 调用即恢复纯 splitter 无玻璃态。

## 后续批次（B2，本轮不展开）

- 命令区折叠卡（Collapsible）、面板过渡动画、响应式断点（<900px 折叠右区）。
- 波形游标 / 多通道图例 / 颜色标记设置。
- 分组工具栏（采集 / 查看 / 分析，对齐 EK-OmniProbe `SerialToolbar`）。
