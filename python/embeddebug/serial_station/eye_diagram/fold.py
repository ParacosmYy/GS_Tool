"""眼图折叠：把波形按位周期（UI）折叠叠加成二维密度网格。

入口 ``fold_eye(samples, samples_per_bit, ...)`` 把一维采样序列按 ``samples_per_bit``
切片，每片对齐到一个 UI 周期，叠加成 ``[level_bins, phase_bins]`` 密度矩阵 +
平铺 overlay 数组。这是眼图渲染与指标测量的前置。

仅依赖 numpy，不 import PyQt、不依赖 transport。
"""

from __future__ import annotations

import numpy as np

from embeddebug.serial_station.eye_diagram.model import EyeDiagram


def fold_eye(
    samples: np.ndarray,
    samples_per_bit: int,
    level_bins: int = 256,
    phase_bins: int = 0,
    level_range: tuple[float, float] | None = None,
) -> EyeDiagram:
    """把 ``samples`` 按 ``samples_per_bit`` 折叠叠加成 ``EyeDiagram``。

    - ``samples_per_bit``：每个位周期（UI）的采样数；>0。
    - ``phase_bins``：相位轴分箱数；默认 = ``samples_per_bit``（1 采样/bin）。
    - ``level_bins``：幅度轴分箱数（密度图分辨率）。
    - ``level_range``：幅度范围 ``(low, high)``；默认用样本 min/max。
    - 不足一个完整 UI 的尾部样本丢弃。

    返回的 ``EyeDiagram`` 含 density 矩阵（热图）+ overlay（曲线叠加）。
    """

    samples = np.asarray(samples, dtype=np.float32).ravel()
    if samples_per_bit <= 0:
        raise ValueError("samples_per_bit must be positive")
    if phase_bins <= 0:
        phase_bins = samples_per_bit
    period_count = samples.shape[0] // samples_per_bit
    if period_count == 0:
        # 不足一个 UI：返回空网格。
        return EyeDiagram(
            phase_axis=np.linspace(0.0, 1.0, phase_bins, dtype=np.float32),
            levels=np.empty(0, dtype=np.float32),
            density=np.empty((0, phase_bins), dtype=np.float32),
            overlay=np.empty(0, dtype=np.float32),
            period_count=0,
            phase_bins=phase_bins,
        )

    usable = samples[: period_count * samples_per_bit]
    # overlay：[period_count, samples_per_bit] 每行一个 UI 周期，平铺。
    overlay = usable.reshape(period_count, samples_per_bit).astype(np.float32, copy=False)

    low, high = _resolve_level_range(usable, level_range)
    levels = np.linspace(low, high, level_bins, dtype=np.float32)
    # 密度矩阵：把每个周期的样本按相位 bin × 幅度 bin 累计。
    # 重采样 overlay 到 phase_bins 列（若 samples_per_bit != phase_bins）。
    phased = _resample_phase(overlay, phase_bins)
    density = _build_density(phased, low, high, level_bins)

    phase_axis = np.linspace(0.0, 1.0, phase_bins, dtype=np.float32)
    return EyeDiagram(
        phase_axis=phase_axis,
        levels=levels,
        density=density,
        overlay=overlay.ravel(),
        period_count=period_count,
        phase_bins=phase_bins,
    )


def _resolve_level_range(
    samples: np.ndarray, level_range: tuple[float, float] | None
) -> tuple[float, float]:
    """确定幅度范围；默认样本 min/max，含退化保护（全零 → ±1）。"""

    if level_range is not None:
        low, high = float(level_range[0]), float(level_range[1])
    else:
        low = float(samples.min())
        high = float(samples.max())
    if high <= low:
        # 退化（恒定信号）：给一个对称小范围避免除零。
        center = (high + low) / 2.0
        low, high = center - 0.5, center + 0.5
    return low, high


def _resample_phase(overlay: np.ndarray, phase_bins: int) -> np.ndarray:
    """把 ``[period_count, samples_per_bit]`` 重采样到 ``[period_count, phase_bins]``。

    用最近邻切片近似（足够眼图叠加；精确插值对密度图意义有限）。
    """

    spb = overlay.shape[1]
    if spb == phase_bins:
        return overlay
    # 对每个目标相位 bin 取源序列的最近邻列。
    indices = (np.arange(phase_bins) * spb / phase_bins).astype(int)
    indices = np.clip(indices, 0, spb - 1)
    return overlay[:, indices]


def _build_density(
    phased: np.ndarray, low: float, high: float, level_bins: int
) -> np.ndarray:
    """把折叠后的 ``[period_count, phase_bins]`` 累计成 ``[level_bins, phase_bins]``。"""

    _period_count, phase_bins = phased.shape
    density = np.zeros((level_bins, phase_bins), dtype=np.float32)
    # 幅度 → bin 索引（clip 到 [0, level_bins-1]）。
    norm = (phased - low) / (high - low)
    level_idx = np.clip((norm * level_bins).astype(int), 0, level_bins - 1)
    # 逐相位 bin 用 bincount 累计该列的幅度分布。
    for col in range(phase_bins):
        counts = np.bincount(level_idx[:, col], minlength=level_bins)
        density[:, col] = counts[:level_bins].astype(np.float32)
    return density


__all__ = ["fold_eye"]
