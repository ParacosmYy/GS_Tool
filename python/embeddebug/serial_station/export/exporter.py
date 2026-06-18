"""统一数据导出器：根据 ExportConfig 将矩阵落盘为 CSV/TSV/JSON/npz。"""

from __future__ import annotations

import json
from pathlib import Path
from typing import Sequence

import numpy as np

from embeddebug.serial_station.export.format import ExportConfig, ExportFormat


class DataExporter:
    """多格式数据导出器。"""

    def export(
        self,
        values: np.ndarray,
        channel_names: Sequence[str],
        timestamps: np.ndarray | None,
        config: ExportConfig,
        path: str | Path,
    ) -> bool:
        """执行导出，成功 True / 失败 False。"""
        try:
            matrix = np.asarray(values, dtype=np.float64)
            if matrix.ndim != 2:
                raise ValueError("values 必须是二维矩阵")
            names = tuple(str(n) for n in channel_names)
            if matrix.shape[1] != len(names):
                raise ValueError("通道数与矩阵列数不一致")
            if matrix.shape[0] == 0:
                self._write_empty(config, path, names)
                return True
            rows, cols, ts = self._prepare(matrix, names, timestamps, config)
            fmt = config.format
            if fmt in (ExportFormat.CSV, ExportFormat.TSV):
                self._write_text(path, rows, cols, ts, config)
            elif fmt is ExportFormat.JSON:
                self._write_json(path, rows, cols, ts, config)
            elif fmt is ExportFormat.NUMPY:
                self._write_npz(path, rows, cols, ts)
            return True
        except (OSError, ValueError, KeyError, TypeError):
            return False

    def _prepare(self, matrix, names, timestamps, config):
        indices = self._channel_indices(names, config.channels)
        cols = [names[i] for i in indices]
        sub = matrix[:, indices]
        ts = None
        if timestamps is not None:
            ts = np.asarray(timestamps, dtype=np.float64).reshape(-1)
            if ts.shape[0] != matrix.shape[0]:
                raise ValueError("timestamps 长度与矩阵行数不一致")
            if config.time_range is not None:
                lo, hi = config.time_range
                mask = (ts >= lo) & (ts <= hi)
                sub = sub[mask]
                ts = ts[mask]
        elif config.time_range is not None:
            raise ValueError("指定 time_range 时必须提供 timestamps")
        if sub.shape[0] == 0:
            raise ValueError("时间范围过滤后无数据行")
        return sub, cols, ts

    @staticmethod
    def _channel_indices(names, channels):
        if channels is None:
            return list(range(len(names)))
        lookup = {n: i for i, n in enumerate(names)}
        missing = [c for c in channels if c not in lookup]
        if missing:
            raise KeyError(f"未找到通道: {missing}")
        return [lookup[c] for c in channels]

    @staticmethod
    def _write_empty(config, path, names):
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        if config.format in (ExportFormat.CSV, ExportFormat.TSV):
            with target.open("w", encoding="utf-8", newline="") as fh:
                if config.include_header:
                    fh.write(config.format.delimiter.join(names) + "\n")
        elif config.format is ExportFormat.JSON:
            target.write_text("[]\n", encoding="utf-8")
        else:
            np.savez(target, channels=np.array(names))

    def _write_text(self, path, rows, cols, ts, config):
        delim = config.format.delimiter
        fmt_spec = f"%.{config.decimal_places}f"
        header_cols = (["time"] if ts is not None else []) + cols
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        with target.open("w", encoding="utf-8", newline="") as fh:
            if config.include_header:
                fh.write(delim.join(header_cols) + "\n")
            for i in range(rows.shape[0]):
                parts = []
                if ts is not None:
                    parts.append(f"{float(ts[i]):.{config.decimal_places}f}")
                parts.extend(fmt_spec % float(v) for v in rows[i])
                fh.write(delim.join(parts) + "\n")

    def _write_json(self, path, rows, cols, ts, config):
        records = []
        for i in range(rows.shape[0]):
            record = {}
            if ts is not None:
                record["time"] = round(float(ts[i]), config.decimal_places)
            for j, col in enumerate(cols):
                record[col] = round(float(rows[i, j]), config.decimal_places)
            records.append(record)
        Path(path).parent.mkdir(parents=True, exist_ok=True)
        Path(path).write_text(json.dumps(records, ensure_ascii=False), encoding="utf-8")

    @staticmethod
    def _write_npz(path, rows, cols, ts):
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        payload = {"values": rows.astype(np.float64), "channels": np.array(cols)}
        if ts is not None:
            payload["timestamps"] = ts.astype(np.float64)
        np.savez(target, **payload)
