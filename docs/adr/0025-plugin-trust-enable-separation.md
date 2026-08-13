# ADR 0025: Keep plugin trust immutable across runtime enablement

- **Status:** implementation in progress for D6.4.1
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

`PluginManager` accepts an explicit trust decision when a plugin is
registered. Runtime enablement is a separate lifecycle preference and must
not become a trust-granting operation. A manager method that changed
`trusted=False` to `trusted=True` before activation would create a bypass
around catalog, approval, and future publisher-identity decisions.

## Decision

1. The application-facing `PluginRuntime.set_enabled()` is the only public
   lifecycle enablement boundary. Its manager implementation validates the
   registered ID and trust before persisting any intent.
2. The manager's private enable primitive runs only after policy persistence
   succeeds; there is no public enable method that can bypass a corrupt or
   oversized policy. Runtime enablement changes only `enabled` and lifecycle
   state; it never mutates `trusted`.
3. The registration-time manifest and permission tuple stored in
   `PluginStatus` remain authoritative across activate, deactivate, failure,
   and status projection. A plugin cannot rebind its `manifest` attribute to
   change its identity or capabilities after registration.
4. External catalog entries remain outside the registry, untrusted, unloaded,
   and non-executable. Trust establishment, approval, signatures, code
   identity, and any future external execution decision remain separate
   versioned boundaries.

## Consequences

- A direct or future internal caller cannot convert a registration-time
  untrusted state into runtime trust or bypass a failed local policy by
  invoking enablement.
- Trusted built-ins retain the existing enable/disable and cleanup behavior.
- Unknown IDs fail before policy persistence, so rejected lifecycle requests do
  not create orphan ledger records.
- Lifecycle diagnostics remain bound to the validated registration manifest,
  even if plugin code attempts to rebind its public attribute.
- The current in-process lifecycle remains a synchronous control surface, not
  a security sandbox or a trust store.

## Verification and limits

- Source smoke must prove policy fail-closed behavior, untrusted rejection,
  unknown-ID no-side-effect rejection, manifest authority across lifecycle
  transitions, and trusted disable/re-enable cleanup.
- `scripts/check.ps1` and the normal Qt/package evidence remain applicable.
- This decision does not add signatures, dynamic loading, installation,
  sandboxing, or external execution, and does not close the independent-review
  or clean-machine release gates.
