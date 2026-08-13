# QuillForge extension catalog

QuillForge has two deliberately separate extension paths:

- Built-in or explicitly created plugin instances are registered by the
  composition root and can run through the governed `PluginManager` API.
- External catalog entries are metadata only. They are discovered on demand,
  validated, shown in the Extension Catalog command, and remain untrusted and
  unloaded.

The catalog directory is the user-local path below the platform's application
data directory:

```text
Windows: %LOCALAPPDATA%\QuillForge\plugins
Fallback: ~/.local/share/QuillForge/plugins
```

Only top-level `*.json` files are inspected. Discovery examines at most 4096
directory entries, reads at most 256 files, and accepts at most 64 KiB per
manifest. A missing directory is an empty catalog. Recursive folders, symlink
manifests, network locations, and non-JSON files are outside the current
contract.

## Manifest schema

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

The schema is strict: all seven keys are required, unknown keys are rejected,
plugin IDs use lowercase dotted/dashed identifiers, entrypoints are validated
as metadata (`module:attribute`) but never imported, and permissions must be a
unique subset of the closed host capability set.

The same public `PluginManifest` policy is applied when a built-in or otherwise
explicitly created in-process plugin reaches `PluginManager.register()`. The
catalog adds JSON/file/entrypoint checks; it does not create a runtime plugin
instance.

Valid descriptors can receive an explicit local approval or revocation from the
catalog window. Approval is bound to the canonical descriptor SHA-256 and is
shown as `approved`, `stale`, or `not-approved`; changing the descriptor makes
an earlier decision stale. This ledger is governance metadata only: entries
remain untrusted and unloaded, and approval does not mean signature
verification or executable trust.

Use **Tools → Extension Catalog** to request a scan. The scan runs through the
background task boundary and reports valid, incompatible, invalid, and
duplicate entries. It does not install, trust, import, execute, or persist an
external extension.

Use **Tools → Plugin Status** to inspect explicitly registered in-process
plugins. The status projection reports trust, enablement, activity, requested
permissions, and lifecycle errors. Disable first deactivates the plugin and
cleans its owned commands/subscriptions; enable is explicit and reactivates
only an already-trusted compatible instance. Enablement never grants trust or
changes the registration trust bit. This is a session-local control surface;
it does not make catalog descriptors loadable.

The runtime enablement policy is stored separately at:

```text
Windows: %LOCALAPPDATA%\QuillForge\plugin-enablement.json
Fallback: ~/.local/share/QuillForge/plugin-enablement.json
```

An absent policy keeps explicitly trusted built-ins enabled by default. A
corrupt or oversized policy disables activation and is not silently replaced by
an enable/disable click; the error remains visible in Plugin Status. This file
is a local operator preference, not a signature, publisher trust record, or
external plugin loader permission.

Use **Tools → Plugin Host Diagnostics** to probe the separate Qt-free host
process. The probe verifies a bounded versioned JSONL hello/probe exchange and
reports the child PID plus timeout/crash/protocol failures. The protocol
accepts exactly the probe capability and rejects unknown capabilities or any
true execution flag. The current host always reports `execution_enabled=false`;
it never imports or executes a
catalog entry. On Windows, the child is attached to an unnamed Job Object for
the exchange. The default policy reports kill-on-close, allows at most two
active processes for a launcher/re-exec wrapper plus the host, and limits
per-process committed memory to 256 MiB. If attachment fails, the probe returns
`containment-error` without starting the protocol; other platforms report an
explicit unsupported containment state. A legacy attach-only adapter is
reported as `attached-after-start` and is not a creation-time execution
boundary. This is lifecycle/resource control,
not a complete sandbox, restricted token, filesystem/network boundary,
signature check, or external execution gate.

Every catalog snapshot is also evaluated by the application-owned
`PluginExecutionGate`. It records `execution=denied`, a deterministic primary
reason, and all failed requirements (approval, signature, code identity,
enablement, permissions, containment, and executor availability). The current
global policy is `external_execution_enabled=false`, so even an approved
descriptor remains untrusted, unloaded, and non-executable.

Signature verification, an enterprise trust store, sandbox/process isolation,
installation, update policy, and dynamic loading require a separate security
and release decision.
