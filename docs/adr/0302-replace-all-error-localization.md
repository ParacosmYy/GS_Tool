# ADR-0302: Replace All error localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D258 / ARCH-236

## Context

The Replace All editor boundary already localized normal status messages and
the bounded `Stopped: more than N matches; document unchanged` outcome. Four
nearby user-visible exception forms still bypassed that boundary in the
Simplified Chinese shell: the match-limit exception, incomplete text capture,
and two rollback-failure messages.

## Decision

Extend the existing `presentation.i18n.localize_message` boundary with one
anchored dynamic mapping for `Replace All is limited to N matches.` and exact
Chinese mappings for the three stable internal error forms. The dynamic
counter is passed unchanged to the catalog formatter. `localize_exception`
therefore picks up the same behavior without changing editor transaction or
rollback code.

Unknown messages remain unchanged and the early en-US return remains in place.
The editor adapter, ReplaceAllSession state machine, error propagation, and
public application contracts are not modified.

## Boundaries and alternatives

The mapping is presentation-only and anchored to the exact message shape. A
broad replacement of `Replace All` would risk translating plugin/provider
diagnostics or hiding unknown details. Moving locale policy into
`EditorWidget` or `ReplaceAllSession` would couple Qt/editor mechanics to the
shell catalog and duplicate the existing status mapping.

## Public-source applicability and review

Python 3.12 first-party [`re.Pattern.fullmatch` documentation](https://docs.python.org/3.12/library/re.html#re.Pattern.fullmatch)
is applicable to the exact dynamic error-shape mapping. The project’s
presentation-localization and editor error contracts are the applicable
engineering references. No manufacturer requirement changed. Public
CloudWeGo/ByteDance material remains an engineering reference only; no private
corporate standard, certification, MISRA, ISO 26262, ASPICE, or embedded
C/C++/MCU/RTOS claim is made.
