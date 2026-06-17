"""Operation result value objects."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Generic, TypeVar


T = TypeVar("T")


@dataclass(frozen=True)
class OperationError:
    """Structured failure information passed across product layers."""

    code: str
    message: str = ""

    def __post_init__(self) -> None:
        if not self.code:
            raise ValueError("failure result requires an error code")


@dataclass(frozen=True)
class OperationResult(Generic[T]):
    """Success or failure result with an optional typed value."""

    ok: bool
    value: T | None = None
    error: OperationError | None = None

    def __post_init__(self) -> None:
        if self.ok and self.error is not None:
            raise ValueError("success result cannot contain an error")
        if not self.ok and self.error is None:
            raise ValueError("failure result requires an error code")

    @classmethod
    def success(cls, value: T | None = None) -> OperationResult[T]:
        return cls(ok=True, value=value)

    @classmethod
    def failure(cls, code: str, message: str = "") -> OperationResult[T]:
        return cls(ok=False, error=OperationError(code=code, message=message))

    @property
    def failed(self) -> bool:
        return not self.ok

    @property
    def error_code(self) -> str:
        return self.error.code if self.error else ""

    @property
    def message(self) -> str:
        return self.error.message if self.error else ""
