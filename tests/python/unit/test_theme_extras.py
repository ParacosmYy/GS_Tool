"""多强调色变体 + Ctrl+Shift+T 主题快捷键 + accent_store 持久化测试。"""
from __future__ import annotations
import json
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
from PyQt6.QtCore import QEvent, Qt
from PyQt6.QtGui import QKeyEvent
from embeddebug.serial_station.shortcuts.definitions import DEFAULT_SHORTCUTS
from embeddebug.serial_station.ui import shortcuts, theme_actions
from embeddebug.serial_station.ui.theme import accent_store, accents, theme_store
from embeddebug.serial_station.ui.theme import palette as dark
from embeddebug.serial_station.ui.theme import palette_light as light
from embeddebug.serial_station.ui.theme.qss_builder import apply_accent_recolor, build_qss
class _FakeHost:
    def __init__(self) -> None:
        self.toggled = 0
    def _send_text(self) -> None: ...
    def _clear_log(self) -> None: ...
    def _refresh_serial_ports(self) -> None: ...
    def _open_command_palette(self) -> None: ...
    def _toggle_theme(self) -> None:
        self.toggled += 1
def _key(key: Qt.Key, modifiers: Qt.KeyboardModifier) -> QKeyEvent:
    return QKeyEvent(QEvent.Type.KeyPress, int(key), modifiers)
def _isolate(monkeypatch, tmp_path):
    prefs_target = tmp_path / "embeddebug" / theme_store.PREFS_FILENAME
    legacy_target = tmp_path / "embeddebug" / accent_store.ACCENT_FILENAME
    monkeypatch.setattr(theme_store, "prefs_path", lambda: prefs_target)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_target)
    return prefs_target
def test_accents_has_seven_variants():
    assert len(accents.ACCENTS) == 7
def test_accent_ids_are_unique():
    ids = [a.id for a in accents.ACCENTS]
    assert len(ids) == len(set(ids))
def test_cyan_is_first_and_default():
    assert accents.ACCENTS[0].id == "cyan"
    assert accents.DEFAULT_ACCENT_ID == "cyan"
def test_accent_tones_fields_complete():
    for variant in accents.ACCENTS:
        for tones in (variant.dark, variant.light):
            assert tones.base.startswith("#")
            assert tones.hover.startswith("#")
            assert tones.pressed.startswith("#")
            assert tones.soft.startswith("rgba(")
            assert tones.border.startswith("rgba(")
            assert tones.gradient_from.startswith("#")
            assert tones.gradient_to.startswith("#")
def test_accent_gradient_from_differs_from_to():
    for variant in accents.ACCENTS:
        for tones in (variant.dark, variant.light):
            assert tones.gradient_from != tones.gradient_to
def test_get_accent_by_id_falls_back_to_cyan():
    assert accents.get_accent_by_id("nonexistent").id == "cyan"
def test_cyan_dark_tones_match_palette():
    cyan = accents.ACCENTS[0]
    assert cyan.dark.base == dark.ACCENT
    assert cyan.dark.hover == dark.ACCENT_HOVER
    assert cyan.dark.pressed == dark.ACCENT_PRESSED
    assert cyan.dark.soft == dark.ACCENT_SOFT
    assert cyan.dark.border == dark.ACCENT_BORDER
    assert cyan.dark.gradient_from == dark.ACCENT_GRADIENT_FROM
    assert cyan.dark.gradient_to == dark.ACCENT_GRADIENT_TO
def test_cyan_light_tones_match_palette_light():
    cyan = accents.ACCENTS[0]
    assert cyan.light.base == light.ACCENT
    assert cyan.light.hover == light.ACCENT_HOVER
    assert cyan.light.pressed == light.ACCENT_PRESSED
    assert cyan.light.soft == light.ACCENT_SOFT
    assert cyan.light.border == light.ACCENT_BORDER
    assert cyan.light.gradient_from == light.ACCENT_GRADIENT_FROM
    assert cyan.light.gradient_to == light.ACCENT_GRADIENT_TO
def test_apply_accent_recolor_cyan_is_identity():
    qss = build_qss()
    cyan = accents.get_accent_by_id("cyan")
    assert apply_accent_recolor(qss, cyan, is_light=False) == qss
def test_apply_accent_recolor_blue_changes_base():
    qss = build_qss()
    blue = accents.get_accent_by_id("blue")
    recolored = apply_accent_recolor(qss, blue, is_light=False)
    assert blue.dark.base in recolored
    assert recolored.count(dark.ACCENT) < qss.count(dark.ACCENT)
def test_apply_accent_recolor_preserves_non_accent_tokens():
    qss = build_qss()
    blue = accents.get_accent_by_id("blue")
    recolored = apply_accent_recolor(qss, blue, is_light=False)
    assert dark.BG_WINDOW in recolored
    assert dark.TEXT_PRIMARY in recolored
def test_apply_accent_recolor_light_path():
    from embeddebug.serial_station.ui.theme.theme_switcher import build_light_qss
    light_qss = build_light_qss()
    blue = accents.get_accent_by_id("blue")
    recolored = apply_accent_recolor(light_qss, blue, is_light=True)
    assert blue.light.base in recolored
def test_get_active_accent_defaults_to_cyan():
    accents.reset_active_accent()
    assert accents.get_active_accent_id() == "cyan"
def test_set_and_get_active_accent():
    accents.reset_active_accent()
    accents.set_active_accent("purple", persist=False)
    assert accents.get_active_accent_id() == "purple"
    accents.reset_active_accent()
def test_set_active_accent_cyan_clears_override():
    accents.reset_active_accent()
    accents.set_active_accent("purple", persist=False)
    assert accents.get_active_accent_id() == "purple"
    accents.set_active_accent("cyan", persist=False)
    assert accents.get_active_accent_id() == "cyan"
    accents.reset_active_accent()
def test_tones_for_is_light():
    cyan = accents.get_accent_by_id("cyan")
    assert cyan.tones_for(is_light=False) is cyan.dark
    assert cyan.tones_for(is_light=True) is cyan.light
def test_toggle_theme_definition_exists():
    defs = {d.id: d for d in DEFAULT_SHORTCUTS}
    assert "toggle_theme" in defs
    assert defs["toggle_theme"].callback_name == "toggle_theme"
    assert defs["toggle_theme"].default_key_sequence == "Ctrl+Shift+T"
def test_ctrl_shift_t_dispatches_to_toggle_theme():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.ControlModifier | Qt.KeyboardModifier.ShiftModifier)
    assert shortcuts.handle_key_press(host, event) is True
    assert host.toggled == 1
    assert event.isAccepted()
def test_ctrl_t_without_shift_does_not_toggle():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.ControlModifier)
    assert shortcuts.handle_key_press(host, event) is False
    assert host.toggled == 0
def test_shift_t_without_ctrl_does_not_toggle():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.ShiftModifier)
    assert shortcuts.handle_key_press(host, event) is False
    assert host.toggled == 0
def test_plain_t_does_not_toggle():
    host = _FakeHost()
    event = _key(Qt.Key.Key_T, Qt.KeyboardModifier.NoModifier)
    assert shortcuts.handle_key_press(host, event) is False
    assert host.toggled == 0
def test_other_ctrl_keys_unaffected_by_toggle_branch():
    host = _FakeHost()
    event = _key(Qt.Key.Key_L, Qt.KeyboardModifier.ControlModifier)
    assert shortcuts.handle_key_press(host, event) is True
    assert host.toggled == 0
def test_main_window_toggle_theme_delegates_to_theme_actions():
    import inspect
    from embeddebug.serial_station.ui import main_window
    source = inspect.getsource(main_window.SerialStationMainWindow._toggle_theme)
    assert "theme_actions.toggle_theme(self)" in source
def test_theme_actions_toggle_switches_theme(qapp, monkeypatch, tmp_path):
    from embeddebug.serial_station.ui.theme import theme_store
    prefs = tmp_path / "embeddebug" / theme_store.PREFS_FILENAME
    legacy = tmp_path / "embeddebug" / "accent.json"
    monkeypatch.setattr(theme_store, "prefs_path", lambda: prefs)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy)
    from embeddebug.serial_station.ui.theme.manager import ThemeManager
    ThemeManager()._current_theme = "serial_station_dark"
    from PyQt6.QtCore import QEventLoop, QTimer
    theme_actions.toggle_theme(qapp)
    loop = QEventLoop()
    QTimer.singleShot(600, loop.quit)
    loop.exec()
    assert ThemeManager().current_theme == "serial_station_light"
    assert theme_store.load_theme_id() == "serial_station_light"
def test_accent_save_load_roundtrip(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    assert accent_store.save_accent_id("purple") is True
    assert target.exists()
    assert accent_store.load_accent_id() == "purple"
def test_accent_load_missing_file_defaults_to_cyan(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert accent_store.load_accent_id() == "cyan"
def test_accent_load_corrupt_json_defaults(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text("{ not valid json", encoding="utf-8")
    assert accent_store.load_accent_id() == "cyan"
def test_accent_load_missing_field_defaults(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps({"other": "value"}), encoding="utf-8")
    assert accent_store.load_accent_id() == "cyan"
def test_accent_load_empty_string_defaults(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps({"accent": ""}), encoding="utf-8")
    assert accent_store.load_accent_id() == "cyan"
def test_accent_save_is_atomic(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    accent_store.save_accent_id("green")
    assert not (target.with_suffix(target.suffix + ".tmp")).exists()
def test_accent_custom_default_param(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert accent_store.load_accent_id(default="blue") == "blue"
def test_restore_active_accent_from_disk(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    accent_store.save_accent_id("amber")
    accents.reset_active_accent()
    restored = accents.restore_active_accent()
    assert restored == "amber"
    assert accents.get_active_accent_id() == "amber"
    accents.reset_active_accent()
def test_restore_active_accent_missing_defaults_cyan(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    accents.reset_active_accent()
    restored = accents.restore_active_accent()
    assert restored == "cyan"
    assert accents.get_active_accent_id() == "cyan"
def test_set_active_accent_persists_by_default(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    accents.reset_active_accent()
    accents.set_active_accent("teal")
    assert accent_store.load_accent_id() == "teal"
    assert target.exists()
    accents.reset_active_accent()
def test_set_active_accent_persist_false_skips_disk(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    accents.reset_active_accent()
    accents.set_active_accent("rose", persist=False)
    assert not target.exists()
    assert accents.get_active_accent_id() == "rose"
    accents.reset_active_accent()
