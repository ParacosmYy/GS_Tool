"""眼图模板测试：判定眼图开口是否侵入多边形掩码区域。

入口 ``test_mask(diagram, mask, ...)`` 用射线法判断掩码多边形是否与密度网格的
「睁开区域」相交（违规）。掩码是 ``PolyMask``（[phase, level] 顶点多边形），
通常居中在眼图开口（phase 0.5，幅度 0），开口收缩到掩码内即违规。

仅依赖 numpy，不 import PyQt、不依赖 transport。
"""

from __future__ import annotations

import numpy as np

from embeddebug.serial_station.eye_diagram.model import EyeDiagram, EyeMetrics, PolyMask


def point_in_polygon(point: tuple[float, float], polygon: np.ndarray) -> bool:
    """射线法判断点是否在多边形内（[N,2] 顶点，逆/顺时针均可）。

    - ``point``：``(phase, level)``。
    - ``polygon``：``[N, 2]`` 顶点数组。
    """

    x, y = float(point[0]), float(point[1])
    poly = np.asarray(polygon, dtype=np.float64)
    n = poly.shape[0]
    inside = False
    j = n - 1
    for i in range(n):
        xi, yi = float(poly[i, 0]), float(poly[i, 1])
        xj, yj = float(poly[j, 0]), float(poly[j, 1])
        # 标准射线法：边的端点跨过 y 水平线，且交点 x 在测试点左侧则翻转。
        if ((yi > y) != (yj > y)) and (x < (xj - xi) * (y - yi) / (yj - yi) + xi):
            inside = not inside
        j = i
    return inside


def mask_violation_points(
    diagram: EyeDiagram, mask: PolyMask, mid: float | None = None
) -> np.ndarray:
    """找出落入掩码内的「眼图睁开样本」相位-幅度点。

    遍历折叠 overlay 的每个 (phase, level) 样本点，筛出落在掩码多边形内且
    处于眼图睁开区（高/低电平外侧，非过渡带）的点。返回 ``[M, 2]`` 数组
    （phase, level）；无违规返回空数组。
    """

    overlay = diagram.overlay
    period_count = diagram.period_count
    phase_bins = diagram.phase_bins
    if period_count == 0 or phase_bins == 0 or overlay.size == 0:
        return np.empty((0, 2), dtype=np.float32)

    matrix = overlay.reshape(period_count, phase_bins).astype(np.float64, copy=False)
    if mid is None:
        mid = float((matrix.max() + matrix.min()) / 2.0)
    vertices = mask.vertices.astype(np.float64)

    violators: list[tuple[float, float]] = []
    for row in matrix:
        for col in range(phase_bins):
            level = float(row[col])
            # 只判睁开区的点（远离中线 = 高/低电平稳定区）。
            if abs(level - mid) < 1e-9:
                continue
            phase = col / phase_bins
            if point_in_polygon((phase, level), vertices):
                violators.append((phase, level))
    if not violators:
        return np.empty((0, 2), dtype=np.float32)
    return np.asarray(violators, dtype=np.float32)


def evaluate_mask(
    diagram: EyeDiagram, mask: PolyMask, mid: float | None = None
) -> bool:
    """掩码测试：返回 True = 违规（眼图开口侵入掩码），False = 通过。

    等价于 ``mask_violation_points`` 非空。便捷布尔入口。
    （函数名用 evaluate_ 前缀避免 pytest 误收集为测试用例。）
    """

    return mask_violation_points(diagram, mask, mid=mid).shape[0] > 0


def mask_margin(
    diagram: EyeDiagram, mask: PolyMask, metrics: EyeMetrics | None = None
) -> float:
    """掩码裕量：眼高相对掩码高度的余量比（>0 = 通过裕量，<0 = 违规）。

    粗估：眼图开口高度 − 掩码多边形幅度跨度，归一化到掩码跨度。
    精确裕量需几何距离，本函数给趋势级估计（UI 用）。
    """

    if metrics is None:
        metrics = compute_or_skip(diagram)
    mask_span = float(mask.vertices[:, 1].max() - mask.vertices[:, 1].min())
    if mask_span <= 0:
        return 0.0
    return (metrics.eye_height - mask_span) / mask_span


def compute_or_skip(diagram: EyeDiagram) -> EyeMetrics:
    """惰性算指标（mask_margin 默认用）。"""

    from embeddebug.serial_station.eye_diagram.metrics import compute_eye_metrics

    return compute_eye_metrics(diagram)


__all__ = ["mask_margin", "mask_violation_points", "point_in_polygon"]
