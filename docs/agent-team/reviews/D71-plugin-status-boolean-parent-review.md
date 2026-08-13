# D71 parent review — plugin status boolean locale projection

| Field | Value |
|---|---|
| Delivery | `D71 / UI-44` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

The Plugin Status tooltip now projects enabled/active booleans through the
centralized locale catalog, with English and Chinese values, while all plugin
status contracts and lifecycle controls remain unchanged.

## Parent multi-axis review

- **Correctness:** PASS by source inspection and pure formatting probe. Both
  boolean states are covered in both locales; IDs, versions, permissions, and
  other tooltip values remain unchanged.
- **Readability/simplicity:** PASS. Two i18n values and two local variables
  make the projection explicit without a new helper or state holder.
- **Architecture:** PASS with limits. Locale remains in presentation; the
  application `PluginRuntimeStatus` dataclass is untouched.
- **Security:** PASS by scope. No plugin lifecycle, trust, filesystem,
  process, or permission operation changes.
- **Performance:** PASS by scope. The change formats two strings per tooltip;
  no new work, storage, or event path is introduced.

## Simplification assessment

No further safe simplification was identified. Reusing lifecycle labels for
booleans would conflate state words with truth values, while moving this into
the application contract would violate the locale boundary.

## Authorized non-destructive validation

- `D71-PLUGIN-STATUS-LOCALE-PROBE=PASS`.
- Targeted compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist hash identity — `PASS`.
- No QApplication/Qt startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native tooltip rendering, live locale switching, accessibility, callback
timing, runtime startup, clean-machine, cross-machine, and release-owner
evidence remain open. Embedded C/C++ and vendor-public-source requirements
are N/A.
