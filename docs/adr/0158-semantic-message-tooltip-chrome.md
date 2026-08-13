# ADR-0158: Semantic message and tooltip chrome

- **Status:** accepted-with-limits; UI-65 bounded presentation slice
- **Date:** 2026-08-11
- **Decision owner:** Architect

## Context

The shell already assigned stable object names to common message boxes and the
recovery prompt, but those dialogs and tooltips still inherited mostly flat
native chrome. Their semantic color roles existed elsewhere in the stylesheet,
yet feedback hierarchy was not consistently visible in the dialog frame or
informative text.

## Decision

Refine the existing centralized QSS only. Common, about, error, and recovery
message boxes receive a rounded bordered frame with a semantic top accent;
message text and informative text receive explicit readable colors; buttons
receive a consistent minimum target and margin. Tooltips reuse existing surface,
accent, text, and font tokens with a compact rounded frame.

## Invariants

1. `MessageSurface` and `RecoveryPromptSurface` retain object names, localized
   text, button roles, `exec()` flow, return decisions, and application policy.
2. Existing global button role selectors continue to own primary/warning/quiet
   hover, focus, pressed, and disabled behavior.
3. The change adds no runtime state, signal, widget, object name, token, or
   per-theme stylesheet branch.
4. Text contrast remains readable across all existing themes and accents under
   the static token probe.

## Alternatives considered

- **Restyle each message surface in Python:** rejected; it duplicates visual
  policy and couples dialog composition to theme tokens.
- **Add new semantic dialog classes:** rejected; existing object names already
  provide the correct QSS boundary.
- **Use icon or animation changes:** rejected; the bounded goal is feedback
  hierarchy and readability, not dialog behavior.

## Public-source applicability and embedded gate

This is Python 3.12/PyQt6 desktop presentation styling. MCU, embedded C/C++,
BSP/HAL, CMSIS, RTOS, ISR/DMA, driver, boot/OTA, Flash/NVM, power-control,
motor-control, and manufacturer requirements are not applicable. The mandatory
embedded assurance workflow and simplifier are N/A for this source scope; no
embedded source was changed. Public CloudWeGo material remains an engineering
reference only. No private ByteDance standard, certification, MISRA, ISO
26262, ASIL, ASPICE, or compliance claim is made.

## Review and simplification

- Architect: Heisenberg the 4th / Luna max; two bounded read-only waits timed
  out and the agent was closed without a conclusion. Status is `NO_CONCLUSION`.
- Independent review: Noether the 4th / Luna max; two bounded read-only waits
  timed out and the agent was closed without a conclusion. Status is
  `NO_CONCLUSION`; no independent PASS is claimed.
- Parent review: PASS for selector scope, object-name compatibility, token
  reuse, button-state inheritance, and unchanged dialog policy.
- Simplification assessment: PASS. Existing QSS selectors and tokens are the
  smallest cohesive seam; no dialog code or new style layer is needed.

## Verification target and limits

- Authorized evidence: message/tooltip source coverage, all-theme contrast,
  compileall, Ruff, format, package identity, handoff/register/index
  synchronization, no-process evidence, and expected release NO-GO evidence.
- Not proven: native Qt dialog rendering, DPI/font metrics, accessibility,
  QApplication startup, runtime interaction, clean-machine, cross-machine,
  signing, installer, updater, legal, support, and release-owner evidence.
- No unit tests, mocks, fixtures, harnesses, or test-only assets are created or
  run under the active project policy.
