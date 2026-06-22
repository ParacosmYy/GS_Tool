"""ScaleAnimation._scaled_rect + CAN 常量 + CanFilter 边界单元测试。

补强 test_animations_integrations / test_can_frame 未直接断言的边角：
- ScaleAnimation._scaled_rect：factor=0/1/2 缩放 + 中心对齐 + min(1,...) clamp。
- CAN 常量：CAN_MAX_DLC=8 / CAN_FD_MAX_DLC=64 / STANDARD_ID_MAX=0x7FF / EXTENDED_ID_MAX=0x1FFFFFFF。
- CanFilter：is_extended=None 不检查帧类型 + 默认 mask=0 匹配全部。
- CanId：value=0 合法 + as_hex 零填充宽度。
- CanFrame：空 data dlc=0 + to_payload 全字段。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from PyQt6.QtWidgets import QWidget

from embeddebug.serial_station.can.frame import (
    CAN_FD_MAX_DLC,
    CAN_MAX_DLC,
    EXTENDED_ID_MAX,
    STANDARD_ID_MAX,
    CanFilter,
    CanFrame,
    CanId,
)
from embeddebug.serial_station.ui.animations.scale import ScaleAnimation


# ── ScaleAnimation._scaled_rect 纯几何 ──────────────────────────────────


def _widget(qtbot, width=100, height=50):
    w = QWidget()
    w.setGeometry(10, 20, width, height)
    qtbot.addWidget(w)
    return w


def test_scaled_rect_factor_one_returns_same_size(qtbot):
    """factor=1.0 → 宽高不变。"""

    w = _widget(qtbot, 100, 50)
    scaled = ScaleAnimation._scaled_rect(w, 1.0)
    assert scaled.width() == 100
    assert scaled.height() == 50


def test_scaled_rect_center_anchored(qtbot):
    """缩放后中心点保持不变。"""

    w = _widget(qtbot, 100, 50)
    scaled = ScaleAnimation._scaled_rect(w, 0.5)
    assert scaled.center() == w.geometry().center()


def test_scaled_rect_factor_half(qtbot):
    """factor=0.5 → 宽高各减半。"""

    w = _widget(qtbot, 100, 50)
    scaled = ScaleAnimation._scaled_rect(w, 0.5)
    assert scaled.width() == 50
    assert scaled.height() == 25


def test_scaled_rect_factor_two(qtbot):
    """factor=2.0 → 宽高加倍。"""

    w = _widget(qtbot, 100, 50)
    scaled = ScaleAnimation._scaled_rect(w, 2.0)
    assert scaled.width() == 200
    assert scaled.height() == 100


def test_scaled_rect_factor_zero_clamped_to_one(qtbot):
    """factor=0 → clamp min(1,...) 防止零尺寸。"""

    w = _widget(qtbot, 100, 50)
    scaled = ScaleAnimation._scaled_rect(w, 0.0)
    assert scaled.width() >= 1
    assert scaled.height() >= 1


# ── CAN 常量契约 ───────────────────────────────────────────────────────


def test_can_max_dlc_is_eight():
    """CAN_MAX_DLC = 8（经典 CAN）。"""

    assert CAN_MAX_DLC == 8


def test_can_fd_max_dlc_is_64():
    """CAN_FD_MAX_DLC = 64（CAN-FD）。"""

    assert CAN_FD_MAX_DLC == 64


def test_standard_id_max():
    """STANDARD_ID_MAX = 0x7FF（11 位标准帧）。"""

    assert STANDARD_ID_MAX == 0x7FF


def test_extended_id_max():
    """EXTENDED_ID_MAX = 0x1FFFFFFF（29 位扩展帧）。"""

    assert EXTENDED_ID_MAX == 0x1FFFFFFF


# ── CanId 边界 ─────────────────────────────────────────────────────────


def test_can_id_zero_is_valid():
    """CanId(0) 合法（零 ID 是有效的 CAN 帧）。"""

    cid = CanId(0)
    assert cid.value == 0


def test_can_id_as_hex_standard_zero():
    """标准帧 ID=0 → "000"（3 位填充）。"""

    assert CanId(0).as_hex() == "000"


def test_can_id_as_hex_extended_zero():
    """扩展帧 ID=0 → "00000000"（8 位填充）。"""

    assert CanId(0, is_extended=True).as_hex() == "00000000"


# ── CanFrame 边界 ──────────────────────────────────────────────────────


def test_can_frame_empty_data_dlc_zero():
    """空 data → dlc=0。"""

    frame = CanFrame(CanId(0x100), b"")
    assert frame.dlc == 0


def test_can_frame_to_payload_all_fields():
    """to_payload 含全部 9 个字段。"""

    frame = CanFrame(CanId(0x123), b"\xAA\xBB", is_fd=False, timestamp=1.5, frame_index=7)
    payload = frame.to_payload()
    assert set(payload.keys()) == {
        "canId", "canIdHex", "isExtended", "isFd", "dlc", "data", "dataHex", "timestamp", "frameIndex",
    }
    assert payload["canId"] == 0x123
    assert payload["canIdHex"] == "123"
    assert payload["dlc"] == 2
    assert payload["dataHex"] == "aa bb"


def test_can_frame_to_payload_empty_data_hex():
    """空 data 的 dataHex 为空串。"""

    frame = CanFrame(CanId(0), b"")
    assert frame.to_payload()["dataHex"] == ""


# ── CanFilter 边界 ─────────────────────────────────────────────────────


def test_can_filter_is_extended_none_matches_both():
    """is_extended=None → 不检查帧类型（标准+扩展都匹配）。"""

    f = CanFilter(id=0x100, mask=0x7FF, is_extended=None)
    assert f.matches(CanId(0x100, is_extended=False)) is True
    assert f.matches(CanId(0x100, is_extended=True)) is True


def test_can_filter_default_mask_zero_matches_all():
    """默认 mask=0 → 匹配所有 ID。"""

    f = CanFilter(id=0)
    assert f.matches(CanId(0x001)) is True
    assert f.matches(CanId(0x7FF)) is True


def test_can_filter_partial_mask():
    """mask=0x700 → 只比较高 3 位。"""

    f = CanFilter(id=0x100, mask=0x700)
    assert f.matches(CanId(0x1FF)) is True   # 高 3 位 = 0x100
    assert f.matches(CanId(0x200)) is False  # 高 3 位 = 0x200
