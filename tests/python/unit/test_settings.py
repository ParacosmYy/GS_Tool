"""AppSettings 设置持久化单元测试。

状态（2026-06-21 审计）：被测模块 ``embeddebug.serial_station.settings`` 尚未落地
（PRD-135/136 服务层重构规划但 ``settings/`` 包未创建）。本文件保留作为契约测试，
模块落地后 ``importorskip`` 会自动恢复执行；在落地前用 skip 防止收集失败阻塞 pytest。
"""

from __future__ import annotations

import json
import os

os.environ.setdefault("QT_QPA_PLATFORM", "offscreen")

import pytest

# 待 settings 包落地后此行自动放行；当前会以 Skipped 状态跳过整文件。
pytest.importorskip("embeddebug.serial_station.settings")

from embeddebug.serial_station.settings import AppSettings, SettingKey
from embeddebug.serial_station.settings.keys import DEFAULTS


def test_set_and_get_str():
    s = AppSettings()
    s.set(SettingKey.THEME, "light")
    assert s.get_str(SettingKey.THEME) == "light"


def test_set_and_get_int():
    s = AppSettings()
    s.set(SettingKey.WINDOW_WIDTH, 1600)
    assert s.get_int(SettingKey.WINDOW_WIDTH) == 1600


def test_set_and_get_bool():
    s = AppSettings()
    s.set(SettingKey.SIDEBAR_COLLAPSED, True)
    assert s.get_bool(SettingKey.SIDEBAR_COLLAPSED) is True


def test_set_and_get_float():
    s = AppSettings()
    s.set(SettingKey.REFRESH_HZ, 59.94)
    assert s.get_float(SettingKey.REFRESH_HZ) == pytest.approx(59.94)


def test_bool_not_treated_as_int():
    s = AppSettings()
    s.set(SettingKey.WINDOW_WIDTH, True)
    assert s.get_int(SettingKey.WINDOW_WIDTH, -1) == -1


def test_defaults_present():
    s = AppSettings()
    assert s.get_str(SettingKey.THEME) == DEFAULTS[SettingKey.THEME.value]
    assert s.get_int(SettingKey.BAUD_RATE) == 115200


def test_get_missing_key_returns_default():
    s = AppSettings()
    assert s.get("no.such.key", "sentinel") == "sentinel"


def test_reset_restores_defaults():
    s = AppSettings()
    s.set(SettingKey.THEME, "light")
    s.reset()
    assert s.get_str(SettingKey.THEME) == DEFAULTS[SettingKey.THEME.value]


def test_set_none_removes_key():
    s = AppSettings()
    s.set(SettingKey.THEME, None)
    assert s.get(SettingKey.THEME, "gone") == "gone"


def test_save_load_round_trip(tmp_path):
    p = tmp_path / "settings.json"
    s = AppSettings()
    s.set(SettingKey.THEME, "light")
    s.set(SettingKey.WINDOW_WIDTH, 1920)
    assert s.save(p) is True
    other = AppSettings()
    assert other.load(p) is True
    assert other.get_str(SettingKey.THEME) == "light"
    assert other.get_int(SettingKey.WINDOW_WIDTH) == 1920


def test_save_creates_parent_dirs(tmp_path):
    p = tmp_path / "nested" / "deep" / "settings.json"
    assert AppSettings().save(p) is True
    assert p.exists()


def test_load_missing_returns_false(tmp_path):
    s = AppSettings()
    assert s.load(tmp_path / "absent.json") is False
    assert s.get_str(SettingKey.THEME) == DEFAULTS[SettingKey.THEME.value]


def test_load_corrupt_json_returns_false(tmp_path):
    p = tmp_path / "bad.json"
    p.write_text("{not valid json", encoding="utf-8")
    s = AppSettings()
    assert s.load(p) is False


def test_load_merges_missing_defaults(tmp_path):
    p = tmp_path / "settings.json"
    p.write_text(json.dumps({"ui.theme": "light"}), encoding="utf-8")
    s = AppSettings()
    assert s.load(p) is True
    assert s.get_str(SettingKey.THEME) == "light"
    assert s.get_int(SettingKey.BAUD_RATE) == 115200


def test_namespace_baud_key():
    s = AppSettings()
    s.set(SettingKey.BAUD_RATE, 9600)
    assert s.get("transport.baud") == 9600


def test_arbitrary_namespace_key():
    s = AppSettings()
    s.set("transport.flow_control", "hardware")
    assert s.get_str("transport.flow_control") == "hardware"


def test_list_value_round_trip(tmp_path):
    p = tmp_path / "settings.json"
    s = AppSettings()
    s.set(SettingKey.COMMAND_HISTORY, ["AT", "AT+RST"])
    s.save(p)
    other = AppSettings()
    other.load(p)
    assert other.get_list(SettingKey.COMMAND_HISTORY) == ["AT", "AT+RST"]


def test_snapshot_is_isolated():
    s = AppSettings()
    snap = s.snapshot()
    snap[SettingKey.THEME.value] = "mutated"
    assert s.get_str(SettingKey.THEME) == DEFAULTS[SettingKey.THEME.value]
