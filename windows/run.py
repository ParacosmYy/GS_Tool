"""Windows 体验入口：启动本机 Web，并自动打开默认浏览器。

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep the root launcher stable for personal, local-first experience.
Module: Windows runtime composition root
"""

from __future__ import annotations

import ipaddress
import os
import socket
import sys
import threading
import webbrowser
from pathlib import Path


def configure_frozen_storage() -> None:
    """Give a packaged EXE a writable, user-scoped database location.

    PyInstaller extracts application resources to a temporary directory. Data
    must therefore live under LOCALAPPDATA rather than beside the executable.
    Source checkouts continue using the documented Windows data directory.
    """

    if not getattr(sys, "frozen", False):
        return
    local_app_data = Path(os.getenv("LOCALAPPDATA") or Path.home() / "AppData" / "Local")
    data_dir = local_app_data / "AITokenTracker"
    data_dir.mkdir(parents=True, exist_ok=True)
    os.environ.setdefault("TOKEN_TRACKER_DB", str(data_dir / "token_tracker.sqlite3"))


def resolve_local_host(configured_host: str | None) -> str:
    """Canonicalize the personal launcher to an IPv4 loopback address.

    The root entry is intended for one person's local experience. A host
    environment variable is therefore treated as an untrusted configuration
    boundary: only ``localhost`` and IPv4 loopback values are accepted.
    Explicit LAN sharing remains available through the documented
    ``--lan-preview`` command, which has a separate confirmation boundary.

    Args:
        configured_host: Value supplied by ``TOKEN_TRACKER_HOST``.

    Returns:
        The canonical IPv4 loopback address.

    Raises:
        ValueError: If the value is invalid, non-loopback, or IPv6.
    """

    candidate = (configured_host or "127.0.0.1").strip()
    if not candidate or candidate.casefold() == "localhost":
        return "127.0.0.1"
    try:
        address = ipaddress.ip_address(candidate)
    except ValueError as exc:
        raise ValueError(
            "个人体验入口只接受 loopback 地址；局域网请使用 --lan-preview。"
        ) from exc
    if not address.is_loopback:
        raise ValueError(
            "个人体验入口拒绝非 loopback 地址；局域网请使用 --lan-preview。"
        )
    if address.version != 4:
        raise ValueError(
            "个人体验入口当前只支持 IPv4 loopback 127.0.0.1；"
            "局域网请使用 --lan-preview。"
        )
    return "127.0.0.1"


def resolve_local_port(host: str, configured_port: int) -> int:
    """Choose a free local port without disturbing an existing service.

    The root launcher is a personal-experience entry point and may be started
    while an older development process is still running. Reusing that process
    would expose stale assets, while terminating it could destroy a user's
    active session. Probe a small deterministic range and let this process
    advertise the port it actually owns.
    """

    candidates = [configured_port, *range(configured_port + 1, configured_port + 21)]
    for candidate in candidates:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
            try:
                probe.bind((host, candidate))
            except OSError:
                continue
        if candidate != configured_port:
            print(
                f"Port {configured_port} is already in use; "
                f"the current source will use free port {candidate}."
            )
        return candidate
    raise RuntimeError(
        f"No free local port found in {configured_port}-{configured_port + 20}. "
        "Close an unused local service or set TOKEN_TRACKER_PORT."
    )


def main() -> None:
    """Compose the local app and serve it through the locked WSGI runtime."""

    configure_frozen_storage()
    # Import configuration only after the frozen storage boundary is set. The
    # application configuration loader reads the project ``.env`` file; doing
    # this import at module load time would let its relative database path win
    # over the packaged EXE's user-scoped LOCALAPPDATA path.
    from token_tracker.web import create_app

    try:
        host = resolve_local_host(os.getenv("TOKEN_TRACKER_HOST"))
    except ValueError as exc:
        print(f"本机体验启动被拒绝：{exc}", file=sys.stderr)
        raise SystemExit(2) from exc
    configured_port = int(os.getenv("TOKEN_TRACKER_PORT", "5000"))
    port = resolve_local_port(host, configured_port)
    app = create_app()
    url = f"http://{host}:{port}"
    if os.getenv("TOKEN_TRACKER_NO_BROWSER", "0") != "1":
        threading.Timer(1.0, lambda: webbrowser.open(url)).start()
    print(f"AI Token Tracker is running at {url}")
    from waitress import serve

    serve(app, host=host, port=port)


if __name__ == "__main__":
    main()
