# QuillForge agent team operating model

This file is the project playbook for Codex. The repository root `AGENTS.md` makes it mandatory; `.codex/agents/` contains the role definitions. The machine-readable source of truth is [`workflow-policy.json`](workflow-policy.json), and every material transfer is recorded under [`docs/handoffs/`](../handoffs/).

## Control model

The parent conversation is the Architect. It is the only role allowed to make the final scope, architecture, integration, verification, and release decision. Subagents provide bounded evidence and recommendations. They do not form an independent project, create a worktree, or override repository constraints.

The mandatory team is always:

| Role | Primary responsibility | Default output |
|---|---|---|
| Architect (parent) | Scope, architecture, integration, final review, handoff | Decision record and completed evidence matrix |
| Project Manager | Plan, sequencing, dependencies, risks, status | Milestone/risk/evidence brief |
| Product | User outcome, scope, acceptance, recovery | Acceptance note |
| Developer 1 | Domain, application, infrastructure, plugin contracts | Bounded implementation proposal or assigned patch |
| Developer 2 | Presentation, integration, packaging | Bounded implementation proposal or assigned patch |
| Test / QA | Read-only checks, smoke evidence, release gate | Verification report with unrun items |

## Required task loop

1. **Orient:** Architect reads `AGENTS.md`, relevant docs, current tree, `acceptance.json`, and `workflow-policy.json`.
2. **Frame:** Project Manager and Product return scope, milestones, risks, and acceptance criteria. Architect records any material decision in an ADR or change note.
3. **Design review:** Architect delegates a bounded read-only architecture review after the design is written.
4. **Build:** Architect assigns non-overlapping slices. Only one shared-checkout writer may be active; the parent integrates all changes.
5. **Review:** After source changes, Architect delegates code review and behavior-preserving simplification review. Escalation follows the Luna → Terra → Sol ladder only when necessary.
6. **Verify:** QA runs the applicable deterministic checks and packaging checks. Startup and visual smoke checks are intentionally unrun under the current user instruction; unit-test assets are not created or run by default.
7. **Handoff:** Architect creates `docs/handoffs/<handoff-id>/handoff.md`, updates the handoff index and delivery register, then runs the handoff verifier and the full project check.

## Handoff contract

Every role returns:

- result: `pass`, `revise`, `blocked`, or `escalate`;
- exact files, symbols, commands, and observations;
- assumptions and unresolved risks;
- acceptance IDs affected;
- recommended next owner/action.

The Architect records those results in the final response or the relevant ADR. “Looks good” without evidence is not a completed gate.

The complete handoff structure is defined in [`../handoffs/HANDOFF_TEMPLATE.md`](../handoffs/HANDOFF_TEMPLATE.md). The ledger is [`../handoffs/index.json`](../handoffs/index.json); do not treat `docs/RELEASE_HANDOFF.md` as a replacement for the per-slice record.

## Safety and concurrency

- Default subagent model: `gpt-5.6-luna`, max reasoning, Fast service tier.
- `gpt-5.6-terra` is reserved for complex call chains, cross-module tradeoffs, concurrency/safety analysis, or high-risk unresolved reviews.
- `gpt-5.6-sol` is reserved for the hardest unresolved system-wide or adversarial review after Terra evidence is insufficient.
- All child agents are read-only by default. A developer write slot must be explicitly assigned, sequential, and limited to an exact file set.
- No Git/Codex worktrees, flashing hardware, destructive operations, network/telemetry additions, untrusted plugin execution, or software/Qt launch are implicit in a task. Software launch requires an explicit user reversal of the current no-launch instruction.
