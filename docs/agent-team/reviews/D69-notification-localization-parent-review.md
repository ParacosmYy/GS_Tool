# D69 parent review — notification localization closure

| Field | Value |
|---|---|
| Delivery | `D69 / UI-42` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

`presentation.i18n` now localizes the remaining known mixed-language shell
notifications and structured plugin summaries. `PluginCatalogDialog` retains
the immutable application summary source and re-projects it when the locale
changes.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. `en-US` returns the source
  unchanged; Chinese output localizes stable labels while retaining dynamic
  diagnostic values. Existing exact/prefix messages remain intact.
- **Readability/simplicity:** PASS. Three bounded parsers are private to the
  existing presentation catalog; no new service, locale contract, or
  application-layer dependency is introduced.
- **Architecture:** PASS with limits. Application `summary()` methods remain
  locale-free; Qt dialog retranslation remains in `PluginCatalogDialog`; the
  existing StatusSurface/i18n projection seam remains the single notification
  owner.
- **Security:** PASS by scope. Plugin trust, approval, execution, filesystem,
  process, and persistence policy are unchanged; raw diagnostics are not
  executed or interpreted as commands.
- **Performance:** PASS by scope. Regexes are bounded to known summary shapes
  and run only at notification/dialog projection time; no worker, polling, or
  retained service is added.

## Simplification assessment

No further safe simplification was identified. A prefix-only change would
leave structured summaries mixed-language, while moving locale handling into
application summaries would violate the presentation boundary. The three
private parsers keep each stable summary shape explicit and preserve dynamic
diagnostic details.

## Authorized non-destructive validation

- `D69-LOCALIZATION-PROBE=PASS`.
- Compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist hash identity — `PASS`.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native dialog rendering, live locale switching, keyboard traversal,
accessibility, callback timing, runtime startup, clean-machine, cross-machine,
and release-owner evidence remain open. Embedded C/C++ and vendor-public-source
requirements are N/A.
