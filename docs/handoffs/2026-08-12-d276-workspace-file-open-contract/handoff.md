# Handoff: 2026-08-12-d276-workspace-file-open-contract

| Field | Value |
|---|---|
| ID | `2026-08-12-d276-workspace-file-open-contract` |
| Delivery / slice | `D276 / ARCH-246 Workspace file-open contract audit` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The reported “folders open but files do not” path now has a durable static
regression gate. File and directory intents are verified from the workspace
tree through the surface and activation coordinator into asynchronous document
opening, while folders remain navigable.

## Scope and boundaries

- Added `_audit_workspace_file_activation_contract()` to the existing Qt-free
  presentation audit.
- Covered six source routes: tree intents, surface signals, activation
  admission, MainWindow bindings, async document admission, and file picker.
- No runtime behavior, signal timing, containment policy, tab reuse, or
  filesystem policy was changed.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Erdos consultation | Boundary, ownership, and integration review |
| Developer | parent | Qt-free source contract audit |
| QA | parent | Contract, static, package, and handoff verification |
| Independent reviewer | Ampere consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `scripts/audit_presentation_contracts.py`
- D276 ADR, review, plan, roadmap, register, acceptance, and handoff records

## Decisions and constraints

- Keep file/directory signal ownership in `WorkspacePanel` and orchestration in
  existing coordinators.
- Use a source contract rather than importing Qt or adding runtime coupling to
  the audit.
- No new tests, mocks, fixtures, harnesses, or test-only assets were created.
- Embedded workflow is not applicable to this Python/PyQt6 desktop change.

## Verification commands and results

| Evidence | Result |
|---|---|
| File-open contract | `D276-FILE-OPEN-CONTRACT=PASS routes=6` |
| Presentation contract audit | `PASS` |
| Compileall | `D276-COMPILEALL=PASS` |
| Ruff and formatting | `PASS` |
| Project check | `PASS` |
| PE/archive | `D276-PE-ARCHIVE=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI required=8` |
| Root/dist identity | `D276-MANIFEST-COPY=PASS bytes=38584607 sha=A0547AC429E76CB7C6EE8B3A3BB180B2446DFE6871C9F27FFBEC88A9EEE51268` |
| Release handoff | Current artifact identity matches; expected no-go retains only unrun artifact-bound startup/performance checks |

## Unrun checks and reason

Native EXE/Qt startup, native file activation, native dialogs, clean-machine
behavior, signing, installer/update, registry, cross-machine repeatability,
and release-owner acceptance remain unrun under the active no-launch and
non-destructive policy.

## Known risks and limits

The source contract does not prove native event ordering, dialog behavior,
filesystem permission handling, or startup success. Independent review returned
`NO_CONCLUSION`; no independent PASS is claimed.

## Acceptance and evidence IDs

- Acceptance: `S316`.
- Architecture slice: `ARCH-246`.
- Evidence: `D276-FILE-OPEN-CONTRACT=PASS`, `D276-CHECK=PASS`,
  `D276-MANIFEST-IDENTITY=PASS`,
  `D276-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D276-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D276-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: perform native file and folder activation with the current root
  candidate; if file opening still fails, attach the newest startup/error log.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size:
  `A0547AC429E76CB7C6EE8B3A3BB180B2446DFE6871C9F27FFBEC88A9EEE51268` /
  `38,584,607` bytes.
- Source revision:
  `tree-sha256:39e0b45f7001340d67e110e02738adedeb04b532f7514382aa5b1d1f5fcf74a8`.

## Disposition

`accepted-with-limits`: file-open source boundaries are guarded and the
candidate is package-bound; native runtime and enterprise release gates remain
open.

## Review and architecture

- Parent review: `PASS`.
- Architecture role `Erdos the 7th / Luna max`: `NO_CONCLUSION`.
- Independent review `Ampere the 7th / Luna max`: `NO_CONCLUSION`.
- Simplification assessment: `PASS`.
- Public-source applicability: Python 3.12 first-party pathlib/AST behavior
  and the existing workspace/document port contracts; no manufacturer
  requirement applies.
