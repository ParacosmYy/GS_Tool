# Constraint State Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild the repository constraints around engineering state, user usability, and device verification so future parallel development remains maintainable and evidence-based.

**Architecture:** Keep `CLAUDE.md` as the authoritative entry, then make workflow, architecture, directory, Git, and Serial Station documents reference the same state model. No production code changes are part of this plan.

**Tech Stack:** Markdown constraints, C++/Qt project governance, Git commit discipline.

---

### Task 1: Define The Three-Axis Delivery State

**Files:**
- Modify: `CLAUDE.md`

- [ ] **Step 1: Add state definitions**

Add a `交付状态口径` section defining:

```text
工程状态: E0未开始 / E1设计中 / E2骨架 / E3已接入构建 / E4自动化测试通过 / E5可维护收口
用户状态: U0不可见 / U1可见不可用 / U2局部可用 / U3主流程可用 / U4体验完整
设备验证: D0未验证 / D1纯单测 / D2替身/模拟验证 / D3虚拟设备验证 / D4真实设备验证
```

- [ ] **Step 2: Add completion rules**

Define engineering completion as `E4+`, user completion as `U3+`, device completion as `D3+`, and real hardware completion as `D4`.

- [ ] **Step 3: Add current Serial Station reclassification**

Record UART and protocol work as engineering-progress evidence, while keeping user and device state separate until entry and hardware/virtual validation are proven.

### Task 2: Enforce The State Model In Workflow

**Files:**
- Modify: `docs/constraints/02-workflow.md`

- [ ] **Step 1: Add state targets to Specs**

Require PRD/Specs to state the target `E/U/D` levels before implementation.

- [ ] **Step 2: Add state results to closure**

Require every closure report to list the actual `E/U/D` result and verification evidence.

- [ ] **Step 3: Strengthen BATCH rules**

Require parallel plans to include editable files, forbidden files, shared locks, interface contracts, verification commands, state targets, and merge order.

### Task 3: Enforce Decoupling In Architecture

**Files:**
- Modify: `docs/constraints/03-architecture.md`

- [ ] **Step 1: Add architecture admission checks**

Require every new capability to declare module ownership, public interface, dependencies, owner, validation method, and state evidence.

- [ ] **Step 2: Add interface admission rules**

Specify where stable interfaces live, which data types may cross module boundaries, who owns lifetimes, and how errors surface.

- [ ] **Step 3: Add parallel architecture locks**

Lock `CMakeLists.txt`, `MainWindow`, `PanelManager`, `shared`, `interfaces`, QSS themes, startup scripts, README, and constraints to a single owner during parallel work.

### Task 4: Align Git And Directory Rules

**Files:**
- Modify: `docs/constraints/06-git-commit.md`
- Modify: `docs/constraints/07-directory-structure.md`

- [ ] **Step 1: Require per-iteration commits**

State that a finished iteration requires verification evidence, state result, and commit. Uncommitted work remains workspace-only.

- [ ] **Step 2: Prevent accidental mixed commits**

Require path-scoped commits when user changes already exist, and stop if changes cannot be isolated.

- [ ] **Step 3: Add directory admission rules**

Require new directories to declare canonical ownership, dependency layer, CMake/test/doc impact, and why no existing directory can own the work.

### Task 5: Tighten Serial Station UART Acceptance

**Files:**
- Modify: `docs/serial_station_architecture.md`
- Modify: `docs/constraints/01-project-overview.md`

- [ ] **Step 1: Add Serial Station state gates**

Define minimum engineering, user, and device evidence for UART config, UART send, UART receive, protocols, logs, export, and replay.

- [ ] **Step 2: Add validation levels**

Define `D1` as pure QTest, `D2` as fake serial or in-memory substitute, `D3` as virtual serial, and `D4` as real hardware.

- [ ] **Step 3: Add migration priority**

Prioritize entry reachability, UART configuration, connect/disconnect, send, receive, and log feedback before adding more advanced capabilities.

### Task 6: Verify And Commit

**Files:**
- Inspect: `CLAUDE.md`
- Inspect: `docs/constraints/*.md`
- Inspect: `docs/serial_station_architecture.md`

- [ ] **Step 1: Search for state language**

Run:

```powershell
rg -n "三轴|工程状态|用户状态|设备验证|D4|并行|每轮迭代|状态提升" CLAUDE.md docs\constraints docs\serial_station_architecture.md
```

Expected: matches in the entry, workflow, architecture, Git, directory, and Serial Station documents.

- [ ] **Step 2: Check changed files**

Run:

```powershell
git diff -- CLAUDE.md docs\constraints\01-project-overview.md docs\constraints\02-workflow.md docs\constraints\03-architecture.md docs\constraints\06-git-commit.md docs\constraints\07-directory-structure.md docs\serial_station_architecture.md docs\superpowers\plans\2026-06-13-constraint-state-framework.md
```

Expected: only constraint and plan documentation changes appear.

- [ ] **Step 3: Commit only this plan's files**

Run a path-scoped commit so unrelated existing workspace changes are not included.

```powershell
git add CLAUDE.md docs\constraints\01-project-overview.md docs\constraints\02-workflow.md docs\constraints\03-architecture.md docs\constraints\06-git-commit.md docs\constraints\07-directory-structure.md docs\serial_station_architecture.md docs\superpowers\plans\2026-06-13-constraint-state-framework.md
git commit -- CLAUDE.md docs\constraints\01-project-overview.md docs\constraints\02-workflow.md docs\constraints\03-architecture.md docs\constraints\06-git-commit.md docs\constraints\07-directory-structure.md docs\serial_station_architecture.md docs\superpowers\plans\2026-06-13-constraint-state-framework.md
```

Expected: commit contains only documentation for the constraint state framework.
