"""Versioned editor settings use cases."""

from ..domain.models import AppearanceSettings, EditorSettings, SettingsSnapshot
from .ports import SettingsStore

CURRENT_SETTINGS_VERSION = 3
SUPPORTED_EDITOR_FONTS = ("Cascadia Code", "Cascadia Mono", "Consolas", "JetBrains Mono")
SUPPORTED_UI_FONTS = ("Microsoft YaHei UI", "Segoe UI", "Arial")
SUPPORTED_FONT_STYLES = ("regular", "semibold", "bold", "italic")
SUPPORTED_LOCALES = ("en-US", "zh-CN")
SUPPORTED_THEMES = ("ink-violet", "paper-sand", "sakura-pop")
SUPPORTED_ACCENTS = ("violet", "cyan", "rose", "amber")
DEFAULT_EDITOR_SETTINGS = EditorSettings()
DEFAULT_APPEARANCE_SETTINGS = AppearanceSettings()
DEFAULT_SETTINGS = SettingsSnapshot(
    CURRENT_SETTINGS_VERSION,
    DEFAULT_EDITOR_SETTINGS,
    DEFAULT_APPEARANCE_SETTINGS,
)


class SettingsService:
    """Loads safe defaults and persists only validated application settings."""

    def __init__(self, store: SettingsStore) -> None:
        self._store = store

    def load(self) -> SettingsSnapshot:
        """Return valid settings without allowing malformed data to block startup."""
        try:
            candidate = self._store.load()
        except OSError:
            return DEFAULT_SETTINGS
        if candidate is None:
            return DEFAULT_SETTINGS
        return normalize_settings(candidate)

    def save(self, settings: SettingsSnapshot) -> SettingsSnapshot:
        """Normalize before writing and return the exact persisted contract."""
        normalized = normalize_settings(settings)
        self._store.save(normalized)
        return normalized


def normalize_settings(settings: SettingsSnapshot) -> SettingsSnapshot:
    """Fallback field-by-field for invalid values and unknown schema versions."""
    if type(settings.schema_version) is not int or settings.schema_version not in {
        1,
        2,
        CURRENT_SETTINGS_VERSION,
    }:
        return DEFAULT_SETTINGS
    editor = settings.editor
    font_family = (
        editor.font_family
        if isinstance(editor.font_family, str) and editor.font_family in SUPPORTED_EDITOR_FONTS
        else DEFAULT_EDITOR_SETTINGS.font_family
    )
    font_size = (
        editor.font_size
        if isinstance(editor.font_size, int)
        and not isinstance(editor.font_size, bool)
        and 8 <= editor.font_size <= 32
        else DEFAULT_EDITOR_SETTINGS.font_size
    )
    font_style = (
        editor.font_style
        if isinstance(editor.font_style, str) and editor.font_style in SUPPORTED_FONT_STYLES
        else DEFAULT_EDITOR_SETTINGS.font_style
    )
    wrap_lines = (
        editor.wrap_lines
        if isinstance(editor.wrap_lines, bool)
        else DEFAULT_EDITOR_SETTINGS.wrap_lines
    )
    show_line_numbers = (
        editor.show_line_numbers
        if isinstance(editor.show_line_numbers, bool)
        else DEFAULT_EDITOR_SETTINGS.show_line_numbers
    )
    appearance = settings.appearance
    locale = (
        appearance.locale
        if isinstance(appearance.locale, str) and appearance.locale in SUPPORTED_LOCALES
        else DEFAULT_APPEARANCE_SETTINGS.locale
    )
    theme = (
        appearance.theme
        if isinstance(appearance.theme, str) and appearance.theme in SUPPORTED_THEMES
        else DEFAULT_APPEARANCE_SETTINGS.theme
    )
    accent = (
        appearance.accent
        if isinstance(appearance.accent, str) and appearance.accent in SUPPORTED_ACCENTS
        else DEFAULT_APPEARANCE_SETTINGS.accent
    )
    ui_font_family = (
        appearance.ui_font_family
        if isinstance(appearance.ui_font_family, str)
        and appearance.ui_font_family in SUPPORTED_UI_FONTS
        else DEFAULT_APPEARANCE_SETTINGS.ui_font_family
    )
    ui_font_size = (
        appearance.ui_font_size
        if isinstance(appearance.ui_font_size, int)
        and not isinstance(appearance.ui_font_size, bool)
        and 9 <= appearance.ui_font_size <= 16
        else DEFAULT_APPEARANCE_SETTINGS.ui_font_size
    )
    ui_font_style = (
        appearance.ui_font_style
        if isinstance(appearance.ui_font_style, str)
        and appearance.ui_font_style in SUPPORTED_FONT_STYLES
        else DEFAULT_APPEARANCE_SETTINGS.ui_font_style
    )
    motion_enabled = (
        appearance.motion_enabled
        if isinstance(appearance.motion_enabled, bool)
        else DEFAULT_APPEARANCE_SETTINGS.motion_enabled
    )
    return SettingsSnapshot(
        CURRENT_SETTINGS_VERSION,
        EditorSettings(
            font_family=font_family,
            font_size=font_size,
            font_style=font_style,
            wrap_lines=wrap_lines,
            show_line_numbers=show_line_numbers,
        ),
        AppearanceSettings(
            locale=locale,
            theme=theme,
            accent=accent,
            ui_font_family=ui_font_family,
            ui_font_size=ui_font_size,
            ui_font_style=ui_font_style,
            motion_enabled=motion_enabled,
        ),
    )
