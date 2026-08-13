# D73 parent review — settings appearance preview

| Field | Value |
|---|---|
| Delivery | `D73 / UI-46` |
| Decision | `accepted-with-limits` |
| Owner | `Architect` |
| Checkout | Current local checkout only |

## Scope

The Settings dialog now projects a pending appearance preview card from the
existing theme token contract. It updates on theme, accent, interface font, and
interface-size changes and retranslates with the dialog locale.

## Parent multi-axis review

- **Correctness:** PASS by source inspection. Preview updates are connected to
  the four appearance controls, uses current combo-box data, and never calls
  persistence or global theme application. Save/Cancel and returned snapshot
  semantics remain unchanged.
- **Readability/simplicity:** PASS with one correction applied. The preview
  uses a small named projection helper; the selected font and size are passed
  into the scoped stylesheet so the sample cannot silently fall back to the
  global font rule.
- **Architecture:** PASS with limits. `theme.py` remains the sole token/QSS
  owner; `SettingsDialog` owns only presentation and preview refresh. No Qt
  dependency enters application/domain code.
- **Security:** PASS by scope. Theme/font values come from the existing bounded
  combo catalogs; no file, process, plugin, or untrusted-data path is added.
- **Performance:** PASS by scope. Refresh work is a small stylesheet/text
  projection on user control changes; no timer, worker, image, or retained
  resource is introduced.

## Simplification assessment

No further safe simplification was identified. Removing the public token
resolver would force private-function coupling or duplicate colors; removing
the preview stylesheet would make pending theme colors invisible. A separate
preview model or custom paint widget would add concepts without a current
behavioral need.

## Authorized non-destructive validation

- `D73-PREVIEW-CONTRAST-PROBE=PASS`.
- Presentation compileall, Ruff, and format checks — `PASS`.
- Portable package build and root/dist identity — `PASS`.
- No `QApplication` startup, screenshots, unit tests, mocks, fixtures,
  harnesses, or test-only assets were used.

## Limits

Native Qt layout/rendering, installed-font fallback, keyboard traversal,
accessibility, DPI, runtime startup, clean-machine, cross-machine, signing,
legal, and release-owner evidence remain open. Embedded C/C++ and vendor
manufacturer-source requirements are N/A. The delegated Architect and
independent reviewer did not return conclusions; no child PASS is claimed.
