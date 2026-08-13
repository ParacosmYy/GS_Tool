# D95 / ARCH-70 parent review: document-tab creation coordinator

## Verdict

`PASS by source review; accepted-with-limits`.

## Five-axis review

- **Correctness — PASS:** Editor creation, tab factory, recovery snapshot
  identity, surface projection, title/modified refresh, session save, and
  status order match the former `_add_tab`.
- **Readability — PASS:** MainWindow retains concrete settings/editor/document
  policy while one generic coordinator owns assembly sequencing.
- **Architecture — PASS:** The coordinator has no Qt, editor, surface, or
  concrete application type dependency.
- **Security/data safety — PASS:** No document text, recovery payload, path,
  dirty state, or persistence schema behavior changed.
- **Performance — PASS:** No additional copy, worker, serialization, or I/O was
  introduced.

## Behavior review

1. Editor creation receives the same language, text, dirty, settings, theme,
   and accent values through the MainWindow adapter.
2. Recovery snapshot identity is passed unchanged into `_DocumentTab`.
3. `add_tab` runs before title/session/status finalization, preserving the
   existing `currentChanged` re-entry point.
4. Modified state is calculated at the same projection point.
5. Open/recovery event publication, notifications, and session-restore
   continuation remain outside the coordinator.

## Simplification assessment

The extraction removes one assembly block without introducing a new state model
or framework. No further safe behavior-preserving simplification was
identified.

## Review-role evidence

Huygens the 3rd / Luna max architecture and Fermat the 3rd / Luna max
independent review both returned `NO_CONCLUSION` after bounded windows. No
child PASS is claimed.

## Verification and limits

The D95 tab-creation boundary probe, targeted compileall, Ruff, format, package
identity, handoff, repository checks, and expected release no-go evidence are
required before delivery. Native editor/currentChanged timing, runtime
startup, accessibility, clean-machine, cross-machine, and external release
gates remain unrun or open.
