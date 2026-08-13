# ADR-0265: Workspace-search application error taxonomy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D218 / ARCH-202

## Context

The shared application error taxonomy covered document/workspace services and
editor policy, but the Qt-free workspace-search boundary still exposed generic
`ValueError` and `TypeError` from policy construction, value objects, request
validation, and provider-result admission. That made callers depend on
implementation details and left the application contract inconsistent.

## Decision

Add `ApplicationTypeError`, inheriting from both `ApplicationError` and the
built-in `TypeError`. Migrate the 39 workspace-search validation and
provider-result branches to `ApplicationValidationError` or
`ApplicationTypeError` according to the existing built-in category. Preserve
exact messages, validation order, request/result dataclass shape, provider
protocol, directory capability, and Qt-free dependency direction.

Keep the two `Path.relative_to()` `except ValueError` clauses as built-in
filesystem/path operations; only their application-owned rethrows change to
`ApplicationValidationError`.

## Boundaries

1. `application.errors` owns stable category types only; it does not own
   presentation messages, provider implementation, or UI policy.
2. `workspace_search.py` remains a pure application boundary and continues to
   delegate traversal to `WorkspaceSearchProvider`.
3. Built-in compatibility is explicit: validation remains a `ValueError`, type
   failures remain a `TypeError`, and callers can still catch either base
   class.
4. This slice does not redesign result unions, change search limits, alter
   filesystem traversal, or claim runtime search/concurrency evidence.

## Public-source applicability and review

This is Python 3.12 application-layer code. Python's public [built-in
exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
is the applicable first-party source for preserving `ValueError` and
`TypeError` inheritance. Public CloudWeGo material is an engineering reference
only; no private ByteDance standard, certification, or compliance claim is
made. Embedded C/C++, MCU, RTOS, and manufacturer requirements are not
applicable.

The architecture role `Pascal the 6th / Luna max` returned `NO_CONCLUSION`
after two bounded waits. The independent role `James the 6th / Luna max`
returned `NO_CONCLUSION` after two bounded waits. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Verification and limits

- `D218-WORKSPACE-SEARCH-TAXONOMY-PROBE=PASS branches=39`.
- `D218-COMPILEALL=PASS`, `D218-RUFF=PASS`, `D218-FORMAT=PASS`, and
  `D218-PRESENTATION-AUDIT=PASS`.
- `D218-PACKAGE-BUILD-PS51=PASS`, `D218-PACKAGE-BUILD-PS7=PASS`, and
  `D218-PACKAGE-IDENTITY-PROBE=PASS`.

No GUI/QApplication, EXE launch, runtime filesystem traversal, concurrency,
screenshot, accessibility, clean-machine, cross-machine, signing, installer,
updater, legal, support, or release-owner evidence was run. No unit-test asset
was created or run.
