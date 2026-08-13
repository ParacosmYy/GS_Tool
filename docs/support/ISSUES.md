# QuillForge support issue route

This file is the local, offline issue route for the current portable candidate.
It is intentionally repository-scoped and does not imply a public support
service or an enterprise release.

## Owner

Project Manager (QuillForge) owns triage, acceptance-state updates, and routing
of release-blocking issues. The Architect owns technical disposition and
evidence integration; Product owns user-impact and distribution decisions; QA
owns verification requests and unrun-environment records.

## Intake format

Add one entry with:

- date and reporter;
- artifact version and SHA-256;
- environment and reproduction steps;
- expected and observed behavior;
- severity (`blocker`, `high`, `normal`, or `low`);
- evidence path or log reference;
- requested owner and current status.

Do not include secrets, personal data, credentials, or full user documents.
Attach only the smallest sanitized reproduction needed to investigate.

## Routing rules

1. A data-loss, corruption, or release-integrity issue is `blocker` and goes to
   Architect plus QA before any release decision changes.
2. A user-facing workflow regression goes to Product and the owning Developer
   role, with QA verification before acceptance changes.
3. A packaging, signing, installer, or update issue goes to Project Manager
   plus Release Engineering/Product; it remains release-blocking until the
   handoff dossier is regenerated.
4. Every closed item records the evidence path and the acceptance or delivery
   status it changed.

## Current route state

The route is assigned and machine-declared in
`dist/QuillForge.release.json`, but the release remains `no-go` while clean
machine, legal, signing, installer, update, filesystem-pressure, and
cross-machine gates remain open.

The companion [`HANDOFF.md`](HANDOFF.md) records the candidate-bound support
packet and the acceptance action required before this route can be treated as
an operational handoff.
