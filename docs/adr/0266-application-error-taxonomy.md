# ADR-0266: Remaining application error taxonomy

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D219 / ARCH-203

## Context

The shared application error taxonomy covered documents, workspace, editor
policy, and workspace search, but several remaining Qt-free use cases and
contracts still exposed generic `ValueError` or `RuntimeError` directly.
That left command registration, plugin governance, session contracts,
recovery admission, release metadata, and event ownership inconsistent for
callers and diagnostics.

## Decision

Route the remaining application-owned validation failures through
`ApplicationValidationError` and state failures through
`ApplicationStateError`, preserving the existing built-in base classes and
messages. The nine bounded modules are `commands`, `events`,
`plugin_enablement`, `plugin_execution`, `plugin_governance`, `plugin_host`,
`ports`, `recovery`, and `release_metadata`.

Keep the dedicated `RecoveryCaptureCancelled` and `RecoveryChannelAborted`
port exceptions unchanged because they are explicit recovery-channel
protocol types. Keep domain, infrastructure, and presentation validation
outside this slice; those layers own their own contracts and should not
depend on application exception categories.

## Boundaries

1. `application.errors` remains a dependency-free category owner; it does not
   own UI messages, adapter behavior, or a catch-all translation layer.
2. Built-in compatibility is intentional: validation remains catchable as
   `ValueError`, and state failures remain catchable as `RuntimeError`.
3. Command, plugin, session, recovery, release, and event behavior,
   validation order, public dataclasses, and protocol shapes remain unchanged.
4. This slice does not claim runtime GUI, event-thread, filesystem,
   plugin-host, release, or executable evidence.

## Public-source applicability and review

This is Python 3.12 application-layer code. Python's public [built-in
exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
is the applicable first-party source for preserving `ValueError` and
`RuntimeError` inheritance. Public CloudWeGo material is an engineering
reference only; no private ByteDance standard, certification, or compliance
claim is made. Embedded C/C++, MCU, RTOS, and manufacturer requirements are
not applicable.

The architecture role `Sagan the 6th / Luna max` returned `NO_CONCLUSION`
after two bounded waits. The independent role `Pasteur the 6th / Luna max`
returned `NO_CONCLUSION` after two bounded waits. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Verification and limits

- `D219-APPLICATION-ERROR-TAXONOMY-PROBE=PASS files=9 direct_generic_raises=0`.
- `D219-COMPILEALL=PASS`, `D219-RUFF=PASS`, `D219-FORMAT=PASS`, and
  `D219-PRESENTATION-AUDIT=PASS`.
- `D219-PACKAGE-BUILD-PS51=PASS`, `D219-PACKAGE-BUILD-PS7=PASS`, and
  `D219-PACKAGE-IDENTITY-PROBE=PASS`.

No GUI/QApplication, EXE launch, event-thread timing, runtime filesystem or
plugin-host exchange, screenshot, accessibility, DPI, clean-machine,
cross-machine, signing, installer, updater, legal, support, or release-owner
evidence was run. No unit-test asset was created or run.
