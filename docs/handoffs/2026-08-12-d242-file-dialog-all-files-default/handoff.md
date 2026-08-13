# Handoff: 2026-08-12-d242-file-dialog-all-files-default

| Field | Value |
|---|---|
| ID | `2026-08-12-d242-file-dialog-all-files-default` |
| Delivery / slice | `D242 / ARCH-223 File dialog all-files default` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D242 / ARCH-223 handoff — file dialog all-files default

## User outcome

The existing Open File and Save dialogs now default to `All files (*)` while
retaining the localized text/source filter as an explicit secondary choice.
Extensionless files and common source/configuration files are no longer
hidden by the first filter, addressing the observed “only folders open”
confusion without creating a second document-open path.

## Scope and boundaries

Changed only the two localized `dialog.text_filter` values in
`src/quillforge/presentation/i18n.py`. `FileDialogSurface`, workspace folder
selection, tree file activation, asynchronous document admission, document
decoding, containment, locale ownership, and error policy remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Plato the 7th / Luna max architecture window: `NO_CONCLUSION` after bounded
  wait.
- Poincare the 7th / Luna max narrowed architecture window: `NO_CONCLUSION`
  after bounded wait.
- Raman the 7th / Luna max independent review window: `NO_CONCLUSION` after
  bounded wait; no child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — reorder the existing localized
  `dialog.text_filter` entries so `All files (*)` is the default while the
  text/source filter remains available.
- `docs/adr/0286-file-dialog-all-files-default.md` — record the decision and
  Qt public-source applicability.
- `docs/agent-team/reviews/` — record parent and unresolved independent review.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, `tasks/plan.md`, and `tasks/todo.md`
  — bind the increment to the project evidence register.

## Decisions and constraints

The existing shared filter key remains the single presentation owner for open
and save dialogs. `All files (*)` is first because the editor must not hide a
user's file merely because its extension is outside a finite allowlist. The
filter is a visibility affordance, not a security or path-containment
boundary. No `QApplication` or native dialog was constructed for this change.

## Verification commands and results

| Evidence | Result |
|---|---|
| Python AST | `D242-AST=PASS` |
| Python compile | `D242-COMPILEALL=PASS` |
| Ruff | `D242-RUFF=PASS` |
| Ruff format | `D242-FORMAT=PASS` |
| Filter source contract | `D242-FILE-FILTER-CONTRACT=PASS` |
| QtCore wildcard contract | `D242-QT-WILDCARD-CONTRACT=PASS` |
| Extensionless regression probe | `D242-EXTENSIONLESS-REGRESSION-PROBE=PASS` |
| Source no-window startup diagnostic | `D242-SOURCE-STARTUP-DIAGNOSTIC=PASS` |
| PS5.1 package build | `PASS`, SHA `C47A2B6E56FE2739AE5B39FD1A6C114E4D061EC04E79B3CAE5FF87EA7E2A310B` during the first shell build |
| PS7 package build | `PASS`, final SHA `DF7F364FA81AF057F8EC2E076A0FF6D6D9123184C527C806F8B7215B8DD867DE` |
| Frozen resources | `D242-FROZEN-RESOURCES=PASS`; qwindows.dll and icon present |
| Frozen application module | `D242-FROZEN-APP-MODULE=PASS` |
| PyInstaller warnings | `D242-PYINSTALLER-WARNING-SCOPE=PASS` |
| Package identity | `D242-PACKAGE-IDENTITY=PASS`, 38,575,619 bytes, dist/root match |

## Unrun checks and reason

Native EXE/Qt startup, native file/folder dialogs, Windows shell rendering,
clean-machine startup, signing, installer/update/rollback, registry,
release-owner checks, and visual accessibility review remain unrun under the
permanent no-launch/non-destructive policy. No unit-test assets were created
or run.

## Known risks and limits

The default filter now exposes arbitrary files. The document service may still
reject undecodable/binary content and reports that failure through the existing
error path; this change does not promise binary editing. Native dialog
metrics, filter presentation, and shell behavior remain runtime evidence
gates.

## Acceptance and evidence IDs

`S290`, `ARCH-223`, `D242-FILE-FILTER-CONTRACT=PASS`,
`D242-QT-WILDCARD-CONTRACT=PASS`, `D242-PACKAGE-IDENTITY=PASS`,
`D242-SIMPLIFICATION-ASSESSMENT=PASS`,
`D242-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D242-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native file-dialog and
startup run. This handoff does not authorize it.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `DF7F364FA81AF057F8EC2E076A0FF6D6D9123184C527C806F8B7215B8DD867DE`
- Bytes: `38,575,619`
- Source revision: `tree-sha256:d5eb8ac29486484553871513d0131fc983b4b4e4c52e18e3b97be82dda44abb2`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The default visibility defect is closed statically and
the package is rebuilt; native dialog/startup and release evidence remain
open.
