# D70 parent review — plugin catalog locale projection

| Field | Value |
|---|---|
| Delivery | `D70 / UI-43` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

The presentation catalog now contains stable plugin-catalog labels and known
execution-reason translations. Plugin Catalog rows, tooltips, and the empty
state are re-rendered from immutable entries when the locale changes.

## Parent multi-axis review

- **Correctness:** PASS by source inspection and pure formatting probe.
  English output keeps the prior vocabulary; Chinese output translates stable
  labels and preserves names, IDs, versions, paths, hashes, permissions,
  entrypoint values, and free-form reasons.
- **Readability/simplicity:** PASS. `catalog_value()` centralizes enum lookup
  and safely falls back to raw future values; row/tooltip formatting remains a
  small presentation helper with no new state machine.
- **Architecture:** PASS with limits. i18n owns vocabulary, the dialog owns
  Qt text projection, and application catalog entries remain immutable and
  locale-free. No signal or policy boundary moved.
- **Security:** PASS by scope. No descriptor is executed, parsed again, or
  treated as a command; translation only reads existing typed values.
- **Performance:** PASS by scope. Locale refresh formats existing in-memory
  rows only; it does not rescan the catalog or start a worker.

## Simplification assessment

No further safe simplification was identified. Keeping stable vocabulary in
the existing i18n bundle avoids per-dialog translations, while keeping the
formatters local avoids introducing a catalog view-model for a text-only
projection. Raw-value fallback protects forward compatibility for new enum
values.

## Authorized non-destructive validation

- `D70-CATALOG-LOCALE-PROBE=PASS`.
- `D70-CATALOG-VALUE-PROBE=PASS` — known values localize, unknown values
  fall back raw, and `en-US` keeps the source value.
- Targeted compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist hash identity — `PASS`.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native QListWidget rendering, live locale switching, accessibility, callback
timing, runtime startup, clean-machine, cross-machine, and release-owner
evidence remain open. Embedded C/C++ and vendor-public-source requirements
are N/A.
