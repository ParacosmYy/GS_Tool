# ADR-0261: Application error taxonomy for document and workspace use cases

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D214 / ARCH-199

## Context

The Phase 3 enterprise-architecture migration target calls for typed
application contracts and a stable error taxonomy. Document and workspace
use cases still raised generic `ValueError` and `RuntimeError` instances for
application-owned validation and state failures. That made failure ownership
less explicit for future coordinators and observability, even though current
presentation code intentionally catches those built-in base classes.

## Decision

Add a Qt-free `application.errors` module with three stable categories:
`ApplicationError`, `ApplicationValidationError`, and `ApplicationStateError`.
The two concrete categories retain `ValueError` and `RuntimeError` as base
classes so existing catch sites and user-facing messages remain compatible.
Migrate only `DocumentService`'s missing-save-target validation and
`WorkspaceService`'s input, containment, and missing-root state failures.

Keep domain `DocumentConflictError` in the domain layer because it represents
an immutable document revision conflict, and leave infrastructure exceptions
and adapter contracts unchanged. This is a typed ownership boundary, not a
catch-all wrapper or a translation of diagnostic text.

## Preserved invariants

- Existing exception messages are byte-for-byte unchanged.
- Existing `except ValueError` and `except RuntimeError` presentation paths
  continue to catch the new categories.
- Application errors import no Qt, filesystem adapter, subprocess, or
  presentation module.
- Document persistence, workspace containment, provider calls, async guards,
  notifications, and file/folder behavior are unchanged.

## Review and applicability

The architecture consultation (`Arendt the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child architecture PASS
is claimed. `Fermat the 6th` completed a bounded independent static source
review with `PASS`; a separate independent review (`Aristotle the 6th / Luna
max`) returned no conclusion after two bounded waits and was closed. Parent
review is `PASS`. The behavior-preserving simplification assessment is `PASS`:
one small Qt-free taxonomy module and two focused service migrations are the
smallest complete Phase 3 slice.

The [Python 3.12 built-in exceptions documentation](https://docs.python.org/3.12/library/exceptions.html)
is the applicable public first-party source for preserving the built-in
exception compatibility contract. This is a Python 3.12 application-layer
change, not embedded C/C++, MCU, RTOS, or manufacturer-requirement work; the
mandatory embedded enterprise workflow is therefore not applicable to this
source slice. Public CloudWeGo material remains an engineering reference only;
no private ByteDance standard, certification, or compliance claim is made.

## Evidence and limits

- `D214-APPLICATION-ERROR-TAXONOMY-SOURCE-PROBE=PASS`
- `D214-EXCEPTION-COMPATIBILITY-PROBE=PASS`
- `D214-COMPILEALL=PASS`
- `D214-RUFF=PASS`
- `D214-FORMAT=PASS`
- `D214-PRESENTATION-AUDIT=PASS`
- `D214-PACKAGE-BUILD-PS51=PASS`
- `D214-PACKAGE-BUILD-PS7=PASS`
- `D214-PACKAGE-IDENTITY-PROBE=PASS`

No GUI, QApplication, EXE launch, screenshot, native rendering,
accessibility, runtime filesystem interaction, clean-machine, cross-machine,
signing, installer, updater, legal, support, or release-owner evidence was
performed. No unit-test asset was created or run.
