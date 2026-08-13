# ADR-0303: Message-dialog standard button localization

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D259 / ARCH-237

## Context

`MessageSurface` used `QMessageBox.StandardButton` values for save-before-
close, about, and error dialogs. The buttons were assigned roles, but their
visible text was left to Qt's default standard-button labels. That made the
Chinese shell depend on the host Qt/system locale and could leave Save,
Discard, Cancel, or OK in English.

## Decision

Add four existing-catalog entries for Save, Discard, Cancel, and OK. After
each message box calls `setStandardButtons`, `MessageSurface` explicitly sets
the corresponding button text through one private helper. The helper uses the
current dialog locale, while the existing `StandardButton` values, default
button, button roles, click result comparisons, and modal flow remain intact.

Recovery and Settings already own explicit translated button text and are not
changed. This keeps dialog-specific presentation ownership local and avoids
changing application decisions or Qt standard-button semantics.

## Boundaries and alternatives

The fix is at the presentation dialog boundary rather than a global Qt
translator installation. A global translator would couple host/process locale
state to the application preference and could affect native controls outside
QuillForge. Adding text immediately after button creation also keeps the
`QMessageBox.StandardButton` return contract stable.

## Public-source applicability and review

Qt 6 first-party [`QMessageBox::button` documentation](https://doc.qt.io/qt-6/qmessagebox.html#button)
is applicable to retrieving the standard button after `setStandardButtons`.
Qt 6 first-party [`QAbstractButton::setText` documentation](https://doc.qt.io/qt-6/qabstractbutton.html#text-prop)
is applicable to the presentation text override. The project’s locale and
dialog-role contracts remain the applicable engineering references. No
manufacturer requirement changed. Public CloudWeGo/ByteDance material remains
an engineering reference only; no private corporate standard, certification,
MISRA, ISO 26262, ASPICE, or embedded C/C++/MCU/RTOS claim is made.
