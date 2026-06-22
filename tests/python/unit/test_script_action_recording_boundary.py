"""ScriptAction + ScriptRecording 防御性边界扩展测试。

test_script_recorder_helpers.py 覆盖常量/to_dict/from_dict round-trip/
duration/add/to_json-from_json 文件往返；本文件补 from_dict 缺字段/类型强转 +
to_json indent + from_json 原始字符串 + frozen + 默认值。

覆盖：
1. ScriptAction.from_dict 缺 type/payload/timestamp_ms raises KeyError。
2. ScriptAction.from_dict 类型强转（int timestamp / str payload）。
3. ScriptAction frozen 不可变。
4. ScriptAction.from_dict label 显式 None → ''。
5. ScriptRecording 默认字段值（空 actions/name/created_at/description）。
6. ScriptRecording.to_json indent=2 格式（含换行缩进）。
7. ScriptRecording.to_json 返回字符串（path=None）。
8. ScriptRecording.from_json 接受原始 JSON 字符串（非文件路径）。
9. ScriptRecording.from_json 缺 name/created_at/description 默认空。
10. ScriptRecording.duration_ms 单动作 / 负 timestamp。
"""

from __future__ import annotations

import json

import pytest

from embeddebug.serial_station.script_recorder.action import ScriptAction
from embeddebug.serial_station.script_recorder.recording import ScriptRecording


# ── ScriptAction.from_dict 缺字段 ────────────────────────────────
def test_from_dict_missing_type_raises_keyerror():
    with pytest.raises(KeyError):
        ScriptAction.from_dict({"payload": "x", "timestamp_ms": 0})


def test_from_dict_missing_payload_raises_keyerror():
    with pytest.raises(KeyError):
        ScriptAction.from_dict({"type": "SEND", "timestamp_ms": 0})


def test_from_dict_missing_timestamp_raises_keyerror():
    with pytest.raises(KeyError):
        ScriptAction.from_dict({"type": "SEND", "payload": "x"})


# ── ScriptAction.from_dict 类型强转 ──────────────────────────────
def test_from_dict_coerces_types():
    """from_dict 把 type/payload 强转 str，timestamp_ms 强转 int。"""

    a = ScriptAction.from_dict({"type": 123, "payload": 456, "timestamp_ms": "789"})
    assert a.type == "123"
    assert a.payload == "456"
    assert a.timestamp_ms == 789
    assert isinstance(a.timestamp_ms, int)


# ── ScriptAction frozen ───────────────────────────────────────────
def test_action_is_frozen():
    a = ScriptAction(type="SEND", payload="x", timestamp_ms=0)
    with pytest.raises(AttributeError):
        a.type = "RECEIVE"  # type: ignore[misc]


# ── ScriptAction.from_dict label None ────────────────────────────
def test_from_dict_label_none_becomes_empty():
    """label 显式 None → str(None)='None'？不，data.get('label','') 当 key 存在返回 None。"""

    # data.get("label", "") → None（key 存在但值为 None）→ str(None)
    a = ScriptAction.from_dict({"type": "SEND", "payload": "x", "timestamp_ms": 0, "label": None})
    assert a.label == "None"  # str(None)


# ── ScriptRecording 默认值 ───────────────────────────────────────
def test_recording_defaults():
    r = ScriptRecording()
    assert r.actions == []
    assert r.name == ""
    assert r.created_at == ""
    assert r.description == ""
    assert r.action_count == 0


# ── to_json indent + 返回字符串 ──────────────────────────────────
def test_to_json_returns_string_when_no_path():
    r = ScriptRecording(name="test", actions=[
        ScriptAction(type="SEND", payload="AT", timestamp_ms=0)
    ])
    text = r.to_json()
    assert isinstance(text, str)
    data = json.loads(text)
    assert data["name"] == "test"
    assert len(data["actions"]) == 1


def test_to_json_indent_format():
    """to_json 用 indent=2（多行格式）。"""

    r = ScriptRecording(name="t")
    text = r.to_json()
    assert "\n" in text  # indent 产生换行
    assert '  "' in text  # 缩进


def test_to_json_writes_file_returns_none(tmp_path):
    r = ScriptRecording(name="test")
    path = tmp_path / "rec.json"
    result = r.to_json(path)
    assert result is None
    assert path.exists()
    data = json.loads(path.read_text(encoding="utf-8"))
    assert data["name"] == "test"


# ── from_json 原始字符串 ──────────────────────────────────────────
def test_from_json_accepts_raw_json_string():
    """from_json 接受原始 JSON 字符串（非文件路径）。"""

    raw = json.dumps({"name": "raw", "actions": [], "created_at": "2024", "description": "d"})
    r = ScriptRecording.from_json(raw)
    assert r.name == "raw"
    assert r.created_at == "2024"
    assert r.description == "d"


def test_from_json_missing_name_defaults_empty():
    raw = json.dumps({"actions": []})
    r = ScriptRecording.from_json(raw)
    assert r.name == ""
    assert r.created_at == ""
    assert r.description == ""


def test_from_json_coerces_field_types():
    """from_json 把 name/created_at/description 强转 str。"""

    raw = json.dumps({"name": 123, "created_at": 456, "description": None, "actions": []})
    r = ScriptRecording.from_json(raw)
    assert r.name == "123"
    assert r.created_at == "456"


# ── duration_ms 边界 ──────────────────────────────────────────────
def test_duration_ms_single_action():
    r = ScriptRecording(actions=[
        ScriptAction(type="SEND", payload="x", timestamp_ms=500)
    ])
    assert r.duration_ms == 500


def test_duration_ms_negative_timestamp():
    """负 timestamp 也参与 max（duration 可能为负）。"""

    r = ScriptRecording(actions=[
        ScriptAction(type="SEND", payload="x", timestamp_ms=-100)
    ])
    assert r.duration_ms == -100


def test_duration_ms_mixed_returns_max():
    r = ScriptRecording(actions=[
        ScriptAction(type="SEND", payload="a", timestamp_ms=100),
        ScriptAction(type="DELAY", payload="b", timestamp_ms=500),
        ScriptAction(type="SEND", payload="c", timestamp_ms=300),
    ])
    assert r.duration_ms == 500


# ── action_count ──────────────────────────────────────────────────
def test_action_count_multiple():
    actions = [
        ScriptAction(type="SEND", payload=str(i), timestamp_ms=i * 100)
        for i in range(5)
    ]
    r = ScriptRecording(actions=actions)
    assert r.action_count == 5


# ── add 累积 ──────────────────────────────────────────────────────
def test_add_accumulates_and_updates_count():
    r = ScriptRecording()
    assert r.action_count == 0
    r.add(ScriptAction(type="SEND", payload="a", timestamp_ms=0))
    assert r.action_count == 1
    r.add(ScriptAction(type="SEND", payload="b", timestamp_ms=100))
    assert r.action_count == 2
    assert r.duration_ms == 100
