# D8 notice inventory coverage parent review

- **Date:** 2026-08-09
- **Delivery:** D8 enterprise release
- **Slice:** exact `uv.lock` package/version coverage in the NOTICE sidecar
- **Owner:** Architect (parent)
- **Disposition:** ACCEPTED WITH LIMITS; legal clearance and external release gates remain open

## Scope and decision

The release candidate now carries a mechanically checked inventory for every
non-editable package resolved by `uv.lock`. The editable `quillforge` project
entry is deliberately excluded, while the CPython runtime-family row remains
an explicit additional release inventory item. The checker compares canonical
package names and exact resolved versions; it does not decide license paths,
validate license texts, or grant redistribution clearance.

The slice is accepted with limits. D8-AC02 remains `defined` because the
release-owner license decision and legal/provenance clearance are still absent.
S13 remains `environment_pending`, and the release dossier remains `no-go`.

## Review findings and fixes

The first delegated Luna/max/Fast read-only review returned `REVISE` with two
blocking findings:

1. `scripts/verify_notice_inventory.py` originally accepted overly broad
   pipe-delimited text. It now parses line-by-line only after a
   `Component | Resolved version` table header and stops at the next non-table
   line.
2. `scripts/verify_release_handoff.ps1` originally could return zero when
   mechanical checks passed but release `openGates` remained populated. It now
   fails closed for either mechanical failures or non-empty open gates.

The same review also identified hard-coded version/path checks. The verifier
now derives the expected version resource from `manifest.version` and the
expected process path from the current checkout root.

The second delegated Luna/max/Fast review returned `PASS`: the original
blocking findings are closed. Residual risks are recorded below rather than
converted into unsupported claims.

## Independent review and simplification

- Independent reviewer: Luna/max/Fast, agent `019fe71e-2ef2-7a81-9726-939b22629ee6`.
- Review coverage: lockfile parsing, editable exclusion, table boundaries,
  version comparison, PowerShell exit-code propagation, open-gate semantics,
  version/path derivation, and release documentation.
- No Terra or Sol escalation was needed after Luna's second review returned
  `PASS`.
- Simplification assessment: the implementation stays standard-library-only
  and uses two small single-purpose readers plus one canonical-name helper.
  No broader Markdown parser or dependency was justified for the current
  fixed NOTICE format; the parser's deliberately narrow boundary is documented
  as a residual risk.
- No production application behavior, public API, or runtime protocol changed.

## Public-source applicability

No embedded C/C++, MCU, BSP/HAL/CMSIS, RTOS, ISR/DMA, driver, or vendor
hardware contract is in this slice. The embedded public-vendor-source workflow
and firmware simplification constraints are therefore not applicable to the
changed Python/PowerShell release tooling. No public license source was used
to make a legal decision; the NOTICE disclaimer and release-owner review remain
authoritative for this project checkout.

## Authorized non-destructive verification

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python scripts/verify_notice_inventory.py --lock uv.lock --notice docs/third_party/NOTICE.md` | `PASS` | 13 non-editable lockfile packages covered; editable project package excluded |
| `uv run python scripts/verify_notice_inventory.py --lock uv.lock --notice dist/NOTICE.md` | `PASS` | Packaged sidecar has the same exact-version coverage |
| `.\scripts\check.ps1` | `PASS` | Handoff contract, formatting, static checks, package metadata, and acceptance checks pass |
| `.\scripts\package.ps1` | `PASS` | Windows x64 one-file candidate rebuilt; no target deployment or launch |
| `.\scripts\verify_release_handoff.ps1` | `EXPECTED NO-GO`, exit 1 | `notice_inventory_complete=true`; three stale/unbound runtime-report failures remain and ten release gates remain open |
| Current identity inspection | `PASS` | Root/dist EXE and NOTICE manifest hashes/sizes match |

## Unrun checks and reason

- QuillForge.exe, QApplication, interactive startup, screenshots, and visual
  acceptance remain prohibited by the active no-launch policy.
- Clean-machine, cross-machine, permission/disk-pressure, hard-power, and
  runtime report refresh evidence remain unavailable or unauthorized.
- Unit tests, mocks, fixtures, harnesses, and test-only assets were not
  created or run.
- Legal license selection, notice-text clearance, signing, installer, update,
  and final support approval require release-owner/external evidence.

## Residual risks and limits

- The NOTICE reader is not a full CommonMark parser; a fenced code block with
  the same table header could still be interpreted as inventory. The current
  NOTICE contains no such block.
- Exact package coverage does not validate hashes, complete license texts,
  selected commercial/GPL/LGPL paths, or redistribution permissions.
- `openGates` contains explicit pending/unrun policy entries, so the current
  candidate must remain no-go even if stale runtime reports are refreshed.

