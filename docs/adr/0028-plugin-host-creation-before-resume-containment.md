# ADR 0028: Plugin-host creation-before-resume containment

- **Status:** implementation in progress for D6.7
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Context

The first D6.7 implementation created the diagnostic child with `Popen()` and
attached it to a Windows Job Object afterward. The protocol request was held
until attachment, but the child could still execute its interpreter startup
and allocate resources before `AssignProcessToJobObject` completed.

## Decision

1. The default Windows `ProcessContainmentLauncher` creates and configures the
   Job Object before creating the child.
2. It launches the child with `CREATE_SUSPENDED`, assigns the process to the
   configured Job Object while the initial thread has not resumed, and resumes
   that initial thread only after assignment succeeds.
3. The initial thread is located through the documented Tool Help thread
   snapshot APIs. Failure to locate, open, or resume it is a containment launch
   failure; the child is terminated, handles are closed, and no protocol
   exchange is started.
4. The existing `attach()` seam remains available for injected or legacy
   adapters. The built-in Windows and unsupported adapters implement
   `launch()`, so the production composition root uses the creation-before-
   resume path. An attach-only custom adapter reports
   `attached-after-start`, not `attached`; it remains a compatibility seam,
   not a future external-execution security guarantee.
5. The bounded policy remains unchanged: kill-on-close, at most two active
   processes for launcher/re-exec plus host, and a 256 MiB per-process
   committed-memory limit. `execution_enabled` remains false.

## Consequences

- The default diagnostic host has no user-mode execution window before Job
  Object assignment.
- The typed distinction between `attached` and `attached-after-start` keeps
  future execution policy from treating a post-start compatibility path as a
  creation-time containment proof.
- Python's existing `subprocess.Popen` stdio and timeout behavior remains in
  use; the containment adapter owns only the launch ordering and Win32 thread
  resume boundary.
- Parent-process Job Object restrictions and Windows-environment differences
  can still reject the launch. The result is typed as a containment error and
  fails closed.
- This is lifecycle/resource containment, not a complete security sandbox and
  not evidence of code identity, signature trust, token restriction,
  filesystem/network isolation, secret brokering, or certification.

## Public primary-source basis

| Vendor | Document | Scope used | URL |
|---|---|---|---|
| Microsoft | CreateProcessW function | `CREATE_SUSPENDED`, startup handles, process/thread creation | <https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw> |
| Microsoft | AssignProcessToJobObject function | assignment before resuming the initial thread | <https://learn.microsoft.com/en-us/windows/win32/api/jobapi2/nf-jobapi2-assignprocesstojobobject> |
| Microsoft | CreateToolhelp32Snapshot function | enumerating the suspended process's initial thread | <https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot> |
| Microsoft | Thread32First / Thread32Next | `THREADENTRY32` traversal | <https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-thread32first> |
| Microsoft | ResumeThread function | resuming the initial suspended thread | <https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-resumethread> |
| Microsoft | Job Objects | kill-on-close and nested-job behavior | <https://learn.microsoft.com/en-us/windows/win32/procthread/job-objects> |

These are Windows API contracts for the supported Windows x64 desktop scope;
they are not a product-wide security or certification claim.

## Verification

- real Windows source smoke for creation-before-resume host launch and
  attached diagnostic result;
- real Windows source smoke for attach-only fail-closed mapping,
  unsupported fallback, timeout, crash, and kill-on-close cleanup;
- Qt offscreen projection smoke after packaging;
- independent post-fix review and release-handoff evidence with the final
  artifact hash refreshed.

The latest source hardening was checked with Ruff and `scripts/check.ps1`.
Fresh process, Qt, packaged, and interactive runs remain intentionally
unperformed because the user has prohibited starting the software.
