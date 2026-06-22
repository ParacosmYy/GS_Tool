"""录制格式：CSV / JSONL 读写。"""

from __future__ import annotations

import csv
import json
from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from typing import Any
from collections.abc import Iterator

import numpy as np

from embeddebug.serial_station.core.measurements import ChannelBatch

DEFAULT_DT_NS = 10_000_000


class RecordingFormat(Enum):
    CSV = "csv"
    JSONL = "jsonl"

    @classmethod
    def from_extension(cls, path: str | Path) -> RecordingFormat:
        suffix = Path(path).suffix.lower().lstrip(".")
        if suffix == "csv":
            return cls.CSV
        if suffix == "jsonl":
            return cls.JSONL
        raise ValueError(f"无法识别扩展名: {suffix}")


@dataclass
class RecordingHeader:
    start_time_ns: int
    dt_ns: int = DEFAULT_DT_NS
    channel_names: tuple[str, ...] = ()

    @property
    def channel_count(self) -> int:
        return len(self.channel_names)

    def to_dict(self) -> dict[str, Any]:
        return {"start_time_ns": self.start_time_ns, "dt_ns": self.dt_ns, "channel_names": list(self.channel_names)}

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> RecordingHeader:
        return cls(start_time_ns=int(data["start_time_ns"]), dt_ns=int(data.get("dt_ns", DEFAULT_DT_NS)), channel_names=tuple(str(n) for n in data.get("channel_names", ())))


class RecordingWriter:
    """按格式流式写入。"""

    def __init__(self, fmt: RecordingFormat, header: RecordingHeader) -> None:
        self._fmt = fmt
        self._header = header
        self._handle = None

    def open(self, path: str | Path) -> None:
        self._handle = Path(path).open("w", encoding="utf-8", newline="")
        if self._fmt == RecordingFormat.CSV:
            self._handle.write("# EDREC " + json.dumps(self._header.to_dict()) + "\n")
            self._handle.write("t_ns," + ",".join(self._header.channel_names) + "\n")
        else:
            self._handle.write(json.dumps({"__header__": self._header.to_dict()}) + "\n")

    def write_batch(self, batch: ChannelBatch) -> None:
        if self._fmt == RecordingFormat.CSV:
            writer = csv.writer(self._handle)
            for i, row in enumerate(batch.values):
                ts = batch.t0_ns + i * batch.dt_ns
                writer.writerow([ts, *[f"{float(v):.9g}" for v in row]])
        else:
            self._handle.write(json.dumps({"t0_ns": batch.t0_ns, "dt_ns": batch.dt_ns, "values": batch.values.astype(np.float32).tolist()}) + "\n")

    def close(self) -> None:
        if self._handle:
            self._handle.close()


class RecordingReader:
    """按格式流式读取。"""

    def __init__(self, fmt: RecordingFormat) -> None:
        self._fmt = fmt
        self.header: RecordingHeader | None = None
        self._handle = None

    def open(self, path: str | Path) -> RecordingHeader:
        self._handle = Path(path).open("r", encoding="utf-8", newline="")
        if self._fmt == RecordingFormat.CSV:
            header_line = self._handle.readline()
            self.header = RecordingHeader.from_dict(json.loads(header_line[len("# EDREC "):].strip()))
            self._handle.readline()
        else:
            envelope = json.loads(self._handle.readline())
            self.header = RecordingHeader.from_dict(envelope["__header__"])
        return self.header  # type: ignore[return-value]

    def iter_batches(self) -> Iterator[ChannelBatch]:
        if self._fmt == RecordingFormat.CSV:
            reader = csv.reader(self._handle)
            rows = [tuple(float(c) for c in row) for row in reader if row]
            if rows:
                values = np.array([[r[i] for i in range(1, len(r))] for r in rows], dtype=np.float32)
                yield ChannelBatch(self.header.channel_names, values, int(rows[0][0]), self.header.dt_ns)  # type: ignore[union-attr]
        else:
            for line in self._handle:
                if line.strip():
                    payload = json.loads(line)
                    values = np.asarray(payload["values"], dtype=np.float32)
                    yield ChannelBatch(self.header.channel_names, values, int(payload["t0_ns"]), int(payload["dt_ns"]))  # type: ignore[union-attr]

    def close(self) -> None:
        if self._handle:
            self._handle.close()
