# D30 parent review — authored vector iconography

| Field | Value |
|---|---|
| Slice | D30 / UI-16 authored vector iconography |
| Reviewer | Architect (parent integration review) |
| Architecture reviewer | Boole the 2nd / Luna max; direction accepted with explicit follow-up boundary |
| Independent reviewer | Franklin the 2nd / Luna max; bounded read-only windows returned no conclusion; no PASS is claimed |
| Scope | `icons.py`, `command_surface.py`, `workspace_panel.py`, `workspace_surface.py`, `main_window.py`, ADR/contract records |
| Decision | accepted-with-limits; parent static review plus explicit independent no-conclusion record |

## Outcome

QuillForge now uses one presentation-only `IconKey`/`themed_icon` provider for
the command rail and workspace navigation. The provider draws small authored
vector glyphs with normal and disabled states from the current palette. The
command rail assigns semantic icons to new/open/save/find/replace/command
palette/workspace actions, and the workspace panel uses document/folder/
warning plus navigation icons. The platform-dependent `QStyle.StandardPixmap`
path is gone from the presentation layer.

The primary workspace action uses the palette's `on_accent`-equivalent
`BrightText` for its normal icon, while disabled icons use the disabled text
palette directly. This keeps vector state aligned with the centralized
primary-action QSS instead of relying on an additional alpha reduction.

Toolbar labels, callbacks, shortcuts, command IDs, locale strings, workspace
directory data, file-first-click behavior, folder-double-click behavior, and
MainWindow application policy are unchanged.

## Parent review findings

### Architecture and behavior

- PASS by static reasoning: `IconKey` and `themed_icon` have no application,
  domain, filesystem, persistence, plugin, or command-registry dependency.
- PASS by static reasoning: `CommandSurface` owns toolbar icon projection and
  retints icons from `ButtonText`/`Link` after `retranslate`; it does not own
  command execution or lifecycle policy.
- PASS by static reasoning: `WorkspacePanel` owns visible entry/button icon
  projection, while `WorkspaceSurface.refresh_icons()` exposes an explicit
  visual-only route. Theme refresh no longer relies on a locale side effect.
- PASS by static reasoning: MainWindow only selects semantic icon metadata and
  invokes the existing surface locale/theme projection; document, task,
  recovery, close, and error policies remain in MainWindow.
- PASS by source probe: no `QStyle.StandardPixmap`, `standardIcon`, or `QStyle`
  reference remains in `src/quillforge/presentation`.

### Visual contract and risk

- PASS by source reasoning: normal and disabled `QIcon` states use the same
  semantic key and palette-derived foreground/accent tint; primary-action
  normal state uses `on_accent` through `BrightText`, and disabled state uses
  the Disabled text role.
- PASS by source reasoning: fixed 22px authored geometry is rendered into an
  icon pixmap and displayed by the existing 18px toolbar/tree contracts; no
  external bitmap download or font glyph dependency was added.
- NOT RUNTIME-VERIFIED: Qt painting/API behavior, high-DPI scaling, native
  metrics, disabled alpha appearance, screenshots, screen-reader output, and
  cross-machine rendering remain unrun under the no-launch policy.
- PASS by static probe: the D30 disabled-icon palette probe confirms explicit
  disabled foreground/accent routes for command and workspace icons.

## Independent architecture/code review

Boole the 2nd / Luna max reviewed the slice read-only and concluded that the
icon surface is an appropriate independent D30/UI-16 boundary. The reviewer
recommended the explicit `WorkspaceSurface.refresh_icons()` route; that
follow-up was applied before this review record was finalized.

The follow-up review returned **PASS** for the explicit facade route: the
workspace surface only forwards the visual projection, `set_locale()` now
updates text only, and MainWindow invokes icon refresh after the theme/locale
shell projection. The reviewer noted only a non-blocking naming concern that
`_retranslate_ui()` also triggers theme icon projection.

Franklin the 2nd / Luna max was assigned an independent read-only review with
no Qt launch, no test creation/run, and no write access. Its bounded review
windows returned no conclusion and the reviewer was closed. No child PASS or
FAIL is claimed; acceptance relies on the parent static review and the
explicit limitation.

The final Boole follow-up window for the disabled-state refinement returned no
new conclusion after the static probe; no additional child PASS is claimed.

## Simplification assessment

The provider removes duplicated platform-icon lookup and keeps all authored
geometry in one presentation module. The explicit workspace refresh method is
smaller and clearer than coupling icon refresh to locale projection. Further
merging the provider into `MainWindow`, `CommandSurface`, or application
commands would reduce cohesion or leak presentation concerns; no additional
safe behavior-preserving simplification is required for D30.

## Public-source applicability and embedded gate

This slice is Python/PyQt6 desktop code, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are **N/A**
for MCU/vendor constraints because no firmware target, SDK, RTOS, ISR/DMA,
driver, protocol, boot, Flash/NVM, power, or hardware was changed. Public
architecture references remain engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They are not private ByteDance standards, manufacturer requirements,
certification evidence, or release-readiness claims.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS; current source compiles |
| `uv run ruff check` on changed presentation modules | PASS |
| `uv run ruff format --check` on changed presentation modules | PASS |
| D30 icon-contract source probe | PASS; semantic keys, theme refresh routes, and no platform standard-icon references |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS; root/dist SHA `263B2265D61BB9425052198107FD39C9880953EE15576A16BB555D56B6301C56`, size `38,400,409` bytes; source `tree-sha256:faf1570e4baf1aa776ab867fddf194ca6e9f5b931db0653d15b4c73c65515b71` |
| `scripts\verify_handoff.ps1` / `scripts\check.ps1` | PASS; D30 handoff/index/register/acceptance and full project static gates synchronized |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; exit 1, 10 open gates, exact failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent` |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review returned no conclusion; no child PASS is represented.
- Runtime icon rendering, DPI, native accessibility, installed-font
  independence, and visual acceptance remain open.
- Existing D7/D8 performance, clean-machine, legal, signing/installer/update,
  and release gates remain open and are not narrowed by D30.

**Disposition:** `accepted-with-limits`; D30 is source-level verified by the
parent with an explicit independent no-conclusion record.
