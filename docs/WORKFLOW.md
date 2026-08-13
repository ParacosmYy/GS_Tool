# QuillForge development workflow

Every material change follows the smallest applicable sequence below. The workflow is designed to keep the editor extensible without turning process into bureaucracy.

## Gate 0 — orient

- Read `AGENTS.md`, `docs/agent-team/workflow-policy.json`, the relevant documents under `docs/`, and the current handoff index.
- Inspect the current tree and Git status.
- Identify the user-visible behavior, affected layer, dependencies, and risk.
- Do not create a worktree or silently overwrite existing work.
- Keep the fixed six-role team visible in the handoff, even when a role returns `not-applicable`.

## Gate 1 — define

Write a short acceptance note in the change description or an ADR when the change affects architecture, compatibility, security, or licensing. Include:

- user outcome;
- in-scope and out-of-scope behavior;
- compatibility and performance expectations;
- recovery and failure behavior;
- verification evidence that will be collected.

## Gate 2 — design

- Keep public contracts in `domain`, `application`, or `plugins` rather than in widgets.
- Add or update an ADR for a new framework, dependency, persistence model, plugin capability, or threading model.
- Define cancellation and error propagation for long-running operations.
- Confirm third-party license and packaging implications.

## Gate 3 — implement

- Make the smallest complete change.
- Keep UI, application, domain, infrastructure, and plugin code in their designated layers.
- Use `uv add` or `uv remove` for dependency changes; commit the resulting `uv.lock`.
- Keep generated build output out of source control.

## Gate 4 — review and simplify

- Review the diff for accidental coupling, duplicated state, blocking UI work, unsafe file replacement, and unbounded resource use.
- Remove speculative abstractions that do not protect a documented extension point.
- Check that names, commands, events, and plugin APIs are stable and discoverable.
- Update documentation when behavior or constraints changed.

## Gate 5 — verify

Run the applicable checks:

```powershell
.\scripts\verify_handoff.ps1
.\scripts\check.ps1
```

For packaging or release changes:

```powershell
.\scripts\package.ps1
```

Record the command, result, artifact path, and any unrun verification. A successful static check is not evidence that large-file behavior or packaging on another architecture has been validated.

Under the current user instruction, do not start `QuillForge.exe`, open a Qt
window, or run interactive startup/visual checks. Mark those checks as
user-owned and intentionally unrun in the handoff. Static checks, compilation,
and packaging remain allowed.

## Gate 6 — handoff and release

- Create `docs/handoffs/<handoff-id>/handoff.md` from `docs/handoffs/HANDOFF_TEMPLATE.md`.
- Update `docs/handoffs/index.json` and `docs/agent-team/delivery-register.json`.
- Record the next owner/action, known limits, actual evidence, and every unrun check.
- Run `scripts/verify_handoff.ps1` before returning control to the user.

- Inspect the actual EXE or distribution directory.
- Confirm version, architecture, icon, included resources, and license notices.
- Check startup, open/save, encoding, recovery, and clean shutdown on a clean Windows environment when available.
- Follow `docs/RELEASE.md`.

## Codex review hooks

The review hooks described in [`agents/CODEX_HOOKS.md`](agents/CODEX_HOOKS.md) are workflow triggers executed by the Codex parent agent. They call read-only Codex subagents for architecture review, code review, simplification review, and release review. They are not Git hooks and are not expected to run from a local shell.

The local `scripts/check.ps1` command remains a deterministic project verification command. It does not replace the Codex review hooks, and it must not be interpreted as a software startup or visual acceptance check.
