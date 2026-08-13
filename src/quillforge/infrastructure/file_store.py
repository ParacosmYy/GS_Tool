"""Filesystem document adapter with encoding preservation and atomic saves."""

import locale
import os
import shutil
import tempfile
from pathlib import Path

from ..application.ports import DocumentStore
from ..domain.models import (
    DocumentConflictError,
    DocumentRevision,
    LineEnding,
    LoadedDocument,
)


class FileDocumentStore(DocumentStore):
    """Read/write text files without leaking filesystem details into the app layer."""

    def __init__(self, *, keep_backups: bool = True) -> None:
        self._keep_backups = keep_backups

    def load(self, path: Path) -> LoadedDocument:
        """Read bytes, detect a practical encoding, and normalize line endings."""
        revision_before = _revision(path)
        data = path.read_bytes()
        encoding = _detect_encoding(data)
        decoded = data.decode(encoding)
        line_ending = _detect_line_ending(decoded)
        text = _normalize_line_endings(decoded)
        revision_after = _revision(path)
        if revision_after != revision_before:
            raise DocumentConflictError(path)
        return LoadedDocument(text, encoding, line_ending, revision_after)

    def current_revision(self, path: Path) -> DocumentRevision | None:
        """Expose a read-only revision query for recovery explanations."""
        return _revision_if_exists(path.resolve())

    def save(
        self,
        path: Path,
        text: str,
        *,
        encoding: str,
        line_ending: LineEnding,
        expected_revision: DocumentRevision | None,
    ) -> DocumentRevision:
        """Write a same-directory temporary file and replace the target atomically."""
        path = path.resolve()
        current_revision = _revision_if_exists(path)
        if expected_revision is not None and current_revision != expected_revision:
            raise DocumentConflictError(path)

        path.parent.mkdir(parents=True, exist_ok=True)
        rendered = _render_line_endings(text, line_ending)
        encoded = rendered.encode(encoding)
        temporary_path: Path | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="wb",
                dir=path.parent,
                prefix=f".{path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary_path = Path(temporary.name)
                temporary.write(encoded)
                temporary.flush()
                os.fsync(temporary.fileno())

            written_revision = _revision(temporary_path)
            # Re-check even when expected_revision is None (for Save As/new targets):
            # a file created or changed during serialization must not be overwritten.
            if _revision_if_exists(path) != current_revision:
                raise DocumentConflictError(path)
            if self._keep_backups and path.exists():
                shutil.copy2(path, path.with_name(f"{path.name}.bak"))
            os.replace(temporary_path, path)
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass

        # Do not read the destination after replace: an external writer could
        # change it between replace and that read and make our UI state appear
        # clean for someone else's bytes.
        return written_revision


def _detect_encoding(data: bytes) -> str:
    if data.startswith(b"\xef\xbb\xbf"):
        return "utf-8-sig"
    if data.startswith((b"\xff\xfe", b"\xfe\xff")):
        return "utf-16"
    try:
        data.decode("utf-8")
    except UnicodeDecodeError:
        return locale.getpreferredencoding(False)
    return "utf-8"


def _detect_line_ending(text: str) -> LineEnding:
    if "\r\n" in text:
        return "CRLF"
    if "\r" in text:
        return "CR"
    return "LF"


def _normalize_line_endings(text: str) -> str:
    return text.replace("\r\n", "\n").replace("\r", "\n")


def _render_line_endings(text: str, line_ending: LineEnding) -> str:
    normalized = _normalize_line_endings(text)
    if line_ending == "CRLF":
        return normalized.replace("\n", "\r\n")
    if line_ending == "CR":
        return normalized.replace("\n", "\r")
    return normalized


def _revision(path: Path) -> DocumentRevision:
    stat = path.stat()
    return DocumentRevision(modified_ns=stat.st_mtime_ns, size=stat.st_size)


def _revision_if_exists(path: Path) -> DocumentRevision | None:
    return _revision(path) if path.exists() else None
