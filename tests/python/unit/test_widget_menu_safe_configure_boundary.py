"""_dashboard_widget_menu _safe_configure + _is_locked + _toggle_lock 边界测试。

_safe_configure 此前无直接测试。
本文件覆盖 None 回调安全 + 异常吞 + 正常调用 + _is_locked 默认。

覆盖：
1. _safe_configure None 回调 → 不崩。
2. _safe_configure 正常回调 → 调用。
3. _safe_configure 回调抛异常 → 不崩（吞）。
4. _safe_configure 传 item_id 到回调。
5. _is_locked 默认 False（无 config）。
6. _is_locked 有 config locked=True → True。
7. _toggle_lock 翻转。
8. _toggle_lock 未知 item → 不崩。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from types import SimpleNamespace
from unittest.mock import MagicMock

from embeddebug.serial_station.ui.panels._dashboard_widget_menu import (
    _is_locked,
    _safe_configure,
    _toggle_lock,
)


# ── _safe_configure ──────────────────────────────────────────────
def test_safe_configure_none_callback():
    _safe_configure(None, "item1")  # 不崩


def test_safe_configure_normal_callback():
    calls = []
    _safe_configure(lambda item_id: calls.append(item_id), "item42")
    assert calls == ["item42"]


def test_safe_configure_exception_swallowed():
    def _boom(item_id):
        raise RuntimeError("boom")

    _safe_configure(_boom, "item1")  # 不崩


def test_safe_configure_mock_callback():
    cb = MagicMock()
    _safe_configure(cb, "abc")
    cb.assert_called_once_with("abc")


# ── _is_locked ───────────────────────────────────────────────────
def test_is_locked_no_config():
    """canvas 无 items → _is_locked False。"""

    canvas = SimpleNamespace(items={})
    assert _is_locked(canvas, "item1") is False


def test_is_locked_locked_true():
    """canvas item config locked=True → True。"""

    canvas = SimpleNamespace(items={"item1": SimpleNamespace(config={"locked": True})})
    assert _is_locked(canvas, "item1") is True


def test_is_locked_unlocked():
    """canvas item config locked=False → False。"""

    canvas = SimpleNamespace(items={"item1": SimpleNamespace(config={"locked": False})})
    assert _is_locked(canvas, "item1") is False


# ── _toggle_lock ─────────────────────────────────────────────────
def test_toggle_lock_flips():
    """_toggle_lock 翻转 locked 状态。"""

    canvas = SimpleNamespace(items={"item1": SimpleNamespace(config={"locked": False})})
    _toggle_lock(canvas, "item1")
    assert _is_locked(canvas, "item1") is True


def test_toggle_lock_unknown_item_no_crash():
    """_toggle_lock 未知 item → 不崩。"""

    canvas = SimpleNamespace(items={})
    _toggle_lock(canvas, "nonexistent")  # 不崩
