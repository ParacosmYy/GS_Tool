# D199 parent review: command menu contract closure

## Decision

`PASS` for the bounded application-contract slice, accepted with explicit
plugin/runtime and release limits.

## Review evidence

- `SUPPORTED_MENU_IDS` is defined once in the application command boundary and
  preserves the existing four-menu order.
- `CommandRegistry.register` rejects unknown IDs before admission, preventing
  the previous registered-but-invisible state.
- `CommandSurface` consumes the same ordered collection for localized menu
  creation; it does not acquire application policy or a second menu contract.
- Existing built-ins and the built-in plugin use supported IDs; execution,
  shortcuts, callbacks, locale, and trust/enablement code are untouched.

## Simplification assessment

`PASS`: one canonical tuple and one registration guard are smaller and safer
than a dynamic menu registry, fallback menu, or new presentation coordinator.

## Limits

Native Qt menu rendering, runtime plugin activation, accessibility, DPI,
clean-machine, cross-machine, installer, signing, updater, and release-owner
evidence remain unrun.
