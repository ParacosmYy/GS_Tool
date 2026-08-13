# D280 parent review — startup settings preflight

## Scope

Reviewed `src/quillforge/app.py` after the D280 change, including the
`settings_preflight` registration and `_startup_settings_preflight()` helper.

## Findings

- PASS — the probe is reached only from `--diagnose-startup`; normal windowed
  startup is unchanged.
- PASS — the helper uses `default_settings_path()` and `JsonSettingsStore`,
  then reuses `DEFAULT_SETTINGS`/`normalize_settings`; no duplicated field
  allowlist or schema policy was introduced.
- PASS — the report contains no user preference values and the helper has no
  write operation. Existing outer `probe()` exception isolation remains in
  force.
- PASS — source diagnostic output reports `file_present=true`,
  `stored_snapshot_valid=true`, and `normalized_schema_version=3` for the
  current local configuration.

## Simplification assessment

PASS. No additional abstraction or public API is justified for one diagnostic
probe. Keeping the read/normalize projection beside the other startup probes
preserves locality and makes the no-window boundary obvious.

## Applicability and limits

Python standard-library behavior is the applicable public-source reference;
there is no manufacturer requirement and no embedded C/C++ scope. Independent
architecture and review windows returned `NO_CONCLUSION` after bounded waits;
this parent review does not convert those windows into a pass. Native EXE/Qt
startup was not run.
