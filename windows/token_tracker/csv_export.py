"""Shared bounded CSV serialization for personal and administrator projections.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Enforce one row/byte budget without loading an unbounded result set.
Module: Infrastructure / export serialization boundary
"""

from __future__ import annotations

import csv
import io
from collections.abc import Iterable, Sequence
from typing import Any


MAX_EXPORT_ROWS = 100_000
MAX_EXPORT_BYTES = 16 * 1024 * 1024


class ExportTooLargeError(RuntimeError):
    """Raised when a CSV result crosses the shared output safety boundary."""


def _check_export_size(byte_buffer: io.BytesIO, row_count: int) -> None:
    """Fail before returning a partial export, keeping response memory predictable."""

    if row_count > MAX_EXPORT_ROWS:
        raise ExportTooLargeError(f"export exceeds {MAX_EXPORT_ROWS} rows")
    if byte_buffer.tell() > MAX_EXPORT_BYTES:
        raise ExportTooLargeError(f"export exceeds {MAX_EXPORT_BYTES} bytes")


def export_rows(columns: Sequence[str], rows: Iterable[Any]) -> bytes:
    """Serialize a fixed projection while consuming the row iterator exactly once."""

    byte_buffer = io.BytesIO()
    text_stream = io.TextIOWrapper(byte_buffer, encoding="utf-8-sig", newline="")
    writer = csv.writer(text_stream)
    row_count = 0
    try:
        writer.writerow(columns)
        text_stream.flush()
        _check_export_size(byte_buffer, row_count)
        for row in rows:
            if row_count >= MAX_EXPORT_ROWS:
                raise ExportTooLargeError(f"export exceeds {MAX_EXPORT_ROWS} rows")
            writer.writerow([row[column] for column in columns])
            row_count += 1
            text_stream.flush()
            _check_export_size(byte_buffer, row_count)
        return byte_buffer.getvalue()
    finally:
        text_stream.detach()
