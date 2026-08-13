# QuillForge constraints

These are the initial product and engineering constraints. A change to a constraint requires an ADR or an explicit product decision.

The permanent agent-process constraints are additionally machine-checked by
`docs/agent-team/workflow-policy.json` and `scripts/verify_handoff.ps1`.

## Product scope

- QuillForge is a Windows-first text and code editor.
- The first milestone is a dependable editing shell, not a full IDE.
- The product must remain useful without network access.
- The default experience must be keyboard-friendly and suitable for long editing sessions.
- The editor engine must be replaceable without rewriting application services or the plugin API.

## Platform and runtime

- Development and release baseline: Windows x64.
- Python baseline: CPython 3.12, pinned by `.python-version` and `pyproject.toml`.
- GUI stack: Qt 6 through PyQt6.
- Initial editor control: QScintilla behind a QuillForge adapter.
- Project environment: `.venv` created and maintained by uv.
- Dependency lock: `uv.lock` is committed and must be current.

## Responsiveness and scale

- The Qt UI thread must not perform potentially slow file I/O, recursive search, indexing, syntax analysis, or plugin discovery.
- Long operations require cancellation or a clear bounded-progress strategy.
- A close request must not destroy the UI while a `TaskRunner` worker or queued completion callback remains retained. Lifecycle observation belongs to the runner's bounded pending-task contract; shutdown must remain non-blocking and must not force-terminate a worker.
- Large-file behavior must be measured before claiming support. At minimum, evaluate file size, long lines, encoding, line ending, search latency, memory use, and recovery behavior.
- Rendering, syntax styling, and search should be incremental where the selected editor engine permits it.
- Find in Files is a read-only literal search over an explicitly activated workspace root. Its configured file-count, total-byte, per-file-byte, depth, line-length, result, and diagnostic bounds are safety limits, not a published performance or large-file SLA. It must run behind the worker boundary, support cooperative cancellation, mark limited results, and never follow child symlinks or search excluded build/cache directories.
- Startup should not scan the entire filesystem or load every plugin synchronously. The optional extension catalog is user-triggered, top-level, and bounded; catalog inspection never loads plugin code.

## File safety

- Preserve encoding and line-ending choices when the user has not explicitly changed them.
- Never overwrite a changed file silently; detect external modification and provide a recovery path.
- Save operations should use a safe replacement strategy and retain an optional backup/recovery copy.
- Unsaved content must be recoverable after an application failure whenever the feature is enabled.
- Local session continuity may persist only bounded metadata (workspace root,
  clean path-backed tab order, active tab, and adapter-level caret position) in
  a versioned atomic user-local manifest. It must not duplicate document text or
  recovery payloads; dirty and untitled content remains owned by Recovery.

## Security and privacy

- No telemetry or remote content by default.
- Plugins are untrusted extension code. Plugin permissions, discovery locations, enablement, and failure isolation must be explicit. A catalog manifest is metadata only and cannot declare its own trust.
- Descriptor approval is governance metadata only: it is bound to the canonical descriptor SHA-256, becomes stale after descriptor changes, and never enables dynamic loading, code execution, signature trust, or sandbox escape.
- The approval ledger is user-local, versioned, bounded, atomically replaced, and fail-closed when malformed or oversized.
- Runtime enable/disable controls apply only to explicitly registered in-process plugins, stay on the owning UI thread, and do not make external catalog descriptors executable. The application-facing enablement boundary validates registration and policy before mutation; it may not promote an untrusted registration or bypass a fail-closed policy, and the trust bit is immutable across this lifecycle boundary.
- Local enablement preferences are separate from trust/approval, default trusted built-ins to enabled only when the policy is absent, and disable activation when the policy is malformed or oversized. A direct manager enable call fails closed for an untrusted instance.
- The plugin host diagnostic is stdio-only, shell-free, bounded, probe-only, and execution-disabled. Its capability allowlist is exactly probe, and hello/probe-result execution flags must remain false; violations fail closed as protocol errors. On Windows it requests an explicit Job Object lease with kill-on-close and bounded active-process/committed-memory limits; attachment failure is fail-closed and non-Windows support is reported as unsupported. A separate PID or Job Object is lifecycle/resource evidence, not a complete Windows sandbox/security guarantee, restricted token, filesystem/network isolation, or safe external code execution.
- External execution decisions must pass through the immutable application
  `PluginExecutionGate`. Its current policy is globally disabled and every
  catalog decision must expose a deterministic deny reason plus all missing
  prerequisites; approval, enablement, signatures, or containment alone never
  authorize a loader path.
- Do not execute file contents, shell commands, or plugin code as a side effect of opening a document.
- Workspace search is content read-only: it must not save, replace, index, load plugins, or expose editor widgets to a worker. A result path is rechecked against the active workspace root before opening.
- Secrets and local paths must not be written to logs by default.

## Compatibility and release

- Publish x64 first. Add ARM64 only after a separate packaging verification pass.
- A release must include third-party license notices required by the selected Qt/PyQt/QScintilla licensing model.
- A single-file EXE is a distribution format, not a substitute for an installer, file associations, update strategy, or code signing.

## Agent workflow and runtime boundary

- Every material task uses the fixed six-role team: Architect (parent), Project Manager, Product, Developer 1, Developer 2, and QA. The Architect owns integration, final review, verification, and handoff.
- Child routing defaults to `gpt-5.6-luna` at max/Fast; Terra and Sol are escalation-only routes under the repository workflow policy.
- Only the current local checkout is allowed; worktrees are forbidden and only one shared-checkout writer may be active.
- Unit-test assets, mocks, fixtures, and test harnesses are not created or run by default.
- Until the user explicitly reverses the current instruction, QuillForge.exe and Qt windows must not be started by Codex or child roles. Static checks, compilation, and packaging are allowed; runtime startup/visual evidence remains user-owned and unrun.
- Every material handoff requires `docs/handoffs/<handoff-id>/handoff.md`, an index update, a delivery-register update, and a passing handoff verifier.
