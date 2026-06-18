from __future__ import annotations

import os

from embeddebug.serial_station.ui.waveform_preview import SafePlotWidget


def test_safe_plot_widget_guards_resize_events_during_offscreen_teardown(qtbot):
    assert "resizeEvent" in SafePlotWidget.__dict__

    os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
    widget = SafePlotWidget()
    qtbot.addWidget(widget)

    widget.resizeEvent(None)
