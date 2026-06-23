"""OperationResult 泛型边界测试。

test_shared_results 覆盖基础 success/failure；
本文件补泛型类型参数 + 默认 value + error_code/message 属性 + failed 属性。

覆盖：
1. OperationResult[int] 泛型 value。
2. OperationResult success 默认 value=None。
3. OperationResult success 带 value。
4. failed 属性 = not ok。
5. error_code 成功时空串。
6. error_code 失败时返回 code。
7. message 成功时空串。
8. message 失败时返回 error.message。
"""

from __future__ import annotations

from embeddebug.shared.results import OperationResult


def test_generic_int_value():
    result: OperationResult[int] = OperationResult.success(42)
    assert result.value == 42
    assert result.ok is True


def test_success_default_value_none():
    result = OperationResult.success()
    assert result.value is None


def test_success_with_value():
    result = OperationResult.success("connected")
    assert result.value == "connected"


def test_failed_property():
    ok = OperationResult.success()
    fail = OperationResult.failure("err", "msg")
    assert ok.failed is False
    assert fail.failed is True


def test_error_code_success_empty():
    result = OperationResult.success()
    assert result.error_code == ""


def test_error_code_failure_returns_code():
    result = OperationResult.failure("timeout", "timed out")
    assert result.error_code == "timeout"


def test_message_success_empty():
    result = OperationResult.success()
    assert result.message == ""


def test_message_failure_returns_message():
    result = OperationResult.failure("io_error", "File not found")
    assert result.message == "File not found"
