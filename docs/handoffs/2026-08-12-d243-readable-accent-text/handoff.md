# Handoff: 2026-08-12-d243-readable-accent-text

| Field | Value |
|---|---|
| ID | `2026-08-12-d243-readable-accent-text` |
| Delivery / slice | `D243 / UI-25 Readable accent text endpoints` |
| Status | `accepted-with-limits` |
| Owner | `architect` |

# D243 / UI-25 handoff — readable accent text endpoints

## User outcome

Accent-colored text no longer becomes low-contrast on light surfaces. Links,
the workspace eyebrow, and generic checkbox hover text now use a shared
readability resolver. Filled accent controls, including the 砂金/amber action
pair, retain their existing explicit foreground tokens.

## Scope and boundaries

Changed only `src/quillforge/presentation/theme.py`. The new private helper
is presentation-only and uses the existing `readable_foreground` contract.
Document opening, file dialogs, locale, settings persistence, application
services, Qt lifecycle, and filled accent control roles remain unchanged.

## Team roles and ownership

- Parent architect: integration, review, simplification, package, and handoff.
- Faraday the 7th / Luna max initial architecture window: `NO_CONCLUSION`.
- Fermat the 7th / Luna max helper-extraction architecture window:
  `NO_CONCLUSION`.
- Linnaeus the 7th / Luna max independent review window: `NO_CONCLUSION`;
  no child PASS is claimed.
- Parent review: `PASS`; simplification: `PASS`.

## Changed files and modules

- `src/quillforge/presentation/theme.py` — centralize readable
  `accent_alt` text resolution and apply it to palette links, workspace
  eyebrow, and generic checkbox hover text.
- `docs/adr/0287-readable-accent-text-endpoints.md` — record the decision and
  public-source applicability.
- `docs/agent-team/reviews/` — record parent and unresolved independent
  review.
- `docs/agent-team/acceptance.json`, `docs/agent-team/delivery-register.json`,
  `docs/ARCHITECTURE.md`, `docs/ROADMAP.md`, `docs/RELEASE_HANDOFF.md`,
  `tasks/plan.md`, and `tasks/todo.md` — bind the increment to evidence.

## Decisions and constraints

`accent_alt` remains available for decorative borders and filled states. Only
surface text uses the conservative foreground helper, with `surface_3` as the
worst-case shared background and `text_primary` as fallback. No `QApplication`
or native dialog was constructed for this change.

## Verification commands and results

| Evidence | Result |
|---|---|
| Python compile | `D243-COMPILEALL=PASS` |
| Ruff | `D243-RUFF=PASS` |
| Ruff format | `D243-FORMAT=PASS` |
| Theme endpoint contrast | `D243-THEME-ENDPOINT-CONTRAST=PASS min=9.207` |
| Direct accent text probe | `D243-DIRECT-ACCENT-TEXT-PROBE=PASS` |
| QSS semantic endpoint matrix | `D243-QSS-SEMANTIC-ENDPOINTS=PASS` |
| PS5.1 package build | `PASS`, SHA `B1FCE515A36C83CAEDF23D876CAFF3316B6F255323BEC75A553A142AC9CE9F88` during the first shell build |
| PS7 package build | `PASS`, final SHA `AF63CEBE385F4DDBA1174C4D71847550BB6FB42D0E417856F71E7B4D3E63B6B0` |
| Package identity | `D243-PACKAGE-IDENTITY=PASS`, 38,573,642 bytes, dist/root match |
| Archive/resources | `D243-ARCHIVE-LIST=PASS`; qwindows.dll, PyQt6, and quillforge entries present |
| PS5.1 project checks | `D243-CHECK-PS51=PASS`; `D243-HANDOFF-PS51=PASS` |
| PS7 project checks | `D243-CHECK-PS7=PASS`; `D243-HANDOFF-PS7=PASS` |
| Expected release verifier | `D243-RELEASE-VERIFY=EXPECTED-NO-GO`; three artifact-bound runtime consistency gates remain open |

## Unrun checks and reason

Native EXE/Qt startup, native link/hover rendering, Windows font metrics,
screen-reader output, clean-machine startup, signing, installer,
updater/rollback, registry, and external release-owner checks remain unrun
under the permanent no-launch/non-destructive policy. No unit-test assets were
created or run.

## Known risks and limits

The fallback intentionally trades the accent hue for readable primary text
when a light accent is placed on a shared surface. This is a surface-text
policy only; it does not claim native Qt rendering, installed-font coverage,
DPI behavior, screen-reader output, or clean-machine startup.

## Acceptance and evidence IDs

`S291`, `UI-25`, `D243-THEME-ENDPOINT-CONTRAST=PASS`,
`D243-PACKAGE-IDENTITY=PASS`, `D243-SIMPLIFICATION-ASSESSMENT=PASS`,
`D243-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
`D243-INDEPENDENT-REVIEW=NO_CONCLUSION`.

## Next owner and next action

Project Manager owns the next explicitly authorized native rendering and
startup review. This handoff does not authorize launching the application.

## Artifact information

- `dist/QuillForge.exe` and `QuillForge.exe`
- SHA-256: `AF63CEBE385F4DDBA1174C4D71847550BB6FB42D0E417856F71E7B4D3E63B6B0`
- Bytes: `38,573,642`
- Source revision: `tree-sha256:27d841ef539a0de2e6e9cfc13876cf0a7b1ed4e0c78618705fbac2c61b39ddf6`
- Portable manifest association status: `not-configured`

## Disposition

Accepted with limits. The confirmed surface-text contrast defect is closed
statically and the package is rebuilt; native rendering/startup and release
evidence remain open.
