# D243 parent review — readable accent text endpoints

## Decision

`PASS` with operational limits.

## Findings

- The source change is confined to `src/quillforge/presentation/theme.py`.
- One private pure resolver owns text-on-surface handling for `accent_alt`;
  the palette and generated QSS projection call it rather than defining
  separate thresholds.
- The resolver uses `surface_3` as a conservative shared background and
  `text_primary` as the existing readable fallback. This removes the
  confirmed paper-sand low-contrast cases without changing deep-theme accent
  behavior.
- Filled action backgrounds still use their existing `on_accent*` pairs, so
  the reported 砂金/amber filled-control behavior is not conflated with
  surface text behavior.
- No application/service, document-open path, settings contract, or Qt
  lifecycle behavior changed.

## Evidence

- `D243-COMPILEALL=PASS`
- `D243-RUFF=PASS`
- `D243-FORMAT=PASS`
- `D243-THEME-ENDPOINT-CONTRAST=PASS min=9.207`
- `D243-DIRECT-ACCENT-TEXT-PROBE=PASS`
- `D243-QSS-SEMANTIC-ENDPOINTS=PASS`
- `D243-ARCHIVE-LIST=PASS`; qwindows.dll, PyQt6, and quillforge entries
  present
- `D243-PACKAGE-IDENTITY=PASS`; root/dist match, 38,573,642 bytes
- `D243-CHECK-PS51=PASS`; `D243-CHECK-PS7=PASS`
- `D243-HANDOFF-PS51=PASS`; `D243-HANDOFF-PS7=PASS`

## Simplification assessment

`D243-SIMPLIFICATION-ASSESSMENT=PASS`: extracting the existing four-line
foreground expression into one private pure resolver removes duplicate
contrast policy without introducing a new token field, service, abstraction,
or behavior branch. No unrelated cleanup was mixed into the change.

## Architecture consultation and independent review

- Faraday the 7th / Luna max architecture window for the initial semantic
  endpoint patch: `NO_CONCLUSION` after a bounded wait.
- Fermat the 7th / Luna max architecture window for the helper extraction:
  `NO_CONCLUSION` after a bounded wait.
- Linnaeus the 7th / Luna max independent review window: `NO_CONCLUSION`
  after a bounded wait; the window was closed without treating silence as
  approval.
- Parent review: `PASS`; simplification: `PASS`.

## Limits

No EXE/Qt launch, updater/installer/registry operation, unit-test asset,
mock, fixture, harness, worktree, or test-only asset was executed or created.
Static contrast does not prove native font rasterization, DPI metrics,
screen-reader output, or Windows desktop rendering.
