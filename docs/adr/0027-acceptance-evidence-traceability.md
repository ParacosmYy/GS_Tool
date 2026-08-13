# ADR 0027: Mechanize acceptance evidence traceability

- **Status:** accepted with limits for D8.4
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The acceptance contract is configurable JSON, but a green source check did not
previously verify that its statuses, required evidence, explicit limits, and
file-backed evidence references were internally coherent. That allowed an
item to be marked as evidence-backed while retaining an empty evidence list.

## Decision

`scripts/check.ps1` is the deterministic structural gate for
`docs/agent-team/acceptance.json`:

1. Subdelivery and scenario status values are checked against their separate
   allowed vocabularies.
2. Every item must declare non-empty required evidence. Items marked
   `accepted-with-limits`, `completed`, `code_verified`, or `smoke_verified`
   must contain at least one evidence entry.
3. Items marked `defined`, `environment_pending`, or `baseline_only` must
   declare limits or an explicit unrun reason.
4. Evidence entries must be non-empty strings. Entries beginning with a
   repository path (`docs/`, `src/`, `scripts/`, `packaging/`, `assets/`,
   `dist/`, or `./`) must resolve to an existing file.

The lint validates traceability structure only. It does not infer that a
descriptive smoke label ran, authorize environment-sensitive experiments, or
replace Product, QA, Architect, Legal, or release-owner judgment.

## Consequences and limits

- New delivery records fail early when their evidence contract is incomplete.
- Existing intentionally open items remain open but must state why evidence is
  not yet available.
- This is a repository-local preflight; it does not prove clean-machine,
  cross-machine, disk-pressure, hard-power, legal, signing, installer, or
  support outcomes.
