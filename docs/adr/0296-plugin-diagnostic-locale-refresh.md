# ADR-0296: Plugin diagnostic locale refresh

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D252 / ARCH-230

## Context

The plugin status and catalog dialogs already rebuilt their rows and tooltips
when the locale changed, but the embedded `status.error` and `entry.reason`
values were passed through as raw provider strings. Known policy and manifest
diagnostics therefore remained English inside an otherwise localized dialog.

## Decision

Keep the existing plugin policy and fix the display boundary:

- `PluginStatusDialog` passes runtime status errors through the shared
  `localize_message()` mapper before composing the tooltip;
- `PluginCatalogDialog` passes catalog entry reasons through the same mapper;
- `i18n.py` translates only the stable plugin policy/manifest reasons and
  prefixes owned by this application, while unknown third-party text remains
  unchanged;
- existing locale refresh calls continue to update rows and tooltips in place.

Trust, approval, enablement, runtime registration, external execution,
catalog scanning, permissions, ordering, and application ownership remain
unchanged. The change is presentation-only and preserves the original source
text in plugin state objects.

## Boundaries and alternatives

No plugin service or validator receives a locale dependency. A provider-side
translation layer would couple governance and infrastructure to presentation
policy and could alter diagnostics before they reach English users. A second
dialog-specific translator would duplicate the shared fallback behavior.

## Public-source applicability and review

Python 3.12 first-party documentation for
[`str.startswith`](https://docs.python.org/3.12/library/stdtypes.html#str.startswith)
and
[`str.removeprefix`](https://docs.python.org/3.12/library/stdtypes.html#str.removeprefix)
is applicable to the stable-prefix catalog and suffix-preserving message
projection. No Qt API or manufacturer contract changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.

The architecture role `Noether the 7th / Luna max` and independent reviewer
`Turing the 7th / Luna max` returned `NO_CONCLUSION` after bounded waits and
closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D252-PLUGIN-STATUS-ERROR-ROUTING=PASS`,
  `D252-PLUGIN-CATALOG-REASON-ROUTING=PASS`, and
  `D252-PLUGIN-UNKNOWN-TEXT-FALLBACK=PASS`.
- `D252-PLUGIN-LOCALIZER-BEHAVIOR=PASS cases=7`.
- Compileall, Ruff, and format checks passed.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `32B23AF303719818A5528F19778888D74058B9419713026EF144B30A91C72F58`,
  38,579,367 bytes, source revision
  `tree-sha256:9e8f908b85342e6b3c138056fb94f38477af6ea03b224a64b221d3497f036936`.
- The frozen archive contains the entry point, PyQt6 Windows platform plugin,
  plugin dialog/i18n modules, and 25 non-empty PyInstaller warning lines.

Native EXE/Qt startup, native dialogs, clean-machine, signing, installer,
updater, and release-owner evidence remain unrun under the active
non-destructive policy. Release remains `no-go` with existing report gates
open.
