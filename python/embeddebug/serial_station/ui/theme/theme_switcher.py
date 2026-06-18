"""主题运行时切换（深色 ⇄ 浅色）。

策略：QSS 由 qss_builder 用深色 palette 生成；切换到浅色时，把 QSS 中的
深色 hex 值按 dark→light 映射替换，无需改动各 section 模块。
``ThemeManager.apply_theme`` 已支持加载外部 qss，本模块提供运行时切换的便捷入口。

约束：本模块只依赖 PyQt6 + theme 子包，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light
from embeddebug.serial_station.ui.theme.manager import ThemeManager
from embeddebug.serial_station.ui.theme.qss_builder import build_qss

THEME_DARK = "serial_station_dark"
THEME_LIGHT = "serial_station_light"
AVAILABLE_THEMES: tuple[str, ...] = (THEME_DARK, THEME_LIGHT)


def _dark_to_light_map() -> dict[str, str]:
    """构建 深色值 → 浅色值 的替换映射（按键名对齐）。"""

    dark_tokens = dark.all_tokens()
    light_tokens = light.all_tokens()
    return {
        dark_value: light_tokens[key]
        for key, dark_value in dark_tokens.items()
        if key in light_tokens and dark_value != light_tokens[key]
    }


def build_light_qss() -> str:
    """用深色 QSS 做 dark→light 颜色替换生成浅色 QSS。

    用唯一占位符避免链式替换（深色源值与浅色目标值重叠时，
    若直接 replace 会把刚替换出的目标值再替换一次）。
    """

    qss = build_qss()
    mapping = _dark_to_light_map()
    # 第一阶段：深色值 → 唯一占位符。
    placeholders: dict[str, str] = {}
    for index, dark_val in enumerate(sorted(mapping, key=len, reverse=True)):
        placeholder = f"\x00COLOR{index}\x00"
        placeholders[placeholder] = mapping[dark_val]
        qss = qss.replace(dark_val, placeholder)
    # 第二阶段：占位符 → 浅色值。
    for placeholder, light_val in placeholders.items():
        qss = qss.replace(placeholder, light_val)
    return qss


class ThemeSwitcher:
    """运行时深色/浅色主题切换器。"""

    def __init__(self, app: QApplication) -> None:
        self._app = app
        self._current = THEME_DARK
        self._light_qss: str | None = None

    @property
    def current_theme(self) -> str:
        return self._current

    def is_dark(self) -> bool:
        return self._current == THEME_DARK

    def toggle(self) -> str:
        """切换到另一主题，返回切换后的主题名。"""

        if self._current == THEME_DARK:
            return self.apply_light()
        return self.apply_dark()

    def apply_dark(self) -> str:
        """应用深色主题。"""

        ThemeManager().apply_theme(self._app, THEME_DARK)
        self._current = THEME_DARK
        return THEME_DARK

    def apply_light(self) -> str:
        """应用浅色主题。"""

        if self._light_qss is None:
            self._light_qss = build_light_qss()
        self._app.setStyleSheet(self._light_qss)
        manager = ThemeManager()
        manager._current_theme = THEME_LIGHT  # type: ignore[attr-defined]
        self._current = THEME_LIGHT
        return THEME_LIGHT


def apply_theme_by_name(app: QApplication, name: str) -> str:
    """按名应用主题，返回生效主题名。未知名回退深色。"""

    switcher = ThemeSwitcher(app)
    if name == THEME_LIGHT:
        return switcher.apply_light()
    return switcher.apply_dark()
