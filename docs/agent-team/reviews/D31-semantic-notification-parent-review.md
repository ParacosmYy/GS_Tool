# D31 parent review — semantic transient notification hierarchy

| Field | Value |
|---|---|
| Slice | D31 / UI-17 / ARCH-21 semantic transient notification hierarchy |
| Reviewer | Architect (parent integration review) |
| Architecture reviewer | Socrates the 2nd / Luna max; recommended the bounded StatusSurface slice after read-only audit |
| Independent reviewer | Ramanujan the 2nd / Luna max; bounded lifecycle/API review returned PASS WITH LIMITS |
| Scope | `status_surface.py`, `main_window.py`, `theme.py`, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits |

## Outcome

The existing `StatusSurface` boundary now owns one localized `statusMessage`
label, its status-bar insertion, message lifetime, locale refresh, and four
presentation-only levels: `info`, `success`, `warning`, and `error`. The
default `MainWindow.notify(message)` call shape remains valid; selected success,
warning, and error outcomes opt into the optional level keyword. Application
operation policy, notification text, status-phase precedence, close guards,
and plugin one-argument notification callbacks remain in their previous
owners.

## Parent review findings

### Correctness and lifecycle

- PASS by source reasoning: `attach_to()` removes both owned widgets from a
  previous status bar before adding them to a new host, and remains idempotent
  for the same host.
- PASS by source reasoning: the message label is parented through the Qt object
  tree, and its single-shot `QTimer` is parented to the label; no global timer,
  singleton, or worker callback is introduced.
- PASS by source reasoning: a positive timeout starts the timer, zero and
  negative values stop it and retain the message, matching the prior
  `QStatusBar.showMessage()` timeout contract; a new message replaces the
  current text and restarts the timer.
- PASS by source reasoning: `set_locale()` reprojects the retained raw message
  through `localize_message()` and reapplies the current state property.
- PASS by source reasoning: an empty message clears text, tooltip, timer, and
  visibility.
- NOT RUNTIME-VERIFIED: QApplication startup, QStatusBar layout, timer
  delivery, focus, font metrics, native style behavior, accessibility, DPI,
  and screen-reader output remain unrun under the no-launch policy.

### API and architecture

- PASS: `StatusMessageLevel` is a presentation-only `Literal`; the surface
  imports only Qt, domain `Locale`, and the existing localization/status rail
  adapters.
- PASS: `MainWindow.notify(message, *, level="info")` preserves all existing
  one-positional-argument callers, including `PluginContext.notify`, while
  allowing the shell to opt into visual severity without changing plugin
  capability or event contracts.
- PASS: notification level is not inferred from arbitrary text and is not
  coupled to `StatusPhase.error`; `_show_error()` chooses both intentionally,
  then existing phase synchronization retains its prior policy.
- PASS: MainWindow still owns operation state, error consequences, phase
  precedence, close guards, and notification call sites; StatusSurface only
  projects the supplied presentation metadata.
- PASS: all message selectors are centralized in `theme.py` and use existing
  `ThemeColors` tokens. No widget-local stylesheet or second visual source was
  introduced.

## Validation evidence

| Evidence | Result |
|---|---|
| D31 status contract probe | PASS — typed levels, widget, explicit locale refresh, no native `showMessage` path, and all four QSS selectors present. |
| D31 notification contrast probe | PASS — 48 supported theme/accent message foreground/background pairs are >= 4.5:1; success/error use `text_primary` on their semantic backgrounds after the first probe exposed low-contrast paper/Sakura token pairs. |
| `uv run python -m compileall -q src\quillforge` | PASS |
| `uv run ruff check src\quillforge` | PASS |
| `uv run ruff format --check src\quillforge` | PASS — 79 source files formatted. |
| JSON synchronization probe | PASS — acceptance, delivery register, and handoff index parse. |
| `scripts\package.ps1` | PASS — root/dist `E4DAFAA9DE4C1BE65E380989E781F1706E6BDA9EFCB4BADE686BDA37A5419397`, `38,403,936` bytes; source `tree-sha256:01d9be56d8283a2e27277e05f0ddc7b308b6057856b6a07e144dbb1796d1ec65`. |
| `scripts\verify_handoff.ps1` | PASS — D31 handoff/index status and required sections are synchronized. |
| `scripts\check.ps1` | PASS — NOTICE, source, handoff, format, and package checks pass. |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO — exit 1, 10 open gates, exact mechanical failures `packaged_report_artifact_match`, `interactive_startup_report_consistent`, and `startup_preflight_report_consistent`. |
| Independent lifecycle/API review | PASS WITH LIMITS — Ramanujan the 2nd / Luna max confirmed timeout/locale/compatibility/token scope by source; Qt runtime remains unverified. |

## Independent review record

The first broader independent D31 review window returned no conclusion after
two bounded waits and was closed; no conclusion is represented as a PASS. A
second, narrower independent window was assigned a disjoint lifecycle/API
scope. Ramanujan the 2nd / Luna max returned:

> PASS WITH LIMITS: QLabel/QTimer 0/negative, replacement, and locale
> projection semantics are source-preserved; `notify(message, *,
> level="info")` keeps one-argument compatibility; `statusMessage` selectors
> remain centralized and use centralized tokens; Qt runtime is unverified.

This is source-level independent review only, not visual acceptance.

## Simplification assessment

The slice reuses the existing StatusSurface seam and replaces an unstyled
native projection with one owned label/timer pair. It does not add a
notification bus, severity parser, service locator, or general animation
framework. Keeping the optional level at the shell boundary avoids changing
plugin/application contracts, and keeping phase/error policy in MainWindow
avoids a second lifecycle model. No further safe behavior-preserving
simplification is required for D31.

## Public-source applicability and embedded gate

This is a Python/PyQt6 desktop change, not embedded C/C++ or firmware. The
embedded-enterprise-workflow and embedded-code-review-simplifier are **N/A**
for MCU/vendor constraints: no MCU, SDK, RTOS, ISR, DMA, driver, boot,
Flash/NVM, power, or hardware target was changed. Public architecture sources
remain engineering references only:

- [CloudWeGo About](https://www.cloudwego.io/about/)
- [CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/)
- [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/)

They are not private ByteDance standards, manufacturer requirements,
certification evidence, or release approval.

## Authorized non-destructive validation limits

No unit tests, mocks, fixtures, harnesses, QApplication/Qt startup,
screenshots, interactive visual acceptance, deployment, target hardware, or
release-environment operation were created or run. Existing static/build/
packaging checks were used only as non-destructive evidence. D7.3/D7.4
authorized workload/runtime evidence and D8 legal/signing/installer/update/
clean-machine/support/release-owner gates remain open.

**Disposition:** accepted-with-limits; source, contract, contrast, independent
review, simplification, and package identity evidence are recorded, while
runtime visual/accessibility and external release gates remain open.
