# Handoff: 2026-08-11-d210-dialog-filter

| Field | Value |
|---|---|
| ID | `2026-08-11-d210-dialog-filter` |
| Delivery / slice | `D210 / UI-112 / ARCH-195 Default document filter coverage` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:59:00+08:00` |

## User outcome

Open and Save dialogs now show a broader set of common text/source files by
default in both English and Simplified Chinese. The existing All files fallback
remains available, and the same async document-open/save architecture is used.

## Scope and boundaries

### In scope

- One localized `dialog.text_filter` catalog entry in each supported locale.
- Static filter grammar/coverage source proof.
- Compile, presentation, package, and release-boundary records.

### Out of scope

- Native file-dialog implementation, file decoding, persistence, async
  dispatch, workspace directory selection, GUI/QApplication, EXE startup,
  screenshots, accessibility, DPI, unit-test assets, and release closure.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, boundary decision, final review, handoff |
| Project Manager | `parent` | Plan, dependency, risk, and status record |
| Product | `user outcome` | Make common files discoverable in the file picker |
| Developer | `parent` | Localized filter catalog implementation |
| QA | `parent` | Static, package, and release-boundary checks |

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — English/Chinese name-filter catalog.
- Synchronized ADR, reviews, acceptance, register, roadmap, and handoff files.

## Decisions and constraints

- Shared checkout writer: `parent`; no worktree or second writer was used.
- Preserve the existing open/save/folder dialog and async document boundaries.
- Architecture window: `Beauvoir the 6th / Luna max` — `NO_CONCLUSION` after
  two bounded waits and closure; no child architecture PASS is claimed.
- Independent review: `Noether the 6th / Luna max` — `NO_CONCLUSION` after two
  bounded waits and closure; no independent PASS is claimed.
- Parent review: `PASS`; simplification assessment: `PASS`.
- Embedded C/C++, MCU, RTOS, and manufacturer requirements are not applicable.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Bilingual dialog-filter probe | `PASS` | 2 locales, 18 extensions, All files fallback present. |
| Compile | `PASS` | `D210-COMPILEALL=PASS`. |
| Ruff | `PASS` | `D210-RUFF=PASS`. |
| Format | `PASS` | `D210-FORMAT=PASS`. |
| Presentation contract audit | `PASS` | `D210-PRESENTATION-AUDIT=PASS`. |
| Windows PowerShell 5.1 package | `PASS` | SHA `E5FBC3840C137224B1E3A6AF4D8A07DE297FF92E167603DEC6EB482F3DB7027A`, 38,562,187 bytes. |
| PowerShell 7 package | `PASS` | Final candidate SHA `D1DC99EF25838F13432F3013269EAB970EA8B86C7E43309C6EAEBAAB7A638FCA`, 38,563,338 bytes. |
| Root/dist/package identity | `PASS` | `D210-PACKAGE-IDENTITY-PROBE=PASS`; root and dist match. |

## Unrun checks and reason

GUI/QApplication, native file dialog, screenshots, accessibility tree, DPI,
live file open/save, clean-machine, cross-machine, signing, installer/updater,
legal, support, permission/disk-pressure, hard-power, and release-owner
checks were not run under the active no-launch or external-authorization
policy. No unit-test asset was created or run.

## Known risks and limits

- Native filter rendering and case sensitivity vary by platform; only source
  grammar and coverage were statically verified.
- The checkout has no Git baseline, and both delegated review windows returned
  `NO_CONCLUSION`; parent review is the only PASS review claim.
- Separate PyInstaller invocations can differ in artifact bytes; each manifest
  binds its own artifact. The final PS7 candidate is the current identity.

## Acceptance and evidence IDs

- Acceptance: `S261`
- Evidence: `D210-DIALOG-FILTER-PROBE=PASS locales=2 extensions=18 all-files-fallback=present`,
  `D210-COMPILEALL=PASS`, `D210-RUFF=PASS`, `D210-FORMAT=PASS`,
  `D210-PRESENTATION-AUDIT=PASS`, `D210-PACKAGE-BUILD-PS51=PASS`,
  `D210-PACKAGE-BUILD-PS7=PASS`, `D210-PACKAGE-IDENTITY-PROBE=PASS`,
  `D210-SIMPLIFICATION-ASSESSMENT=PASS`,
  `D210-ARCHITECT-REVIEW=NO_CONCLUSION`,
  `D210-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

- Owner: Architect / QA.
- Action: continue user-feature and visual-state audit while keeping native
  dialog and document-service ownership stable.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `D1DC99EF25838F13432F3013269EAB970EA8B86C7E43309C6EAEBAAB7A638FCA` /
  `38,563,338` bytes
- Source revision: `tree-sha256:6e1172d4d768ea969c2f0b85416ee0cdc7db8f508f15774f49aa2d5d3594f88d`
- PS5 package evidence: `E5FBC3840C137224B1E3A6AF4D8A07DE297FF92E167603DEC6EB482F3DB7027A` /
  `38,562,187` bytes; final current candidate is the PS7 package.

## Disposition

`accepted-with-limits`: common text/source files are discoverable in the
existing native file picker without changing document behavior; native dialog
rendering and enterprise release gates remain open.

## Required final marker

`EMBEDDED_WORKFLOW_COMPLETE: review + simplification + verification recorded`
