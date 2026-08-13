# ADR-0244: Release verifier PowerShell compatibility

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D196 / ARCH-182

## Context

The release dossier verifier used `ConvertFrom-Json -DateKind String`, which
is unavailable in Windows PowerShell 5.1. After adding a compatibility adapter,
5.1 still read the existing UTF-8 performance reports with its system ANSI
default and corrupted Chinese text before JSON parsing. PowerShell 7 did not
expose either failure.

## Decision

Keep `scripts/verify_release_handoff.ps1` as the single owner of the current
artifact identity checks, dossier generation, mechanical-failure list, and
non-zero no-go behavior. Add `ConvertFrom-JsonCompat`: PowerShell 7+ keeps the
existing `-DateKind String` behavior, while older PowerShell uses the native
JSON conversion without that unsupported parameter. Read the manifest and four
JSON reports explicitly as UTF-8 in both shells. Leave every predicate,
failure name, open gate, and dossier field unchanged.

## Preserved invariants

- UTF-8 report content, including Chinese text, is decoded before JSON parsing
  under Windows PowerShell 5.1 and PowerShell 7.
- The same three stale artifact-bound mechanical failures remain expected:
  `packaged_report_artifact_match`, `interactive_startup_report_consistent`,
  and `startup_preflight_report_consistent`.
- No report is refreshed, no performance result is reinterpreted, and no
  runtime, installer, registry, network, or release decision is bypassed.
- Parser/conversion errors still propagate; the compatibility adapter does not
  catch or hide malformed JSON.
- The change is reversible by removing one adapter and five explicit encoding
  flags.

## Review and applicability

The architecture consultation (`Helmholtz the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; the encoding correction was
then re-consulted with `Franklin the 6th / Luna max`, which also returned no
conclusion after two bounded waits. No child PASS is claimed. The independent
review (`Euler the 6th / Luna max`) returned no conclusion after two bounded
waits and was closed. Parent review is `PASS`, and the behavior-preserving
simplification assessment is `PASS` because the adapter and explicit UTF-8
reads are the smallest compatibility boundary.

This is a PowerShell tooling change for the Python 3.12/PyQt6 project. Embedded
C/C++, MCU, RTOS, and manufacturer requirements are not applicable; the
mandatory embedded enterprise workflow is therefore not applicable to this
source slice. Microsoft PowerShell/.NET API documentation is the applicable
public first-party source. Public CloudWeGo material remains engineering
reference only; this ADR makes no private ByteDance standard, certification,
or compliance claim.

## Evidence and limits

- `D196-PS51-PARSE-PROBE=PASS`
- `D196-PS7-PARSE-PROBE=PASS`
- `D196-COMPAT-SOURCE-PROBE=PASS`
- `D196-PS51-VERIFIER=EXPECTED-NO-GO`
- `D196-PS7-VERIFIER=EXPECTED-NO-GO`
- `D196-SAME-MECHANICAL-FAILURES-PROBE=PASS`
- `D196-COMPILE-RUFF-FORMAT=PASS`
- `D196-PRESENTATION-AUDIT=PASS`
- `D196-PACKAGE-BUILD=PASS`
- `D196-PACKAGE-IDENTITY-PROBE=PASS`

No EXE was launched. Signing, installer, updater, file-association,
clean-machine, legal, support, permission/disk-pressure, hard-power,
cross-machine, and release-owner evidence remain open; release verification is
an expected no-go because the reports are not bound to this candidate.
