# D9 / UI-14 parent review: command-palette hierarchy and surface rhythm

| Field | Value |
|---|---|
| Hook | `after-design` + `after-source-change` |
| Scope | Centralized Qt surface rhythm and command-palette presentation |
| Decision | `accepted-with-limits` |
| Owner | Architect |
| Checkout | Current local checkout only |

## User outcome

The shell no longer treats every ordinary control as a glossy capsule. Normal
buttons and work surfaces use a calmer flat hierarchy, while the primary action
keeps the strongest accent. The command palette now reads as a focused work
surface with a dedicated query, result list, selected row, and hint hierarchy.

## Architecture decision

- Keep UI-14 in `presentation`: only `theme.py` and the command-palette widget
  were changed; no application, domain, infrastructure, plugin, editor, or
  signal contract moved.
- Reuse the existing `ThemeColors` tokens and QSS contract. Ordinary controls
  lose their global vertical gradient and excessive radius; the primary action
  retains the two-endpoint accent gradient as the intentional visual anchor.
- Use semantic object names (`commandPalette`, `commandPaletteQuery`,
  `commandPaletteList`, `commandPaletteHint`) to localize styling and avoid
  widening global selectors.
- Preserve command filtering, `UserRole` selection, Enter acceptance, item
  activation, locale projection, and the existing command registry boundary.
- Keep the UI skill's state matrix and contrast invariant: any filled amber
  accent still uses the centralized readable `on_accent` foreground.

## Fixed-role input and ownership

| Role | Agent | Contribution | Disposition |
|---|---|---|---|
| Architect | parent | Chose the bounded scope, edited the presentation files, and owns final verification | accepted |
| Project Manager | Zeno / Luna | Recommended a command-palette-focused modernization slice and evidence boundary | incorporated |
| Product | Harvey / Luna | Returned a response tied to another repository; it was not used as source authority | not used |
| Developer 1 | Pasteur / Luna | Supplied a provisional presentation-only boundary and global-QSS coupling risks without completing source inspection | considered; no PASS claimed |
| Developer 2 | Newton / Luna | Confirmed localized selectors, reduced radius/gradient direction, QSS specificity risk, and amber invariant | incorporated |
| QA | Goodall / Luna | Supplied static, contrast, boundary, packaging, handoff, and no-launch checks | incorporated |

The independent code reviewer Laplace / Luna returned `NO CONCLUSION` within
the bounded window because the checkout has no Git metadata and it could not
establish a reliable historical diff. No child PASS is claimed; the parent
review is based on direct source inspection and the explicit probes below.

## Simplification assessment

This slice reduces visual complexity rather than adding another decoration
system: ordinary `QPushButton` surfaces are flat, rounding and padding are
tighter, and only the primary action retains a gradient. The command palette
adds semantic names and one alternating-row presentation flag; it does not add
custom painting, animation, duplicate tokens, or signal wrappers. No further
behavior-preserving simplification was identified.

## Public source applicability

This change touches Python/PyQt6 presentation code and project-local skill
documentation only. No embedded C/C++, MCU, BSP/HAL/CMSIS, RTOS, ISR/DMA,
driver, bootloader, Flash/NVM, power, or motor-control code was modified. The
embedded vendor-source workflow and manufacturer-requirement applicability are
`N/A`; no MISRA, ISO 26262, automotive, or certification claim is made.

The project-local skills are engineering guidance authored for this checkout,
not ByteDance internal policy. Any future enterprise-architecture claim must
be grounded in a public, first-party source and labeled as project decision or
engineering judgment where appropriate.

## Independent review result

Laplace / Luna: `NO CONCLUSION`, not a PASS. The returned risk list was still
useful and is retained: QSS specificity, runtime native style, actual diff
baseline, and runtime contrast remain open. The parent directly confirmed the
new command-palette selectors appear after the generic rules, normal-control
gradients are removed while the two primary gradients remain, and all
accent-filled warning/primary paths keep `on_accent`.

## Authorized verification

- `uv run python -m compileall -q src\\quillforge\\presentation` — PASS.
- `uv run ruff check src\\quillforge\\presentation\\theme.py src\\quillforge\\presentation\\command_palette.py` — PASS.
- `uv run ruff format --check src\\quillforge\\presentation\\theme.py src\\quillforge\\presentation\\command_palette.py` — PASS.
- UI-14 QSS boundary probe — PASS: localized selectors exist, ordinary buttons
  are flat, primary gradients remain intentional, and warning uses `on_accent`.
- UI-14 command-palette behavior-boundary probe — PASS: semantic names are
  present and existing text/filter/Enter/item-activation connections remain.
- UI-14 contrast probe — PASS: 36 accent/pink/gold endpoints and 12 warning
  combinations pass the static 4.5:1 target.
- Skill validation — PASS for both project-local skills through
  `quick_validate.py` with temporary `pyyaml` execution support.
- `scripts/verify_handoff.ps1` and `scripts/check.ps1` — to be rerun after
  final UI-14 ledger synchronization.
- `scripts/package.ps1` through PowerShell Core — PASS for the historical UI-14
  snapshot; root/dist SHA-256 is
  `7A40DE4214CF57DBA3D720F65E2D082EA1BA90B818EB57402944CA82605437D2`,
  `38367650` bytes, with source snapshot
  `tree-sha256:e24f3d27764a7c14ed2c78aba111ecd8e7f2239c29449757e22f982dae3e3779`.

## Unrun checks and reason

- QApplication/Qt startup, screenshots, command-palette interaction, theme
  switching, high-DPI layout, native style, and runtime visual acceptance —
  prohibited by the active project no-launch policy.
- Unit tests, mocks, fixtures, harnesses, and test-only assets — not created,
  modified, or run under project constraints.
- Clean-machine, screen-reader, cross-machine, signing, installer, update,
  deployment, and hardware checks — outside this local non-destructive slice
  and not authorized.

## Known risks and limits

- QSS specificity and native Qt style rendering can still change the perceived
  result; static selector order is not runtime visual proof.
- Tighter spacing may expose font/DPI truncation or native metric differences;
  those need authorized runtime review.
- Release remains `NO-GO`; UI-14 does not close D7/D8 or external release gates.

## Acceptance and evidence IDs

- Acceptance: `D9-AC12`, `S39`.
- Evidence: `src/quillforge/presentation/theme.py`,
  `src/quillforge/presentation/command_palette.py`,
  `skills/quillforge-ui-visual-quality/SKILL.md`,
  `skills/quillforge-enterprise-architecture/SKILL.md`, this review,
  `docs/handoffs/2026-08-10-d9-ui-14-command-palette-surface-rhythm/handoff.md`,
  `tasks/plan.md`, `tasks/todo.md`, and the final package manifest.

## Next owner and next action

- Owner: Architect.
- Action: finish UI-14 package/ledger closure, then complete the public-source
  enterprise-architecture baseline and first application-boundary migration
  slice as a separate ADR-backed handoff.

## Artifact information

- Artifact path: `QuillForge.exe` and `dist/QuillForge.exe`.
- Version: `0.1.0` / package manifest version.
- Historical UI-14 SHA-256 / size: `7A40DE4214CF57DBA3D720F65E2D082EA1BA90B818EB57402944CA82605437D2` /
  `38367650` bytes for both root and dist copies; source
  `tree-sha256:e24f3d27764a7c14ed2c78aba111ecd8e7f2239c29449757e22f982dae3e3779`.
- Packaging note: portable package must be rebuilt through PowerShell Core;
  package evidence is not release approval.

## Disposition

`accepted-with-limits`: the bounded visual cleanup and command-palette hierarchy
are implemented and statically reviewed by the Architect with role inputs and
an honest independent no-conclusion record. Runtime visual and release gates
remain open.
