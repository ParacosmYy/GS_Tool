# Handoff: 2026-08-09-d6-7-containment-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-7-containment-review` |
| Delivery / slice | `D6 / D6.7 plugin-host lifecycle and resource containment review` |
| Status | `in-progress` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:32:01+08:00` |

## User outcome

The diagnostic host containment contract and its creation-before-resume review
are traceable: the default Windows path assigns a suspended child to a Job
Object before resuming it, while legacy attach-only compatibility is labeled
separately and external execution remains disabled.

## Scope and boundaries

### In scope

- Job Object limit/cleanup contract, creation-before-resume ordering, failure
  mapping, unsupported fallback, and attach-only state distinction.
- Existing Terra `REVISE` findings and parent source-hardening disposition.
- D6.7 handoff/index/register traceability.

### Out of scope

- Complete security sandbox, restricted token, AppContainer, filesystem/network
  isolation, signatures, dynamic loading, installation, updates, or external
  execution.
- Fresh Windows process/Qt/package smoke, visual review, unit tests, mocks,
  fixtures, harnesses, and test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate source review and retain risk limits |
| Project Manager | Parent role record | Host lifecycle and release risk |
| Product | Parent role record | Diagnostic containment outcome |
| Developer 1 | Parent role record | Application/infrastructure containment seam |
| Developer 2 | Parent role record | Host projection and cleanup evidence |
| QA | Parent record; Terra historical review | Review prior hardening; no current PASS claimed |

## Changed files and modules

- `docs/handoffs/2026-08-09-d6-7-containment-review/handoff.md` — add the
  missing D6.7 main handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`, and
  `docs/agent-team/delivery-register.json` — link D6.7 evidence.

## Decisions and constraints

- Only the creation-before-resume `attached` state is eligible for any future
  execution design; `attached-after-start` is compatibility telemetry.
- Job Object limits are lifecycle/resource controls, not a security boundary or
  certification claim.
- Shared-checkout writer: Architect only; reviewers are read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Historical Terra/Mencius high-risk review | `REVISE→ADDRESSED` | Return-value, pointer-width sentinel, and attach-state issues were addressed in the parent source contract. |
| Historical D6.7 creation-before-resume source smoke | `RECORDED` | Suspended launch, assigned Job Object, bounded labels, PID, and disabled execution were previously reported. |
| Historical D6.7 cleanup/failure smoke | `RECORDED` | Lease close, attach failure, unsupported fallback, timeout/crash cleanup were previously reported. |
| Current fresh high-risk/runtime verification | `NOT RUN` | No new child conclusion or software launch is authorized. |
| `scripts/verify_handoff.ps1` | `PENDING` | Run after indexing this handoff. |
| `scripts/check.ps1` | `PENDING` | Required static/format/compile/acceptance gate. |

## Unrun checks and reason

- Fresh Windows process/Qt/package containment smoke, startup, and visual
  review — prohibited by the project no-launch instruction.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under project policy.
- Clean-machine, cross-machine, permission-pressure, hard-power, and nested
  Job Object behavior — require authorized environment evidence.

## Known risks and limits

- D6.7 remains `in-progress` until fresh permitted runtime evidence and the
  high-risk review closure are available.
- Attach-only adapters are intentionally retained for compatibility and cannot
  serve as a creation-time external execution boundary.
- A Job Object does not establish token, code identity, signature, secret,
  filesystem, or network isolation.

## Acceptance and evidence IDs

- Acceptance: `D6-AC07`, `S20`, `S21`
- Evidence: `src/quillforge/infrastructure/process_containment.py`,
  `src/quillforge/infrastructure/plugin_host.py`,
  `docs/adr/0019-plugin-host-lifecycle-containment.md`,
  `docs/adr/0028-plugin-host-creation-before-resume-containment.md`,
  `docs/agent-team/reviews/D6.7-creation-order-follow-up-parent-review.md`,
  and this handoff.

## Next owner and next action

- Owner: Architect
- Action: keep the default path fail-closed and obtain authorized fresh
  containment evidence before D6.7 promotion.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Packaging note: documentation-only handoff; no package input changed.

## Disposition

`in-progress`: the containment source contract and prior hardening evidence are
traceable, but fresh runtime/high-risk closure remains open.
