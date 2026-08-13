# Handoff: 2026-08-10-d8-5-handoff-freshness-enforcement

| Field | Value |
|---|---|
| ID | `2026-08-10-d8-5-handoff-freshness-enforcement` |
| Delivery / slice | `D8 / D8.5 human release-handoff freshness enforcement` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-10T00:34:57.6478623+08:00` |

## User outcome

The release verifier now detects when the human-readable release handoff does
not identify the current package. A later rebuild cannot silently leave the
human handoff claiming an older hash, size, or source snapshot.

## Scope and boundaries

### In scope

- Add a fail-closed `human_handoff_identity_match` check to
  `scripts/verify_release_handoff.ps1`.
- Document the contract in ADR-0030 and bind the rebuilt candidate to current
  manifest/dossier evidence.

### Out of scope

- Regenerating packaged/startup reports or launching any executable.
- Signing, installer, updater, legal, clean-machine, support, cross-machine,
  pressure, or hard-power release gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Design, implementation, review, package, and disposition |
| Project Manager | `repository policy` | Release dependency and status tracking |
| Product | `existing release contract` | Preserve no-go and support boundary |
| Developer 1 | `parent release-tooling slice` | Verifier implementation |
| Developer 2 | `parent packaging integration` | Candidate rebuild and identity sync |
| QA | `parent read-only verification` | Static, handoff, and unrun evidence |

## Changed files and modules

- `scripts/verify_release_handoff.ps1` — validate current human handoff hash,
  byte count, and source snapshot and expose the result in the dossier.
- `docs/adr/0030-release-handoff-freshness.md` — document the enforced human
  handoff contract.
- `docs/RELEASE_HANDOFF.md`, manifest, dossier, acceptance/register evidence,
  and this review/handoff — synchronize current candidate identity.

## Decisions and constraints

- The generated dossier remains authoritative and continues to fail closed for
  stale runtime reports; the new check only adds a human-document identity
  invariant.
- The check uses the existing Markdown contract and standard PowerShell/.NET
  string operations; no dependency or parser was introduced.
- Shared-checkout writer: parent only. Runtime launch and test-only assets
  remain prohibited.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| PowerShell AST parse | `PASS` | `scripts/verify_release_handoff.ps1` parsed without errors. |
| `scripts/package.ps1` | `PASS` | Root/dist portable candidate rebuilt. |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Human handoff identity check passes; three stale-runtime failures and ten open gates remain. |
| Root/dist/manifest identity | `PASS` | SHA-256 `9AF55C83A07A4D18567E217576194F63B541B0577086383E93DC266A9D594975`; 38,328,962 bytes. |
| `scripts/verify_handoff.ps1` | `PASS` | Index, status, required sections, and repository-backed paths validated. |
| `scripts/check.ps1` | `PASS` | NOTICE, formatting, static, metadata, acceptance, and project checks passed. |

## Unrun checks and reason

- EXE/QApplication, packaged diagnostic, interactive/clean-machine startup,
  visual, cross-machine, pressure, and hard-power evidence — prohibited or
  unavailable under the current boundary.
- Signing, installer, updater, legal clearance, and final support approval —
  external owner decisions.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created or
  run under repository policy.

## Known risks and limits

- Runtime reports remain bound to earlier artifacts; the new check does not
  regenerate them.
- Release remains `no-go` with ten external/environment gates open.

## Acceptance and evidence IDs

- Acceptance: `D8-AC06`
- Evidence: `scripts/verify_release_handoff.ps1`, ADR-0030,
  `docs/RELEASE_HANDOFF.md`, `dist/QuillForge.release.json`,
  `docs/release/handoff-2026-08-09.json`, and the parent review.

## Next owner and next action

- Owner: `authorized runtime/release owners`
- Action: regenerate current runtime evidence only after authorization, then
  rerun the freshness gate and retain no-go until all gates clear.

## Artifact information

- Artifact path: `QuillForge.exe` / `dist/QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `9AF55C83A07A4D18567E217576194F63B541B0577086383E93DC266A9D594975` /
  `38,328,962` bytes
- Source snapshot: `tree-sha256:3aa49a122c828f924e5599f226f2a1091702b48f4e54e2a0c028242569bb85fd`
- Packaging note: rebuilt portable candidate; runtime evidence intentionally
  not regenerated.

## Disposition

`accepted-with-limits`: human handoff freshness is mechanically enforced and
the candidate is identity-bound; runtime/legal/release gates remain open.
