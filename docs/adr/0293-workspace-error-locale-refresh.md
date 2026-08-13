# ADR-0293: Workspace error locale refresh

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D249 / ARCH-227

## Context

`WorkspacePanel.show_error()` correctly projected typed navigation failures,
but `set_locale()` subsequently rebuilt the status text from the current
directory/loading state. A user who changed language after an open failure
could therefore lose both the error level and the localized error message.

## Decision

Retain the current workspace error source as a presentation-only field and
reproject it whenever the panel locale changes:

- `show_error()` stores the original `str | Exception` source and projects it
  through the existing `localize_exception()`/`localize_message()` boundary;
- a successful `set_directory()` and the start of a new `set_loading(True)`
  clear the retained error because a newer workspace result supersedes it;
- `set_locale()` gives the retained error precedence over loading and normal
  directory status, preserving the error level while translating its text;
- coordinator, surface, domain, service, notification, and page-retention
  policy remain unchanged.

## Boundaries and alternatives

The retained value is UI-local, transient, and neither serialized nor passed
into application services. Storing the source rather than the translated text
avoids stale-language state and preserves typed filesystem/codec metadata.
Adding a workspace-specific translation table or changing coordinator policy
would duplicate the shared presentation i18n boundary and increase coupling.

## Public-source applicability and review

Python 3.12 first-party documentation for
[built-in exceptions](https://docs.python.org/3.12/library/exceptions.html)
is applicable to preserving the original exception object and its diagnostic
metadata across presentation re-projection. No Qt API or manufacturer
contract changed. Public CloudWeGo/ByteDance material remains an engineering
reference only; no private corporate standard, certification, MISRA, ISO
26262, ASPICE, or embedded C/C++/MCU/RTOS claim is made.

The architecture role `Epicurus the 7th / Luna max` and independent reviewer
`Feynman the 7th / Luna max` both returned `NO_CONCLUSION` after bounded waits
and closure. No child approval is claimed. Parent review is `PASS`; the
behavior-preserving simplification assessment is `PASS`.

## Evidence and limits

- `D249-ERROR-SOURCE-RETENTION=PASS`,
  `D249-LOCALE-ERROR-REPROJECTION=PASS`, and
  `D249-SUCCESS-AND-LOAD-CLEARING=PASS`.
- Compileall, Ruff, and format checks passed.
- PS5.1 and PS7 package builds passed. Final root/dist identity is SHA-256
  `41DF3416DEBC5C2E59D6E091BB36BA7F9C954D7A3952D6F6E009F5999D6B5E9D`,
  38,578,143 bytes, source revision
  `tree-sha256:ff124786da0879a1fa867ae79293297ed029ab4ce3175a6ee2acc94579a726a7`.
- The frozen archive contains the entry point, PyQt6 platform plugin,
  composition modules, workspace panel/coordinator modules, and 25 non-empty
  PyInstaller warning lines.

No EXE/Qt startup, native dialog, registry, installer, updater, clean-machine,
signing, or release-owner evidence was run under the active non-destructive
policy. Release remains `no-go` with the existing artifact-bound report gates
open.
