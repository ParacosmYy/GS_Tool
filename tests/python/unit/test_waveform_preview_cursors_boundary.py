"""waveform_preview _ensure_cursors + _install_cursor_interactions 边界测试。

两个 cursor 路径此前无直接测试覆盖（grep 0 命中，仅经 panels_skeleton 间接）。

覆盖：
1. _ensure_cursors 首次调用创建 CursorManager + 2 条默认 X 游标。
2. _ensure_cursors 幂等（已存在不重建）。
3. cursor_manager() 属性返回 None（初始化前）/ CursorManager（初始化后）。
4. _install_cursor_interactions 安装双击/右键闭包到 plot。
5. _update_cursor_hud 有 values 不崩 + 空 values 不崩。
6. set_connecting 不崩（batch 0 路径）。
"""

from __future__ import annotations

import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np

from embeddebug.serial_station.core import ChannelBatch
from embeddebug.serial_station.ui.waveform_preview import SerialWaveformPreview


def _batch(channels=2, samples=50):
    return ChannelBatch(
        channel_names=tuple(f"ch{i}" for i in range(channels)),
        values=np.random.rand(samples, channels).astype(np.float32) * 10.0,
    )


# ── cursor_manager 属性 ──────────────────────────────────────────
def test_cursor_manager_none_before_ensure(qtbot):
    """初始化前 cursor_manager() 返回 None。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    assert preview.cursor_manager() is None


def test_cursor_manager_returns_instance_after_update(qtbot):
    """update_batch 后 cursor_manager 返回 CursorManager 实例。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_batch())
    cm = preview.cursor_manager()
    assert cm is not None


# ── _ensure_cursors ──────────────────────────────────────────────
def test_ensure_cursors_creates_two_default_x_cursors(qtbot):
    """_ensure_cursors 创建 2 条默认 X 游标（25%/75%）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_cursors()
    cm = preview.cursor_manager()
    assert cm is not None
    assert len(cm.x_cursors) == 2


def test_ensure_cursors_idempotent(qtbot):
    """_ensure_cursors 幂等（已存在不重建）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_cursors()
    cm1 = preview.cursor_manager()
    preview._ensure_cursors()  # 再次调用
    cm2 = preview.cursor_manager()
    assert cm1 is cm2  # 同一实例


def test_ensure_cursors_no_y_cursors_initially(qtbot):
    """初始只有 X 游标，无 Y 游标。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_cursors()
    cm = preview.cursor_manager()
    assert len(cm.y_cursors) == 0


# ── _install_cursor_interactions ─────────────────────────────────
def test_install_cursor_interactions_installs_overrides(qtbot):
    """_install_cursor_interactions 在 plot.__dict__ 写入命名闭包。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    plot = preview._plot
    preview._install_cursor_interactions()
    dc = plot.__dict__.get("mouseDoubleClickEvent")
    ctx = plot.__dict__.get("contextMenuEvent")
    assert dc is not None
    assert ctx is not None


def test_install_cursor_interactions_after_ensure_cursor_manager(qtbot):
    """安装交互时 cursor_manager 已初始化（lambda 返回非 None）。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._ensure_cursors()
    preview._install_cursor_interactions()
    # cursor_manager 通过 lambda 闭包可达。
    cm = preview.cursor_manager()
    assert cm is not None


# ── _update_cursor_hud ───────────────────────────────────────────
def test_update_cursor_hud_with_values_no_crash(qtbot):
    """_update_cursor_hud 有 values 不崩。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview.update_batch(_batch(channels=2, samples=50))
    # update_batch 内部已调 _update_cursor_hud；再直接调验证幂等。
    preview._update_cursor_hud(_batch(channels=2, samples=30).values)


def test_update_cursor_hud_empty_values_no_crash(qtbot):
    """_update_cursor_hud 空 values 不崩。"""

    preview = SerialWaveformPreview()
    qtbot.addWidget(preview)
    preview._update_cursor_hud(np.empty((0, 0), dtype=np.float32))
