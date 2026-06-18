"""波形测量计算（对齐 VOFA+ 测量面板）。

提供波形统计与游标测量：
- 通道统计：峰峰值 Vpp、均值 Mean、最大 Max、最小 Min、标准差 Std、RMS。
- 游标测量：两 X 游标间的时间差 ΔT、频率估计、占空比；两 Y 游标差值。

约束：本模块只依赖 numpy + 标准库，不 import PyQt（纯计算，便于单测）。
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass(frozen=True)
class ChannelStats:
    """单通道波形统计。"""

    vpp: float       # 峰峰值
    mean: float      # 均值
    maximum: float   # 最大值
    minimum: float   # 最小值
    std: float       # 标准差
    rms: float       # 均方根


@dataclass(frozen=True)
class CursorMeasurement:
    """游标测量结果。"""

    delta_t: float | None      # 两 X 游标时间差（秒），无两个 X 游标时 None
    frequency: float | None    # 频率估计（Hz），ΔT 为 0 或 None 时 None
    delta_y: float | None      # 两 Y 游标差值，无两个 Y 游标时 None


def compute_channel_stats(signal: np.ndarray) -> ChannelStats:
    """计算单通道波形统计。"""

    flat = np.asarray(signal, dtype=np.float32).ravel()
    if flat.size == 0:
        return ChannelStats(vpp=0.0, mean=0.0, maximum=0.0, minimum=0.0, std=0.0, rms=0.0)
    maximum = float(flat.max())
    minimum = float(flat.min())
    mean = float(flat.mean())
    std = float(flat.std())
    rms = float(np.sqrt(np.mean(flat ** 2)))
    return ChannelStats(
        vpp=maximum - minimum,
        mean=mean,
        maximum=maximum,
        minimum=minimum,
        std=std,
        rms=rms,
    )


def compute_cursor_measurement(
    x_cursor_values: list[float],
    y_cursor_values: list[float],
    sample_rate: float,
) -> CursorMeasurement:
    """根据游标值计算时间差/频率/Y 差值。

    - 需要至少两个 X 游标才能算 ΔT/频率。
    - 需要至少两个 Y 游标才能算 ΔY。
    - sample_rate 单位 Hz；ΔT = |Δindex| / sample_rate。
    """

    delta_t = None
    frequency = None
    if len(x_cursor_values) >= 2 and sample_rate > 0:
        delta_index = abs(x_cursor_values[1] - x_cursor_values[0])
        delta_t = float(delta_index / sample_rate)
        if delta_t > 0:
            frequency = 1.0 / delta_t

    delta_y = None
    if len(y_cursor_values) >= 2:
        delta_y = float(abs(y_cursor_values[1] - y_cursor_values[0]))

    return CursorMeasurement(delta_t=delta_t, frequency=frequency, delta_y=delta_y)


def format_stats(stats: ChannelStats) -> str:
    """把通道统计格式化为单行读数字符串。"""

    return (
        f"Vpp {stats.vpp:.3f}  ·  Mean {stats.mean:.3f}  ·  "
        f"Max {stats.maximum:.3f}  ·  Min {stats.minimum:.3f}  ·  "
        f"Std {stats.std:.3f}  ·  RMS {stats.rms:.3f}"
    )


def format_cursor_measurement(measurement: CursorMeasurement) -> str:
    """把游标测量格式化为单行读数字符串。"""

    parts: list[str] = []
    if measurement.delta_t is not None:
        parts.append(f"ΔT {measurement.delta_t:.6f}s")
    if measurement.frequency is not None:
        parts.append(f"Freq {measurement.frequency:.3f}Hz")
    if measurement.delta_y is not None:
        parts.append(f"ΔY {measurement.delta_y:.3f}")
    if not parts:
        return "ΔT —  ·  Freq —  ·  ΔY —"
    return "  ·  ".join(parts)
