# QuillForge handoff records

Every material slice, role transition, and release-oriented change must leave
one indexed Markdown record at:

```text
docs/handoffs/<handoff-id>/handoff.md
```

The contract is machine-checked by [`scripts/verify_handoff.ps1`](../../scripts/verify_handoff.ps1)
and is part of [`scripts/check.ps1`](../../scripts/check.ps1). The canonical
policy is [`docs/agent-team/workflow-policy.json`](../agent-team/workflow-policy.json).

## Required flow

1. Read the project root `AGENTS.md`, the team playbook, the acceptance contract,
   and the workflow policy before starting a material slice.
2. Use the fixed team roster: Architect, Project Manager, Product, Developer 1,
   Developer 2, and QA. The Architect remains the parent and integration owner.
3. Give each child agent a bounded, non-overlapping scope. Keep the shared
   checkout single-writer and use the configured Luna → Terra → Sol escalation
   ladder only when its conditions are met.
4. Create the handoff directory and copy the required headings from
   [`HANDOFF_TEMPLATE.md`](HANDOFF_TEMPLATE.md). The optional scaffold command
   is:

   ```powershell
   .\scripts\new_handoff.ps1 -HandoffId 2026-08-09-example -Delivery D10
   ```

5. Record actual files, symbols, commands, evidence, assumptions, unresolved
   risks, unrun checks, next owner, and artifact identity. Do not turn a static
   check into runtime proof.
6. Add the record to [`index.json`](index.json) and update
   `docs/agent-team/delivery-register.json` in the same handoff.
7. Run `.\scripts\verify_handoff.ps1` and `.\scripts\check.ps1` before handing back the slice.

## Runtime boundary

The current user instruction forbids starting QuillForge or opening Qt windows.
Static source checks, JSON validation, compilation, and packaging are allowed;
startup and visual acceptance remain explicitly unrun and user-owned until the
user reverses that instruction.

`docs/RELEASE_HANDOFF.md` remains the release dossier. The records in this
directory are the per-slice and per-role handoff ledger and are required even
when no packaged artifact changes.
