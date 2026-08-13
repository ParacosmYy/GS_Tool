# Handoff: 2026-08-09-d6-8-execution-gate-review

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-8-execution-gate-review` |
| Delivery / slice | `D6 / D6.8 deny-by-default external execution gate review` |
| Status | `in-progress` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T21:32:43+08:00` |

## User outcome

External catalog evidence is evaluated through one immutable, deterministic
policy and remains denied by default. The catalog can show the primary reason
and every failed prerequisite without importing, loading, or executing an
external entry.

## Scope and boundaries

### In scope

- Typed evidence/decision contract and stable deny-reason ordering.
- Global `external_execution_enabled=false` policy.
- Catalog projection of approval/stale/trust/containment/executor failures.
- D6.8 handoff/index/register traceability.

### Out of scope

- Signature verification, code identity, dynamic loading, executor lifecycle,
  sandboxing, installation, updates, or external execution.
- Runtime launch, visual review, unit tests, mocks, fixtures, harnesses, and
  test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate gate evidence and preserve fail-closed policy |
| Project Manager | Parent role record | Security/release dependency tracking |
| Product | Parent role record | Explainable denial outcome in catalog |
| Developer 1 | Parent role record | Application execution policy/evidence contract |
| Developer 2 | Parent role record | Catalog and Qt diagnostic projection |
| QA | Parent record; independent reviews unavailable | No child PASS claimed for this current slice |

## Changed files and modules

- `docs/handoffs/2026-08-09-d6-8-execution-gate-review/handoff.md` — add the
  missing main D6.8 handoff.
- `docs/handoffs/index.json`, `docs/agent-team/acceptance.json`, and
  `docs/agent-team/delivery-register.json` — link D6.8 evidence.

## Decisions and constraints

- `PluginExecutionGate` is pure application policy; it has no loader, process,
  Qt, or ledger mutation path.
- A fully evidenced hypothetical decision cannot execute through this slice;
  production composition keeps the global policy disabled.
- Shared-checkout writer: Architect only; no child writer was used.
- Runtime launch policy: forbidden by the current project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Historical D6.8 gate source smoke | `RECORDED` | Global-disabled denial, malformed evidence fail-closed, frozen decision, deterministic requirement order, and each deny reason were previously reported. |
| Historical D6.8 catalog projection smoke | `RECORDED` | Approval/stale/duplicate catalog entries remained denied with complete requirements. |
| Historical D6.8 Qt offscreen diagnostic smoke | `RECORDED` | Denial reason and failed requirements were previously visible; no entry was executed. |
| Current independent high-risk review | `NO RESULT` | Terra was requested, returned no conclusion in bounded waits, and was closed; no child PASS is claimed. |
| `scripts/verify_handoff.ps1` | `PENDING` | Run after indexing this handoff. |
| `scripts/check.ps1` | `PENDING` | Required static/format/compile/acceptance gate. |

## Unrun checks and reason

- QuillForge.exe launch, external execution, packaged probe, startup, and
  visual review — prohibited by the project no-launch instruction.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under project policy.
- Signature, publisher, code identity, executor, sandbox, installation, and
  update review — future authorized security/release gates.

## Known risks and limits

- D6.8 remains `in-progress`; explainable denial is not executable-plugin
  authorization or a security certification.
- Enabling the global policy in a future change requires independent evidence
  for signatures, code identity, permissions, containment, executor lifecycle,
  installation/update provenance, and adversarial security review.
- Clean-machine, cross-machine, permission-pressure, hard-power, and D8
  release gates remain open.

## Acceptance and evidence IDs

- Acceptance: `D6-AC08`, `S21`
- Evidence: `src/quillforge/application/plugin_execution.py`,
  `src/quillforge/application/plugin_catalog.py`,
  `src/quillforge/plugins/catalog.py`,
  `docs/agent-team/reviews/D6.8-parent-review.md`, and this handoff.

## Next owner and next action

- Owner: Architect
- Action: retain global denial and obtain an authorized independent security
  review before any external execution decision can be promoted.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7702EE52B51664B179551EE56B08736849D0B673C368E33FE8306FE9289592B4` / `38,327,064`
- Packaging note: documentation-only handoff; no package input changed.

## Disposition

`in-progress`: the deny-by-default policy and diagnostic projection are
traceable, but independent security review and all future execution
prerequisites remain open.
