---
name: project-ci-cd-and-automation
description: Project-local release, toolchain, evidence, and deployment gates for AI Token Tracker.
---

# Project CI/CD and Automation Skill

**Author:** AI Token Tracker Engineering Team
**Maintainer:** Project Owner
**Scope:** `windows/`, `android/`, root launcher, packaging, and deployment evidence

## Delivery sequence

1. Read the current task, API contract, ADR, role review, and release-readiness matrix.
2. Run source-only checks: compile/parse diagnostics, repository-wide line cap, author headers,
   required artifacts, contract references, and `git diff --check`.
3. Run the relevant readonly doctor. A doctor may report `PENDING`; it must not install tools,
   mutate PATH, create release artifacts, open a firewall, or start a production service.
4. For Web changes, collect runtime browser evidence at 320/768/1024/1440, including DOM/ARIA,
   reduced motion, console/network state, focus behavior, and screenshot observations.
5. For Android/EXE/Caddy gates, use the approved environment only. Record exact tool versions,
   artifact hashes, launch/upgrade/rollback evidence, and every item not executed.
6. Update `tasks/todo.md`, `tasks/plan.md`, relevant ADRs, `docs/role-review.md`, and
   `docs/release-readiness.md` in the same delivery slice.

## Non-negotiable boundaries

- Never place passwords, provider keys, bearer tokens, cookies, prompts, or real student data in
  source, logs, screenshots, fixtures, or release archives.
- Do not claim APK, EXE, HTTPS, or real provider integration from a static file or a pending doctor.
- Do not download/install external toolchains without explicit project-owner approval.
- Do not create test-only assets by default; prefer isolated, non-persistent, no-network smoke checks
  when a runtime observation is necessary.
- Keep source files below 1000 lines and preserve the author header on every owned code file.
- Keep the current checkout as the integration boundary; do not create or use worktrees.

## Status semantics

`PASS` requires direct evidence for the named gate. `PENDING` means a required external state is
missing and must remain visible. `FAIL` means the current source or configuration contradicts the
contract and blocks delivery. A release is complete only when all required gates are `PASS` and
the architect signs the release-readiness matrix.
