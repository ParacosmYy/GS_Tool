# ADR 0019: Plugin-host lifecycle and resource containment

- **Status:** implementation in progress for D6.7
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

QuillForge can report whether its diagnostic plugin-host process is governed by
an explicit lifecycle/resource-containment adapter. On Windows, the host is
attached to an unnamed Job Object with bounded active processes and committed
process memory, and the last job handle closes with kill-on-close semantics. If
Windows cannot attach the process, the probe fails closed instead of silently
continuing without the requested containment. Other platforms report an
explicit unsupported diagnostic fallback. External extension execution remains
disabled in every state.

## Decision

1. `SubprocessPluginHost` receives a `ProcessContainment` adapter from the
   infrastructure layer and owns one short-lived containment lease per probe.
   The adapter does not know about Qt, plugin manifests, or application state.
2. The Windows adapter creates an unnamed Job Object, sets
   `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`, `JOB_OBJECT_LIMIT_ACTIVE_PROCESS` with a
   maximum of two active processes (one launcher/re-exec wrapper plus the host),
   and `JOB_OBJECT_LIMIT_PROCESS_MEMORY` with a
   256 MiB per-process committed-memory limit through
   `JOBOBJECT_EXTENDED_LIMIT_INFORMATION`. It then assigns the child process
   to the job. The default launch path creates the child suspended and resumes
   its initial thread only after assignment; the ordering contract is detailed
   in ADR 0028. It does not request breakaway from an existing parent job.
3. The adapter closes process/job handles deterministically. Closing the last
   job handle is the lifecycle cleanup path; timeout handling still kills and
   waits for the child before the lease is released. A pre-existing parent job,
   access failure, unsupported API, or any other attach failure is surfaced as
   `containment-error`; the diagnostic protocol is not started in that case.
4. Non-Windows builds use an explicit unsupported lease so the diagnostic
   protocol remains observable without pretending that a Windows Job Object is
   available. The result carries `attached` (creation-before-resume),
   `attached-after-start` (legacy/attach-only compatibility), `unsupported`,
   `failed`, or `not-requested`, plus the configured limit labels. Only
   `attached` is eligible for any future creation-time execution gate.
5. `PluginHostProbeResult` remains the application-owned immutable projection.
   Presentation shows containment state and limits as diagnostics; it does not
   gain access to ctypes, process handles, or platform APIs.
6. `execution_enabled` remains a hard-coded false capability in the protocol
   and result path. Job Objects are lifecycle/resource controls, not proof of
   code identity, signature, token restriction, filesystem isolation, network
   isolation, secret brokering, or a complete security sandbox.

## Public primary-source basis

The implementation uses these versioned Microsoft Learn Win32 contracts for
Windows x64 desktop scope:

| Vendor | Document | Scope used | URL |
|---|---|---|---|
| Microsoft | CreateJobObjectW function (`jobapi2.h`) | unnamed job creation, last-handle cleanup, kill-on-close behavior | <https://learn.microsoft.com/en-us/windows/win32/api/jobapi2/nf-jobapi2-createjobobjectw> |
| Microsoft | AssignProcessToJobObject function (`jobapi2.h`) | process assignment and the already-in-a-job failure/nesting behavior | <https://learn.microsoft.com/en-us/windows/win32/api/jobapi2/nf-jobapi2-assignprocesstojobobject> |
| Microsoft | SetInformationJobObject function (`jobapi2.h`) | applying extended job limits | <https://learn.microsoft.com/en-us/windows/win32/api/jobapi2/nf-jobapi2-setinformationjobobject> |
| Microsoft | `JOBOBJECT_BASIC_LIMIT_INFORMATION` (`winnt.h`) | active-process, kill-on-close, and process-memory limit flags | <https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_basic_limit_information> |
| Microsoft | `JOBOBJECT_EXTENDED_LIMIT_INFORMATION` (`winnt.h`) | per-process committed-memory field and structure layout | <https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-jobobject_extended_limit_information> |
| Microsoft | Job Objects | nested-job and lifecycle semantics; security limits are separate | <https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects> |

These are manufacturer/platform requirements for the Windows API surface only;
they are not a claim of certification, enterprise security compliance, or
support on unverified Windows environments.

## Consequences

- A host crash or timeout has an explicit process-lifecycle owner, and a
  successful Windows diagnostic result includes the resource policy that was
  applied before the child was resumed.
- The infrastructure seam stays replaceable: a future restricted-token,
  capability broker, or another OS adapter can be added without changing the
  application port or Qt projection.
- A host launched from an environment that already imposes incompatible Job
  Object rules can be rejected rather than receiving an untracked weaker mode.
- Resource values are diagnostic defaults, not a product-wide performance or
  security guarantee; they must be measured on supported environments before
  any external execution feature is considered.

## Out of scope

- dynamic import or execution of catalog modules;
- Authenticode/signature verification, publisher identity, installation,
  updates, remote policy, restricted tokens, AppContainer, ACL changes,
  filesystem/network/secret isolation, or brokered capabilities;
- arbitrary child-process orchestration, long-lived RPC, UI remoting, or
  resource telemetry beyond the containment status returned by this probe;
- a claim that a Job Object is a complete sandbox or security boundary.

## Verification

- source smoke for Windows adapter state, configured limits, invalid-limit
  rejection, idempotent lease cleanup, and unsupported-platform fallback;
- subprocess smoke for attached/failed containment, distinct PID, timeout,
  crash, malformed reply, and `execution_enabled=false`;
- Qt offscreen smoke proving containment diagnostics remain behind `TaskRunner`;
- packaged `--plugin-host --probe`, startup, repeat-measurement, and release
  handoff evidence with the artifact hash refreshed.
