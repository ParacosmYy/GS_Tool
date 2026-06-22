"""session_operations 单元测试 — service-backed 日志/回放/profile 操作。

这些 helper 把 SerialLogService / SerialReplayService / SerialProfileService
包装成 OperationResult 风格的纯函数。测试覆盖：
- export_log_result: 写入 jsonl + 失败路径（OSError → failure）
- replay_entries_result: 加载并应用 entry_factory + 失败传播
- save_profile_result / load_profile_result: profile round-trip + 失败路径
- entry_factory / event_factory 回调契约
"""

from __future__ import annotations

import json

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import (
    entry_from_event,
    event_from_entry,
)
from embeddebug.serial_station.controllers.session_operations import (
    export_log_result,
    load_profile_result,
    replay_entries_result,
    save_profile_result,
)


# ----------------------- export_log_result -----------------------


def test_export_log_result_writes_entries_as_jsonl(tmp_path):
    log_path = tmp_path / "session.jsonl"
    entries = [
        SerialWorkbenchLogEntry("tx", "ping", b"ping"),
        SerialWorkbenchLogEntry("rx", "pong", b"pong"),
    ]
    result = export_log_result(
        log_path,
        entries,
        lambda entry: event_from_entry(entry, "raw_data"),
    )
    assert result.ok
    assert result.value == log_path
    lines = log_path.read_text(encoding="utf-8").splitlines()
    assert len(lines) == 2
    records = [json.loads(line) for line in lines]
    assert [r["payload"]["direction"] for r in records] == ["tx", "rx"]


def test_export_log_result_empty_entries_creates_empty_file(tmp_path):
    log_path = tmp_path / "empty.jsonl"
    result = export_log_result(log_path, [], lambda entry: event_from_entry(entry, "raw_data"))
    assert result.ok
    assert log_path.read_text(encoding="utf-8") == ""


def test_export_log_result_reports_failure_on_unwritable_path(tmp_path):
    """父路径是文件而非目录 → OSError → OperationResult.failure。"""
    blocked_parent = tmp_path / "blocked"
    blocked_parent.write_text("not a directory", encoding="utf-8")
    result = export_log_result(
        blocked_parent / "session.jsonl",
        [SerialWorkbenchLogEntry("tx", "x", b"x")],
        lambda entry: event_from_entry(entry, "raw_data"),
    )
    assert result.failed
    assert result.error_code == "log_export_failed"
    assert "blocked" in result.message


# ----------------------- replay_entries_result -----------------------


def test_replay_entries_result_applies_entry_factory(tmp_path):
    log_path = tmp_path / "session.jsonl"
    log_path.write_text(
        "\n".join(
            json.dumps(record)
            for record in [
                {
                    "type": "frame",
                    "protocolName": "raw_data",
                    "payload": {"text": "ping", "direction": "tx"},
                    "rawHex": b"ping".hex(),
                },
                {
                    "type": "frame",
                    "protocolName": "raw_data",
                    "payload": {"text": "pong", "direction": "rx"},
                    "rawHex": b"pong".hex(),
                },
            ]
        ),
        encoding="utf-8",
    )
    result = replay_entries_result(log_path, entry_from_event)
    assert result.ok
    entries = result.value
    assert [(e.direction, e.text) for e in entries] == [("tx", "ping"), ("rx", "pong")]


def test_replay_entries_result_skips_blank_lines(tmp_path):
    log_path = tmp_path / "with_blanks.jsonl"
    record = {
        "type": "frame",
        "protocolName": "raw_data",
        "payload": {"text": "ok", "direction": "rx"},
        "rawHex": b"ok".hex(),
    }
    log_path.write_text(
        "\n".join([json.dumps(record), "", "   ", json.dumps(record)]),
        encoding="utf-8",
    )
    result = replay_entries_result(log_path, entry_from_event)
    assert result.ok
    assert len(result.value) == 2  # 空白行被跳过


def test_replay_entries_result_propagates_load_failure(tmp_path):
    """文件不存在 → load_events_result 失败 → 传播为 OperationResult.failure。"""
    result = replay_entries_result(tmp_path / "missing.jsonl", entry_from_event)
    assert result.failed
    assert result.error_code == "replay_load_failed"


def test_replay_entries_result_malformed_json_reports_failure(tmp_path):
    """单行 JSON 损坏 → failure 而非异常逃逸。"""
    log_path = tmp_path / "broken.jsonl"
    log_path.write_text("{not valid json", encoding="utf-8")
    result = replay_entries_result(log_path, entry_from_event)
    assert result.failed
    assert result.error_code == "replay_load_failed"


# ----------------------- save_profile_result / load_profile_result -----------------------


def test_save_and_load_profile_result_round_trip(tmp_path):
    profile_path = tmp_path / "profile.json"
    profile = {
        "name": "bench-uart",
        "transport": {"mode": "serial", "portName": "COM3", "baudRate": 115200},
        "protocol": "fire_water",
        "commandHistory": ["status?", "reset"],
    }
    save_result = save_profile_result(profile_path, profile)
    assert save_result.ok
    assert save_result.value == profile_path

    load_result = load_profile_result(profile_path)
    assert load_result.ok
    assert load_result.value == profile


def test_save_profile_result_creates_parent_dirs(tmp_path):
    profile_path = tmp_path / "nested" / "deep" / "profile.json"
    result = save_profile_result(profile_path, {"name": "x"})
    assert result.ok
    assert profile_path.exists()


def test_save_profile_result_reports_os_failure(tmp_path):
    blocked_parent = tmp_path / "blocked"
    blocked_parent.write_text("not a directory", encoding="utf-8")
    result = save_profile_result(blocked_parent / "profile.json", {"name": "x"})
    assert result.failed
    assert result.error_code == "profile_save_failed"


def test_load_profile_result_reports_missing_file(tmp_path):
    result = load_profile_result(tmp_path / "missing.json")
    assert result.failed
    assert result.error_code == "profile_load_failed"


def test_load_profile_result_reports_malformed_json(tmp_path):
    profile_path = tmp_path / "broken.json"
    profile_path.write_text("{not valid json", encoding="utf-8")
    result = load_profile_result(profile_path)
    assert result.failed
    assert result.error_code == "profile_load_failed"
