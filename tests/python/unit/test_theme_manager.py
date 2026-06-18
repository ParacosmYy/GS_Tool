"""ThemeManager 单元测试 — 覆盖色板完整性、QSS 加载、应用与资源解析。"""

from __future__ import annotations

import re
from pathlib import Path

import pytest

from embeddebug.serial_station.ui.theme import (
    BUILTIN_THEMES,
    DEFAULT_THEME,
    ThemeManager,
    apply_theme,
    current_theme_name,
    palette_tokens,
    size_tokens,
)
from embeddebug.serial_station.ui.theme import palette as P
from embeddebug.serial_station.ui.theme.qss_builder import build_qss

REPO_ROOT = Path(__file__).resolve().parents[3]
UI_DIR = REPO_ROOT / "python" / "embeddebug" / "serial_station" / "ui"

EXPECTED_PALETTE_KEYS = (
    "bg_window",
    "bg_panel",
    "text_primary",
    "accent",
    "success",
    "warning",
    "error",
    "term_rx",
    "term_tx",
    "border",
    "scrollbar",
)
EXPECTED_SIZE_KEYS = (
    "radius_md",
    "spacing_md",
    "padding_md",
    "border_thin",
    "font_base",
    "font_family_mono",
)

# 冒烟测试硬断言的 9 个 objectName，必须被 QSS 覆盖。
SMOKE_CRITICAL_OBJECTNAMES = (
    "serialStationConnectButton",
    "serialStationSendEdit",
    "serialStationCommandHistoryCombo",
    "serialStationSendButton",
    "serialStationInjectEdit",
    "serialStationInjectButton",
    "serialStationClearButton",
    "serialStationLogView",
    "serialStationStatusLabel",
)


def _extract_serial_station_objectnames() -> set[str]:
    """扫描 ui/ 源码，提取所有 setObjectName(...) 中的 serialStation* 名称。"""

    pattern = re.compile(r'setObjectName\(\s*["\']([A-Za-z0-9_]+)["\']\s*\)')
    names: set[str] = set()
    for path in UI_DIR.rglob("*.py"):
        if "theme" in path.parts:
            continue
        text = path.read_text(encoding="utf-8")
        for match in pattern.finditer(text):
            name = match.group(1)
            if name.startswith("serialStation"):
                names.add(name)
    return names


# ── 色板与尺寸 token 完整性 ───────────────────────────────────────
def test_palette_tokens_contain_all_expected_keys():
    tokens = palette_tokens()
    for key in EXPECTED_PALETTE_KEYS:
        assert key in tokens, f"palette missing token: {key}"
    assert all(isinstance(v, str) and v for v in tokens.values())


def test_size_tokens_contain_all_expected_keys():
    tokens = size_tokens()
    for key in EXPECTED_SIZE_KEYS:
        assert key in tokens, f"size token missing: {key}"
    assert all(isinstance(v, str) and v for v in tokens.values())


def test_palette_constants_match_modern_dark_industrial_palette():
    """色板常量必须与 modern_dark.qss 的 PRD-071 工业风分区一致。"""

    assert P.BG_WINDOW == "#0d1118"
    assert P.ACCENT == "#22d3ee"
    assert P.SUCCESS == "#22c55e"
    assert P.WARNING == "#f59e0b"
    assert P.ERROR == "#ef4444"
    assert P.TERM_TX == "#38bdf8"
    assert P.TERM_RX == "#22c55e"


# ── QSS 生成器 ────────────────────────────────────────────────────
def test_build_qss_returns_non_empty_industrial_stylesheet():
    qss = build_qss()
    assert isinstance(qss, str)
    assert len(qss) > 1000
    assert qss.endswith("\n")


@pytest.mark.parametrize("object_name", SMOKE_CRITICAL_OBJECTNAMES)
def test_build_qss_covers_smoke_critical_objectnames(object_name):
    qss = build_qss()
    assert f"#{object_name}" in qss, f"QSS missing critical objectName: {object_name}"


def test_build_qss_embeds_three_button_states():
    qss = build_qss()
    for state in (":hover", ":pressed", ":disabled", ":focus"):
        assert state in qss, f"QSS missing button state: {state}"


def test_build_qss_embeds_industrial_accent_color():
    qss = build_qss()
    assert P.ACCENT in qss
    assert P.TERM_BACKGROUND in qss


# ── ThemeManager 单例与状态 ───────────────────────────────────────
def test_theme_manager_is_singleton():
    a = ThemeManager()
    b = ThemeManager()
    assert a is b


def test_theme_manager_reset_clears_current_theme():
    manager = ThemeManager()
    manager._current_theme = "stale"
    manager.reset()
    assert manager.current_theme is None


# ── apply_theme 端到端 ────────────────────────────────────────────
def test_apply_theme_applies_qss_to_application(qapp):
    manager = ThemeManager()
    manager.reset()
    qss = manager.apply_theme(qapp, DEFAULT_THEME)
    assert qss
    assert manager.current_theme == DEFAULT_THEME
    assert "#serialStationConnectButton" in qapp.styleSheet()


def test_apply_theme_convenience_function_updates_current_name(qapp):
    ThemeManager().reset()
    qss = apply_theme(qapp, DEFAULT_THEME)
    assert qss
    assert current_theme_name() == DEFAULT_THEME


def test_builtin_themes_contains_default():
    assert DEFAULT_THEME in BUILTIN_THEMES


# ── 外部 QSS 加载与回退 ───────────────────────────────────────────
def test_load_qss_prefers_external_file_when_present():
    """外部 serial_station_dark.qss 存在时应被加载（设计期编辑路径）。"""

    external = REPO_ROOT / "resources" / "themes" / f"{DEFAULT_THEME}.qss"
    if not external.is_file():
        pytest.skip(f"external theme file not present: {external}")
    loaded = ThemeManager().load_qss(DEFAULT_THEME)
    assert "serialStationConnectButton" in loaded


def test_resolve_resource_path_finds_themes_directory():
    resolved = ThemeManager.resolve_resource_path(Path("resources") / "themes")
    assert resolved.is_dir()
    assert (resolved / f"{DEFAULT_THEME}.qss").is_file()


def test_resolve_resource_path_finds_existing_resource_file():
    resolved = ThemeManager.resolve_resource_path(
        Path("resources") / "themes" / f"{DEFAULT_THEME}.qss"
    )
    assert resolved.is_file()


def test_resolve_resource_path_returns_candidate_for_missing_resource():
    resolved = ThemeManager.resolve_resource_path(
        Path("resources") / "nonexistent_xyz.qss"
    )
    assert not resolved.exists()
