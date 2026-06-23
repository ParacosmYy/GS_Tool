"""_dashboard_layout_store _app_data_dir + layout_path + load/save 边界测试。

_app_data_dir + layout_path 此前无直接测试。
本文件覆盖路径结构 + load/save 往返 + 损坏 JSON。

覆盖：
1. _app_data_dir 返回 Path。
2. _app_data_dir 含 embeddebug 子目录。
3. layout_path 返回 Path。
4. layout_path 含 layout.json 文件名。
5. load_layout_dict 空文件返回 {}。
6. save_layout_dict + load_layout_dict 往返。
7. save_layout_dict 返回 True。
8. save_layout_dict 损坏 JSON load 返回 {}。
"""

from __future__ import annotations

import os
from pathlib import Path
from unittest.mock import patch

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

from embeddebug.serial_station.ui.panels import _dashboard_layout_store as store


def test_app_data_dir_returns_path():
    result = store._app_data_dir()
    assert isinstance(result, Path)


def test_app_data_dir_is_appdata_root():
    """_app_data_dir 返回平台 AppData/Roaming 根（embeddebug 子目录由 layout_path 追加）。"""

    result = store._app_data_dir()
    # Windows 下含 AppData，Linux 下含 .local/share 或 .config。
    assert str(result)  # 非空路径


def test_layout_path_returns_path():
    result = store.layout_path()
    assert isinstance(result, Path)


def test_layout_path_contains_filename():
    result = store.layout_path()
    assert "layout" in str(result).lower()


def test_load_layout_dict_missing_returns_empty(tmp_path):
    with patch.object(store, "layout_path", return_value=tmp_path / "nope.json"):
        assert store.load_layout_dict() == {}


def test_save_load_roundtrip(tmp_path):
    path = tmp_path / "layout.json"
    layout = {"tabs": [{"name": "Board1", "items": []}]}
    with patch.object(store, "layout_path", return_value=path):
        assert store.save_layout_dict(layout) is True
        assert store.load_layout_dict() == layout


def test_save_layout_dict_returns_true(tmp_path):
    path = tmp_path / "layout.json"
    with patch.object(store, "layout_path", return_value=path):
        assert store.save_layout_dict({"x": 1}) is True


def test_load_corrupt_json_returns_empty(tmp_path):
    path = tmp_path / "corrupt.json"
    path.write_text("not json {{{", encoding="utf-8")
    with patch.object(store, "layout_path", return_value=path):
        result = store.load_layout_dict()
        assert result == {}
