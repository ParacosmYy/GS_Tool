# ADR-0289: Typed file-error localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D245 / UI-27

## Context

The existing file-open and file-save coordinators forwarded `str(error)` to
the shared error dialog. That preserved a useful Python diagnostic, but under
`zh-CN` filesystem failures such as `FileNotFoundError`, `PermissionError`,
`IsADirectoryError`, and codec failures remained entirely in English. The
result was a translated error title with an incomplete, hard-to-understand
failure body.

## Decision

Keep typed exception ownership in the existing document coordinators and pass
the exception object through the existing `MessageSurface` error port. Add one
presentation-only `localize_exception()` boundary in `i18n.py`:

- `en-US` returns `str(error)` unchanged;
- common filesystem errors receive stable Chinese summaries with their target
  path preserved;
- `UnicodeDecodeError`/`UnicodeEncodeError` report the codec, operation,
  byte/character position, and a bounded translation of common codec reasons;
- other `UnicodeError` instances use the existing string fallback so the
  error-projection path cannot fail while reporting an error;
- all other exceptions fall back to the existing `localize_message()` mapping.

The document store, application services, domain errors, async lifecycle, and
Qt event-loop policy remain unchanged. The same message surface is used for
open and save failures, so no second error-translation owner is introduced.

## Boundaries and alternatives

- The presentation layer does not catch or replace exceptions; it only
  projects already-classified failures.
- Unknown OS/codec detail remains intact rather than being guessed or
  discarded.
- No application-level error taxonomy or filesystem adapter rewrite is part
  of this slice.
- No EXE/Qt launch, registry, installer, updater, or unit-test asset is used.

## Public-source applicability and review

Python 3.12's public first-party documentation for
[built-in exceptions](https://docs.python.org/3.12/library/exceptions.html)
and [Unicode error attributes](https://docs.python.org/3.12/library/codecs.html)
is the applicable engineering reference for the preserved exception fields
and inheritance. No Qt API or manufacturer contract changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or manufacturer
requirement is claimed. Embedded C/C++, MCU, RTOS, and related public-source
workflows are not applicable.

The architecture role `Ohm the 7th / Luna max` returned `NO_CONCLUSION` after
a bounded wait and closure. The independent reviewer `Boyle the 7th / Luna
max` likewise returned `NO_CONCLUSION` after a bounded wait and closure. No
child approval is claimed. Parent review is `PASS`; behavior-preserving
simplification is `PASS`.

## Evidence and limits

- `D245-EXCEPTION-LOCALIZATION=PASS` for missing-path, permission,
  directory-as-file, decode, and encode examples.
- `D245-UNICODE-FALLBACK=PASS` for generic Unicode exception safety.
- `D245-CONFLICT-LOCALIZATION=PASS` for the existing external-change prefix.
- `D245-SOURCE-GATE=PASS`, compileall, Ruff, and format checks passed.
- `D245-CHECK-PS51=PASS`, `D245-CHECK-PS7=PASS`,
  `D245-HANDOFF-PS51=PASS`, and `D245-HANDOFF-PS7=PASS`.
- PS5.1 package build passed with intermediate SHA
  `B357BDD4088BB3D6B2528FFC0FC1892288528C6A17DD899E89B5A8277027E377`.
- PS7 package build passed with final SHA
  `C0DB69ED66CF38F9CA26F23C60335E0419449D064F7ED9A4FE8235F343DF3B63`,
  38,576,075 bytes; root and `dist` copies match.
- The frozen archive contains `__main__`, `quillforge.app`,
  `quillforge.composition`, PyQt6, and `qwindows.dll`; the warning file has
  25 non-empty lines and remains within the established packaging scope.
- `D245-RELEASE-VERIFY=EXPECTED-NO-GO` with 10 open gates and the three
  expected artifact-bound report consistency failures;
  `D245-FINAL-DOSSIER-BINDING=PASS` and `D245-FINAL-JSON-PARSE=PASS`.

Native dialog rendering, actual file I/O through the GUI, EXE/Qt startup,
clean-machine behavior, Windows shell associations, signing, installer,
updater, support, and release-owner evidence remain open under the active
no-launch/non-destructive policy.
