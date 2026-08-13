# ADR 0017: Persistent plugin enablement policy

- **Status:** accepted with limits for D6.5; local policy is not trust
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

An operator's explicit enable/disable choice for a registered in-process
plugin survives a restart, while an absent policy keeps the current trusted
built-in default. A malformed or oversized policy never causes a plugin to
activate and cannot be silently overwritten by a UI toggle.

## Decision

1. `PluginEnablementPolicy` is an application boundary with two operations:
   resolve one plugin's desired state at registration and persist one explicit
   state after a lifecycle intent. It depends only on the
   `PluginEnablementStore` port.
2. The user-local JSON ledger is versioned, bounded to 256 records/64 KiB,
   validates plugin IDs and boolean states, rejects duplicate IDs, and uses
   same-directory temporary write plus flush/fsync/replace.
3. A missing ledger has no override: an explicitly registered trusted plugin
   defaults enabled. A corrupt, unreadable, or oversized ledger returns a
   policy error and resolves every plugin disabled. Mutation refuses to
   overwrite a ledger that cannot be read, preserving fail-closed behavior.
4. `PluginManager` asks the policy for the initial enabled state at
   registration and persists only through the application policy when the
   runtime control protocol toggles a trusted plugin. Lifecycle ownership,
   command/event cleanup, and failure isolation remain in `PluginManager`.
5. The enablement ledger is separate from descriptor approval. It applies only
   to explicit in-process registrations and cannot make an external catalog
   entry trusted, loadable, or executable.

## Consequences

- Restart behavior is deterministic without coupling the manager to a JSON
  path or filesystem adapter.
- Corruption can leave a trusted built-in inactive until the operator repairs
  or removes the local policy file outside the UI; this is intentional safety
  behavior and must be surfaced in Plugin Status diagnostics.
- The policy records a local operator preference, not publisher trust,
  signature validity, installation provenance, or enterprise distribution.

## Out of scope

- external module discovery/loading;
- signatures, publisher identity, remote policy, installation, updates,
  sandboxing, or process isolation;
- automatic repair or deletion of a corrupt policy file.

## Verification

- architecture boundary, format, lint, lock, and compile checks;
- source smoke for absent-default, disable/restart persistence,
  enable/restart persistence, corrupt/oversized fail-closed resolution, and
  mutation refusal;
- Qt offscreen smoke for persistence-backed status and lifecycle controls;
- package/startup evidence after composition-root changes.
