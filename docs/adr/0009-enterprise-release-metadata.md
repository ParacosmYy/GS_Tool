# ADR-0009: Enterprise release candidate identity and handoff

## Status

Accepted for implementation with explicit release gates still open.

## Context

QuillForge currently produces a usable Windows x64 one-file executable, but the
executable does not carry Windows version resources and the package has no
machine-readable artifact identity or adjacent third-party notice inventory.
Without those artifacts, a root EXE is convenient for testing but is not a
supportable release candidate.

## Decision

1. Keep `pyproject.toml` and `src/quillforge/__init__.py` as the canonical
   application version sources and require them to agree.
2. Inject the matching `0.1.0.0` Windows version resource through
   `packaging/version_info.txt` and the PyInstaller spec.
3. Make `scripts/package.ps1` emit `dist/QuillForge.release.json` containing
   version, architecture, exact root/dist hashes and sizes, lockfile hash,
   build-tool provenance, notice path, and explicit signing/installer/update/
   support states.
4. Ship `dist/NOTICE.md` as a sidecar copied from the reviewed inventory at
   `docs/third_party/NOTICE.md`, and record its byte size and SHA-256 in the
   manifest. The sidecar is an auditable handoff artifact; it does not itself
   grant a license.
5. Treat the current output as an unsigned portable one-file test/release
   candidate. Authenticode signing, an installer, update policy, clean-machine
   verification, and legal notice completion remain separate gates.

## Consequences

- Windows Explorer and file properties can identify the application version.
- Artifact identity is recorded without depending on Git history, which this
  checkout intentionally does not have.
- A successful packaging command still does not imply signing, installation,
  clean-machine support, or redistribution clearance.
- The root EXE remains a test convenience; `dist/` is the handoff directory.

## Verification

- `scripts/check.ps1` validates the version sources, version resource, and
  notice inventory.
- `scripts/package.ps1` must generate the EXE, root/dist SHA-256 match, release
  manifest, and notice sidecar.
- Windows x64 startup and FileVersion/ProductVersion inspection are required;
  clean interactive-machine startup remains environment-dependent.
