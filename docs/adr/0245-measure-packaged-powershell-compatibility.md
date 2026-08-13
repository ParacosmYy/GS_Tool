# ADR-0245: Packaged measurement PowerShell compatibility

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D197 / ARCH-183

## Context

The packaged-capture evidence script still used `ConvertFrom-Json -DateKind
String` and default-encoded reads for the release manifest and each capture
report. That left the evidence tool incompatible with Windows PowerShell 5.1,
even after the packaging and release-dossier tools were made dual-shell
compatible.

## Decision

Keep `scripts/measure_packaged.ps1` as the sole owner of packaged-capture
process lifecycle, artifact binding, report validation, cleanup, and evidence
writing. Add the same local `ConvertFrom-JsonCompat` boundary used by the
release verifier: PowerShell 7+ retains string-date conversion and older
PowerShell uses its available conversion surface. Read the manifest and
per-run capture report as UTF-8. Do not launch or measure the EXE as part of
this source change.

## Preserved invariants

- Artifact bytes/hash/path binding, release-manifest checks, process timeout,
  offscreen environment, capture fields, queue bounds, cleanup, and report
  schema remain unchanged.
- JSON parse errors and report validation failures still propagate; no failure
  is caught or downgraded.
- Only the two JSON read sites change; no runtime measurement policy or
  performance claim is added.
- The change is reversible by removing one adapter and two encoding flags.

## Review and applicability

The architecture consultation (`Kierkegaard the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child PASS is claimed.
The independent review (`Planck the 6th / Luna max`) likewise returned no
conclusion after two bounded waits and was closed. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS` because this is a
single existing JSON boundary with no new measurement abstraction.

This is a PowerShell tooling change for the Python 3.12/PyQt6 project.
Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable;
the mandatory embedded enterprise workflow is therefore not applicable to
this source slice. Microsoft PowerShell/.NET documentation remains the
applicable public first-party source. Public CloudWeGo material is engineering
reference only; no private ByteDance standard, certification, or compliance
claim is made.

## Evidence and limits

- `D197-PS51-PARSE-PROBE=PASS`
- `D197-PS7-PARSE-PROBE=PASS`
- `D197-COMPAT-SOURCE-PROBE=PASS`
- `D197-COMPILE-RUFF-FORMAT=PASS`
- `D197-PRESENTATION-AUDIT=PASS`
- `D197-PACKAGE-BUILD=PASS`
- `D197-PACKAGE-IDENTITY-PROBE=PASS`

No packaged measurement or EXE launch was performed under the current
no-launch policy. Runtime capture, clean-machine, cross-machine, performance,
signing, installer, updater, legal, support, and release-owner evidence remain
open.
