# ADR-0301: Built-in plugin status-name localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D257 / ARCH-235

## Context

The built-in `quillforge.document-stats` plugin already had a localized
command title, but the plugin-status presentation still rendered
`status.name` directly. Its manifest name is the English string
`Document Statistics`, so the Chinese shell could still show that name in the
plugin status surface after D256.

External plugin names are user-supplied metadata and must remain visible as
provided. Translating every status name would invent a catalog contract for
third-party plugins and could hide useful identity information.

## Decision

Add one presentation-only `plugin_display_name` resolver. It maps the stable
built-in plugin ID `quillforge.document-stats` to the existing
`tools.document-stats` command catalog entry and returns the supplied fallback
for every other plugin ID. `PluginStatusDialog._format_status` uses this
resolver, so the existing `set_locale` refresh path reprojects the name when
the language changes.

The public plugin manifest and status contracts remain unchanged. The built-in
plugin implementation remains the owner of its manifest metadata; the
presentation layer owns the locale projection.

## Boundaries and alternatives

The mapping stays in `presentation.i18n` and is keyed by the stable plugin ID,
not by the English display text. This prevents accidental translation of
external metadata and avoids duplicating locale policy in plugin code. Adding
translated fields to the plugin API or mutating manifests would increase
coupling without improving the current user-visible boundary.

## Public-source applicability and review

Python 3.12 first-party [`str` documentation](https://docs.python.org/3.12/library/stdtypes.html#text-sequence-type-str)
is applicable to the immutable fallback-name contract. The project’s existing
plugin ID and command-catalog contracts are the applicable engineering
references. No manufacturer requirement changed. Public CloudWeGo/ByteDance
material remains an engineering reference only; no private corporate
standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.
