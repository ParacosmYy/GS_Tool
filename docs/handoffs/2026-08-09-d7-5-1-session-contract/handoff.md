# Handoff: 2026-08-09-d7-5-1-session-contract

| Field | Value |
|---|---|
| ID | `2026-08-09-d7-5-1-session-contract` |
| Delivery / slice | `D7 / D7.5.1 session invalid-state contract` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T15:00:00+08:00` |

## User outcome

The session-continuity design documentation now matches the implemented data-
safety behavior: absent, valid, and invalid manifests are distinct; malformed
or oversized bytes are preserved; and startup/close do not silently replace an
invalid manifest before an explicit later session change.

## Scope and boundaries

### In scope

- Align ADR-0023 with the typed `SessionLoadResult` contract recorded in ADR-0026.
- Record the D7.5.1 evidence handoff and synchronize the roadmap, handoff index,
  and delivery register.
- Keep the existing bounded, local, clean-path-only session scope unchanged.

### Out of scope

- New session schema or runtime behavior.
- Dirty/untitled content, multi-instance locking, encryption, cloud sync,
  hard-power durability, cross-machine behavior, or large-file claims.
- Starting QuillForge, creating a Qt window, or performing visual acceptance.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integration, final review, verification, and handoff decision |
| Project Manager | Kepler / Luna | Selected the evidence-closure candidate and identified the missing slice handoff |
| Product | Pauli / Luna | Defined restart, recovery-first, invalid-manifest, and bounded-scope outcomes |
| Developer 1 | Dirac / Luna | Identified the ADR-0023/ADR-0026 contract drift and application call chain |
| Developer 2 | Franklin / Luna | Audited presentation alternatives and confirmed no UI change was needed |
| QA | Copernicus / Luna | Audited static gates and recorded launch/test/package limits |

## Changed files and modules

- `docs/adr/0023-local-session-continuity.md` — states typed invalid-state
  preservation and explicit-repair semantics.
- `docs/ROADMAP.md` — records the synchronized D7.5.1 contract and remaining
  independent-review limit.
- `docs/agent-team/delivery-register.json` — makes D7 the active delivery and
  adds this handoff to D7.5.1 evidence.
- `docs/handoffs/index.json` — indexes this handoff as the latest slice.
- `docs/handoffs/2026-08-09-d7-5-1-session-contract/handoff.md` — records the
  bounded delivery decision and evidence.

## Decisions and constraints

- ADR-0026 is the authoritative invalid-manifest behavior; ADR-0023 now
  describes the same behavior rather than a competing safe-snapshot shortcut.
- No application or presentation source was changed because the implementation
  already preserves invalid bytes and repairs only after an explicit session
  change.
- Shared checkout writer: Architect, limited to the documentation and delivery
  ledger files listed above; no child writer slot was used.
- Runtime launch policy: forbidden by the current project instruction; static
  checks and compilation are allowed.
- The broader D7.5.1 delivery remains `in-progress` until the required post-fix
  independent review and any user-owned runtime acceptance are resolved.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `.\scripts\verify_handoff.ps1` | `PASS` | Handoff index, paths, required headings, and file coverage pass after this handoff is added. |
| `.\scripts\check.ps1` | `PASS` | JSON/acceptance policy, architecture boundaries, lock, formatting, lint, and compile checks pass. |
| `uv run ruff check src scripts` | `PASS` | No source or script lint findings. |
| `uv run ruff format --check src scripts` | `PASS` | All 62 files already formatted. |
| `uv run python -m compileall -q src scripts` | `PASS` | Python compilation completed without diagnostics. |
| Existing D7.5.1 source/offscreen evidence | `PASS` | Invalid/absent/valid load state, preservation, explicit repair, recovery-first restore, and close-safe writes are recorded in the parent follow-up review. |

## Unrun checks and reason

- QuillForge.exe startup, Qt-window inspection, and visual review — intentionally
  unrun because the project instruction prohibits software launch; user/Product
  owns the later review if that instruction is reversed.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Package rebuild — not applicable to this documentation-only contract sync;
  existing package identity remains unchanged.
- Post-fix independent child `PASS` review — still open; the handoff does not
  claim one merely from the parent audit.

## Known risks and limits

- D7.5.1 remains `in-progress`; this handoff closes wording drift, not the
  complete session-continuity acceptance.
- Session state remains one-profile, user-local, clean path-backed metadata;
  hard-power, cross-machine, multi-instance, encryption, selection/layout, and
  dirty/untitled continuity remain unclaimed.
- The repository has no Git metadata, so no Git diff or revision comparison is
  available; current files and command output are the source of truth.

## Acceptance and evidence IDs

- Acceptance: `D751-AC01`, `S25`
- Evidence: `docs/adr/0023-local-session-continuity.md`,
  `docs/adr/0026-session-invalid-state-preservation.md`,
  `docs/agent-team/reviews/D7.5.1-follow-up-parent-review.md`,
  `docs/handoffs/index.json`, `.\scripts\verify_handoff.ps1`,
  `.\scripts\check.ps1`

## Next owner and next action

- Owner: Architect
- Action: close the post-fix independent D7.5.1 review when a bounded reviewer
  result is available; then proceed to the already implemented UI-02 command-rail
  slice and create its separate handoff.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`, unchanged
- SHA-256 / size: existing package identity unchanged; no package rebuild in this slice
- Packaging note: documentation-only; packaged visual/startup checks remain unrun.

## Disposition

`accepted-with-limits`: ADR-0023 now agrees with the implemented preservation
contract and the evidence ledger passes static validation. D7.5.1 itself stays
`in-progress` until independent post-fix review and the explicitly unrun runtime
gates are resolved.
