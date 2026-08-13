---
name: quillforge-ui-visual-quality
description: Improve QuillForge PyQt6 presentation quality with human-readable visual hierarchy, centralized QSS tokens, cute-modern styling, state clarity, and contrast evidence. Use when changing theme.py, FindBar, dialogs, command rails, status surfaces, typography, accent colors, animations, or any user-visible Qt UI.
---

# QuillForge UI Visual Quality

## Overview

Use this skill for bounded, production-quality QuillForge UI slices. Make the
interface feel modern and soft without turning every control into a gradient
or pill; preserve readability, keyboard flow, accessibility, and the existing
presentation/application boundary.

## Workflow

1. Read the current `src/quillforge/presentation/theme.py` token/QSS contract,
   the target widget, its signals, and the relevant acceptance/handoff record.
2. Choose one visual problem with a measurable outcome: hierarchy, contrast,
   focus, disabled state, density, typography, or motion.
3. Keep behavior in the current owner. Use semantic `objectName`, dynamic
   properties, or existing signals only as a presentation projection; do not
   put document, task, or persistence logic in a widget.
4. Centralize tokens and selectors. Prefer a small semantic role such as
   `primaryAction`, `warningAction`, `surface_2`, or `focus` over widget-local
   stylesheet fragments.
5. Check the visual state matrix: normal, hover, pressed, focus, checked,
   selected, disabled, read-only, warning, and error. State differences must
   not depend on color alone when the state is consequential.
6. Validate statically. Under the project no-launch policy do not instantiate
   `QApplication`, open windows, take screenshots, or claim runtime visual
   acceptance. Run compileall, Ruff, formatting, JSON/handoff checks, and a
   targeted source/contrast probe instead.
7. For source changes, rebuild the portable package, synchronize artifact
   identity, update acceptance/register/index/roadmap, and create exactly one
   new handoff for the material slice.

## Visual rules

- Use a clear surface ladder: canvas, panel, card, and elevated/interactive
  surface should differ in value or border, not only decoration.
- Reserve the strongest accent for the current task's primary action or active
  navigation. Keep destructive or bulk actions visually distinct as warnings.
- Never assume white text is readable on an accent. Audit every gradient
  endpoint and warning state; target at least 4.5:1 for normal text and record
  any runtime/font limitations explicitly.
- Use typography as hierarchy: a small number of weights and sizes, stable
  line-height, and sufficient whitespace. Avoid all-caps micro-labels and
  excessive letter spacing in Chinese UI.
- Keep animation optional, short, and state-preserving. Respect the persisted
  motion preference and never make a task's completion depend on animation.
- Cute/anime-inspired styling should come from palette, warmth, rounded geometry,
  micro-copy, and restrained ornamentation; do not add unlicensed characters,
  distracting particles, or decorative noise to work surfaces.

## Boundary and verification

Keep `presentation` dependent on application contracts and adapters, never the
reverse. Preserve signal names, keyboard routing, task ownership, editor
adapter boundaries, and locale/theme settings contracts unless the acceptance
explicitly includes a contract change.

Record independent review, simplification assessment, public-source
applicability, authorized non-destructive verification, unrun runtime checks,
and the exact package identity. A static pass is not a screenshot pass and a
beautiful token palette is not evidence that native Qt specificity works at
runtime.
