# ADR-0002: Extensible application shell and document boundaries

- Status: accepted for the editor-shell milestone
- Date: 2026-08-09

## Acceptance note

- **User outcome:** QuillForge supports multiple document tabs, asynchronous open/save, safe replacement saves, external-change detection, and host commands that can be extended by plugins.
- **In scope:** application-owned editor ports, document use cases, a filesystem adapter, an in-process event bus, explicit plugin lifecycle management, and a command-driven presentation shell.
- **Out of scope:** remote plugin installation, automatic execution of arbitrary plugin code, indexing, collaboration, updater/installer, telemetry, and a public compatibility promise beyond plugin API `1.0`.
- **Compatibility:** preserve Python 3.12, PyQt6/QScintilla, the existing command-line entry point, and the single-file PyInstaller path. No new runtime dependency is required.
- **Responsiveness:** file operations run through a Qt thread-pool boundary; widgets remain owned by the UI thread.
- **Failure/recovery:** saves use a temporary file plus `os.replace`, compare the last loaded file revision, return the temporary file's written revision without a post-replace destination read, and can retain a `.bak` copy. External changes fail visibly instead of being overwritten silently.
- **Verification:** run `scripts/check.ps1`, rebuild with `scripts/package.ps1`, inspect the EXE, and launch it briefly on the supported Windows x64 environment.

## Decision

Adopt an application-shell architecture with five explicit seams:

1. `EditorEngine` is the application port implemented by `EditorWidget`; application services do not depend on QScintilla.
2. `DocumentService` owns document use cases while `DocumentStore` is the infrastructure port. Domain state carries encoding, line-ending, dirty, and disk-revision metadata without filesystem behavior.
3. `CommandRegistry` and `EventBus` are stable application contracts. Menus are projections of registered commands rather than the source of business behavior.
4. `PluginManager` owns explicit registration, API-version validation, activation/deactivation, command ownership, event subscription cleanup, and failure isolation. The public plugin context exposes capabilities, not widgets or a mutable service container.
5. The composition root wires concrete adapters and the presentation shell. External plugin discovery is deliberately not enabled until permissions, trust, and distribution policy are separately decided.

## Operational contracts

- **I/O scheduling:** the presentation layer assigns a monotonically increasing operation ID and permits at most one document I/O operation at a time in the first shell. A completion callback is accepted only when its operation ID is still current and its target tab still exists. The operation boundary has no fake cancellation; the UI stays responsive and reports completion or failure.
- **Thread ownership:** `DocumentStore` runs only in the worker boundary. `EditorEngine`, `EventBus.publish`, plugin commands, and event handlers run on the Qt UI thread. Worker results are marshalled back before they touch state, widgets, or plugin callbacks.
- **Revision semantics:** `DocumentRevision` is an optimistic `(mtime_ns, size)` token captured on load/save. Save returns the revision captured from the fully flushed temporary file so a post-replace external write cannot be adopted as the editor's clean revision. It is not a filesystem lock and cannot eliminate a write that races exactly between the final comparison and replacement. That residual limitation is reported rather than hidden.
- **Save/recovery:** the store writes a same-directory temporary file, flushes it, optionally copies the previous file to `.bak`, then uses `os.replace`. Temporary cleanup is best-effort in the failure path. Recovery UI and restore workflows were future scope for this original shell milestone and are now delivered under D3, with D7 scale limits still explicit.
- **Plugin trust:** only built-in or explicitly allowlisted plugin instances registered by the composition root may activate in this milestone. No external module discovery/import occurs. In-process Python plugins are trusted extensions, not a security sandbox; permissions, signing, user approval, and process isolation are prerequisites for opening this boundary.
- **Command/event ownership:** command IDs are unique and commands are removed with their owner on deactivation. Event delivery is synchronous and exact-type, with plugin callbacks wrapped for failure isolation. The host owns lifecycle cleanup; plugin-created threads, timers, and processes are outside the API and prohibited by the current contract.

## Consequences

- New editor engines, document stores, and plugin implementations can be introduced without rewriting the main window.
- The first shell gains more lifecycle code and must keep asynchronous completion paths observable and bounded.
- Plugin commands can be removed cleanly during deactivation, but API compatibility still requires versioned review.
- Revision checks are optimistic rather than a formal filesystem lock; the remaining race is an explicit release risk.
- A single-file EXE remains a distribution artifact rather than an installer, signed release, update mechanism, or licensing decision.

## Alternatives considered

- **Widget-driven feature code:** faster for one screen, but couples persistence, commands, and plugins to Qt internals.
- **Automatic plugin folder imports:** convenient discovery, but executes untrusted code without a product-level trust and permission model.
- **Event-only extension:** simple notification, but insufficient for reusable commands and lifecycle ownership.
