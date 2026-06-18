"""Serial Station VOFA+ 精致工业风深色色板。

单一真相源：所有 UI 颜色必须通过本模块的常量引用，禁止在控件逻辑中硬编码颜色。
色板取自 ``resources/themes/modern_dark.qss`` 的 PRD-071 工业风分区，
对齐 VOFA+ 深色工业风观感（深蓝灰底 + 青色强调 + 终端 RX/TX 分色）。

约束：本模块只依赖标准库，不 import PyQt 或其它项目层，便于在测试与
QSS 生成器中独立复用。
"""

from __future__ import annotations


# ── 背景分层 ────────────────────────────────────────────────────────
# 从最深到最浅，用于窗口底、面板底、控件底、悬停态分层。
BG_WINDOW = "#0d1118"        # QMainWindow / 应用最底层
BG_APP = "#101218"           # 应用容器层
BG_PANEL = "#151b24"         # 卡片/面板底
BG_PANEL_RAISED = "#1b2634"  # 悬停/凸起面板
BG_INPUT = "#0f141d"         # 输入框/下拉底
BG_INPUT_FOCUS = "#111827"   # 输入框聚焦底
BG_DISABLED = "#151b24"      # 禁用控件底
BG_SELECTION = "#203044"     # 选中项底
BG_OVERLAY = "rgba(13, 17, 24, 200)"  # 模态遮罩

# ── 文本分层 ────────────────────────────────────────────────────────
TEXT_PRIMARY = "#d7def3"     # 主文本（标题、正文）
TEXT_SECONDARY = "#b7c0d8"   # 次文本（标签、说明）
TEXT_MUTED = "#9aa7bd"       # 弱文本（占位、统计）
TEXT_DISABLED = "#5f6f86"    # 禁用文本
TEXT_ON_ACCENT = "#0d1118"   # 强调色按钮上的文字（深底）
TEXT_INVERTED = "#f8fafc"    # 选中/反白文字

# ── 强调色（VOFA+ 工业青） ──────────────────────────────────────────
ACCENT = "#22d3ee"           # 主强调：聚焦边框、连接态、主按钮
ACCENT_HOVER = "#67e8f9"     # 悬停
ACCENT_PRESSED = "#0891b2"   # 按下
ACCENT_SOFT = "rgba(34, 211, 238, 0.12)"  # 强调底色（选中条/徽章）
ACCENT_BORDER = "rgba(34, 211, 238, 0.35)"

# ── 状态色 ──────────────────────────────────────────────────────────
SUCCESS = "#22c55e"          # 已连接 / RX 字节 / 成功
SUCCESS_HOVER = "#4ade80"
SUCCESS_SOFT = "rgba(34, 197, 94, 0.10)"
WARNING = "#f59e0b"          # 连接状态 / 警告
WARNING_HOVER = "#fbbf24"
WARNING_SOFT = "rgba(245, 158, 11, 0.08)"
WARNING_BORDER = "rgba(245, 158, 11, 0.35)"
ERROR = "#ef4444"            # 断开 / 错误
ERROR_HOVER = "#f87171"
ERROR_SOFT = "rgba(239, 68, 68, 0.10)"

# ── 终端/日志通道分色（VOFA+ RX/TX 一等对象） ──────────────────────
TERM_RX = "#22c55e"          # 接收文本（绿）
TERM_TX = "#38bdf8"          # 发送文本（蓝）
TERM_TIMESTAMP = "#5f6f86"   # 时间戳
TERM_SYSTEM = "#9aa7bd"      # 系统日志
TERM_BACKGROUND = "#0d1118"  # 日志区底
TERM_SELECTION = "#2563eb"   # 日志区选中底

# ── 边框 / 分隔 ────────────────────────────────────────────────────
BORDER = "#27313f"           # 默认边框 / 分隔线
BORDER_STRONG = "#334155"    # 输入框边框
BORDER_FOCUS = "#22d3ee"     # 聚焦边框（同 ACCENT）

# ── 滚动条 ─────────────────────────────────────────────────────────
SCROLLBAR = "#334155"
SCROLLBAR_HOVER = "#475569"
SCROLLBAR_BACKGROUND = "transparent"

# ── 阴影 ───────────────────────────────────────────────────────────
SHADOW = "rgba(0, 0, 0, 0.35)"

# ── 玻璃卡片 / TopBar 视觉层（QSS 无 blur，用渐变+边框近似 EK-OmniProbe surface） ──
# 卡片顶部高光（inner highlight，模拟玻璃反光）。
CARD_HIGHLIGHT_TOP = "rgba(255, 255, 255, 0.05)"
CARD_HIGHLIGHT_BOTTOM = "rgba(255, 255, 255, 0.00)"
# TopBar 表面：比窗口底略亮的分层渐变。
TOPBAR_BG_TOP = "#131a25"
TOPBAR_BG_BOTTOM = "#10141d"
TOPBAR_BORDER = "#2a3647"
# 品牌徽标底（强调青软底，对齐 EK-OmniProbe logo chip）。
BRAND_CHIP_BG = "rgba(34, 211, 238, 0.14)"
BRAND_CHIP_BORDER = "rgba(34, 211, 238, 0.40)"


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
        "card_highlight_top": CARD_HIGHLIGHT_TOP,
        "card_highlight_bottom": CARD_HIGHLIGHT_BOTTOM,
        "topbar_bg_top": TOPBAR_BG_TOP,
        "topbar_bg_bottom": TOPBAR_BG_BOTTOM,
        "topbar_border": TOPBAR_BORDER,
        "brand_chip_bg": BRAND_CHIP_BG,
        "brand_chip_border": BRAND_CHIP_BORDER,
    }
