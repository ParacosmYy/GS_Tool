"""眼图分析器（Wave 60）。

提供数字信号眼图的折叠叠加、参数测量（眼高/眼宽/抖动/上升下降时间/SNR）与
模板测试（多边形掩码违规检测）。输入一维采样序列，输出 ``EyeDiagram``（折叠网格）
+ ``EyeMetrics``（标量指标）。

仅依赖 numpy + 标准库；不 import PyQt、不依赖 transport。UI/渲染层只 import
本 ``__init__`` 聚合的公共 API。

公开符号：
- ``EyeDiagram`` / ``EyeMetrics`` / ``PolyMask``：数据模型。
- ``fold_eye``：位周期折叠叠加。
- ``compute_eye_metrics``：指标测量。
- ``test_mask`` / ``mask_violation_points`` / ``mask_margin`` / ``point_in_polygon``：模板测试。
"""

from __future__ import annotations

from embeddebug.serial_station.eye_diagram.fold import fold_eye
from embeddebug.serial_station.eye_diagram.mask import (
    evaluate_mask,
    mask_margin,
    mask_violation_points,
    point_in_polygon,
)
from embeddebug.serial_station.eye_diagram.metrics import compute_eye_metrics
from embeddebug.serial_station.eye_diagram.model import (
    EyeDiagram,
    EyeMetrics,
    PolyMask,
)

__all__ = [
    "EyeDiagram",
    "EyeMetrics",
    "PolyMask",
    "compute_eye_metrics",
    "evaluate_mask",
    "fold_eye",
    "mask_margin",
    "mask_violation_points",
    "point_in_polygon",
]
