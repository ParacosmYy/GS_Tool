"""高级波形引擎单元测试。"""
from __future__ import annotations
import os
os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")
import numpy as np
from embeddebug.serial_station.waveform_advanced import MultiAxisPlot, ScatterPlot, WaterfallPlot, SpectrumWaterfall

def test_multi_axis_add_and_plot(qtbot):
    plot = MultiAxisPlot()
    qtbot.addWidget(plot)
    idx = plot.add_axis("ch0")
    assert idx == 0
    idx2 = plot.add_axis("ch1")
    assert idx2 == 1
    plot.plot_channel(0, np.arange(10), np.random.rand(10))
    plot.shutdown()

def test_multi_axis_bad_index(qtbot):
    plot = MultiAxisPlot()
    qtbot.addWidget(plot)
    plot.add_axis("x")
    plot.plot_channel(99, np.arange(5), np.arange(5))  # no crash

def test_scatter_points(qtbot):
    sp = ScatterPlot()
    qtbot.addWidget(sp)
    x = np.random.rand(20)
    y = np.random.rand(20)
    z = np.random.rand(20)
    sp.set_points(x, y, z)
    assert sp._scatter is not None

def test_scatter_empty_no_crash(qtbot):
    sp = ScatterPlot()
    qtbot.addWidget(sp)
    sp.set_points(np.array([]), np.array([]))
    assert sp._scatter is None

def test_waterfall_append_and_scroll(qtbot):
    wf = WaterfallPlot(max_rows=3)
    qtbot.addWidget(wf)
    for i in range(5):
        wf.append_row(np.random.rand(8))
    assert wf.row_count == 3

def test_waterfall_clear(qtbot):
    wf = WaterfallPlot()
    qtbot.addWidget(wf)
    wf.append_row(np.random.rand(8))
    wf.clear()
    assert wf.row_count == 0

def test_spectrum_waterfall(qtbot):
    sw = SpectrumWaterfall(freq_count=16, max_rows=3)
    qtbot.addWidget(sw)
    sw.append_spectrum(np.arange(16), np.random.rand(16))
    assert sw.row_count == 1

def test_spectrum_waterfall_empty(qtbot):
    sw = SpectrumWaterfall(freq_count=16)
    qtbot.addWidget(sw)
    sw.append_spectrum(np.arange(16), np.array([]))
    assert sw.row_count == 0
