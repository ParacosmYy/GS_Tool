from __future__ import annotations

from embeddebug.serial_station.ui.endpoint_validation import validate_endpoint_fields


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
