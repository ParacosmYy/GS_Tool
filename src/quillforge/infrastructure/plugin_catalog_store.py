"""Bounded JSON adapter for the user-local extension catalog."""

import json
import os
from pathlib import Path

from ..application.ports import PluginCatalogCandidate, PluginCatalogReadResult, PluginCatalogStore

DEFAULT_PLUGIN_CATALOG_MAX_ENTRIES = 256
DEFAULT_PLUGIN_CATALOG_MAX_DIRECTORY_ENTRIES = 4096
DEFAULT_PLUGIN_MANIFEST_MAX_BYTES = 64 * 1024


class JsonPluginCatalogStore(PluginCatalogStore):
    """Read top-level manifest files without importing or executing their entrypoints."""

    def __init__(
        self,
        directory: Path,
        *,
        max_entries: int = DEFAULT_PLUGIN_CATALOG_MAX_ENTRIES,
        max_directory_entries: int = DEFAULT_PLUGIN_CATALOG_MAX_DIRECTORY_ENTRIES,
        max_manifest_bytes: int = DEFAULT_PLUGIN_MANIFEST_MAX_BYTES,
    ) -> None:
        if max_entries < 1:
            raise ValueError("Plugin catalog entry limit must be positive")
        if max_directory_entries < 1:
            raise ValueError("Plugin catalog directory entry limit must be positive")
        if max_manifest_bytes < 1:
            raise ValueError("Plugin manifest byte limit must be positive")
        self._directory = directory.expanduser().resolve()
        self._max_entries = max_entries
        self._max_directory_entries = max_directory_entries
        self._max_manifest_bytes = max_manifest_bytes

    def read_candidates(self) -> PluginCatalogReadResult:
        """Read a bounded set of JSON files and preserve per-file diagnostics."""
        if not self._directory.exists():
            return PluginCatalogReadResult(self._directory, ())
        if not self._directory.is_dir():
            return PluginCatalogReadResult(
                self._directory,
                (),
                error="Plugin catalog root is not a directory",
            )
        try:
            paths: list[Path] = []
            truncated = False
            for index, path in enumerate(self._directory.iterdir()):
                if index >= self._max_directory_entries:
                    truncated = True
                    break
                if path.suffix.casefold() != ".json":
                    continue
                if len(paths) >= self._max_entries:
                    truncated = True
                    break
                paths.append(path)
        except OSError as error:
            return PluginCatalogReadResult(self._directory, (), error=str(error))

        paths.sort(key=lambda path: path.name.casefold())
        selected_paths = paths
        candidates = tuple(self._read_candidate(path) for path in selected_paths)
        return PluginCatalogReadResult(
            root=self._directory,
            candidates=candidates,
            truncated=truncated,
        )

    def _read_candidate(self, path: Path) -> PluginCatalogCandidate:
        if path.is_symlink():
            return PluginCatalogCandidate(path, error="Symbolic-link manifests are not read")
        try:
            if not path.is_file():
                return PluginCatalogCandidate(path, error="Manifest path is not a regular file")
            if path.stat().st_size > self._max_manifest_bytes:
                return PluginCatalogCandidate(
                    path,
                    error=f"Manifest exceeds {self._max_manifest_bytes} bytes",
                )
            with path.open("rb") as source:
                raw_payload = source.read(self._max_manifest_bytes + 1)
            if len(raw_payload) > self._max_manifest_bytes:
                return PluginCatalogCandidate(
                    path,
                    error=f"Manifest exceeds {self._max_manifest_bytes} bytes",
                )
            payload = json.loads(raw_payload)
        except (OSError, UnicodeError, TypeError, ValueError, json.JSONDecodeError) as error:
            return PluginCatalogCandidate(path, error=f"Manifest could not be read: {error}")
        return PluginCatalogCandidate(path, payload=payload)


def default_plugin_catalog_directory() -> Path:
    """Return the explicit user-local catalog root without creating it."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / "plugins"
