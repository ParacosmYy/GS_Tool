"""Qt-free normalization of desktop launch arguments."""

from __future__ import annotations

from collections.abc import Sequence
from dataclasses import dataclass
from pathlib import Path

from ..domain.path_identity import path_key

_QT_ARGUMENTS_WITH_VALUES = frozenset(
    {
        "-display",
        "-platform",
        "-platformpluginpath",
        "-platformtheme",
        "-plugin",
        "-qmljsdebugger",
        "-qwindowgeometry",
        "-qwindowicon",
        "-qwindowtitle",
        "-session",
        "-style",
        "-stylesheet",
    }
)


@dataclass(frozen=True, slots=True)
class DesktopLaunchRequest:
    """Keep Qt arguments separate from explicit paths dropped onto the app."""

    qt_arguments: tuple[str, ...]
    startup_paths: tuple[Path, ...]


def parse_desktop_launch_request(arguments: Sequence[str]) -> DesktopLaunchRequest:
    """Split Qt options from ordered file or directory paths.

    A bare argument is an explicit startup path. ``--`` makes paths beginning
    with ``-`` unambiguous, while the small Qt value-option list prevents
    values such as ``windows`` in ``-platform windows`` from being misrouted.
    """
    raw_arguments = tuple(str(argument) for argument in arguments)
    if not raw_arguments:
        raw_arguments = ("quillforge",)

    qt_arguments = [raw_arguments[0]]
    startup_paths: list[Path] = []
    seen_paths: set[str] = set()
    paths_only = False
    index = 1
    while index < len(raw_arguments):
        argument = raw_arguments[index]
        if paths_only:
            _append_startup_path(argument, startup_paths, seen_paths)
        elif argument == "--":
            paths_only = True
        elif argument.startswith("-"):
            qt_arguments.append(argument)
            if _takes_qt_value(argument) and index + 1 < len(raw_arguments):
                index += 1
                qt_arguments.append(raw_arguments[index])
        else:
            _append_startup_path(argument, startup_paths, seen_paths)
        index += 1

    return DesktopLaunchRequest(tuple(qt_arguments), tuple(startup_paths))


def _append_startup_path(
    value: str,
    startup_paths: list[Path],
    seen_paths: set[str],
) -> None:
    """Normalize one explicit path and retain the first occurrence only."""
    path = Path(value).expanduser()
    try:
        path = path.resolve(strict=False)
    except (OSError, RuntimeError, ValueError):
        # Keep an unresolvable path so the presentation layer can report it
        # without making startup argument parsing fail.
        pass
    identity = path_key(path)
    if identity in seen_paths:
        return
    seen_paths.add(identity)
    startup_paths.append(path)


def _takes_qt_value(argument: str) -> bool:
    """Recognize Qt value options with either one or two leading dashes."""
    return f"-{argument.lstrip('-')}" in _QT_ARGUMENTS_WITH_VALUES


__all__ = ["DesktopLaunchRequest", "parse_desktop_launch_request"]
