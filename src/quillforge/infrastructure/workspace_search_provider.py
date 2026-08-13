"""Bounded local filesystem adapter for workspace content search."""

from __future__ import annotations

import io
import os
import stat
from collections.abc import Callable
from dataclasses import dataclass
from pathlib import Path
from typing import BinaryIO

from ..application.workspace_search import (
    WorkspaceSearchIssue,
    WorkspaceSearchLimitReason,
    WorkspaceSearchMatch,
    WorkspaceSearchProvider,
    WorkspaceSearchRequest,
    WorkspaceSearchResult,
)

_BINARY_PROBE_BYTES = 4_096
_UTF8_BOM = b"\xef\xbb\xbf"
_UTF16_LE_BOM = b"\xff\xfe"
_UTF16_BE_BOM = b"\xfe\xff"
_REPARSE_POINT = getattr(stat, "FILE_ATTRIBUTE_REPARSE_POINT", 0x400)


class FileWorkspaceSearchProvider(WorkspaceSearchProvider):
    """Search regular, non-symlink files under one explicit root."""

    def is_directory(self, path: Path) -> bool:
        """Keep search-root filesystem classification inside the adapter."""
        return path.is_dir()

    def search(
        self,
        request: WorkspaceSearchRequest,
        cancel_requested: Callable[[], bool],
    ) -> WorkspaceSearchResult:
        root = request.query.root.expanduser().resolve()
        if not self.is_directory(root):
            raise ValueError(f"Workspace search root does not exist: {root}")
        accumulator = _SearchAccumulator(request, cancel_requested)
        self._visit_directory(root, depth=0, accumulator=accumulator)
        return accumulator.result()

    def _visit_directory(
        self,
        directory: Path,
        *,
        depth: int,
        accumulator: _SearchAccumulator,
    ) -> None:
        if accumulator.should_stop():
            return
        try:
            entries = []
            iterator = directory.iterdir()
            for _ in range(accumulator.policy.max_directory_entries + 1):
                if accumulator.should_stop():
                    return
                try:
                    entries.append(next(iterator))
                except StopIteration:
                    break
        except OSError as error:
            accumulator.issue(directory, f"directory unavailable: {error}")
            return
        if len(entries) > accumulator.policy.max_directory_entries:
            accumulator.mark_limit("max-directory-entries")
            entries = entries[: accumulator.policy.max_directory_entries]
        entries.sort(key=lambda item: (item.name.casefold(), item.name))

        for entry in entries:
            if accumulator.should_stop():
                return
            try:
                if entry.is_symlink() or _is_reparse_point(entry):
                    accumulator.skip_file(entry, "symbolic link or reparse point skipped")
                    continue
                if entry.is_dir():
                    if entry.name.casefold() in accumulator.excluded_directories:
                        accumulator.skip_directory(entry, "excluded directory")
                        continue
                    if depth >= accumulator.policy.max_depth:
                        accumulator.mark_limit("max-depth")
                        accumulator.skip_directory(entry, "maximum search depth reached")
                        continue
                    self._visit_directory(
                        entry,
                        depth=depth + 1,
                        accumulator=accumulator,
                    )
                    continue
                if entry.is_file():
                    self._search_file(entry, accumulator)
                    continue
                accumulator.issue(entry, "unsupported filesystem entry")
            except OSError as error:
                accumulator.issue(entry, f"entry unavailable: {error}")

    def _search_file(self, path: Path, accumulator: _SearchAccumulator) -> None:
        if accumulator.files_scanned >= accumulator.policy.max_files:
            accumulator.mark_limit("max-files")
            return
        try:
            file_stat = path.stat(follow_symlinks=False)
        except OSError as error:
            accumulator.issue(path, f"file unavailable: {error}")
            return
        if not stat.S_ISREG(file_stat.st_mode):
            accumulator.issue(path, "regular files only")
            return

        accumulator.files_scanned += 1
        size = file_stat.st_size
        if size > accumulator.policy.max_file_bytes:
            accumulator.files_skipped += 1
            accumulator.mark_limit("max-file-bytes")
            accumulator.issue(path, "file exceeds the configured per-file byte limit")
            return
        if accumulator.bytes_reserved + size > accumulator.policy.max_total_bytes:
            accumulator.files_skipped += 1
            accumulator.mark_limit("max-total-bytes")
            return
        previous_reserved = accumulator.bytes_reserved
        accumulator.bytes_reserved += size
        total_budget = accumulator.policy.max_total_bytes - previous_reserved
        read_limit = min(accumulator.policy.max_file_bytes, total_budget)
        read_limit_reason: WorkspaceSearchLimitReason = (
            "max-file-bytes"
            if accumulator.policy.max_file_bytes <= total_budget
            else "max-total-bytes"
        )
        try:
            bytes_read, limit_reached = self._scan_file(
                path,
                accumulator,
                max_bytes=read_limit,
            )
        except (OSError, UnicodeError) as error:
            accumulator.issue(path, f"file read failed: {error}")
        else:
            actual_bytes = min(read_limit, bytes_read)
            if actual_bytes > size:
                accumulator.bytes_reserved += actual_bytes - size
            accumulator.bytes_scanned += actual_bytes
            if limit_reached:
                accumulator.mark_limit(read_limit_reason)

    def _scan_file(
        self,
        path: Path,
        accumulator: _SearchAccumulator,
        *,
        max_bytes: int,
    ) -> tuple[int, bool]:
        with path.open("rb") as stream:
            prefix = stream.read(min(_BINARY_PROBE_BYTES, max_bytes))
            encoding = _encoding_for(prefix)
            if encoding is None and b"\x00" in prefix:
                accumulator.files_skipped += 1
                accumulator.issue(path, "binary file skipped")
                return len(prefix), False
            if encoding is None:
                try:
                    prefix.decode("utf-8")
                except UnicodeDecodeError:
                    accumulator.issue(path, "invalid UTF-8 decoded with replacement")
            stream.seek(0)
            bounded_raw = _BoundedRawReader(stream, max_bytes)
            buffered = io.BufferedReader(bounded_raw)
            decoder = io.TextIOWrapper(
                buffered,
                encoding=encoding or "utf-8",
                errors="replace",
                newline="",
            )
            try:
                bytes_read = self._scan_text_stream(path, decoder, accumulator)
                return bytes_read, bounded_raw.limit_reached
            finally:
                decoder.detach()
                buffered.close()

    def _scan_text_stream(
        self,
        path: Path,
        stream: io.TextIOBase,
        accumulator: _SearchAccumulator,
    ) -> int:
        line_number = 0
        while True:
            if accumulator.cancel_requested():
                accumulator.cancelled = True
                return _stream_position(stream.buffer)
            text_line = stream.readline(accumulator.policy.max_line_bytes + 1)
            if not text_line:
                return _stream_position(stream.buffer)
            line_number += 1
            encoded_length = len(text_line.encode(stream.encoding or "utf-8", errors="replace"))
            if encoded_length > accumulator.policy.max_line_bytes:
                accumulator.mark_limit("max-line-bytes")
                while text_line and not _line_terminated(text_line):
                    if accumulator.cancel_requested():
                        accumulator.cancelled = True
                        return _stream_position(stream.buffer)
                    text_line = stream.readline(accumulator.policy.max_line_bytes + 1)
                continue
            text_line = text_line.rstrip("\r\n")
            if not self._record_line_matches(path, line_number, text_line, accumulator):
                return _stream_position(stream.buffer)

    def _record_line_matches(
        self,
        path: Path,
        line_number: int,
        text_line: str,
        accumulator: _SearchAccumulator,
    ) -> bool:
        needle = accumulator.query.needle
        if accumulator.query.case_sensitive:
            haystack = text_line
            original_indices = None
            normalized_needle = needle
        else:
            haystack, original_indices = _casefold_with_indices(text_line)
            normalized_needle = needle.casefold()
        cursor = 0
        while True:
            match_index = haystack.find(normalized_needle, cursor)
            if match_index < 0:
                return True
            if len(accumulator.matches) >= accumulator.policy.max_results:
                accumulator.mark_limit("max-results")
                return False
            original_index = (
                match_index if original_indices is None else original_indices[match_index]
            )
            accumulator.matches.append(
                WorkspaceSearchMatch(
                    path=path,
                    line=line_number,
                    column=original_index + 1,
                    preview=_preview(
                        text_line,
                        original_index,
                        len(needle),
                        accumulator.policy.max_preview_chars,
                    ),
                )
            )
            cursor = match_index + max(1, len(normalized_needle))


@dataclass(slots=True)
class _SearchAccumulator:
    request: WorkspaceSearchRequest
    cancel_requested: Callable[[], bool]
    matches: list[WorkspaceSearchMatch]
    issues: list[WorkspaceSearchIssue]
    files_scanned: int = 0
    files_skipped: int = 0
    bytes_scanned: int = 0
    bytes_reserved: int = 0
    truncated: bool = False
    cancelled: bool = False
    limit_reason: WorkspaceSearchLimitReason = "none"
    issues_truncated: bool = False

    def __init__(
        self,
        request: WorkspaceSearchRequest,
        cancel_requested: Callable[[], bool],
    ) -> None:
        self.request = request
        self.cancel_requested = cancel_requested
        self.matches = []
        self.issues = []
        self.files_scanned = 0
        self.files_skipped = 0
        self.bytes_scanned = 0
        self.bytes_reserved = 0
        self.truncated = False
        self.cancelled = False
        self.limit_reason = "none"
        self.issues_truncated = False

    @property
    def query(self):
        return self.request.query

    @property
    def policy(self):
        return self.request.policy

    @property
    def excluded_directories(self) -> frozenset[str]:
        return frozenset(name.casefold() for name in self.policy.excluded_directory_names)

    def should_stop(self) -> bool:
        if (
            self.cancelled
            or self.truncated
            and self.limit_reason
            in {
                "max-files",
                "max-total-bytes",
                "max-results",
            }
        ):
            return True
        if self.cancel_requested():
            self.cancelled = True
            return True
        return False

    def mark_limit(self, reason: WorkspaceSearchLimitReason) -> None:
        self.truncated = True
        if self.limit_reason == "none":
            self.limit_reason = reason

    def issue(self, path: Path, reason: str) -> None:
        if len(self.issues) < self.policy.max_issue_records:
            self.issues.append(WorkspaceSearchIssue(path, reason[:240]))
        else:
            self.issues_truncated = True
            self.mark_limit("max-issue-records")

    def skip_file(self, path: Path, reason: str) -> None:
        self.files_skipped += 1
        self.issue(path, reason)

    def skip_directory(self, path: Path, reason: str) -> None:
        self.issue(path, reason)

    def result(self) -> WorkspaceSearchResult:
        return WorkspaceSearchResult(
            query=self.query,
            matches=tuple(self.matches),
            files_scanned=self.files_scanned,
            files_skipped=self.files_skipped,
            bytes_scanned=self.bytes_scanned,
            truncated=self.truncated,
            cancelled=self.cancelled,
            limit_reason=self.limit_reason,
            issues=tuple(self.issues),
            issues_truncated=self.issues_truncated,
        )


class _BoundedRawReader(io.RawIOBase):
    """Expose a seekable binary stream without reading beyond a byte budget."""

    def __init__(self, stream: BinaryIO, limit: int) -> None:
        super().__init__()
        if limit < 0:
            raise ValueError("bounded reader limit must be non-negative")
        self._stream = stream
        self._limit = limit
        self._position = 0
        self._limit_probed = False
        self.limit_reached = False

    def readable(self) -> bool:
        return True

    def seekable(self) -> bool:
        return True

    def readinto(self, buffer: bytearray | memoryview) -> int:
        if self.closed:
            raise ValueError("I/O operation on closed bounded reader")
        if self._position >= self._limit:
            if not self._limit_probed:
                self.limit_reached = bool(self._stream.read(1))
                self._limit_probed = True
            return 0
        maximum = min(len(buffer), self._limit - self._position)
        chunk = self._stream.read(maximum)
        if not chunk:
            return 0
        buffer[: len(chunk)] = chunk
        self._position += len(chunk)
        return len(chunk)

    def seek(self, offset: int, whence: int = io.SEEK_SET) -> int:
        if self.closed:
            raise ValueError("I/O operation on closed bounded reader")
        if whence == io.SEEK_SET:
            target = offset
        elif whence == io.SEEK_CUR:
            target = self._position + offset
        else:
            raise io.UnsupportedOperation("bounded reader only supports start-relative seek")
        if target < 0 or target > self._limit:
            raise ValueError("bounded reader seek is outside its limit")
        self._stream.seek(target)
        self._position = target
        self._limit_probed = False
        self.limit_reached = False
        return target

    def tell(self) -> int:
        if self.closed:
            raise ValueError("I/O operation on closed bounded reader")
        return self._position

    def close(self) -> None:
        if not self.closed:
            try:
                self._stream.close()
            finally:
                super().close()


def _encoding_for(prefix: bytes) -> str | None:
    if prefix.startswith(_UTF8_BOM):
        return "utf-8-sig"
    if prefix.startswith(_UTF16_LE_BOM):
        return "utf-16"
    if prefix.startswith(_UTF16_BE_BOM):
        return "utf-16"
    return None


def _is_reparse_point(path: Path) -> bool:
    if os.name != "nt":
        return False
    try:
        attributes = getattr(path.stat(follow_symlinks=False), "st_file_attributes", 0)
    except (OSError, TypeError):
        return True
    return bool(attributes & _REPARSE_POINT)


def _casefold_with_indices(text: str) -> tuple[str, tuple[int, ...]]:
    folded_parts: list[str] = []
    original_indices: list[int] = []
    for original_index, character in enumerate(text):
        folded = character.casefold()
        folded_parts.append(folded)
        original_indices.extend([original_index] * len(folded))
    return "".join(folded_parts), tuple(original_indices)


def _stream_position(stream) -> int:
    try:
        return max(0, stream.tell())
    except OSError:
        return 0


def _line_terminated(text: str) -> bool:
    return text.endswith(("\n", "\r"))


def _preview(text_line: str, index: int, needle_length: int, maximum: int) -> str:
    expanded_line = text_line.replace("\t", "    ")
    expanded_index = len(text_line[:index].replace("\t", "    "))
    expanded_needle_length = len(text_line[index : index + needle_length].replace("\t", "    "))
    if len(expanded_line) <= maximum:
        return expanded_line
    half = max(1, (maximum - needle_length) // 2)
    start = max(0, expanded_index - half)
    end = min(len(expanded_line), expanded_index + expanded_needle_length + half)
    prefix = "…" if start > 0 else ""
    suffix = "…" if end < len(expanded_line) else ""
    if len(prefix) + len(suffix) >= maximum:
        return "…" * maximum
    body_limit = max(0, maximum - len(prefix) - len(suffix))
    body = expanded_line[start:end][:body_limit]
    return f"{prefix}{body}{suffix}"
