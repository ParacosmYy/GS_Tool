"""shared/results OperationError 边界测试。

OperationError 此前仅经 OperationResult 间接测试。
本文件覆盖 OperationError 构造 + 空 code ValueError + message 默认空。

覆盖：
1. OperationError code+message 构造。
2. OperationError 默认 message 空串。
3. OperationError 空 code raises ValueError。
4. OperationError code 属性。
5. OperationError message 属性。
6. OperationError 自定义 message。
7. OperationError frozen。
8. OperationError 多实例独立。
"""

from __future__ import annotations

import pytest

from embeddebug.shared.results import OperationError


def test_error_code_and_message():
    e = OperationError(code="err", message="boom")
    assert e.code == "err"
    assert e.message == "boom"


def test_error_default_message_empty():
    e = OperationError(code="err")
    assert e.message == ""


def test_error_empty_code_raises():
    with pytest.raises(ValueError, match="error code"):
        OperationError(code="")


def test_error_code_attribute():
    e = OperationError(code="timeout")
    assert e.code == "timeout"


def test_error_message_attribute():
    e = OperationError(code="x", message="detail")
    assert e.message == "detail"


def test_error_custom_message():
    e = OperationError(code="io", message="File not found")
    assert e.message == "File not found"


def test_error_is_frozen():
    e = OperationError(code="x")
    with pytest.raises(AttributeError):
        e.code = "y"  # type: ignore[misc]


def test_error_multiple_independent():
    e1 = OperationError(code="a", message="1")
    e2 = OperationError(code="b", message="2")
    assert e1.code == "a"
    assert e2.code == "b"
