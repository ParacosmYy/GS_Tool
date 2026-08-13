# ADR 0018: Process-isolated plugin host protocol

- **Status:** implementation in progress for D6.6
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

QuillForge can start a separate, non-Qt plugin-host process and verify a
versioned handshake without putting a future extension callback in the editor
process. A host timeout, crash, malformed frame, or rejected capability is
reported as a diagnostic result and does not take down the main window.

## Decision

1. The wire contract is newline-delimited UTF-8 JSON, protocol name
   `quillforge.plugin-host`, version `1`, and a 64 KiB maximum frame. The
   current exchange is `hello` → `probe` → `probe-result`; messages have strict
   schemas, bounded fields, and no executable payloads.
2. The host entrypoint is `QuillForge.exe --plugin-host --probe` when frozen,
   or `python -m quillforge --plugin-host --probe` in the source environment.
   The launcher uses an argument list with `shell=False`, pipes stdin/stdout/
   stderr, and never launches a catalog entrypoint. The host entrypoint enforces
   this exact executable-plus-two-flag shape before beginning the stdio exchange.
3. The host reports its PID and `execution_enabled=false`. The process boundary
   is a crash/failure isolation seam and a protocol seam, not yet a Windows
   security sandbox. No external module is imported, resolved, or executed by
   this increment.
4. `SubprocessPluginHost` owns process creation and bounded protocol I/O in the
   infrastructure layer. It maps timeout, non-zero exit, oversized/malformed
   output, and invalid handshake to typed `PluginHostProbeResult` states. The
   application exposes only the `PluginHostClient` protocol; `MainWindow`
   invokes it through `TaskRunner`.
5. A future execution request must add a separate trust gate that proves code
   identity, signature/provenance, permission policy, and platform isolation.
   Descriptor approval and local enablement are necessary governance inputs at
   most; they are not sufficient to turn `execution_enabled` on.

## Consequences

- The main process receives observable evidence of a separate host process and
  can survive host failure or forced termination.
- The JSONL protocol is independently versioned and can later be implemented
  by another host binary without leaking Qt or manager internals across the
  boundary.
- The current host is intentionally diagnostic-only; a green handshake is not
  an external plugin execution or sandbox claim.

## Out of scope

- dynamic import or execution of catalog modules;
- Authenticode/signature verification, publisher identity, installation,
  update, enterprise policy distribution, Windows Job Object/restricted-token
  sandboxing, network/filesystem isolation, or secrets brokering;
- long-lived RPC, streaming document contents, or plugin UI remoting.

## Verification

- protocol codec source smoke for bounds, schema, duplicate-key rejection, and
  round trip;
- exact-argv source smoke for the documented `--plugin-host --probe` entrypoint,
  including rejection of unknown, duplicate, extra, and reordered arguments;
- subprocess smoke for separate PID/handshake, timeout, crash, and malformed
  reply mapping;
- Qt offscreen smoke proving the host diagnostic command uses `TaskRunner`;
- package/startup evidence after the new `--plugin-host` entry path is bundled.
