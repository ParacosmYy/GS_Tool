"""数据导出子模块单测。"""

from __future__ import annotations

import csv
import json
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import numpy as np
import pytest

from embeddebug.serial_station.export import DataExporter, ExportConfig, ExportFormat


def _matrix():
    return np.array([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 9.0]], dtype=np.float32)


def _names():
    return ["temp", "volt", "curr"]


def _timestamps():
    return np.array([0.0, 1.0, 2.0], dtype=np.float64)


@pytest.mark.parametrize("fmt", list(ExportFormat))
def test_format_round_trip(fmt, tmp_path):
    ext = "npz" if fmt is ExportFormat.NUMPY else fmt.value
    path = tmp_path / f"out.{ext}"
    config = ExportConfig(format=fmt)
    assert DataExporter().export(_matrix(), _names(), _timestamps(), config, path) is True
    if fmt is ExportFormat.NUMPY:
        data = np.load(path, allow_pickle=False)
        assert np.allclose(data["values"], _matrix())
    elif fmt is ExportFormat.JSON:
        records = json.loads(path.read_text(encoding="utf-8"))
        assert len(records) == 3
        assert np.allclose([r["temp"] for r in records], [1.0, 4.0, 7.0])
    else:
        delim = "\t" if fmt is ExportFormat.TSV else ","
        with path.open(encoding="utf-8") as fh:
            rows = list(csv.reader(fh, delimiter=delim))
        assert rows[0] == ["time", "temp", "volt", "curr"]
        assert float(rows[1][1]) == pytest.approx(1.0)


def test_csv_header_toggle(tmp_path):
    path = tmp_path / "no_header.csv"
    config = ExportConfig(format=ExportFormat.CSV, include_header=False)
    assert DataExporter().export(_matrix(), _names(), None, config, path) is True
    lines = path.read_text(encoding="utf-8").strip().splitlines()
    assert len(lines) == 3


def test_channel_subset(tmp_path):
    path = tmp_path / "sub.tsv"
    config = ExportConfig(format=ExportFormat.TSV, channels=("volt", "temp"))
    assert DataExporter().export(_matrix(), _names(), None, config, path) is True
    with path.open(encoding="utf-8") as fh:
        rows = list(csv.reader(fh, delimiter="\t"))
    assert rows[0] == ["volt", "temp"]
    assert float(rows[1][0]) == pytest.approx(2.0)


def test_time_range_filter(tmp_path):
    path = tmp_path / "filt.json"
    config = ExportConfig(format=ExportFormat.JSON, time_range=(0.5, 1.5))
    assert DataExporter().export(_matrix(), _names(), _timestamps(), config, path) is True
    records = json.loads(path.read_text(encoding="utf-8"))
    assert [r["time"] for r in records] == [1.0]


def test_decimal_places(tmp_path):
    path = tmp_path / "dec.csv"
    config = ExportConfig(format=ExportFormat.CSV, decimal_places=2)
    assert DataExporter().export(np.array([[1.23456789]], dtype=np.float32), ["c1"], None, config, path) is True
    with path.open(encoding="utf-8") as fh:
        rows = list(csv.reader(fh))
    # header row [c1], data row [1.23] — data is in column 0
    assert rows[1][0] == "1.23"


def test_empty_data_csv(tmp_path):
    path = tmp_path / "empty.csv"
    config = ExportConfig(format=ExportFormat.CSV)
    assert DataExporter().export(np.empty((0, 3), dtype=np.float32), _names(), None, config, path) is True
    assert path.read_text(encoding="utf-8").splitlines() == ["temp,volt,curr"]


def test_invalid_returns_false(tmp_path):
    config = ExportConfig(format=ExportFormat.CSV, time_range=(0.0, 1.0))
    assert DataExporter().export(_matrix(), _names(), None, config, tmp_path / "bad.csv") is False


def test_unknown_channel_returns_false(tmp_path):
    config = ExportConfig(format=ExportFormat.CSV, channels=("nope",))
    assert DataExporter().export(_matrix(), _names(), None, config, tmp_path / "miss.csv") is False


def test_from_extension():
    assert ExportFormat.from_extension("a.csv") is ExportFormat.CSV
    with pytest.raises(ValueError):
        ExportFormat.from_extension("a.bin")


def test_export_config_validation():
    with pytest.raises(ValueError):
        ExportConfig(format=ExportFormat.CSV, decimal_places=-1)
    with pytest.raises(ValueError):
        ExportConfig(format=ExportFormat.CSV, time_range=(2.0, 1.0))
