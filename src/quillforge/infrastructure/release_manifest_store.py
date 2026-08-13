"""Atomic JSON persistence for release-candidate metadata."""

from __future__ import annotations

import json
import os
import tempfile
from pathlib import Path
from typing import Any

from ..application.ports import ReleaseManifestStore
from ..application.release_metadata import (
    ReleaseArtifact,
    ReleaseBuild,
    ReleaseGateDecision,
    ReleaseManifest,
)


class ReleaseManifestError(Exception):
    """Base error for release metadata persistence and decoding."""


class ReleaseManifestUnavailable(ReleaseManifestError):
    """The manifest is absent or cannot be read from the requested path."""


class ReleaseManifestCorrupt(ReleaseManifestError):
    """The manifest exists but is malformed or violates its contract."""


class ReleaseManifestPersistenceError(ReleaseManifestError):
    """The manifest could not be atomically persisted."""


class JsonReleaseManifestStore(ReleaseManifestStore):
    """Read and write one validated release manifest atomically."""

    def __init__(self, path: Path) -> None:
        self._path = path.expanduser().resolve()

    def load(self) -> ReleaseManifest:
        try:
            payload = json.loads(self._path.read_text(encoding="utf-8"))
        except FileNotFoundError as error:
            raise ReleaseManifestUnavailable(
                f"Release manifest is missing: {self._path}"
            ) from error
        except (OSError, UnicodeError) as error:
            raise ReleaseManifestUnavailable(
                f"Release manifest could not be read: {self._path}"
            ) from error
        except json.JSONDecodeError as error:
            raise ReleaseManifestCorrupt(
                f"Release manifest is not valid JSON: {self._path}"
            ) from error
        try:
            return _decode_manifest(payload)
        except (TypeError, ValueError, KeyError) as error:
            raise ReleaseManifestCorrupt(f"Release manifest is invalid: {self._path}") from error

    def save(self, manifest: ReleaseManifest) -> None:
        temporary_path: Path | None = None
        try:
            self._path.parent.mkdir(parents=True, exist_ok=True)
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                dir=self._path.parent,
                prefix=f".{self._path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temporary:
                temporary_path = Path(temporary.name)
                json.dump(
                    manifest.to_payload(), temporary, ensure_ascii=False, separators=(",", ":")
                )
                temporary.flush()
                os.fsync(temporary.fileno())
            os.replace(temporary_path, self._path)
        except OSError as error:
            raise ReleaseManifestPersistenceError(
                f"Release manifest could not be saved: {self._path}"
            ) from error
        finally:
            if temporary_path is not None and temporary_path.exists():
                try:
                    temporary_path.unlink()
                except OSError:
                    pass


def _decode_manifest(payload: object) -> ReleaseManifest:
    if not isinstance(payload, dict):
        raise TypeError("manifest payload must be an object")
    return ReleaseManifest(
        schema_version=_required_text(payload, "schema_version"),
        name=_required_text(payload, "name"),
        version=_required_text(payload, "version"),
        architecture=_required_text(payload, "architecture"),
        packaging=_required_text(payload, "packaging"),
        source_revision=_optional_text(payload, "source_revision"),
        lockfile_sha256=_required_text(payload, "lockfile_sha256"),
        build=_decode_build(payload.get("build")),
        artifact=_decode_artifact(payload.get("artifact")),
        root_test_copy=_decode_artifact(payload.get("root_test_copy")),
        notices=_decode_notices(payload.get("notices")),
        signing=_decode_gate(payload.get("signing")),
        installer=_decode_gate(payload.get("installer")),
        update=_decode_gate(payload.get("update")),
        file_associations=_decode_gate(payload.get("file_associations")),
        support=_decode_gate(payload.get("support")),
    )


def _decode_build(value: object) -> ReleaseBuild:
    payload = _required_dict(value, "build")
    return ReleaseBuild(
        built_at_utc=_required_text(payload, "built_at_utc"),
        python=_required_text(payload, "python"),
        pyinstaller=_required_text(payload, "pyinstaller"),
        version_resource=_required_text(payload, "version_resource"),
        warning_file=_required_text(payload, "warning_file"),
    )


def _decode_artifact(value: object) -> ReleaseArtifact:
    payload = _required_dict(value, "artifact")
    bytes_value = payload.get("bytes")
    if type(bytes_value) is not int:
        raise ValueError("artifact.bytes must be an integer")
    return ReleaseArtifact(
        path=_required_text(payload, "path"),
        bytes=bytes_value,
        sha256=_required_text(payload, "sha256"),
    )


def _decode_gate(value: object) -> ReleaseGateDecision:
    payload = _required_dict(value, "gate")
    return ReleaseGateDecision(
        status=_required_text(payload, "status"),
        decision=_required_text(payload, "decision"),
        owner=_optional_text(payload, "owner"),
        clean_machine=_optional_text(payload, "clean_machine"),
        issue_route=_optional_text(payload, "issue_route"),
    )


def _required_dict(value: object, field: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise TypeError(f"{field} must be an object")
    return value


def _required_text(payload: dict[str, Any], field: str) -> str:
    value = payload.get(field)
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{field} must be a non-empty string")
    return value


def _optional_text(payload: dict[str, Any], field: str) -> str | None:
    value = payload.get(field)
    if value is not None and (not isinstance(value, str) or not value.strip()):
        raise ValueError(f"{field} must be null or a non-empty string")
    return value


def _decode_notices(value: object) -> tuple[ReleaseArtifact, ...]:
    if not isinstance(value, list):
        raise TypeError("notices must be an array")
    return tuple(_decode_artifact(item) for item in value)
