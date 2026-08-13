# Handoff: 2026-08-09-d6-8-fail-closed-follow-up

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-8-fail-closed-follow-up` |
| Delivery / slice | `D6 / D6.8 malformed-evidence fail-closed repair` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T22:35:00+08:00` |

## User outcome

Malformed external-execution evidence now fails closed deterministically as
`invalid-evidence`, including when an untrusted field is unhashable. The
execution gate continues to deny external execution by default, and the
`executor-unavailable` reason is documented consistently.

## Scope and boundaries

### In scope

- Repairing the malformed-evidence exception path in the pure execution gate.
- Aligning the complete deny-reason documentation.
- Performing a post-fix independent Luna source audit.

### Out of scope

- Enabling external execution, adding a loader, or implementing signatures,
  code identity, containment, executor, installation, or update policy.
- Starting QuillForge or instantiating a Qt application.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate the smallest fix and final gates |
| Project Manager | Parent role record | Track security/release dependencies and limits |
| Product | Parent role record | Preserve explainable denial in the catalog |
| Developer 1 | Parent role record | Repair evidence-shape validation and short-circuiting |
| Developer 2 | Parent role record | Align ADR/acceptance/register denial vocabulary |
| QA | Hooke / Luna | Independent post-fix source audit |

## Changed files and modules

- `src/quillforge/application/plugin_execution.py` — reject malformed typed
  evidence before policy evaluation and validate literal fields without hashing
  untrusted values.
- `docs/adr/0020-deny-by-default-plugin-execution-gate.md` — add the complete
  `executor-unavailable` deny reason and record the accepted-with-limits state.
- `docs/agent-team/reviews/D6.8-parent-review.md` and
  `docs/agent-team/reviews/D6.8-independent-luna-follow-up.md` — record the
  finding, repair, and independent result.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/ROADMAP.md`, and this handoff/index — synchronize the evidence and
  accepted-with-limits status.

## Decisions and constraints

- Malformed evidence returns one immutable `invalid-evidence` failure before
  any field is used by the policy ordering.
- The gate remains deny-by-default; no execution capability is enabled or
  wired to a loader/callback.
- D6.8 is accepted only with explicit limits; future executable-plugin
  prerequisites and security review remain separate gates.
- Shared-checkout writer: Architect only. Runtime launch and test-only assets
  remain prohibited by project policy.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| D6.8 malformed-evidence source probe | `PASS` | Unhashable `catalog_status=[]` returned only `invalid-evidence` |
| Hooke / Luna post-fix source audit | `PASS` | No files changed; no launch or tests |
| `scripts/check.ps1` | `PASS` | Formatting, compile, JSON, acceptance, and project checks passed |
| `scripts/verify_handoff.ps1` | `PASS` | Index, status, required sections, and paths validated |
| `scripts/package.ps1` | `PASS` | Root/dist copies rebuilt and synchronized after the source fix |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Current dossier refreshed; exactly three stale runtime artifact failures remain |

## Unrun checks and reason

- QuillForge.exe startup, external execution, Qt-window inspection, and visual
  review — prohibited by the current no-launch project instruction.
- Runtime security, clean-machine, cross-machine, permission-pressure,
  hard-power, signing, installer, update, legal, and support evidence — open
  release/security gates not established by this source repair.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.

## Known risks and limits

- The gate is a policy seam, not a complete security sandbox or certification
  boundary.
- The global execution policy remains disabled; no external module can be
  authorized or loaded by this increment.
- Future enablement still requires independently verified signatures,
  provenance, code identity, permissions, containment, executor lifecycle,
  installation/update policy, and adversarial security review.

## Acceptance and evidence IDs

- Acceptance: `D6-AC08`, `S21`
- Evidence: `src/quillforge/application/plugin_execution.py`,
  `docs/adr/0020-deny-by-default-plugin-execution-gate.md`,
  `docs/agent-team/reviews/D6.8-parent-review.md`,
  `docs/agent-team/reviews/D6.8-independent-luna-follow-up.md`, and the
  malformed-evidence source probe.

## Next owner and next action

- Owner: Architect
- Action: retain the refreshed artifact identity and deny-by-default policy,
  then continue the remaining D6/D7/D8/D9 gates.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `8B52D5209B6A080972D85F150595A3F85FF0CE54AA3EA7FA000E87E679A9AB7B` /
  `38,328,441` bytes; root and dist copies match
- Source revision: `tree-sha256:59ff0e8850061812defb57ac63cb3fff1d866ee2a1c604421577c3183150c1bb`
- Packaging note: source fix is packaged; runtime reports remain bound to older
  artifacts and the release decision remains no-go.

## Disposition

`accepted-with-limits`: the malformed-evidence fail-closed repair passed its
independent source audit and static probe. External execution, future security
prerequisites, runtime behavior, and release gates remain explicitly open.
