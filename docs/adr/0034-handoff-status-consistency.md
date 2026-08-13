# ADR-0034: Handoff status consistency

- **Status:** accepted with limits for D10 GOV-02
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The handoff index and the Markdown handoff files are two required parts of the
delivery ledger. Three current entries had already been accepted in their
handoff files while the index still reported `in-progress`, so a reader could
not rely on the index status as a projection of the handoff decision.

## Decision

1. `scripts/verify_handoff.ps1` extracts the required Markdown `Status` row for
   every indexed handoff and requires exact equality with the index entry.
2. The three stale index records for D6.6 PID provenance, D7.4.2 startup
   cancellation, and D9 UI-07 accessibility are synchronized to
   `accepted-with-limits`.
3. The check remains structural: it does not judge the quality of a status
   decision, authorize runtime launch, or close delivery-register, legal,
   security, or release gates.

## Consequences and limits

- Future status drift fails the handoff gate with the affected ID and both
  observed values.
- Status consistency does not make a source slice complete or turn static
  evidence into runtime proof.
- The no-launch boundary, no-test-asset policy, and parent-owned final review
  remain unchanged.

## Verification

- `scripts/verify_handoff.ps1` checks every indexed handoff status row.
- `scripts/check.ps1` exercises the verifier and repository evidence contract.
- Packaging is rerun because the verifier is part of the release source tree.
