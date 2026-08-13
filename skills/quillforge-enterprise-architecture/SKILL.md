---
name: quillforge-enterprise-architecture
description: Plan and implement incremental enterprise architecture changes in QuillForge with explicit module boundaries, typed contracts, public-source grounding, ADRs, independent review, simplification, and evidence. Use when changing application architecture, APIs, composition roots, plugins, infrastructure adapters, cross-module dependencies, release governance, or large refactors.
---

# QuillForge Enterprise Architecture

## Overview

Use this skill to move QuillForge toward an enterprise-grade architecture
without a risky wholesale rewrite. Enterprise quality here means explicit
ownership, stable contracts, observable failure paths, incremental migration,
and reproducible evidence—not a claim to any company's private methodology.

## Required workflow

1. Establish a written spec and update `tasks/plan.md` / `tasks/todo.md` for
   multi-file work. State the user outcome, non-goals, compatibility promise,
   risk, and acceptance evidence before editing.
2. Inspect the current dependency graph and exact package/tool versions. Use
   public, first-party, versioned documentation for framework behavior. Public
   ByteDance open-source material can inform patterns, but never treat it as an
   internal ByteDance standard or copy an assumption without a source.
3. Define the contract first: typed input/output, error semantics, ownership,
   lifecycle, and versioning. Keep one canonical contract per boundary; avoid
   parallel temporary APIs.
4. Preserve the dependency direction:
   `presentation -> application -> domain`; infrastructure implements ports;
   plugins depend on public plugin contracts; the composition root wires the
   graph. Domain code must not import Qt, filesystem, subprocess, or widget
   types.
5. Choose the smallest vertical migration slice. Add an adapter or façade at
   an existing boundary before moving internals. Keep behavior compatible and
   make invalid state explicit rather than silently repairing it.
6. Ask the fixed Architect-led team for bounded role input, then perform an
   independent read-only code review and a behavior-preserving simplification
   assessment. Record no-conclusion results honestly.
7. Verify non-destructively: compile/lint/format, contract/source probes,
   configured static checks, package provenance, and authorized diagnostics.
   Do not create or run unit-test-only assets under the project policy and do
   not launch Qt unless the user explicitly reverses the no-launch boundary.
8. Record one ADR for a material architecture decision and exactly one new
   handoff for the material slice. Synchronize acceptance JSON, delivery
   register, handoff index, roadmap, and current artifact identity.

## QuillForge architecture invariants

- `MainWindow` and other Qt widgets are composition/presentation concerns, not
  application services. Do not leak widget instances through a port.
- Application services own use-case sequencing and translate domain results to
  presentation projections; they do not own QSS, fonts, or locale widgets.
- Domain models and policies remain deterministic and framework-neutral.
- Infrastructure owns filesystem, subprocess, process isolation, persistence,
  release metadata, and platform APIs behind narrow ports.
- Plugin discovery, trust, enablement, capability, and execution are separate
  decisions. External entries remain deny-by-default until every required gate
  exists.
- Async operations expose lifecycle/status contracts and close-safe ownership;
  background workers never mutate widgets directly.
- Errors have typed meaning and preserve diagnostics without exposing secrets or
  implementation details across a public boundary.

## Review checklist

- Does the diff have one owner per changed boundary and no reverse import?
- Is there exactly one source of truth for each contract and setting?
- Are compatibility, failure, cancellation, concurrency, and close behavior
  explicit?
- Can the slice be removed or rolled back without destroying user data?
- Did simplification reduce indirection without weakening observability or
  safety?
- Are all claims separated into public-source applicability, project rule, or
  engineering judgment?
- Are runtime, clean-machine, legal, signing, and deployment limitations stated
  instead of inferred away?
