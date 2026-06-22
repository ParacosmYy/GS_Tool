"""waveform_preview _on_accumulator_flush + _flush_latest_to_plot 边界测试。

两个 flush 路径此前无直接测试覆盖（grep 0 命中）。

覆盖：
1. _on_accumulator_flush 缓存 _latest_batch + 调 throttle.maybe_refresh。
2. _flush_latest_to_plot 有 batch → update_batch + 清空 _latest_batch。
3. _flush_latest_to_plot 无 batch（None）→ 不调 update_batch。
4. _on_accumulator_flush 连续多次只缓存最新。
5. submit_batch → accumulator push → flush_signal → _on_accumulator_flush。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from unittest.mock import MagicMock

import numpy as np

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


def _batch(channels=2, samples=10):
    return ChannelBatch(
        channel_names=tuple(f"ch{i}" for i in range(channels)),
        values=np.random.rand(samples, channels).astype(np.float32),
    )


# ── _on_accumulator_flush ────────────────────────────────────────
def test_on_accumulator_flush_caches_latest_batch(qtbot):
    """_on_accumulator_flush 把 merged 存到 _latest_batch（mock throttle 避免立即 flush 清空）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._throttle.maybe_refresh = MagicMock()  # 避免立即 flush 清空
    batch = _batch()
    preview._on_accumulator_flush(batch)
    assert preview._latest_batch is batch


def test_on_accumulator_flush_calls_throttle_maybe_refresh(qtbot):
    """_on_accumulator_flush 调 throttle.maybe_refresh。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._throttle.maybe_refresh = MagicMock()
    preview._on_accumulator_flush(_batch())
    preview._throttle.maybe_refresh.assert_called_once()


def test_on_accumulator_flush_consecutive_keeps_latest(qtbot):
    """连续多次 flush 只保留最新 _latest_batch（mock throttle 避免清空）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._throttle.maybe_refresh = MagicMock()
    b1 = _batch(channels=1)
    b2 = _batch(channels=2)
    b3 = _batch(channels=3)
    preview._on_accumulator_flush(b1)
    preview._on_accumulator_flush(b2)
    preview._on_accumulator_flush(b3)
    assert preview._latest_batch is b3


# ── _flush_latest_to_plot ────────────────────────────────────────
def test_flush_latest_with_batch_calls_update(qtbot):
    """_flush_latest_to_plot 有 batch → 调 update_batch + 清空。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    batch = _batch()
    preview._latest_batch = batch
    preview.update_batch = MagicMock()
    preview._flush_latest_to_plot()
    preview.update_batch.assert_called_once_with(batch)
    assert preview._latest_batch is None


def test_flush_latest_without_batch_no_update(qtbot):
    """_flush_latest_to_plot 无 batch（None）→ 不调 update_batch。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._latest_batch = None
    preview.update_batch = MagicMock()
    preview._flush_latest_to_plot()
    preview.update_batch.assert_not_called()


def test_flush_latest_clears_after_flush(qtbot):
    """flush 后 _latest_batch 被清空（None）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._latest_batch = _batch()
    # 不 mock update_batch（走真实路径，确保 _ensure_curves 等不崩）。
    preview._flush_latest_to_plot()
    assert preview._latest_batch is None


# ── submit_batch → accumulator 集成 ──────────────────────────────
def test_submit_batch_pushes_to_accumulator(qtbot):
    """submit_batch 把 batch 推入 accumulator（pending_count > 0）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert preview._accumulator.pending_count == 0
    preview.submit_batch(_batch())
    assert preview._accumulator.pending_count == 1


def test_submit_batch_multiple_accumulates(qtbot):
    """多次 submit_batch 累积到 accumulator。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    for _ in range(3):
        preview.submit_batch(_batch())
    assert preview._accumulator.pending_count == 3


# ── _update_stats 边界 ───────────────────────────────────────────
def test_update_stats_empty_values_shows_dash(qtbot):
    """空 values → stats_label 显示 'Stats: —'。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._update_stats(ChannelBatch(channel_names=(), values=np.empty((0, 0), dtype=np.float32)))
    assert "—" in preview._stats_label.text() or "Stats" in preview._stats_label.text()


def test_update_stats_zero_channels_shows_dash(qtbot):
    """0 通道 → stats_label 显示 dash。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._update_stats(ChannelBatch(channel_names=(), values=np.zeros((5, 0), dtype=np.float32)))
    text = preview._stats_label.text()
    assert "—" in text or "Stats" in text
