"""数据检查引擎。"""
from __future__ import annotations
from collections.abc import Sequence
import numpy as np
from embeddebug.serial_station.data_inspector.result import InspectionResult

DEFAULT_OUTLIER_THRESHOLD = 3.0

class DataInspector:
    """多通道数据检查器。"""

    def __init__(self, bin_count: int = 16, outlier_threshold: float = DEFAULT_OUTLIER_THRESHOLD) -> None:
        self._bin_count = max(4, min(256, int(bin_count)))
        self._threshold = float(outlier_threshold)

    @property
    def outlier_threshold(self) -> float:
        return self._threshold

    def inspect(self, values: np.ndarray, channel_names: Sequence[str] | None = None) -> InspectionResult:
        matrix = np.asarray(values, dtype=np.float64)
        if matrix.ndim != 2:
            raise ValueError("values must be 2D")
        n_ch = matrix.shape[1]
        names = tuple(channel_names) if channel_names and len(channel_names) == n_ch else tuple(f"ch{i}" for i in range(n_ch))
        stats = {name: self._stat(matrix[:, col]) for col, name in enumerate(names)}
        outliers = self._outliers(matrix, names)
        corrs = self._correlations(matrix, names)
        return InspectionResult(channel_stats=stats, correlations=corrs, outliers=outliers, threshold=self._threshold)

    def _stat(self, col: np.ndarray) -> dict:
        if col.size == 0:
            return {"min": 0, "max": 0, "mean": 0, "std": 0, "range": 0, "count": 0}
        return {"min": float(col.min()), "max": float(col.max()), "mean": float(col.mean()), "std": float(col.std()), "range": float(col.max() - col.min()), "count": int(col.size)}

    def _outliers(self, matrix: np.ndarray, names: tuple[str, ...]) -> list[dict]:
        result: list[dict] = []
        for col, name in enumerate(names):
            column = matrix[:, col]
            std = float(np.std(column))
            if column.size == 0 or std == 0:
                continue
            mean = float(np.mean(column))
            z = np.abs((column - mean) / std)
            for idx in np.nonzero(z > self._threshold)[0]:
                result.append({"channel": name, "index": int(idx), "value": float(column[idx]), "z_score": float(z[idx])})
        return result

    @staticmethod
    def _correlations(matrix: np.ndarray, names: tuple[str, ...]) -> dict[tuple[str, str], float]:
        result: dict[tuple[str, str], float] = {}
        stds = np.std(matrix, axis=0)
        for a in range(matrix.shape[1]):
            for b in range(a + 1, matrix.shape[1]):
                if stds[a] == 0 or stds[b] == 0:
                    result[(names[a], names[b])] = 0.0
                else:
                    val = float(np.corrcoef(matrix[:, a], matrix[:, b])[0, 1])
                    result[(names[a], names[b])] = 0.0 if np.isnan(val) else val
        return result
