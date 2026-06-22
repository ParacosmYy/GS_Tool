"""log_empty_state 日志区空态 helper 边界测试。

模块此前无直接测试覆盖（grep 0 命中）。本文件覆盖：

1. build_log_empty_state 返回 EmptyStateWidget + icon inbox + 标题/描述文案。
2. hide_log_empty_state 对无 _log_empty_state 属性的 host 安全跳过。
3. hide_log_empty_state 对 None overlay 安全跳过。
4. hide_log_empty_state 调用 hide_with_fade（覆盖层淡出）。
5. show_log_empty_state 对无属性 / None 安全跳过。
6. show_log_empty_state 调用 show_with_fade（淡入）。
7. _LogEmptyStateHost Protocol 存在（类型契约）。
"""

from __future__ import annotations

import os
from unittest.mock import patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.ui.log_empty_state import (
    _LogEmptyStateHost,
    build_log_empty_state,
    hide_log_empty_state,
    show_log_empty_state,
)
from embeddebug.serial_station.ui.widgets import EmptyStateWidget


# ── build_log_empty_state ────────────────────────────────────────
def test_build_returns_empty_state_widget(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    empty = build_log_empty_state(parent)
    assert isinstance(empty, EmptyStateWidget)


def test_build_uses_inbox_icon(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    empty = build_log_empty_state(parent)
    # icon 通过 IconManager 加载，objectName 固定。
    assert empty._icon_label.objectName() == "serialStationEmptyStateIcon"


def test_build_title_and_description(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    empty = build_log_empty_state(parent)
    assert empty._title_label.text() == "暂无日志"
    assert "连接设备" in empty._desc_label.text()


# ── hide_log_empty_state 安全跳过 ────────────────────────────────
def test_hide_without_attribute_does_not_crash():
    """host 无 _log_empty_state 属性 → 安全跳过。"""

    host = SimpleNamespace()
    hide_log_empty_state(host)  # 不抛


def test_hide_with_none_overlay_skips():
    """host._log_empty_state = None → 安全跳过。"""

    host = SimpleNamespace(_log_empty_state=None)
    hide_log_empty_state(host)


# ── hide 调用 hide_with_fade ─────────────────────────────────────
def test_hide_calls_hide_with_fade(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    empty = build_log_empty_state(parent)
    host = SimpleNamespace(_log_empty_state=empty)
    with patch.object(empty, "hide_with_fade") as mock_hide:
        hide_log_empty_state(host)
        mock_hide.assert_called_once()


# ── show_log_empty_state 安全跳过 ────────────────────────────────
def test_show_without_attribute_does_not_crash():
    host = SimpleNamespace()
    show_log_empty_state(host)


def test_show_with_none_overlay_skips():
    host = SimpleNamespace(_log_empty_state=None)
    show_log_empty_state(host)


# ── show 调用 show_with_fade ─────────────────────────────────────
def test_show_calls_show_with_fade(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    empty = build_log_empty_state(parent)
    host = SimpleNamespace(_log_empty_state=empty)
    with patch.object(empty, "show_with_fade") as mock_show:
        show_log_empty_state(host)
        mock_show.assert_called_once()


# ── Protocol 契约 ─────────────────────────────────────────────────
def test_host_protocol_exists():
    """_LogEmptyStateHost Protocol 应可访问（类型契约）。"""

    assert _LogEmptyStateHost is not None
