# ADR-0248: Package source-revision determinism

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D200 / ARCH-186

## Context

The package script sorted source files with PowerShell's `Sort-Object
FullName`. Windows PowerShell 5.1 and PowerShell 7 used different culture
ordering for the same 166 source entries, producing different source-revision
hashes even though every file hash was identical. That weakened cross-shell
artifact traceability.

## Decision

Keep the existing source-file set and per-file SHA-256 calculation, collect
canonical `relative-path=uppercase-hash` lines in a .NET generic string list,
and sort those lines with `System.StringComparer.Ordinal` before hashing the
manifest source revision. This uses APIs available in Windows PowerShell 5.1
and PowerShell 7 and makes ordering independent of the host culture.

## Preserved invariants

- The `src`, `scripts`, `packaging`, `pyproject.toml`, and `uv.lock` inventory,
  exclusion of `__pycache__`, manifest schema, artifact hash/size, atomic root
  copy, notices, cleanup, and release-gate decisions remain unchanged.
- The one-file EXE is not launched, and no installer, updater, registry, or
  network behavior is added.
- Only source-line collection/order changes; application and presentation code
  are untouched by D200.
- Both shells now produce the same `source_revision`; artifact bytes may still
  differ between separate PyInstaller invocations and remain individually
  bound by each manifest.

## Review and applicability

The architecture consultation (`Carver the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no architecture child PASS
is claimed. The independent review (`Hubble the 6th / Luna max`) returned
`PASS` for this bounded packaging-provenance slice after the review completed,
with its source-revision migration and runtime limits recorded in the review
file. Parent review is `PASS`, and the behavior-preserving simplification
assessment is `PASS` because the fix removes culture-sensitive ordering
without introducing a shared runtime module or changing packaging ownership.

This is a PowerShell tooling change for the Python 3.12/PyQt6 project.
Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.
Public Microsoft PowerShell/.NET documentation is the applicable first-party
source; public CloudWeGo material is engineering reference only and no private
ByteDance standard or compliance claim is made.

Applicable public sources: Microsoft Learn, `HashAlgorithm.ComputeHash`, .NET
Framework 4.8.1, https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.hashalgorithm.computehash?view=netframework-4.8.1;
and Microsoft Learn, `Path.GetRelativePath`, .NET Standard 2.1,
https://learn.microsoft.com/en-us/dotnet/api/system.io.path.getrelativepath?view=netstandard-2.1.
These are public API references for the preserved hashing/path contract; they
are not manufacturer requirements.

## Evidence and limits

- `D200-STABLE-SORT-PROTOTYPE=PASS shells_identical`
- `D200-PS51-PARSE-PROBE=PASS`
- `D200-PS7-PARSE-PROBE=PASS`
- `D200-DETERMINISTIC-SORT-SOURCE-PROBE=PASS`
- `D200-PS51-SOURCE-REVISION=tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`
- `D200-PS7-SOURCE-REVISION=tree-sha256:79cca7bc5704fca205ef41385a588c49d11c9cef77f7ec9a90d574acafad7646`
- `D200-COMPILE-RUFF-FORMAT=PASS`
- `D200-PACKAGE-BUILD-PS51=PASS`
- `D200-PACKAGE-BUILD-PS7=PASS`
- `D200-INDEPENDENT-REVIEW=PASS bounded packaging-provenance slice`

No EXE launch or runtime capture was performed. Clean-machine, cross-machine,
signing, installer, updater, legal, support, permission/disk-pressure,
hard-power, and release-owner evidence remain open.
