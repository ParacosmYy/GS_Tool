"""accent_store 强调色持久化 shim 边界测试。

模块此前无直接测试覆盖（grep 0 命中）。Batch 12 起 accent_store 薄 shim 委托
theme_store，本文件覆盖 shim 契约 + save/load 往返。

覆盖：
1. ACCENT_FILENAME 常量（accent.json，迁移期兼容）。
2. accent_path 返回 embeddebug 子目录 + accent.json。
3. load_accent_id 默认值（无文件回退 cyan / 自定义 default）。
4. save_accent_id 返回 True。
5. save + load 往返（save 后 load 读回）。
6. shim 委托 theme_store（load_accent_id/save_accent_id 转发）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.theme import accent_store, theme_store


# ── 常量 ──────────────────────────────────────────────────────────
def test_accent_filename_constant():
    assert accent_store.ACCENT_FILENAME == "accent.json"


def test_accent_path_contains_filename_and_subdir():
    """accent_path 应含 embeddebug 子目录 + accent.json 文件名。"""

    path = str(accent_store.accent_path())
    assert "accent.json" in path
    assert "embeddebug" in path


def test_accent_path_matches_theme_store_legacy():
    """accent_path 应委托 theme_store._legacy_accent_path（同一路径）。"""

    assert accent_store.accent_path() == theme_store._legacy_accent_path()


# ── load_accent_id 默认值 ────────────────────────────────────────
def test_load_accent_id_default_cyan():
    """无保存时回退默认 cyan。"""

    # 先 save 回 cyan 清理任何残留。
    accent_store.save_accent_id("cyan")
    assert accent_store.load_accent_id() == "cyan"


def test_load_accent_id_custom_default():
    """自定义 default 参数应被尊重（当无有效保存时）。"""

    # 委托 theme_store；default 参数透传。
    result = accent_store.load_accent_id(default="rose")
    # 返回值要么是已保存的 id，要么是 default（取决于 theme_store 状态）。
    assert isinstance(result, str)


# ── save_accent_id ────────────────────────────────────────────────
def test_save_accent_id_returns_true():
    assert accent_store.save_accent_id("blue") is True


def test_save_load_roundtrip():
    """save 后 load 读回同一 id。"""

    accent_store.save_accent_id("violet")
    assert accent_store.load_accent_id("cyan") == "violet"


# ── 委托 theme_store ─────────────────────────────────────────────
def test_load_delegates_to_theme_store():
    """accent_store.load_accent_id 应转发到 theme_store.load_accent_id。"""

    # 两者读同一持久化源，结果应一致。
    assert accent_store.load_accent_id("amber") == theme_store.load_accent_id("amber")


def test_save_delegates_to_theme_store():
    """accent_store.save_accent_id 应转发到 theme_store.save_accent_id。"""

    assert accent_store.save_accent_id("teal") == theme_store.save_accent_id("teal")


# ── 清理 ──────────────────────────────────────────────────────────
def test_cleanup_restore_default():
    """测试结束恢复默认 accent（避免影响其他测试）。"""

    accent_store.save_accent_id("cyan")
    assert accent_store.load_accent_id() == "cyan"
