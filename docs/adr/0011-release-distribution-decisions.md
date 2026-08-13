# ADR-0011: Release distribution decisions

## Status

Accepted as the D8.2 distribution decision contract. Signing, installer, and
update implementation remain release gates.

## Context

The current artifact is a Windows x64 PyInstaller one-file candidate. It is
useful for controlled testing, but an enterprise release also needs an
explicit distribution posture. The checkout has no signing certificate,
installer project, update service, or clean-machine deployment environment;
those absences must be represented as decisions rather than hidden assumptions.

## Decisions

1. **Current channel:** distribute only as a manually handed-off portable
   candidate. `dist/QuillForge.exe`, `dist/QuillForge.release.json`, and
   `dist/NOTICE.md` are the release-handoff set; the root EXE remains a test
   convenience.
2. **Signing:** Authenticode signing is required before an enterprise release.
   It is not performed in this checkout because no approved certificate,
   signing identity, timestamp policy, or protected key workflow is available.
   Owner: Release Engineering. Gate: `not-signed`.
3. **Installer:** no installer is shipped for the current candidate. The
   future installer must be selected and approved after signing and clean-
   machine deployment testing; it must not silently write file associations or
   registry state. Gate: `not-an-installer`.
4. **File associations:** none by default. Association policy is deferred to
   the installer decision and must be opt-in and reversible.
5. **Updates:** no in-app updater or network update channel is implemented.
   Until an approved channel exists, updates are manual replacement of the
   portable artifact after verifying the manifest SHA-256. Rollback means
   retaining the previous artifact and its manifest. Gate: `not-implemented`.
6. **Release blocking:** an unsigned artifact, absent installer decision,
   absent update channel, or unverified clean-machine/support handoff cannot
   be described as an enterprise release.

## Evidence and open work

- `dist/QuillForge.release.json` records the current gate states and exact
  artifact identity.
- `docs/RELEASE.md` describes the packaging and warning classification policy.
- Clean-machine verification, legal notice clearance, and support ownership
  remain D8.3 work.

## Consequences

The current output has a deterministic and honest internal-test distribution
contract. It does not claim installation, trust, automatic updates, file
association, or enterprise support until the named owners supply the missing
artifacts and environment evidence.
