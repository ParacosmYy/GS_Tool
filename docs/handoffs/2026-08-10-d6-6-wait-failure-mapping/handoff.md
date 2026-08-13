# Handoff: 2026-08-10-d6-6-wait-failure-mapping

| Field | Value |
|---|---|
| ID | `2026-08-10-d6-6-wait-failure-mapping` |
| Delivery / slice | `D6 / D6.6 typed plugin-host wait-failure mapping` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T00:55:39.7941681+08:00` |

## User outcome

Communication failures at the process wait boundary now produce a typed
`crashed` plugin-host diagnostic instead of escaping as an unclassified worker
exception; the exceptional path also closes streams and joins bounded reader
threads. The existing probe-only and execution-disabled boundary is unchanged.

## Scope and boundaries

### In scope

- Catch `OSError`/`ValueError` from the bounded host communication call.
- Preserve launcher PID and containment diagnostics in the typed result.
- Record independent review, static verification, packaging, and unrun limits.

### Out of scope

- Protocol, timeout, capability, or Job Object redesign.
- Dynamic plugin loading or external code execution.
- EXE/QApplication/Qt-window launch or interactive acceptance.
- Unit tests, mocks, fixtures, harnesses, or test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Scope, source repair, integration, final disposition |
| Project Manager | `repository policy` | Dependency, risk, and next-owner tracking |
| Product | `D6.6 acceptance contract` | Preserve typed diagnostics and execution boundary |
| Developer 1 | `parent / infrastructure` | `plugin_host.py` process failure mapping |
| Developer 2 | `not assigned; no presentation change` | Confirm no UI contract change |
| QA | `Descartes / Luna-max requested` | Read-only review and authorized static verification |

## Changed files and modules

- `src/quillforge/infrastructure/plugin_host.py` — map wait/communication
  `OSError`/`ValueError` to a typed crashed result.
- `docs/agent-team/reviews/D6.6-wait-failure-mapping-parent-review.md` — parent
  finding, repair boundary, review and simplification record.
- `docs/handoffs/2026-08-10-d6-6-wait-failure-mapping/handoff.md` — this
  handoff and evidence ledger.
- `docs/handoffs/index.json`, `docs/agent-team/delivery-register.json`, and
  `docs/agent-team/acceptance.json` — traceability for D6-AC06/S19.

## Decisions and constraints

- The catch is limited to the communication call; protocol errors retain their
  existing typed `protocol-error` mapping.
- `finally` cleanup remains authoritative and is not bypassed by the new return.
- `execution_enabled` remains false by construction and validation.
- Shared-checkout writer: parent only, source file above.
- Runtime launch policy: prohibited under the active project instruction.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| Parent source call-chain audit | `PASS WITH LIMITS` | Identified an explicit untyped wait-failure path and applied the local repair. |
| Luna/max post-change review | `PASS — patch-scoped` | Initial and cleanup follow-up reviews confirmed typed crashed mapping, bounded kill/stream-close/reader-join cleanup, preserved timeout/protocol paths, outer `finally`, and execution disabled. Full D6.6 review/runtime acceptance remains open. |
| PowerShell/Python static parse | `PASS` | D6.6 Python modules and changed source parsed without errors. |
| `scripts/verify_handoff.ps1` | `PASS` | Index, status, required sections, and repository-backed paths validated. |
| `scripts/check.ps1` | `PASS` | NOTICE, formatting, static, metadata, acceptance, and project checks passed. |
| `scripts/package.ps1` | `PASS` | Root/dist portable candidate rebuilt after the production source change. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Current dossier remains no-go; stale runtime reports and external gates remain open. |

## Unrun checks and reason

- EXE/QApplication/Qt-window launch, packaged `--plugin-host --probe`, fresh
  process wait-failure injection, Windows Job Object behavior, interactive
  startup, visual, clean-machine, cross-machine, pressure, and hard-power
  evidence — prohibited or unavailable under the current boundary.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under repository policy.

## Known risks and limits

- The repair improves typed error mapping but does not provide independent
  high-risk review or fresh packaged/runtime proof.
- The patch-scoped reviewer retained a risk that reader join timeouts are not
  promoted to a separate failure state and a possible cleanup-exception
  masking risk for custom containment/process implementations; neither is
  broadened into this slice.
- Job Object containment remains a lifecycle/resource boundary, not a complete
  security sandbox or authenticated process identity.
- D6-AC06 and S19 remain `in-progress` until the missing review/runtime gates
  are satisfied.

## Acceptance and evidence IDs

- Acceptance: `D6-AC06`, `S19`
- Evidence: `src/quillforge/infrastructure/plugin_host.py`, parent review above,
  static gates, package identity, and current release no-go dossier.

## Next owner and next action

- Owner: `authorized independent reviewer / runtime owner`
- Action: return a bounded review conclusion, then run authorized fresh host
  process evidence and retain no-go until all release gates clear.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `56749BC332C38E4337F82E8584E939A94EBFA236DC059D6DAE894BD703C46464` /
  `38,330,253` bytes
- Source snapshot: `tree-sha256:4b236625e995773148825f5817b27a25c0ea0f6c828f6912579dd6cb3eafe350`
- Packaging note: rebuilt portable candidate; runtime evidence intentionally not
  regenerated.

## Disposition

`accepted-with-limits`: the typed wait-failure repair passed its patch-scoped
independent review. Static/package gates and the broader D6.6 runtime/release
limits remain open.
