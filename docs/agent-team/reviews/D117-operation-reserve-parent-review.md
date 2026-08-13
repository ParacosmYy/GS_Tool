# D117 / ARCH-91 — operation-reserve facade simplification parent review

## Scope

Reviewed the current changes in:

- `src/quillforge/presentation/main_window.py`
- `src/quillforge/presentation/session_save_coordinator.py`
- `src/quillforge/presentation/operation_tracker.py` (contract owner)

## Findings

- PASS: `OperationTracker` is initialized before SessionSaveCoordinator
  construction, so the bound `reserve` port is valid at composition time.
- PASS: all seven former `_next_operation_id()` calls now use the canonical
  tracker; no call site silently switched to a new counter.
- PASS: SessionSaveCoordinator's typed callable contract is unchanged and
  still receives a zero-argument allocator.
- PASS: `_begin_operation()` and `_complete_operation()` retain busy/status
  policy; no worker dispatch, stale guard, close, or persistence policy moved.

## Review result

`PASS` within the bounded source scope. Runtime callback interleaving,
startup, and close behavior remain unproven under the no-launch boundary.

## Public-source applicability

Python 3.12/PyQt6 desktop presentation orchestration only. Embedded C/C++,
MCU, vendor-manufacturer, and firmware requirements are not applicable.
Public CloudWeGo material remains an engineering reference; no private
ByteDance standard or certification/compliance claim is made.

## Simplification

`PASS`: the pure `_next_operation_id()` forwarding method was removed without
duplicating allocation state or weakening the lifecycle/policy boundary.
