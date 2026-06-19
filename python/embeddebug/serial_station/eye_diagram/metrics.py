"""眼图指标测量：从折叠的 ``EyeDiagram`` 提取眼高/眼宽/抖动/上升下降时间。

入口 ``compute_eye_metrics(diagram, ...)`` 在密度网格上找眼图开口（中心相位列的高低
电平间距 = 眼高；过零点附近的时间散布 = 眼宽/抖动；跨阈值时间 = 上升/下降）。
``rise_fall_times`` 直接从原始 overlay 波形跨阈值测。

指标在样本不足或眼图闭合时置 0，不抛。仅依赖 numpy，不 import PyQt/transport。
"""

from __future__ import annotations

import numpy as np

from embeddebug.serial_station.eye_diagram.model import EyeDiagram, EyeMetrics

# 抖动测量：过零点附近 ±JITTER_HALF_WINDOW UI 内的相位列用于估计过零散布。
_JITTER_HALF_WINDOW = 0.1


def compute_eye_metrics(
    diagram: EyeDiagram,
    low_threshold: float | None = None,
    high_threshold: float | None = None,
) -> EyeMetrics:
    """从 ``diagram`` 提取 ``EyeMetrics``。

    - ``low_threshold`` / ``high_threshold``：上升/下降时间测量的跨阈值电压；
      默认用样本均值作为判决中线，低/高阈值取中线 ∓ 幅度 10%。
    - 眼高：中心相位列（phase=0.5）高电平均值 − 低电平均值。
    - 眼宽：眼图睁开的时间窗口（归一化 UI）。
    - 抖动：过零点（phase≈0 与 ≈1 边界）样本相位散布的 pp / rms。
    """

    overlay = diagram.overlay
    period_count = diagram.period_count
    phase_bins = diagram.phase_bins
    if period_count == 0 or phase_bins == 0 or overlay.size == 0:
        return EyeMetrics()

    matrix = overlay.reshape(period_count, phase_bins).astype(np.float64, copy=False)
    overall_min = float(matrix.min())
    overall_max = float(matrix.max())
    amplitude = overall_max - overall_min
    if amplitude <= 0:
        return EyeMetrics()  # 恒定信号，无眼图。

    mid = (overall_max + overall_min) / 2.0
    if low_threshold is None:
        low_threshold = mid - 0.1 * amplitude
    if high_threshold is None:
        high_threshold = mid + 0.1 * amplitude

    eye_height = _eye_height(matrix, mid)
    eye_width = _eye_width(matrix, mid)
    jitter_pp, jitter_rms = _jitter(matrix, mid, phase_bins)
    rise_time, fall_time = _rise_fall(matrix, low_threshold, high_threshold, phase_bins)
    snr = _snr(matrix, mid, eye_height)

    return EyeMetrics(
        eye_height=eye_height,
        eye_width=eye_width,
        eye_opening=eye_height * eye_width,
        jitter_pp=jitter_pp,
        jitter_rms=jitter_rms,
        rise_time=rise_time,
        fall_time=fall_time,
        snr=snr,
    )


def _eye_height(matrix: np.ndarray, mid: float) -> float:
    """眼高 = 中心列高电平均值 − 低电平均值。"""

    center_col = matrix.shape[1] // 2
    col = matrix[:, center_col]
    high = col[col > mid]
    low = col[col < mid]
    if high.size == 0 or low.size == 0:
        return 0.0
    return float(high.mean() - low.mean())


def _eye_width(matrix: np.ndarray, mid: float) -> float:
    """眼宽 = 中心附近「眼图睁开」的相位占比。

    遍历相位列，对每列算高/低电平均值差；差值 > 阈值（幅度 5%）视为睁开，
    睁开的连续相位宽度归一化到 UI。
    """

    if matrix.shape[1] < 4:
        return 0.0
    center = matrix.shape[1] // 2
    half = max(matrix.shape[1] // 4, 1)
    lo = max(center - half, 0)
    hi = min(center + half + 1, matrix.shape[1])
    center_region = matrix[:, lo:hi]

    amplitude = float(center_region.max() - center_region.min())
    if amplitude <= 0:
        return 0.0
    threshold = 0.05 * amplitude
    widths: list[int] = []
    run = 0
    for col in range(center_region.shape[1]):
        c = center_region[:, col]
        high = c[c > mid]
        low = c[c < mid]
        gap = (high.mean() - low.mean()) if high.size and low.size else 0.0
        if gap > threshold:
            run += 1
        else:
            if run:
                widths.append(run)
            run = 0
    if run:
        widths.append(run)
    if not widths:
        return 0.0
    # 归一化到 UI：宽度 bin 数 / phase_bins。
    return float(max(widths)) / float(matrix.shape[1])


def _jitter(
    matrix: np.ndarray, mid: float, phase_bins: int
) -> tuple[float, float]:
    """过零点抖动：phase≈0 与 phase≈phase_bins-1 边界附近的过零相位散布。

    返回 (峰峰值, 均方根)，单位 UI。
    """

    window = max(int(_JITTER_HALF_WINDOW * phase_bins), 1)
    # 取相位边界附近两段：开头 [0, window) 与结尾 [phase_bins-window, phase_bins)。
    edge = np.concatenate([matrix[:, :window], matrix[:, phase_bins - window:]], axis=1)
    # 找每行（每个 UI 周期）在边界段的过零相位（最接近 mid 的相位 bin）。
    crossings: list[float] = []
    for row in edge:
        # 最近邻过零：跨过 mid 的相邻 bin。
        above = row > mid
        cross_idx = np.where(np.diff(above.astype(int)) != 0)[0]
        if cross_idx.size:
            # 取第一个过零点，归一化相位。
            idx = float(cross_idx[0])
            # 边界段前半段相位 ~[-window,0)，后半段 ~[phase_bins-window, phase_bins)，
            # 映射到过零点附近相位。
            crossings.append(idx / phase_bins)
    if len(crossings) < 2:
        return 0.0, 0.0
    arr = np.asarray(crossings)
    pp = float(arr.max() - arr.min())
    rms = float(np.sqrt(np.mean((arr - arr.mean()) ** 2)))
    return pp, rms


def _rise_fall(
    matrix: np.ndarray,
    low_threshold: float,
    high_threshold: float,
    phase_bins: int,
) -> tuple[float, float]:
    """上升/下降时间（归一化 UI）：跨 low→high 与 high→low 的平均相位跨度。

    对每个 UI 周期找首个跨 low_threshold 与 high_threshold 的相位，差值 = 转换时间。
    """

    rises: list[float] = []
    falls: list[float] = []
    for row in matrix:
        low_idx = np.where(row > low_threshold)[0]
        high_idx = np.where(row > high_threshold)[0]
        if low_idx.size and high_idx.size:
            r_from = float(low_idx[0])
            r_to = float(high_idx[0])
            if r_to >= r_from:
                rises.append((r_to - r_from) / phase_bins)
        # 下降：首个跨 high（已下降）到首个跨 low。
        if high_idx.size and low_idx.size:
            # 从高位回落：找 high_idx 之后首个 < low。
            below_low = np.where(row < low_threshold)[0]
            if below_low.size and high_idx[0] < below_low[-1]:
                after = below_low[below_low > high_idx[0]]
                if after.size:
                    f_from = float(high_idx[0])
                    f_to = float(after[0])
                    falls.append((f_to - f_from) / phase_bins)
    rise = float(np.mean(rises)) if rises else 0.0
    fall = float(np.mean(falls)) if falls else 0.0
    return rise, fall


def _snr(matrix: np.ndarray, mid: float, eye_height: float) -> float:
    """SNR = 眼高 / 中心列噪声标准差（线性比）。"""

    if eye_height <= 0:
        return 0.0
    center_col = matrix.shape[1] // 2
    col = matrix[:, center_col]
    high = col[col > mid]
    low = col[col < mid]
    if high.size < 2 or low.size < 2:
        return 0.0
    noise_std = float(np.sqrt((high.var() + low.var()) / 2.0))
    if noise_std <= 0:
        return 0.0
    return eye_height / noise_std


__all__ = ["compute_eye_metrics"]
