"""Stable failure categories shared by application use cases."""

from __future__ import annotations


class ApplicationError(Exception):
    """Base class for failures owned by application-layer policy."""


class ApplicationValidationError(ApplicationError, ValueError):
    """A caller input or provider result violates an application invariant."""


class ApplicationTypeError(ApplicationError, TypeError):
    """A caller input or provider result has an invalid application type."""


class ApplicationStateError(ApplicationError, RuntimeError):
    """A use case cannot proceed from the current application state."""


__all__ = [
    "ApplicationError",
    "ApplicationStateError",
    "ApplicationTypeError",
    "ApplicationValidationError",
]
