"""Theme palettes and selectable visual directions for the presentation layer."""

from __future__ import annotations

from dataclasses import dataclass

BACKGROUND = "#0b0918"
SURFACE = "#17132b"
SURFACE_RAISED = "#241b40"
SURFACE_INPUT = "#0f0d20"
BORDER = "#3c2e60"
BORDER_STRONG = "#7459aa"
TEXT = "#f8f1ff"
TEXT_MUTED = "#c8b9e2"
TEXT_SUBTLE = "#988ab6"
ACCENT = "#67f2d1"
ACCENT_STRONG = "#35d2b6"
ACCENT_BLUE = "#8cbaff"
ACCENT_PURPLE = "#d6a5ff"
ACCENT_PINK = "#ff7ac8"
WARNING = "#ffd37e"
ERROR = "#ff86ad"
TERMINAL_BACKGROUND = "#090817"
TERMINAL_TEXT = "#e8ddff"

# Semantic roles keep selector-level styling independent from a theme's visual
# direction.  These defaults intentionally mirror the original Star Trail
# stylesheet so selecting another theme does not silently change the default.
SUCCESS = "#d8fff5"
SUCCESS_SURFACE = "#15353b"
SUCCESS_BORDER = "#4ccbb6"
INFO_SURFACE = "#192d4e"
INFO_BORDER = "#557fc2"
WARNING_SURFACE = "#342918"
WARNING_BORDER = "#80623b"
ERROR_SURFACE = "#3c1c36"
ERROR_BORDER = "#a24f79"
HISTORY_SURFACE = "#302044"
HISTORY_BORDER = "#7658a9"
NEUTRAL_SURFACE = "#17132b"
NEUTRAL_BORDER = "#3c2e60"
INTERACTIVE_HOVER = "#2b1d49"
INTERACTIVE_PRESSED = "#171329"
DISABLED_SURFACE = "#101a2a"
DISABLED_BORDER = "#1c2b40"
DISABLED_TEXT = "#8296af"
FOCUS = ACCENT
SELECTION_BACKGROUND = "#5c3f83"
SELECTION_TEXT = "#fff4ff"
ON_ACCENT = "#130f24"


@dataclass(frozen=True, slots=True)
class ThemeSpec:
    """Selectable palette metadata used by the shell and theme picker."""

    key: str
    label: str
    description: str
    background: str
    surface: str
    surface_input: str
    border: str
    text: str
    text_muted: str
    text_subtle: str
    accent: str
    accent_blue: str
    accent_purple: str
    accent_pink: str
    warning: str
    error: str
    terminal_background: str
    terminal_text: str
    success: str
    success_surface: str
    success_border: str
    info_surface: str
    info_border: str
    warning_surface: str
    warning_border: str
    error_surface: str
    error_border: str
    history_surface: str
    history_border: str
    neutral_surface: str
    neutral_border: str
    interactive_hover: str
    interactive_pressed: str
    disabled_surface: str
    disabled_border: str
    disabled_text: str
    focus: str
    selection_background: str
    selection_text: str
    on_accent: str


THEME_OPTIONS: tuple[ThemeSpec, ...] = (
    ThemeSpec(
        key="star_trail",
        label="星轨霓虹",
        description="紫黑基底、薄荷信号和淡紫轨道的默认二次元调试主题。",
        background=BACKGROUND,
        surface=SURFACE,
        surface_input=SURFACE_INPUT,
        border=BORDER,
        text=TEXT,
        text_muted=TEXT_MUTED,
        text_subtle=TEXT_SUBTLE,
        accent=ACCENT,
        accent_blue=ACCENT_BLUE,
        accent_purple=ACCENT_PURPLE,
        accent_pink=ACCENT_PINK,
        warning=WARNING,
        error=ERROR,
        terminal_background=TERMINAL_BACKGROUND,
        terminal_text=TERMINAL_TEXT,
        success=SUCCESS,
        success_surface=SUCCESS_SURFACE,
        success_border=SUCCESS_BORDER,
        info_surface=INFO_SURFACE,
        info_border=INFO_BORDER,
        warning_surface=WARNING_SURFACE,
        warning_border=WARNING_BORDER,
        error_surface=ERROR_SURFACE,
        error_border=ERROR_BORDER,
        history_surface=HISTORY_SURFACE,
        history_border=HISTORY_BORDER,
        neutral_surface=NEUTRAL_SURFACE,
        neutral_border=NEUTRAL_BORDER,
        interactive_hover=INTERACTIVE_HOVER,
        interactive_pressed=INTERACTIVE_PRESSED,
        disabled_surface=DISABLED_SURFACE,
        disabled_border=DISABLED_BORDER,
        disabled_text=DISABLED_TEXT,
        focus=FOCUS,
        selection_background=SELECTION_BACKGROUND,
        selection_text=SELECTION_TEXT,
        on_accent=ON_ACCENT,
    ),
    ThemeSpec(
        key="moonlit_ocean",
        label="月影深海",
        description="深海蓝与青绿色信号，适合长时间串口观察。",
        background="#07131f",
        surface="#102638",
        surface_input="#0a1c2b",
        border="#25495e",
        text="#e9fbff",
        text_muted="#acd5e3",
        text_subtle="#769bab",
        accent="#63ead8",
        accent_blue="#7fc7ff",
        accent_purple="#9bb8ff",
        accent_pink="#7de1ff",
        warning="#f7d58b",
        error="#ff9db0",
        terminal_background="#06101a",
        terminal_text="#d9f7ff",
        success="#d9fff6",
        success_surface="#10383a",
        success_border="#48c7b6",
        info_surface="#122c45",
        info_border="#4c8fce",
        warning_surface="#3a3022",
        warning_border="#ae8953",
        error_surface="#432238",
        error_border="#c06482",
        history_surface="#233354",
        history_border="#7189cb",
        neutral_surface="#102638",
        neutral_border="#25495e",
        interactive_hover="#193a50",
        interactive_pressed="#0a1c2b",
        disabled_surface="#10202d",
        disabled_border="#264154",
        disabled_text="#769bab",
        focus="#63ead8",
        selection_background="#365b82",
        selection_text="#e9fbff",
        on_accent="#06242a",
    ),
    ThemeSpec(
        key="sakura_night",
        label="樱雾夜航",
        description="墨蓝底色、樱粉高光和暖紫状态，突出二次元氛围。",
        background="#160b1b",
        surface="#29152d",
        surface_input="#1d0f25",
        border="#5b315e",
        text="#fff1f8",
        text_muted="#e8bfd5",
        text_subtle="#b88ea9",
        accent="#8ff0d0",
        accent_blue="#a7c8ff",
        accent_purple="#e5a9ff",
        accent_pink="#ff8fcf",
        warning="#ffd18e",
        error="#ff91aa",
        terminal_background="#100817",
        terminal_text="#ffe9f5",
        success="#d8fff5",
        success_surface="#1c3e3b",
        success_border="#55c5ae",
        info_surface="#25314a",
        info_border="#7896d0",
        warning_surface="#3f2c22",
        warning_border="#b68857",
        error_surface="#482035",
        error_border="#c06181",
        history_surface="#47283f",
        history_border="#b36fc1",
        neutral_surface="#29152d",
        neutral_border="#5b315e",
        interactive_hover="#432545",
        interactive_pressed="#1d0f25",
        disabled_surface="#241626",
        disabled_border="#4f2d4e",
        disabled_text="#b88ea9",
        focus="#8ff0d0",
        selection_background="#74436f",
        selection_text="#fff1f8",
        on_accent="#29132b",
    ),
)

DEFAULT_THEME_KEY = "star_trail"


def theme_spec(key: str) -> ThemeSpec:
    """Return a known theme, falling back to the safe default."""

    return next((theme for theme in THEME_OPTIONS if theme.key == key), THEME_OPTIONS[0])


__all__ = [
    "ACCENT",
    "ACCENT_BLUE",
    "ACCENT_PINK",
    "ACCENT_PURPLE",
    "ACCENT_STRONG",
    "BACKGROUND",
    "BORDER",
    "BORDER_STRONG",
    "DEFAULT_THEME_KEY",
    "DISABLED_BORDER",
    "DISABLED_SURFACE",
    "DISABLED_TEXT",
    "ERROR",
    "ERROR_BORDER",
    "ERROR_SURFACE",
    "FOCUS",
    "HISTORY_BORDER",
    "HISTORY_SURFACE",
    "INFO_BORDER",
    "INFO_SURFACE",
    "INTERACTIVE_HOVER",
    "INTERACTIVE_PRESSED",
    "NEUTRAL_BORDER",
    "NEUTRAL_SURFACE",
    "ON_ACCENT",
    "SELECTION_BACKGROUND",
    "SELECTION_TEXT",
    "SUCCESS",
    "SUCCESS_BORDER",
    "SUCCESS_SURFACE",
    "SURFACE",
    "SURFACE_INPUT",
    "SURFACE_RAISED",
    "TERMINAL_BACKGROUND",
    "TERMINAL_TEXT",
    "TEXT",
    "TEXT_MUTED",
    "TEXT_SUBTLE",
    "THEME_OPTIONS",
    "WARNING",
    "WARNING_BORDER",
    "WARNING_SURFACE",
    "ThemeSpec",
    "theme_spec",
]
