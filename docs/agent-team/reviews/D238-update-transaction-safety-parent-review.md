# D238 parent review — update transaction safety

## Decision

`PASS` with explicit operational limits.

## Findings

- Ordinary update now tracks the previous target move and state commit; a
  failed state write restores the previous artifact only when the target hash
  is known and the state-owned backup is available.
- Explicit rollback tracks both file moves and can restore the original target
  and original backup layout after a failed state commit.
- A target changed by another actor, an occupied backup path, or an uncertain
  hash prevents forced replacement and emits a warning.
- Existing containment, hash validation, `ShouldProcess`, and backup naming
  boundaries remain the owning contracts; no second state store is introduced.

## Evidence

- `D238-UPDATE-TRANSACTION-STATIC-CONTRACT=PASS`
- `D238-PS51-AST=PASS`
- `D238-PS7-AST=PASS`
- `D238-COMPILEALL=PASS`
- `D238-RUFF=PASS`
- `D238-PACKAGE-IDENTITY-PROBE=PASS`
- `D238-PACKAGE-ARCHIVE=PASS`
- `D238-PYINSTALLER-WARNING-SCOPE=PASS`

## Simplification assessment

`D238-SIMPLIFICATION-ASSESSMENT=PASS`: recovery remains local to the existing
update transaction, uses two boolean move/commit markers and hash checks, and
does not introduce a generic transaction framework or new persistence layer.

## Independent review

The post-fix Luna/max independent window returned `NO_CONCLUSION` after two
bounded waits and was closed. No independent PASS is claimed.

## Limits

No updater or rollback script, file move under an install root, EXE/Qt,
registry, unit-test asset, mock, fixture, harness, worktree, or test-only asset
was executed or created.
