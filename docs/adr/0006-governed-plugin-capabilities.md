# ADR 0006: Governed plugin capabilities and lifecycle

- **Status:** accepted for D6 governance MVP
- **Date:** 2026-08-09
- **Decision owner:** Architect

## Decision

QuillForge plugins remain explicitly registered in-process. Each manifest declares the compatible API version and a closed set of requested capabilities: commands, events, active-document snapshots, and notifications. The host records trust, active state, permissions, and failure diagnostics. Untrusted or incompatible plugins do not activate; undeclared capability calls raise a permission error inside the lifecycle boundary.

`PluginContext` tracks every command and event subscription owned by one plugin. Disable, deactivation failure, and command/event failure all dispose owned resources and publish `PluginFailed` without crashing the host. Diagnostics retain the original failure reason even when a plugin is subsequently disabled by policy.

## Limits

This MVP does not discover modules, download plugins, sandbox Python, or persist third-party settings. Those behaviors require separate threat modeling, trust policy, signing, and enterprise release evidence.
