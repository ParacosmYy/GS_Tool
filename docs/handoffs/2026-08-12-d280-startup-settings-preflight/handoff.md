# Handoff: 2026-08-12-d280-startup-settings-preflight

| Field | Value |
|---|---|
| ID | `2026-08-12-d280-startup-settings-preflight` |
| Delivery / slice | `D280 / ARCH-250 Startup settings preflight` |
| Status | `accepted-with-limits` |
| Owner | `architect` |
| Shared checkout | `D:\Workplace\Agent_Workplace\QuillForge` |

## User outcome

The existing no-window startup diagnostic now explains whether the user-local
settings file is present, decodable, and normalized to the current schema.
This makes configuration-related startup failures distinguishable without
writing settings or exposing the user's locale, theme, accent, font, or motion
values.

## Scope and boundaries

- Added a `settings_preflight` probe to `quillforge.app`.
- Reused `default_settings_path()`, `JsonSettingsStore`, `DEFAULT_SETTINGS`,
  and `normalize_settings` rather than adding a second settings policy.
- Kept the probe Qt-free and limited the report to path/presence/validity/schema
  metadata.
- No normal windowed startup path, settings schema, or persistence behavior
  changed.
- No EXE/Qt launch, native dialog, updater, installer, registry operation,
  unit-test asset, or worktree was used.

## Team roles and ownership

| Role | Owner / agent | Responsibility |
|---|---|---|
| Architect | parent; Kuhn consultation | Boundary, ownership, and integration review |
| Developer | parent | Startup diagnostic slice |
| QA | parent | Static, source-diagnostic, archive, package, and handoff verification |
| Independent reviewer | Archimedes consultation | Requested independent source review; no conclusion returned |

## Changed files and modules

- `src/quillforge/app.py` — no-window settings preflight and report projection.
- D280 ADR, review, plan, roadmap, register, acceptance, and handoff records.

## Decisions and constraints

- Keep startup behavior in `SettingsService`; the diagnostic reuses the
  infrastructure decoder and application normalizer.
- Report only metadata needed to classify a settings failure; never include
  preference values.
- Shared checkout writer: parent agent, limited to the listed source and
  delivery-record files.
- Runtime launch policy: EXE/Qt startup was not allowed; only non-destructive
  source, archive, and package evidence was authorized.

## Public-source applicability

Python 3.12 first-party `pathlib` and `json` documentation are applicable
references for path resolution, file predicates, and local JSON decoding. No
manufacturer requirement applies; no private ByteDance standard,
certification, MISRA, ISO 26262, ASPICE, or embedded claim is made. Embedded
workflow and embedded simplifier: `N/A`.

## Verification commands and results

| Evidence | Result |
|---|---|
| Settings preflight | `D280-SETTINGS-PREFLIGHT=PASS file_present=true valid=true schema=3` |
| Source startup diagnostic | `D280-SOURCE-DIAGNOSTIC=PASS exit=0 status=passed failed=0` |
| Compileall | `D280-COMPILEALL=PASS` |
| Ruff and formatting | `D280-RUFF=PASS`; `D280-FORMAT=PASS` |
| Project checks | `D280-CHECK=PASS` |
| PE header | `D280-PE-HEADER=PASS machine=AMD64 optional=PE32+ subsystem=WINDOWS_GUI` |
| Frozen archive | `D280-PE-ARCHIVE=PASS outer_entries=166 pyz_entries=261 required=7 embedded_startup_module=True` |
| Root/dist identity | `D280-MANIFEST-COPY=PASS bytes=38582863 sha=8E2D076F9AA685F4836525438163612B1A192C6517B5D92E0485E10947B7196F` |
| Source revision | `tree-sha256:0f777b3f81f4186af2d9b489f85beed2f40724a27298d20d6ba09083ffa19933` |
| Parent review | `PASS` |
| Independent review | `NO_CONCLUSION` after bounded wait and closure |
| Simplification | `PASS` |

## Unrun checks and reason

Native EXE/Qt startup, native settings rendering, clean-machine behavior,
signing, installer/update, registry, cross-machine repeatability, and
release-owner acceptance remain unrun under the active no-launch and
non-destructive policy. Release handoff verification remains expected `NO-GO`
with artifact-bound mechanical failures and remaining enterprise gates open.

## Known risks and limits

The preflight reports the current local file's readability and normalized
schema metadata, but it does not prove native Windows startup, Qt plugin
loading, or rendering. A valid settings report also cannot rule out unrelated
startup failures after the no-window probes.

## Acceptance and evidence IDs

- Acceptance: `S320`.
- Architecture slice: `ARCH-250`.
- Evidence: `D280-SETTINGS-PREFLIGHT=PASS`, `D280-CHECK=PASS`,
  `D280-MANIFEST-IDENTITY=PASS`,
  `D280-ARCHITECTURE-CONSULTATION=NO_CONCLUSION`,
  `D280-INDEPENDENT-REVIEW=NO_CONCLUSION`, and
  `D280-SIMPLIFICATION-ASSESSMENT=PASS`.

## Next owner and next action

- Owner: user / authorized QA.
- Action: if the native candidate still does not open, run the approved
  no-window diagnostic and attach its JSON report; do not infer native startup
  success from this source-only evidence.

## Artifact information

- Artifact paths: `dist/QuillForge.exe` and `QuillForge.exe`.
- Version: `0.1.0`.
- SHA-256 / size:
  `8E2D076F9AA685F4836525438163612B1A192C6517B5D92E0485E10947B7196F` /
  `38,582,863` bytes.
- Source revision:
  `tree-sha256:0f777b3f81f4186af2d9b489f85beed2f40724a27298d20d6ba09083ffa19933`.

## Disposition

`accepted-with-limits`: settings-related preflight evidence is now available;
native startup and enterprise release gates remain open.
