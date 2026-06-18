"""Serial Station VOFA+ 精致工业风深色色板。

单一真相源：所有 UI 颜色必须通过本模块的常量引用，禁止在控件逻辑中硬编码颜色。
色板取自 ``resources/themes/modern_dark.qss`` 的 PRD-071 工业风分区，
对齐 VOFA+ 深色工业风观感（深蓝灰底 + 青色强调 + 终端 RX/TX 分色）。

本批次（Batch 2 / A2）的深度改进：
1. **拉开深度层次**：BG_PANEL 由 ``#151b24`` 提亮到 ``#1a2230``，与 BG_WINDOW
   ``#0d1118`` 的亮度差从 ~3% 拉到 ~8%，配合新增的 elevation token，卡片真正「浮」起来。
2. **elevation token 体系**：新增 ``SHADOW_CARD`` / ``SHADOW_POPOVER`` / ``SHADOW_MODAL``
   三档投影色 + ``CARD_GLOW``（accent tint 投影），供 QGraphicsDropShadowEffect 与
   hover_lift 使用，建立 Linear/Vercel 级的多层 z 轴深度。
3. **accent gradient**：新增 ``ACCENT_GRADIENT_FROM`` / ``ACCENT_GRADIENT_TO``（青→蓝
   跨色相渐变），主按钮/激活态/品牌徽标走渐变而非纯色，打破单一色相单调感。
4. **删除死代码**：``sync_to_palette`` / ``reset_to_dark`` / ``_DARK_DEFAULTS`` 无任何
   外部调用方（已 grep 确认），连同 ``palette_defs.py`` 一并清除，消除双轨混乱。

约束：本模块只依赖标准库，不 import PyQt 或其它项目层，便于在测试与
QSS 生成器中独立复用。
"""

from __future__ import annotations


# ── 背景分层 ────────────────────────────────────────────────────────
# 从最深到最浅，用于窗口底、面板底、控件底、悬停态分层。
# BG_WINDOW 锁定 #0d1118（test_palette_constants_match_modern_dark_industrial_palette 硬断言）。
# BG_PANEL 提亮到 #1a2230（原 #151b24），与 BG_WINDOW 亮度差 ~8%，卡片浮起。
BG_WINDOW = "#0d1118"        # QMainWindow / 应用最底层（硬断言锁定）
BG_APP = "#101218"           # 应用容器层
BG_PANEL = "#1a2230"         # 卡片/面板底（提亮，拉开与窗口底的深度差）
BG_PANEL_RAISED = "#222d3e"  # 悬停/凸起面板（对应提亮）
BG_INPUT = "#0f141d"         # 输入框/下拉底（保持深，聚焦时文字对比强）
BG_INPUT_FOCUS = "#13202e"   # 输入框聚焦底（轻微提亮 + accent tint）
BG_DISABLED = "#161d28"      # 禁用控件底（与 BG_PANEL 区分，原同值导致层次丢失）
BG_SELECTION = "#203044"     # 选中项底
BG_OVERLAY = "rgba(8, 12, 18, 210)"  # 模态遮罩（加深，遮罩更聚焦）

# ── 文本分层 ────────────────────────────────────────────────────────
TEXT_PRIMARY = "#d7def3"     # 主文本（标题、正文）
TEXT_SECONDARY = "#b7c0d8"   # 次文本（标签、说明）
TEXT_MUTED = "#9aa7bd"       # 弱文本（占位、统计）
TEXT_DISABLED = "#5f6f86"    # 禁用文本
TEXT_ON_ACCENT = "#0d1118"   # 强调色按钮上的文字（深底）
TEXT_INVERTED = "#f8fafc"    # 选中/反白文字

# ── 强调色（VOFA+ 工业青） ──────────────────────────────────────────
ACCENT = "#22d3ee"           # 主强调：聚焦边框、连接态、主按钮（硬断言锁定）
ACCENT_HOVER = "#67e8f9"     # 悬停
ACCENT_PRESSED = "#0891b2"   # 按下
ACCENT_SOFT = "rgba(34, 211, 238, 0.12)"  # 强调底色（选中条/徽章）
ACCENT_BORDER = "rgba(34, 211, 238, 0.35)"

# ── accent gradient（跨色相品牌渐变，Linear/Vercel 级） ─────────────
# 主按钮/激活态/品牌徽标用 2-stop 渐变，打破单一青色相单调感。
# 从青 (ACCENT) 渐变到蓝 (#3b82f6)，跨色相但同冷色调，保持工业风统一。
ACCENT_GRADIENT_FROM = "#22d3ee"   # 渐变起点（青）
ACCENT_GRADIENT_TO = "#3b82f6"     # 渐变终点（蓝）
ACCENT_GRADIENT = f"qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 {ACCENT_GRADIENT_FROM}, stop:1 {ACCENT_GRADIENT_TO})"

# ── 状态色 ──────────────────────────────────────────────────────────
SUCCESS = "#22c55e"          # 已连接 / RX 字节 / 成功（硬断言锁定）
SUCCESS_HOVER = "#4ade80"
SUCCESS_SOFT = "rgba(34, 197, 94, 0.10)"
WARNING = "#f59e0b"          # 连接状态 / 警告（硬断言锁定）
WARNING_HOVER = "#fbbf24"
WARNING_SOFT = "rgba(245, 158, 11, 0.08)"
WARNING_BORDER = "rgba(245, 158, 11, 0.35)"
ERROR = "#ef4444"            # 断开 / 错误（硬断言锁定）
ERROR_HOVER = "#f87171"
ERROR_SOFT = "rgba(239, 68, 68, 0.10)"

# ── 终端/日志通道分色（VOFA+ RX/TX 一等对象） ──────────────────────
TERM_RX = "#22c55e"          # 接收文本（绿，硬断言锁定）
TERM_TX = "#38bdf8"          # 发送文本（蓝，硬断言锁定）
TERM_TIMESTAMP = "#5f6f86"   # 时间戳
TERM_SYSTEM = "#9aa7bd"      # 系统日志
TERM_BACKGROUND = "#0d1118"  # 日志区底
TERM_SELECTION = "#2563eb"   # 日志区选中底

# ── 波形曲线配色序列（固定循环，对齐 EK-OmniProbe PRESET_COLORS 范式） ──
# 前 4 色复用现有强调/状态 token，后 3 色补足深底高对比。避免 pg.intColor 随通道数漂移。
WAVE_CURVES = (ACCENT, SUCCESS, WARNING, TERM_TX, "#a78bfa", "#ec4899", "#f97316")

# ── 边框 / 分隔 ────────────────────────────────────────────────────
BORDER = "#2a3645"           # 默认边框 / 分隔线（提亮，与新 BG_PANEL 协调）
BORDER_STRONG = "#3a4a5e"    # 输入框边框（提亮）
BORDER_FOCUS = "#22d3ee"     # 聚焦边框（同 ACCENT）

# ── 滚动条 ─────────────────────────────────────────────────────────
SCROLLBAR = "#3a4a5e"
SCROLLBAR_HOVER = "#52647a"
SCROLLBAR_BACKGROUND = "transparent"

# ── 阴影 / elevation（多层 z 轴深度，对齐 macOS Big Sur / Linear） ──
# 三档投影 + accent tint glow。QSS 不支持 box-shadow，这些 token 供
# QGraphicsDropShadowEffect（micro_interactions / 控件）与 QSS 近似投影使用。
SHADOW = "rgba(0, 0, 0, 0.40)"                  # 通用阴影（加深，原 0.35）
SHADOW_CARD = "rgba(0, 0, 0, 0.35)"             # 卡片级（elevation-1）
SHADOW_POPOVER = "rgba(0, 0, 0, 0.45)"          # 弹出/浮层级（elevation-2）
SHADOW_MODAL = "rgba(0, 0, 0, 0.60)"            # 模态级（elevation-3）
CARD_GLOW = "rgba(34, 211, 238, 0.35)"          # accent tint glow（hover/active 卡片辉光）

# ── 玻璃卡片 / TopBar 视觉层（QSS 无 blur，用渐变+边框近似 surface） ──
# 卡片顶部光带（3-stop 玻璃高光，压缩到顶部 18%）。
CARD_SHEEN_TOP = "rgba(255, 255, 255, 0.14)"    # 提亮（原 0.12），玻璃感更可见
CARD_SHEEN_MID = "rgba(255, 255, 255, 0.05)"
CARD_SHEEN_BOTTOM = "rgba(255, 255, 255, 0.00)"
# 卡片 per-side border（内顶白光 + 接地暗边，近似 inset shadow + 投影）。
CARD_INNER_TOP_EDGE = "rgba(255, 255, 255, 0.14)"
CARD_GROUND_SHADOW = "rgba(0, 0, 0, 0.35)"
# 卡片悬停外环（比 ACCENT_BORDER 更亮的强调青）。
CARD_HOVER_RING = "rgba(34, 211, 238, 0.55)"
# 向后兼容别名（all_tokens / tokens.CARD_HIGHLIGHT_STOP_* 仍引用）。
CARD_HIGHLIGHT_TOP = CARD_SHEEN_TOP
CARD_HIGHLIGHT_BOTTOM = CARD_SHEEN_BOTTOM
# TopBar 表面：比窗口底略亮的分层渐变。
TOPBAR_BG_TOP = "#161e2b"
TOPBAR_BG_BOTTOM = "#10141d"
TOPBAR_BORDER = "#2e3b4d"
# 品牌徽标底（强调青软底 + 渐变，对齐 EK-OmniProbe logo chip）。
BRAND_CHIP_BG = "rgba(34, 211, 238, 0.16)"
BRAND_CHIP_BORDER = "rgba(34, 211, 238, 0.42)"


def all_tokens() -> dict[str, str]:
    """返回色板常量名 -> 颜色值的有序映射，供 QSS 生成与测试校验。"""

    return {
        "bg_window": BG_WINDOW,
        "bg_app": BG_APP,
        "bg_panel": BG_PANEL,
        "bg_panel_raised": BG_PANEL_RAISED,
        "bg_input": BG_INPUT,
        "bg_input_focus": BG_INPUT_FOCUS,
        "bg_disabled": BG_DISABLED,
        "bg_selection": BG_SELECTION,
        "bg_overlay": BG_OVERLAY,
        "text_primary": TEXT_PRIMARY,
        "text_secondary": TEXT_SECONDARY,
        "text_muted": TEXT_MUTED,
        "text_disabled": TEXT_DISABLED,
        "text_on_accent": TEXT_ON_ACCENT,
        "text_inverted": TEXT_INVERTED,
        "accent": ACCENT,
        "accent_hover": ACCENT_HOVER,
        "accent_pressed": ACCENT_PRESSED,
        "accent_soft": ACCENT_SOFT,
        "accent_border": ACCENT_BORDER,
        "accent_gradient_from": ACCENT_GRADIENT_FROM,
        "accent_gradient_to": ACCENT_GRADIENT_TO,
        "accent_gradient": ACCENT_GRADIENT,
        "success": SUCCESS,
        "success_hover": SUCCESS_HOVER,
        "success_soft": SUCCESS_SOFT,
        "warning": WARNING,
        "warning_hover": WARNING_HOVER,
        "warning_soft": WARNING_SOFT,
        "warning_border": WARNING_BORDER,
        "error": ERROR,
        "error_hover": ERROR_HOVER,
        "error_soft": ERROR_SOFT,
        "term_rx": TERM_RX,
        "term_tx": TERM_TX,
        "term_timestamp": TERM_TIMESTAMP,
        "term_system": TERM_SYSTEM,
        "term_background": TERM_BACKGROUND,
        "term_selection": TERM_SELECTION,
        "border": BORDER,
        "border_strong": BORDER_STRONG,
        "border_focus": BORDER_FOCUS,
        "scrollbar": SCROLLBAR,
        "scrollbar_hover": SCROLLBAR_HOVER,
        "scrollbar_background": SCROLLBAR_BACKGROUND,
        "shadow": SHADOW,
        "shadow_card": SHADOW_CARD,
        "shadow_popover": SHADOW_POPOVER,
        "shadow_modal": SHADOW_MODAL,
        "card_glow": CARD_GLOW,
        "card_highlight_top": CARD_HIGHLIGHT_TOP,
        "card_highlight_bottom": CARD_HIGHLIGHT_BOTTOM,
        "topbar_bg_top": TOPBAR_BG_TOP,
        "topbar_bg_bottom": TOPBAR_BG_BOTTOM,
        "topbar_border": TOPBAR_BORDER,
        "brand_chip_bg": BRAND_CHIP_BG,
        "brand_chip_border": BRAND_CHIP_BORDER,
    }
