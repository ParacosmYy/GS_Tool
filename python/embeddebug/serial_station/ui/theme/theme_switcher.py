"""主题运行时切换（深色 ⇄ 浅色）+ 多强调色 override。

策略：QSS 由 qss_builder 用深色 palette 生成；切换到浅色时，把 QSS 中的
深色 hex 值按 dark→light 映射替换，无需改动各 section 模块。
``ThemeManager.apply_theme`` 已支持加载外部 qss，本模块提供运行时切换的便捷入口。

Batch 10 — 多强调色：在 dark/light 之上叠加 7 套 accent override。accent 变体
只换强调色调，base palette 常量保持 cyan 不动（测试锁定 + 默认零回归）。
切 accent 时复用 ``apply_accent_recolor`` 的两阶段占位符替换，对 dark/light QSS
分别注入当前 accent 变体对应值。

约束：本模块只依赖 PyQt6 + theme 子包，不访问 controller/transport。
"""

from __future__ import annotations

from PyQt6.QtWidgets import QApplication

from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light
from embeddebug.serial_station.ui.theme.accents import (
    DEFAULT_ACCENT_ID,
    get_active_accent_id,
    set_active_accent,
)
from embeddebug.serial_station.ui.theme.manager import ThemeManager
from embeddebug.serial_station.ui.theme.qss_builder import apply_accent_recolor, build_qss

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


def build_themed_qss(theme: str, accent_id: str | None = None) -> str:
    """生成指定主题 + 强调色的最终 QSS（accent recolor 在 dark→light 之后叠加）。

    Args:
        theme: ``THEME_DARK`` / ``THEME_LIGHT``。
        accent_id: 强调色变体 id；``None`` 取当前运行时活动 accent。

    切换顺序：先 dark→light（如需），再 accent recolor。accent recolor 的源值取
    对应明度 palette 的 cyan 字面（dark 用 ``palette.ACCENT``，light 用
    ``palette_light.ACCENT``），因此 light 路径下 recolor 必须在 build_light_qss 之后。
    """

    is_light = theme == THEME_LIGHT
    qss = build_light_qss() if is_light else build_qss()
    from embeddebug.serial_station.ui.theme.accents import get_accent_by_id

    variant = get_accent_by_id(accent_id) if accent_id is not None else get_accent_by_id(get_active_accent_id())
    return apply_accent_recolor(qss, variant, is_light=is_light)


class ThemeSwitcher:
    """运行时深色/浅色主题 + 强调色切换器。

    Batch 10: 增加 accent override 能力。``apply_dark`` / ``apply_light`` 在生成
    QSS 时叠加当前 accent recolor；``apply_accent`` 切换强调色并重新应用当前主题。
    QSS 缓存按 (theme, accent_id) 联合键失效，避免 accent 切换后命中旧 cyan QSS。
    """

    def __init__(self, app: QApplication) -> None:
        self._app = app
        self._current = THEME_DARK
        # QSS 缓存：(theme, accent_id) -> qss。accent 切换时换键，自动失效旧条目。
        self._qss_cache: dict[tuple[str, str], str] = {}

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

    def _cached_qss(self, theme: str) -> str:
        """取 (theme, 当前 accent) 的 QSS，缓存命中则复用。"""

        accent_id = get_active_accent_id()
        key = (theme, accent_id)
        if key not in self._qss_cache:
            self._qss_cache[key] = build_themed_qss(theme, accent_id)
        return self._qss_cache[key]

    def apply_dark(self) -> str:
        """应用深色主题（叠加当前 accent recolor）。"""

        qss = self._cached_qss(THEME_DARK)
        self._app.setStyleSheet(qss)
        manager = ThemeManager()
        manager._current_theme = THEME_DARK  # type: ignore[attr-defined]
        self._current = THEME_DARK
        self._persist_theme(THEME_DARK)
        return THEME_DARK

    def apply_light(self) -> str:
        """应用浅色主题（叠加当前 accent recolor）。"""

        qss = self._cached_qss(THEME_LIGHT)
        self._app.setStyleSheet(qss)
        manager = ThemeManager()
        manager._current_theme = THEME_LIGHT  # type: ignore[attr-defined]
        self._current = THEME_LIGHT
        self._persist_theme(THEME_LIGHT)
        return THEME_LIGHT

    @staticmethod
    def _persist_theme(theme_id: str) -> None:
        """Batch 12: 持久化主题选择（深/浅）到 theme_store。

        启动期恢复路径（``main.create_application`` → ``apply_theme_by_name``）也会
        走这里，但那时写入的值与读出的值一致，幂等无害。失败静默（偏好持久化是
        best-effort，不应阻断主题应用）。
        """

        try:
            from embeddebug.serial_station.ui.theme import theme_store

            theme_store.save_theme_id(theme_id)
        except Exception:
            pass

    def apply_accent(self, accent_id: str) -> str:
        """切换强调色变体，重新应用当前主题。返回当前主题名。

        清 QSS 缓存中当前主题的旧 accent 条目，确保 recolor 重新计算。
        不带过渡动画（调用方 ``settings_panel`` 自行决定是否包过渡）。
        """

        set_active_accent(accent_id)
        # 清掉当前主题的所有 accent 缓存条目，强制重算。
        self._qss_cache = {k: v for k, v in self._qss_cache.items() if k[0] != self._current}
        if self._current == THEME_LIGHT:
            return self.apply_light()
        return self.apply_dark()


def apply_theme_by_name(app: QApplication, name: str, accent: str | None = None) -> str:
    """按名应用主题，返回生效主题名。未知名回退深色。

    Args:
        app: 目标 ``QApplication``。
        name: ``THEME_DARK`` / ``THEME_LIGHT``。
        accent: 可选强调色变体 id；``None`` 保持当前 accent 不变（向后兼容既有调用）。
    """

    if accent is not None:
        set_active_accent(accent)
    switcher = ThemeSwitcher(app)
    if name == THEME_LIGHT:
        return switcher.apply_light()
    return switcher.apply_dark()


def apply_accent_by_name(app: QApplication, accent_id: str) -> str:
    """切换强调色，重新应用当前主题。便捷入口（不带过渡动画）。"""

    switcher = ThemeSwitcher(app)
    # 同步 switcher 内部 _current 到 ThemeManager 实际状态。
    switcher._current = ThemeManager().current_theme or THEME_DARK
    return switcher.apply_accent(accent_id)


def default_accent_id() -> str:
    """默认强调色 id（cyan）。"""

    return DEFAULT_ACCENT_ID
