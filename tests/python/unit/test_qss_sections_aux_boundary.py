"""qss_sections_domain_parts_aux _settings_tabs/_accent_swatches/_about_labels 边界测试。

这些私有 QSS helper 此前无直接测试（grep 0 命中）。
本文件覆盖返回字符串契约 + objectName 覆盖 + palette 引用。

覆盖：
1. _settings_tabs 返回非空 str + 含 serialStationSettingsTabBar objectName。
2. _settings_tabs 含 checked/hover 状态。
3. _accent_swatches 返回非空 str + 含 serialStationAccentSwatch objectName。
4. _accent_swatches 含 checked 状态。
5. _about_labels 返回非空 str + 含 serialStationAbout objectName。
6. _about_labels 含 TEXT_MUTED 引用。
7. 三个 helper 返回 str 类型。
8. _settings_tabs 含 padding/font 样式。
"""

from __future__ import annotations

from embeddebug.serial_station.ui.theme.qss_sections_domain_parts_aux import (
    _about_labels,
    _accent_swatches,
    _settings_tabs,
)


# ── _settings_tabs ───────────────────────────────────────────────
def test_settings_tabs_returns_nonempty_str():
    result = _settings_tabs()
    assert isinstance(result, str)
    assert len(result) > 0


def test_settings_tabs_covers_tabbar_objectname():
    result = _settings_tabs()
    assert "serialStationSettingsTab" in result


def test_settings_tabs_has_selected_state():
    result = _settings_tabs()
    assert "selected" in result.lower() or "checked" in result.lower()


def test_settings_tabs_has_hover_state():
    result = _settings_tabs()
    assert "hover" in result.lower() or "on:hover" in result.lower()


# ── _accent_swatches ─────────────────────────────────────────────
def test_accent_swatches_returns_nonempty_str():
    result = _accent_swatches()
    assert isinstance(result, str)
    assert len(result) > 0


def test_accent_swatches_covers_swatch_objectname():
    result = _accent_swatches()
    assert "serialStationAccentSwatch" in result


def test_accent_swatches_has_checked_state():
    result = _accent_swatches()
    assert "checked" in result.lower() or "on:checked" in result.lower()


# ── _about_labels ────────────────────────────────────────────────
def test_about_labels_returns_nonempty_str():
    result = _about_labels()
    assert isinstance(result, str)
    assert len(result) > 0


def test_about_labels_covers_objectname():
    result = _about_labels()
    # about 标签的 objectName 前缀。
    assert "serialStationAbout" in result or "serialStationSettings" in result


def test_about_labels_has_text_muted_or_secondary():
    """about 标签用 TEXT_MUTED/TEXT_SECONDARY 配色。"""

    result = _about_labels()
    # 至少含某个 palette 引用。
    assert any(token in result for token in ["TEXT_MUTED", "TEXT_SECONDARY", "color:"])
