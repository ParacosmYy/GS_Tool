# D23 parent review — plugin surface

| Field | Value |
|---|---|
| Slice | D23 / ARCH-14 MainWindow plugin surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Godel / Luna max; bounded read-only review window returned no conclusion before handoff; no PASS is claimed |
| Scope | `plugin_surface.py`, plugin dialog lifecycle/callback sites in MainWindow, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; parent source review and explicit independent no-conclusion record |

## Outcome

`src/quillforge/presentation/plugin_surface.py` now owns extension-catalog and
registered-plugin status dialog construction, replacement, locale, activation,
four semantic callback routes, and catalog governance-button projection.
MainWindow retains catalog scanning, approval/revocation, runtime enablement,
TaskRunner/operation state, result validation, notifications, and error policy.

## Review findings

### Correctness and lifecycle

- PASS by source reasoning: `show_catalog()` and `show_status()` close any
  previous dialog, create the new parented dialog with the current locale,
  connect the semantic routes, then show/raise/activate it.
- PASS by source reasoning: catalog approval/revocation payloads remain
  `object`, and runtime enable/disable payloads remain `str`; MainWindow
  callbacks are unchanged.
- PASS by source reasoning: `set_locale()` updates both open dialogs and the
  locale retained for future dialog creation.
- PASS by source reasoning: governance button enablement is projected through
  one surface method; `_plugin_catalog_governance_inflight` remains in
  MainWindow.
- NOT RUNTIME-VERIFIED: QApplication startup, QWidget lifetime, signal
  delivery, focus, dialog replacement, visual styling, accessibility, DPI,
  and screen-reader behavior remain unrun under the no-launch policy.

### Architecture and policy

- PASS by source reasoning: MainWindow has no concrete plugin dialog imports or
  direct construction after D23.
- PASS by source reasoning: PluginSurface does not import infrastructure, does
  not submit tasks, and does not decide trust, approval, enablement, or
  external execution.
- PASS by source reasoning: no duplicate inflight state, event bus, singleton,
  service locator, or speculative plugin lifecycle abstraction was added.
- Accepted limitation: the surface callback payloads use the existing
  application snapshot/status contracts and `object` where the existing Qt
  signal contract is intentionally broad; a typed descriptor refinement is a
  separate API-boundary task.

## Independent review

Godel / Luna max was assigned a bounded read-only D23 review with no write
access, no Qt launch, and no test creation/run. The review window did not
return a conclusion before the bounded handoff window; no child PASS or FAIL
is claimed. The parent therefore records the exact no-conclusion outcome and
limits acceptance to static source reasoning plus authorized deterministic
checks. A later independent review is required before any claim beyond this
accepted-with-limits slice.

## Simplification assessment

The extraction removes duplicate catalog/status dialog lifecycle blocks,
locale branches, signal wiring, and governance-widget reach-through from
MainWindow. The surface exposes only five semantic methods/constructor
callbacks and stores only the existing dialog references. No further safe
behavior-preserving simplification is required for this bounded slice.

## Public-source applicability and embedded gate

This is a Python/PyQt6 desktop change, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are **N/A**
for MCU/vendor constraints: no MCU, SDK, RTOS, ISR, DMA, driver, boot,
Flash/NVM, power, or hardware target was changed. Public architecture
references are engineering references only: [CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They are not private ByteDance standards, manufacturer requirements,
certification evidence, or a release-readiness claim.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS | Non-destructive compile gate. |
| `uv run ruff check src\quillforge` | PASS | Import/lint gate. |
| `uv run ruff format --check src\quillforge` | PASS | Format gate. |
| D23 source boundary probe | PASS | MainWindow has no concrete plugin dialog imports/construction; surface has catalog/status lifecycle and four callback routes; governance/locale routes remain. |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS | Historical D23 candidate; root/dist identity `A1B68658E4FBA96923421077E214F0204A53F6EB7A3225C4D968FBDEC5C6712E` / `38,388,423` bytes; source `tree-sha256:b23efff15976bf804ce8be01dd4aeff89a87a201d190227c9e1dad4347b61768`. |
| `scripts/verify_handoff.ps1` | PASS | Handoff/index/register status and required sections are synchronized. |
| `scripts/check.ps1` | PASS | Repository static/source/format gates pass; NOTICE coverage reports 13 lockfile packages and 77 formatted files. |
| `scripts/verify_release_handoff.ps1` | EXPECTED NO-GO | Exit 1; exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, `startup_preflight_report_consistent`; 10 release gates remain open. |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review has no conclusion and must be revisited for future
  release-level confidence; this is explicitly not a PASS claim.
- Runtime dialog lifecycle, signal delivery, visual/accessibility behavior,
  and cross-machine rendering remain unverified.
- Plugin trust/approval/enablement/external-execution limits remain governed by
  existing D6 contracts; D23 does not broaden them.
- D7/D8 legal, clean-machine, signing/installer/update/support, and external
  release gates remain open.

**Disposition:** `accepted-with-limits`; D23 presentation composition is
source-level verified by the parent, with an explicit independent
no-conclusion record and all runtime/external release gates open.
