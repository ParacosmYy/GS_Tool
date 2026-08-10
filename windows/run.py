"""Windows 体验入口：启动本机 Web，并自动打开默认浏览器。

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Keep the root launcher stable for personal, local-first experience.
Module: Windows runtime composition root
"""

from __future__ import annotations

import os
import socket
import sys
import threading
import webbrowser
from pathlib import Path

from token_tracker.web import create_app


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
    app = create_app()
    host = os.getenv("TOKEN_TRACKER_HOST", "127.0.0.1")
    configured_port = int(os.getenv("TOKEN_TRACKER_PORT", "5000"))
    port = resolve_local_port(host, configured_port)
    url = f"http://{host}:{port}"
    if os.getenv("TOKEN_TRACKER_NO_BROWSER", "0") != "1":
        threading.Timer(1.0, lambda: webbrowser.open(url)).start()
    print(f"AI Token Tracker is running at {url}")
    from waitress import serve

    serve(app, host=host, port=port)


if __name__ == "__main__":
    main()
