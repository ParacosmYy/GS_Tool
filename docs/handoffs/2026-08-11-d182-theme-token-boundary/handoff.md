# Handoff: 2026-08-11-d182-theme-token-boundary

| Field | Value |
|---|---|
| ID | `2026-08-11-d182-theme-token-boundary` |
| Delivery / slice | `D182 / ARCH-169 / UI-94 Theme token boundary` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Checkout | Current local checkout only |
| Created | `2026-08-11T23:00:00+08:00` |

## User outcome

The visual system is easier to extend without coupling pure palette decisions
to Qt rendering. Existing theme, accent, gold readability, editor token, and
QSS behavior remain available through the same presentation entry points.

## Scope and boundaries

### In scope

- Move immutable theme/editor tokens and contrast resolution to `theme_tokens`.
- Keep Qt palette, QSS, icon, and editor-adapter projection in `theme`.
- Preserve existing imports and fallback behavior.

### Out of scope

- New themes, settings fields, widget behavior, or styling systems.
- Native Qt visual/accessibility/runtime validation.
- Signing, installer, updater, clean-machine, or release-owner gates.

## Team roles and ownership

| Role | Owner / agent | Responsibility in this handoff |
|---|---|---|
| Architect | `parent` | Integration, final review, verification, and handoff decision |
| Project Manager | `parent` | Plan, dependencies, risks, and status |
| Product | `user outcome` | Visual extensibility and readability goal |
| Developer 1 | `parent` | Token boundary implementation |
| Developer 2 | `parent` | Presentation compatibility integration |
| QA | `parent` | Read-only static and contract verification |

## Changed files and modules

- `src/quillforge/presentation/theme_tokens.py` — pure token registry and contrast resolver.
- `src/quillforge/presentation/theme.py` — Qt projection and compatibility façade.
- `docs/adr/0231-theme-token-boundary.md` — architecture decision.
- `docs/agent-team/reviews/D182-theme-token-boundary-parent-review.md` — parent review.
- `docs/agent-team/reviews/D182-theme-token-boundary-independent-review.md` — no-conclusion record.

## Decisions and constraints

- `theme_tokens` has no Qt, application, infrastructure, plugin, or widget dependency.
- `theme.py` remains the only generated-QSS and Qt palette owner.
- Shared checkout writer: parent only; no worktree was created or used.
- Runtime launch policy: not allowed under the project policy; no Qt window or EXE was started.

## Verification commands and results

| Command / evidence | Result | Notes |
|---|---|---|
| `uv run python -m compileall -q src/quillforge/presentation/theme.py src/quillforge/presentation/theme_tokens.py` | `PASS` | Targeted source compilation |
| `uv run ruff check src/quillforge/presentation/theme.py src/quillforge/presentation/theme_tokens.py` | `PASS` | Ruff checks passed |
| `uv run ruff format --check src/quillforge/presentation/theme.py src/quillforge/presentation/theme_tokens.py` | `PASS` | Both files formatted |
| `D182-TOKEN-BOUNDARY-PROBE` | `PASS` | No Qt imports in `theme_tokens` |
| `D182-TOKEN-COMPATIBILITY-PROBE` | `PASS` | 12 supported theme/accent combinations |
| `D182-CONTRAST-ENDPOINT-PROBE` | `PASS` | Filled accent and gold endpoints remain readable |
| `D182-QSS-WIRING-PROBE` | `PASS` | Existing semantic selectors remain generated |
| `scripts/package.ps1` | `PASS` | Portable one-file package rebuilt without launch |
| `D182-PACKAGE-IDENTITY-PROBE` | `PASS` | SHA `0CA47B28FE886B5B88E7BA557B52B9A2DB3E8F4A483721DDE5E4DA3269ABA84E`, 38,555,019 bytes |
| `D182-MANIFEST-TRACEABILITY-PROBE` | `PASS` | Manifest and root copy match the package |
| `scripts/check.ps1` | `PASS` | Full project static checks |
| `scripts/verify_handoff.ps1` | `PASS` | Handoff contract synchronized |
| `scripts/verify_release_handoff.ps1` | `EXPECTED NO-GO` | Three mechanical report mismatches and ten open release gates remain |
| Architect child review | `NO_CONCLUSION` | Bounded child window timed out |
| Independent child review | `NO_CONCLUSION` | Bounded child window timed out |

## Unrun checks and reason

- Native Qt painting, DPI, font fallback, screen-reader tree, and screenshot review — runtime launch is prohibited.
- Unit tests, mocks, fixtures, and test-only assets — project policy forbids creating or running them by default.
- Clean-machine, cross-machine, signing, installer, updater, legal, and release-owner checks — external authorization/evidence remains open.

## Known risks and limits

- The compatibility façade preserves source-level imports but not native Qt runtime proof.
- The release dossier remains no-go with its existing mechanical failures and open gates.
- The checkout has no Git baseline; no Git diff or worktree claim is made.

## Acceptance and evidence IDs

- Acceptance: `S235`, `D182-AC01`
- Evidence: `D182-TOKEN-BOUNDARY-PROBE=PASS`, `D182-TOKEN-COMPATIBILITY-PROBE=PASS`, `D182-CONTRAST-ENDPOINT-PROBE=PASS`, `D182-QSS-WIRING-PROBE=PASS`, `D182-SIMPLIFICATION-ASSESSMENT=PASS`

## Next owner and next action

- Owner: `release owner`
- Action: obtain authorized runtime/release evidence before changing the explicit no-go dossier.

## Artifact information

- Artifact path: `dist/QuillForge.exe` and root `QuillForge.exe`
- Version: `0.1.0`
- SHA-256 / size: `0CA47B28FE886B5B88E7BA557B52B9A2DB3E8F4A483721DDE5E4DA3269ABA84E` / `38,555,019` bytes
- Source revision: `tree-sha256:48d4c761d0c0ae0f7262ea927d6eb5cb0f892fec985d5345238c3b0e1c98abf9`
- Packaging note: portable one-file candidate rebuilt successfully; no launch.

## Disposition

Accepted with limits. The source boundary is complete and statically verified;
runtime and external release gates remain explicit follow-up work.
