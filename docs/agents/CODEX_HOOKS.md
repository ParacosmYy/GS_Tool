# Codex subagent review hooks

## Purpose

These hooks define when the parent Codex agent must delegate bounded review work to a child agent. They are orchestration rules, not executable Git hooks. The Codex runtime invokes the subagent tool; the repository stores the trigger, scope, prompt, and evidence contract so the process is repeatable.

The fixed project roster and no-launch/handoff rules are canonical in
[`docs/agent-team/workflow-policy.json`](../agent-team/workflow-policy.json).
Every hook result must be carried into the current
[`handoff.md`](../handoffs/HANDOFF_TEMPLATE.md) record.

## Routing policy

Use this ladder for every delegated review:

1. `luna_max` — default reviewer: gpt-5.6-luna, max reasoning, Fast service tier.
2. `terra_max` — only for complex call-chain tracing, cross-module design tradeoffs, concurrency/safety analysis, or high-risk review Luna cannot resolve reliably.
3. `sol_medium` — only for the hardest unresolved system-wide, safety-sensitive, or final adversarial review after Terra evidence is insufficient.

Do not select another model or reasoning level for these hooks.

## Hook triggers

### `after-design`

Trigger after a new feature design, dependency choice, public API, threading model, persistence model, or plugin capability is documented.

Delegated scope:

- Read `AGENTS.md` and the relevant `docs/` files.
- Inspect the proposed dependency direction and extension boundary.
- Identify missing constraints, accidental coupling, and unresolved license/security concerns.
- Do not edit files.

Expected result: architecture findings with severity, evidence, and a recommendation to proceed, revise, or escalate.

### `after-source-change`

Trigger after a qualifying source change or a non-trivial multi-file refactor.

Delegated scope:

- Review the changed files and their immediate call chain.
- Check behavior preservation, error propagation, UI-thread blocking, resource ownership, and observability.
- Perform an independent behavior-preserving simplification assessment.
- Do not edit files or create tests, mocks, fixtures, or worktrees.

Expected result: actionable findings, simplified alternatives where safe, verification gaps, and unresolved risks.

### `before-release`

Trigger before claiming a packaged release or handing off a distribution artifact.

Delegated scope:

- Review packaging configuration, architecture target, resource inclusion, versioning, and third-party notices.
- Compare claimed verification with actual evidence.
- Check that one-file packaging is not being mistaken for an installer or signed release.
- Do not edit files.

Expected result: release gate decision, artifact evidence required, and explicit unrun checks. This hook must not start QuillForge or open a Qt window while the project no-launch policy is active.

## Parent-agent contract

The parent agent must:

1. Decide the immediate local task before delegating.
2. Give each child a bounded, non-overlapping, read-only scope.
3. Include the exact repository path and changed-file set in the prompt.
4. Keep the parent responsible for integration, final diff review, and verification.
5. Escalate only when the routing conditions above are met.
6. Preserve the child result as an evidence note in the final handoff or the relevant ADR.
7. Create/update the indexed `docs/handoffs/<handoff-id>/handoff.md` record before returning control to the user.

## Standard prompt

```text
You are a read-only QuillForge reviewer.

Role: <architecture | code | simplification | release>
Repository: D:\Workplace\Agent_Workplace\QuillForge
Scope: <exact files or question>
Changed files: <list>

Read AGENTS.md and the relevant docs first. Do not edit files, create tests,
create a worktree, or revert other changes. Return:
1. findings ordered by severity;
2. exact file and symbol evidence;
3. assumptions;
4. unresolved risks;
5. required verification;
6. whether escalation is needed.
```

## Evidence format

Record each review as:

```text
Hook: <after-design | after-source-change | before-release>
Reviewer: <luna_max | terra_max | sol_medium>
Scope: <files and question>
Result: <pass | revise | escalate>
Evidence: <commands, files, symbols, or observations>
Findings: <severity and details>
Unresolved risks: <none or list>
Parent action: <integrated, deferred, or escalated>
```
