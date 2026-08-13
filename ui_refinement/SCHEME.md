# QuillForge UI 美化 · 1000 轮渐进式迭代方案（SCHEME）

> 目标项目：`D:\Workplace\Agent_Workplace\QuillForge`（PyQt6 + QScintilla 桌面编辑器）
> 美化载体：集中式 QSS 设计系统 `presentation/theme.py` + `presentation/theme_tokens.py`
> 执行方式：**将"1000 轮 UI 美化"建模为对"设计令牌空间"的可度量优化过程**
> —— 每一轮 = 一次"提案 → 评分 → 接受（仅当更优）"的闭环，保证在第 *N* 轮结果上单调不降地提升美观度与一致性。

---

## 1. 为什么用"优化引擎"而非手工 1000 遍

手工做 1000 次独立设计修改既不可复现，也无法保证"每轮都在前一轮基础上提升"。
把 UI 美化转译为**可度量的优化问题**后：

- **设计令牌（颜色 / 几何 / 排版）** 是连续可调的参数空间；
- **美观度与一致性** 被拆成 6 个可量化维度，合成一个 `0–100` 复合评分；
- **1000 轮 hill-climbing** 每轮只扰动一个参数，若复合分更高才接受 —— 自然满足
  "每一轮都在前一轮基础上提升"（报告中的 *best-so-far* 单调非降）。

这与项目既有 `quillforge-ui-visual-quality` 技能的设计准则完全一致（surface ladder、
对比度 ≥ 4.5:1、状态矩阵、动效尊重、排版层级），并把它从"人工检查清单"升级为"自动闭环"。

---

## 2. 迭代目标（Iteration Goals）

| # | 目标 | 含义 |
|---|------|------|
| G1 | 视觉层级清晰 | 画布 / 面板 / 卡片 / 浮层四个层级在明度上可分辨，不靠装饰 |
| G2 | 配色和谐且可达 | 三套主题文本对比度 ≥ 4.5:1，语义色对比 ≥ 3:1，accent 家族色相协调 |
| G3 | 布局节奏一致 | 间距 / 圆角遵循统一模块化比例（spacing ≈ 1.5×、radius 阶梯一致） |
| G4 | 排版成体系 | 字号走模块化比例（≈1.2），权重 ≤ 4 档，行高稳定 |
| G5 | 组件状态完整 | hover / pressed / focus / checked / selected / disabled / readonly / warning / error 九态齐备且可分辨 |
| G6 | 交互反馈明确 | 焦点环可见（对比 ≥ 3:1），动效短且可被 `motion_enabled` 关闭 |

---

## 3. 六维评估标准（Rubric · 每维 0–1 → 加权合成 0–100）

权重：`视觉 .18 · 布局 .14 · 配色 .26 · 排版 .14 · 组件 .16 · 交互 .12`
（配色权重最高，因为它同时承载可达性与一致性；视觉/组件次之。）

### 3.1 视觉呈现 `visual`（.18）
- **surface ladder 分离度**：`surface_0→1→2→3→hover` 明度单调且每级间隔 ≥ 目标值；
  评分 = `clamp(平均间隔 / 目标间隔)`。
- **accent 鲜度**：accent 饱和度落在甜区 `[0.40, 0.85]`，过灰或过刺均扣分。

### 3.2 布局结构 `layout`（.14）
- **间距一致性**：`spacing_xs/sm/md/lg` 实际比值与统一比例（≈1.5）的接近度。
- **圆角一致性**：`radius_md ≈ 1.4×radius_sm`、`radius_lg ≈ 1.6×radius_md` 的接近度。

### 3.3 配色方案 `color`（.26）
- **文本对比**：三主题下 `text_primary/surface_0`、`text_secondary/surface_1`、
  `text_muted/surface_2` 的最小对比度，目标 ≥ 4.5（硬下限 3.0）。
- **语义对比**：`on_accent/accent`、`success/danger` 前景/背景、`accent_alt_text/surface_3` ≥ 阈值。
- **色相和谐**：accent 与 accent_alt 互补、pink/gold 暖色近似，色相跨度落在 `[40°,180°]`。

### 3.4 字体排版 `typography`（.14）
- **字号比例**：正文→标题模块化比例落于 `[1.15, 1.30]`，中心 1.2 最佳。
- **行高稳定**：`line_height_ratio ∈ [1.40, 1.60]`。
- **权重克制**：使用 ≤ 4 档字重（静态优势，保持一致）。

### 3.5 组件样式 `components`（.16）
- **状态齐备度**：令牌集能支撑九态且彼此可分辨（`pressed` 与 `surface_hover` 对比 ≥ 2，
  `danger` 与 `accent` 可区分）。
- **边框对比**：`border/surface`、`border_strong/surface` ≥ 3:1。

### 3.6 交互反馈 `interaction`（.12）
- **焦点环可见**：焦点色（accent_alt）在 `surface_hover` 上的对比 ≥ 3:1（越高越好）。
- **动效尊重**：存在 `motion_enabled` 开关且 `transition_ms ∈ [120,260]`，关闭后无动画。
- **时序一致**：所有过渡共用同一 `transition_ms`。

---

## 4. 1000 轮执行结构（Phases）

| 阶段 | 轮次 | 焦点 | 动作 |
|------|------|------|------|
| P0 基线 | 0 | 载入三套基础主题 + 几何令牌，记录初始分 | 冻结起点 |
| P1 对比收敛 | 1–350 | `color` 维度 | 调 accent/文本/语义色，把最小对比推过 4.5 并逼近 7 |
| P2 层级与节奏 | 351–650 | `visual`+`layout` | 拉开 surface 阶梯、对齐 spacing/radius 比例 |
| P3 状态与反馈 | 651–900 | `components`+`interaction` | 补齐九态可分辨度、焦点环、动效时序 |
| P4 全局精修 | 901–1000 | 全维一致性 | 小幅退火，抬升最弱主题，锁定 best-so-far |

每轮算法（`refine.py`）：
```
round:
    param = random_choice(可优化参数)
    candidate = param + uniform(-step, +step)   # 在 [min,max] 内
    score_new = composite(resolve(candidate))
    if score_new > best_score + ε:
        accept; best_score = score_new
    else with prob p(anneal): accept neutral move (逃逸平台，但不更新 best)
    log(round, param, best_score, per-dim scores)   # best 单调非降
```

---

## 5. 收敛判据（Convergence）

- **主判据**：`best_score` 在最后 100 轮内提升 < 0.1 分（百分制 < 0.1）。
- **可达性硬约束**：任一主题文本最小对比度 **不得低于 4.5:1**（违例直接重罚并拒绝）。
- **一致性约束**：`composite = 0.5·mean(各主题分) + 0.5·min(各主题分)`，强制最弱主题也被拉起。
- 达标即输出 `optimized_tokens.json` + `REPORT.md`；未达则在报告中说明仍可继续 `refine.py`。

---

## 6. 交付物（Deliverables）

| 文件 | 内容 |
|------|------|
| `ui_refinement/SCHEME.md` | 本方案（目标 / 六维 rubric / 阶段 / 收敛） |
| `ui_refinement/refine.py` | 1000 轮自动优化引擎（可复跑） |
| `ui_refinement/convergence.csv` | 每轮 best 复合分 + 六维分 |
| `ui_refinement/optimized_tokens.json` | 1000 轮后最优令牌集 |
| `ui_refinement/REPORT.md` | 收敛证明 + 前后对比 + 评分 |
| `src/.../theme_tokens.py`（增量） | 新增 `aurora-refined` 主题、色相旋转 accent 色调、`UiGeometryTokens` |
| `src/.../theme_enhancements.py`（新增） | 增强 QSS 覆盖层（九态矩阵 / 焦点环 / 层级 / 排版 / 动效尊重）+ `apply_theme_enhanced` |
| 静态校验 | `ruff` + `compileall` + 对比度探针（不实例化 `QApplication`） |

> 原有三套主题与全部调用方（`composition.py`、`main_window.py`）**保持不变**；
> 优化结果以新增主题 + 可选增强层形式落地，可一键回退。
