# D287 parent review — startup state preflight

## Scope

Reviewed the D287 changes in `src/quillforge/app.py` and
`scripts/audit_presentation_contracts.py`.

## Findings

- PASS — `session_preflight` reuses `SessionService.load()` and therefore the
  production normalization and absent/valid/invalid state contract.
- PASS — `recovery_preflight` reuses `JsonRecoverySnapshotStore.list_snapshots()`
  and distinguishes manifest files from valid snapshots without exposing
  snapshot text or document path lists.
- PASS — probes are registered only in `--diagnose-startup`; normal desktop
  startup code is unchanged.
- PASS — the static contract guards probe order, production store calls,
  read-only behavior, and the no-content reporting boundary.
- PASS — simplification assessment found no safe extraction that would reduce
  duplication without introducing a second diagnostic service or changing the
  existing entry-point boundary.

## Review limits

The independent Luna/max reviewer returned `NO_CONCLUSION` after two bounded
wait windows and was closed. This is recorded rather than presented as an
independent approval. Native EXE/Qt launch and asynchronous restore completion
remain intentionally unrun.

## Decision

Parent review: PASS. Simplification assessment: PASS.
