"""Command-line entry point for the QuillForge desktop application."""

import locale
import os
import sys
import traceback
from collections.abc import Sequence
from datetime import UTC, datetime
from pathlib import Path

__all__ = ["main"]

_STARTUP_ERROR_LOG = "startup-error.log"


def main(argv: Sequence[str] | None = None) -> int:
    """Load and dispatch the application only inside the guarded entry call."""
    _clear_previous_startup_failure()
    if __package__:
        from .app import main as application_main
    else:
        # Keep direct `python src/quillforge/__main__.py` diagnostics useful while
        # letting the frozen importer resolve the package without source-style
        # `__file__` path surgery.
        if not getattr(sys, "frozen", False):
            sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
        from quillforge.app import main as application_main
    return application_main(argv)


def _startup_error_path() -> Path:
    """Return the user-local startup log path without depending on Qt."""
    local_app_data = os.environ.get("LOCALAPPDATA")
    base = Path(local_app_data) if local_app_data else Path.home() / ".local" / "share"
    return base / "QuillForge" / _STARTUP_ERROR_LOG


def _clear_previous_startup_failure() -> None:
    """Remove only the prior app-owned startup log before a new attempt."""
    try:
        _startup_error_path().unlink(missing_ok=True)
    except Exception:
        # A stale diagnostic must never prevent a new startup attempt.
        return


def _safe_resolved_path(value: object, fallback: str) -> str:
    """Resolve one diagnostic path without allowing context collection to fail startup."""
    if not isinstance(value, (str, Path)) or not str(value):
        return fallback
    try:
        return str(Path(value).expanduser().resolve())
    except (OSError, RuntimeError, TypeError, ValueError):
        return fallback


def _startup_context_lines() -> tuple[str, ...]:
    """Return fixed, path-only process context for an early startup traceback."""
    frozen_root = getattr(sys, "_MEIPASS", None)
    bundle_root = (
        _safe_resolved_path(frozen_root, "<unavailable>") if frozen_root else "<not-frozen>"
    )
    python_version = ".".join(str(part) for part in sys.version_info[:3])
    return (
        "execution_context:\n",
        f"executable={_safe_resolved_path(sys.executable, '<unavailable>')}\n",
        f"working_directory={_safe_resolved_path(Path.cwd(), '<unavailable>')}\n",
        f"frozen={bool(getattr(sys, 'frozen', False))}\n",
        f"python_version={python_version}\n",
        f"bundle_root={bundle_root}\n",
    )


def _record_startup_failure(error: Exception) -> Path | None:
    """Persist one actionable startup traceback when a windowed build fails early."""
    try:
        report_path = _startup_error_path()
    except Exception:
        # Startup diagnostics must never replace the original startup error.
        return None
    try:
        context_lines = _startup_context_lines()
    except Exception:
        context_lines = ("execution_context: unavailable\n",)
    try:
        payload = "".join(
            (
                f"timestamp_utc={datetime.now(UTC).isoformat()}\n",
                *context_lines,
                f"error_type={type(error).__name__}\n",
                f"error_message={error}\n",
                "traceback:\n",
                traceback.format_exc(),
            )
        )
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(payload, encoding="utf-8")
    except Exception:
        # Startup diagnostics must never replace the original startup error.
        return None
    return report_path


def _startup_locale() -> str:
    """Return a safe two-locale choice for the pre-Qt startup fallback."""
    candidates: list[object] = [
        os.environ.get("LANGUAGE"),
        os.environ.get("LC_ALL"),
        os.environ.get("LC_MESSAGES"),
        os.environ.get("LANG"),
    ]
    try:
        candidates.append(locale.getlocale()[0])
    except Exception:
        pass
    for candidate in candidates:
        if not isinstance(candidate, str):
            continue
        normalized = candidate.casefold().replace("-", "_")
        if normalized.startswith(("zh", "chinese (simplified)")):
            return "zh-CN"
    return "en-US"


def _show_startup_failure(error: Exception, report_path: Path | None) -> None:
    """Show a native fallback message because the normal Qt shell may not exist."""
    try:
        details = f"{type(error).__name__}: {error}"
        chinese = _startup_locale() == "zh-CN"
        if report_path is not None:
            diagnostic_label = "诊断日志：" if chinese else "Diagnostic log:"
            details = f"{details}\n\n{diagnostic_label}\n{report_path}"
        message = (
            f"QuillForge 无法启动。\n\n{details}"
            if chinese
            else f"QuillForge could not start.\n\n{details}"
        )
    except Exception:
        try:
            message = (
                "QuillForge 无法启动。\n\n启动诊断详情不可用。"
                if _startup_locale() == "zh-CN"
                else "QuillForge could not start.\n\nStartup diagnostic details unavailable."
            )
        except Exception:
            message = "QuillForge could not start.\n\nStartup diagnostic details unavailable."
    if os.name == "nt":
        try:
            import ctypes

            title = (
                "QuillForge 启动错误"
                if _startup_locale() == "zh-CN"
                else "QuillForge startup error"
            )
            displayed = ctypes.windll.user32.MessageBoxW(
                0,
                message,
                title,
                0x10,
            )
            if displayed:
                return
        except Exception:
            pass
    try:
        print(message, file=sys.stderr)
    except Exception:
        pass


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        _show_startup_failure(error, _record_startup_failure(error))
        raise SystemExit(1) from error
