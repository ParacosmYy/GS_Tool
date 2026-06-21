"""ByteFrequencyAnalyzer 单元测试。

覆盖：
- ``compute_frequency``：空数据 / 基本计数 / 256 全分布。
- ``top_n_bytes``：排序（count 降序）+ 并列 tie-break（byte_value 升序）。
- ``entropy_bits``：空数据（0.0）/ 均匀 256 分布（≈8.0 bits）/ 单字节重复（0.0）。
- ``format_stats_text``：输出包含 "Total"。
- ``ByteFrequencyAnalyzer`` widget：objectName、构造不抛、canvas objectName、
  点击「分析」后统计标签非空且反映输入数据。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

from embeddebug.serial_station.ui.tools.byte_frequency import (
    ByteFrequencyAnalyzer,
    _FrequencyCanvas,
    compute_frequency,
    entropy_bits,
    format_stats_text,
    top_n_bytes,
)


# ══════════════════════════════════════════════════════════════════
#  频率核（纯函数，无 Qt 依赖）
# ══════════════════════════════════════════════════════════════════
def test_compute_frequency_empty():
    """空字节流应得 256 个 0。"""

    freq = compute_frequency(b"")
    assert len(freq) == 256
    assert all(c == 0 for c in freq)


def test_compute_frequency_basic():
    """b"aaab"：97 计 3 次，98 计 1 次，其余 0。"""

    freq = compute_frequency(b"aaab")
    assert len(freq) == 256
    assert freq[97] == 3
    assert freq[98] == 1
    # 其余应为 0
    for i in range(256):
        if i not in (97, 98):
            assert freq[i] == 0


def test_compute_frequency_all_256_distinct():
    """bytes(range(256))：每个字节值恰好出现 1 次。"""

    freq = compute_frequency(bytes(range(256)))
    assert len(freq) == 256
    assert all(c == 1 for c in freq)


def test_top_n_bytes_order():
    """b"aaabbc" 前 2 名应为 [(97, 3), (98, 2)]（count 降序）。"""

    freq = compute_frequency(b"aaabbc")
    assert top_n_bytes(freq, 2) == [(97, 3), (98, 2)]


def test_top_n_bytes_tie_break():
    """相同 count 时按 byte_value 升序：97 应排在 98 前。"""

    freq = [0] * 256
    freq[99] = 1   # byte 99, count 1
    freq[97] = 2   # byte 97, count 2
    freq[98] = 2   # byte 98, count 2（与 97 并列）
    result = top_n_bytes(freq, 2)
    assert result == [(97, 2), (98, 2)]


def test_top_n_bytes_empty():
    """全零频率应返回空列表。"""

    assert top_n_bytes([0] * 256, 5) == []


def test_entropy_empty_is_zero():
    """空数据熵为 0.0。"""

    assert entropy_bits([0] * 256, 0) == 0.0


def test_entropy_uniform_256():
    """256 个等概率字节（均匀分布）熵应 ≈ 8.0 bits（log2 256）。"""

    freq = compute_frequency(bytes(range(256)))
    assert entropy_bits(freq, 256) == pytest.approx(8.0, abs=0.01)


def test_entropy_single_byte_repeated():
    """单字节重复 100 次熵为 0.0（完全确定）。"""

    freq = compute_frequency(b"\x00" * 100)
    assert entropy_bits(freq, 100) == 0.0


def test_format_stats_text_contains_total():
    """统计文本必须包含 "Total" 关键字。"""

    freq = compute_frequency(b"abc")
    text = format_stats_text(freq, 3)
    assert "Total" in text


def test_format_stats_text_empty_state():
    """空数据统计文本应包含 "Total: 0 bytes"。"""

    text = format_stats_text([0] * 256, 0)
    assert "Total: 0 bytes" in text
    assert "0.000 bits" in text


# ══════════════════════════════════════════════════════════════════
#  ByteFrequencyAnalyzer widget（需 qtbot）
# ══════════════════════════════════════════════════════════════════
def test_panel_objectname(qtbot):
    """面板 objectName 必须为 serialStationByteFreqAnalyzer。"""

    panel = ByteFrequencyAnalyzer()
    qtbot.addWidget(panel)
    assert panel.objectName() == "serialStationByteFreqAnalyzer"


def test_panel_constructs_without_error(qtbot):
    """构造面板不抛异常；初始统计标签非空（含 Total 基线）。"""

    panel = ByteFrequencyAnalyzer()
    qtbot.addWidget(panel)
    text = panel._stats_label.text()
    assert "Total" in text


def test_canvas_objectname(qtbot):
    """画布 objectName 必须为 serialStationByteFreqCanvas。"""

    panel = ByteFrequencyAnalyzer()
    qtbot.addWidget(panel)
    assert panel._canvas.objectName() == "serialStationByteFreqCanvas"


def test_canvas_set_frequency_updates_internal_state(qtbot):
    """set_frequency 应更新画布内部频率并接受 repaint 不抛错。"""

    canvas = _FrequencyCanvas()
    qtbot.addWidget(canvas)
    freq = compute_frequency(b"aaab")
    canvas.set_frequency(freq)
    assert canvas._freq[97] == 3
    assert canvas._freq[98] == 1


def test_analyze_button_updates_stats(qtbot):
    """点击「分析」后统计标签应反映输入：含 Total 与正确字节数。"""

    panel = ByteFrequencyAnalyzer()
    qtbot.addWidget(panel)
    panel._mode_ascii.setChecked(True)
    panel._input_edit.setText("aaab")  # 4 bytes
    panel._analyze_btn.click()
    stats = panel._stats_label.text()
    assert "Total" in stats
    assert "4 bytes" in stats
    # 主导字节 0x61 (97) 计 3 次，应出现在 Most common 行
    assert "0x61" in stats
    assert "(3x)" in stats


def test_analyze_hex_mode(qtbot):
    """Hex 模式下应正确解析并更新统计。"""

    panel = ByteFrequencyAnalyzer()
    qtbot.addWidget(panel)
    panel._mode_hex.setChecked(True)
    panel._input_edit.setText("61 61 61 62")  # = b"aaab"
    panel._analyze_btn.click()
    stats = panel._stats_label.text()
    assert "4 bytes" in stats


def test_clear_button_resets_state(qtbot):
    """点击「清空」后输入框清空、统计回到基线 0 bytes。"""

    panel = ByteFrequencyAnalyzer()
    qtbot.addWidget(panel)
    panel._input_edit.setText("aaab")
    panel._analyze_btn.click()
    panel._clear_btn.click()
    assert panel._input_edit.toPlainText() == ""
    assert "Total: 0 bytes" in panel._stats_label.text()
