# D9 / UI-03 parent review

## Scope

- **Delivery:** D9 Modern UI iteration
- **Slice:** UI-03 Dialog surfaces
- **Owner:** Architect (parent)
- **Change type:** presentation-only spacing, object names, and theme selectors

## Product and architecture decision

UI-03 makes the command palette, settings, Extension Catalog, Plugin Status,
and Find in Files surfaces look like one product. Each dialog keeps its own
layout and behavior. No modal/non-modal boundary, signal, stable `UserRole`,
async cancellation, or application service ownership is changed.

## Implementation record

- `command_palette.py` receives a named query/list surface, dialog hint, and
  consistent 18 px shell margins with 10 px control rhythm.
- `settings_dialog.py` receives the same dialog rhythm without moving the
  settings value-object projection.
- `plugin_catalog_dialog.py` and `plugin_status_dialog.py` receive named
  summary/list/hint surfaces; approval, revoke, enable, and disable actions
  remain separate.
- `workspace_search_dialog.py` receives named root/status surfaces and the
  same margins; search results, diagnostics, and cancellation remain bounded.
- `theme.py` owns the shared selectors, muted hierarchy, and focus treatment.

## Verification and limits

- Ruff check/format and `scripts/check.ps1` are the required static gates.
- `.\scripts\package.ps1` completed after UI-03; root/dist are 38,323,141
  bytes with SHA-256
  `B60E836A504363D03F5828B2703E0AE86BB36353B25526F255D2E475EF4A21CF`.
- No EXE, Qt window, interactive startup, or screenshot is launched because
  the user explicitly prohibited starting the software.
- Runtime DPI, font, native dialog, focus-ring, and accessibility contrast
  review remain open for a later user-permitted visual pass.

## Disposition

`PROCEED WITH LIMITS`: the source slice is bounded and preserves behavior;
visual acceptance is intentionally pending the user's manual review.
