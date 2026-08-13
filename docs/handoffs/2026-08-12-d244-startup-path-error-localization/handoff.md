# Handoff: 2026-08-12-d244-startup-path-error-localization

| Field | Value |
|---|---|
| ID | `2026-08-12-d244-startup-path-error-localization` |
| Delivery / slice | `D244 / UI-26 Startup-path error localization` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D244 / UI-26 handoff — startup-path error localization

## User outcome

When a file or folder path supplied to the desktop launch path cannot be
opened, the Chinese locale now presents a Chinese failure prefix instead of
leaking the English `Cannot open path:` or `Unable to open path:` text. The
selected path and underlying error detail remain visible. English behavior is
unchanged.

## Scope and boundaries

Changed only the existing `prefixes` table in
`src/quillforge/presentation/i18n.py`. Startup argument parsing, file/folder
classification, asynchronous document opening, workspace routing, settings,
and application services remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Pasteur the 7th / Luna max architecture window: `NO_CONCLUSION`.
- Cicero the 7th / Luna max independent review window: `NO_CONCLUSION`; no
  child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/i18n.py` — add two existing-prefix mappings.
- `docs/adr/0288-startup-path-error-localization.md` — record the decision and
  public-source applicability.
- `docs/agent-team/reviews/` — record parent and unresolved independent
  review.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, `docs/RELEASE_HANDOFF.md`,
  `tasks/plan.md`, and `tasks/todo.md` — bind the increment to evidence.

## Decisions and constraints

The two mappings remain in the existing ordered prefix table because the
status and message surfaces already persist source messages and reproject
them when locale changes. Translation is intentionally limited to the stable
prefix; path and exception suffixes remain untouched. No `QApplication`, native
dialog, or launch parser was constructed for this change.

## Verification commands and results

| Evidence | Result |
|---|---|
| Python compile | `D244-COMPILEALL=PASS` |
| Ruff | `D244-RUFF=PASS` |
| Ruff format | `D244-FORMAT=PASS` |
| Focused locale probe | `D244-STARTUP-PATH-LOCALIZATION=PASS` |
| PS5.1 package build | `PASS`, SHA `016B0F974E0EA3B975B60DD729F4DD33A4B339ACB8912772F6754B03D1B7BF4D` during the first shell build |
| PS7 package build | `PASS`, final SHA `35A0D6EDDDE7E5BF0BB79E7FCA1EFD30B6580C29062CFB5D8821599213E5ECE7` |
| Package identity | `D244-PACKAGE-IDENTITY=PASS`, 38,576,104 bytes, dist/root match |
| Recursive archive | `D244-ARCHIVE-__main__=PASS`; app/composition/PyQt6/qwindows.dll present |
| Expected release verifier | `D244-RELEASE-VERIFY=EXPECTED-NO-GO`; three artifact-bound runtime consistency gates remain open |

## Unrun checks and reason

Native EXE/Qt startup, desktop file association launch, native file dialog,
clean-machine startup, signing, installer, updater/rollback, registry, and
external release-owner checks remain unrun under the permanent
no-launch/non-destructive policy. No unit-test assets were created or run.

## Known risks and limits

Only the user-facing prefix is translated; exception text from Windows or the
filesystem may remain in its source language by design so diagnostics are not
lost. This change does not make unsupported, missing, or unreadable files
openable.

## Acceptance and evidence IDs

`S292`, `UI-26`, `D244-STARTUP-PATH-LOCALIZATION=PASS`,
`D244-PACKAGE-IDENTITY=PASS`, `D244-SIMPLIFICATION-ASSESSMENT=PASS`,
`D244-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D244-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native startup and desktop
file-association review. This handoff does not authorize launching the
application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `35A0D6EDDDE7E5BF0BB79E7FCA1EFD30B6580C29062CFB5D8821599213E5ECE7`
- Bytes: `38,576,104`
- Source revision: `tree-sha256:66d2d6529b40ab77549dd4c278651a83f588cdf29c4d399fc7c42eceb9d69ba1`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The explicit desktop-launch localization gap is closed
statically and the package is rebuilt; native startup and release evidence
remain open.
