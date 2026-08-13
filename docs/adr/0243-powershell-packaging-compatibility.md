# ADR-0243: Windows PowerShell packaging compatibility

- Status: accepted-with-limits
- Date: 2026-08-11
- Delivery: D195 / ARCH-181

## Context

The documented `scripts/package.ps1` entry point successfully built the
portable EXE under Windows PowerShell 5.1, then failed while writing the
release manifest because `Path.GetRelativePath`, `SHA256.HashData`, and
`Convert.ToHexString` are not available in the .NET Framework surface used by
that shell. PowerShell 7 completed the same script, but requiring a second
shell was an avoidable Windows-first delivery defect.

## Decision

Keep the packaging script as the single owner of local artifact staging,
source inventory, SHA-256 provenance, and manifest composition. Replace only
the unavailable helpers with compatibility calls: `Uri.MakeRelativeUri` plus
`UnescapeDataString` for root-relative inventory paths,
`SHA256.Create().ComputeHash` for the source revision bytes, and
`BitConverter.ToString` for hexadecimal formatting. Dispose the hash object in
`finally`; preserve the sorted source list, input newline semantics, artifact
hashes, atomic root-copy replacement, and cleanup behavior.

Microsoft's public [HashAlgorithm.ComputeHash documentation](https://learn.microsoft.com/en-us/dotnet/api/system.security.cryptography.hashalgorithm.computehash?view=netframework-4.8.1)
explicitly covers .NET Framework, while [Path.GetRelativePath](https://learn.microsoft.com/en-us/dotnet/api/system.io.path.getrelativepath?view=netstandard-2.1)
is a newer API surface. The compatibility implementation avoids requiring
that newer surface without changing the manifest contract.

## Preserved invariants

- `src`, `scripts`, `packaging`, `pyproject.toml`, and `uv.lock` remain the
  exact sorted source inventory; files under `__pycache__` remain excluded.
- Hash inputs, uppercase file hashes, lowercase `tree-sha256` output, manifest
  schema, artifact copy identity, and notice/lockfile provenance are unchanged.
- Root test-copy staging still uses the existing replace/move and cleanup path;
  no registry, network, installer, or executable launch was added.
- The fix is shell/tooling-only and does not change application, domain,
  presentation, plugin, or release-gate policy.

## Review and applicability

The architecture consultation (`Archimedes the 6th / Luna max`) returned no
conclusion after two bounded waits and was closed; no child PASS is claimed.
The independent review (`Sartre the 6th / Luna max`) likewise returned no
conclusion after two bounded waits and was closed. Parent review is `PASS`, and
the behavior-preserving simplification assessment is `PASS` because only three
unavailable API calls were replaced at their existing ownership point.

This is a PowerShell packaging-tool change for the Python 3.12/PyQt6 project.
Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable;
the mandatory embedded enterprise workflow is therefore not applicable to
this source slice. Microsoft API documentation is the applicable public
first-party source. Public CloudWeGo material remains engineering reference
only; this ADR makes no private ByteDance standard, certification, or
compliance claim.

## Evidence and limits

- `D195-PS51-PARSE-PROBE=PASS`
- `D195-PS7-PARSE-PROBE=PASS`
- `D195-PS51-PACKAGE=PASS`
- `D195-PS7-PACKAGE=PASS`
- `D195-POWERSHELL-COMPAT-SOURCE-PROBE=PASS`
- `D195-COMPILE-RUFF-FORMAT=PASS`
- `D195-PRESENTATION-AUDIT=PASS`
- `D195-PACKAGE-IDENTITY-PROBE=PASS`

No EXE was launched. External signing, installer, updater, file-association,
clean-machine, legal, support, permission/disk-pressure, hard-power,
cross-machine, and release-owner evidence remain open; release verification
remains `no-go` because the current reports are not bound to this artifact.
