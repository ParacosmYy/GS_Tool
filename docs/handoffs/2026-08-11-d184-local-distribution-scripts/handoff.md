# Handoff: 2026-08-11-d184-local-distribution-scripts

| Field | Value |
|---|---|
| ID | `2026-08-11-d184-local-distribution-scripts` |
| Delivery / slice | `D184 / ARCH-171 Local hash-gated distribution scripts` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:55:00+08:00` |

## User outcome

QuillForge now has a documented local install/update/uninstall boundary around
the portable executable. It verifies a caller-supplied SHA-256, defaults to a
user-local install target, retains bounded rollback state, and keeps file
associations explicit and opt-in.

## Scope and boundaries

### In scope

- Shared PowerShell path, hash, state, rollback, and association helpers.
- `install.ps1`, `update.ps1`, and `uninstall.ps1` sequencing.
- Static parsing and source-contract verification.

### Out of scope

- Network delivery, elevation, EXE startup, registry execution, signing,
  clean-machine validation, and enterprise release approval.
- Changes to Python application behavior, UI, settings, or portable manifest
  policy.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependencies, risks, and status |
| Product | `user outcome` | Safe local distribution and explicit association policy |
| Developer 1 | `parent` | Shared distribution module and safety invariants |
| Developer 2 | `parent` | Install/update/uninstall entry points and packaging integration |
| QA | `parent` | Read-only parser, lexical, package, and handoff verification |

## Changed files and modules

- `packaging/QuillForge.Distribution.psm1` — shared path/hash/state/rollback/
  association boundary.
- `packaging/install.ps1` — first install with explicit SHA and optional
  associations.
- `packaging/update.ps1` — staged update and explicit rollback.
- `packaging/uninstall.ps1` — exact owned cleanup.
- `docs/adr/0233-local-distribution-scripts.md` — architecture decision.

## Decisions and constraints

- Shared checkout writer: `parent`, packaging and D184 documentation only.
- Runtime launch policy: not allowed; no script was executed, no registry was
  touched, and no EXE was started.
- Associations are HKCU-only, explicit-switch opt-in, extension-validated,
  and refuse existing user keys.
- This is Python/PyQt6 distribution work; embedded C/C++, MCU, RTOS, and
  manufacturer requirements are not applicable. Public engineering guidance
  does not establish a private ByteDance standard or compliance claim.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| PowerShell AST parser for four packaging files | `PASS` | `D184-POWERSHELL-PARSE=PASS`; scripts were parsed, not run. |
| Static distribution probe | `PASS` | `D184-STATIC-DISTRIBUTION-PROBE=PASS`; no network/process, hash, HKCU, rollback, and `ShouldProcess` contracts present. |
| Source contract probes | `PASS` | `D184-SHA-CONTRACT-PROBE=PASS`, `D184-CONTAINMENT-PROBE=PASS`, `D184-OPT-IN-HKCU-PROBE=PASS`. |
| `scripts/package.ps1` | `PASS` | Portable artifact rebuilt; no EXE launch. |
| `scripts/verify_release_handoff.ps1` | `NO-GO` expected | Three artifact-bound runtime/mechanical consistency failures remain. |

## Unrun checks and reason

- Install/update/rollback/uninstall execution — not authorized by the active
  no-launch/registry policy.
- Registry behavior and file-association runtime — not authorized; static
  HKCU-only and opt-in evidence is the available safe evidence.
- Clean-machine, cross-machine, signing, and release-owner checks — external
  release evidence is not available in the current checkout/policy.

## Known risks and limits

- `New-Item` registry-provider behavior, filesystem replacement atomicity, and
  rollback durability still require an authorized Windows operational run.
- The portable manifest intentionally remains `not-an-installer`,
  `not-implemented`, and `not-configured` for installer/update/associations.
- Release remains `no-go` with ten open gates.

## Acceptance and evidence IDs

- Acceptance: `S237`, `D184-AC01`
- Evidence: `D184-POWERSHELL-PARSE=PASS`,
  `D184-STATIC-DISTRIBUTION-PROBE=PASS`, `D184-SHA-CONTRACT-PROBE=PASS`,
  `D184-CONTAINMENT-PROBE=PASS`, `D184-OPT-IN-HKCU-PROBE=PASS`,
  `D184-SIMPLIFICATION-ASSESSMENT=PASS`,
  `docs/agent-team/reviews/D184-local-distribution-scripts-parent-review.md`,
  `docs/agent-team/reviews/D184-local-distribution-scripts-independent-review.md`.

## Next owner and next action

- Owner: Release Engineering / Product / QA
- Action: authorize and schedule a disposable Windows validation environment
  for script, registry, rollback, clean-machine, and release evidence; do not
  infer those results from this handoff.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `7FEA6A5E9F5E9E361BD15018FF7CD43DC8AA343E727EE8EEE299F965F1B5A2B8` / `38,553,558` bytes
- Source revision: `tree-sha256:ef64cab5c9ae2886b40727b8cb45ad376acd75761cff77f768069872b378a193`
- Packaging note: portable PyInstaller one-file candidate rebuilt; no installer
  or updater artifact is claimed.

## Disposition

`accepted-with-limits`: the source boundary and static evidence are recorded;
runtime distribution and enterprise release gates remain open.
