# Handoff: 2026-08-09-d6-6-handle-conversion-fix

| Field | Value |
|---|---|
| ID | `2026-08-09-d6-6-handle-conversion-fix` |
| Delivery / slice | `D6 / D6.6` Windows Job Object handle conversion repair |
| Status | `in-progress` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T23:40:21+08:00` |

## User outcome

The Windows Job Object containment adapter no longer converts native
`wintypes.HANDLE` objects with the invalid `int(handle)` operation. Both Job
Object creation and process assignment now read `handle.value`, removing a
reproducible fail-before-assignment defect while preserving the existing
fail-closed lifecycle policy.

D6.6 remains open because the required independent high-risk review and fresh
packaged host/containment evidence are unavailable under the current boundary.

## Scope and boundaries

### In scope

- Repair `_Win32JobApi.create_job()` and `_Win32JobApi.open_process()` native
  handle conversion.
- Re-audit the D6.6 protocol/subprocess/containment/UI call chain.
- Rebuild the current candidate and synchronize current evidence.

### Out of scope

- Dynamic loading, signatures, code identity, external execution, complete
  security sandboxing, installation, updates, or certification claims.
- QuillForge.exe/QApplication launch, packaged host probing, interactive UI,
  cross-machine/clean-machine/pressure/hard-power evidence.
- Unit tests, mocks, fixtures, harnesses, and test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate the minimal fix and retain D6.6 open |
| Project Manager | Parent role record | Track high-risk review and runtime evidence dependency |
| Product | Parent role record | Preserve diagnostic-only scope and no security claim |
| Developer 1 | Parent role record | Review ctypes/Win32 handle boundary |
| Developer 2 | Parent role record | Review subprocess, containment, and TaskRunner integration |
| QA | Luna/max then Terra/max; no conclusion returned | Independent high-risk review requested; no child PASS claimed |

## Changed files and modules

- `src/quillforge/infrastructure/process_containment.py` — use
  `HANDLE.value` for Job Object and process handles.
- `docs/agent-team/reviews/D6.6-handle-conversion-parent-review.md` — record
  finding, repair, review status, simplification, and evidence.
- `docs/agent-team/reviews/D6.6-parent-review.md` — append the superseding
  source-audit correction.
- `docs/agent-team/acceptance.json` and
  `docs/agent-team/delivery-register.json` — link the current repair while
  keeping D6-AC06/S19 `in-progress`.
- `docs/handoffs/index.json` and this handoff — record the material slice.
- `QuillForge.exe`, `dist/QuillForge.exe`, and
  `dist/QuillForge.release.json` — rebuilt current candidate artifacts.

## Decisions and constraints

- The parent confirmed the defect with a read-only ctypes type probe; no
  speculative Win32 redesign was introduced.
- The two-line `.value` repair is the smallest complete diff and preserves
  existing Job Object limits, launch ordering, lease cleanup, PID semantics,
  and `execution_enabled=false`.
- D6-AC06 and S19 remain `in-progress`; no historical runtime evidence is
  relabeled as current proof.
- The project no-launch and no-test-asset constraints remain binding.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `ctypes.wintypes.HANDLE(123)` conversion probe | `PASS` | `.value` conversion works; prior direct conversion failure reproduced |
| AST parse of D6.6 Python modules | `PASS` | Five protocol/host/containment/application modules parsed |
| `.\scripts\check.ps1` | `PASS` | Full project static gate passed |
| `.\scripts\package.ps1` | `PASS` | Current portable Windows x64 candidate rebuilt |
| `.\scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO`, exit 1 | Three artifact-bound stale-runtime failures; ten external gates open |
| Root/dist/manifest identity | `PASS` | Current artifact and manifest agree |

## Unrun checks and reason

- Packaged `--plugin-host --probe`, fresh Job Object assignment/resume/cleanup,
  QuillForge.exe, QApplication, interactive startup, visual review, clean
  machine, cross-machine, permission/disk pressure, hard-power, and runtime
  report refresh remain prohibited or unavailable.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.
- Luna and Terra high-risk reviewers returned no conclusion in their bounded
  windows; no independent child PASS is claimed.

## Known risks and limits

- D6.6 remains `in-progress` pending an independent high-risk conclusion and
  authorized fresh packaged/runtime evidence.
- A valid native handle conversion does not itself prove Job Object membership,
  process identity, security sandboxing, or cross-machine cleanup behavior.
- The host remains diagnostic-only and probe-only; external execution stays
  disabled.

## Acceptance and evidence IDs

- `D6-AC06` remains `in-progress`.
- `S19` remains `in-progress`.
- Evidence: `src/quillforge/infrastructure/process_containment.py`,
  `docs/agent-team/reviews/D6.6-handle-conversion-parent-review.md`,
  `dist/QuillForge.release.json`, and the current release dossier.

## Next owner and next action

- Owner: Architect / QA / authorized Windows runtime owner.
- Action: obtain an independent high-risk conclusion and authorized packaged
  host/containment evidence before promoting D6.6 or S19.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version / architecture: `0.1.0` / `Windows X64`
- SHA-256 / size: `C39C069B57790E90E3D34E77A95D93E2D152865CCC27BEC1943E4F1404DD878B` /
  `38,329,173` bytes; root and dist copies match
- NOTICE SHA-256 / size: `2D91F74D29F983E390F25F4868323E26BED2544F945BB674414904046CD00BB8` /
  `3,331` bytes
- Source snapshot: `tree-sha256:8e3867b0fe4a5be1d85e79d4126dbad39b5211f85fa736e0630baa98f95a03b9`

## Disposition

`in-progress`: the reproducible native-handle conversion defect is repaired
and current static/package evidence is recorded, but D6.6/S19 cannot honestly
promote without independent high-risk and authorized runtime evidence.

