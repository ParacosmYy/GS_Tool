# PRD-072 - Simplify Scan Specs

## 1. Goal

Make the LOOP Simplify layer executable through a read-only scan that identifies cleanup entry points before any refactor starts.

The scan focuses on:

- Oversized `.cpp` and `.h` files.
- Frozen historical directories that still exist and must not receive new work.
- Parallel build directories.
- Known duplicated directory families such as `font/fonts`, `icon/icons`, `widgets/widgets2`, `loader/loader2`.
- Basic risk keywords such as `TODO`, `FIXME`, and direct `setStyleSheet`.

## 2. Non-goals

- Do not modify C++ source.
- Do not move files.
- Do not delete frozen directories.
- Do not create a refactor plan larger than the evidence supports.
- Do not replace Doctor or Debug.

## 3. Required Constraints

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/07-directory-structure.md`
- `docs/superpowers/LOOP_PROTOCOL.md`

## 4. Change Scope

| Scope | Path | Allowed action |
|------|------|----------------|
| Simplify script | `tools/simplify-scan.ps1` | create / modify |
| Simplify reports | `docs/reviews/simplify/` | create |
| Specs | `docs/superpowers/specs/PRD_072_Simplify_Scan_Specs.md` | create / modify |
| LOOP docs | `docs/superpowers/LOOP_PROTOCOL.md` | modify |
| Directory docs | `docs/constraints/07-directory-structure.md` | modify |
| PRD-072 | `docs/prd/PRD_072_Agent_Iteration_Runtime.md` | modify |
| Forbidden | `src/`, `CMakeLists.txt`, `EmbedDebug.bat` | no edits in this task |

## 5. Acceptance

- [ ] `tools/simplify-scan.ps1` exists.
- [ ] Default mode writes no files and prints Markdown to stdout.
- [ ] `-OutFile` only allows paths inside `docs/reviews/simplify/`.
- [ ] The scan reports oversized files, frozen directories, duplicate directory families, and risk keyword counts.
- [ ] LOOP docs include the Simplify local entry.
- [ ] Qt build and `EmbedDebug.bat` launch still pass.

## 6. Failure Conditions

- The scan modifies source files.
- The scan writes outside `docs/reviews/simplify/`.
- The scan creates or references a second build directory as a valid target.
- The scan claims a refactor is safe without a follow-up PRD or task.

## 7. GO Config

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\simplify-scan.ps1"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1"
  ],
  "fix": []
}
```

## 8. BATCH Decision

- Needs BATCH: no.
- Reason: this task adds one read-only scan tool and documentation.
- Parallelism: 1.

## 9. LOOP Route

- Doctor: verify build and launch chain.
- Debug: use `debug-trace.ps1` if the scan itself fails or exposes a reproducible bug.
- Simplify: use this scan to pick the next small, behavior-preserving cleanup.
