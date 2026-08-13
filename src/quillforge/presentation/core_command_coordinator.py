"""Qt-free registration of QuillForge's built-in application commands."""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass

from ..application.commands import Command, CommandRegistry


@dataclass(frozen=True, slots=True)
class CoreCommandPorts:
    """Application callbacks used to assemble the stable core command catalog."""

    new_document: Callable[[], None]
    open_document: Callable[[], None]
    choose_workspace: Callable[[], None]
    save_document: Callable[[], None]
    save_as_document: Callable[[], None]
    close_current_tab: Callable[[], None]
    show_recovery_candidates: Callable[[], None]
    quit_application: Callable[[], None]
    undo: Callable[[], None]
    redo: Callable[[], None]
    cut: Callable[[], None]
    copy: Callable[[], None]
    paste: Callable[[], None]
    select_all: Callable[[], None]
    show_find: Callable[[], None]
    show_replace: Callable[[], None]
    show_workspace_search: Callable[[], None]
    show_command_palette: Callable[[], None]
    show_settings: Callable[[], None]
    show_extension_catalog: Callable[[], None]
    show_plugin_status: Callable[[], None]
    show_plugin_host_diagnostics: Callable[[], None]
    show_about: Callable[[], None]


class CoreCommandCoordinator:
    """Register the built-in command catalog without owning command behavior."""

    def __init__(self, registry: CommandRegistry, ports: CoreCommandPorts) -> None:
        self._registry = registry
        self._ports = ports

    def register(self) -> None:
        """Register every built-in command in its established deterministic order."""
        for command in self._commands():
            self._registry.register(command)

    def _commands(self) -> tuple[Command, ...]:
        ports = self._ports
        return (
            Command("file.new", "&New", ports.new_document, "Ctrl+N", "file"),
            Command("file.open", "&Open...", ports.open_document, "Ctrl+O", "file"),
            Command(
                "file.open-workspace",
                "Open &Workspace...",
                ports.choose_workspace,
                menu_id="file",
            ),
            Command("file.save", "&Save", ports.save_document, "Ctrl+S", "file"),
            Command(
                "file.save-as",
                "Save &As...",
                ports.save_as_document,
                "Ctrl+Shift+S",
                "file",
            ),
            Command(
                "file.close",
                "&Close Tab",
                ports.close_current_tab,
                "Ctrl+W",
                "file",
            ),
            Command(
                "file.recover",
                "Recover Unsaved &Work...",
                ports.show_recovery_candidates,
                menu_id="file",
            ),
            Command("file.quit", "&Quit", ports.quit_application, "Ctrl+Q", "file"),
            Command("edit.undo", "&Undo", ports.undo, "Ctrl+Z", "edit"),
            Command("edit.redo", "&Redo", ports.redo, "Ctrl+Y", "edit"),
            Command("edit.cut", "Cu&t", ports.cut, "Ctrl+X", "edit"),
            Command("edit.copy", "&Copy", ports.copy, "Ctrl+C", "edit"),
            Command("edit.paste", "&Paste", ports.paste, "Ctrl+V", "edit"),
            Command("edit.select-all", "Select &All", ports.select_all, "Ctrl+A", "edit"),
            Command("edit.find", "&Find", ports.show_find, "Ctrl+F", "edit"),
            Command(
                "edit.replace",
                "Find and &Replace",
                ports.show_replace,
                "Ctrl+H",
                "edit",
            ),
            Command(
                "edit.find-in-files",
                "Find in &Files...",
                ports.show_workspace_search,
                "Ctrl+Shift+F",
                "edit",
            ),
            Command(
                "tools.command-palette",
                "Command &Palette",
                ports.show_command_palette,
                "Ctrl+Shift+P",
                "tools",
            ),
            Command("tools.settings", "&Settings...", ports.show_settings, menu_id="tools"),
            Command(
                "tools.extension-catalog",
                "&Extension Catalog...",
                ports.show_extension_catalog,
                menu_id="tools",
            ),
            Command(
                "tools.plugin-status",
                "&Plugin Status...",
                ports.show_plugin_status,
                menu_id="tools",
            ),
            Command(
                "tools.plugin-host-diagnostics",
                "Plugin Host &Diagnostics...",
                ports.show_plugin_host_diagnostics,
                menu_id="tools",
            ),
            Command(
                "help.about",
                "&About QuillForge",
                ports.show_about,
                menu_id="help",
            ),
        )


__all__ = ["CoreCommandCoordinator", "CoreCommandPorts"]
