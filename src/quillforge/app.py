"""Application entry-point dispatcher and Qt event-loop owner."""

import hashlib
import json
import os
import sys
from collections.abc import Callable, Sequence
from pathlib import Path


def main(argv: Sequence[str] | None = None) -> int:
    """Dispatch diagnostics or run the composed Qt desktop runtime."""
    arguments = list(argv if argv is not None else sys.argv)
    safe_mode = "--safe-mode" in arguments
    if safe_mode:
        arguments = [argument for argument in arguments if argument != "--safe-mode"]
    _configure_frozen_qt_plugins()
    if "--plugin-host" in arguments:
        from .plugins.host_process import run_plugin_host

        return run_plugin_host(arguments)
    if "--diagnose-capture" in arguments:
        from .presentation.diagnostic_runner import run_capture_diagnostic

        diagnostic_start = arguments.index("--diagnose-capture")
        return run_capture_diagnostic(arguments[diagnostic_start:])
    if "--diagnose-workspace-search" in arguments:
        diagnostic_start = arguments.index("--diagnose-workspace-search")
        return _run_workspace_search_diagnostic(arguments[diagnostic_start:])
    if "--diagnose-file-open" in arguments:
        diagnostic_start = arguments.index("--diagnose-file-open")
        return _run_file_open_diagnostic(arguments[diagnostic_start:])
    if "--diagnose-startup" in arguments:
        diagnostic_start = arguments.index("--diagnose-startup")
        return _run_startup_diagnostic(arguments[diagnostic_start:])

    _validate_frozen_qt_runtime()
    from PyQt6.QtWidgets import QApplication

    from .application.desktop_launch import parse_desktop_launch_request
    from .composition import build_desktop_runtime

    launch_request = parse_desktop_launch_request(arguments)
    application = QApplication(list(launch_request.qt_arguments))
    runtime = build_desktop_runtime(
        application,
        startup_paths=launch_request.startup_paths,
        safe_mode=safe_mode,
    )
    try:
        runtime.start()
        return application.exec()
    finally:
        runtime.stop()


def _configure_frozen_qt_plugins() -> None:
    """Bind frozen Qt plugin discovery to this executable's extracted bundle."""
    if not getattr(sys, "frozen", False):
        return
    frozen_root = getattr(sys, "_MEIPASS", None)
    if not isinstance(frozen_root, str) or not frozen_root:
        return
    bundle_root = Path(frozen_root)
    plugin_root = _frozen_qt_plugin_root(bundle_root)
    if not plugin_root.is_dir():
        return
    os.environ["QT_PLUGIN_PATH"] = str(plugin_root)
    platform_root = plugin_root / "platforms"
    if platform_root.is_dir():
        os.environ["QT_QPA_PLATFORM_PLUGIN_PATH"] = str(platform_root)


def _frozen_qt_plugin_root(bundle_root: Path) -> Path:
    """Select a bundle plugin root with a usable Windows platform plugin first."""
    plugin_roots = (
        bundle_root / "PyQt6" / "Qt6" / "plugins",
        bundle_root / "PyQt6" / "Qt" / "plugins",
    )
    return next(
        (
            candidate
            for candidate in plugin_roots
            if (candidate / "platforms" / "qwindows.dll").is_file()
        ),
        next((candidate for candidate in plugin_roots if candidate.is_dir()), plugin_roots[0]),
    )


def _validate_frozen_qt_runtime() -> None:
    """Fail early with actionable paths when the frozen Qt bundle is incomplete."""
    if not getattr(sys, "frozen", False):
        return
    frozen_root = getattr(sys, "_MEIPASS", None)
    if not isinstance(frozen_root, str) or not frozen_root:
        raise RuntimeError("Frozen Qt runtime root is unavailable.")
    bundle_root = Path(frozen_root)
    plugin_check = _startup_qt_plugin_path(bundle_root)
    dependency_check = _startup_qt_runtime_dependencies(bundle_root)
    missing: list[str] = []
    if plugin_check.get("status") != "passed":
        platform_plugin = plugin_check.get("platform_plugin")
        if isinstance(platform_plugin, str):
            try:
                missing.append(str(Path(platform_plugin).relative_to(bundle_root)))
            except ValueError:
                missing.append(platform_plugin)
        else:
            missing.append("PyQt6/Qt6/plugins/platforms/qwindows.dll")
    missing.extend(
        value for value in dependency_check.get("missing", []) if isinstance(value, str) and value
    )
    if missing:
        unique_missing = ", ".join(sorted(set(missing)))
        raise RuntimeError(
            f"Frozen Qt runtime is incomplete; missing bundle files: {unique_missing}"
        )


def _run_workspace_search_diagnostic(arguments: list[str]) -> int:
    """Run a Qt-free packaged search probe against one explicit local root."""
    import argparse

    from .application.workspace_search import WorkspaceSearchQuery
    from .diagnostic_composition import build_workspace_search_service

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("root")
    parser.add_argument("literal")
    parser.add_argument("--case-sensitive", action="store_true")
    parser.add_argument("--report", required=True)
    try:
        options = parser.parse_args(arguments[1:])
    except SystemExit:
        return 2
    root = Path(options.root).expanduser().resolve()
    artifact = _diagnostic_artifact_identity()
    execution = "packaged-exe" if getattr(sys, "frozen", False) else "source-python"
    try:
        result = build_workspace_search_service().search(
            WorkspaceSearchQuery(root, options.literal, options.case_sensitive),
        )
    except (OSError, TypeError, ValueError) as error:
        Path(options.report).write_text(
            json.dumps(
                {
                    "diagnostic": "workspace-search",
                    "execution": execution,
                    "artifact": artifact,
                    "error": str(error),
                },
                ensure_ascii=False,
            ),
            encoding="utf-8",
        )
        return 2
    matches = []
    for match in result.matches:
        try:
            display_path = str(match.path.relative_to(root))
        except ValueError:
            display_path = str(match.path)
        matches.append(
            {
                "path": display_path,
                "line": match.line,
                "column": match.column,
                "preview": match.preview,
            }
        )
    payload = {
        "diagnostic": "workspace-search",
        "execution": execution,
        "artifact": artifact,
        "root": str(root),
        "query": options.literal,
        "case_sensitive": options.case_sensitive,
        "summary": result.summary(),
        "matches": matches,
        "files_scanned": result.files_scanned,
        "files_skipped": result.files_skipped,
        "bytes_scanned": result.bytes_scanned,
        "truncated": result.truncated,
        "cancelled": result.cancelled,
        "limit_reason": result.limit_reason,
        "issues_truncated": result.issues_truncated,
        "issues": [issue.reason for issue in result.issues],
        "issue_records": [
            {
                "path": str(issue.path.relative_to(root)),
                "reason": issue.reason,
            }
            for issue in result.issues
        ],
    }
    serialized = json.dumps(payload, ensure_ascii=False)
    print(serialized)
    Path(options.report).write_text(
        serialized,
        encoding="utf-8",
    )
    return 0


def _run_file_open_diagnostic(arguments: list[str]) -> int:
    """Run one explicit file through the production startup/open boundary."""
    import argparse

    from PyQt6.QtWidgets import QApplication

    from .composition import build_desktop_runtime
    from .infrastructure.recovery_store import (
        JsonRecoverySnapshotStore,
        default_recovery_directory,
    )

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("path")
    parser.add_argument("--report", required=True)
    try:
        options = parser.parse_args(arguments[1:])
    except SystemExit:
        return 2

    requested_path = Path(options.path).expanduser().resolve(strict=False)
    execution = "packaged-exe" if getattr(sys, "frozen", False) else "source-python"
    checks: dict[str, dict[str, object]] = {}
    try:
        if not requested_path.is_file():
            raise FileNotFoundError(f"The requested path is not a regular file: {requested_path}")
        recovery_root = default_recovery_directory().expanduser().resolve()
        recovery_candidates = JsonRecoverySnapshotStore(recovery_root).list_snapshots()
        if recovery_candidates:
            checks["file_open"] = {
                "status": "skipped",
                "reason": "recovery_candidates_require_explicit_user_decision",
                "file_present": True,
                "recovery_candidate_count": len(recovery_candidates),
                "window_shown": False,
                "event_loop_entered": False,
            }
        else:
            application = QApplication([])
            runtime = None
            try:
                runtime = build_desktop_runtime(application)
                runtime.prepare_startup()
                result = runtime.preflight_startup_paths((requested_path,))
                checks["file_open"] = {
                    "status": "passed",
                    "file_present": True,
                    "recovery_candidate_count": 0,
                    **result,
                }
            finally:
                if runtime is not None:
                    try:
                        runtime.stop()
                    except Exception:
                        pass
                application.quit()
    except Exception as error:
        checks["file_open"] = {
            "status": "failed",
            "file_present": requested_path.is_file(),
            "error_type": type(error).__name__,
            "error": str(error),
        }

    failed_checks = [name for name, result in checks.items() if result.get("status") == "failed"]
    payload = {
        "schema_version": "1.0",
        "diagnostic": "file-open",
        "execution": execution,
        "artifact": _diagnostic_artifact_identity(),
        "requested_path": str(requested_path),
        "checks": checks,
        "summary": {
            "status": "failed" if failed_checks else "passed",
            "failed_checks": failed_checks,
        },
    }
    serialized = json.dumps(payload, ensure_ascii=False, indent=2)
    report_path = Path(options.report).expanduser().resolve()
    try:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(serialized, encoding="utf-8")
    except OSError as error:
        print(f"Unable to write file-open diagnostic report: {error}", file=sys.stderr)
        return 2
    print(serialized)
    return 2 if failed_checks else 0


def _run_startup_diagnostic(arguments: list[str]) -> int:
    """Run a no-window package and composition preflight for a startup failure."""
    import argparse
    import importlib
    import platform

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--report", required=True)
    try:
        options = parser.parse_args(arguments[1:])
    except SystemExit:
        return 2

    execution = "packaged-exe" if getattr(sys, "frozen", False) else "source-python"
    frozen_root = getattr(sys, "_MEIPASS", None)
    checks: dict[str, dict[str, object]] = {}

    def probe(name: str, operation: Callable[[], dict[str, object]]) -> None:
        try:
            details = operation()
        except Exception as error:
            checks[name] = {
                "status": "failed",
                "error_type": type(error).__name__,
                "error": str(error),
            }
        else:
            checks[name] = {"status": "passed", **details}

    diagnostic_application = None
    owns_diagnostic_application = False
    try:
        from PyQt6.QtWidgets import QApplication

        diagnostic_application = QApplication.instance()
        if diagnostic_application is None:
            diagnostic_application = QApplication([])
            owns_diagnostic_application = True
    except Exception:
        diagnostic_application = None

    try:
        probe(
            "python_runtime",
            lambda: {"version": platform.python_version()},
        )
        probe(
            "pyqt6_qtcore",
            lambda: _startup_qtcore_versions(),
        )
        probe(
            "qscintilla",
            lambda: {"module": importlib.import_module("PyQt6.Qsci").__name__},
        )
        probe(
            "application_import",
            lambda: _startup_application_import(importlib.import_module),
        )
        probe(
            "composition_import",
            lambda: {"module": importlib.import_module("quillforge.composition").__name__},
        )
        probe("runtime_composition", _startup_runtime_composition)
        probe("settings_preflight", _startup_settings_preflight)
        probe("session_preflight", _startup_session_preflight)
        probe("recovery_preflight", _startup_recovery_preflight)
        probe("startup_restore_preflight", _startup_restore_preflight)
    finally:
        if owns_diagnostic_application and diagnostic_application is not None:
            try:
                diagnostic_application.quit()
            except Exception:
                pass

    if isinstance(frozen_root, str):
        resource_root = Path(frozen_root)
        checks["frozen_bundle"] = {
            "status": "passed",
            "root": str(resource_root),
        }
        qt_plugin_check = _startup_qt_plugin_path(resource_root)
        checks["qt_plugin_path"] = qt_plugin_check
        checks["qwindows_platform_plugin"] = {
            "status": qt_plugin_check["status"],
            "path": qt_plugin_check["platform_plugin"],
        }
        checks["qt_runtime_dependencies"] = _startup_qt_runtime_dependencies(resource_root)
        for name, relative in (("application_icon", "assets/quillforge.ico"),):
            path = resource_root / relative
            checks[name] = {
                "status": "passed" if path.is_file() else "failed",
                "path": str(path),
            }
    else:
        checks["frozen_bundle"] = {
            "status": "not_applicable",
            "reason": "source execution",
        }
        source_icon = Path(__file__).resolve().parents[2] / "assets" / "quillforge.ico"
        checks["application_icon"] = {
            "status": "passed" if source_icon.is_file() else "failed",
            "path": str(source_icon),
        }
        checks["qwindows_platform_plugin"] = {
            "status": "not_applicable",
            "reason": "source execution",
        }
        checks["qt_plugin_path"] = _startup_qt_plugin_path(None)
        checks["qt_runtime_dependencies"] = {
            "status": "not_applicable",
            "reason": "source execution",
        }

    failed_checks = [name for name, result in checks.items() if result.get("status") == "failed"]
    payload = {
        "schema_version": "1.0",
        "diagnostic": "startup",
        "execution": execution,
        "artifact": _diagnostic_artifact_identity(),
        "executable": str(Path(sys.executable).resolve()),
        "working_directory": str(Path.cwd().resolve()),
        "frozen_root": frozen_root,
        "checks": checks,
        "summary": {
            "status": "failed" if failed_checks else "passed",
            "failed_checks": failed_checks,
        },
    }
    serialized = json.dumps(payload, ensure_ascii=False, indent=2)
    report_path = Path(options.report).expanduser().resolve()
    try:
        report_path.parent.mkdir(parents=True, exist_ok=True)
        report_path.write_text(serialized, encoding="utf-8")
    except OSError as error:
        print(f"Unable to write startup diagnostic report: {error}", file=sys.stderr)
        return 2
    print(serialized)
    return 2 if failed_checks else 0


def _startup_runtime_composition() -> dict[str, object]:
    """Construct the desktop graph without showing a window or entering Qt."""
    from PyQt6.QtWidgets import QApplication

    from .composition import build_desktop_runtime

    application = QApplication.instance()
    owns_application = application is None
    if owns_application:
        application = QApplication([])
    runtime = None
    try:
        runtime = build_desktop_runtime(application)
        runtime.prepare_startup()
        runtime.refresh_startup_commands()
        runtime.preflight_editor_shell()
        return {
            "runtime": type(runtime).__name__,
            "window": type(runtime.window).__name__,
            "startup_prepared": True,
            "editor_shell_prepared": True,
            "window_shown": False,
            "event_loop_entered": False,
        }
    finally:
        if runtime is not None:
            try:
                runtime.stop()
            except Exception:
                pass
        if owns_application:
            try:
                application.quit()
            except Exception:
                pass


def _startup_qtcore_versions() -> dict[str, str]:
    """Return Qt/PyQt versions without constructing a QApplication."""
    from PyQt6 import QtCore

    return {
        "qt_version": QtCore.QT_VERSION_STR,
        "pyqt_version": QtCore.PYQT_VERSION_STR,
    }


def _startup_application_import(
    import_module: Callable[[str], object],
) -> dict[str, object]:
    """Record the packaged application module's import provenance."""
    module = import_module("quillforge.app")
    spec = getattr(module, "__spec__", None)
    loader = getattr(spec, "loader", None)
    origin = getattr(module, "__file__", None)
    return {
        "module": getattr(module, "__name__", None),
        "package": getattr(module, "__package__", None),
        "origin": str(origin) if isinstance(origin, (str, Path)) else None,
        "loader": type(loader).__name__ if loader is not None else None,
    }


def _startup_settings_preflight() -> dict[str, object]:
    """Report local settings readability without exposing user preference values."""
    from .application.settings import DEFAULT_SETTINGS, normalize_settings
    from .infrastructure.settings_store import JsonSettingsStore, default_settings_path

    path = default_settings_path().expanduser().resolve()
    stored_snapshot = JsonSettingsStore(path).load()
    normalized = (
        normalize_settings(stored_snapshot) if stored_snapshot is not None else DEFAULT_SETTINGS
    )
    return {
        "path": str(path),
        "file_present": path.is_file(),
        "stored_snapshot_valid": stored_snapshot is not None,
        "normalized_schema_version": normalized.schema_version,
    }


def _startup_session_preflight() -> dict[str, object]:
    """Report session continuity metadata through the production load policy."""
    from .application.session import SessionService
    from .infrastructure.session_store import JsonSessionStore, default_session_path

    path = default_session_path().expanduser().resolve()
    result = SessionService(JsonSessionStore(path)).load()
    snapshot = result.snapshot
    documents = snapshot.documents if snapshot is not None else ()
    return {
        "path": str(path),
        "file_present": path.is_file(),
        "load_state": result.state,
        "document_count": len(documents),
        "missing_document_count": sum(1 for document in documents if not document.path.is_file()),
        "workspace_root_present": (snapshot is not None and snapshot.workspace_root is not None),
    }


def _startup_recovery_preflight() -> dict[str, object]:
    """Report recovery-manifest health without returning snapshot content or paths."""
    from .infrastructure.recovery_store import (
        JsonRecoverySnapshotStore,
        default_recovery_directory,
    )

    root = default_recovery_directory().expanduser().resolve()
    directory_present = root.is_dir()
    snapshot_file_count = len(tuple(root.glob("*.qfrecovery"))) if directory_present else 0
    snapshots = JsonRecoverySnapshotStore(root).list_snapshots()
    valid_snapshot_count = len(snapshots)
    return {
        "root": str(root),
        "directory_present": directory_present,
        "snapshot_file_count": snapshot_file_count,
        "valid_snapshot_count": valid_snapshot_count,
        "invalid_snapshot_count": max(0, snapshot_file_count - valid_snapshot_count),
    }


def _startup_restore_preflight() -> dict[str, object]:
    """Exercise production restore only when no modal recovery decision is needed."""
    from PyQt6.QtWidgets import QApplication

    from .composition import build_desktop_runtime
    from .infrastructure.recovery_store import (
        JsonRecoverySnapshotStore,
        default_recovery_directory,
    )

    recovery_root = default_recovery_directory().expanduser().resolve()
    recovery_candidates = JsonRecoverySnapshotStore(recovery_root).list_snapshots()
    if recovery_candidates:
        return {
            "restore_stage": "skipped_recovery_candidates",
            "recovery_candidate_count": len(recovery_candidates),
            "startup_restore_completed": False,
            "window_shown": False,
            "event_loop_entered": False,
        }

    application = QApplication.instance()
    owns_application = application is None
    if owns_application:
        application = QApplication([])
    runtime = None
    try:
        runtime = build_desktop_runtime(application)
        runtime.prepare_startup()
        runtime.refresh_startup_commands()
        return {
            "restore_stage": "executed",
            "recovery_candidate_count": 0,
            **runtime.preflight_startup_restore(),
        }
    finally:
        if runtime is not None:
            try:
                runtime.stop()
            except Exception:
                pass
        if owns_application:
            try:
                application.quit()
            except Exception:
                pass


def _startup_qt_plugin_path(frozen_root: Path | None) -> dict[str, object]:
    """Report frozen Qt plugin presence without changing the runtime environment."""
    if frozen_root is None:
        return {"status": "not_applicable", "reason": "source execution"}

    import os

    plugin_root = _frozen_qt_plugin_root(frozen_root)
    platform_plugin = plugin_root / "platforms" / "qwindows.dll"
    configured_path = os.environ.get("QT_PLUGIN_PATH")
    return {
        "status": "passed" if platform_plugin.is_file() else "failed",
        "root": str(plugin_root),
        "root_exists": plugin_root.is_dir(),
        "platform_plugin": str(platform_plugin),
        "platform_plugin_exists": platform_plugin.is_file(),
        "environment_configured": bool(configured_path),
    }


def _startup_qt_runtime_dependencies(frozen_root: Path) -> dict[str, object]:
    """Report the frozen Qt/QScintilla binaries required before QApplication."""
    dependency_roots = (
        Path("PyQt6") / "Qt6" / "bin",
        Path("PyQt6") / "Qt" / "bin",
    )
    qscintilla = Path("PyQt6") / "Qsci.pyd"
    candidates: list[tuple[tuple[Path, ...], list[str]]] = []
    for dependency_root in dependency_roots:
        required = (
            dependency_root / "Qt6Core.dll",
            dependency_root / "Qt6Gui.dll",
            dependency_root / "Qt6Widgets.dll",
            qscintilla,
        )
        missing = [
            relative.as_posix() for relative in required if not (frozen_root / relative).is_file()
        ]
        if not missing:
            return {
                "status": "passed",
                "required": [relative.as_posix() for relative in required],
                "missing": [],
            }
        candidates.append((required, missing))
    required, missing = min(candidates, key=lambda candidate: len(candidate[1]))
    return {
        "status": "failed",
        "required": [relative.as_posix() for relative in required],
        "missing": missing,
    }


def _diagnostic_artifact_identity() -> dict[str, object] | None:
    """Bind a frozen diagnostic report to the executable that produced it."""
    if not getattr(sys, "frozen", False):
        return None
    executable = Path(sys.executable).resolve()
    try:
        digest = hashlib.sha256()
        with executable.open("rb") as source:
            for chunk in iter(lambda: source.read(1024 * 1024), b""):
                digest.update(chunk)
        return {
            "path": str(executable),
            "bytes": executable.stat().st_size,
            "sha256": digest.hexdigest().upper(),
        }
    except OSError as error:
        return {"path": str(executable), "error": str(error)}
