# D65 parent review — document-tab forwarding simplification

| Field | Value |
|---|---|
| Delivery | `D65 / ARCH-50` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Architecture outcome

MainWindow no longer exposes four private tab-query forwarding aliases. The
existing DocumentTabSurface registry contract is now visible at each caller,
reducing coordinator indirection while preserving every tab-dependent policy.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | Laplace the 2nd / Luna max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Bacon the 3rd / Luna max | Two bounded waits returned no conclusion; agent was closed; no independent PASS claimed |
| Parent | Architect | Sole writer, integrated, inspected, and statically verified the direct-call slice |

No child PASS is claimed.

## Contract and boundary

`DocumentTabSurface` continues to own active-tab, editor, path-identity, and
identity-containment lookup. MainWindow retains save/open duplicate guards,
session restore sequencing and subset selection, close guards, recovery and
Replace All lifecycle, Find behavior, status projection, and all application
policy. The path `exclude=tab` guard and session snapshot active-tab read are
explicitly retained.

## Simplification assessment

Removing the four no-policy forwarding methods is the smallest safe change.
Adding another lookup service or moving restore policy would increase coupling.
No further behavior-preserving simplification was identified.

## Public-source applicability and embedded gate

This is Python/PyQt6 application/presentation code only. Embedded C/C++
assurance and manufacturer-source applicability are `N/A`; no MISRA, ISO,
certification, or private ByteDance-standard claim is made. Public CloudWeGo
material is only an engineering reference.

## Authorized non-destructive validation

- `D65-tab-forwarding-simplification-probe=PASS`.
- Targeted compileall, Ruff, and format checks — `PASS` before final
  documentation/package synchronization.
- Full compileall, handoff, repository check, package identity, and expected
  release NO-GO evidence are recorded after final synchronization.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static direct-call evidence cannot prove native callback interleaving or Qt
event timing. The slice is accepted with those limits; runtime and release
gates remain open, and delegated reviews are recorded as no-conclusion.
