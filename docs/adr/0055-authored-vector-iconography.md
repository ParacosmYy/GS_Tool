# ADR-0055: Use authored vector iconography for shell navigation

- Status: Accepted with limits
- Date: 2026-08-10
- Decision owners: Architect / fixed six-role workflow
- Delivery: D30 / UI-16

## Context

The command rail and workspace tree still request `QStyle.StandardPixmap`
icons. Those icons are platform-dependent, visually dated, and do not share
QuillForge's rounded, warm visual language. They also do not refresh as a
selected theme changes, so the shell can look inconsistent after a settings
save.

## Decision

Add one presentation-only vector icon provider in `presentation/icons.py`.
It draws a small, authored line icon into normal and disabled `QIcon` states
from a caller-provided foreground/accent tint. `CommandSurface` owns toolbar
icon assignment and refreshes the tint from the current application palette.
`WorkspacePanel` owns entry/button icon projection and refreshes existing tree
items after a theme update. The provider has no application, document,
filesystem, persistence, or command dependencies.

Normal primary-action icons use the palette role bound to the stylesheet's
`on_accent` foreground, while disabled icons accept the disabled text role
explicitly. This keeps state-specific readability aligned with the centralized
theme contract instead of applying an opaque universal alpha rule.

Use semantic `IconKey` values instead of `QStyle.StandardPixmap` values. Keep
toolbar labels, tooltips, keyboard shortcuts, signals, workspace click/double
click behavior, locale contracts, and object names unchanged. Reuse the same
provider for command and workspace presentation so future surfaces do not
invent local icon painters.

## Invariants

1. Icon selection is presentation metadata only; callbacks and command IDs are
   unchanged.
2. Normal and disabled icon states are authored from the same semantic key;
   normal primary-action and disabled foregrounds come from their respective
   semantic palette roles.
3. Theme changes refresh toolbar and existing workspace-item icons without
   reloading directory data or changing selection/navigation state.
4. No second stylesheet, global icon singleton, external asset download, or
   platform-specific icon contract is introduced.
5. Runtime Qt painting, DPI scaling, installed fonts, screenshots, and screen
   reader output remain explicit limits under the no-launch policy.

## Alternatives rejected

- Keeping `QStyle.StandardPixmap` preserves platform variance and the reported
  old-fashioned appearance.
- Emoji/text glyphs depend on installed fonts and have unstable metrics across
  Windows machines.
- Adding a bitmap icon pack would increase asset/versioning surface without
  improving theme-aware tinting or DPI behavior.
- Moving command or workspace behavior into an icon module would violate the
  existing presentation/application boundary.

## Public-source applicability

This is Python/PyQt6 desktop presentation code, not embedded C/C++ or
firmware. The embedded enterprise workflow and embedded code-review
simplifier are N/A for MCU/vendor constraints. CloudWeGo public material is
used only as an engineering reference for explicit modular boundaries:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
No private ByteDance standard or certification claim is made.

## Verification target

Run compile, Ruff, format, handoff, project, package, icon-contract, and
artifact-identity checks. Do not launch Qt, create tests, or claim runtime
visual acceptance.
