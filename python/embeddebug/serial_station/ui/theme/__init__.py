"""Serial Station 主题子系统。

VOFA+ 精致工业风深色主题的单一入口。颜色集中在 ``palette``，尺寸集中在
``tokens``，QSS 由 ``qss_builder`` 程序化生成，``ThemeManager`` 负责应用与切换。

约束：本子包只依赖 PyQt6 + 标准库，不 import controller/core/protocols/services。
"""

from embeddebug.serial_station.ui.theme.manager import (
    BUILTIN_THEMES,
    DEFAULT_THEME,
    ThemeManager,
    apply_theme,
    current_theme_name,
)
from embeddebug.serial_station.ui.theme.palette import all_tokens as palette_tokens
from embeddebug.serial_station.ui.theme.tokens import all_tokens as size_tokens

__all__ = [
    "BUILTIN_THEMES",
    "DEFAULT_THEME",
    "ThemeManager",
    "apply_theme",
    "current_theme_name",
    "palette_tokens",
    "size_tokens",
]
