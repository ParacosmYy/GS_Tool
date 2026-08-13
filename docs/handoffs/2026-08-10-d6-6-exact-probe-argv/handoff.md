# Handoff: 2026-08-10-d6-6-exact-probe-argv

| Field | Value |
|---|---|
| ID | `2026-08-10-d6-6-exact-probe-argv` |
| Delivery / slice | `D6 / D6.6 exact probe-only host entry guard` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-10T02:30:24+08:00` |

## User outcome

The diagnostic plugin host now enters its protocol path only through the
documented `QuillForge.exe --plugin-host --probe` or
`python -m quillforge --plugin-host --probe` shape. Unknown, duplicate, extra,
or reordered arguments fail closed before stdio or Qt work begins.

## Scope and boundaries

### In scope

- Normalize host arguments and enforce the exact executable-plus-two-flag
  boundary.
- Record the Architect review, independent review, simplification assessment,
  and source-only verification.
- Project D6-AC06 and S19 to accepted-with-limits with runtime limits preserved.

### Out of scope

- Dynamic plugin loading, signatures, installation, updates, external execution,
  or a complete security sandbox.
- QuillForge.exe/QApplication launch, interactive visual acceptance, and clean or
  cross-machine evidence under the current project boundary.
- Unit-test assets, mocks, fixtures, harnesses, or test-only code.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent | Integration, final architecture decision, verification, and handoff |
| Project Manager | Parent role record | Scope, dependencies, and remaining D7/D8 risk |
| Product | Parent role record | Confirm exact diagnostic-entry outcome and limits |
| Developer 1 | Parent writer | `host_process.py` argument-boundary change |
| Developer 2 | Parent role record | Ledger, ADR, and package evidence synchronization |
| QA | Sagan / Luna-max read-only; parent static probes | Independent review and authorized non-destructive checks |

## Changed files and modules

- `src/quillforge/plugins/host_process.py` — exact argument predicate at the
  host boundary.
- `docs/agent-team/reviews/D6.6-exact-probe-argv-parent-review.md` — architecture,
  independent review, simplification, applicability, and limits.
- `docs/adr/0018-process-isolated-plugin-host-protocol.md` — implementation note
  that the documented entry shape is enforced.
- `docs/RELEASE_HANDOFF.md` — bind the release summary to the rebuilt candidate.
- `docs/agent-team/acceptance.json` — D6-AC06/S19 evidence and bounded status.
- `docs/agent-team/delivery-register.json`, `docs/ROADMAP.md` — D6/D6.6 status
  projection and evidence.
- `docs/handoffs/index.json` — current handoff ledger entry.

## Decisions and constraints

- The host protocol boundary owns the exact CLI validation; no UI/application
  code is responsible for filtering unknown host flags.
- The incoming `Sequence[str]` is normalized once before the pure predicate,
  preserving the normal `sys.argv` list and direct tuple callers.
- Shared checkout writer: parent Architect only; no worktree or second writer.
- Runtime launch policy: prohibited by the active project no-launch boundary.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge` | PASS | Static compilation only |
| `uv run ruff check src/quillforge/plugins/host_process.py` | PASS | Static lint |
| Source argument rejection probe | PASS | `probe-only`, unknown-tail, and duplicate-probe all returned `2`; valid exchange intentionally not invoked |
| `scripts/verify_handoff.ps1` | PASS | Handoff index and Markdown status agree |
| `scripts/check.ps1` | PASS | Formatting, static, JSON, acceptance, and project checks pass |
| `scripts/package.ps1` | PASS | Root/dist Windows x64 candidate rebuilt |
| Package identity | PASS | Root/dist SHA-256 `31BB45C6F69D5FED2F1B63FBCA5990956AAD796225C6C809C872221ACF79C78F`; both 38,363,243 bytes; source revision `tree-sha256:5c7895008d0fa4244320ba60648d7adfa54997fedc6f4fbae7d6ea63ad168097` |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Current dossier matches the candidate; three stale runtime-report failures and ten external release gates remain |

## Unrun checks and reason

- Packaged host exchange, executable startup, QApplication, visual review,
  interactive UI, clean-machine, cross-machine, pressure, and hard-power checks
  — prohibited or unavailable under current instructions.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run by project policy.

## Known risks and limits

- The host remains diagnostic-only with `execution_enabled=false`; exact argv
  validation is not a security sandbox.
- Fresh package/runtime evidence is still required before claiming operational
  release readiness.
- D7 scale/search and D8 signing, legal, installer, clean-machine, support, and
  runtime gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D6-AC06`, `S19`
- Evidence: `docs/agent-team/reviews/D6.6-exact-probe-argv-parent-review.md`,
  `src/quillforge/plugins/host_process.py`, source argument rejection probe,
  `scripts/verify_handoff.ps1`, `scripts/check.ps1`, and the rebuilt package
  identity recorded below.

## Next owner and next action

- Owner: Architect.
- Action: run the handoff/project/package gates, then continue with the next
  evidence-backed D7 or D8 slice without launching the application.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `31BB45C6F69D5FED2F1B63FBCA5990956AAD796225C6C809C872221ACF79C78F` / `38,363,243` bytes; root/dist match.
- Packaging note: rebuilt portable one-file candidate; unsigned/manual-update/no-installer posture unchanged.

## Disposition

`accepted-with-limits`: the exact probe-only source boundary is implemented and
independently reviewed. D6-AC06/S19 remain limited by prohibited fresh runtime
and packaged-exchange evidence; the parent project is not complete while D7/D8
and release gates remain open.
