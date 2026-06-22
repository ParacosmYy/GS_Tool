"""OperationResult + OperationError 单元测试 — 成功/失败值对象。

覆盖：success/failure 工厂、ok/failed 属性、error_code/message、
__post_init__ 验证（成功带 error 抛异常、失败无 error 抛异常）。
"""

from __future__ import annotations

import pytest

from embeddebug.shared.results import OperationError, OperationResult


def test_success_factory():
    r = OperationResult.success(42)
    assert r.ok is True
    assert r.value == 42
    assert r.error is None


def test_success_no_value():
    r = OperationResult.success()
    assert r.ok is True
    assert r.value is None


def test_failure_factory():
    r = OperationResult.failure("timeout", "connection timed out")
    assert r.ok is False
    assert r.failed is True
    assert r.error_code == "timeout"
    assert r.message == "connection timed out"


def test_failure_no_message():
    r = OperationResult.failure("error")
    assert r.error_code == "error"
    assert r.message == ""


def test_failed_property():
    assert OperationResult.success().failed is False
    assert OperationResult.failure("x").failed is True


def test_error_code_empty_when_success():
    r = OperationResult.success()
    assert r.error_code == ""
    assert r.message == ""


def test_post_init_success_with_error_raises():
    with pytest.raises(ValueError):
        OperationResult(ok=True, error=OperationError(code="x"))


def test_post_init_failure_without_error_raises():
    with pytest.raises(ValueError):
        OperationResult(ok=False, error=None)


def test_operation_error_empty_code_raises():
    with pytest.raises(ValueError):
        OperationError(code="")


def test_operation_error_defaults():
    e = OperationError(code="timeout")
    assert e.message == ""


def test_result_frozen():
    r = OperationResult.success()
    with pytest.raises((AttributeError, TypeError)):
        r.ok = False


def test_result_generic_type():
    """泛型类型参数不影响运行时行为。"""
    r: OperationResult[str] = OperationResult.success("hello")
    assert r.value == "hello"
    r2: OperationResult[int] = OperationResult.failure("err")
    assert r2.value is None
