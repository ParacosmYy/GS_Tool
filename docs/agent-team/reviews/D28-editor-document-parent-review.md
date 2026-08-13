# D28 parent review — editor document presentation adapter

| Field | Value |
|---|---|
| Slice | D28 / ARCH-19 editor document surface |
| Reviewer | Architect (parent integration review) |
| Independent reviewer | Chandrasekhar the 2nd / Luna max; bounded read-only review returned PASS |
| Scope | `editor_document_surface.py`, MainWindow editor creation/settings/Save As integration, ADR/spec/acceptance/handoff projections |
| Decision | accepted-with-limits; parent source review plus independent PASS |

## Outcome

`EditorDocumentSurface` now owns per-document `EditorWidget` creation,
presentation-safe settings/theme application, Save As language-hint refresh,
and modified/content/caret signal routing. MainWindow retains `_DocumentTab`,
document state, dirty/content-version policy, save/recovery/Replace All/session
coordination, operation locks, and notification policy.

## Parent review findings

### Correctness and lifecycle

- PASS by source reasoning: creation order remains language hint, settings,
  text, dirty state, then semantic signal connection, matching the previous
  MainWindow sequence.
- PASS by source reasoning: the surface forwards the existing modified,
  content, and caret callback shapes without mutating document state.
- PASS by source reasoning: Save As delegates only the language-hint refresh;
  the existing read-only reset, state replacement, title update, recovery
  cleanup, event publication, and follow-up policy remain in MainWindow.
- NOT RUNTIME-VERIFIED: QApplication startup, QScintilla signal delivery,
  editor interaction, focus, tab lifetime, visual rendering, DPI, and
  accessibility remain unrun under the no-launch policy.

### Architecture and policy

- PASS by source reasoning: the surface imports only domain editor/theme types
  and the existing EditorWidget adapter; it imports no application service,
  document record, persistence adapter, TaskRunner, or operation policy.
- PASS by source reasoning: MainWindow no longer directly constructs an
  EditorWidget, applies editor settings, refreshes language, or connects its
  editor signals; it supplies semantic callbacks and retains policy.
- PASS by source reasoning: no second document state model, event bus,
  singleton, or generic UI factory was introduced.

## Simplification assessment

The extraction removes repeated editor setup and three direct Qt signal
connections from MainWindow while preserving callback ownership and editor
adapter methods. The concrete surface is smaller than a generic factory and
no further safe behavior-preserving simplification is required for D28.

## Independent review

Chandrasekhar the 2nd / Luna max was assigned a bounded read-only review with
no write access, no Qt launch, and no test creation/run. The reviewer returned
PASS: creation order, callback signatures, Save As language refresh, settings
application, policy boundaries, and absence of duplicate wiring were all
confirmed from source. The reviewer also agreed that the signal-adapting
lambdas are necessary and recorded the unrun Qt lifecycle as the remaining
risk.

## Public-source applicability and embedded gate

This slice is Python/PyQt6 desktop code, not embedded C/C++ or firmware. The
embedded enterprise workflow and embedded code-review simplifier are **N/A**
for MCU/vendor constraints because no firmware target, SDK, RTOS, ISR/DMA,
driver, protocol, boot, Flash/NVM, power, or hardware was changed. Public
architecture references are engineering references only:
[CloudWeGo About](https://www.cloudwego.io/about/),
[CloudWeGo open-source announcement](https://www.cloudwego.io/blog/2021/09/13/cloudwego-open-source-announcement/),
and [Kitex framework extension](https://www.cloudwego.io/docs/kitex/tutorials/framework-exten/).
They are not private ByteDance standards, manufacturer requirements,
certification evidence, or a release-readiness claim.

## Authorized non-destructive validation

| Evidence | Result |
|---|---|
| `uv run python -m compileall -q src\quillforge` | PASS |
| `uv run ruff check src\quillforge` | PASS |
| `uv run ruff format --check src\quillforge` | PASS; 79 source files formatted |
| D28 source boundary probe | PASS; MainWindow delegates creation/settings/language and signal wiring is surface-owned |
| `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\package.ps1` | PASS; historical D28 root/dist SHA `CDCB3F4ECB9800B44C28D34E92B1F2077D36BF8F0E26F0054EAF6C41746842FE`, size `38,394,286` bytes; source `tree-sha256:7a410bd083789eecf06c7a7e59c1a1d6ca3bf41cd94b85d0ea6249a51ed2e004` |
| `scripts\verify_handoff.ps1` | PASS; D28 handoff/index/register/acceptance status and required sections are synchronized |
| `scripts\check.ps1` | PASS; D28 source, formatting, inventory, and static project checks passed |
| `scripts\verify_release_handoff.ps1` | EXPECTED NO-GO; exit 1 with the known three artifact/report consistency failures and 10 open release gates |

No unit tests, mocks, fixtures, harnesses, QApplication launch, screenshots,
deployment, or hardware operation were created or run.

## Open risks and disposition

- Independent review returned PASS for the bounded static/source scope; it is
  not runtime evidence.
- Runtime editor lifecycle, signal delivery, interaction, visual hierarchy,
  accessibility, and cross-machine rendering remain unverified.
- Existing D7/D8 and release gates remain open and are not narrowed by D28.

**Disposition:** `accepted-with-limits`; D28 editor-document composition is
source-level verified by the parent and independent Luna review, with all
runtime/external release gates open.
