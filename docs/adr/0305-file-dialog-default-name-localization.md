# ADR-0305: File-dialog default-name localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D261 / ARCH-239

## Context

The Save As presentation surface already localized its dialog title and text
filter, but the no-current-document fallback remained the hard-coded
`Untitled.txt`. In the Simplified Chinese shell that left one visible default
filename in English.

## Decision

Reuse the existing `document.untitled` presentation catalog value when no
current path exists, and append the existing `.txt` extension. When a current
path exists, pass `str(current)` unchanged. The file-dialog title, filter,
selected-path return contract, and document persistence policy remain
unchanged.

The default name belongs to the presentation boundary because it is a dialog
display fallback. It does not belong in the document domain, storage layer, or
filesystem policy.

## Boundaries and alternatives

The implementation keeps one localized fallback expression at the existing
`QFileDialog.getSaveFileName` call. Adding a second catalog key would duplicate
the already-owned untitled-document concept; changing the document model would
couple a display-only filename to persistence and save policy.

## Public-source applicability and review

Qt 6 first-party [`QFileDialog::getSaveFileName` documentation](https://doc.qt.io/qt-6/qfiledialog.html#getSaveFileName)
is applicable to the initial filename argument and does not change the
selected-path contract. The project document naming and locale contracts are
the applicable engineering references. No manufacturer requirement changed.
Public CloudWeGo/ByteDance material remains an engineering reference only; no
private corporate standard, certification, MISRA, ISO 26262, ASPICE, or
embedded C/C++/MCU/RTOS claim is made.

