# Compat Migration and Skeletonization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild the project's maintainable file-structure and constraint-entry skeleton without changing existing source behavior.

**Architecture:** Keep the current working code paths intact, but formalize a migration skeleton that clearly separates stable entry docs, module responsibilities, frozen historical branches, and future landing zones. The work is documentation- and structure-first: we will tighten the constraint system, define canonical folders, and add skeletal landing areas for future features while leaving production source behavior untouched.

**Tech Stack:** Markdown docs, repository directory layout, git worktree-safe editing, existing Qt/CMake project structure.

---

### Task 1: Rebuild the constraint entry and workflow split

**Files:**
- Modify: `AGENTS.md`
- Modify: `docs/constraints/02-workflow.md`
- Modify: `docs/constraints/01-project-overview.md`
- Modify: `docs/constraints/06-git-commit.md`
- Modify: `docs/tracking/SCORE_TRACKING.md`

- [ ] **Step 1: Write the updated constraint flow**

Update the entry docs so they only keep stable index content and push dynamic status, score history, and commit template details into the proper module docs.

- [ ] **Step 2: Verify the new split is internally consistent**

Confirm that PRD, bugfix, technical debt, documentation, and build work are each assigned a single primary route with no duplicated ownership.

- [ ] **Step 3: Save and review the markdown diff**

Inspect the rewritten sections for contradictions, repeated rules, and stale references.

### Task 2: Consolidate architecture boundaries and the target folder skeleton

**Files:**
- Modify: `docs/constraints/03-architecture.md`
- Modify: `docs/constraints/07-directory-structure.md`
- Modify: `docs/constraints/01-project-overview.md`

- [ ] **Step 1: Rewrite the architecture boundary language**

Merge overlapping dependency descriptions into one canonical layering story and describe the migration target as a future-friendly skeleton instead of a vague aspiration.

- [ ] **Step 2: Add frozen-branch and landing-zone rules**

Define which historical split directories are frozen, which folders are canonical, and where future work should land.

- [ ] **Step 3: Validate the folder taxonomy against the current repository**

Ensure every top-level module has one clear role, and every historical fork is labeled as frozen or transitional.

### Task 3: Tighten coding, UI, Git, and icon standards

**Files:**
- Modify: `docs/constraints/04-coding-standard.md`
- Modify: `docs/constraints/05-ui-standard.md`
- Modify: `docs/constraints/06-git-commit.md`
- Modify: `docs/constraints/08-icon-standard.md`

- [ ] **Step 1: Normalize the default-vs-exception language**

Make the line-count and file-size rules default targets with explicit exception handling instead of rigid hard stops.

- [ ] **Step 2: Turn UI rules into checkable requirements**

Ensure token rules, state names, objectName usage, tr(), animation, and responsive behavior read like an executable checklist.

- [ ] **Step 3: Remove duplicated commit/score wording**

Keep commit formatting in one place and keep score/history in tracking only.

### Task 4: Create the migration skeleton surface

**Files:**
- Create: `docs/architecture/README.md`
- Create: `docs/architecture/target-structure.md`
- Create: `docs/architecture/frozen-dirs.md`
- Create: `docs/architecture/module-boundaries.md`
- Create: `docs/architecture/migration-roadmap.md`
- Create: `src/features/README.md`
- Create: `src/shared/README.md`
- Create: `src/interfaces/README.md`
- Create: `src/core/README.md`

- [ ] **Step 1: Add the target skeleton docs**

Create documentation that explains the canonical landing zones for future code, without moving or rewriting existing implementation files.

- [ ] **Step 2: Add module README placeholders**

Add readme-style placeholders in the canonical landing folders to explain what belongs there and what must not be added there.

- [ ] **Step 3: Cross-link the skeleton docs**

Point the architecture and directory rules to the new skeleton docs so the migration path is obvious.

### Task 5: Verify consistency and keep the worktree clean

**Files:**
- Modify: any docs touched above only

- [ ] **Step 1: Run markdown consistency checks**

Check for broken links, repeated rules, and any text that still places dynamic data in entry docs.

- [ ] **Step 2: Inspect the final folder list**

Confirm the worktree only contains documentation and skeleton surface changes, not source behavior changes.

- [ ] **Step 3: Summarize the migration state**

Record what is now canonical, what is frozen, and what future work should use as its landing zone.
