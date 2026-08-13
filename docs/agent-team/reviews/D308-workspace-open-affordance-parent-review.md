# D308 parent review — Workspace open-action affordance

## Scope

Reviewed the workspace folder/file action roles, locale refresh ordering,
Tooltip and accessible-description projection, file/folder QSS edge hierarchy,
existing file activation signals, source contract, diagnostics, and package
boundary.

## Findings

- PASS — the four semantic roles are assigned at construction without changing
  signal connections or file-open admission.
- PASS — one helper keeps visible labels, tooltips, and accessible names and
  descriptions synchronized at `set_locale()`; no Locale dependency leaks into
  Qt-free coordinators.
- PASS — folder and document opening actions have separate semantic edges with
  a 3.57:1 minimum edge contrast across the 3-theme × 4-accent matrix.
- PASS — existing click, double-click, Enter, native picker, containment, and
  asynchronous document-open paths remain present and unchanged.
- PASS — format, compileall, Ruff, presentation audit, source diagnostics,
  package identity, PE header, and frozen archive checks pass.

## Simplification assessment

PASS. A single four-argument helper removes repeated Tooltip/accessibility
projection without introducing a new state model or service. The semantic
roles are presentation metadata and remain scoped to the workspace panel; no
further abstraction is justified for one surface.

## Review status and limits

The architecture consultation and independent review each returned
`NO_CONCLUSION` after three bounded waits. Native Qt/EXE rendering, screen
reader behavior, focus traversal, clean-machine behavior, and release gates
remain unverified. No unit tests, mocks, fixtures, or harnesses were added or
run.

## Public-source applicability

Qt QWidget accessibility APIs, dynamic properties, QSS, and WCAG 2.2 are
public engineering references. Embedded public-vendor applicability is N/A
because this is Python/PyQt6 desktop presentation code, not embedded C/C++ or
firmware.
