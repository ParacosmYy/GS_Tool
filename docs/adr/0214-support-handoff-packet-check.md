# ADR-0214: Support handoff packet check

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D165 / ARCH-152

## Context

The release manifest already declared a local issue route and support owner,
but the release verification chain only required `docs/support/ISSUES.md`. The
new local support packet records candidate identity, ownership, triage,
escalation, and explicit unrun limits. A release dossier should prove that the
packet exists without treating it as owner acceptance or a support-release
approval.

## Decision

Require the exact root-relative file `docs/support/HANDOFF.md` in
`scripts/check.ps1` and `scripts/verify_release_handoff.ps1`. Add the boolean
`support_handoff_packet_exists` to the dossier checks and evidence, while
leaving all manifest gate statuses, the ten-item `$openGates` list, `no-go`
decision, output path, and JSON compatibility unchanged.

The check is deliberately mechanical. It does not parse or rewrite the packet,
contact an owner, attest a clean machine, launch QuillForge, or change the
`support:handoff-pending` gate.

## Alternatives considered

- Keep the packet outside the verifier: rejected because a required handoff
  artifact could silently disappear from the release set.
- Mark support complete when the file exists: rejected because file presence is
  not owner acceptance, clean-machine evidence, or an operational support
  channel.
- Parse packet contents in PowerShell: rejected because that would duplicate
  the documentation contract and create brittle release coupling.

## Review and simplification

- Architect role: Faraday the 5th / Luna max; `PASS` for the bounded path and
  dossier additions.
- Independent review: Leibniz the 5th / Luna max; `NO_CONCLUSION` because the
  bounded reviewer could not establish a Git baseline and reviewed before the
  regenerated dossier. The parent records the exact static evidence below.
- Parent review: `PASS` for the two-file mechanical change, unchanged gate
  list, and no runtime/test path.
- Simplification assessment: `PASS`; one required path and one boolean check
  avoid parsing, adapters, or duplicated support state.

## Authorized evidence

- `D165-POWERSHELL-PARSE-PROBE=PASS`.
- `D165-SCOPE-INVARIANT-PROBE=PASS`.
- `D165-SUPPORT-PACKET-CHECK-PROBE=PASS` after the final dossier is generated.
- `scripts/check.ps1` and the expected release `NO-GO` dossier.
- No QApplication, EXE, screenshot, network, or test asset execution was
  authorized.

## Public-source applicability

This is Python/PyQt6 project release tooling and documentation traceability.
Embedded C/C++, MCU/RTOS/BSP/HAL, and manufacturer requirements are not
applicable. Public CloudWeGo material remains an engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.

## Limits

The check proves only repository-local packet presence. It does not prove
support owner acceptance, service availability, clean-machine behavior,
signing, installer/update readiness, legal clearance, runtime startup, or
enterprise release readiness.
