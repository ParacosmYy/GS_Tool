"""TimestampConverterPanel 单元测试。

覆盖：
- 纯函数核（无 Qt 依赖）：``epoch_to_datetime`` / ``datetime_to_epoch`` /
  ``epoch_to_iso`` / ``iso_to_epoch`` / ``epoch_to_hex``，含已知值、round-trip、
  非法输入抛错。
- ``TZ_PRESETS``：非空 + 所有 offset_seconds 在合法 UTC 偏移范围。
- ``TimestampConverterPanel`` widget：objectName、构造不抛、Now 按钮回填 epoch。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from datetime import datetime, timezone  # noqa: E402

import pytest  # noqa: E402

from embeddebug.serial_station.ui.tools.timestamp_converter import (  # noqa: E402
    TZ_PRESETS,
    TimestampConverterPanel,
    datetime_to_epoch,
    epoch_to_datetime,
    epoch_to_hex,
    epoch_to_iso,
    iso_to_epoch,
)


# ══════════════════════════════════════════════════════════════════
#  纯函数核（无 Qt 依赖）
# ══════════════════════════════════════════════════════════════════
def test_epoch_to_datetime_known_value():
    """epoch 0（UTC）→ 1970-01-01 00:00:00（naive）。"""

    dt = epoch_to_datetime(0)
    assert dt == datetime(1970, 1, 1, 0, 0, 0)


def test_epoch_to_datetime_tz_aware_has_utc():
    """tz_aware=True 时返回带 timezone.utc 的 aware datetime。"""

    dt = epoch_to_datetime(0, tz_aware=True)
    assert dt.tzinfo is not None
    assert dt.utcoffset().total_seconds() == 0


def test_epoch_to_datetime_negative_raises():
    """负 epoch 应抛 ValueError。"""

    with pytest.raises(ValueError):
        epoch_to_datetime(-1)


def test_datetime_to_epoch_roundtrip():
    """now(UTC) → epoch → back 应在 1s 内。"""

    now = datetime.now(timezone.utc)
    epoch = datetime_to_epoch(now)
    back = epoch_to_datetime(epoch, tz_aware=True)
    assert abs((back - now).total_seconds()) < 1


def test_datetime_to_epoch_naive_assumed_utc():
    """naive datetime 应视为 UTC。"""

    dt = datetime(1970, 1, 1, 0, 0, 0)
    assert datetime_to_epoch(dt) == 0.0


def test_epoch_to_iso_format():
    """epoch 0 tz_aware → '1970-01-01T00:00:00+00:00'。"""

    assert epoch_to_iso(0, tz_aware=True) == "1970-01-01T00:00:00+00:00"


def test_iso_to_epoch_known():
    """'2024-01-01T00:00:00+00:00' → 已知 epoch 1704067200。"""

    assert iso_to_epoch("2024-01-01T00:00:00+00:00") == 1704067200.0


def test_iso_to_epoch_roundtrip():
    """epoch → iso → epoch 应保持一致。"""

    epoch = 1_700_000_000.0
    iso = epoch_to_iso(epoch, tz_aware=True)
    assert iso_to_epoch(iso) == epoch


def test_invalid_iso_raises():
    """非法 ISO 字符串应抛 ValueError。"""

    with pytest.raises(ValueError):
        iso_to_epoch("not-a-date")


def test_epoch_to_hex_format():
    """epoch 0 → '0x00000000'；epoch 0x100 → '0x00000100'。"""

    assert epoch_to_hex(0) == "0x00000000"
    assert epoch_to_hex(0x100) == "0x00000100"


def test_epoch_to_hex_negative_raises():
    """负 epoch 应抛 ValueError。"""

    with pytest.raises(ValueError):
        epoch_to_hex(-1)


def test_tz_presets_non_empty():
    """TZ_PRESETS 应非空，且所有 offset_seconds 在 [-12h, +14h] 内。"""

    assert len(TZ_PRESETS) >= 1
    for preset in TZ_PRESETS:
        assert -12 * 3600 <= preset.offset_seconds <= 14 * 3600


def test_tz_preset_is_frozen_dataclass():
    """TzPreset 必须是 frozen dataclass（不可变）。"""

    preset = TZ_PRESETS[0]
    with pytest.raises((AttributeError, TypeError)):  # frozen dataclass 不可变
        preset.offset_seconds = 999  # type: ignore[misc]


# ══════════════════════════════════════════════════════════════════
#  TimestampConverterPanel widget（需 qtbot）
# ══════════════════════════════════════════════════════════════════
def test_panel_objectname(qtbot):
    """面板 objectName 必须为 serialStationTimestampConverter。"""

    panel = TimestampConverterPanel()
    qtbot.addWidget(panel)
    assert panel.objectName() == "serialStationTimestampConverter"


def test_panel_constructs_without_error(qtbot):
    """构造面板不抛异常。"""

    panel = TimestampConverterPanel()
    qtbot.addWidget(panel)
    assert panel.isVisible() is False  # 仅构造，未 show()


def test_panel_subcontainer_objectnames(qtbot):
    """三个子容器应有指定 objectName。"""

    from PyQt6.QtWidgets import QWidget

    panel = TimestampConverterPanel()
    qtbot.addWidget(panel)
    names = {w.objectName() for w in panel.findChildren(QWidget)}
    assert "serialStationTimestampConverterInput" in names
    assert "serialStationTimestampConverterConfig" in names
    assert "serialStationTimestampConverterResult" in names


def test_now_button_fills_epoch(qtbot):
    """点击 Now 后 epoch 输入框应非空。"""

    panel = TimestampConverterPanel()
    qtbot.addWidget(panel)
    assert panel._epoch_edit.text() == ""
    panel._now_button.click()
    assert panel._epoch_edit.text().strip() != ""


def test_now_button_produces_results(qtbot):
    """Now 后 4 个结果字段都应填上非空值。"""

    panel = TimestampConverterPanel()
    qtbot.addWidget(panel)
    panel._now_button.click()
    for key, edit in panel._result_edits.items():
        assert edit.text() != "", f"result {key!r} should be populated after Now"


def test_invalid_input_shows_error(qtbot):
    """非法 epoch 输入下结果字段应显示错误提示而非崩溃。"""

    panel = TimestampConverterPanel()
    qtbot.addWidget(panel)
    panel._epoch_edit.setText("not-a-number")
    for edit in panel._result_edits.values():
        assert edit.text() != ""
