# Handoff: 2026-08-12-d279-font-availability-status

| Field | Value |
|---|---|
| ID | `2026-08-12-d279-font-availability-status` |
| Delivery / slice | `D279 / ARCH-249 Font availability status projection` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The Settings dialog now explains whether the selected interface and editor
font families are installed, will use the system fallback, or could not be
queried. Font family, size, style, locale, theme, accent, and motion settings
remain selectable and persist through the existing validated snapshot.

## Scope and boundaries

- Added presentation-only `QFontDatabase.families()` status projection.
- Added English and Simplified Chinese status strings.
- Added a Qt-free source contract and probes for localization and persistence
  invariants.
- No application/domain/settings-store schema or runtime editor policy changed.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Popper consultation | Boundary, ownership, and integration review |
| Developer | parent | Settings presentation and localization slice |
| QA | parent | Static, source-diagnostic, archive, package, and handoff verification |
| Independent reviewer | Curie consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/presentation/settings_dialog.py` — derived installed/fallback status.
- `src/quillforge/presentation/i18n.py` — bilingual status catalog.
- `scripts/audit_presentation_contracts.py` — settings/font status contract.
- D279 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Keep supported font values and persistence in application/infrastructure;
  keep platform enumeration in presentation.
- Preserve raw combo family values in `SettingsSnapshot`; status text is never
  an input value.
- Shared checkout writer: parent agent, limited to the listed files.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, and package evidence was authorized.

## Public-source applicability

Qt 6 first-party `QFontDatabase::families()` documentation is applicable to
installed-family enumeration. Python 3.12 `frozenset` and RuntimeError
behavior are runtime references. No manufacturer requirement applies; no
private ByteDance standard, certification, MISRA, ISO 26262, ASPICE, or
embedded claim is made. Embedded workflow and embedded simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Font status localization | `D279-FONT-STATUS-LOCALIZATION=PASS locales=2 placeholders=4` |
| Persistence boundary | `D279-FONT-PERSISTENCE-CONTRACT=PASS raw_family_values=2 status_not_persisted=1` |
| Presentation audit | `PASS` |
| Compileall | `PASS` |
| Ruff and formatting | `PASS` |
| Source startup diagnostic | `D279-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed failed=0` |
| PE header | `D279-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI` |
| Frozen archive | `D279-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=8 embedded_settings_dialog=True` |
| Root/dist identity | `D279-MANIFEST-COPY=PASS bytes=38585345 sha=A4E1AD249D3131407F27CACD621AD24AD4BF4C43A49E371BA30B1749CB4F79C8` |
| Source revision | `tree-sha256:0f9aed11ab80f40aa92f6a63e7e5121bb995dd1106ecf1581f2f0fd3c3f4a51f` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded wait and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, native font enumeration/rendering, font metrics, DPI,
accessibility, clean-machine behavior, signing, installer/update, registry,
cross-machine repeatability, and release-owner acceptance remain unrun under
the active no-launch and non-destructive policy.

## Known risks and limits

The status is based on the current platform catalog at dialog creation and
does not prove that every requested font style/weight is available in that
family. Qt may still perform its own fallback for a missing style or glyph;
the existing generic note remains visible.

## Acceptance and evidence IDs

- Acceptance: `S319`.
- Architecture slice: `ARCH-249`.
- Evidence: `D279-FONT-STATUS-LOCALIZATION=PASS`,
  `D279-FONT-PERSISTENCE-CONTRACT=PASS`, `D279-CHECK=PASS`,
  `D279-MANIFEST-IDENTITY=PASS`,
  `D279-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D279-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D279-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: open Settings on the target machine and confirm the status line
  matches installed fonts and remains legible in both themes/locales.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size:
  `A4E1AD249D3131407F27CACD621AD24AD4BF4C43A49E371BA30B1749CB4F79C8` /
  `38,585,345` bytes.
- Source revision:
  `tree-sha256:0f9aed11ab80f40aa92f6a63e7e5121bb995dd1106ecf1581f2f0fd3c3f4a51f`.

## Disposition

`accepted-with-limits`: font availability is now visible and safely derived;
native font rendering and enterprise release gates remain open.
