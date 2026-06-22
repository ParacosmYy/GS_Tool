"""UI 文本/校验/日志相关测试合并：endpoint/profile/command/log 文本 + 校验 + 过滤 + codec。"""

from __future__ import annotations

from dataclasses import dataclass

from embeddebug.serial_station.controllers.log_entry import SerialWorkbenchLogEntry
from embeddebug.serial_station.controllers.log_entry_codec import entry_from_event, event_from_entry
from embeddebug.serial_station.protocols import ProtocolEvent
from embeddebug.serial_station.ui.command_entry_text import apply_command_history_selection
from embeddebug.serial_station.ui.endpoint_control_text import endpoint_control_text
from embeddebug.serial_station.ui.endpoint_default_text import apply_default_endpoint_text
from embeddebug.serial_station.ui.endpoint_validation import validate_endpoint_fields
from embeddebug.serial_station.ui.log_entry_filter import log_entry_matches_filter
from embeddebug.serial_station.ui.log_filter_options import (
    default_log_filter_text,
    log_filter_options,
)
from embeddebug.serial_station.ui.log_view_content import clear_log_view
from embeddebug.serial_station.ui.profile_name_text import apply_profile_name_text


class _TextEdit:
    def __init__(self) -> None:
        self.value = ""

    def setText(self, text: str) -> None:
        self.value = text


class _LineEdit:
    def __init__(self, initial: str = "") -> None:
        self.text_value = initial

    def setText(self, text: str) -> None:
        self.text_value = text


class _LogView:
    def __init__(self) -> None:
        self.lines = ["TX AT", "RX OK"]
        self.clear_count = 0

    def clear(self) -> None:
        self.clear_count += 1
        self.lines.clear()


@dataclass(frozen=True)
class _FilterEntry:
    direction: str
    text: str


_FILTER_KW = {"tx_text": "TX", "rx_text": "RX", "system_text": "System", "error_text": "Error"}


# ── endpoint default text ──
def test_apply_default_endpoint_text_sets_shared_host_and_port_defaults():
    host_edit = _TextEdit()
    port_edit = _TextEdit()
    apply_default_endpoint_text(host_edit, port_edit)
    assert host_edit.value == "127.0.0.1"
    assert port_edit.value == "19000"


# ── endpoint control text ──
def test_endpoint_control_text_describes_tcp_controls():
    text = endpoint_control_text("tcp")
    assert text.host_placeholder == "TCP host"
    assert text.host_tooltip == "TCP host name or address"
    assert text.port_placeholder == "TCP port"
    assert text.port_tooltip == "TCP port number"
    assert text.connect_label == "Connect TCP"
    assert text.connect_tooltip == "Open a TCP client connection"


def test_endpoint_control_text_describes_udp_controls():
    text = endpoint_control_text("udp")
    assert text.host_placeholder == "UDP host"
    assert text.host_tooltip == "UDP remote host name or address"
    assert text.port_placeholder == "UDP port"
    assert text.port_tooltip == "UDP remote port number"
    assert text.connect_label == "Connect UDP"
    assert text.connect_tooltip == "Open a UDP datagram connection"


# ── endpoint validation ──
def test_validate_endpoint_fields_accepts_host_and_port():
    result = validate_endpoint_fields(" 127.0.0.1 ", " 19000 ", "UDP")
    assert result.ok
    assert result.host == "127.0.0.1"
    assert result.port == 19000
    assert result.message == ""


def test_validate_endpoint_fields_rejects_missing_host():
    result = validate_endpoint_fields("", "19000", "TCP")
    assert not result.ok
    assert result.host == ""
    assert result.port == 0
    assert result.message == "TCP host is empty"


def test_validate_endpoint_fields_rejects_invalid_port():
    for port_text in ["abc", "0", "65536"]:
        result = validate_endpoint_fields("127.0.0.1", port_text, "UDP")
        assert not result.ok
        assert result.message == "UDP port is invalid"


# ── profile name text ──
def test_apply_profile_name_text_writes_loaded_profile_name():
    edit = _LineEdit("old")
    apply_profile_name_text(edit, "factory-a")
    assert edit.text_value == "factory-a"


def test_apply_profile_name_text_allows_empty_name_to_clear_stale_text():
    edit = _LineEdit("old")
    apply_profile_name_text(edit, "")
    assert edit.text_value == ""


# ── command entry text ──
def test_apply_command_history_selection_writes_non_empty_text():
    edit = _LineEdit()
    apply_command_history_selection(edit, "ATI")
    assert edit.text_value == "ATI"


def test_apply_command_history_selection_ignores_empty_text():
    edit = _LineEdit("AT")
    apply_command_history_selection(edit, "")
    assert edit.text_value == "AT"


# ── log view content ──
def test_clear_log_view_clears_existing_lines():
    view = _LogView()
    clear_log_view(view)
    assert view.lines == []
    assert view.clear_count == 1


# ── log filter options ──
def test_log_filter_options_return_default_visible_filters():
    assert log_filter_options() == ("All", "TX", "RX", "System", "Error")


def test_default_log_filter_text_matches_all_filter():
    assert default_log_filter_text() == "All"


# ── log entry codec ──
def test_entry_from_event_restores_known_payload_direction():
    event = ProtocolEvent(
        type="frame", protocol_name="raw_data",
        payload={"text": "port denied", "direction": "error"}, raw=b"port denied",
    )
    entry = entry_from_event(event)
    assert entry.direction == "error"
    assert entry.text == "port denied"


def test_event_from_entry_preserves_diagnostic_direction_in_payload():
    event = event_from_entry(
        SerialWorkbenchLogEntry(direction="system", text="profile loaded", raw=b"profile loaded"),
        "raw_data",
    )
    assert event.type == "frame"
    assert event.payload["direction"] == "system"


# ── log entry filter ──
def test_log_entry_matches_filter_accepts_all_direction_without_search():
    assert log_entry_matches_filter(_FilterEntry("rx", "OK"), selected_filter="All", search_text="", **_FILTER_KW)


def test_log_entry_matches_filter_rejects_non_matching_direction():
    assert not log_entry_matches_filter(_FilterEntry("rx", "OK"), selected_filter="TX", search_text="", **_FILTER_KW)


def test_log_entry_matches_filter_searches_direction_prefix_and_text():
    assert log_entry_matches_filter(_FilterEntry("tx", "AT+RST"), selected_filter="All", search_text="tx at", **_FILTER_KW)
    assert not log_entry_matches_filter(_FilterEntry("rx", "OK"), selected_filter="All", search_text="tx at", **_FILTER_KW)


def test_log_entry_matches_filter_accepts_system_direction():
    assert log_entry_matches_filter(_FilterEntry("system", "profile loaded"), selected_filter="System", search_text="system profile", **_FILTER_KW)
    assert not log_entry_matches_filter(_FilterEntry("error", "port denied"), selected_filter="System", search_text="", **_FILTER_KW)


def test_log_entry_matches_filter_accepts_error_direction():
    assert log_entry_matches_filter(_FilterEntry("error", "port denied"), selected_filter="Error", search_text="error port", **_FILTER_KW)
    assert not log_entry_matches_filter(_FilterEntry("system", "profile loaded"), selected_filter="Error", search_text="", **_FILTER_KW)
