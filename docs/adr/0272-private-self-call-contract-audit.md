# ADR-0272: Private self-call contract audit

- Status: accepted-with-limits
- Delivery: D226 / ARCH-208
- Date: 2026-08-12

## Context

D225 exposed a startup failure that ordinary linting did not catch: the
presentation `CommandSurface` called `self._locale()` from several projection
paths, but the class had only stored `_locale_provider` and had not defined the
accessor. The failure occurred during menu construction before the shell was
visible.

## Decision

Extend the existing `scripts/audit_presentation_contracts.py` AST audit with
`_audit_private_self_calls()`. For each top-level presentation class, it
records direct `self._private()` calls and accepts the call only when the name
is a method defined by that class or an attribute assigned by one of its
methods. Any remaining call is reported with its source path, line, class, and
attribute name.

This is a static typo-detection heuristic, not a type checker or a runtime
proof. It deliberately preserves injected callable fields such as
`_locale_provider`, `_sink`, and `_recovery_channel_factory`.

## Boundaries and alternatives

- The existing presentation audit remains the only owner; no second lint
  framework, runtime reflection, or dependency was added.
- No Qt, application, domain, infrastructure, plugin, locale, theme, or
  executable behavior changes.
- A broad type-checker migration was rejected as disproportionate to the
  captured failure and would add a new project-wide tool contract.
- Test-only assets and runtime startup were not added or run under project
  policy.

## Review and simplification

- Parent review: `PASS` for the current source/static scope.
- Architecture role `Lagrange the 6th / Luna max`: `NO_CONCLUSION` after
  bounded waits and closure.
- Independent review `Popper the 6th / Luna max`: `NO_CONCLUSION` after
  bounded waits and closure.
- Simplification: `PASS`; the rule reuses the existing AST audit and keeps
  one named helper. Further factoring would add abstraction without reducing
  the small decision surface.

## Public-source applicability

Python 3.12's public `ast` module and existing Ruff/uv project tooling are the
applicable engineering references. No dependency changed. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

## Evidence and limits

The source audit, private-call probe, compileall, Ruff, formatting, dual-shell
package builds, archive inspection, package identity, project check, and
handoff verification are the applicable non-destructive evidence. Qt/EXE
startup, native rendering, accessibility, clean-machine, cross-machine,
signing, installer/update, legal, support, permission/disk-pressure,
hard-power, and release-owner evidence remain user-owned or externally gated.

