# PRD-209 Serial Station Theme Manager

## 目标

把 VOFA+ 精致工业风深色主题真正落地到 Python/PyQt Serial Station UI。
当前 `resources/themes/` 有 7246 行 QSS 但 Python 应用一行都没加载，
且现有 QSS 的 objectName 选择器与 Python 实际用的（`serialStation*` 前缀）完全不匹配，
UI 一直停留在 Qt 默认样式。本轮引入主题加载基础设施 + Python-first QSS 生成器，
覆盖全部 `serialStation*` objectName，对齐 VOFA+ 深色工业风观感。

## 范围

- 新增 `python/embeddebug/serial_station/ui/theme/` 子包：
  - `palette.py`：从 `modern_dark.qss` PRD-071 工业风分区提取的色板常量（单一真相源）。
  - `tokens.py`：尺寸/圆角/间距/字号 token。
  - `qss_builder.py`：程序化生成完整 QSS 的协调入口。
  - `qss_sections_core.py`：全局/窗口/标签/输入/下拉分区。
  - `qss_sections_widgets.py`：按钮/日志/波形/状态/滚动条/快捷键分区。
  - `manager.py`：`ThemeManager` 单例，负责 QSS 加载、应用、运行时切换与资源定位。
  - `__init__.py`：主题子系统公共入口。
- 新增 `resources/themes/serial_station_dark.qss`：外部 QSS 参考源（设计期编辑路径）。
- 改动 `python/embeddebug/app/main.py`：`create_application` 末尾调用 `apply_theme(app)`（最小侵入 +2 行）。
- 新增测试 `tests/python/unit/test_theme_manager.py` 与 `test_theme_qss_coverage.py`。

## 非目标

- 不改任何 `serialStation*` objectName（冒烟测试硬断言 9 个）。
- 不改任何 action 模块委托边界。
- 不动 controller/core/protocols/services 业务逻辑。
- 不做布局重排（留给 B2 批次）。
- 不引入新第三方依赖。
- 不提升真实设备验证口径。

## 设计决策

### 程序化 QSS 优先，外部 QSS 为编辑路径

- 主路径：`qss_builder.build_qss()` 程序化生成，零文件依赖、PyInstaller 打包无忧。
- 编辑路径：`resources/themes/serial_station_dark.qss` 存在时 `ThemeManager.load_qss` 优先加载，
  便于设计期手改 QSS 调试；缺失或读取失败回退到程序化生成。
- 资源定位 `resolve_resource_path` 兼容 dev（仓库根回溯）与 PyInstaller onedir
  （`sys._MEIPASS` / 可执行文件目录）。

### 色板来源

取自 `resources/themes/modern_dark.qss` 的 PRD-071 工业风分区（lines 2145+），
对齐 VOFA+ 深色工业风观感：深蓝灰底（`#0d1118`/`#101218`/`#151b24` 分层）+
强调青（`#22d3ee`）+ 终端 RX 绿（`#22c55e`）/TX 蓝（`#38bdf8`）分色。

### 文件拆分

单文件 QSS 自然较长，受仓库运行时文件 `<= 300 行` 门禁约束（`test_python_default_cutover.py`），
按域拆为 `qss_sections_core` 与 `qss_sections_widgets`，`qss_builder` 仅做协调。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | theme/ 子包落地，27 个新单测覆盖色板/QSS/加载/应用/资源解析，全部门禁通过 |
| 用户 | `U3` | 主入口启动即生效深色工业风主题；连接按钮三态、日志区终端配色、状态药丸可见 |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实设备未验证 |

## 验收

- `uv run test-embeddebug-py` → 215 测试全绿（188 既有 + 27 新增）。
- `uv run start-embeddebug --smoke` → 退出码 0。
- `cmd /c EmbedDebug.bat --smoke` → 退出码 0。
- `uv run python -c "..."` 验证 `app.styleSheet()` 含 `#serialStationConnectButton` 与强调色。
- QSS 覆盖率守护测试：扫描 ui/ 源码全部 `serialStation*` objectName，断言 QSS 含对应选择器。

## 架构检查

- `theme/` 只依赖 PyQt6 + 标准库 + 本包内 palette/tokens/qss_sections_*，
  不 import controller/core/protocols/services。
- `main.py` 仅 +2 行调用，不做业务编排。
- 不动 `sections.py`（保持 < 260 行门禁余量）。
- 不动任何 action 模块和 objectName。

## 风险与回退

- **QSS 资源 PyInstaller 打包后路径失效** → `resolve_resource_path` 多候选回退；
  程序化生成是打包期唯一可靠路径；外部文件缺失自动回退。
- **回退点**：任一门禁失败，删除 `main.py` 的 `apply_theme` 调用即恢复无主题状态（零业务影响）。

## 后续批次

- **B2**：主窗口三区分栏布局（QSplitter）+ 图标接入（IconManager + lucide SVG）+ 卡片化面板。
- **B3**：面板过渡动画、波形游标/多通道图例、响应式断点、命令面板。
