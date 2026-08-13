"""Contract-only OTA extension boundary.

The package is intentionally not wired into the composition root yet. Future
transport and security adapters must implement the contracts here without
adding Qt, vendor SDK, or device-handle dependencies to the UI.
"""
