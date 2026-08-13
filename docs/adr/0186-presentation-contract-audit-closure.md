# ADR-0186: presentation contract audit closure

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D142 / ARCH-124

## Context

The presentation layer already used Qt-free coordinators, typed notification
levels, and TaskRunner pending-work observation, but the static audit did not
enforce two architecture conventions consistently: coordinator `*Ports`
contracts should be immutable/slotted, and coordinator notification calls
should make their semantic level explicit.

## Decision

Extend `scripts/audit_presentation_contracts.py` with two AST-only rules:

- top-level coordinator classes whose names end in `Ports` must use
  `@dataclass(frozen=True, slots=True)`;
- direct `self._ports.notify(...)` calls must include an explicit `level=`
  keyword.

The existing checks for Qt/infrastructure dependency leakage, MainWindow
notification levels, and TaskRunner pending-work observability remain in the
same audit. The audit is a source contract gate; it does not import or launch
Qt and it does not move policy between MainWindow and coordinators.

## Alternatives rejected

- Runtime reflection would make the audit depend on importing the presentation
  graph and could initialize Qt or hide source-boundary violations.
- A broad type checker or new framework would be disproportionate to these
  two local invariants.
- Rewriting existing coordinators is unnecessary; the current source already
  satisfies the rules, so this delivery adds enforcement only.

## Review and evidence

Planck the 4th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Russell the 4th / Luna max was
assigned the independent read-only review and also returned no conclusion
after two short waits. No child PASS is claimed. Parent review is `PASS`;
simplification assessment is `PASS` because the smallest complete change is
two AST predicates in the existing audit.

Authorized non-destructive evidence:

- `D142-AUDIT-PORTS-PROBE=PASS:23 contracts`
- `D142-AUDIT-NOTIFICATION-LEVEL-PROBE=PASS:21 coordinator calls`
- `D142-AUDIT-DEPENDENCY-PROBE=PASS`
- `D142-AUDIT-RUN-PROBE=PASS`
- `D142-PRESENTATION-AUDIT=PASS`
- `D142-COMPILEALL=PASS`
- `D142-RUFF=PASS`
- `D142-FORMAT=PASS`
- `D142-CHECK=PASS`
- `D142-HANDOFF=PASS`
- `D142-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and traceability/no-launch checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `A5FF72C40FDDEC90838331E05FCB7888BAD4DDCF84FA5DDF4A487F6847AB3966`
- bytes: `38534523`
- source revision: `tree-sha256:d966a75dc667bd0cde93f8146269c1a3090aeec745c7209add1060111d10799f`

Public-source applicability is Python 3.12/PyQt6 source auditing; embedded
C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

The AST gate does not prove native Qt rendering, runtime startup, worker
timing, accessibility, font/DPI metrics, filesystem durability,
clean-machine behavior, cross-machine behavior, signing, installer/update,
legal clearance, support ownership, or release readiness. Those gates remain
open under the active no-launch/no-release authorization boundary.
