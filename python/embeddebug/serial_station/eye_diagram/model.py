"""眼图分析数据模型（Wave 60）。

封装眼图的折叠样本网格、测量指标与模板掩码。``EyeDiagram`` 是把波形按位周期
折叠叠加后的二维网格（相位 × 幅度），``EyeMetrics`` 是从网格提取的标量指标，
``PolyMask`` 是模板测试的多边形区域。仅依赖 numpy，不 import PyQt、不依赖 transport。

约定（对齐 ``waveform_math/model.py`` / ``waveform_fft.py``）：frozen dataclass；
数组字段强制 float32；标量字段用 float/int。
"""

from __future__ import annotations

from dataclasses import dataclass, field

import numpy as np


@dataclass(frozen=True)
class EyeDiagram:
    """按位周期折叠叠加后的眼图网格。

    - ``phase_axis``：相位轴（归一化 0~1，对应一个 UI 周期），长度 ``phase_bins``。
    - ``levels``：幅度轴（V），长度 ``level_bins``。
    - ``density``：二维密度矩阵 ``[level_bins, phase_bins]``，值为落入该格的样本数
      （用于渲染热图/等高线）。已强制 float32。
    - ``overlay``：折叠后所有样本的平铺一维数组（每行一个 UI 周期），长度
      ``period_count * phase_bins``，供曲线叠加渲染用。可为空（仅密度图时）。
    """

    phase_axis: np.ndarray
    levels: np.ndarray
    density: np.ndarray
    overlay: np.ndarray = field(default_factory=lambda: np.empty(0, dtype=np.float32))
    period_count: int = 0
    phase_bins: int = 0

    def __post_init__(self) -> None:
        object.__setattr__(self, "phase_axis", np.asarray(self.phase_axis, dtype=np.float32))
        object.__setattr__(self, "levels", np.asarray(self.levels, dtype=np.float32))
        object.__setattr__(self, "density", np.asarray(self.density, dtype=np.float32))
        object.__setattr__(self, "overlay", np.asarray(self.overlay, dtype=np.float32).ravel())

    @property
    def has_density(self) -> bool:
        """密度矩阵是否非空（含样本）。"""

        return self.density.size > 0 and float(self.density.sum()) > 0


@dataclass(frozen=True)
class EyeMetrics:
    """从眼图提取的标量测量指标（对齐 ``ChannelStats`` 风格）。

    - ``eye_height``：眼图开口高度（V，1 UI 中心时刻的高低电平均值差）。
    - ``eye_width``：眼图开口宽度（UI，归一化相位，眼图睁开的时间窗口）。
    - ``eye_opening``：眼图开口面积相对值（高度×宽度）。
    - ``jitter_pp``：峰峰值抖动（UI，过零点时间散布的峰峰值）。
    - ``jitter_rms``：均方根抖动（UI）。
    - ``rise_time``：上升时间（UI，低→高跨阈值的时间）。
    - ``fall_time``：下降时间（UI，高→低跨阈值的时间）。
    - ``snr``：信噪比估计（眼高 / 噪声标准差，线性比）。

    任一指标不可测量（样本不足/眼图闭合）时置 0.0，不抛。
    """

    eye_height: float = 0.0
    eye_width: float = 0.0
    eye_opening: float = 0.0
    jitter_pp: float = 0.0
    jitter_rms: float = 0.0
    rise_time: float = 0.0
    fall_time: float = 0.0
    snr: float = 0.0

    def to_payload(self) -> dict[str, object]:
        """扁平 dict（UI/日志友好，camelCase 键）。"""

        return {
            "type": "eye_metrics",
            "eyeHeight": self.eye_height,
            "eyeWidth": self.eye_width,
            "eyeOpening": self.eye_opening,
            "jitterPp": self.jitter_pp,
            "jitterRms": self.jitter_rms,
            "riseTime": self.rise_time,
            "fallTime": self.fall_time,
            "snr": self.snr,
        }


@dataclass(frozen=True)
class PolyMask:
    """眼图模板掩码（多边形，ANSI/Golden Gate 风格）。

    - ``vertices``：多边形顶点，``[N, 2]`` 数组，每行 ``[phase, level]``（phase 归一化
      0~1，level 为绝对电压或归一化幅度）。
    - ``name``：掩码名（如 ``"ANSI"`` / ``"custom"``）。
    掩码测试 = 判定眼图开口是否侵入掩码多边形内（违规）。
    """

    vertices: np.ndarray
    name: str = ""

    def __post_init__(self) -> None:
        arr = np.asarray(self.vertices, dtype=np.float32)
        if arr.ndim != 2 or arr.shape[1] != 2:
            raise ValueError("PolyMask vertices must be [N, 2] array of [phase, level]")
        object.__setattr__(self, "vertices", arr)


__all__ = ["EyeDiagram", "EyeMetrics", "PolyMask"]
