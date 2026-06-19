"""强调色选择持久化测试（accent_store → theme_store shim）。

Batch 12 起 accent_store 转为 theme_store 的薄 shim，真实落点是
``theme_store.prefs_path()``（``theme_prefs.json``）。测试隔离改为 monkeypatch
``theme_store.prefs_path``，并把 ``_legacy_accent_path`` 也一并重定向到 tmp_path
（避免迁移回退误读开发机旧 accent.json）。

覆盖：
- save/load 往返（round-trip）：保存的 id 能完整读回。
- 缺失文件 / 损坏 JSON / 非法字段 → 回退 cyan 默认。
- 原子写入（``.tmp`` + os.replace）：保存后文件存在且内容正确。
- restore_active_accent 集成：从磁盘恢复后 get_active_accent_id 一致。
"""

from __future__ import annotations

import json
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.theme import accent_store, accents, theme_store


def _isolate(monkeypatch, tmp_path):
    """把 theme_store 的落点（新 prefs + 旧 legacy accent）重定向到 tmp_path。"""

    prefs_target = tmp_path / "embeddebug" / theme_store.PREFS_FILENAME
    legacy_target = tmp_path / "embeddebug" / accent_store.ACCENT_FILENAME
    monkeypatch.setattr(theme_store, "prefs_path", lambda: prefs_target)
    monkeypatch.setattr(theme_store, "_legacy_accent_path", lambda: legacy_target)
    return prefs_target


def test_save_load_roundtrip(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    assert accent_store.save_accent_id("purple") is True
    assert target.exists()
    assert accent_store.load_accent_id() == "purple"


def test_load_missing_file_defaults_to_cyan(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    assert accent_store.load_accent_id() == "cyan"


def test_load_corrupt_json_defaults(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text("{ not valid json", encoding="utf-8")
    assert accent_store.load_accent_id() == "cyan"


def test_load_missing_accent_field_defaults(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps({"other": "value"}), encoding="utf-8")
    assert accent_store.load_accent_id() == "cyan"


def test_load_empty_accent_string_defaults(monkeypatch, tmp_path):
    target = _isolate(monkeypatch, tmp_path)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps({"accent": ""}), encoding="utf-8")
    assert accent_store.load_accent_id() == "cyan"


def test_save_is_atomic(monkeypatch, tmp_path):
    """保存后不应残留 .tmp 文件（os.replace 已替换）。"""

    target = _isolate(monkeypatch, tmp_path)
    accent_store.save_accent_id("green")
    assert not (target.with_suffix(target.suffix + ".tmp")).exists()


def test_custom_default_param(monkeypatch, tmp_path):
    _isolate(monkeypatch, tmp_path)
    # 缺失文件时，default 参数应被尊重。
    assert accent_store.load_accent_id(default="blue") == "blue"


def test_restore_active_accent_from_disk(monkeypatch, tmp_path):
    """restore_active_accent 读盘后 get_active_accent_id 一致；且不重复持久化。"""

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
    """set_active_accent 默认 persist=True，应落盘。"""

    target = _isolate(monkeypatch, tmp_path)
    accents.reset_active_accent()
    accents.set_active_accent("teal")  # persist=True 默认
    assert accent_store.load_accent_id() == "teal"
    assert target.exists()
    accents.reset_active_accent()


def test_set_active_accent_persist_false_skips_disk(monkeypatch, tmp_path):
    """persist=False 不写盘（启动恢复 / 测试场景）。"""

    target = _isolate(monkeypatch, tmp_path)
    accents.reset_active_accent()
    accents.set_active_accent("rose", persist=False)
    assert not target.exists()
    assert accents.get_active_accent_id() == "rose"
    accents.reset_active_accent()
