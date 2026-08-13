# Handoff: 2026-08-09-d6-5-enablement-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-5-enablement-review` |
| Delivery / slice | `D6 / D6.5 independent persistent-enablement review closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:25:46+08:00` |

## User outcome

Trusted, explicitly registered in-process plugins retain bounded local
enablement choices across startup, while corrupt or oversized policy fails
closed and cannot be silently overwritten by a UI mutation.

## Scope and boundaries

### In scope

- Application/store separation for enablement policy.
- Absent-default, persistence round-trip, corruption/size failure, and
  mutation-refusal review.
- D6.5 independent review and delivery traceability.

### Out of scope

- External catalog loading, signatures, sandboxing, process isolation,
  installation, updates, or runtime launch.
- Visual/clean-machine/cross-machine review, unit tests, mocks, fixtures,
  harnesses, and test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate review and final disposition |
| Project Manager | Parent role record | Local policy dependency and risk tracking |
| Product | Parent role record | Restart persistence and fail-closed user outcome |
| Developer 1 | Parent role record | Application enablement policy boundary |
| Developer 2 | Parent role record | Atomic store and UI evidence synchronization |
| QA | Helmholtz / Luna, read-only | Independent accepted-with-limits source review |

## Changed files and modules

- `docs/agent-team/reviews/D6.5-independent-luna-follow-up.md` — record the
  independent source review.
- `docs/handoffs/2026-08-09-d6-5-enablement-review/handoff.md` — add the
  material-slice handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, and `docs/ROADMAP.md` — synchronize
  D6.5 evidence.

## Decisions and constraints

- Enablement is a local operator preference and cannot promote trust or admit a
  catalog descriptor into the runtime registry.
- Policy failure disables activation and blocks mutation rather than replacing
  the evidence with a clean default.
- Shared-checkout writer: Architect only; the Luna reviewer was read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `Helmholtz / Luna independent source review` | `ACCEPTED WITH LIMITS` | Direct source evidence supports bounded local policy; no files changed. |
| Historical D6.5 enablement source smoke | `RECORDED` | Absent default, disable/enable restart restoration, corruption/oversize fail-closed, mutation refusal, and no activation after policy failure were previously reported. |
| Historical D6.5 Qt offscreen persistence smoke | `RECORDED` | UI disable/enable, ledger write, fresh-window restoration, and command restoration were previously reported. |
| `scripts/verify_handoff.ps1` | `PENDING` | Run after indexing this handoff. |
| `scripts/check.ps1` | `PENDING` | Required static/format/compile/acceptance gate. |

## Unrun checks and reason

- QuillForge.exe launch, live Plugin Status, visual review, clean-machine,
  cross-machine, and power-loss durability — prohibited or unavailable under
  the current no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Signatures, remote policy, sandboxing, installation, and updates — future
  security/release gates.

## Known risks and limits

- A user who can replace a valid local policy can change the next startup
  decision; this is not a security boundary.
- Activation remains synchronous and in-process; untrusted external code must
  not reuse this path.
- D6.6–D6.8 and D8 release/security gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D6-AC05`, `S18`
- Evidence: `src/quillforge/application/plugin_enablement.py`,
  `src/quillforge/infrastructure/plugin_enablement_store.py`,
  `src/quillforge/plugins/manager.py`,
  `docs/agent-team/reviews/D6.5-parent-review.md`,
  `docs/agent-team/reviews/D6.5-independent-luna-follow-up.md`, and current
  package identity `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4`.

## Next owner and next action

- Owner: Architect
- Action: audit D6.6–D6.8 host protocol, containment, and deny-by-default
  execution gates with the required higher-risk review routing.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Packaging note: documentation-only closure; no package input changed.

## Disposition

`accepted-with-limits`: D6.5 local persistence and independent review are
complete within the registered in-process boundary; external trust/security
and release gates remain open.
