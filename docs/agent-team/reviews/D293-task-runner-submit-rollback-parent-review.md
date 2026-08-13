# D293 parent review — TaskRunner submission rollback boundary

## Scope

Reviewed `src/quillforge/presentation/task_runner.py`, all existing
`TaskRunner.submit` dispatch call sites, the worker observability contract in
`scripts/audit_presentation_contracts.py`, and the D292 shutdown limits.

## Findings

- PASS — task retention still happens before signal connection and pool start,
  preserving the existing UI-thread-owned pending and race boundary.
- PASS — connection/start failures release the retained task and re-raise the
  original exception, so existing coordinator error propagation is unchanged.
- PASS — normal completion and submission rollback share `_release_task()`;
  duplicate cleanup is a no-op and does not emit a second pending transition.
- PASS — the completion callback still runs before normal release, preserving
  close-guard and status-projection semantics.
- PASS — no forced wait, thread termination, quiescing flag, `BaseException`
  policy change, or coordinator dependency leak was introduced.
- PASS — the targeted AST/source contract requires the shared release boundary
  and the submission rollback branch.

## Simplification assessment

PASS. A single idempotent release helper is the smallest complete change. A
second task state machine, generic lifecycle service, or shutdown wait would
duplicate ownership or expand asynchronous behavior without addressing the
reported stale-retention failure more safely.

## Architecture consultation and limits

The required Luna/max architecture window was called for this slice but
returned no conclusion after three bounded 60-second waits and was closed. No
architecture PASS is claimed. The parent decision follows the already-recorded
D292 boundary and the direct source evidence. Native Qt timing, runtime reuse,
and real EXE startup remain unverified.

## Applicability

Python/PyQt6 desktop code only. No embedded public-vendor requirement applies;
no certification or private enterprise-standard claim is made.
