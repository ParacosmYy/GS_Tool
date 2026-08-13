# D225 / ARCH-207 parent review: command-surface locale accessor startup fix

## Decision

`PASS` for the root-cause fix, accepted with the explicit no-launch limit.

## Evidence

- The preserved startup log identifies the exact failing call chain:
  `__main__ -> app.main -> build_desktop_runtime -> MainWindow._create_menus ->
  CommandSurface.create_menus`.
- The implementation adds only `_locale()` and delegates to the already stored
  `_locale_provider`; it does not alter command registration, menu order,
  callback binding, toolbar projection, or locale catalog data.
- All existing `_locale()` call sites in `CommandSurface` now have a concrete
  implementation, and the source import/compile, no-window diagnostic, dual
  package builds, archive, identity, and project checks pass.

## Simplification assessment

`PASS`: one named accessor is clearer and safer than replacing the provider
call at every existing projection site or adding a second locale source. It
preserves the current dynamic provider boundary with one small, reviewable
change.

## Limits

The architecture and independent bounded windows returned `NO_CONCLUSION`.
The actual Qt window and packaged EXE remain unlaunched under the permanent
project boundary; the fix is proven by the captured root-cause trace,
source/static checks, and rebuilt archive only.
