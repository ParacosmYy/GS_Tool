# QuillForge support handoff packet

Status: prepared locally, pending owner acceptance and clean-machine evidence.

This packet defines the offline support boundary for the current portable
candidate. It is a repository-scoped handoff record, not a public support
service, service-level agreement, or enterprise-release approval.

## Candidate identity

- Artifact set: `dist/QuillForge.exe` and the root test copy
- SHA-256: `4C6188F655EDB31C58E835FEBBC04DA21302F480E2516D916E9D140D94C43BD5`
- Size: `38,547,913` bytes
- Source snapshot: `tree-sha256:89f8e174586c4a4e5d2460668cc5bf23eef5327c8f32ab743e3c3acf121a39cf`
- Manifest: `dist/QuillForge.release.json`
- Intake route: [`ISSUES.md`](ISSUES.md)

## Ownership

| Responsibility | Owner | Boundary |
|---|---|---|
| Initial triage and routing | Project Manager (QuillForge) | Records the issue, severity, candidate identity, and requested owner. |
| Technical disposition | Architect | Determines whether the report changes architecture, safety, recovery, or release evidence. |
| User-impact decision | Product | Decides distribution scope, supported workflow, and user-facing disposition. |
| Verification request | QA | Runs only the separately authorized environment or workload checks. |
| Packaging and trust | Release Engineering | Owns signing, installer, update, rollback, and distribution evidence. |

## Triage contract

Every intake entry must include the date, reporter, exact candidate SHA-256,
Windows environment, reproduction steps, expected/observed behavior, severity,
evidence path, requested owner, and current status. Redact secrets, personal
data, credentials, and full user documents.

Escalate immediately to Architect plus QA for data loss, corruption, recovery,
or artifact-integrity reports. Route packaging, signing, installer, update, or
file-association reports to Project Manager plus Release Engineering and
Product. A release decision must not change from an issue entry alone; the
handoff dossier and acceptance evidence must be regenerated.

## Explicit limits

The current checkout does not authorize executable launch, clean-machine
attestation, cross-machine repeatability, permission/disk-pressure workloads,
hard-power durability checks, signing, installer deployment, update-channel
operation, or legal/license clearance. No support range, native-memory limit,
enterprise support promise, or durability guarantee is implied by this packet.

## Acceptance action

The Project Manager and QA owner must acknowledge this packet on the target
support channel, then attach the authorized clean-machine/support matrix and
its artifact-bound evidence. Until that happens, the release manifest remains
`support.status = handoff-pending` and the release dossier remains `no-go`.
