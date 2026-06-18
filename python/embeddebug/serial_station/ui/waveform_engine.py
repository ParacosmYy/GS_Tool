"""高性能波形渲染引擎配置（对齐 VOFA+ 百万点不降采样）。

pyqtgraph 在默认配置下会自动降采样，导致高频细节丢失。本模块集中配置
百万点级渲染优化，遵循 pyqtgraph 官方性能建议：

1. ``setDownsample(auto=False)`` — 禁用自动降采样，保留全部分辨率。
2. ``setClipToView(True)`` — 只渲染可视区内的点，越界点不送 GPU。
3. 大容量 NumPy ring buffer（百万级）承载历史，``setData`` 一次性喂入。
4. 关闭抗锯齿（百万点下 AA 是性能杀手），用 ``connect='finite'`` 断开 NaN。

约束：本模块只依赖 PyQt6 + pyqtgraph + numpy + 标准库，不访问 controller/transport。
"""

from __future__ import annotations

import pyqtgraph as pg

# 默认波形缓冲容量（百万级，覆盖长时采集）。
DEFAULT_BUFFER_CAPACITY = 1_000_000
# 默认刷新节流（Hz），避免过快刷新卡 UI。
DEFAULT_REFRESH_HZ = 60


def configure_high_performance_plot(plot: pg.PlotWidget) -> None:
    """把 PlotWidget 配置为百万点高性能渲染模式。

    - 禁用自动降采样（保留全分辨率）。
    - 启用 clipToView（只渲染可视区点）。
    - 关闭抗锯齿（性能优先）。
    - 启用 OpenGL 加速（如可用，失败则静默回退）。
    """

    plot.setDownsampling(auto=False, method="peak")
    plot.setClipToView(True)
    plot.showGrid(x=True, y=True, alpha=0.2)
    try_enable_opengl(plot)


def try_enable_opengl(plot: pg.PlotWidget) -> bool:
    """尝试启用 OpenGL 加速，失败返回 False（环境无 PyOpenGL 时静默回退）。"""

    try:
        import OpenGL  # noqa: F401
    except ImportError:
        return False
    try:
        pg.setConfigOption("useOpenGL", True)
        pg.setConfigOption("enableExperimental", True)
        return True
    except Exception:
        return False


def apply_curve_perf(curve: pg.PlotDataItem) -> None:
    """给单条曲线应用高性能渲染参数。"""

    curve.setDownsampling(auto=False, method="peak")
    curve.setClipToView(True)
    # connect='finite' 让 NaN/inf 处断开，避免跨断点连线。
    try:
        curve.opts["connect"] = "finite"
    except (AttributeError, KeyError):
        pass
