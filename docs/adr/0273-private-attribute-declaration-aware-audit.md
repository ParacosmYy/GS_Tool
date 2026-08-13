# ADR-0273: Private attribute declaration-aware audit

- Status: accepted-with-limits
- Delivery: D227 / ARCH-209
- Date: 2026-08-12

## Context

D226 added a presentation AST guard for direct `self._private()` calls. Its
known-name set included class methods and attributes assigned from instance
methods, but not class-level assignments or annotated fields. A future
callable provider declared on a presentation class could therefore be
reported as a missing accessor even though the attribute was intentionally
part of the class contract.

## Decision

Extend the existing `_audit_private_self_calls()` implementation with one
small `_class_attribute_names()` helper. It collects direct class-body
`Assign`, `AnnAssign`, and `AugAssign` names, then merges them with the
instance attributes discovered by the existing method walk. The guard remains
limited to direct private calls on `self` in top-level presentation classes.

No broad private-read audit, runtime reflection, type checker, provider
registry, or presentation/application boundary change is introduced.

## Boundaries and alternatives

- The helper recognizes direct class-body names; dynamic `setattr`, inherited
  private methods, destructuring declarations, and runtime callability remain
  outside the static proof.
- An annotated field is treated as a declared name, not as proof that its
  runtime value is callable. This preserves the existing heuristic contract;
  it does not replace Python type checking.
- The existing presentation audit remains the single owner. No dependency or
  test-only asset was added.
- Native Qt/EXE startup remains unrun under the permanent project no-launch
  policy.

## Review and simplification

- Parent review: `PASS` for the bounded source/static scope.
- Architecture role `Dalton the 6th / Luna max`: `NO_CONCLUSION` after
  bounded waits and closure; the agent did not read the checkout source.
- Independent review `Socrates the 6th / Luna max`: `NO_CONCLUSION` after
  bounded waits and closure; the agent could not obtain a repository diff.
- Simplification: `PASS`; one named helper and one merged known-name set are
  the smallest clear extension of the D226 gate.

## Public-source applicability

Python 3.12's public `ast` module and the existing uv/Ruff toolchain are the
applicable engineering references. No dependency changed. Public CloudWeGo
material remains an engineering reference only; no private ByteDance standard,
certification, or compliance claim is made. Embedded C/C++, MCU, RTOS, and
manufacturer requirements are not applicable.

## Evidence and limits

The class-declaration probe, presentation audit, compileall, Ruff, formatting,
dual-shell package builds, archive inspection, package identity, project check,
and handoff verification are the applicable non-destructive evidence. Qt/
QApplication/EXE startup, native rendering, accessibility, clean-machine,
cross-machine, signing, installer/update, legal, support, permission/
disk-pressure, hard-power, and release-owner evidence remain open.

