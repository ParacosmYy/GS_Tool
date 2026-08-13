# ADR-0188: close-guard feedback coordinator

- Status: accepted-with-limits
- Date: 2026-08-12
- Delivery: D144 / ARCH-126

## Context

`MainWindow._project_close_guard_block()` still contained the complete mapping
from the Qt-free `CloseGuardDecision` reasons to recoverable error title/body
copy, including the pending-worker count. The close-readiness classification
was already isolated, but its presentation mapping kept the composition root
larger and coupled to the message text.

## Decision

Add a Qt-free `CloseGuardFeedbackCoordinator` with frozen/slotted
`CloseGuardFeedbackPorts`. It preserves the five existing reason messages,
pending-count interpolation, and allowed-decision no-op. MainWindow injects
only `TaskRunner.pending_count` and its existing `_show_error` projection;
MainWindow retains `CloseGuardCoordinator`, `QCloseEvent`, MessageSurface,
close acceptance, and all policy.

## Alternatives rejected

- Moving close readiness or event handling would mix classification with Qt
  lifecycle policy.
- A generic message registry would add an unnecessary second text/locale
  system.
- Keeping the mapping in MainWindow would leave a pure cross-module projection
  list in the composition root without a concrete ownership reason.

## Review and evidence

Lovelace the 5th / Luna max was assigned the architecture assessment and
returned no conclusion in the bounded window. Huygens the 5th / Luna max was
assigned the independent read-only review and also returned no conclusion after
two short waits. No child PASS is claimed. Parent review is `PASS`;
simplification assessment is `PASS` because one two-port mapping boundary is
the smallest complete extraction.

Authorized non-destructive evidence:

- `D144-CLOSE-FEEDBACK-PROBE=PASS`
- `D144-CLOSE-FEEDBACK-REASON-COVERAGE-PROBE=PASS`
- `D144-QT-FREE-CLOSE-FEEDBACK-PROBE=PASS`
- `D144-MAINWINDOW-WIRING-PROBE=PASS`
- `D144-COMPILEALL=PASS`
- `D144-RUFF=PASS`
- `D144-FORMAT=PASS`
- `D144-CHECK=PASS`
- `D144-VERIFY-HANDOFF=PASS`
- `D144-PACKAGE-IDENTITY-PROBE=PASS`
- expected release `NO-GO` dossier and no-launch/traceability checks.

The packaged candidate is `dist/QuillForge.exe` and the root test copy:

- SHA-256: `689E22005721D8489595C463E73F23C7DE642154F45F535BA93DAAB7B373E0AB`
- bytes: `38536226`
- source revision: `tree-sha256:c45d03ef75a623e5ce9a2c79e73988745cc0bcc91b9c2ad07b8c3afe2147d228`

Public-source applicability is Python 3.12/PyQt6 presentation composition;
embedded C/C++, MCU, RTOS, and manufacturer requirements do not apply. Public
CloudWeGo material remains an engineering reference only, not a private
ByteDance standard or a certification/compliance claim.

## Limits

Static and inline probes do not prove native dialog rendering, event-loop
timing, accessibility, font/DPI metrics, runtime startup, clean-machine or
cross-machine behavior, filesystem durability, signing, installer/update,
legal clearance, support ownership, or release readiness. Those gates remain
open under the active no-launch/no-release authorization boundary.
