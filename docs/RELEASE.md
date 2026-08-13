# Release and packaging policy

## Build inputs

- Use the pinned Python version and the committed `uv.lock`.
- Build from a clean checkout or record any local modifications.
- Do not rely on packages installed outside the project environment.
- Keep the canonical version in `pyproject.toml` and `src/quillforge/__init__.py`
  synchronized; the build injects it into the Windows version resource.

## Packaging sequence

1. Run `uv sync`.
2. Run `scripts/check.ps1`.
3. Build the one-folder artifact first when diagnosing packaging changes.
4. Build the single-file EXE through `packaging/quillforge.spec`.
5. Run `scripts/package.ps1`; it copies the successful artifact to the project root as `QuillForge.exe`, writes `dist/QuillForge.sha256`, copies `dist/NOTICE.md`, and emits `dist/QuillForge.release.json`. The package/check flow also verifies that every non-editable `uv.lock` package has an exact-version NOTICE row.
6. Inspect `dist/QuillForge.exe`, the root test copy, the manifest, the notice sidecar, and startup behavior. The two EXE files must have the same SHA-256.
7. Record architecture, Windows FileVersion/ProductVersion, source-revision provenance, signing/installer/update decisions, and unresolved verification.
8. Run `scripts/verify_release_handoff.ps1` to produce the evidence-bound handoff dossier and explicit go/no-go.

## Distribution formats

- **One-folder** is preferred for diagnosis and can be preferable for faster startup.
- **One-file** is convenient for direct sharing but extracts bundled components at startup and may start more slowly.
- An installer, code signing, file associations, and update mechanism are separate product decisions.
- The current distribution decision contract is [`docs/adr/0011-release-distribution-decisions.md`](adr/0011-release-distribution-decisions.md): portable/manual/unsigned, with no default file associations or updater.
- The current candidate is portable and unsigned; the root EXE is a test convenience, while `dist/` is the release-handoff directory.

## Release checklist

- [ ] `uv.lock` is current.
- [ ] `scripts/check.ps1` passes.
- [ ] Packaging completes without hidden-import or resource warnings.
- [ ] The EXE starts on the supported Windows architecture.
- [ ] Version and application name are correct in runtime metadata and Windows version resources.
- [ ] `dist/QuillForge.release.json` matches both EXE hashes and sizes.
- [ ] `dist/NOTICE.md` is present, passes `scripts/verify_notice_inventory.py`, and matches the reviewed dependency inventory.
- [ ] Signing, installer, update, and support decisions are recorded; open decisions block a release claim.
- [ ] No secrets, local paths, debug logs, or build caches are included.
- [ ] Large-file and recovery claims are backed by actual evidence.

## Warning classification

PyInstaller's warning file must be inspected, not ignored. Conditional POSIX modules such as `pwd`, `grp`, `fcntl`, `resource`, and `_posixsubprocess` are expected on a Windows build when standard-library modules expose cross-platform imports. PyInstaller's delayed `pyimod02_importers` warning is also a packaging-tool diagnostic. Any warning naming `quillforge`, PyQt6/QScintilla runtime modules, a required resource, or a platform plugin is a release blocker until resolved.

The current shell does not claim large-file performance, recovery UI, clean-machine startup, installer behavior, code signing, update behavior, or commercial Qt/PyQt/QScintilla licensing. Those claims require separate evidence and product decisions. The current D8.1 manifest records these as explicit open states rather than implying completion.
