"""Typed release-candidate metadata contracts."""

import re
from dataclasses import dataclass

from .errors import ApplicationValidationError

RELEASE_MANIFEST_SCHEMA_VERSION = "1.0"
_SHA256_PATTERN = re.compile(r"^[0-9a-fA-F]{64}$")
_VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+(?:\.\d+)?$")


def _require_text(value: str, field: str) -> None:
    if not isinstance(value, str) or not value.strip():
        raise ApplicationValidationError(f"{field} must be a non-empty string")


def _require_sha256(value: str, field: str) -> None:
    if not isinstance(value, str) or _SHA256_PATTERN.fullmatch(value) is None:
        raise ApplicationValidationError(f"{field} must be a 64-character SHA-256 digest")


@dataclass(frozen=True, slots=True)
class ReleaseArtifact:
    """Identity of one packaged file."""

    path: str
    bytes: int
    sha256: str

    def __post_init__(self) -> None:
        _require_text(self.path, "artifact.path")
        if type(self.bytes) is not int or self.bytes < 1:
            raise ApplicationValidationError("artifact.bytes must be a positive integer")
        _require_sha256(self.sha256, "artifact.sha256")


@dataclass(frozen=True, slots=True)
class ReleaseBuild:
    """Build-tool provenance attached to a release candidate."""

    built_at_utc: str
    python: str
    pyinstaller: str
    version_resource: str
    warning_file: str

    def __post_init__(self) -> None:
        for field in ("built_at_utc", "python", "pyinstaller", "version_resource", "warning_file"):
            _require_text(getattr(self, field), f"build.{field}")


@dataclass(frozen=True, slots=True)
class ReleaseGateDecision:
    """Explicit state for a release gate that may still be open."""

    status: str
    decision: str
    owner: str | None = None
    clean_machine: str | None = None
    issue_route: str | None = None

    def __post_init__(self) -> None:
        _require_text(self.status, "gate.status")
        _require_text(self.decision, "gate.decision")
        if self.owner is not None:
            _require_text(self.owner, "gate.owner")
        if self.clean_machine is not None:
            _require_text(self.clean_machine, "gate.clean_machine")
        if self.issue_route is not None:
            _require_text(self.issue_route, "gate.issue_route")


@dataclass(frozen=True, slots=True)
class ReleaseManifest:
    """Validated, machine-readable identity for one release candidate."""

    schema_version: str
    name: str
    version: str
    architecture: str
    packaging: str
    source_revision: str | None
    lockfile_sha256: str
    build: ReleaseBuild
    artifact: ReleaseArtifact
    root_test_copy: ReleaseArtifact
    notices: tuple[ReleaseArtifact, ...]
    signing: ReleaseGateDecision
    installer: ReleaseGateDecision
    update: ReleaseGateDecision
    file_associations: ReleaseGateDecision
    support: ReleaseGateDecision

    def __post_init__(self) -> None:
        if self.schema_version != RELEASE_MANIFEST_SCHEMA_VERSION:
            raise ApplicationValidationError(
                f"Unsupported release manifest schema: {self.schema_version}"
            )
        _require_text(self.name, "name")
        if _VERSION_PATTERN.fullmatch(self.version) is None:
            raise ApplicationValidationError("version must use numeric dotted release notation")
        _require_text(self.architecture, "architecture")
        _require_text(self.packaging, "packaging")
        if self.source_revision is not None:
            _require_text(self.source_revision, "source_revision")
        _require_sha256(self.lockfile_sha256, "lockfile_sha256")
        if self.artifact.bytes != self.root_test_copy.bytes:
            raise ApplicationValidationError("artifact and root test copy sizes must match")
        if self.artifact.sha256.lower() != self.root_test_copy.sha256.lower():
            raise ApplicationValidationError("artifact and root test copy hashes must match")
        if not self.notices:
            raise ApplicationValidationError("notices must contain at least one non-empty path")

    def to_payload(self) -> dict[str, object]:
        """Return a JSON-ready payload without exposing mutable internal state."""
        return {
            "schema_version": self.schema_version,
            "name": self.name,
            "version": self.version,
            "architecture": self.architecture,
            "packaging": self.packaging,
            "source_revision": self.source_revision,
            "lockfile_sha256": self.lockfile_sha256,
            "build": {
                "built_at_utc": self.build.built_at_utc,
                "python": self.build.python,
                "pyinstaller": self.build.pyinstaller,
                "version_resource": self.build.version_resource,
                "warning_file": self.build.warning_file,
            },
            "artifact": {
                "path": self.artifact.path,
                "bytes": self.artifact.bytes,
                "sha256": self.artifact.sha256,
            },
            "root_test_copy": {
                "path": self.root_test_copy.path,
                "bytes": self.root_test_copy.bytes,
                "sha256": self.root_test_copy.sha256,
            },
            "notices": [
                {"path": item.path, "bytes": item.bytes, "sha256": item.sha256}
                for item in self.notices
            ],
            "signing": _gate_payload(self.signing),
            "installer": _gate_payload(self.installer),
            "update": _gate_payload(self.update),
            "file_associations": _gate_payload(self.file_associations),
            "support": _gate_payload(self.support),
        }


def _gate_payload(gate: ReleaseGateDecision) -> dict[str, str]:
    payload = {"status": gate.status, "decision": gate.decision}
    if gate.owner is not None:
        payload["owner"] = gate.owner
    if gate.clean_machine is not None:
        payload["clean_machine"] = gate.clean_machine
    if gate.issue_route is not None:
        payload["issue_route"] = gate.issue_route
    return payload
