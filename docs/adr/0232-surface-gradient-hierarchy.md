# ADR-0232: Restrained surface-gradient hierarchy

- Status: Accepted with limits
- Date: 2026-08-11
- Delivery: D183 / ARCH-170 / UI-95

## Context

The shell already had explicit surface tokens and state selectors, but the
main window, editor shell, and command rail were visually flat between their
cards and rails. The user-visible goal is a more modern, human-readable depth
cue without adding decorative noise or moving behavior into the style layer.

## Decision

1. Use the existing `ThemeColors.surface_0`, `surface_1`, and `surface_2`
   endpoints to generate exactly three restrained QSS linear gradients.
2. Scope them to `QMainWindow#mainWindow`, `QWidget#editorShell`, and
   `QToolBar#commandBar`; keep editor canvas, controls, status states, dialogs,
   and semantic action selectors unchanged.
3. Do not add a gradient token registry, widget-local stylesheet, animation,
   image asset, or behavior callback.
4. Keep text on both endpoints within the existing contrast contract for all
   supported theme/accent combinations.

## Compatibility and limits

This is a presentation-only QSS change. Theme/accent IDs, settings, locale,
font, motion, editor behavior, command roles, signals, persistence, and
application ownership are unchanged. Native Qt QSS parsing/painting, DPI,
font fallback, screenshots, and runtime visual acceptance remain unrun under
the active no-launch policy.

## Applicability and review

Python 3.12/PyQt6 presentation code is the applicable scope. Embedded C/C++,
MCU, RTOS, and manufacturer requirements do not apply. Public CloudWeGo or
ByteDance-adjacent material is engineering reference only, not an internal
standard or compliance evidence. Parent review: PASS. The Architect and
independent child windows returned `NO_CONCLUSION` within bounded time; no child
PASS is claimed. No further safe simplification was identified.
