"""实时直方图计算（对齐 VOFA+ 直方统计）。

对单通道信号做实时分布统计，返回 bin 边界与计数，供 pyqtgraph BarGraphItem 渲染。

约束：本模块只依赖 numpy + 标准库，不 import PyQt（纯计算，便于单测）。
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

DEFAULT_BIN_COUNT = 32
MIN_BIN_COUNT = 4
MAX_BIN_COUNT = 256


@dataclass(frozen=True)
class HistogramResult:
    """一次直方图计算结果。"""

    bin_edges: np.ndarray   # bin 边界，长度 bin_count+1
    counts: np.ndarray      # 每个 bin 的计数，长度 bin_count
    centers: np.ndarray     # bin 中心，长度 bin_count


def compute_histogram(
    signal: np.ndarray,
    bin_count: int = DEFAULT_BIN_COUNT,
    value_range: tuple[float, float] | None = None,
) -> HistogramResult:
    """对单通道信号做直方图统计。

    - bin_count 限制在 [MIN, MAX]。
    - value_range 为 None 时自动用信号 min/max；否则用指定范围。
    - 空信号返回空结果。
    """

    bin_count = max(MIN_BIN_COUNT, min(MAX_BIN_COUNT, bin_count))
    flat = np.asarray(signal, dtype=np.float32).ravel()
    if flat.size == 0:
        empty = np.array([], dtype=np.float32)
        return HistogramResult(bin_edges=empty, counts=empty, centers=empty)
    lo, hi = value_range if value_range is not None else (float(flat.min()), float(flat.max()))
    if lo == hi:
        # 单值信号，给一点宽度避免除零。
        hi = lo + 1.0
    counts, edges = np.histogram(flat, bins=bin_count, range=(lo, hi))
    centers = ((edges[:-1] + edges[1:]) / 2.0).astype(np.float32)
    return HistogramResult(
        bin_edges=edges.astype(np.float32),
        counts=counts.astype(np.float32),
        centers=centers,
    )
