# Handoff: 2026-08-09-d6-3-approval-ledger-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-3-approval-ledger-review` |
| Delivery / slice | `D6 / D6.3 independent approval-ledger review closure` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:24:02+08:00` |

## User outcome

The extension approval ledger has an independent source review and a complete
handoff record. Approval remains explicit local governance metadata bound to a
descriptor digest; it does not grant trust or execution.

## Scope and boundaries

### In scope

- Canonical descriptor digest and stale-state review.
- Bounded, atomic, corruption-fail-closed ledger behavior.
- Async approval/revocation projection and delivery traceability.

### Out of scope

- Signatures, publisher identity, dynamic loading, sandboxing, installation,
  updates, or external execution.
- Runtime launch, visual review, unit tests, mocks, fixtures, harnesses, and
  test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate independent review and final disposition |
| Project Manager | Parent role record | D6 governance scope and risk tracking |
| Product | Parent role record | Explicit operator approval outcome |
| Developer 1 | Parent role record | Digest/approval application policy |
| Developer 2 | Parent role record | Bounded persistence and async UI evidence |
| QA | Helmholtz / Luna, read-only | Independent accepted-with-limits source review |

## Changed files and modules

- `docs/agent-team/reviews/D6.3-independent-luna-follow-up.md` — record the
  independent source review.
- `docs/handoffs/2026-08-09-d6-3-approval-ledger-review/handoff.md` — add the
  required material-slice handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`,
  `docs/agent-team/delivery-register.json`, and `docs/ROADMAP.md` — synchronize
  D6.3 evidence.

## Decisions and constraints

- Approval is keyed by the canonical descriptor digest and cannot change
  `trust_state=untrusted` or `loadable=false`.
- Corrupt/unavailable policy never silently grants approval.
- Shared-checkout writer: Architect only; the Luna reviewer was read-only.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `Helmholtz / Luna independent source review` | `ACCEPTED WITH LIMITS` | Direct source review supports bounded governance behavior; no files changed. |
| Historical D6.3 approval-ledger source smoke | `RECORDED` | Round trip, changed-digest stale, revoke, corruption fail-closed, mutation refusal, atomic cleanup, and byte-bound cases were previously reported. |
| Historical D6.3 Qt offscreen projection smoke | `RECORDED` | Async scan, approval, re-projection, revoke, and re-projection were previously reported. |
| `scripts/verify_handoff.ps1` | `PENDING` | Run after indexing this handoff. |
| `scripts/check.ps1` | `PENDING` | Required static/format/compile/acceptance gate. |

## Unrun checks and reason

- QuillForge.exe startup, live approval dialog, clean-machine, cross-machine,
  power-loss, and visual review — prohibited or unavailable under the current
  no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under the project policy.
- Signature and publisher-trust review — separate external security/release
  gate.

## Known risks and limits

- An operator able to replace both descriptor and local ledger can change the
  governance result; this is not a cryptographic trust boundary.
- Atomic replacement is not a hard-power durability or cross-machine proof.
- D6.4–D6.8 and D8 security/release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D6-AC03`, `S15`
- Evidence: `src/quillforge/application/plugin_governance.py`,
  `src/quillforge/infrastructure/plugin_approval_store.py`,
  `src/quillforge/presentation/main_window.py`,
  `docs/agent-team/reviews/D6.3-parent-review.md`,
  `docs/agent-team/reviews/D6.3-independent-luna-follow-up.md`, and the
  current package identity `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4`.

## Next owner and next action

- Owner: Architect
- Action: continue D6.4 and D6.5 independent-review handoff closure.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Packaging note: documentation-only closure; no package input changed.

## Disposition

`accepted-with-limits`: D6.3 governance behavior and independent review are
recorded; approval is not trust and external/runtime gates remain open.
