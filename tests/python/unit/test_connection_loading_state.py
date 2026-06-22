"""connection_loading + log_loading_state 两个加载态 helper 边界测试。

两个模块均无直接测试覆盖（grep 0 命中）。本文件覆盖：

connection_loading.set_button_loading：
1. button=None 安全跳过（不抛）。
2. loading=True：缓存原文字 + 禁用 + 文字清空 + 内嵌 ProgressRing（indeterminate + objectName + 18×18）。
3. loading=False：恢复文字 + 启用 + ring hide。
4. 重复 enter 不创建第二个 ring（复用 _loading_ring）。
5. _RING_SIZE 常量契约。

log_loading_state：
6. build_log_loading_state 返回 LogLoadingOverlay + objectName + 子控件（skeleton + label）。
7. show/hide 对无 _log_loading_state 的 host 安全跳过。
8. show/hide 切换 overlay 可见性。
9. overlay 初始 hide。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace

from PyQt6.QtWidgets import QPushButton, QWidget

from embeddebug.serial_station.ui.connection_loading import (
    _RING_SIZE,
    set_button_loading,
)
from embeddebug.serial_station.ui.controls import ProgressRing
from embeddebug.serial_station.ui.log_loading_state import (
    LogLoadingOverlay,
    build_log_loading_state,
    hide_log_loading_state,
    show_log_loading_state,
)


# ── connection_loading 常量 ──────────────────────────────────────
def test_ring_size_constant():
    assert _RING_SIZE == 18


# ── set_button_loading None 安全 ─────────────────────────────────
def test_set_button_loading_none_does_not_crash():
    """button=None 安全跳过（对齐原 _set_loading 契约）。"""

    set_button_loading(None, True)
    set_button_loading(None, False)  # 不抛异常


# ── loading=True ──────────────────────────────────────────────────
def test_enter_loading_caches_text_and_disables(qtbot):
    btn = QPushButton("连接")
    qtbot.addWidget(btn)
    set_button_loading(btn, True)
    assert btn._loading_orig_text == "连接"
    assert btn.isEnabled() is False
    assert btn.text() == ""  # 文字清空避免与 ring 重叠


def test_enter_loading_creates_indeterminate_ring(qtbot):
    btn = QPushButton("连接")
    qtbot.addWidget(btn)
    set_button_loading(btn, True)
    ring = btn._loading_ring
    assert isinstance(ring, ProgressRing)
    assert ring.objectName() == "serialStationButtonLoadingRing"
    assert ring.isIndeterminate() is True
    assert ring.width() == _RING_SIZE and ring.height() == _RING_SIZE


def test_enter_loading_reuses_existing_ring(qtbot):
    """重复 enter 不创建第二个 ring（复用 _loading_ring）。"""

    btn = QPushButton("连接")
    qtbot.addWidget(btn)
    set_button_loading(btn, True)
    ring1 = btn._loading_ring
    set_button_loading(btn, True)  # 再次进入
    assert btn._loading_ring is ring1  # 同一对象


# ── loading=False ─────────────────────────────────────────────────
def test_exit_loading_restores_text_and_enables(qtbot):
    btn = QPushButton("刷新")
    qtbot.addWidget(btn)
    set_button_loading(btn, True)
    set_button_loading(btn, False)
    assert btn.isEnabled() is True
    assert btn.text() == "刷新"
    assert btn._loading_orig_text is None


def test_exit_loading_hides_ring(qtbot):
    btn = QPushButton("刷新")
    qtbot.addWidget(btn)
    set_button_loading(btn, True)
    ring = btn._loading_ring
    set_button_loading(btn, False)
    assert ring.isHidden()


def test_exit_without_enter_does_not_crash(qtbot):
    """未进入加载态直接 exit：无 _loading_orig_text 不抛。"""

    btn = QPushButton("连接")
    qtbot.addWidget(btn)
    set_button_loading(btn, False)  # 无缓存文字，安全
    assert btn.isEnabled() is True


# ── log_loading_state ────────────────────────────────────────────
def test_build_log_loading_state_returns_overlay(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = build_log_loading_state(parent)
    assert isinstance(overlay, LogLoadingOverlay)
    assert overlay.objectName() == "serialStationLogLoadingOverlay"


def test_overlay_initially_hidden(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = build_log_loading_state(parent)
    assert overlay.isHidden()


def test_overlay_has_skeleton_and_label(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = build_log_loading_state(parent)
    assert overlay._skeleton.objectName() == "serialStationLogLoadingSkeleton"
    assert overlay._label.objectName() == "serialStationLogLoadingLabel"


def test_show_log_loading_state_without_overlay_does_not_crash():
    """host 无 _log_loading_state 属性 → 安全跳过。"""

    host = SimpleNamespace()
    show_log_loading_state(host)  # 不抛
    hide_log_loading_state(host)


def test_show_hide_toggles_overlay_visibility(qtbot):
    parent = QWidget()
    qtbot.addWidget(parent)
    overlay = build_log_loading_state(parent)
    overlay.hide()
    host = SimpleNamespace(_log_loading_state=overlay)

    show_log_loading_state(host)
    assert not overlay.isHidden()

    hide_log_loading_state(host)
    assert overlay.isHidden()


def test_show_log_loading_state_none_overlay_skips():
    """host._log_loading_state = None → 安全跳过。"""

    host = SimpleNamespace(_log_loading_state=None)
    show_log_loading_state(host)  # 不抛
    hide_log_loading_state(host)
