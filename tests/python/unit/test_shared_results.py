from __future__ import annotations

import pytest

from embeddebug.shared import OperationError, OperationResult


def test_operation_result_success_exposes_value_without_error():
    result = OperationResult.success("connected")

    assert result.ok is True
    assert result.failed is False
    assert result.value == "connected"
    assert result.error is None
    assert result.error_code == ""
    assert result.message == ""


def test_operation_result_failure_requires_code_and_exposes_message():
    result = OperationResult.failure("transport_not_open", "Open a connection first")

    assert result.ok is False
    assert result.failed is True
    assert result.value is None
    assert result.error == OperationError(
        code="transport_not_open",
        message="Open a connection first",
    )
    assert result.error_code == "transport_not_open"
    assert result.message == "Open a connection first"


def test_operation_result_rejects_invalid_state():
    with pytest.raises(ValueError, match="success result cannot contain an error"):
        OperationResult(ok=True, error=OperationError("unexpected"))

    with pytest.raises(ValueError, match="failure result requires an error code"):
        OperationResult.failure("", "missing code")


def test_operation_result_success_without_value_defaults_none():
    result = OperationResult.success()

    assert result.ok is True
    assert result.failed is False
    assert result.value is None
    assert result.error_code == ""
    assert result.message == ""


def test_operation_result_failure_without_message_defaults_empty():
    result = OperationResult.failure("timeout")

    assert result.failed is True
    assert result.error_code == "timeout"
    assert result.message == ""


def test_operation_result_rejects_failure_without_error():
    with pytest.raises(ValueError, match="failure result requires an error code"):
        OperationResult(ok=False, error=None)


def test_operation_result_is_frozen():
    result = OperationResult.success()

    with pytest.raises((AttributeError, TypeError)):
        result.ok = False


def test_operation_result_runtime_generic_value():
    result: OperationResult[str] = OperationResult.success("hello")
    failed: OperationResult[int] = OperationResult.failure("err")

    assert result.value == "hello"
    assert failed.value is None


def test_operation_error_rejects_empty_code():
    with pytest.raises(ValueError, match="error code"):
        OperationError(code="")


def test_operation_error_defaults_and_custom_message():
    default = OperationError(code="timeout")
    custom = OperationError(code="io", message="File not found")

    assert default.message == ""
    assert custom.code == "io"
    assert custom.message == "File not found"


def test_operation_error_is_frozen():
    error = OperationError(code="x")

    with pytest.raises(AttributeError):
        error.code = "y"
