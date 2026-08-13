# ADR 0026: Preserve invalid session manifests until explicit repair

- **Status:** implementation in progress for D7.5.1 follow-up
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The session store previously returned the same `None` value for an absent
manifest and a malformed, oversized, or unsupported manifest. Startup then
used an empty safe snapshot and could queue that snapshot during normal
startup/close, replacing the original bytes before the operator had a chance
to diagnose or recover them.

## Decision

1. The `SessionStore` port returns a typed `SessionLoadResult` with `absent`,
   `valid`, or `invalid` state. A valid result carries a normalized candidate;
   absent and invalid results carry no snapshot.
2. `JsonSessionStore` maps file-not-found to `absent` and all bounded decode,
   schema, I/O, encoding, and size failures to `invalid` without writing.
3. `SessionService` normalizes only a valid candidate and otherwise returns a
   safe default with the original load state preserved.
4. `MainWindow` seeds its last-saved baseline from the load result. Startup
   and close do not rewrite an invalid manifest when no session change has
   occurred; a later explicit session change may atomically repair it.

## Consequences and limits

- Corrupt or unknown session bytes remain available for diagnosis and manual
  recovery instead of being silently replaced by an empty manifest.
- Missing files remain a normal first-run state and do not cause a needless
  startup write.
- This does not provide multi-instance locking, hard-power durability,
  encryption, or a backup/repair UI; those remain outside D7.5.1.
