# ADR-0294: Workspace search status locale refresh

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D250 / ARCH-228

## Context

The workspace-search dialog stored only the already-rendered status text and
severity. `set_locale()` refreshed labels and feedback styling but did not
rebuild the current status. Search failures and dynamic completion summaries
therefore stayed in the previous language after a locale switch.

## Decision

Keep a presentation-local status source and regenerate text for the active
locale:

- catalog statuses retain their i18n key;
- failures retain the original message string and reapply the shared
  `localize_message()` mapping;
- completion/limited/cancelled summaries retain the immutable
  `WorkspaceSearchResult` and rerun `workspace_search_summary()`;
- `set_locale()` projects the rebuilt text while preserving the existing
  `FeedbackLevel` and result/query state.

Search service policy, operation lifecycle, cancellation, result projection,
diagnostics, and application ownership remain unchanged.

## Boundaries and alternatives

The status source is transient UI state and is not serialized or sent back to
the application layer. Storing source data instead of translated text avoids
stale-locale caching and keeps summary formatting in the existing i18n helper.
A second translator, service callback, or localized-text cache would add
coupling without improving the boundary.

## Public-source applicability and review

Python 3.12 first-party documentation for
[dataclasses](https://docs.python.org/3.12/library/dataclasses.html) and
standard exception/string behavior is applicable to retaining the immutable
search result and raw message source in presentation state. No Qt API or
manufacturer contract changed. Public CloudWeGo/ByteDance material remains
an engineering reference only; no private corporate standard, certification,
MISRA, ISO 26262, ASPICE, or embedded C/C++/MCU/RTOS claim is made.

The architecture role `Godel the 7th / Luna max` and independent reviewer
`Lorentz the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D250-STATUS-SOURCE-RETENTION=PASS`,
  `D250-LOCALE-ERROR-REPROJECTION=PASS`,
  `D250-LOCALE-SUMMARY-REPROJECTION=PASS`, and
  `D250-CATALOG-STATUS-REPROJECTION=PASS`.
- Compileall, Ruff, and format checks passed.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `65345D8C6484C57A272501E68BCDBC82873A400D02539B582FD2733686A7BA34`,
  38,578,545 bytes, source revision
  `tree-sha256:fd5630adf006bccea7c69226e598d7384a3f8bfa8bf1453349ec50eb15196075`.
- The frozen archive contains the entry point, PyQt6 platform plugin,
  composition modules, workspace-search dialog/surface modules, and 25
  non-empty PyInstaller warning lines.

Native EXE/Qt startup, native dialog, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.
