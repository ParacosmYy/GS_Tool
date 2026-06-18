"""数据对比引擎。"""
from __future__ import annotations
from typing import Any
import numpy as np
from embeddebug.serial_station.diff_tools.config import DiffConfig
from embeddebug.serial_station.diff_tools.result import DiffResult

class DataDiffer:
    """二维数值数据的逐单元对比器。"""

    def compare(self, a: np.ndarray, b: np.ndarray, a_names: list[str], b_names: list[str], config: DiffConfig | None = None) -> DiffResult:
        cfg = config or DiffConfig()
        cfg.validate()
        arr_a = np.asarray(a, dtype=np.float64)
        arr_b = np.asarray(b, dtype=np.float64)
        if arr_a.ndim != 2 or arr_b.ndim != 2:
            raise ValueError("必须是二维矩阵")
        pairs = self._pairs(a_names, b_names, cfg.ignore_columns)
        common = min(arr_a.shape[0], arr_b.shape[0])
        matching = differing = 0
        max_diff = 0.0
        diffs: list[dict[str, Any]] = []
        for i in range(common):
            for name, ai, bi in pairs:
                va, vb = float(arr_a[i, ai]), float(arr_b[i, bi])
                delta = abs(va - vb)
                if delta <= cfg.tolerance:
                    matching += 1
                else:
                    differing += 1
                    max_diff = max(max_diff, delta)
                    if len(diffs) < cfg.max_display_rows:
                        diffs.append({"row": i, "column": name, "a_value": va, "b_value": vb, "delta": va - vb})
        unmatched_a = max(arr_a.shape[0] - common, 0)
        unmatched_b = max(arr_b.shape[0] - common, 0)
        return DiffResult(summary={"matching_cells": matching, "differing_cells": differing, "total_cells": matching + differing, "max_diff": max_diff, "a_rows": arr_a.shape[0], "b_rows": arr_b.shape[0], "unmatched_a_rows": unmatched_a, "unmatched_b_rows": unmatched_b, "compared_columns": [n for n, _, _ in pairs]}, row_diffs=diffs, is_identical=(differing == 0 and unmatched_a == 0 and unmatched_b == 0))

    @staticmethod
    def _pairs(a_names, b_names, ignore):
        ignore_set = set(ignore)
        b_idx = {n: i for i, n in enumerate(b_names)}
        return [(n, ai, b_idx[n]) for ai, n in enumerate(a_names) if n not in ignore_set and n in b_idx]
