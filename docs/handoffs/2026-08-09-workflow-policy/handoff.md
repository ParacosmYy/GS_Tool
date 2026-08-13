# Handoff: 2026-08-09-workflow-policy

| Field | Value |
|---|---|
| ID | `2026-08-09-workflow-policy` |
| Delivery / slice | `D10 / GOV-01` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T00:00:00+08:00` |

## User outcome

QuillForge now has a repository-enforced six-role team workflow: Architect,
Project Manager, Product, Developer 1, Developer 2, and QA. Every material
slice must produce an indexed `handoff.md` record with evidence, limits, and a
next owner/action.

## Scope and boundaries

### In scope

- Canonical machine-readable routing, checkout, verification, no-launch, and handoff policy.
- Root project instructions, handoff template/index, current handoff record, and static verifier.
- Acceptance and delivery-register traceability for the governance contract.

### Out of scope

- UI-06 or other product source changes.
- Starting QuillForge, opening a Qt window, or performing runtime visual/startup acceptance.
- Rebuilding or replacing the existing EXE because this slice changes only policy, docs, and checks.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integration, final review, verification, and handoff decision |
| Project Manager | Required project role | Plan, dependencies, risks, and status |
| Product | Required project role | User outcome and acceptance |
| Developer 1 | Required project role | Domain/application/infrastructure slice |
| Developer 2 | Required project role | Presentation/integration/packaging slice |
| QA | Required project role | Read-only verification and explicit unrun evidence |

This governance slice is integrated by the Architect in the shared checkout;
no child write slot was used and no child result is being represented as an
executed review. The fixed roster and child routing rules are now repository
contracts for future slices.

## Changed files and modules

- `AGENTS.md` — makes the permanent roster, no-launch boundary, and handoff gate explicit.
- `docs/agent-team/workflow-policy.json` — machine-readable workflow source of truth.
- `docs/agent-team/TEAM.md`, `docs/WORKFLOW.md`, `docs/agents/CODEX_HOOKS.md`, `docs/CONSTRAINTS.md` — aligns human playbooks.
- `docs/handoffs/README.md`, `HANDOFF_TEMPLATE.md`, `index.json` — defines the handoff ledger.
- `scripts/verify_handoff.ps1`, `scripts/new_handoff.ps1`, `scripts/check.ps1` — adds static enforcement and scaffolding.
- `docs/agent-team/acceptance.json`, `delivery-register.json`, `docs/ROADMAP.md` — records D10/S30/D10-AC01.

## Decisions and constraints

- The Architect is always the parent and owns integration, final diff review, and verification.
- Child routing is Luna/max/Fast by default; Terra and Sol are escalation-only routes with explicit conditions.
- The shared local checkout is the only workspace and has at most one writer at a time.
- Unit-test assets, mocks, fixtures, and harnesses are not created or run by default.
- QuillForge.exe and Qt windows must not be started unless the user explicitly reverses the current instruction. Static checks, compilation, and packaging remain allowed.
- The exact handoff filename is `docs/handoffs/<handoff-id>/handoff.md`; the index and delivery register must be updated together.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `.\scripts\verify_handoff.ps1` | `PASS` | Policy, index, headings, and indexed file coverage pass. |
| `.\scripts\check.ps1` | `PASS` | Acceptance, architecture boundaries, lock, formatting, lint, and compile checks pass. |
| Existing packaged artifact identity | `PASS / unchanged` | Root and `dist` EXE remain SHA-256 `B9151D4D3AAA25AA73827ABF033CC3C6F2022174C0A3BFAED1A904E768F3EAF7`, size `38,323,223` bytes. |

## Unrun checks and reason

- QuillForge.exe startup and Qt-window visual review — intentionally not run because the user prohibited starting the software; runtime acceptance remains user-owned.
- Package rebuild — not needed for docs/scripts-only governance changes; the existing artifact is recorded as unchanged.
- Unit tests or test-only assets — intentionally not created or run under the project policy.

## Known risks and limits

- The verifier enforces structure and repository traceability; it cannot judge the quality of a role's narrative evidence or external human approval.
- Runtime appearance, startup, DPI, clean-machine behavior, signing, installer, and support gates remain separate release evidence.

## Acceptance and evidence IDs

- Acceptance: `S30`, `D10-AC01`
- Evidence: `AGENTS.md`, `docs/agent-team/workflow-policy.json`, `docs/handoffs/index.json`, `scripts/verify_handoff.ps1`, `scripts/check.ps1`

## Next owner and next action

- Owner: Architect
- Action: Use `scripts/new_handoff.ps1` or the template for the next material slice, then update the index and delivery register before handoff.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`
- Version: `0.1.0` unchanged
- SHA-256 / size: `B9151D4D3AAA25AA73827ABF033CC3C6F2022174C0A3BFAED1A904E768F3EAF7` / `38,323,223` bytes
- Packaging note: Existing package unchanged; no software launch performed.

## Disposition

`accepted-with-limits`: the workflow and handoff contract are statically
enforced. Runtime and visual evidence remain intentionally open under the
user's no-launch instruction.
