# Handoff: 2026-08-09-d6-2-shared-validator-length

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-2-shared-validator-length` |
| Delivery / slice | `D6 / D6.2 shared plugin_id validator invariant` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:20:48+08:00` |

## User outcome

In-process plugin registration and external catalog classification now share the
same maximum `plugin_id` length invariant before registry mutation or
compatibility classification.

## Scope and boundaries

### In scope

- Add the missing public-validator `plugin_id` length guard.
- Record the independent review finding and repair evidence.
- Synchronize D6.2 acceptance and delivery records.

### Out of scope

- Signature or publisher verification, trust persistence, dynamic loading,
  sandboxing, installation, updates, or external execution.
- Runtime launch, visual review, unit tests, mocks, fixtures, harnesses, or
  test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate the invariant fix and final verification |
| Project Manager | Parent role record | Track D6 dependency and security limits |
| Product | Parent role record | Preserve valid plugin behavior and explicit boundaries |
| Developer 1 | Parent Codex | Update the public manifest validator |
| Developer 2 | Parent Codex | Update D6 evidence and package handoff |
| QA | Helmholtz / Luna, read-only | Identified the missing shared length invariant |

## Changed files and modules

- `src/quillforge/plugins/api.py` — enforce the shared `plugin_id` length bound.
- `docs/adr/0014-unified-plugin-manifest-policy.md` — record the repaired
  invariant.
- `docs/agent-team/reviews/D6.2-shared-validator-follow-up-parent-review.md` —
  record the independent finding and parent fix audit.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/ROADMAP.md`, and `docs/handoffs/index.json` — synchronize evidence.

## Decisions and constraints

- `PLUGIN_MANIFEST_MAX_FIELD_LENGTH` remains the single constant for all
  manifest identity/display fields.
- A structural validator is not a security boundary for already executing
  Python code.
- Shared-checkout writer: Architect only; the Luna reviewer was read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `Helmholtz / Luna read-only review` | `REVISE FINDING` | Found catalog-only `plugin_id` length enforcement; no files changed. |
| No-write validator source probe | `PASS` | Oversized `plugin_id` is rejected by the shared validator; valid ID remains accepted. |
| `scripts/verify_handoff.ps1` | `PASS` | Indexed handoffs have unique, well-formed, case-sensitive status parity, including CRLF-safe parsing. |
| `scripts/check.ps1` | `PASS` | Static, formatting, compile, and acceptance checks passed; 62 files were already formatted. |
| `scripts/package.ps1` | `PASS` | Root/dist identity is `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4`, 38,327,064 bytes. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Wrote the current dossier; the three artifact-bound runtime reports are stale after this package. |

## Unrun checks and reason

- QuillForge.exe startup, plugin dialogs, packaged plugin diagnostics, and
  visual review — prohibited by the project no-launch instruction.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Signature, sandbox, installer, update, clean-machine, and external security
  review — future release/security gates.

## Known risks and limits

- The validator enforces structure only; it does not establish trust or code
  identity.
- D6.1 and D6.3–D6.5 still need their own handoff/status closure decisions;
  D6.4/D6.5 were conditionally reviewed on this shared policy.
- Packaging changes the artifact identity and keeps the release dossier no-go
  until permitted runtime evidence is refreshed.

## Acceptance and evidence IDs

- Acceptance: `D6-AC02`, `S14`
- Evidence: `src/quillforge/plugins/api.py`,
  `src/quillforge/plugins/catalog.py`,
  `docs/adr/0014-unified-plugin-manifest-policy.md`,
  `docs/agent-team/reviews/D6.2-shared-validator-follow-up-parent-review.md`,
  and the post-package manifest/dossier identity.

## Next owner and next action

- Owner: Architect
- Action: continue D6.1/D6.3–D6.5 handoff/status closure, preserving the
  repaired shared validator as the pre-registration gate.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Source snapshot: `tree-sha256:0428b196b6a397ce1a3a53807d21c108cede04b69f147accafe28f8f40ccc241`
- Packaging note: application source changed; historical runtime reports will
  be stale until a permitted refresh.

## Disposition

`accepted-with-limits`: the D6.2 shared structural invariant is repaired and
will be package/static verified; trust and external execution gates remain
open.
