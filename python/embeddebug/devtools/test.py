"""Test runner for the Python/PyQt migration lane."""

from __future__ import annotations

import sys
from collections.abc import Sequence


def main(argv: Sequence[str] | None = None) -> int:
    """Run Python migration tests through pytest."""

    try:
        import pytest
    except ModuleNotFoundError:
        print(
            "pytest is required. Run through uv so the dev dependency group is active.",
            file=sys.stderr,
        )
        return 2

    args = list(argv) if argv is not None else []
    if not args:
        args = ["tests/python"]
    return int(pytest.main(args))


if __name__ == "__main__":
    raise SystemExit(main())
