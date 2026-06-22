"""SessionState 会话状态数据结构边界测试。

覆盖 to_dict/from_dict 的防御性契约（缺失字段/None/类型不符回退）。

1. 默认工厂（_default_geometry / _default_connection_config 结构）。
2. to_dict 全键 + 拷贝隔离（修改原不影响返回）。
3. from_dict None → 默认状态。
4. from_dict 缺失字段 → 默认值。
5. from_dict None 字段值 → 默认（_get 守卫）。
6. from_dict 类型不符回退（geometry 非 dict / history 非 list / history 项 str 强转）。
7. to_dict → from_dict 往返等价。
8. timestamp_ns int 强转（float → int）。
"""

from __future__ import annotations

from embeddebug.serial_station.session.state import (
    SessionState,
    _default_connection_config,
    _default_geometry,
)


# ── 默认工厂 ──────────────────────────────────────────────────────
def test_default_geometry_structure():
    g = _default_geometry()
    assert g == {"x": 0, "y": 0, "width": 1280, "height": 800}


def test_default_connection_config_structure():
    c = _default_connection_config()
    assert c == {"port": "", "baudrate": 115200}


def test_default_factories_return_fresh_instances():
    """工厂每次返回新 dict（避免共享可变默认）。"""

    g1 = _default_geometry()
    g1["x"] = 999
    g2 = _default_geometry()
    assert g2["x"] == 0  # 不受 g1 修改影响


# ── SessionState 默认值 ──────────────────────────────────────────
def test_session_state_defaults():
    s = SessionState()
    assert s.transport_mode == "uart"
    assert s.protocol == "raw_data"
    assert s.command_history == []
    assert s.log_filter == ""
    assert s.active_tab == ""
    assert s.timestamp_ns == 0
    assert s.window_geometry == _default_geometry()
    assert s.connection_config == _default_connection_config()


# ── to_dict ───────────────────────────────────────────────────────
def test_to_dict_has_all_keys():
    d = SessionState().to_dict()
    for key in (
        "window_geometry",
        "transport_mode",
        "protocol",
        "connection_config",
        "command_history",
        "log_filter",
        "active_tab",
        "timestamp_ns",
    ):
        assert key in d


def test_to_dict_copies_collections():
    """to_dict 返回的 list/dict 应是拷贝（修改不影响原状态）。"""

    s = SessionState(command_history=["a"])
    d = s.to_dict()
    d["command_history"].append("b")
    assert s.command_history == ["a"]  # 原未变

    d["window_geometry"]["x"] = 999
    assert s.window_geometry["x"] == 0


# ── from_dict None / 缺失 ────────────────────────────────────────
def test_from_dict_none_returns_default():
    s = SessionState.from_dict(None)
    assert s.transport_mode == "uart"
    assert s.command_history == []


def test_from_dict_empty_dict_returns_defaults():
    s = SessionState.from_dict({})
    assert s.transport_mode == "uart"
    assert s.protocol == "raw_data"
    assert s.command_history == []
    assert s.window_geometry == _default_geometry()


def test_from_dict_partial_keys():
    """只提供部分字段，其余用默认。"""

    s = SessionState.from_dict({"transport_mode": "tcp", "log_filter": "ERR"})
    assert s.transport_mode == "tcp"
    assert s.log_filter == "ERR"
    assert s.protocol == "raw_data"  # 默认
    assert s.command_history == []  # 默认


# ── from_dict None 字段值 ────────────────────────────────────────
def test_from_dict_none_values_fall_back_to_default():
    """字段值为 None 时应回退默认（_get 守卫）。"""

    s = SessionState.from_dict({
        "transport_mode": None,
        "protocol": None,
        "log_filter": None,
        "active_tab": None,
        "timestamp_ns": None,
    })
    assert s.transport_mode == "uart"
    assert s.protocol == "raw_data"
    assert s.log_filter == ""
    assert s.timestamp_ns == 0


# ── from_dict 类型不符回退 ───────────────────────────────────────
def test_from_dict_geometry_not_dict_falls_back():
    s = SessionState.from_dict({"window_geometry": "not a dict"})
    assert s.window_geometry == _default_geometry()


def test_from_dict_connection_not_dict_falls_back():
    s = SessionState.from_dict({"connection_config": [1, 2, 3]})
    assert s.connection_config == _default_connection_config()


def test_from_dict_history_not_list_falls_back():
    s = SessionState.from_dict({"command_history": "not a list"})
    assert s.command_history == []


def test_from_dict_history_items_stringified():
    """history 项非 str 时强转为 str。"""

    s = SessionState.from_dict({"command_history": [1, 2.5, True]})
    assert s.command_history == ["1", "2.5", "True"]


def test_from_dict_timestamp_float_to_int():
    """timestamp_ns float 强转为 int。"""

    s = SessionState.from_dict({"timestamp_ns": 123.7})
    assert s.timestamp_ns == 123
    assert isinstance(s.timestamp_ns, int)


def test_from_dict_transport_mode_stringified():
    """transport_mode 非 str 强转。"""

    s = SessionState.from_dict({"transport_mode": 42})
    assert s.transport_mode == "42"


# ── to_dict → from_dict 往返 ─────────────────────────────────────
def test_roundtrip_to_dict_from_dict():
    s = SessionState(
        transport_mode="tcp",
        protocol="modbus_rtu",
        command_history=["a", "b"],
        log_filter="WARN",
        active_tab="serial",
        timestamp_ns=999,
        window_geometry={"x": 10, "y": 20, "width": 800, "height": 600},
        connection_config={"port": "COM3", "baudrate": 9600},
    )
    restored = SessionState.from_dict(s.to_dict())
    assert restored.transport_mode == s.transport_mode
    assert restored.protocol == s.protocol
    assert restored.command_history == s.command_history
    assert restored.log_filter == s.log_filter
    assert restored.active_tab == s.active_tab
    assert restored.timestamp_ns == s.timestamp_ns
    assert restored.window_geometry == s.window_geometry
    assert restored.connection_config == s.connection_config
