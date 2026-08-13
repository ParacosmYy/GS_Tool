# D180 parent review: font-style settings contract

## Scope

Reviewed the D180 source slice across:

- `domain.models.FontStyle`, `EditorSettings`, and `AppearanceSettings`;
- `application.settings` schema v3 normalization;
- `infrastructure.settings_store` JSON round-trip and legacy defaults;
- `presentation.font_style`, `theme.py`, `settings_dialog.py`,
  `settings_preview.py`, `editor_document_surface.py`, and `editor_widget.py`;
- the existing `SettingsSaveProjectionCoordinator` call chain.

## Findings

No blocking correctness or ownership finding was identified in the reviewed
source. The domain remains Qt-free, malformed values are normalized before
projection, v1/v2 payloads receive regular-style defaults, and the existing
save projection applies the new editor style to all live tabs after the
validated snapshot is accepted. QSS style mapping and QFont mapping are
centralized in the presentation boundary.

The residual risk is native Qt font fallback, actual widget metrics, DPI,
accessibility, and runtime rendering. Those checks were not run because the
project policy explicitly disallows QApplication/window startup and screenshots
for this slice.

## Decision

`PASS` with the recorded native-runtime limits. The implementation preserves
the established high-cohesion/low-coupling boundaries and introduces no
unrelated behavior change.

## Simplification assessment

`PASS`. Reuse is adequate: the existing settings dialog/preview surface,
settings-save projection coordinator, theme stylesheet, and editor adapter are
extended. The new `presentation.font_style` helper keeps Qt mapping in one
place without moving policy into the UI or adding a second settings system.

## Public-source and embedded gate record

The applicable engineering references are Python 3.12/PyQt6 project contracts;
public CloudWeGo material is only a non-binding architecture reference. No
embedded C/C++ or firmware target exists in this change, so the embedded
vendor-source, timing, memory, ISR/DMA, and hardware-validation tracks are
recorded as not applicable. No certification or ByteDance-private-standard
claim is made.

## Evidence

- `D180-SETTINGS-FONT-STYLE-CONTRACT-PROBE=PASS`
- `D180-I18N-FONT-STYLE-PROBE=PASS locales=2 placeholders=style`
- `D180-QSS-FONT-STYLE-PROBE=PASS matrix=12x4`
- `D180-QFONT-PROJECTION-PROBE=PASS`
- `D180-SOURCE-WIRING-PROBE=PASS`
- `D180-COMPILE-RUFF-FORMAT=PASS`
- `D180-CHECK=PASS`
- `D180-PACKAGE-BUILD=PASS`
- `D180-PACKAGE-IDENTITY-PROBE=PASS`
- `D180-VERIFY-HANDOFF=PASS`
- `D180-RELEASE-DOSSIER-INVARIANT-PROBE=PASS`
