"""浅色主题色板（对齐 VOFA+ 浅色模式 + 深浅一致性）。

与 ``palette.py`` 的深色 token 一一对应，供 ThemeManager 运行时切换。
约束：本模块只依赖标准库，不 import PyQt。
"""

from __future__ import annotations

# ── 背景分层（浅色：白→浅灰） ─────────────────────────────────────
BG_WINDOW = "#f5f6f8"
BG_APP = "#ffffff"
BG_PANEL = "#ffffff"
BG_PANEL_RAISED = "#eef1f5"
BG_INPUT = "#ffffff"
BG_INPUT_FOCUS = "#f8fbff"
BG_DISABLED = "#eef1f5"
BG_SELECTION = "#e0f2fe"
BG_OVERLAY = "rgba(255, 255, 255, 180)"

# ── 文本分层 ──────────────────────────────────────────────────────
TEXT_PRIMARY = "#1e293b"
TEXT_SECONDARY = "#475569"
TEXT_MUTED = "#94a3b8"
TEXT_DISABLED = "#cbd5e1"
TEXT_ON_ACCENT = "#ffffff"
TEXT_INVERTED = "#1e293b"

# ── 强调色（与深色一致，保持品牌识别） ────────────────────────────
ACCENT = "#0891b2"
ACCENT_HOVER = "#06b6d4"
ACCENT_PRESSED = "#0e7490"
ACCENT_SOFT = "rgba(8, 145, 178, 0.10)"
ACCENT_BORDER = "rgba(8, 145, 178, 0.35)"

# ── 状态色 ────────────────────────────────────────────────────────
SUCCESS = "#16a34a"
SUCCESS_HOVER = "#22c55e"
SUCCESS_SOFT = "rgba(22, 163, 74, 0.10)"
WARNING = "#d97706"
WARNING_HOVER = "#f59e0b"
WARNING_SOFT = "rgba(217, 119, 6, 0.08)"
WARNING_BORDER = "rgba(217, 119, 6, 0.35)"
ERROR = "#dc2626"
ERROR_HOVER = "#ef4444"
ERROR_SOFT = "rgba(220, 38, 38, 0.10)"

# ── 终端/日志（浅底深字） ─────────────────────────────────────────
TERM_RX = "#16a34a"
TERM_TX = "#0284c7"
TERM_TIMESTAMP = "#94a3b8"
TERM_SYSTEM = "#475569"
TERM_BACKGROUND = "#f8fafc"
TERM_SELECTION = "#bae6fd"

# ── 边框 / 分隔 ───────────────────────────────────────────────────
BORDER = "#e2e8f0"
BORDER_STRONG = "#cbd5e1"
BORDER_FOCUS = "#0891b2"

# ── 滚动条 ────────────────────────────────────────────────────────
SCROLLBAR = "#cbd5e1"
SCROLLBAR_HOVER = "#94a3b8"
SCROLLBAR_BACKGROUND = "transparent"

# ── 阴影 ──────────────────────────────────────────────────────────
SHADOW = "rgba(15, 23, 42, 0.08)"

# ── 玻璃卡片 / TopBar 视觉层（与深色 palette 对齐，浅色版本） ──────
CARD_HIGHLIGHT_TOP = "rgba(255, 255, 255, 0.6)"
CARD_HIGHLIGHT_BOTTOM = "rgba(255, 255, 255, 0.0)"
TOPBAR_BG_TOP = "#ffffff"
TOPBAR_BG_BOTTOM = "#f1f5f9"
TOPBAR_BORDER = "#e2e8f0"
BRAND_CHIP_BG = "rgba(8, 145, 178, 0.12)"
BRAND_CHIP_BORDER = "rgba(8, 145, 178, 0.30)"


def all_tokens() -> dict[str, str]:
    """返回浅色色板常量名 -> 颜色值的映射，键与 palette.all_tokens() 对齐。"""

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
