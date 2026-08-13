# ADR 0020: Deny-by-default external plugin execution gate

- **Status:** accepted with limits for D6.8; future execution prerequisites and security/release gates remain open
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

QuillForge reports one explicit, immutable decision when an external catalog
entry is considered for execution. The decision explains every missing or
invalid prerequisite and remains denied by default. Approval, local enablement,
a separate host process, or resource containment cannot accidentally turn a
metadata-only descriptor into executable code.

## Decision

1. Add an application-owned `PluginExecutionGate` with a pure evaluation
   function. It accepts a typed `PluginExecutionEvidence` snapshot and returns
   an immutable `PluginExecutionDecision`; it does not import modules, spawn
   processes, access widgets, or mutate ledgers.
2. The evidence contract separates the independent prerequisites:
   catalog validity, trust state, digest-bound approval, signature state, code
   identity, explicit enablement, permission grant, and host containment.
   Missing evidence is represented explicitly rather than inferred as success.
3. The current policy has `external_execution_enabled=false`. Therefore every
   external request is denied with the primary reason
   `external-execution-disabled`, while the complete failed-requirement tuple
   remains available for diagnostics. Even a hypothetical fully evidenced
   request cannot execute through this increment because no loader or executor
   is wired to the gate.
4. `PluginCatalogService` evaluates every catalog entry after approval
   decoration and projects `execution_state`/`execution_reason` into the
   immutable `PluginCatalogEntry`. The presentation layer shows the decision
   alongside trust, approval, and loadability; it receives no executable
   callback.
5. The gate is a policy seam, not a security boundary. A future change that
   enables external execution must separately provide verified signatures and
   provenance, code-identity binding, capability authorization, host
   containment, installation/update policy, and an independent adversarial
   security review before changing the policy threshold.

## Dependency direction and ownership

```text
PluginCatalogEntry ──> PluginExecutionGate ──> PluginExecutionDecision
       ▲                       │                         │
       │                       │                         └──> Qt diagnostic text
       └── PluginCatalogService┘
```

`application.plugin_execution` owns policy and immutable evidence. The
`plugins.catalog` data contract owns only the projected metadata fields.
`PluginCatalogService` orchestrates the two. Infrastructure stores remain
unaware of execution policy, and presentation renders strings only.

## Current deny reasons

- `invalid-evidence`
- `external-execution-disabled`
- `catalog-entry-invalid`
- `untrusted-plugin`
- `approval-missing`
- `approval-stale`
- `signature-not-valid`
- `code-identity-not-verified`
- `plugin-not-enabled`
- `permissions-not-granted`
- `host-containment-unavailable`
- `executor-unavailable`

The primary reason is deterministic; the decision also returns all applicable
requirements in stable order so support diagnostics do not hide a second
failure behind the first one.

## Out of scope

- dynamic import, Python execution, native module loading, or plugin callbacks;
- signature verification, publisher trust, code identity, installation,
  update, remote policy, or capability brokering;
- changing the existing in-process built-in plugin lifecycle;
- treating descriptor approval or `loadable` metadata as execution authority;
- a security, compliance, or certification claim.

## Verification

- source smoke for each deny reason, deterministic ordering, immutable result,
  and the global disabled policy;
- catalog source smoke proving valid/approved, stale, unapproved, malformed,
  and duplicate entries remain denied with explicit reasons;
- Qt offscreen catalog projection smoke proving the decision is visible while
  approval actions remain metadata-only;
- malformed evidence with an unhashable literal field returns the immutable
  `invalid-evidence` decision without raising;
- independent post-fix Luna source review confirms the fail-closed repair and
  deny-reason vocabulary alignment;
- `scripts/check.ps1`, package/startup evidence, and root/dist artifact
  synchronization after source changes.
