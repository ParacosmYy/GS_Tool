from __future__ import annotations

from embeddebug.serial_station.ui.status_messages import (
    append_log_entry_line,
    result_message,
    set_log_stats_label,
    set_profile_label,
    set_result_status,
    set_status_text,
    translated_result_message,
)
from embeddebug.shared.results import OperationResult


class Label:
    def __init__(self) -> None:
        self.text = ""

    def setText(self, text: str) -> None:
        self.text = text


class PlainTextView:
    def __init__(self) -> None:
        self.lines: list[str] = []

    def appendPlainText(self, text: str) -> None:
        self.lines.append(text)


class TrHost:
    def __init__(self) -> None:
        self._log_view = PlainTextView()
        self._log_stats_label = Label()
        self._profile_label = Label()
        self._status_label = Label()

    def tr(self, text: str) -> str:
        return f"tr:{text}"


def test_result_message_returns_success_text_for_ok_result():
    result = OperationResult.success()

    assert (
        result_message(result, success_text="Command sent", failure_prefix="Send failed")
        == "Command sent"
    )


def test_result_message_formats_failure_with_result_message():
    result = OperationResult.failure("transport.closed", "Open a connection first")

    assert (
        result_message(result, success_text="Command sent", failure_prefix="Send failed")
        == "Send failed: Open a connection first"
    )


def test_translated_result_message_applies_translation_then_format_values():
    result = OperationResult.success()

    assert (
        translated_result_message(
            TrHost(),
            result,
            success_text="Connected to {endpoint}",
            failure_prefix="Connection failed",
            endpoint="127.0.0.1:19000",
        )
        == "tr:Connected to 127.0.0.1:19000"
    )


def test_set_result_status_writes_translated_message_to_status_label():
    host = TrHost()
    result = OperationResult.success()

    set_result_status(
        host,
        result,
        success_text="Connected to {endpoint}",
        failure_prefix="Connection failed",
        endpoint="127.0.0.1:19000",
    )

    assert host._status_label.text == "tr:Connected to 127.0.0.1:19000"


def test_set_status_text_writes_translated_and_formatted_text_to_status_label():
    host = TrHost()

    set_status_text(host, "Protocol: {name}", name="RawData")

    assert host._status_label.text == "tr:Protocol: RawData"


def test_set_profile_label_writes_translated_profile_name_to_profile_label():
    host = TrHost()

    set_profile_label(host, "factory-default")

    assert host._profile_label.text == "tr:Profile: factory-default"


def test_set_log_stats_label_writes_translated_counts_to_log_stats_label():
    host = TrHost()

    set_log_stats_label(host, visible=2, total=3, tx=1, rx=2)

    assert host._log_stats_label.text == "tr:Visible 2 / Total 3 | TX 1 | RX 2"


def test_append_log_entry_line_writes_translated_direction_and_text_to_log_view():
    host = TrHost()

    append_log_entry_line(host, direction="rx", text="0A 0B")

    assert host._log_view.lines == ["tr:RX 0A 0B"]
