# Batch 41 — UI 组件 + 工具补强（5 个并行新增）

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 1/∞
> 来源：用户目标「永远迭代下去 / 10 个 subagent 并行 / UI 美化 + 组件 + 动画 + 比例 + 工具」
> 关联：[02-workflow.md](../../constraints/02-workflow.md) §3 并行开发；[05-ui-standard.md](../../constraints/05-ui-standard.md)
> 评分目标：666 → 667（+1）
> 范围调整：本批缩至 5 个模块（Batch 40 已落地 10 个），优先补「工具」+「常用 UI 组件」缺口

---

## 1. 目标

为「世界第一嵌入式调试工具」补齐工具面板和高频 UI 组件缺口。5 个独立新增模块：

- **2 个 UI 组件**：Drawer（侧边抽屉）/ Badge（状态徽章）
- **2 个动画**：Skeleton（加载骨架屏闪烁）/ PageSlide（页面切换滑入）
- **1 个工具**：CrcCalculator（CRC 计算器面板）

每个模块完全独立：自己的源码文件、自己的测试文件、不修改任何既有文件（包括 `__init__.py`）。
合流阶段由主线统一更新 `__init__.py`、QSS、SCORE_TRACKING，跑 pytest，commit。

## 2. 并行边界（铁律 10.6 切边界）

| Agent | 唯一写文件 | 唯一测试文件 | 依赖 | 风险 |
|-------|------------|--------------|------|------|
| D1 Drawer | `ui/controls/drawer.py` | `tests/python/unit/test_drawer.py` | palette / fade / slide | 无 |
| D2 Badge | `ui/controls/badge.py` | `tests/python/unit/test_badge.py` | palette / tokens | 无 |
| A1 Skeleton | `ui/animations/skeleton.py` | `tests/python/unit/test_skeleton.py` | tokens / palette | 无 |
| A2 PageSlide | `ui/animations/page_slide.py` | `tests/python/unit/test_page_slide.py` | tokens / slide | 无 |
| T1 CrcCalculator | `ui/tools/crc_calculator.py` | `tests/python/unit/test_crc_calculator.py` | palette / tokens | 无 |

**绝对禁止**：任何 agent 修改 `__init__.py`、`tokens.py`、`palette.py`、`qss_sections_*.py`、`pyproject.toml`、`uv.lock`、`EmbedDebug.bat`、`SCORE_TRACKING.md`，或彼此的文件。所有共享读，0 共享写。

## 3. API 契约（每个 agent 必须严格遵守）

新增模块遵循现有 Batch 40 范式（参考 `chip.py` / `info_banner.py` / `bounce_path.py`）：

- 模块顶部：模块 docstring 说明职责（必要中文注释，铁律 13）
- 使用 `from __future__ import annotations`
- 仅 `import` 标准库 / PyQt6 / 项目内 `embeddebug.serial_station.ui.animations.tokens` / `embeddebug.serial_station.ui.theme.palette as P`
- 公共类必须有 `setObjectName(...)`（铁律 16）
- 文件 ≤ 300 行（铁律 20），单方法 ≤ 80 行（铁律 22）
- 测试文件聚焦单一行为域，≤ 250 行（铁律 21），用 `pytest` + `QApplication` fixture
- 信号用 `pyqtSignal`，工厂方法返回 `QPropertyAnimation` 并通过 `_track()` 风格机制防 GC
- 用户可见文字走 `tr()`（铁律 19）

每个测试至少覆盖：
1. objectName 合规（QSS 依赖）
2. 基础 API 调用不抛异常（smoke）
3. 关键属性在动画结束后处于期望终态（终点断言）
4. paintEvent 不抛异常（自绘控件）

## 4. 5 个子任务的行为契约

### D1 `drawer.py` — 侧边抽屉
- `Drawer(side=Qt.LeftEdge, parent=None)`：从屏幕左/右/上/下边缘滑入的面板容器
- `set_content(widget)`：替换内容控件
- `open()` / `close()`：带 fade + slide 动画（参考 `info_banner._animate_out` 的绑定方法范式避免 access violation）
- `isOpen()` 状态查询；`opened(bool)` / `closed(bool)` 信号
- 默认宽度 280px（左/右）或默认高度 200px（上/下）
- 半透明遮罩（点击关闭），用 `QPainter` 自绘

### D2 `badge.py` — 状态徽章
- `Badge(text="", parent=None)`：小数字/文字徽章，自绘
- 支持 kind: `info` / `warning` / `error` / `success`（复用 `InfoBanner.KIND_COLORS` 范式）
- `set_text(str)` / `set_kind(BadgeKind)`；`text()` / `kind()` getter
- sizeHint 默认 24×24（带 padding 自适应文字宽度）
- paintEvent 不抛异常；hover 不切换状态（纯展示控件）

### A1 `skeleton.py` — 加载骨架屏闪烁
- `SkeletonAnimation.shimmer(widget, loops=3)`：widget 的 `pos` 属性做 shimmer 位移动画
- 时长 `DURATION_SLOWER`，循环 `loops` 次，缓动 `EASE_IN_OUT`
- 通过类级 `_active` 列表防 GC（参考 `BouncePathAnimation._track`）
- 完成后从 `_active` 移除

### A2 `page_slide.py` — 页面切换滑入
- `PageSlideAnimation.slide_in(widget, direction=SlideDirection.LEFT)`：从指定方向滑入
- 时长 `DURATION_NORMAL`，缓动 `EASE_OUT_QUART`
- 支持 4 个方向：LEFT / RIGHT / TOP / BOTTOM
- `SlideDirection` 枚举类（`enum.Enum`）
- 通过类级 `_active` 列表防 GC

### T1 `crc_calculator.py` — CRC 计算器面板
- `CrcCalculatorPanel(parent=None)`：自包含 CRC 计算工具面板（QWidget）
- 输入：QTextEdit 输入 hex 字节序列（空格/换行分隔）或 ASCII 文本（radio 切换）
- 配置：CRC 宽度（8/16/32）下拉、多项式 hex 输入、初始值 hex 输入、输入/输出反射 checkbox、异或输出 hex 输入
- 输出：QLineEdit 显示计算结果（hex），只读
- 复制按钮（复制结果到剪贴板）
- **算法内核独立**：`compute_crc(data: bytes, width: int, poly: int, init: int, ref_in: bool, ref_out: bool, xor_out: int) -> int` 是模块级纯函数，便于单测；UI 只是 wrapper
- 预设：CRC-8/MAXIM、CRC-16/MODBUS、CRC-32/ISO-HDLC（4 个 preset 按钮快速填表）
- 用 `objectName="serialStationCrcCalculator*"` 命名所有子控件（QSS 依赖）

## 5. 验收命令

```powershell
# 1. 单元测试（5 个新文件）
uv run pytest tests/python/unit/test_drawer.py tests/python/unit/test_badge.py `
  tests/python/unit/test_skeleton.py tests/python/unit/test_page_slide.py `
  tests/python/unit/test_crc_calculator.py -v

# 2. 全量 pytest 不退化
uv run pytest tests/python/unit -x --tb=short

# 3. smoke 验证（铁律 5）
uv run start-embeddebug --smoke
```

## 6. 三轴状态（铁律 5.9）

| 模块 | 工程 | 用户 | 设备 |
|------|------|------|------|
| 2 组件 + 2 动画 + 1 工具（API + 测试） | E2 骨架 | U1 可见不可用 | D1 纯单测 |

> 用户态 U1：本批仅提供 API + 单测，未接入主面板。下一批（Batch 42）选 2-3 个接入实际面板，U1→U2/U3。

## 7. 合流顺序（铁律 10.6）

1. 主线在所有 agent 完成后**串行**更新：
   - `python/embeddebug/serial_station/ui/controls/__init__.py`（追加 Drawer、Badge import + `__all__`）
   - `python/embeddebug/serial_station/ui/animations/__init__.py`（追加 SkeletonAnimation、PageSlideAnimation、SlideDirection import + `__all__`）
   - 新建 `python/embeddebug/serial_station/ui/tools/__init__.py`（追加 CrcCalculatorPanel）
   - `python/embeddebug/serial_station/ui/theme/qss_sections_controls.py`（追加 5 个新 objectName 的 QSS 占位）
2. 主线运行 5 个新测试 + 全量 pytest
3. 主线运行 `--smoke`
4. 全绿后 commit，commit message 按项目模板（评分 +1）
5. 立即进入 Batch 42（永续）

## 8. 永续迭代策略（用户目标）

每批结束后自动进入下一批，候选池（按用户关键词优先级）：
- **UI 美化**：Lucide 图标继续扩展到 100+、QSS 主题生成器自动化
- **组件开发**：对话框系统统一（替换 QMessageBox）、CommandPalette 增强
- **过渡动画**：连接状态脉冲、页面切换 slide
- **UI 比例协调**：DPI 感知、density 切换器、响应式断点补强
- **各种工具**：Hex 编辑器、时间戳转换、字节频率分析

每批 ≥500 行净增（铁律 3），每批必须 commit（铁律 5.10）。
