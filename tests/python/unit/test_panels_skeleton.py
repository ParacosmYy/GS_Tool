"""Batch 9 测试：域面板加载占位 + 波形游标右键交互接线。

覆盖：
1. OTA 传输中 skeleton shimmer（传输开始 show/完成 hide）。
2. CAN/BLE/RTT 空数据 EmptyState（有数据 hide/清空 show）。
3. CursorManager 右键/双击游标交互（install_cursor_interactions 接入 preview）。

源码级断言（避免实例化复杂面板需要 AppController/transport），验证接入点存在。
"""

from __future__ import annotations

import inspect
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")



def test_ota_panel_has_transfer_skeleton():
    """OTA 面板应含 _transfer_skeleton 字段 + show/hide 接入。"""

    from embeddebug.serial_station.ui.panels.ota_panel import OtaPanel

    src = inspect.getsource(OtaPanel)
    assert "_transfer_skeleton" in src
    assert "SkeletonBlock" in src
    # 传输开始 show，完成 hide。
    assert "self._transfer_skeleton.show()" in src
    assert "self._transfer_skeleton.hide()" in src


def test_can_panel_has_empty_state():
    """CAN 面板应含 _empty_state + 首帧 hide/清空 show（Batch 11 后可为 show_with_fade）。"""

    from embeddebug.serial_station.ui.panels.can_panel import CanPanel

    src = inspect.getsource(CanPanel)
    assert "_empty_state" in src
    assert "EmptyStateWidget" in src
    # _append_frame 首帧 hide。
    assert "self._empty_state.hide()" in src
    # _clear 恢复显示（Batch 9 show / Batch 11 show_with_fade 均可）。
    assert _assert_empty_state_revealed(src)


def test_ble_panel_has_empty_state():
    """BLE 面板应含 _empty_state + 连接 hide/断开 show。"""

    from embeddebug.serial_station.ui.panels.ble_panel import BlePanel

    src = inspect.getsource(BlePanel)
    assert "_empty_state" in src
    assert "EmptyStateWidget" in src
    # 连接成功 hide，断开 show。
    assert "self._empty_state.hide()" in src
    assert _assert_empty_state_revealed(src)


def test_rtt_panel_has_empty_state():
    """RTT 面板应含 _empty_state + 收到数据 hide/清屏 show。"""

    from embeddebug.serial_station.ui.panels.rtt_panel import RttPanel

    src = inspect.getsource(RttPanel)
    assert "_empty_state" in src
    assert "EmptyStateWidget" in src
    assert "self._empty_state.hide()" in src
    assert _assert_empty_state_revealed(src)


def _assert_empty_state_revealed(src: str) -> bool:
    """空状态恢复路径：show()（Batch 9）或 show_with_fade()（Batch 11）均可。"""

    return "self._empty_state.show()" in src or "self._empty_state.show_with_fade()" in src


def test_skeleton_block_importable():
    """SkeletonBlock 应可从 widgets 导入（Batch 5 基建）。"""

    from embeddebug.serial_station.ui.widgets import SkeletonWidget
    from embeddebug.serial_station.ui.widgets.skeleton import SkeletonBlock

    assert SkeletonWidget is not None
    assert SkeletonBlock is not None


def test_empty_state_importable():
    """EmptyStateWidget 应可从 widgets 导入。"""

    from embeddebug.serial_station.ui.widgets import EmptyStateWidget

    assert EmptyStateWidget is not None


# ── Batch 9-3: CursorManager 右键菜单 UI ──────────────────────────
def test_cursor_interactions_module_provides_install():
    """waveform_cursor_interactions 应提供 install_cursor_interactions。"""

    from embeddebug.serial_station.ui.waveform_cursor_interactions import (
        install_cursor_interactions,
    )

    assert callable(install_cursor_interactions)


def test_waveform_preview_installs_cursor_interactions(qtbot):
    """preview 构造时应装双击/右键游标交互（mouseDoubleClickEvent 被替换为闭包）。

    用实例 __dict__ 验证 override：pyqtgraph 对 bound method 的 getattr 会回退到
    类方法，实例属性需通过 __dict__ 检查。
    """

    from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    plot = preview._plot
    # install_cursor_interactions 在 plot.__dict__ 写入命名闭包。
    dc = plot.__dict__.get("mouseDoubleClickEvent")
    ctx = plot.__dict__.get("contextMenuEvent")
    assert dc is not None and getattr(dc, "__name__", "") == "_on_double_click"
    assert ctx is not None and getattr(ctx, "__name__", "") == "_on_context"


def test_cursor_interactions_cursor_manager_available_after_update(qtbot):
    """update_batch 后 cursor_manager 初始化，交互可添加游标。"""

    import numpy as np

    from embeddebug.serial_station.core import ChannelBatch
    from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    values = np.random.rand(50, 1).astype(np.float32) * 10.0
    preview.update_batch(ChannelBatch(channel_names=("ch0",), values=values))
    cm = preview.cursor_manager()
    assert cm is not None
    assert len(cm.x_cursors) == 2  # 两条默认游标


# ── Batch 9-3: Qt6 鼠标事件兼容 helper ─────────────────────────────
def test_cursor_interaction_qt6_compat_helpers():
    """_event_local_point / _event_global_point 应兼容 Qt6 position()/globalPosition()。"""

    from PyQt6.QtCore import QPoint
    from PyQt6.QtGui import QContextMenuEvent
    from embeddebug.serial_station.ui.waveform_cursor_interactions import (
        _event_global_point,
        _event_local_point,
    )

    # 真实 QContextMenuEvent 携带局部 + 全局 QPoint。
    event = QContextMenuEvent(
        QContextMenuEvent.Reason.Mouse, QPoint(10, 20), QPoint(100, 200)
    )
    assert _event_local_point(event) == QPoint(10, 20)
    assert _event_global_point(event) == QPoint(100, 200)


def test_cursor_interaction_qt5_fallback_for_missing_methods():
    """无 position()/globalPosition() 的事件（Qt5 风格 stub）应回退到 pos()/globalPos()。"""

    from embeddebug.serial_station.ui.waveform_cursor_interactions import (
        _event_global_point,
        _event_local_point,
    )

    class _Qt5Event:
        def pos(self):
            return "local-stub"

        def globalPos(self):
            return "global-stub"

    event = _Qt5Event()
    assert _event_local_point(event) == "local-stub"
    assert _event_global_point(event) == "global-stub"
