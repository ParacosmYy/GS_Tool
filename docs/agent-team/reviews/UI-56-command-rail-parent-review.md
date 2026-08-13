# UI-56 / ARCH-87 parent review: command-rail visual role hierarchy

## Scope

Reviewed UI-56 changes in
src/quillforge/presentation/command_surface.py,
src/quillforge/presentation/main_window.py, and
src/quillforge/presentation/theme.py against the UI-56 specification.

## Findings

| Axis | Result | Evidence |
|---|---|---|
| Role contract | PASS | ToolbarActionRole is a closed Literal and ToolbarActionSpec keeps a backward-compatible default of standard. |
| Composition ownership | PASS | MainWindow assigns only presentation roles; CommandSurface projects the property; CommandRegistry and callbacks remain untouched. |
| Visual hierarchy | PASS | Primary, quiet, and context roles have scoped base/hover/focus/pressed/disabled states; standard actions retain existing styling. |
| Readability | PASS | Primary and hover foregrounds are selected from canonical accent endpoints; the all-theme/all-accent token probe passed a 4.5 contrast threshold. |
| Accessibility/behavior | PASS | Action text and tooltips remain intact; shortcuts, locale retranslation, icon retinting, action order, callbacks, and layout are unchanged. |
| Dependency direction | PASS | No theme or business-policy import was added to CommandSurface; QSS remains centralized in theme.py. |

## Simplification assessment

PASS. A typed role field plus one dynamic presentation property is the
smallest complete change. No toolbar subclass, command-policy layer, or new
runtime state is needed.

## Review limits

Native Qt QSS specificity/painting, QApplication startup, screen-reader
output, DPI/font metrics, clean-machine, cross-machine, and release-owner
evidence were not run under the active no-launch/authorization boundary. The
delegated architect consultation returned NO_CONCLUSION; no child PASS is
claimed. Independent-review status is carried in the separate UI-56 record.

## Applicability

Python/PyQt6 desktop code only. Embedded C/C++ assurance and manufacturer
requirements are not applicable. Public CloudWeGo material is an engineering
reference only and not a private ByteDance standard or compliance basis.
