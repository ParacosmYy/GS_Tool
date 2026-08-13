# ADR-0291: Typed secondary error localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D247 / ARCH-225

## Context

The open/save coordinators already passed typed exceptions to the shared
`MessageSurface`, which allowed the Chinese catalog to retain filesystem and
codec context. Two neighboring presentation paths still called `str(error)`
before reaching that surface: settings persistence failure and Replace All
failure. That conversion made a typed `PermissionError`, `FileNotFoundError`,
or codec exception indistinguishable from an arbitrary message and caused
Chinese feedback to fall back to the original English exception text.

## Decision

Keep error policy in `MainWindow` and keep translation/category handling in
the existing `MessageSurface`/`i18n` boundary. Pass the existing `Exception`
object directly from:

- `_show_settings_save_failed()`;
- the Replace All exception handler.

No new exception hierarchy, service, translator, callback, persistence field,
or dialog surface is introduced. English behavior remains the existing
`str(error)` result, while `zh-CN` can use the typed exception category and
preserve path/encoding/position details.

## Boundaries and alternatives

- The change is presentation-only; settings, document, replace, and worker
  services retain their existing contracts and ownership.
- The shared error surface remains the single localization endpoint; adding
  per-call translation tables would duplicate policy and increase coupling.
- Passing an exception object does not expose traceback state or mutate the
  exception; it only preserves type information until the modal projection.
- No EXE/Qt launch, registry, installer, updater, or unit-test asset is used.

## Public-source applicability and review

Python 3.12 first-party documentation for
[built-in exceptions](https://docs.python.org/3.12/library/exceptions.html)
and [Unicode exceptions](https://docs.python.org/3.12/library/exceptions.html#UnicodeError)
is the applicable engineering reference for preserving exception types and
codec metadata. No Qt API or manufacturer contract changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or manufacturer
requirement is claimed. Embedded C/C++, MCU, RTOS, and related public-source
workflows are not applicable.

The architecture role `Volta the 7th / Luna max` returned `NO_CONCLUSION`
after a bounded wait and closure. The independent reviewer `Euclid the 7th /
Luna max` likewise returned `NO_CONCLUSION` after a bounded wait and closure.
No child approval is claimed. Parent review is `PASS`; behavior-preserving
simplification is `PASS`.

## Evidence and limits

- `D247-TYPED-ERROR-BOUNDARY=PASS`,
  `D247-SETTINGS-ERROR-LOCALIZATION=PASS`, and
  `D247-REPLACE-ERROR-LOCALIZATION=PASS`.
- `D247-COMPILEALL=PASS`, `D247-RUFF=PASS`, and `D247-FORMAT=PASS`.
- PS5.1 and PS7 package builds passed; final PS7/root artifact identity is
  SHA-256 `C7D8AD797BA785C033CCD205156CF79E21F228E664CBB91CD02C2C6F82B92729`,
  38,577,691 bytes, source revision
  `tree-sha256:761f83cf30977e6cf45f9fb6e8210db5fec49c77ac8e96aad2240561841b5447`.
- The frozen archive contains the `__main__` outer entry,
  `quillforge.app`, `quillforge.composition`,
  `quillforge.presentation.main_window`, PyQt6, and `qwindows.dll`; the
  warning file has 25 non-empty lines.

Native EXE/Qt startup, native dialogs, shell association launch, clean-machine
behavior, signing, installer, updater, support, and release-owner evidence
remain open under the active no-launch/non-destructive policy.
