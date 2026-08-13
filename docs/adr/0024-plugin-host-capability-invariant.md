# ADR-0024: Plugin-host capability fail-closed invariant

- Status: implementation in progress (D6.6 capability hardening)
- Date: 2026-08-09
- Decision owner: Architect

## Context

The diagnostic host protocol already declares external execution disabled, but
the decoder previously accepted any boolean execution_enabled value and any
unique capability list. A replaced or malformed host could therefore send a
well-formed frame that was projected as execution-enabled, even though no
executor exists. This would weaken the future execution boundary and violate
the configured acceptance contract.

## Decision

The current diagnostic protocol has one capability: probe. The hello frame
must contain exactly the probe capability, and both hello and probe-result
frames must contain execution_enabled=false. A true value, an unknown
capability, or a missing/extra capability is a protocol error.

PluginHostProbeResult also enforces the application-level invariant that
execution_enabled is always false. Infrastructure converts a protocol
violation into the existing typed protocol-error result with the safe default
value; it never projects the untrusted true value to the UI.

This decision does not add an executor, dynamic loading, signature
verification, installation, or a security sandbox. Future capabilities must
be introduced through a separate versioned ADR and independent security
review.

## Consequences

- A replaced diagnostic host cannot advertise execution as enabled through a
  valid-looking frame.
- Unknown capability expansion fails closed instead of silently widening the
  protocol.
- The application, UI, and execution gate share one explicit disabled
  invariant.
- A future protocol version can add capabilities deliberately without
  changing the current probe semantics.

## Verification

- Source codec smoke for exact probe capability and true-state rejection.
- Source subprocess smoke for malformed hello/result mapping and safe false
  projection.
- Packaged root EXE probe with distinct PID and execution disabled.
- Existing Qt offscreen host diagnostic and close-guard paths remain required.
