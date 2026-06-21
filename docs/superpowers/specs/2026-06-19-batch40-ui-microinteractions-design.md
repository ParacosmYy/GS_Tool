# Batch 40 — UI 微交互补强（10 个并行新增）

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 1/∞
> 来源：用户目标（`/goal`）「永远迭代下去 / 10 个 subagent 并行 / UI 美化 + 组件 + 动画 + 比例 + 工具」
> 关联：[02-workflow.md](../../constraints/02-workflow.md) §3 并行开发；[05-ui-standard.md](../../constraints/05-ui-standard.md) §六 动画服务信息传达
> 评分目标：665 → 666（+1）

---

## 1. 目标

为「世界第一嵌入式调试工具」补齐 UI 微交互层基础设施。10 个独立新增模块，覆盖：
- **5 个新动画**：bounce_path / glow / rotate / typewriter / elastic_snap
- **5 个新控件**：chip / segmented / rich_tooltip / info_banner / progress_ring

每个模块完全独立：自己的源码文件、自己的测试文件、不修改任何既有文件（包括 `__init__.py`）。
合流阶段由主线统一更新 2 个 `__init__.py`，跑 pytest，跑 `--smoke`，commit。

## 2. 并行边界（铁律 10.6 切边界）

| Agent | 唯一写文件 | 唯一测试文件 | 依赖 | 风险 |
|-------|------------|--------------|------|------|
| A1 bounce_path | `animations/bounce_path.py` | `tests/python/unit/test_bounce_path.py` | tokens / scale 既有 | 无 |
| A2 glow | `animations/glow.py` | `tests/python/unit/test_glow.py` | tokens / palette | 无 |
| A3 rotate | `animations/rotate.py` | `tests/python/unit/test_rotate.py` | tokens | 无 |
| A4 typewriter | `animations/typewriter.py` | `tests/python/unit/test_typewriter.py` | tokens | 无 |
| A5 elastic_snap | `animations/elastic_snap.py` | `tests/python/unit/test_elastic_snap.py` | tokens / scale | 无 |
| C1 chip | `controls/chip.py` | `tests/python/unit/test_chip.py` | palette / fade | 无 |
| C2 segmented | `controls/segmented.py` | `tests/python/unit/test_segmented.py` | palette / slide | 无 |
| C3 rich_tooltip | `controls/rich_tooltip.py` | `tests/python/unit/test_rich_tooltip.py` | palette / fade | 无 |
| C4 info_banner | `controls/info_banner.py` | `tests/python/unit/test_info_banner.py` | palette / slide | 无 |
| C5 progress_ring | `controls/progress_ring.py` | `tests/python/unit/test_progress_ring.py` | palette / tokens | 无 |

**绝对禁止**：任何 agent 修改 `__init__.py`、`tokens.py`、`palette.py`、`pyproject.toml`、`uv.lock`、`EmbedDebug.bat`、`SCORE_TRACKING.md`，或彼此的文件。所有共享读，0 共享写。

## 3. API 契约（每个 agent 必须严格遵守）

每个新增模块遵循现有 `scale.py` / `ripple.py` 的范式：

- 模块顶部：模块 docstring 说明职责（必要中文注释，铁律 13）
- 使用 `from __future__ import annotations`
- 仅 `import` 标准库 / PyQt6 / 项目内 `embeddebug.serial_station.ui.animations.tokens` / `embeddebug.serial_station.ui.theme.palette as P`
- 公共类必须有 `setObjectName(...)`（铁律 16）
- 公共类暴露 classmethod / staticmethod 工厂返回 `QPropertyAnimation`，并通过 `_track()` 风格机制防 GC
- 文件 ≤ 300 行（铁律 20），单方法 ≤ 80 行（铁律 22）
- 测试文件聚焦单一行为域（铁律 21），用 `pytest` + `QApplication` fixture（参考既有 `test_scale_press.py`）

每个测试至少覆盖：
1. 基础 API 调用不抛异常（smoke）
2. 关键属性在动画结束后处于期望终态（终点断言）
3. 动画对象注册到 GC 防护列表，完成后被移除（生命周期断言）

## 4. 10 个子任务的行为契约

### A1 `bounce_path.py` — 弹跳路径动画
- `BouncePathAnimation.drop_in(widget)`：从 widget 上方 -50px 偏移 + 0.6 scale，弹跳到原位
- 实现：QPropertyAnimation 双 `geometry` keyframe（先落到位再回弹）+ OutBounce easing
- 时长：`DURATION_SLOW`（360ms）

### A2 `glow.py` — 脉冲发光
- `GlowEffect`：QGraphicsEffect 子类（实际通过 QGraphicsDropShadowEffect 派生或自绘）做 accent 色 glow
- `GlowEffect.pulse(widget)`：blurRadius 在 NORMAL..HOVER 间脉冲，循环 3 次
- 用于：连接成功提示、错误高亮

### A3 `rotate.py` — 旋转动画
- `RotateAnimation.spin(widget, loops=1)`：以 widget 中心为锚 0° → 360°×loops
- 通过 `QPropertyAnimation(widget, b"rotation")` + QWidget 自定义旋转属性（paintEvent 重写）
- 或者更稳妥：返回 `QVariantAnimation`，调用方接 `valueChanged` 自行重绘
- 用于：刷新按钮、加载 spinner

### A4 `typewriter.py` — 打字机文字
- `TypewriterAnimation.run(label, text, cps=30)`：逐字符 reveal 到 QLabel/控件
- `valueChanged(str)` 信号驱动 `label.setText(...)`
- 时长：根据 `len(text) / cps * 1000` 自动计算

### A5 `elastic_snap.py` — 弹性归位
- `ElasticSnapAnimation.snap_to(widget, target_rect)`：从当前 geometry 弹性吸附到 target_rect
- Easing：`EASE_OUT_ELASTIC`；时长 `DURATION_SLOWER`
- 用于：拖拽释放、面板 magnet 吸附

### C1 `chip.py` — Material Chip 标签
- `Chip(text, removable=True)`：圆角胶囊标签，关闭按钮（×）发 `removed` 信号
- 选中态可切换（clickable），发 `toggled(bool)` 信号
- 自绘 paintEvent + hover 颜色过渡

### C2 `segmented.py` — 分段控件
- `SegmentedControl(options: list[str])`：iOS 风格分段选择器
- 选中段滑块动画（slide indicator），发 `currentChanged(int)` 信号
- 默认第 0 段选中

### C3 `rich_tooltip.py` — 富文本 tooltip
- `RichTooltip(title, body, icon=None)`：标题 + 正文两行 tooltip
- 显示/隐藏带 fade 过渡（`DURATION_INSTANT`）
- `install_on(widget)` 静态方法挂接到任意 QWidget 的 `entered`/`left` 事件

### C4 `info_banner.py` — 可关闭信息条
- `InfoBanner(text, kind="info")`：info/warning/error/success 四种 kind
- 右侧关闭按钮（×），点关闭发 `dismissed` 信号 + slide-out 动画
- 顶部 accent 条颜色随 kind 变化

### C5 `progress_ring.py` — 环形进度
- `ProgressRing(minimum=0, maximum=100)`：自绘环形进度条
- `setValue(int)` 触发 0→value 的平滑插值动画（DURATION_NORMAL）
- 支持 indeterminate 模式（`setIndeterminate(True)`，环旋转）

## 5. 验收命令

```powershell
# 1. 单元测试（10 个新文件）
uv run pytest tests/python/unit/test_bounce_path.py tests/python/unit/test_glow.py `
  tests/python/unit/test_rotate.py tests/python/unit/test_typewriter.py `
  tests/python/unit/test_elastic_snap.py tests/python/unit/test_chip.py `
  tests/python/unit/test_segmented.py tests/python/unit/test_rich_tooltip.py `
  tests/python/unit/test_info_banner.py tests/python/unit/test_progress_ring.py -v

# 2. 全量 pytest 不退化
uv run pytest tests/python/unit -x --tb=short

# 3. smoke 验证（铁律 5）
uv run start-embeddebug --smoke
```

## 6. 三轴状态（铁律 5.9）

| 模块 | 工程 | 用户 | 设备 |
|------|------|------|------|
| 5 动画 + 5 控件（API + 测试） | E2 骨架 | U1 可见不可用 | D1 纯单测 |

> 用户态 U1：本批仅提供 API + 单测，未接入主面板。下一批（Batch 41）选 2-3 个接入实际面板，U1→U2/U3。

## 7. 合流顺序（铁律 10.6）

1. 主线在所有 agent 完成后**串行**更新：
   - `python/embeddebug/serial_station/ui/animations/__init__.py`（追加 5 个 import + 5 个 `__all__`）
   - `python/embeddebug/serial_station/ui/controls/__init__.py`（追加 5 个 import + 5 个 `__all__`）
2. 主线运行 10 个新测试 + 全量 pytest
3. 主线运行 `--smoke`
4. 全绿后 commit，commit message 按项目模板（评分 +1）
5. 立即进入 Batch 41（永续）

## 8. 永续迭代策略（用户目标）

每批结束后自动进入下一批，候选池（按用户关键词优先级）：

- **UI 美化**：色板/阴影/排版细化、Lucide 图标继续扩展到 100+、QSS 主题生成器自动化
- **组件开发**：抽屉 Drawer、命令面板增强、对话框系统统一（替换 QMessageBox）
- **过渡动画**：页面切换 slide、loading skeleton、连接状态脉冲
- **UI 比例协调**：DPI 感知、density 切换器、响应式断点补强
- **各种工具**：CRC 计算器、Hex 编辑器、时间戳转换、字节频率分析

每批 10 个并行 agent，每批 ≥500 行净增（铁律 3），每批必须 commit（铁律 5.10）。
