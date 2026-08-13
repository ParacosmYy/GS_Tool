"""Build-time provenance, PE version-resource, and payload verification helpers."""

from __future__ import annotations

import argparse
import hashlib
import importlib.metadata
import json
import os
import re
import sys
from datetime import UTC, datetime
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
VERSION_SOURCE = PROJECT_ROOT / "src" / "serialforge" / "__init__.py"
VERSION_PATTERN = re.compile(r'^\s*__version__\s*=\s*"([^"]+)"\s*$', re.MULTILINE)
VERSION_VALUE_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")
MANIFEST_NAMES = {"PROVENANCE.json", "SHA256SUMS.txt"}
BLE_TOKENS = ("bleak", "winrt")
VENDOR_FILE_PATTERN = re.compile(
    r"(?i)(?:j[-_]?link|segger).*\.(?:dll|exe|sys|pyd)$|"
    r"(?:^|[/\\])probe[-_]rs(?:[/\\]|\.(?:dll|exe|sys|pyd)$)"
)


def read_version(path: Path = VERSION_SOURCE) -> str:
    """Read and validate the one project version source."""

    match = VERSION_PATTERN.search(path.read_text(encoding="utf-8"))
    if match is None or VERSION_VALUE_PATTERN.fullmatch(match.group(1)) is None:
        raise ValueError(f"invalid project version source: {path}")
    return match.group(1)


def version_tuple(version: str) -> tuple[int, int, int, int]:
    """Convert a semantic project version to a Windows four-part version."""

    if VERSION_VALUE_PATTERN.fullmatch(version) is None:
        raise ValueError(f"PE version requires x.y.z: {version}")
    return (*[int(part) for part in version.split(".")], 0)


def write_version_file(source: Path, output: Path) -> None:
    """Generate a PyInstaller version resource from the project version."""

    version = read_version(source)
    parts = version_tuple(version)
    version_text = ".".join(str(part) for part in parts)
    output.parent.mkdir(parents=True, exist_ok=True)
    text = f"""# UTF-8
VSVersionInfo(
  ffi=FixedFileInfo(
    filevers={parts}, prodvers={parts},
    mask=0x3f, flags=0x0, OS=0x40004, fileType=0x1,
    subtype=0x0, date=(0, 0)),
  kids=[
    StringFileInfo([
      StringTable('040904B0', [
        StringStruct('CompanyName', 'SerialForge'),
        StringStruct('FileDescription', 'Embedded debug console'),
        StringStruct('FileVersion', '{version_text}'),
        StringStruct('InternalName', 'SerialForge'),
        StringStruct('OriginalFilename', 'SerialForge.exe'),
        StringStruct('ProductName', 'SerialForge'),
        StringStruct('ProductVersion', '{version_text}')])]),
    VarFileInfo([VarStruct('Translation', [1033, 1200])])
  ]
)
"""
    output.write_text(text, encoding="utf-8", newline="\n")


def sha256(path: Path) -> str:
    """Hash one build artifact without loading it all into memory."""

    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def relative_path(root: Path, path: Path) -> str:
    """Return a safe, slash-normalized path below root."""

    root = root.resolve()
    path = path.resolve()
    try:
        relative = path.relative_to(root)
    except ValueError as exc:
        raise ValueError(f"path is outside provenance root: {path}") from exc
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError(f"unsafe relative path: {relative}")
    return relative.as_posix()


def payload_files(root: Path) -> list[tuple[str, Path]]:
    """Collect deterministic payload files, excluding self-referential sidecars."""

    result: list[tuple[str, Path]] = []
    for path in sorted(root.resolve().rglob("*")):
        if not path.is_file():
            continue
        if path.is_symlink():
            raise ValueError(f"symlink is not allowed in payload: {path}")
        relative = relative_path(root, path)
        if relative in MANIFEST_NAMES:
            continue
        result.append((relative, path))
    if not result:
        raise ValueError(f"provenance root has no payload files: {root}")
    return result


def archive_entries(path: Path | None) -> list[str]:
    """Read a PyInstaller archive listing captured by the packaging script."""

    if path is None:
        return []
    if not path.is_file():
        raise FileNotFoundError(path)
    return [
        line.strip() for line in path.read_text(encoding="utf-8-sig").splitlines() if line.strip()
    ]


def optional_matches(entries: list[str]) -> list[str]:
    """Find optional BLE names in paths or archive entries."""

    return sorted(
        {entry for entry in entries if any(token in entry.lower() for token in BLE_TOKENS)}
    )


def vendor_matches(entries: list[str]) -> list[str]:
    """Find vendor binaries, while allowing the app's RTT config string."""

    return sorted({entry for entry in entries if VENDOR_FILE_PATTERN.search(entry)})


def installed_version(distribution: str) -> str | None:
    """Return one installed build-tool version without adding a dependency."""

    try:
        return importlib.metadata.version(distribution)
    except importlib.metadata.PackageNotFoundError:
        return None


def write_manifest(args: argparse.Namespace) -> None:
    """Write a manifest and checksum sidecar, failing on package-content violations."""

    root = args.root.resolve()
    artifact = args.artifact.resolve()
    version = read_version(args.version_source.resolve())
    if args.version != version:
        raise ValueError(f"manifest version mismatch: {args.version} != {version}")
    files = payload_files(root)
    file_records = [
        {"path": relative, "size": path.stat().st_size, "sha256": sha256(path)}
        for relative, path in files
    ]
    artifact_relative = relative_path(root, artifact)
    archive = archive_entries(args.archive_listing)
    optional = optional_matches(archive + [record["path"] for record in file_records])
    vendor = vendor_matches(archive + [record["path"] for record in file_records])
    if vendor:
        raise ValueError(f"vendor binary content is not allowed: {vendor[:8]}")
    if args.variant == "core" and optional:
        raise ValueError(f"core package contains BLE content: {optional[:8]}")
    if args.variant == "ble" and not all(
        any(token in entry.lower() for entry in archive) for token in BLE_TOKENS
    ):
        raise ValueError("BLE package archive is missing Bleak or WinRT content")
    lock_path = args.lockfile.resolve()
    archive_sha = sha256(args.archive_listing) if args.archive_listing else None
    manifest = {
        "schema_version": 1,
        "product": "SerialForge",
        "version": version,
        "variant": args.variant,
        "mode": args.mode,
        "platform": "windows-x64",
        "status": "engineering_build",
        "release_eligible": False,
        "source": {
            "revision": args.source_revision or os.environ.get("GITHUB_SHA") or "local-unpinned",
            "version_source": relative_path(PROJECT_ROOT, args.version_source),
            "uv_lock_sha256": sha256(lock_path),
        },
        "toolchain": {
            "python": sys.version.split()[0],
            "pyinstaller": installed_version("pyinstaller"),
            "platform": sys.platform,
        },
        "artifact": {
            "path": artifact_relative,
            "size": next(
                item["size"] for item in file_records if item["path"] == artifact_relative
            ),
            "sha256": next(
                item["sha256"] for item in file_records if item["path"] == artifact_relative
            ),
        },
        "pe": {
            "file_version": args.pe_file_version,
            "product_version": args.pe_product_version,
            "product_name": args.pe_product_name,
        },
        "signature": {"status": args.signature_status or "unknown"},
        "hardware_acceptance": "not_run",
        "license": {
            "status": "inventory_only_pending_full_texts",
            "inventory": args.license_inventory,
            "full_text_bundle_present": False,
        },
        "capabilities": (
            ["uart", "tcp", "udp", "ble_gatt"] if args.variant == "ble" else ["uart", "tcp", "udp"]
        ),
        "content_scan": {
            "archive_listing_sha256": archive_sha,
            "ble_matches": optional,
            "vendor_binary_matches": vendor,
        },
        "files": file_records,
        "built_at_utc": datetime.now(UTC).isoformat(),
    }
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    checksum_path = output.with_name("SHA256SUMS.txt")
    checksum_path.write_text(
        "".join(f"{item['sha256']}  {item['path']}\n" for item in file_records),
        encoding="utf-8",
    )


def verify_manifest(path: Path) -> None:
    """Verify every manifest file exists below its sidecar directory and hashes match."""

    manifest_path = path.resolve()
    data = json.loads(manifest_path.read_text(encoding="utf-8"))
    root = manifest_path.parent
    if data.get("schema_version") != 1 or data.get("product") != "SerialForge":
        raise ValueError(f"unsupported provenance manifest: {manifest_path}")
    for item in data.get("files", ()):
        relative = item["path"]
        target = (root / relative).resolve()
        if relative.startswith(("/", "\\")) or ".." in Path(relative).parts:
            raise ValueError(f"unsafe manifest path: {relative}")
        if relative_path(root, target) != relative or not target.is_file():
            raise ValueError(f"manifest file missing: {relative}")
        if target.stat().st_size != int(item["size"]):
            raise ValueError(f"manifest size mismatch: {relative}")
        if sha256(target) != item["sha256"]:
            raise ValueError(f"manifest hash mismatch: {relative}")
    if data.get("content_scan", {}).get("vendor_binary_matches"):
        raise ValueError("manifest contains forbidden vendor binaries")
    print(f"provenance verified: {manifest_path}")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    version_parser = subparsers.add_parser("version")
    version_parser.add_argument("--source", type=Path, default=VERSION_SOURCE)
    version_parser.set_defaults(handler=lambda args: print(read_version(args.source)))

    version_parser = subparsers.add_parser("version-file")
    version_parser.add_argument("--source", type=Path, default=VERSION_SOURCE)
    version_parser.add_argument("--output", type=Path, required=True)
    version_parser.set_defaults(handler=lambda args: write_version_file(args.source, args.output))

    manifest_parser = subparsers.add_parser("manifest")
    manifest_parser.add_argument("--root", type=Path, required=True)
    manifest_parser.add_argument("--artifact", type=Path, required=True)
    manifest_parser.add_argument("--version", required=True)
    manifest_parser.add_argument("--version-source", type=Path, default=VERSION_SOURCE)
    manifest_parser.add_argument("--variant", choices=("core", "ble"), required=True)
    manifest_parser.add_argument("--mode", choices=("onedir", "onefile"), required=True)
    manifest_parser.add_argument("--source-revision", default="")
    manifest_parser.add_argument("--lockfile", type=Path, required=True)
    manifest_parser.add_argument("--archive-listing", type=Path)
    manifest_parser.add_argument("--pe-file-version", default="")
    manifest_parser.add_argument("--pe-product-version", default="")
    manifest_parser.add_argument("--pe-product-name", default="")
    manifest_parser.add_argument("--signature-status", default="")
    manifest_parser.add_argument("--license-inventory", default="THIRD_PARTY_NOTICES.md")
    manifest_parser.add_argument("--output", type=Path, required=True)
    manifest_parser.set_defaults(handler=write_manifest)

    verify_parser = subparsers.add_parser("verify")
    verify_parser.add_argument("--manifest", type=Path, required=True)
    verify_parser.set_defaults(handler=lambda args: verify_manifest(args.manifest))
    return parser


def main() -> int:
    args = build_parser().parse_args()
    args.handler(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
