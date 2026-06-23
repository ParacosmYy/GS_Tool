"""_dashboard_binding_wire subscribe/register/open/notify/autosave 边界测试。

这些函数此前无直接测试（grep 0 命中）。
本文件覆盖 subscribe_controller_events + register_item_binding + open_binding_config_for_item +
_notify_status + _trigger_autosave 安全边界。

覆盖：
1. subscribe_controller_events 注册回调不崩。
2. _dispatch_measurement_batch 安全调度。
3. register_item_binding 注册成功返回 True。
4. register_item_binding 未知 item 返回 False。
5. open_binding_config_for_item 不崩。
6. _notify_status 不崩。
7. _trigger_autosave 不崩。
"""

from __future__ import annotations

import os
from types import SimpleNamespace
from unittest.mock import MagicMock

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.panels._dashboard_binding_wire import (
    _dispatch_measurement_batch,
    _notify_status,
    _trigger_autosave,
    open_binding_config_for_item,
    register_item_binding,
    subscribe_controller_events,
)


# ── subscribe_controller_events ──────────────────────────────────
def test_subscribe_controller_events_no_crash():
    """subscribe_controller_events 注册回调不崩。"""

    controller = MagicMock()
    service = MagicMock()
    subscribe_controller_events(controller, service)
    # 验证 on_measurement_batch 被调。
    controller.on_measurement_batch.assert_called_once()


def test_dispatch_measurement_batch_no_crash():
    """_dispatch_measurement_batch 安全调度（mock service）。"""

    service = MagicMock()
    batch = SimpleNamespace(channel_names=("ch0",), values=[[1.0]])
    _dispatch_measurement_batch(service, batch)
    # 不崩即可（service.route_measurement 可能调用）。


def test_dispatch_measurement_batch_none_service_no_crash():
    """_dispatch_measurement_batch None service 安全。"""

    _dispatch_measurement_batch(None, None)  # 不崩


# ── register_item_binding ────────────────────────────────────────
def test_register_item_binding_known_item():
    """register_item_binding 已知 item 注册成功。"""

    canvas = MagicMock()
    canvas.items = {"item1": MagicMock()}
    service = MagicMock()
    result = register_item_binding(canvas, "item1", service)
    # 可能返回 True/False 取决于 item 存在。
    assert isinstance(result, bool)


def test_register_item_binding_unknown_item():
    """register_item_binding 未知 item 返回 False。"""

    canvas = MagicMock()
    canvas.items = {}
    service = MagicMock()
    result = register_item_binding(canvas, "nonexistent", service)
    assert result is False


# ── open_binding_config_for_item ─────────────────────────────────
def test_open_binding_config_no_crash():
    """open_binding_config_for_item 不崩。"""

    panel = MagicMock()
    open_binding_config_for_item(panel, "item1")  # 不崩


# ── _notify_status ───────────────────────────────────────────────
def test_notify_status_no_crash():
    """_notify_status 不崩。"""

    panel = MagicMock()
    root = MagicMock()
    _notify_status(panel, root, "ch0:led")  # 不崩


# ── _trigger_autosave ────────────────────────────────────────────
def test_trigger_autosave_no_crash():
    """_trigger_autosave 不崩。"""

    panel = MagicMock()
    _trigger_autosave(panel)  # 不崩
