"""theme_store 常量契约 + _read_prefs_dict 迁移逻辑 + 路径结构单元测试。

补强 test_theme_core.py 未直接断言的边角：
- 常量：PREFS_FILENAME / _LEGACY_ACCENT_FILENAME / DEFAULT_THEME_ID / DEFAULT_ACCENT_ID / _SUBDIR。
- prefs_path / _legacy_accent_path：结构（含 embeddebug 子目录 + 文件名）。
- _read_prefs_dict：新文件优先 + 旧 accent.json 迁移 + 损坏 JSON 回退空 + 非顶层 dict 回退空。
- load_theme_prefs：缺 theme/accent 回退默认 + 非字符串回退默认。
- save_theme_prefs：合并写（None 保留旧值）+ 原子 .tmp 替换。
"""

from __future__ import annotations

import json


from embeddebug.serial_station.ui.theme import theme_store


# ── 常量契约 ─────────────────────────────────────────────────────────────


def test_prefs_filename_is_theme_prefs_json():
    """PREFS_FILENAME = theme_prefs.json（统一偏好文件）。"""

    assert theme_store.PREFS_FILENAME == "theme_prefs.json"


def test_legacy_accent_filename():
    """_LEGACY_ACCENT_FILENAME = accent.json（Batch 11 旧文件）。"""

    assert theme_store._LEGACY_ACCENT_FILENAME == "accent.json"


def test_default_theme_id_is_dark():
    """DEFAULT_THEME_ID = serial_station_dark。"""

    assert theme_store.DEFAULT_THEME_ID == "serial_station_dark"


def test_default_accent_id_is_cyan():
    """DEFAULT_ACCENT_ID = cyan。"""

    assert theme_store.DEFAULT_ACCENT_ID == "cyan"


def test_subdir_is_embeddebug():
    """_SUBDIR = embeddebug（应用数据子目录）。"""

    assert theme_store._SUBDIR == "embeddebug"


# ── prefs_path / _legacy_accent_path 结构 ────────────────────────────────


def test_prefs_path_ends_with_filename():
    """prefs_path() 以 theme_prefs.json 结尾。"""

    p = theme_store.prefs_path()
    assert p.name == "theme_prefs.json"


def test_prefs_path_contains_subdir():
    """prefs_path() 含 embeddebug 子目录。"""

    p = theme_store.prefs_path()
    assert "embeddebug" in p.parts


def test_legacy_accent_path_ends_with_filename():
    """_legacy_accent_path() 以 accent.json 结尾。"""

    p = theme_store._legacy_accent_path()
    assert p.name == "accent.json"


def test_prefs_and_legacy_share_parent():
    """prefs_path 和 _legacy_accent_path 同父目录。"""

    assert theme_store.prefs_path().parent == theme_store._legacy_accent_path().parent


# ── _read_prefs_dict 迁移逻辑 ────────────────────────────────────────────


def test_read_prefs_dict_new_file_takes_priority(tmp_path, monkeypatch):
    """新 theme_prefs.json 优先于旧 accent.json。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text(json.dumps({"theme": "light", "accent": "blue"}))
    legacy_path = tmp_path / "accent.json"
    legacy_path.write_text(json.dumps({"accent": "old_cyan"}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_path)
    result = theme_store._read_prefs_dict()
    assert result == {"theme": "light", "accent": "blue"}


def test_read_prefs_dict_migrates_legacy_accent(tmp_path, monkeypatch):
    """新文件不存在 + 旧 accent.json 存在 → 迁移 accent。"""

    new_path = tmp_path / "theme_prefs.json"
    legacy_path = tmp_path / "accent.json"
    legacy_path.write_text(json.dumps({"accent": "purple"}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_path)
    result = theme_store._read_prefs_dict()
    assert result == {"accent": "purple"}


def test_read_prefs_dict_neither_file_returns_empty(tmp_path, monkeypatch):
    """新/旧文件都不存在 → 空 dict。"""

    monkeypatch.setattr(theme_store, "prefs_path", lambda: tmp_path / "theme_prefs.json")
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    assert theme_store._read_prefs_dict() == {}


def test_read_prefs_dict_corrupt_new_returns_empty(tmp_path, monkeypatch):
    """新文件损坏 JSON → 空 dict（不迁移旧文件）。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text("{not json")
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    assert theme_store._read_prefs_dict() == {}


def test_read_prefs_dict_corrupt_legacy_returns_empty(tmp_path, monkeypatch):
    """新文件不存在 + 旧 accent.json 损坏 → 空 dict。"""

    legacy_path = tmp_path / "accent.json"
    legacy_path.write_text("{broken")
    monkeypatch.setattr(theme_store, "prefs_path", lambda: tmp_path / "theme_prefs.json")
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_path)
    assert theme_store._read_prefs_dict() == {}


def test_read_prefs_dict_new_not_dict_returns_empty(tmp_path, monkeypatch):
    """新文件 JSON 是数组（非 dict）→ 空 dict。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text(json.dumps(["not", "a", "dict"]))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    assert theme_store._read_prefs_dict() == {}


def test_read_prefs_dict_legacy_empty_accent_skipped(tmp_path, monkeypatch):
    """旧 accent.json accent 值为空串 → 不迁移（跳过）。"""

    legacy_path = tmp_path / "accent.json"
    legacy_path.write_text(json.dumps({"accent": ""}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: tmp_path / "theme_prefs.json")
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_path)
    assert theme_store._read_prefs_dict() == {}


# ── load_theme_prefs 回退默认 ────────────────────────────────────────────


def test_load_theme_prefs_missing_returns_defaults(tmp_path, monkeypatch):
    """无文件 → theme=dark / accent=cyan 默认。"""

    monkeypatch.setattr(theme_store, "prefs_path", lambda: tmp_path / "theme_prefs.json")
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    prefs = theme_store.load_theme_prefs()
    assert prefs == {"theme": "serial_station_dark", "accent": "cyan"}


def test_load_theme_prefs_non_string_values_default(tmp_path, monkeypatch):
    """theme/accent 是非字符串（如 int）→ 回退默认。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text(json.dumps({"theme": 123, "accent": None}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_dark"
    assert prefs["accent"] == "cyan"


def test_load_theme_prefs_empty_strings_default(tmp_path, monkeypatch):
    """theme/accent 是空串 → 回退默认。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text(json.dumps({"theme": "", "accent": ""}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    prefs = theme_store.load_theme_prefs()
    assert prefs["theme"] == "serial_station_dark"
    assert prefs["accent"] == "cyan"


# ── save_theme_prefs 合并写 ──────────────────────────────────────────────


def test_save_theme_prefs_none_preserves_old(tmp_path, monkeypatch):
    """save_theme_prefs(None, None) 保留旧值不变。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text(json.dumps({"theme": "light", "accent": "purple"}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    assert theme_store.save_theme_prefs() is True
    saved = json.loads(new_path.read_text(encoding="utf-8"))
    assert saved == {"theme": "light", "accent": "purple"}


def test_save_theme_prefs_partial_update(tmp_path, monkeypatch):
    """save_theme_prefs(theme=...) 只更新 theme，保留 accent。"""

    new_path = tmp_path / "theme_prefs.json"
    new_path.write_text(json.dumps({"theme": "dark", "accent": "purple"}))
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    theme_store.save_theme_prefs(theme="serial_station_light")
    saved = json.loads(new_path.read_text(encoding="utf-8"))
    assert saved["theme"] == "serial_station_light"
    assert saved["accent"] == "purple"


def test_save_theme_prefs_creates_parent_dir(tmp_path, monkeypatch):
    """save_theme_prefs 自动创建不存在的父目录。"""

    new_path = tmp_path / "subdir" / "theme_prefs.json"
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    assert theme_store.save_theme_prefs(theme="dark") is True
    assert new_path.exists()


def test_save_theme_prefs_no_tmp_file_left(tmp_path, monkeypatch):
    """原子写后无 .tmp 残留文件。"""

    new_path = tmp_path / "theme_prefs.json"
    monkeypatch.setattr(theme_store, "prefs_path", lambda: new_path)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: tmp_path / "accent.json")
    theme_store.save_theme_prefs(theme="dark")
    assert not (tmp_path / "theme_prefs.json.tmp").exists()
