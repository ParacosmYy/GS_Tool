# ADR-0263: Editor policy validation error taxonomy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D216 / ARCH-201

## Context

The Phase 3 application error taxonomy now names document/workspace service
failures, but `EditorOperationPolicy` still raised generic `ValueError` for
its eight application-owned limit invariants. The policy is constructed at
the composition boundary and has no reason to depend on Qt or an adapter.

## Decision

Use the existing Qt-free `ApplicationValidationError` for all eight
`EditorOperationPolicy` validation branches. Preserve every message, default,
dataclass field, and `ValueError` catch compatibility. Keep workspace-search,
command-registry, domain-model, and presentation validation out of this
bounded slice.

## Invariants

1. `editor_policy.py` imports only the existing application error contract and
   standard-library dataclass support.
2. Invalid values still fail during policy construction with the same text and
   timing; valid defaults remain unchanged.
3. Presentation code continues to consume the policy as an immutable value and
   no Qt or editor behavior moves into the application module.
4. This is taxonomy clarification, not a catch-all adapter wrapper or result
   union redesign.

## Public-source applicability and review

This is Python 3.12 application-layer code. Python's public [built-in
exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
is the applicable first-party source for preserving `ValueError` inheritance.
Public CloudWeGo material is an engineering reference only; no private
ByteDance standard, certification, or compliance claim is made. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

The architecture role `Leibniz the 6th / Luna max` returned `NO_CONCLUSION`
after two bounded waits. The independent role `Dewey the 6th / Luna max`
returned `NO_CONCLUSION` after two bounded waits. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Verification and limits

- `D216-EDITOR-POLICY-TAXONOMY-PROBE=PASS fields=8`.
- `D216-COMPILEALL=PASS`, `D216-RUFF=PASS`, `D216-FORMAT=PASS`, and
  `D216-PRESENTATION-AUDIT=PASS`.
- `D216-PACKAGE-BUILD-PS51=PASS`, `D216-PACKAGE-BUILD-PS7=PASS`, and
  `D216-PACKAGE-IDENTITY-PROBE=PASS`.

No GUI/QApplication, EXE launch, runtime editor operation, screenshot,
accessibility, clean-machine, cross-machine, signing, installer, updater,
legal, support, or release-owner evidence was run. No unit-test asset was
created or run.
