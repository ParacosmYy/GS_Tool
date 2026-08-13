# Handoff: 2026-08-09-d8-notice-inventory-coverage

| Field | Value |
|---|---|
| ID | `2026-08-09-d8-notice-inventory-coverage` |
| Delivery / slice | `D8 / D8.1 / D8-AC02` NOTICE inventory coverage |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only; no worktree |
| Created | `2026-08-09T23:29:25+08:00` |

## User outcome

The release candidate's NOTICE sidecar now has a mechanical exact-version
coverage check against `uv.lock`. All 13 non-editable resolved packages are
listed, and the editable project package is explicitly excluded. The check is
connected to both normal project checks and the release handoff verifier.

This closes the mechanical inventory gap only. It does not provide a legal
license decision or change the release candidate's no-go status.

## Scope and boundaries

### In scope

- Add `scripts/verify_notice_inventory.py` using only Python's standard library.
- Cover every non-editable `uv.lock` package with an exact NOTICE version row.
- Make NOTICE coverage fail closed in `scripts/check.ps1` and
  `scripts/verify_release_handoff.ps1`.
- Make the release verifier fail closed when mechanical checks or explicit
  open release gates remain.
- Rebuild the current portable Windows x64 candidate and update current
  manifest/acceptance/register evidence.

### Out of scope

- Legal clearance, license selection, complete license texts, signing,
  installer, updater, file associations, clean-machine release approval, or
  support acceptance.
- Launching QuillForge.exe, QApplication, interactive GUI checks, screenshots,
  runtime report refresh, target deployment, or hardware operation.
- Unit tests, mocks, fixtures, harnesses, and test-only assets.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | Parent Codex | Integrate the inventory contract and retain external gates |
| Project Manager | Parent role record | Track release evidence and support boundary |
| Product | Parent role record | Preserve no-go/legal wording and distribution limits |
| Developer 1 | Parent role record | Implement the Python inventory checker |
| Developer 2 | Parent role record | Integrate PowerShell release/check gates and package identity |
| QA | Luna/max/Fast agent `019fe71e-2ef2-7a81-9726-939b22629ee6` | Independent read-only review; second pass `PASS` |

## Changed files and modules

- `scripts/verify_notice_inventory.py` — new lockfile-to-Markdown coverage
  checker with bounded table parsing and editable-package exclusion.
- `docs/third_party/NOTICE.md` — complete 13-package resolved inventory and
  explicit legal-review limits.
- `scripts/check.ps1` — enforce source NOTICE coverage.
- `scripts/verify_release_handoff.ps1` — enforce packaged NOTICE coverage,
  dynamic current-checkout identity expectations, and non-zero open-gate exit.
- `docs/RELEASE.md`, `docs/DEPENDENCIES.md`, and `docs/ROADMAP.md` — document
  mechanical coverage without implying legal clearance.
- `docs/agent-team/acceptance.json` and
  `docs/agent-team/delivery-register.json` — synchronize current evidence while
  keeping D8-AC02 `defined` and S13 `environment_pending`.
- `docs/agent-team/reviews/D8-notice-inventory-coverage-parent-review.md` —
  independent review, simplification, applicability, and limits record.
- `dist/QuillForge.exe`, root `QuillForge.exe`, `dist/NOTICE.md`, and
  `dist/QuillForge.release.json` — rebuilt current candidate artifacts.

## Decisions and constraints

- The editable `quillforge` lock entry is excluded from third-party inventory;
  all 13 non-editable resolved packages require exact NOTICE rows.
- Mechanical version coverage is accepted as inventory evidence only;
  D8-AC02 remains defined until a release owner selects and clears the
  applicable license/notice path.
- The release remains unsigned, portable, and no-go. No launch, test asset,
  deployment, or hardware operation is authorized in this slice.

## Public-source applicability

This slice changes Python/PowerShell release tooling only. No embedded C/C++,
MCU, BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, or vendor hardware contract is
present, so the embedded public-vendor-source requirement is not applicable.
The notice inventory is provenance bookkeeping, not a legal determination; no
manufacturer or third-party license claim is elevated to project clearance.

## Independent review and simplification assessment

The first Luna review returned `REVISE` for broad Markdown row matching and a
release-verifier exit-code gap. Both were fixed. The second Luna review
returned `PASS`, confirming line-bounded table parsing, dynamic identity
derivation, and fail-closed open-gate behavior.

The implementation was kept standard-library-only and narrowly scoped. No
additional dependency, general Markdown parser, or production abstraction was
justified. The deliberate narrow-format parser risk is retained in the review
record and does not affect the current NOTICE content.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python scripts/verify_notice_inventory.py --lock uv.lock --notice docs/third_party/NOTICE.md` | `PASS` | 13 lockfile packages covered; editable project package excluded |
| `uv run python scripts/verify_notice_inventory.py --lock uv.lock --notice dist/NOTICE.md` | `PASS` | Packaged sidecar coverage matches source inventory |
| `.\scripts\check.ps1` | `PASS` | Handoff, formatting, static, metadata, and acceptance checks pass |
| `.\scripts\package.ps1` | `PASS` | Current Windows x64 portable candidate rebuilt |
| `.\scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO`, exit 1 | Dossier records `notice_inventory_complete=true`, three stale runtime-report failures, and ten open gates |
| Root/dist identity inspection | `PASS` | EXE hashes/sizes and NOTICE manifest hash match |

## Unrun checks and reason

- QuillForge.exe, QApplication, interactive startup, visual acceptance, and
  screenshots were not run because the no-launch instruction is active.
- Clean-machine and cross-machine startup, permission/disk-pressure,
  hard-power, runtime report refresh, legal clearance, signing, installer,
  update, and final support approval remain unavailable or unauthorized.
- No unit tests, mocks, fixtures, harnesses, or test-only assets were created
  or run.

## Known risks and limits

- The NOTICE reader is intentionally narrow rather than a full CommonMark
  parser; a fenced block with the same table header could be misread. The
  current NOTICE has no such block.
- Exact package/version coverage does not validate hashes, full license texts,
  commercial/GPL/LGPL selection, or redistribution permissions.
- The current dossier remains no-go because three runtime reports are stale or
  artifact-unbound and ten explicit release gates are open.

## Acceptance and evidence IDs

- `D8-AC02` remains `defined`: exact package/version coverage is present, but
  the license decision and legal/provenance clearance are still pending.
- `D8.1` remains `accepted-with-limits`.
- `S13` remains `environment_pending`.
- Evidence: `scripts/verify_notice_inventory.py`, `uv.lock`,
  `docs/third_party/NOTICE.md`, `dist/NOTICE.md`,
  `dist/QuillForge.release.json`,
  `docs/release/handoff-2026-08-09.json`, and the parent review above.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version / architecture: `0.1.0` / `Windows X64`
- SHA-256 / size: `4846E875E4DE97CFAFE507978BD2BD60041FE5D62A9C1ECB2DAF6E234FBB7342` /
  `38,327,593` bytes; root and dist copies match
- NOTICE SHA-256 / size: `2D91F74D29F983E390F25F4868323E26BED2544F945BB674414904046CD00BB8` /
  `3,331` bytes
- `uv.lock` SHA-256: `6D4DC74574DFC75FFBE1EF314D589638DCF6B20D414FE7366CD55634C5D135B4`
- Source snapshot: `tree-sha256:480c2e541863d2c3eb6e5dfe4fc88a393d24551436ac8098b1630a7104ff6c4d`

## Next owner and next action

- Owner: release owner / legal reviewer / QA environment owner.
- Action: decide the license/notice path and obtain authorized clean-machine,
  signing, installer/update, support, and refreshed runtime evidence before any
  public release claim.

## Disposition

`accepted-with-limits`: mechanical NOTICE inventory coverage and fail-closed
release-tooling integration are recorded. Legal clearance, external runtime
evidence, and all open release gates remain outstanding.
