# ADR 0013: Read-only extension catalog boundary

- **Status:** accepted with limits for D6.1
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

The editor can inspect a bounded, explicit extension directory and report which
JSON descriptors are valid, incompatible, malformed, or duplicated. This gives
an enterprise operator a visible extension inventory without turning inventory
inspection into code execution.

## Decision

1. `JsonPluginCatalogStore` reads only the top level of an explicit user-local
   QuillForge extension directory. It examines at most 4096 directory entries,
   reads at most 256 `*.json` files, and rejects descriptors larger than 64 KiB
   before decoding them.
2. `PluginCatalogService` owns descriptor validation, API compatibility, closed
   permission validation, and deterministic duplicate-ID classification.
3. A descriptor contains identity, version, API version, an import-shaped
   entrypoint string, and requested permissions. The entrypoint is metadata
   only; the catalog never imports it, resolves it, or starts a process.
4. Every externally discovered entry is `untrusted` and `loadable=false`. Only
   the existing composition-root registration path can provide executable
   plugin instances. Trust approval, signature verification, sandboxing, and
   dynamic loading require a later threat-modelled delivery.
5. Catalog scanning is user-triggered through the `Extension Catalog` command
   and runs through `TaskRunner`. Startup does not scan the directory, and the
   UI receives only an immutable summary snapshot.
6. Missing directories are an empty inventory. Malformed files, duplicate IDs,
   incompatible API versions, symlinks, oversize files, and directory errors
   are surfaced as bounded diagnostics without stopping the editor.

## Contract shape

The manifest schema is versioned independently from the executable plugin API:

```json
{
  "schema_version": 1,
  "plugin_id": "vendor.example",
  "name": "Example Extension",
  "version": "1.0.0",
  "api_version": "1.0",
  "entrypoint": "vendor.example:Plugin",
  "permissions": ["commands"]
}
```

The schema is intentionally strict. Unknown keys, duplicate permissions,
invalid IDs, path-like entrypoints, and unsupported permissions are invalid.

## Consequences

- The executable plugin path remains high-cohesion and low-coupling: filesystem
  access belongs to infrastructure, policy belongs to the application, and
  plugin metadata belongs to the public plugin contract.
- The current feature improves inventory and governance without promising a
  Python security sandbox. It does not make third-party code safe to execute.
- The command is asynchronous and bounded, but the current summary is not a
  persistent enterprise inventory or a signed software bill of materials.

## Out of scope

- importing or executing external Python modules;
- downloading or installing extensions;
- signature/chain-of-trust verification or enterprise allowlist persistence;
- process isolation, sandboxing, remote catalogs, telemetry, or automatic
  updates;
- recursive filesystem discovery.

## Verification evidence

- `scripts/check.ps1` for boundaries, formatting, lint, lock, and compilation;
- deterministic source smoke for valid, malformed, incompatible, duplicate,
  oversize, and missing-directory catalog inputs;
- Qt offscreen command projection smoke;
- packaged startup smoke after the composition-root import is changed.
