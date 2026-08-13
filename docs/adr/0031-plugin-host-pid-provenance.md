# ADR-0031: Plugin-host PID provenance

- **Status:** implementation in progress for D6.6/D6.7
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The diagnostic host protocol reports the host process's own PID, while the
parent process observes the PID returned by process creation. A launcher or
re-exec wrapper may make those values differ. The former application result
used one `host_pid` field for both sources, which made failure diagnostics look
like a protocol-reported host identity and could invite future lifecycle or
authorization misuse.

## Decision

`PluginHostProbeResult` carries two explicit, optional provenance fields:

1. `launcher_pid` is the PID observed by the parent from the process adapter.
   It is available after process creation, including timeout, crash, protocol,
   and containment failure results.
2. `reported_host_pid` is the PID reported by a successfully decoded host hello
   frame and kept only when that handshake evidence exists. It is not filled by
   a pre-handshake failure path.

The hello/result consistency check remains. The implementation does not require
`reported_host_pid == launcher_pid`, because the supported launcher/re-exec
compatibility path may have two Job Object members. Neither integer is an
authorization, code-identity, signature, or containment proof; future external
execution must bind those properties to an OS-owned process handle, Job Object
membership, and independently verified code identity.

## Consequences and limits

- User-facing diagnostics state whether a PID is launcher-observed or
  host-reported, removing source ambiguity.
- A timeout or malformed frame cannot fabricate a host-reported PID before a
  hello is decoded.
- The current host remains probe-only and `execution_enabled=false`.
- This is provenance hygiene, not process authentication, sandboxing, or a
  security certification claim.

## Public platform references

These public Microsoft Learn references describe the platform distinction used
by this decision; they do not constitute a product security certification:

- [Process handles and identifiers](https://learn.microsoft.com/en-us/windows/win32/procthread/process-handles-and-identifiers)
- [Job Objects](https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects)
- [Process creation flags](https://learn.microsoft.com/en-us/windows/win32/procthread/process-creation-flags)

## Verification

- Static source review confirms that `PluginExecutionEvidence` contains no PID
  field and the global external execution gate remains disabled.
- Protocol/subprocess verification must cover direct host and wrapper/re-exec
  provenance when runtime execution is authorized.
- QuillForge startup, Qt inspection, packaged host probe, and visual acceptance
  remain unrun under the current project no-launch instruction.

