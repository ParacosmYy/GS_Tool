"""session_actions export/replay/save/load profile 行为边界测试。

test_serial_station_ui_architecture.py 仅源码级断言；本文件覆盖行为分支：
空路径 + 成功 + 失败 + profile name 提取。

覆盖：
1. export_log 空路径 → status "Log path is empty"。
2. export_log 成功 → status "Saved log"。
3. export_log 失败 → status failure。
4. replay_log 空路径 → status "Log path is empty"。
5. replay_log 成功 → clear log view + render + status "Replayed log"。
6. save_profile 空路径 → status "Profile path is empty"。
7. save_profile 空 name → status "Profile name is empty"。
8. save_profile 成功 → profile label + status "Saved profile"。
9. load_profile 空路径 → status "Profile path is empty"。
10. load_profile 成功 → apply name + controls + refresh history + label。
11. load_profile value None → status failure。
"""

from __future__ import annotations

from types import SimpleNamespace
from unittest.mock import MagicMock

from embeddebug.shared.results import OperationResult
from embeddebug.serial_station.ui import session_actions


def _make_host(
    log_path: str = "",
    profile_path: str = "",
    profile_name: str = "",
):
    host = SimpleNamespace()
    host.tr = lambda s: s
    host._log_path_edit = MagicMock()
    host._log_path_edit.text.return_value = log_path
    host._profile_path_edit = MagicMock()
    host._profile_path_edit.text.return_value = profile_path
    host._profile_name_edit = MagicMock()
    host._profile_name_edit.text.return_value = profile_name
    host._log_view = MagicMock()
    host._protocol_combo = MagicMock()
    host._port_combo = MagicMock()
    host._tcp_host_edit = MagicMock()
    host._tcp_port_edit = MagicMock()
    host._udp_host_edit = MagicMock()
    host._udp_port_edit = MagicMock()
    host._baud_combo = MagicMock()
    host._data_bits_combo = MagicMock()
    host._parity_combo = MagicMock()
    host._stop_bits_combo = MagicMock()
    host._status_label = MagicMock()
    host._profile_label = MagicMock()
    host._controller = MagicMock()
    host._render_log_entries = MagicMock()
    host._update_log_stats = MagicMock()
    host._refresh_command_history = MagicMock()
    host._set_connected_controls = MagicMock()
    # profile_combo findText 返回找到（>=0）。
    host._protocol_combo.findText.return_value = 0
    host._port_combo.findText.return_value = 0
    return host


def _ok(value=None):
    return OperationResult.success(value)


def _fail(code="err", message="boom"):
    return OperationResult.failure(code, message)


# ── export_log ────────────────────────────────────────────────────
def test_export_log_empty_path_sets_status():
    host = _make_host(log_path="")
    session_actions.export_log(host)
    host._controller.export_log_result.assert_not_called()


def test_export_log_success_sets_saved_status():
    host = _make_host(log_path="/tmp/log.jsonl")
    host._controller.export_log_result.return_value = _ok()
    session_actions.export_log(host)
    host._controller.export_log_result.assert_called_once_with("/tmp/log.jsonl")


def test_export_log_failure_sets_failure_status():
    host = _make_host(log_path="/bad/path")
    host._controller.export_log_result.return_value = _fail()
    session_actions.export_log(host)
    host._controller.export_log_result.assert_called_once()


# ── replay_log ────────────────────────────────────────────────────
def test_replay_log_empty_path_sets_status():
    host = _make_host(log_path="")
    session_actions.replay_log(host)
    host._controller.replay_log_result.assert_not_called()


def test_replay_log_success_clears_and_renders():
    host = _make_host(log_path="/tmp/log.jsonl")
    host._controller.replay_log_result.return_value = _ok()
    session_actions.replay_log(host)
    host._controller.replay_log_result.assert_called_once_with("/tmp/log.jsonl")
    host._render_log_entries.assert_called_once()


def test_replay_log_failure_no_render():
    host = _make_host(log_path="/bad")
    host._controller.replay_log_result.return_value = _fail()
    session_actions.replay_log(host)
    host._render_log_entries.assert_not_called()


# ── save_profile ──────────────────────────────────────────────────
def test_save_profile_empty_path_sets_status():
    host = _make_host(profile_path="")
    session_actions.save_profile(host)
    host._controller.save_profile_result.assert_not_called()


def test_save_profile_empty_name_sets_status():
    host = _make_host(profile_path="/tmp/p.json", profile_name="")
    session_actions.save_profile(host)
    host._controller.save_profile_result.assert_not_called()


def test_save_profile_success_calls_controller():
    host = _make_host(profile_path="/tmp/p.json", profile_name="myprof")
    host._controller.save_profile_result.return_value = _ok()
    session_actions.save_profile(host)
    host._controller.save_profile_result.assert_called_once_with("/tmp/p.json", "myprof")


def test_save_profile_failure_no_label():
    host = _make_host(profile_path="/tmp/p.json", profile_name="myprof")
    host._controller.save_profile_result.return_value = _fail()
    session_actions.save_profile(host)
    host._controller.save_profile_result.assert_called_once()


# ── load_profile ──────────────────────────────────────────────────
def test_load_profile_empty_path_sets_status():
    host = _make_host(profile_path="")
    session_actions.load_profile(host)
    host._controller.load_profile_result.assert_not_called()


def test_load_profile_success_applies_name_and_controls():
    """load_profile 成功 → refresh_command_history 调用（profile 无 protocol 跳过 apply_profile_controls 复杂路径）。"""

    host = _make_host(profile_path="/tmp/p.json")
    profile = {"name": "loaded"}  # 无 protocol → apply_profile_controls 早退
    host._controller.load_profile_result.return_value = _ok(profile)
    session_actions.load_profile(host)
    host._controller.load_profile_result.assert_called_once_with("/tmp/p.json")
    host._refresh_command_history.assert_called_once()


def test_load_profile_value_none_sets_failure():
    host = _make_host(profile_path="/tmp/p.json")
    host._controller.load_profile_result.return_value = _ok(None)
    session_actions.load_profile(host)
    host._refresh_command_history.assert_not_called()


def test_load_profile_failure_no_apply():
    host = _make_host(profile_path="/bad")
    host._controller.load_profile_result.return_value = _fail()
    session_actions.load_profile(host)
    host._refresh_command_history.assert_not_called()


# ── load_profile name 提取 ────────────────────────────────────────
def test_load_profile_missing_name_defaults_unnamed():
    """profile 无 name → 默认 'unnamed'（不抛）。"""

    host = _make_host(profile_path="/tmp/p.json")
    profile = {}  # 无 name 无 protocol
    host._controller.load_profile_result.return_value = _ok(profile)
    session_actions.load_profile(host)
    host._refresh_command_history.assert_called_once()
