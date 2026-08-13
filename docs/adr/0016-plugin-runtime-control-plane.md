# ADR 0016: Plugin runtime control plane

- **Status:** accepted with limits for D6.4; external runtime gates remain open
- **Date:** 2026-08-09
- **Decision owner:** Architect

## User outcome

An operator can inspect the lifecycle state of explicitly registered
in-process plugins and explicitly enable or disable one without reaching into
Qt widgets or the plugin implementation. The control surface is diagnostic and
lifecycle-oriented; external catalog descriptors remain metadata-only.

## Decision

1. The application layer owns a small `PluginRuntime` protocol and immutable
   `PluginRuntimeStatus` projection. It exposes only identity, version,
   trust/enabled/active state, permissions, and the last lifecycle error.
2. `PluginManager` remains the owner of registration, lifecycle callbacks,
   command/event cleanup, and failure isolation. Its internal manifest/status
   objects are mapped to the application projection; presentation does not
   import manager internals.
3. `set_enabled(plugin_id, enabled)` is an explicit UI-thread lifecycle
   operation for already registered in-process plugins. Disabling deactivates
   first and removes owned commands/subscriptions; enabling clears the
   enablement error and activates only a trusted compatible instance.
4. `MainWindow` exposes a `Plugin Status` command and a read-only status dialog
   with enable/disable actions. Lifecycle callbacks stay on the owning UI
   thread because the current `PluginContext` publishes application events and
   invokes host-owned command/notifier seams.
5. This increment does not persist enablement, discover external modules, or
   change catalog approval/trust. Persistent policy and a process-isolated
   external host are separate future deliveries.

## Consequences

- The runtime state contract is testable and replaceable without coupling the
  presentation layer to `PluginManager` or Qt widgets.
- The operator can recover from an explicit disable without restarting the
  editor, while plugin-owned host resources remain manager-owned.
- Lifecycle work is synchronous on the UI thread in this increment; plugins
  must keep activation/deactivation bounded. A future process host must not
  reuse this synchronous in-process seam for untrusted code.

## Out of scope

- external catalog loading or dynamic import;
- persisted enablement, enterprise policy distribution, signatures, or
  publisher identity;
- process isolation, sandboxing, installation, updating, or remote control.

## Verification

- architecture boundary, format, lint, lock, and compile checks;
- source smoke for status projection, disable cleanup, re-enable activation,
  and unknown-ID rejection;
- Qt offscreen smoke for the status command and enable/disable projection;
- package/startup evidence after the composition root changes.
