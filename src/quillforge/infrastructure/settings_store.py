"""Atomic local JSON settings storage."""

import json
import os
import tempfile
from pathlib import Path

from ..application.ports import SettingsStore
from ..domain.models import AppearanceSettings, EditorSettings, SettingsSnapshot


class JsonSettingsStore(SettingsStore):
    """Persist one small versioned settings file in the user-local app directory."""

    def __init__(self, path: Path) -> None:
        self._path = path.expanduser().resolve()

    def load(self) -> SettingsSnapshot | None:
        try:
            payload = json.loads(self._path.read_text(encoding="utf-8"))
            return _decode_settings(payload)
        except (OSError, UnicodeError, TypeError, ValueError, json.JSONDecodeError):
            return None

    def save(self, settings: SettingsSnapshot) -> None:
        self._path.parent.mkdir(parents=True, exist_ok=True)
        temporary_path: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                dir=self._path.parent,
                prefix=f".{self._path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary_path = Path(temporary.name)
                json.dump(
                    _encode_settings(settings), temporary, ensure_ascii=False, separators=(",", ":")
                )
                temporary.flush()
                os.fsync(temporary.fileno())
            os.replace(temporary_path, self._path)
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass


def default_settings_path() -> Path:
    """Return the local settings path without depending on Qt."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / "settings.json"


def _encode_settings(settings: SettingsSnapshot) -> dict[str, object]:
    return {
        "schema_version": settings.schema_version,
        "editor": {
            "font_family": settings.editor.font_family,
            "font_size": settings.editor.font_size,
            "font_style": settings.editor.font_style,
            "wrap_lines": settings.editor.wrap_lines,
            "show_line_numbers": settings.editor.show_line_numbers,
        },
        "appearance": {
            "locale": settings.appearance.locale,
            "theme": settings.appearance.theme,
            "accent": settings.appearance.accent,
            "ui_font_family": settings.appearance.ui_font_family,
            "ui_font_size": settings.appearance.ui_font_size,
            "ui_font_style": settings.appearance.ui_font_style,
            "motion_enabled": settings.appearance.motion_enabled,
        },
    }


def _decode_settings(payload: object) -> SettingsSnapshot | None:
    if not isinstance(payload, dict):
        return None
    schema_version = payload.get("schema_version")
    editor = payload.get("editor")
    if type(schema_version) is not int or not isinstance(editor, dict):
        return None
    font_family = editor.get("font_family", EditorSettings().font_family)
    font_size = editor.get("font_size")
    font_style = editor.get("font_style", EditorSettings().font_style)
    wrap_lines = editor.get("wrap_lines")
    show_line_numbers = editor.get("show_line_numbers", EditorSettings().show_line_numbers)
    if (
        not isinstance(font_family, str)
        or type(font_size) is not int
        or not isinstance(font_style, str)
        or type(wrap_lines) is not bool
        or type(show_line_numbers) is not bool
    ):
        return None
    appearance_payload = payload.get("appearance", {})
    if not isinstance(appearance_payload, dict):
        return None
    appearance = AppearanceSettings(
        locale=appearance_payload.get("locale", AppearanceSettings().locale),
        theme=appearance_payload.get("theme", AppearanceSettings().theme),
        accent=appearance_payload.get("accent", AppearanceSettings().accent),
        ui_font_family=appearance_payload.get(
            "ui_font_family", AppearanceSettings().ui_font_family
        ),
        ui_font_size=appearance_payload.get("ui_font_size", AppearanceSettings().ui_font_size),
        ui_font_style=appearance_payload.get("ui_font_style", AppearanceSettings().ui_font_style),
        motion_enabled=appearance_payload.get(
            "motion_enabled", AppearanceSettings().motion_enabled
        ),
    )
    return SettingsSnapshot(
        schema_version=schema_version,
        editor=EditorSettings(
            font_family=font_family,
            font_size=font_size,
            font_style=font_style,
            wrap_lines=wrap_lines,
            show_line_numbers=show_line_numbers,
        ),
        appearance=appearance,
    )
