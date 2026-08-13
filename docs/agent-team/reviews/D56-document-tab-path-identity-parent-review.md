# D56 parent review — document-tab path identity boundary

| Field | Value |
|---|---|
| Delivery | `D56 / ARCH-45` |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User and architecture outcome

Complete document-tab path identity lookup now belongs to
`DocumentTabSurface`. MainWindow delegates ordinary full-registry lookup while
retaining the startup restore subset and active-tab selection policy.

## Role and review record

| Role | Agent | Result |
|---|---|---|
| Architect | Copernicus the 2nd / Luna max | Two bounded waits returned no conclusion; no architecture PASS claimed |
| Independent reviewer | Raman the 2nd / Luna max | Two bounded waits returned no conclusion; no independent PASS claimed |
| Parent | Architect | Integrated, inspected, and verified the bounded change |

No child PASS is claimed.

## Contract and boundary

`DocumentTabSurface.find_by_path(path, exclude=None)` accepts an optional
`Path`, returns a projected record or `None`, compares with the canonical
`path_key()` helper, ignores pathless records, and excludes by object identity.
The `DocumentTabLike` protocol now explicitly includes `DocumentState`, which
is already the record contract used by MainWindow.

The surface does not own file opening, save behavior, session restoration, or
document mutations. MainWindow keeps the `_session_restore_tabs` subset scan
because that selection is startup policy rather than generic tab identity.

## Simplification assessment

The migration deletes one repeated registry traversal and keeps one canonical
lookup owner. A path-index cache, new coordinator, or generic identity service
would add lifecycle and invalidation complexity for the current bounded tab
collection, so none was introduced.

## Public-source applicability and embedded gate

This is Python/PyQt6 presentation code only. Embedded C/C++ assurance and
manufacturer-source applicability are `N/A`; no MISRA, ISO, certification, or
private ByteDance-standard claim is made. Public CloudWeGo material is only an
engineering reference.

## Authorized non-destructive validation

- `D56-document-tab-path-identity-boundary-probe=PASS`.
- `D56-session-restore-subset-policy-retained-probe=PASS`.
- Targeted compileall, Ruff, and format checks — `PASS`.
- Full compileall, Ruff, format, handoff, repository check, package identity,
  and expected release NO-GO evidence are recorded after the final package.
- D56 package identity: root/dist SHA-256
  `5DD9201399C8FC583DDDC0D970A8141A9B601E9F6AC75B26210C8D7B21CC0FB4`,
  `38,433,230` bytes, source
  `tree-sha256:cdd72c1aa9799946e25d8983a69fb539dcbcd293b6a983c504980e1fc8e25a1c`.
- No QApplication/Qt/EXE launch, screenshots, unit tests, mocks, fixtures,
  harnesses, test-only assets, deployment, or hardware operation were run.

## Limits and disposition

Static source evidence cannot prove native Qt event ordering or filesystem
case behavior on every Windows volume. The slice is accepted with those
limits; remaining runtime and release gates stay open.
