# ADR 0014: Unified plugin manifest policy

- **Status:** accepted with limits for D6.2; external trust and loading remain open
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Problem

The in-process `PluginManager` and the read-only external catalog both inspect
plugin identity, API compatibility, and permissions, but they previously used
different validation strength. A descriptor could pass one boundary while
violating the other, which is unsafe for a long-lived extension platform.

## Decision

1. The public plugin contract owns one structural validator for
   `PluginManifest` identity, display fields, API version shape, and the closed
   permission set. It rejects empty/oversize fields, invalid lowercase dotted
   IDs, duplicate permissions, unknown permissions, and unsupported API
   versions according to the selected mode.
2. `PluginManager.register()` calls that validator before mutating its plugin
   registry. Duplicate IDs remain a registry concern because they depend on
   existing host state.
3. The catalog parser constructs the same public manifest value and reuses the
   validator in structural-only mode. It then classifies an otherwise valid
   manifest with a different API version as `incompatible`, rather than
   executing or registering it.
4. Entrypoint syntax, JSON keys, file limits, source paths, trust state, and
   catalog status remain catalog concerns. Runtime plugin instances never
   receive filesystem paths or catalog trust metadata.
5. This is a policy consistency increment. It does not add dynamic module
   loading, persisted trust, signatures, sandboxing, installation, or update
   behavior.

## Consequences

- Built-in and externally described extensions share the same identity and
  capability invariants.
- The extension API remains independent of Qt and infrastructure adapters.
- Error messages become a stable diagnostic surface for future settings and
  enterprise support UI, while callers still receive ordinary `ValueError`
  failures at the registration boundary.
- Existing valid built-in plugins remain behaviorally unchanged.
- The shared validator now applies the same 256-character upper bound to
  `plugin_id` that the catalog parser already applied to its JSON fields.

## Verification

- `scripts/check.ps1` boundary, format, lint, lock, and compile checks;
- source smoke for valid registration and rejected invalid/API/permission
  manifests;
- catalog smoke proving it uses the shared policy for the same inputs;
- existing Qt plugin lifecycle and catalog projection smoke;
- package/startup evidence when the composition import graph changes.
