"""Build utility for writing a validated QuillForge release manifest."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

if not __package__:
    sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from quillforge.application.release_metadata import (  # noqa: E402
    RELEASE_MANIFEST_SCHEMA_VERSION,
    ReleaseArtifact,
    ReleaseBuild,
    ReleaseGateDecision,
    ReleaseManifest,
)
from quillforge.infrastructure.release_manifest_store import (  # noqa: E402
    JsonReleaseManifestStore,
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--version", required=True)
    parser.add_argument("--architecture", required=True)
    parser.add_argument("--packaging", required=True)
    parser.add_argument("--lockfile-sha256", required=True)
    parser.add_argument("--built-at-utc", required=True)
    parser.add_argument("--python", required=True)
    parser.add_argument("--pyinstaller", required=True)
    parser.add_argument("--version-resource", required=True)
    parser.add_argument("--warning-file", required=True)
    parser.add_argument("--artifact-path", required=True)
    parser.add_argument("--artifact-bytes", required=True, type=int)
    parser.add_argument("--artifact-sha256", required=True)
    parser.add_argument("--root-path", required=True)
    parser.add_argument("--root-bytes", required=True, type=int)
    parser.add_argument("--root-sha256", required=True)
    parser.add_argument("--support-owner", required=True)
    parser.add_argument("--support-issue-route", required=True)
    parser.add_argument(
        "--notice",
        required=True,
        action="append",
        nargs=3,
        metavar=("PATH", "BYTES", "SHA256"),
    )
    parser.add_argument("--source-revision")
    args = parser.parse_args()

    manifest = ReleaseManifest(
        schema_version=RELEASE_MANIFEST_SCHEMA_VERSION,
        name="QuillForge",
        version=args.version,
        architecture=args.architecture,
        packaging=args.packaging,
        source_revision=args.source_revision,
        lockfile_sha256=args.lockfile_sha256,
        build=ReleaseBuild(
            built_at_utc=args.built_at_utc,
            python=args.python,
            pyinstaller=args.pyinstaller,
            version_resource=args.version_resource,
            warning_file=args.warning_file,
        ),
        artifact=ReleaseArtifact(args.artifact_path, args.artifact_bytes, args.artifact_sha256),
        root_test_copy=ReleaseArtifact(args.root_path, args.root_bytes, args.root_sha256),
        notices=tuple(ReleaseArtifact(item[0], int(item[1]), item[2]) for item in args.notice),
        signing=ReleaseGateDecision(
            status="not-signed",
            decision="Authenticode signing remains an open release gate.",
        ),
        installer=ReleaseGateDecision(
            status="not-an-installer",
            decision="Portable one-file output only; installer decision remains open.",
        ),
        update=ReleaseGateDecision(
            status="not-implemented",
            decision="Manual artifact replacement until an update channel is approved.",
        ),
        file_associations=ReleaseGateDecision(
            status="not-configured",
            decision=(
                "Portable candidate registers no file associations; an installer policy "
                "must define any future opt-in and reversible associations."
            ),
        ),
        support=ReleaseGateDecision(
            status="handoff-pending",
            decision=(
                "Support handoff has a named project owner and local issue route; "
                "clean-machine evidence remains open."
            ),
            owner=args.support_owner,
            clean_machine="not-verified",
            issue_route=args.support_issue_route,
        ),
    )
    JsonReleaseManifestStore(args.manifest).save(manifest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
