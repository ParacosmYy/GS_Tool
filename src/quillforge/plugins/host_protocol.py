"""Strict, bounded wire contract for the diagnostic plugin host."""

import json
from collections.abc import Mapping
from dataclasses import dataclass

PLUGIN_HOST_PROTOCOL = "quillforge.plugin-host"
PLUGIN_HOST_PROTOCOL_VERSION = 1
PLUGIN_HOST_MAX_FRAME_BYTES = 64 * 1024

_HELLO_KEYS = frozenset(
    {
        "protocol",
        "protocol_version",
        "kind",
        "host_pid",
        "capabilities",
        "execution_enabled",
    }
)
_PROBE_KEYS = frozenset({"protocol", "protocol_version", "kind"})
_RESULT_KEYS = frozenset(
    {"protocol", "protocol_version", "kind", "state", "host_pid", "execution_enabled"}
)
_ERROR_KEYS = frozenset({"protocol", "protocol_version", "kind", "reason"})


class PluginHostProtocolError(ValueError):
    """Raised when a host frame violates the bounded protocol contract."""


@dataclass(frozen=True, slots=True)
class HostHello:
    """Validated host greeting."""

    host_pid: int
    capabilities: tuple[str, ...]
    execution_enabled: bool


@dataclass(frozen=True, slots=True)
class HostProbeReply:
    """Validated diagnostic result returned by a host."""

    state: str
    host_pid: int
    execution_enabled: bool


def encode_frame(message: Mapping[str, object]) -> bytes:
    """Encode one strict JSONL frame within the maximum byte bound."""
    if not isinstance(message, Mapping):
        raise PluginHostProtocolError("Host frame must be an object")
    try:
        payload = json.dumps(
            dict(message),
            ensure_ascii=False,
            allow_nan=False,
            separators=(",", ":"),
        ).encode("utf-8")
    except (TypeError, ValueError, UnicodeError) as error:
        raise PluginHostProtocolError(f"Host frame cannot be encoded: {error}") from error
    if len(payload) + 1 > PLUGIN_HOST_MAX_FRAME_BYTES:
        raise PluginHostProtocolError(f"Host frame exceeds {PLUGIN_HOST_MAX_FRAME_BYTES} bytes")
    return payload + b"\n"


def decode_frame(raw: bytes) -> dict[str, object]:
    """Decode one newline-terminated JSONL frame with duplicate-key rejection."""
    if not isinstance(raw, bytes):
        raise PluginHostProtocolError("Host frame must be bytes")
    if len(raw) > PLUGIN_HOST_MAX_FRAME_BYTES:
        raise PluginHostProtocolError(f"Host frame exceeds {PLUGIN_HOST_MAX_FRAME_BYTES} bytes")
    if not raw.endswith(b"\n"):
        raise PluginHostProtocolError("Host frame must be newline terminated")
    body = raw[:-1]
    if body.endswith(b"\r"):
        body = body[:-1]
    try:
        payload = json.loads(
            body,
            object_pairs_hook=_reject_duplicate_keys,
            parse_constant=_reject_non_finite,
        )
    except (TypeError, ValueError, UnicodeError) as error:
        raise PluginHostProtocolError(f"Host frame is invalid JSON: {error}") from error
    if not isinstance(payload, dict):
        raise PluginHostProtocolError("Host frame root must be an object")
    return payload


def make_hello(host_pid: int) -> dict[str, object]:
    """Build the diagnostic-only host greeting."""
    if type(host_pid) is not int or host_pid < 1:
        raise ValueError("Host PID must be a positive integer")
    return {
        "protocol": PLUGIN_HOST_PROTOCOL,
        "protocol_version": PLUGIN_HOST_PROTOCOL_VERSION,
        "kind": "hello",
        "host_pid": host_pid,
        "capabilities": ["probe"],
        "execution_enabled": False,
    }


def make_probe_request() -> dict[str, object]:
    """Build the only request supported by this delivery."""
    return {
        "protocol": PLUGIN_HOST_PROTOCOL,
        "protocol_version": PLUGIN_HOST_PROTOCOL_VERSION,
        "kind": "probe",
    }


def make_probe_result(host_pid: int) -> dict[str, object]:
    """Build a successful diagnostic result without enabling execution."""
    if type(host_pid) is not int or host_pid < 1:
        raise ValueError("Host PID must be a positive integer")
    return {
        "protocol": PLUGIN_HOST_PROTOCOL,
        "protocol_version": PLUGIN_HOST_PROTOCOL_VERSION,
        "kind": "probe-result",
        "state": "ready",
        "host_pid": host_pid,
        "execution_enabled": False,
    }


def make_error(reason: str) -> dict[str, object]:
    """Build a bounded protocol error without returning arbitrary tracebacks."""
    if type(reason) is not str or not reason.strip() or len(reason) > 256:
        raise ValueError("Host protocol error reason is invalid")
    return {
        "protocol": PLUGIN_HOST_PROTOCOL,
        "protocol_version": PLUGIN_HOST_PROTOCOL_VERSION,
        "kind": "error",
        "reason": reason,
    }


def parse_hello(message: Mapping[str, object]) -> HostHello:
    """Validate a host greeting and return a typed projection."""
    _validate_envelope(message, _HELLO_KEYS, "hello")
    host_pid = message["host_pid"]
    capabilities = message["capabilities"]
    execution_enabled = message["execution_enabled"]
    if type(host_pid) is not int or host_pid < 1:
        raise PluginHostProtocolError("Host hello PID is invalid")
    if (
        not isinstance(capabilities, list)
        or any(type(capability) is not str for capability in capabilities)
        or len(set(capabilities)) != len(capabilities)
        or tuple(capabilities) != ("probe",)
    ):
        raise PluginHostProtocolError("Host hello capabilities are invalid")
    if type(execution_enabled) is not bool or execution_enabled:
        raise PluginHostProtocolError("Host hello execution must remain disabled")
    return HostHello(host_pid, tuple(capabilities), execution_enabled)


def parse_probe_request(message: Mapping[str, object]) -> None:
    """Validate the diagnostic request; no plugin payload is accepted."""
    _validate_envelope(message, _PROBE_KEYS, "probe")


def parse_probe_result(message: Mapping[str, object]) -> HostProbeReply:
    """Validate a successful probe result."""
    _validate_envelope(message, _RESULT_KEYS, "probe-result")
    state = message["state"]
    host_pid = message["host_pid"]
    execution_enabled = message["execution_enabled"]
    if type(state) is not str or state not in {"ready", "rejected"}:
        raise PluginHostProtocolError("Host probe state is invalid")
    if type(host_pid) is not int or host_pid < 1:
        raise PluginHostProtocolError("Host probe PID is invalid")
    if type(execution_enabled) is not bool or execution_enabled:
        raise PluginHostProtocolError("Host probe execution must remain disabled")
    return HostProbeReply(state, host_pid, execution_enabled)


def parse_error(message: Mapping[str, object]) -> str:
    """Validate and return a bounded host error reason."""
    _validate_envelope(message, _ERROR_KEYS, "error")
    reason = message["reason"]
    if type(reason) is not str or not reason.strip() or len(reason) > 256:
        raise PluginHostProtocolError("Host error reason is invalid")
    return reason


def _validate_envelope(
    message: Mapping[str, object],
    expected_keys: frozenset[str],
    expected_kind: str,
) -> None:
    if not isinstance(message, Mapping) or set(message) != expected_keys:
        raise PluginHostProtocolError(f"Host {expected_kind} schema is invalid")
    if message["protocol"] != PLUGIN_HOST_PROTOCOL:
        raise PluginHostProtocolError("Host protocol name is unsupported")
    if message["protocol_version"] != PLUGIN_HOST_PROTOCOL_VERSION:
        raise PluginHostProtocolError("Host protocol version is unsupported")
    if message["kind"] != expected_kind:
        raise PluginHostProtocolError(f"Host message kind is not {expected_kind!r}")


def _reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result: dict[str, object] = {}
    for key, value in pairs:
        if key in result:
            raise PluginHostProtocolError(f"Duplicate host frame key: {key}")
        result[key] = value
    return result


def _reject_non_finite(value: str) -> object:
    raise PluginHostProtocolError(f"Non-finite JSON value is not allowed: {value}")
