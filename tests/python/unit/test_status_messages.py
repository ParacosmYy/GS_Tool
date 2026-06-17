from __future__ import annotations

from embeddebug.serial_station.ui.status_messages import result_message, translated_result_message
from embeddebug.shared.results import OperationResult


class TrHost:
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
